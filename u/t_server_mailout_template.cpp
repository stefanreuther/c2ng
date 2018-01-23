/**
  *  \file u/t_server_mailout_template.cpp
  *  \brief Test for server::mailout::Template
  */

#include "server/mailout/template.hpp"

#include "t_server_mailout.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/net/nullnetworkstack.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/textfile.hpp"
#include "server/interface/composablecommandhandler.hpp"
#include "server/types.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/sys/thread.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/server.hpp"
#include "afl/net/name.hpp"
#include "afl/string/format.hpp"

/** Simple test. */
void
TestServerMailoutTemplate::testSimple()
{
    // Environment
    const char* INPUT =
        "From: me\n"
        "Subject: read this!\n"
        "\n"
        "Value is $(v)\n";
    afl::io::ConstMemoryStream in(afl::string::toBytes(INPUT));
    afl::io::TextFile textIn(in);
    afl::net::NullNetworkStack net;

    // Testee
    server::mailout::Template testee;
    testee.addVariable("v", "42");
    std::auto_ptr<afl::net::MimeBuilder> result(testee.generate(textIn, net, "user", "rx@host.invalid"));

    // Verify
    TS_ASSERT(result.get() != 0);

    afl::io::InternalSink out;
    result->write(out, false);

    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     "From: me\r\n"
                     "Subject: read this!\r\n"
                     "To: rx@host.invalid\r\n"
                     "Content-Type: text/plain; charset=UTF-8\r\n"
                     "Content-Transfer-Encoding: quoted-printable\r\n"
                     "\r\n"
                     "Value is 42\r\n");
}

/** Test header overrides. */
void
TestServerMailoutTemplate::testHeaderOverride()
{
    // Environment
    const char* INPUT =
        "From: me\n"
        "Subject: read this!\n"
        "Content-Type: text/html\n"
        "Content-Transfer-Encoding: none\n"
        "\n"
        "<html></html>\n";
    afl::io::ConstMemoryStream in(afl::string::toBytes(INPUT));
    afl::io::TextFile textIn(in);
    afl::net::NullNetworkStack net;

    // Testee
    server::mailout::Template testee;
    std::auto_ptr<afl::net::MimeBuilder> result(testee.generate(textIn, net, "user", "rx@host.invalid"));

    // Verify
    TS_ASSERT(result.get() != 0);

    afl::io::InternalSink out;
    result->write(out, false);

    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     "From: me\r\n"
                     "Subject: read this!\r\n"
                     "To: rx@host.invalid\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Transfer-Encoding: none\r\n"
                     "\r\n"
                     "<html></html>\r\n");
}

/** Test complex variable references. */
void
TestServerMailoutTemplate::testVariable()
{
    // Environment
    const char* INPUT =
        "From: me\n"
        "Subject: $(h_$(v))!\n"
        "\n"
        "Value $(v) is $(b_$(v))\n"
        "but $(w) is $(b_$(w))\n";
    afl::io::ConstMemoryStream in(afl::string::toBytes(INPUT));
    afl::io::TextFile textIn(in);
    afl::net::NullNetworkStack net;

    // Testee
    server::mailout::Template testee;
    testee.addVariable("v", "42");
    testee.addVariable("w", "99");
    testee.addVariable("h_42", "header");
    testee.addVariable("b_42", "body");
    std::auto_ptr<afl::net::MimeBuilder> result(testee.generate(textIn, net, "user", "rx@host.invalid"));

    // Verify
    TS_ASSERT(result.get() != 0);

    afl::io::InternalSink out;
    result->write(out, false);

    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     "From: me\r\n"
                     "Subject: header!\r\n"
                     "To: rx@host.invalid\r\n"
                     "Content-Type: text/plain; charset=UTF-8\r\n"
                     "Content-Transfer-Encoding: quoted-printable\r\n"
                     "\r\n"
                     "Value 42 is body\r\n"
                     "but 99 is \r\n");
}

