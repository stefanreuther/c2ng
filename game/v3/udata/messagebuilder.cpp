/**
  *  \file game/v3/udata/messagebuilder.cpp
  *  \brief Class game::v3::udata::MessageBuilder
  */

#include <cmath>
#include "game/v3/udata/messagebuilder.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/bits/fixedstring.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/udata/nameprovider.hpp"
#include "game/v3/udata/reader.hpp"
#include "util/string.hpp"

namespace gt = game::v3::structures;
using afl::string::Format;
using afl::sys::LogListener;

namespace {
    const char*const LOG_NAME = "game.v3.udata";

    String_t makeMessage(const String_t& header, const String_t& intro, const String_t& body)
    {
        String_t result = header;
        result += "\n\n";
        result += intro;
        result += "\n\n";
        result += body;
        return result;
    }
}

/** Definition of one util.dat record type. */
struct game::v3::udata::MessageBuilder::Description {
    uint16_t recordType;             ///< Record type.
    uint16_t alias;                  ///< Alias record type, if hasAlias is set.
    bool hasAlias;                   ///< If set, take textTemplate and size parameters from record identified by @c alias.
    size_t loopSize;                 ///< If nonzero, loop size.
    size_t fixedSize;                ///< If loop present, size of invariant part that is repeated in each iteration.
    size_t loadLimit;                ///< Maximum bytes to load.
    String_t name;                   ///< Name of record.
    String_t headerTemplate;         ///< Header template (single line, with placeholders).
    String_t textTemplate;           ///< Text template (multiple lines).

    Description(uint16_t recordType, const String_t& name)
        : recordType(recordType),
          alias(),
          hasAlias(false),
          loopSize(0),
          fixedSize(0),
          loadLimit(-1U),
          name(name),
          headerTemplate("(-h0000)"),
          textTemplate()
        { }
};

/*
 *  MessageBuilder
 */

game::v3::udata::MessageBuilder::MessageBuilder(NameProvider& provider, afl::charset::Charset& cs, afl::string::Translator& tx)
    : m_provider(provider),
      m_charset(cs),
      m_translator(tx),
      m_descriptions()
{
    // GUtilMessageParser::GUtilMessageParser
}

game::v3::udata::MessageBuilder::~MessageBuilder()
{
    // GUtilMessageParser::~GUtilMessageParser
}

void
game::v3::udata::MessageBuilder::loadDefinition(afl::io::Stream& in, afl::sys::LogListener& log)
{
    // GUtilMessageParser::loadDefinition(Stream& in), utilmsgs.pas:InitUtilViewer
    afl::io::TextFile tf(in);
    String_t line;
    Description* current = 0;

    while (tf.readLine(line)) {
        line = afl::string::strTrim(line);
        if (line.empty() || line[0] == ';') {
            continue;
        }
        String_t::size_type p = line.find_first_of(",=");
        if (p == String_t::npos) {
            log.write(LogListener::Error, LOG_NAME, Format(m_translator("%s:%d: missing delimiter"), in.getName(), tf.getLineNumber()));
        } else if (line[p] == '=') {
            // It's an assignment
            if (!current) {
                continue;
            }

            String_t lhs = afl::string::strTrim(line.substr(0, p));
            String_t rhs = afl::string::strTrim(line.substr(p+1));
            bool ok = true;
            if (util::stringMatch("Heading", lhs)) {
                current->headerTemplate = rhs;
            } else if (util::stringMatch("Text", lhs)) {
                current->textTemplate += rhs;
                current->textTemplate += '\n';
            } else if (util::stringMatch("Alias", lhs)) {
                ok = afl::string::strToInteger(rhs, current->alias);
                if (ok) {
                    current->hasAlias = true;
                }
            } else if (util::stringMatch("Loop", lhs)) {
                ok = afl::string::strToInteger(rhs, current->loopSize);
            } else if (util::stringMatch("Fixed", lhs)) {
                ok = afl::string::strToInteger(rhs, current->fixedSize);
            } else if (util::stringMatch("Max", lhs)) {
                ok = afl::string::strToInteger(rhs, current->loadLimit);
            }

            if (!ok) {
                log.write(LogListener::Error, LOG_NAME, Format(m_translator("%s:%d: invalid number"), in.getName(), tf.getLineNumber()));
            }
        } else {
            // New record
            uint16_t id;
            if (!afl::string::strToInteger(afl::string::strTrim(line.substr(0, p)), id)) {
                log.write(LogListener::Error, LOG_NAME, Format(m_translator("%s:%d: invalid number"), in.getName(), tf.getLineNumber()));
                current = 0;
            } else {
                current = m_descriptions.pushBackNew(new Description(id, afl::string::strTrim(line.substr(p+1))));
            }
        }
    }
}

