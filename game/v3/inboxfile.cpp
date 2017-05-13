/**
  *  \file game/v3/inboxfile.cpp
  *  \brief Class game::v3::InboxFile
  */

#include "game/v3/inboxfile.hpp"
#include "afl/except/fileformatexception.hpp"
#include "util/translation.hpp"

namespace {
    /** Remove a line, and return it.
        \param text [in/out] Message text, multiple lines separated by "\n"
        \return first line of message, including "\n" */
    String_t cutLine(String_t& text)
    {
        String_t::size_type n = text.find('\n');
        String_t result;
        if (n != text.npos) {
            result.assign(text, 0, n+1);
            text.erase(0, n+1);
        } else {
            result = text;
            text.clear();
        }
        return result;
    }

    /** Check for and update header line.
        If we got a "<CC" line, remove the "<".
        \param text [in/out] possible header line
        \return true iff this was a header line */
    bool checkTweakHeader(String_t& text)
    {
        if (text.size() >= 5 && text.compare(0, 5, "<CC: ", 5) == 0) {
            text.erase(0, 1);
        }

        static const char PREFIX1[] = "  <<< Universal Message >>>";
        static const size_t SIZE1 = sizeof(PREFIX1)-1;
        static const char PREFIX2[] = "CC: ";
        static const size_t SIZE2 = sizeof(PREFIX2)-1;
        return (text.size() >= SIZE1 && text.compare(0, SIZE1, PREFIX1, SIZE1) == 0)
            || (text.size() >= SIZE2 && text.compare(0, SIZE2, PREFIX2, SIZE2) == 0);
    }

    /** Tweak incoming message headers.
        This decodes the "<CC" hack.

        Messages to multiple receivers including the sender would trigger the PHost command processor.
        To avoid that, PCC automatically starts the message with "<".
        Because PCC2 always inserts a "CC:" line, that'll always be the affected line.

        In addition, because "CC:" or "<<< Universal Message >>>" are generated at the client side,
        they will may be after the blank line inserted by Host.
        Move them up to visually associate them with the headers. */
    String_t tweakIncomingHeader(String_t text)
    {
        // PCC 1.x:
        //   IF (EditText[4]='') THEN BEGIN
        //     IF TweakLine(5) THEN BEGIN
        //       EditText[4]:=EditText[5];
        //       EditText[5]:='';
        //     END;
        //   END ELSE TweakLine(4);
        // THost:
        //   --- Message 2 (/dos/g/games/MANOS2/HOST/_H2-1506.ZIP(PLAYER1.RST)) ---
        //   (-r1000)<< Sub Space Message >>
        //   FROM: Southern United Planets
        //   TO: Southern United Planets
        //
        //   <CC: 6 11
        //   (...text...)
        // PHost:
        //   --- Message 7 (/dos/g/games/NF/RSTS/TURN029.ZIP(player9.rst)) ---
        //   (-r9000)<<< Sub Space Message >>>
        //   FROM: The Robotic Imperium <9>
        //   TO  : The Robotic Imperium
        //   <CC: The Singing Pirates
        //   (...text...)

        if (text.size() > 10 && text[0] == '(' && text[2] == 'r') {
            // Copy first three lines
            String_t copy;
            for (int i = 0; i < 3; ++i) {
                copy += cutLine(text);
            }

            // Blank line?
            String_t line4 = cutLine(text);
            if (line4 == "\n") {
                String_t line5 = cutLine(text);
                if (checkTweakHeader(line5)) {
                    copy += line5;
                    copy += line4;
                } else {
                    copy += line4;
                    copy += line5;
                }
            } else {
                checkTweakHeader(line4);
                copy += line4;
            }

            // Append remainder
            copy += text;
            text = copy;
        }
        return text;
    }
}

// Parse a byte array into a message.
String_t
game::v3::decodeMessage(afl::base::ConstBytes_t data, afl::charset::Charset& charset, bool rewrap)
{
    // ex game/msg.h:getMessageFromArray
    afl::base::GrowableBytes_t result;
    enum { None, Before, Inside } rewrapStatus;

    // Winplan fixup needed?
    // It is if we have an (encoded) linefeed.
    rewrapStatus = None;
    if (rewrap && data.find(10+13) != data.size()) {
        rewrapStatus = Before;
    }

    // Strip trailing blanks
    const uint8_t* p;
    while ((p = data.atEnd(0)) != 0 && (*p == 32+13 || *p == 13+13 || *p == 13+10)) {
        data.removeEnd(1);
    }

    // Convert message
    bool skipping = false;
    while (const uint8_t* p = data.eat()) {
        uint8_t c = uint8_t(*p - 13);
        switch (c) {
         case 13:
            // CR. Regular line ending except if we're rewrapping Winplan mess.
            if (rewrapStatus != Inside) {
                if (rewrapStatus == Before) {
                    if (const uint8_t* p = result.atEnd(0)) {
                        if (*p == '\n') {
                            // a blank line, i.e. end of the headers
                            rewrapStatus = Inside;
                        }
                    }
                }
                result.append('\n');
            }
            skipping = false;
            break;
         case 10:
            // LF. Line ending in Winplan mess.
            if (rewrapStatus == Inside) {
                result.append('\n');
            }
            skipping = false;
            break;
         case 0:
            // NUL. Message cites Dominate's ship name which is followed by garbage.
            skipping = true;
            break;
         default:
            if (!skipping) {
                result.append(c);
            }
            break;
        }
    }
    return charset.decode(result);
}

/********************************* Inbox *********************************/

// Constructor.
game::v3::InboxFile::InboxFile(afl::io::Stream& file, afl::charset::Charset& charset)
    : m_file(file),
      m_charset(charset),
      m_directory()
{
    init();
}

// Destructor.
game::v3::InboxFile::~InboxFile()
{ }

// Get number of messages.
size_t
game::v3::InboxFile::getNumMessages() const
{
    return m_directory.size();
}

// Load a message.
String_t
game::v3::InboxFile::loadMessage(size_t index) const
{
    // ex GInbox::loadInbox (part)
    if (structures::IncomingMessageHeader* mh = m_directory.at(index)) {
        afl::base::GrowableMemory<uint8_t> buffer;
        buffer.resize(mh->length);
        m_file.setPos(mh->address-1);
        m_file.fullRead(buffer);
        return tweakIncomingHeader(decodeMessage(buffer, m_charset, true /* FIXME: getUserPreferences().RewrapMessages() */));
    } else {
        return String_t();
    }
}

// Initialize.
void
game::v3::InboxFile::init()
{
    // ex GInbox::loadInbox (part)

    // Read count
    structures::Int16_t rawCount;
    m_file.fullRead(afl::base::fromObject(rawCount));

    // Validate
    int count = rawCount;
    if (count < 0) {
        throw afl::except::FileFormatException(m_file, _("File is invalid"));
    }
    if (count == 0) {
        return;
    }

    // Read message directory
    m_directory.ensureSize(count);
    m_file.fullRead(m_directory.toBytes());

    // Verify message directory
    for (int i = 0; i < count; ++i) {
        structures::IncomingMessageHeader* mh = m_directory.at(i);
        if (mh->address <= 0 || mh->length <= 0) {
            throw afl::except::FileFormatException(m_file, _("File is invalid"));
        }
    }
}
