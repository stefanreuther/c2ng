/**
  *  \file game/v3/inboxfile.hpp
  *  \brief Class game::v3::InboxFile
  */
#ifndef C2NG_GAME_V3_INBOXFILE_HPP
#define C2NG_GAME_V3_INBOXFILE_HPP

#include "afl/base/growablememory.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/v3/structures.hpp"

namespace game { namespace v3 {

    /** Utility to read a v3 inbox file (MDATAx.DAT or RST MessageSection).
        Inbox files consist of a message directory followed by messages.
        The containing file defines the location of the message directory (beginning for MDATA, section for RST).
        Messages are addressed using absolute addresses and can thus be in any order/any place in the file.
        Messages are encrypted ("rot13"). */
    class InboxFile {
     public:
        /** Constructor.
            This will load the message directory.
            \param file File to read. Must be seekable. File pointer must be at beginning of message directory.
                        Object must live at least as long as the InboxFile.
            \param charset Game character set.
                        Object must live at least as long as the InboxFile.
            \param tx Translator
            \throw afl::except::FileFormatException on error */
        InboxFile(afl::io::Stream& file, afl::charset::Charset& charset, afl::string::Translator& tx);

        /** Destructor. */
        ~InboxFile();

        /** Get number of messages.
            \return number of messages */
        size_t getNumMessages() const;

        /** Load a message.
            This will actually access the file and load the message.
            The message is returned in C++ format; see decodeMessage().
            \param index Message number [0,getNumMessages())
            \return message; empty string if number is out of range */
        String_t loadMessage(size_t index) const;

     private:
        /** Initialize. This loads the message directory. */
        void init(afl::string::Translator& tx);

        afl::io::Stream& m_file;
        afl::charset::Charset& m_charset;
        afl::base::GrowableMemory<structures::IncomingMessageHeader> m_directory;
    };

    /** Parse a byte array into a message.
        This applies a few fixups to the message:
        - decode rot13;
        - Dominate fixup: drop everything from NUL to EOL;
        - Winplan fixup (if rewrap is on), see below.
        - Recoding character sets.

        The message is returned in C++ format (lines separated by '\n'),
        no matter what that means in binary.

        Winplan fixup: older Winplans send messages with CR+LF line terminators, not CR as usual.
        Host messes these up because it thinks the LFs are text characters, and inserts additional CRs.
        Therefore, we change the meaning of the control characters such that LF means a linefeed and CR is ignored
        (because it was most likely added by HOST).
        However, headers contain CRs that are real linefeeds.
        Note that this might return lines longer than 40 characters; display fixes those up.

        \param data message data
        \param charset game character set
        \param rewrap true iff data is from inbox (and user wants rewrap), false if from outbox

        More or less a copy of SendMsg::PChar2Edit, ReadMsg::DecodeMessage. */
    String_t decodeMessage(afl::base::ConstBytes_t data, afl::charset::Charset& charset, bool rewrap);

} }

#endif
