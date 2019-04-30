/**
  *  \file server/mailin/mailinapplication.cpp
  *  \brief Class server::mailin::MailInApplication
  */

#include "server/mailin/mailinapplication.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/net/headerconsumer.hpp"
#include "afl/net/mimeparser.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/parsedtime.hpp"
#include "afl/sys/time.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "server/mailin/mailprocessor.hpp"
#include "util/translation.hpp"
#include "version.hpp"
#include "server/ports.hpp"
#include "util/string.hpp"

using afl::string::Format;

namespace {
    const char LOG_NAME[] = "mailin";

    bool dumpWantHeader(const String_t& name)
    {
        using afl::string::strCaseCompare;
        return strCaseCompare(name, "Subject") == 0
            || strCaseCompare(name, "From") == 0
            || strCaseCompare(name, "To") == 0
            || strCaseCompare(name, "Date") == 0
            || strCaseCompare(name, "Message-Id") == 0
            || (name.size() > 8 && strCaseCompare(name.substr(0, 8), "Content-") == 0);
    }

    void dumpMail(afl::io::TextWriter& out, const afl::net::MimeParser& parser, String_t indent)
    {
        // Headers
        class HeaderVisitor : public afl::net::HeaderConsumer {
         public:
            HeaderVisitor(afl::io::TextWriter& out, const String_t& indent)
                : m_out(out), m_indent(indent)
                { }
            virtual void handleHeader(const String_t key, const String_t value)
                {
                    if (dumpWantHeader(key)) {
                        m_out.writeLine(Format("%s%s: %s", m_indent, key, value));
                    }
                }
         private:
            afl::io::TextWriter& m_out;
            const String_t& m_indent;
        };
        HeaderVisitor v(out, indent);
        parser.getHeaders().enumerateHeaders(v);

        // Trace
        String_t trace = parser.getTrace();
        if (!trace.empty()) {
            out.writeLine(Format("%sTRACE: %s", indent, trace));
        }

        // File name
        afl::base::Optional<String_t> fileName = parser.getFileName();
        if (const String_t* pfn = fileName.get()) {
            out.writeLine(Format("%sFile-Name: %s", indent, *pfn));
        }
        out.writeLine();

        // Content
        afl::base::Ptr<afl::base::Enumerator<afl::net::MimeParser> > parts = parser.getParts();
        if (parts.get() != 0) {
            afl::net::MimeParser part;
            int i = 0;
            while (parts->getNextElement(part)) {
                ++i;
                out.writeLine(Format("%s---- Part %d:", indent, i));
                dumpMail(out, part, indent + "     ");
            }
            if (i == 0) {
                out.writeLine(Format("%s---- Empty Multi-Part Message\n", indent));
            }
        } else {
            String_t ctyp = afl::string::strLCase(parser.getHeader("Content-Type").orElse(""));
            if (ctyp.empty() || (ctyp.size() > 4 && afl::string::strLCase(ctyp.substr(0, 4)) == "text")) {
                const afl::net::MimeParser::BodyVec_t& b = parser.getBody();
                for (afl::net::MimeParser::BodyVec_t::const_iterator i = b.begin(); i != b.end(); ++i) {
                    String_t line = *i;
                    if (line.size() > 75) {
                        line.erase(70);
                        line += "...";
                    }
                    out.writeLine(Format("%s%s", indent, line));
                }
            } else {
                out.writeLine(Format("%d(Non-Text Content)", indent));
            }
        }
    }
}


server::mailin::MailInApplication::MailInApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
    : Application(LOG_NAME, env, fs, net),
      m_dump(false),
      m_hostAddress(DEFAULT_ADDRESS, HOST_PORT),
      m_mailAddress(DEFAULT_ADDRESS, MAILOUT_PORT),
      m_rejectDirectory()
{ }

void
server::mailin::MailInApplication::serverMain()
{
    // Read mail into a buffer
    afl::io::InternalStream buffer;
    readMail(buffer);

    // Parse it
    afl::net::MimeParser parser;
    parser.handleFullData(buffer.getContent());

    // What to do?
    if (m_dump) {
        dumpMail(standardOutput(), parser, String_t());
    } else {
        // Connect to services
        afl::base::Deleter del;
        server::interface::MailQueueClient mail(createClient(m_mailAddress, del, false));
        afl::net::CommandHandler& host(createClient(m_hostAddress, del, false));

        // Do it
        bool status = MailProcessor(log(), mail, host).process(parser);
        if (!status) {
            if (!saveRejectedMail(buffer.getContent())) {
                exit(1);
            }
        }
    }
}

bool
server::mailin::MailInApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex planetscentral/mailin/mailin.cc:processConfig
    if (key == "HOST.HOST") {
        m_hostAddress.setName(value);
        return true;
    } else if (key == "HOST.PORT") {
        m_hostAddress.setService(value);
        return true;
    } else if (key == "MAILOUT.HOST") {
        m_mailAddress.setName(value);
        return true;
    } else if (key == "MAILOUT.PORT") {
        m_mailAddress.setService(value);
        return true;
    } else if (key == "MAILIN.REJECTDIR") {
        m_rejectDirectory = value;
        return true;
    } else {
        return false;
    }
}

bool
server::mailin::MailInApplication::handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& /*parser*/)
{
    if (option == "dump") {
        m_dump = true;
        return true;
    } else {
        return false;
    }
}

void
server::mailin::MailInApplication::readMail(afl::io::Stream& buffer)
{
    // Obtain standard input.
    // If there is no standard input, this will throw.
    afl::base::Ref<afl::io::Stream> input = environment().attachStream(afl::sys::Environment::Input);

    // Read
    buffer.copyFrom(*input);
}

bool
server::mailin::MailInApplication::saveRejectedMail(afl::base::ConstBytes_t buffer)
{
    // Do we want to save rejects?
    if (m_rejectDirectory.empty()) {
        return true;
    }

    // Generate file name
    afl::sys::ParsedTime pt;
    afl::sys::Time::getCurrentTime().unpack(pt, afl::sys::Time::UniversalTime);
    String_t timestamp = pt.format("%Y%m%d-%H%M%S");

    int index = 0;
    String_t fileName;
    afl::io::FileSystem& fs = fileSystem();
    do {
        fileName = fs.makePathName(m_rejectDirectory, Format("%s-%d", timestamp, ++index));
    } while (fs.openFileNT(fileName, fs.OpenRead).get() != 0);

    // Save
    try {
        fs.openFile(fileName, fs.Create)->fullWrite(buffer);
        log().write(afl::sys::LogListener::Info, LOG_NAME, Format("[reject] saved as '%s'", fileName));
    }
    catch (std::exception& e) {
        log().write(afl::sys::LogListener::Warn, LOG_NAME, Format("[error] writing file '%s'", fileName), e);
        return false;
    }
    return true;
}

String_t
server::mailin::MailInApplication::getApplicationName() const
{
    return afl::string::Format(_("PCC2 Incoming Mail Processor v%s - (c) 2017-2019 Stefan Reuther").c_str(), PCC2_VERSION);
}

String_t
server::mailin::MailInApplication::getCommandLineOptionHelp() const
{
    return "--dump\tShow mail content instead of submitting to server\n";
}