/** Test conditionals. */
void
TestServerMailoutTemplate::testConditional()
{
    // Environment
    const char* INPUT =
        "From: me\n"
        "Subject: s\n"
        "!if $(a)\n"            // A header conditional.
        "X-A: yes\n"            // This one is taken...
        "!else\n"
        "X-A: no\n"             // This one is not.
        "!endif\n"
        "\n"
        "Text\n"
        "\n"
        "!if $(a)\n"            // A body conditional.
        "Conditional a\n"       // This one is taken.
        "!endif\n"
        "!if $(b)\n"            // Another body conditional which is not taken.
        "Conditional b\n"
        "!endif\n"
        "\n"
        "Final text\n";
    afl::io::ConstMemoryStream in(afl::string::toBytes(INPUT));
    afl::io::TextFile textIn(in);
    afl::net::NullNetworkStack net;

    // Testee
    server::mailout::Template testee;
    testee.addVariable("a", "1");
    std::auto_ptr<afl::net::MimeBuilder> result(testee.generate(textIn, net, "user", "rx@host.invalid"));

    // Verify
    TS_ASSERT(result.get() != 0);

    afl::io::InternalSink out;
    result->write(out, false);

    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     "From: me\r\n"
                     "Subject: s\r\n"
                     "X-A: yes\r\n"
                     "To: rx@host.invalid\r\n"
                     "Content-Type: text/plain; charset=UTF-8\r\n"
                     "Content-Transfer-Encoding: quoted-printable\r\n"
                     "\r\n"
                     "Text\r\n"
                     "\r\n"
                     "Conditional a\r\n"
                     "\r\n"
                     "Final text\r\n");
}

/** Test attachments. */
void
TestServerMailoutTemplate::testAttachment()
{
    /*
     *  Configuration
     */
    static const char*const FILE_NAME = "path/file.jpg";
    static const char*const REQUIRED_USER = "the_user";
    static const uint16_t PORT_NR = 20042;

    /*
     *  Server
     */
    class ServerMock : public server::interface::ComposableCommandHandler,
                       public afl::net::ProtocolHandlerFactory
    {
     public:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
            {
                if (upcasedCommand == "USER") {
                    TS_ASSERT_EQUALS(args.getNumArgs(), 1U);
                    m_user = server::toString(args.getNext());
                    return true;
                } else if (upcasedCommand == "GET") {
                    TS_ASSERT_EQUALS(args.getNumArgs(), 1U);
                    TS_ASSERT_EQUALS(m_user, REQUIRED_USER);
                    TS_ASSERT_EQUALS(server::toString(args.getNext()), FILE_NAME);
                    result.reset(server::makeStringValue("file content"));
                    return true;
                } else {
                    TS_ASSERT(false);
                    return false;
                }
            }
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::resp::ProtocolHandler(*this); }
     private:
        String_t m_user;
    };
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    ServerMock serverPH;
    afl::net::Server server(net.listen(afl::net::Name("127.0.0.1", PORT_NR), 10), serverPH);
    afl::sys::Thread serverThread("testAttachment", server);
    serverThread.start();

    /*
     *  Test it
     */
    
    // Environment
    const char* INPUT =
        "Subject: read this!\n"
        "\n"
        "Body\n";
    afl::io::ConstMemoryStream in(afl::string::toBytes(INPUT));
    afl::io::TextFile textIn(in);

    // Testee
    server::mailout::Template testee;
    testee.addFile(afl::string::Format("c2file://127.0.0.1:%d/path/file.jpg", PORT_NR));
    std::auto_ptr<afl::net::MimeBuilder> result(testee.generate(textIn, net, REQUIRED_USER, "rx@host.invalid"));

    // Shut down environment
    server.stop();
    serverThread.join();

    // Verify
    TS_ASSERT(result.get() != 0);

    afl::io::InternalSink out;
    result->write(out, false);

    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     "Content-Type: multipart/mixed; boundary=000\r\n"
                     "Subject: read this!\r\n"
                     "To: rx@host.invalid\r\n"
                     "\r\n"
                     "--000\r\n"
                     "Content-Type: text/plain; charset=UTF-8\r\n"
                     "Content-Disposition: inline\r\n"
                     "Content-Transfer-Encoding: quoted-printable\r\n"
                     "\r\n"
                     "Body\r\n"
                     "--000\r\n"
                     "Content-Type: image/jpeg\r\n"
                     "Content-Disposition: attachment; filename=\"file.jpg\"\r\n"
                     "Content-Transfer-Encoding: base64\r\n"
                     "\r\n"
                     "ZmlsZSBjb250ZW50\r\n"
                     "--000--\r\n");
}