void
game::v3::udata::MessageBuilder::loadFile(afl::io::Stream& in, game::msg::Inbox& out) const
{
    // GUtilMessageParser::loadFile
    class MyReader : public Reader {
     public:
        MyReader(game::msg::Inbox& out, const MessageBuilder& parent)
            : m_out(out), m_parent(parent), m_turnNumber()
            { }

        virtual bool handleRecord(uint16_t recordId, afl::base::ConstBytes_t data)
            {
                // Snoop turn number
                if (recordId == gt::UTIL_CONTROL_ID && data.size() >= sizeof(gt::Util13ControlMinimal)) {
                    gt::Util13ControlMinimal controlData;
                    afl::base::fromObject(controlData).copyFrom(data);
                    m_turnNumber = controlData.turnNumber;
                }

                // Process data
                m_parent.addRecord(m_out, m_turnNumber, recordId, data);
                return true;
            }

        virtual void handleError(afl::io::Stream& /*in*/)
            { }

        virtual void handleEnd()
            { }

     private:
        game::msg::Inbox& m_out;
        const MessageBuilder& m_parent;
        int m_turnNumber;
    };

    MyReader(out, *this).read(in);
}

/** Get description, given a record type.
    @param recordType Type to look up
    @return description; null if not found */
const game::v3::udata::MessageBuilder::Description*
game::v3::udata::MessageBuilder::getDescriptionByType(uint16_t recordType) const
{
    // ex GUtilMessageParser::getDescriptionById(int id), utilmsgs.pas:GetRecordById
    for (size_t i = 0, n = m_descriptions.size(); i < n; ++i) {
        if (m_descriptions[i]->recordType == recordType) {
            return m_descriptions[i];
        }
    }
    return 0;
}

/** Add single record from file.
    Look up and process the type definition.
    @param [out] out         Mailbox
    @param [in]  turnNumber  Turn number to apply to message
    @param [in]  recordType  Record type
    @param [in]  data        Record data */
void
game::v3::udata::MessageBuilder::addRecord(game::msg::Inbox& out, int turnNumber, uint16_t recordType, afl::base::ConstBytes_t data) const
{
    // ex GUtilMessageParser::addRecord, CUtilMailbox.Msg2Edit, CUtilMailbox.ReadFile
    const Description* desc = getDescriptionByType(recordType);
    if (desc == 0) {
        // Undefined
        out.addMessage(makeMessage(m_translator("(-h0000)<<< Unknown >>>"),
                                   Format(m_translator("Record type %d, %d byte%!1{s%}"), recordType, data.size()),
                                   m_translator("Unknown record type.")),
                       turnNumber);
    } else {
        // Save header
        const String_t& headerTemplate = desc->headerTemplate;
        const String_t& name = desc->name;

        // Chase 'alias' links
        int paranoia = 10;
        while (desc != 0 && desc->hasAlias && paranoia > 0) {
            desc = getDescriptionByType(desc->alias);
            --paranoia;
        }
        if (desc == 0) {
            out.addMessage(makeMessage(Format("%s<<< %s >>>", decodeRecord(data, headerTemplate), name),
                                       Format(m_translator("Record type %d, %d byte%!1{s%}"), recordType, data.size()),
                                       m_translator("Unknown reference target in record definition.")),
                           turnNumber);
        } else {
            // Apply loadLimit
            const size_t origSize = data.size();
            data.trim(desc->loadLimit);

            // Apply loop
            if (desc->loopSize == 0) {
                out.addMessage(makeMessage(Format("%s<<< %s >>>", decodeRecord(data, headerTemplate), name),
                                           Format(m_translator("Record type %d, %d byte%!1{s%}"), recordType, origSize),
                                           decodeRecord(data, desc->textTemplate)),
                               turnNumber);
            } else {
                int partNumber = 0;
                for (size_t i = desc->fixedSize; i < data.size(); i += desc->loopSize) {
                    afl::base::GrowableBytes_t tmp;
                    tmp.reserve(desc->fixedSize + desc->loopSize);
                    tmp.append(data.subrange(0, desc->fixedSize));
                    tmp.append(data.subrange(i, desc->loopSize));
                    ++partNumber;
                    out.addMessage(makeMessage(Format("%s<<< %s >>>", decodeRecord(tmp, headerTemplate), name),
                                               Format(m_translator("Record type %d, part %d"), recordType, partNumber),
                                               decodeRecord(tmp, desc->textTemplate)),
                                   turnNumber);
                }
            }
        }
    }
}

/** Decode record according to a template.
    @param data Data (for loops, invariant part plus loop part concatenated to single blob)
    @param tpl  Template string
    @return Result string */
String_t
game::v3::udata::MessageBuilder::decodeRecord(afl::base::ConstBytes_t data, const String_t& tpl) const
{
    // GUtilMessageParser::decodeRecord, utilmsgs.pas:DecodeRecord
    size_t idx = 0;
    const size_t len = tpl.size();

    bool had_undef = false;
    String_t out_line;
    String_t out_text;
    size_t cur_dat = 0;
    while (idx < len) {
        char c = tpl[idx++];
        if (c == '%') {
            // Determine cursor and process flags
            size_t i = 0;
            bool tmp = false;
            bool ignore_undef = false;
            bool force_print = false;
            int32_t L = -1;

            while (idx < len) {
                c = tpl[idx++];
                if (c >= '0' && c <= '9') {
                    i = 10*i + (c - '0');
                    tmp = true;
                } else if (c == '?') {
                    ignore_undef = true;
                } else if (c == '!') {
                    force_print = true;
                } else {
                    break;
                }
            }
            if (tmp) {
                cur_dat = i;
            }

            // Handle item
            // If we left the above loop by idx >= len (syntax error), this will interpret the
            // '%' or final flag as type code. Why not.
            String_t app;
            switch (c) {
             case 'S':
             {
                 size_t slen = 0;
                 for (int i = 0; i < 2; ++i) {
                     if (idx < len && tpl[idx] >= '0' && tpl[idx] <= '9') {
                         slen = 10*slen + tpl[idx++] - '0';
                     }
                 }

                 afl::base::ConstBytes_t bytes = data.subrange(cur_dat, slen);
                 cur_dat += slen;
                 if (bytes.size() == slen) {
                     app = m_charset.decode(afl::bits::unpackFixedString(bytes));
                 } else {
                     if (!ignore_undef) {
                         had_undef = true;
                     }
                 }
                 break;
             }
             case 'X':
             case 'l':
             case 'F':
                // long
                if (const afl::bits::Int32LE::Bytes_t* p = data.subrange(cur_dat).eatN<4>()) {
                    L = afl::bits::Int32LE::unpack(*p);
                } else {
                    L = -1;
                }
                cur_dat += 4;
                goto FormatNum;
             case 'b':
                // byte
                if (const uint8_t* p = data.at(cur_dat)) {
                    L = *p;
                } else {
                    L = -1;
                }
                ++cur_dat;
                goto FormatNum;
             case '%':
                app = "%";
                break;
             case '|':
                app = "  ";
                break;
             default:
                // words
                if (const afl::bits::Int16LE::Bytes_t* p = data.subrange(cur_dat).eatN<2>()) {
                    L = afl::bits::Int16LE::unpack(*p);
                } else {
                    L = -1;
                }
                cur_dat += 2;
             FormatNum:
                if (!force_print && ((L == -1) || ((L == 0 && std::strchr("prhg", c) != 0)))) {
                    if (!ignore_undef) {
                        had_undef = true;
                    }
                } else {
                    if (c == 'g') {
                        app = m_provider.getName(NameProvider::NativeGovernmentName, L);
                    } else if (c == 'h') {
                        app = m_provider.getName(NameProvider::HullName, L);
                    } else if (c == 'H') {
                        app = m_provider.getName(NameProvider::HullFunctionName, L);
                    } else if (c == 'n') {
                        app = m_provider.getName(NameProvider::NativeRaceName, L);
                    } else if (c == 'B') {
                        for (int i = 0; i <= 15; ++i) {
                            if ((L & (1L << i)) != 0) {
                                util::addListItem(app, " ", Format("%d", i));
                            }
                        }
                        if (app.empty()) {
                            app = m_translator("none");
                        }
                    } else if (c == 'p') {
                        app = m_provider.getName(NameProvider::PlanetName, L);
                    } else if (c == 'r') {
                        app = m_provider.getName(NameProvider::ShortRaceName, L);
                    } else if (c == 'u') {
                        if (const Description* def = getDescriptionByType(uint16_t(L))) {
                            app = def->name;
                        } else {
                            app = Format("%d", L);
                        }
                    } else if (c == 'W') {
                        app = Format("%04d", L);
                    } else if (c == 'R') {
                        app = Format("%5d", L);
                    } else if (c == 'X') {
                        app = Format("%08X", uint32_t(L));
                    } else if (c == 'x') {
                        app = Format("%04x", uint16_t(L & 65535));
                    } else if (c == 'F') {
                        app = Format("%04d", std::abs(L));
                        app.insert(app.size()-3, ".");
                        if (L < 0) {
                            app.insert(0, "-");
                        }
                    } else if (c == '(') {
                        int32_t cnt = 0;
                        while (idx < len) {
                            c = tpl[idx++];
                            if (c == ',') {
                                ++cnt;
                            } else if (c == ')') {
                                break;
                            } else if (c == '\n') {
                                // skip
                            } else {
                                if (cnt == L) {
                                    app += c;
                                }
                            }
                        }
                        if (app.empty()) {
                            app = Format("%d", L);
                        }
                    } else {
                        app = Format("%d", L);
                    }
                }
            }
            out_line += app;
        } else if (c == '\n') {
            // New line
            if (!had_undef && (!out_line.empty() || out_text.size() <= 2 || out_text[out_text.size()-2] != '\n')) {
                out_text += out_line;
                out_text += '\n';
            }
            out_line.clear();
            had_undef = false;
        } else {
            // normal character
            out_line += c;
        }
    }

    // Add potential partial line
    out_text += out_line;

    return out_text;
}
