/**
  *  \file server/console/consoleapplication.cpp
  *  \brief Class server::console::ConsoleApplication
  */

#include "server/console/consoleapplication.hpp"
#include "afl/base/optional.hpp"
#include "afl/data/access.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/visitor.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "interpreter/values.hpp"
#include "server/console/arcanecommandhandler.hpp"
#include "server/console/connectioncontextfactory.hpp"
#include "server/console/context.hpp"
#include "server/console/contextfactory.hpp"
#include "server/console/filecommandhandler.hpp"
#include "server/console/fundamentalcommandhandler.hpp"
#include "server/console/integercommandhandler.hpp"
#include "server/console/parser.hpp"
#include "server/console/pipeterminal.hpp"
#include "server/console/routercontextfactory.hpp"
#include "server/console/stringcommandhandler.hpp"
#include "server/console/terminal.hpp"
#include "server/ports.hpp"
#include "server/types.hpp"
#include "version.hpp"
#include "util/string.hpp"

#ifdef TARGET_OS_POSIX
# include <unistd.h>
# define IS_INTERACTIVE_TERMINAL() isatty(0)
#else
# define IS_INTERACTIVE_TERMINAL() false
#endif

using afl::string::Format;

namespace {
    String_t quoteString(String_t s)
    {
        String_t result = "\"";
        for (size_t i = 0, n = s.size(); i < n; ++i) {
            if (s[i] == '\\' || s[i] == '"') {
                result += '\\';
            }
            result += s[i];
        }
        result += '"';
        return result;
    }

    void showValue(afl::io::TextWriter& out, afl::data::Value* p, String_t prefix)
    {
        // ex planetscentral/console/console.cc:showValue (sort-of)
        class Converter : public afl::data::Visitor {
         public:
            Converter(afl::io::TextWriter& out, const String_t& prefix)
                : m_out(out), m_prefix(prefix)
                { }
            virtual void visitString(const String_t& str)
                { m_out.writeText(quoteString(str)); }
            virtual void visitInteger(int32_t iv)
                { m_out.writeText(Format("%d", iv)); }
            virtual void visitFloat(double fv)
                { m_out.writeText(Format("%.25g", fv)); }
            virtual void visitBoolean(bool bv)
                { m_out.writeText(bv ? "true" : "false"); }
            virtual void visitHash(const afl::data::Hash& /*hv*/)
                { m_out.writeText("#<hash>"); }
            virtual void visitVector(const afl::data::Vector& vv)
                {
                    size_t n = vv.size();
                    if (n == 0) {
                        m_out.writeText("[ ]");
                    } else {
                        m_out.writeText("[");
                        for (size_t i = 0; i < n; ++i) {
                            m_out.writeLine();
                            m_out.writeText(m_prefix + "  ");
                            showValue(m_out, vv[i], m_prefix + "  ");
                            if (i+1 < n) {
                                m_out.writeText(",");
                            }
                        }
                        m_out.writeLine();
                        m_out.writeText(m_prefix + "]");
                    }
                }
            virtual void visitOther(const afl::data::Value& /*other*/)
                { m_out.writeText("#<other>"); }
            virtual void visitNull()
                { m_out.writeText("null"); }
            virtual void visitError(const String_t& /*source*/, const String_t& str)
                { m_out.writeText(Format("#<error:%s>", str)); }
         private:
            afl::io::TextWriter& m_out;
            const String_t& m_prefix;
        };
        Converter(out, prefix).visit(p);
    }

    class RootContext : public server::console::Context {
     public:
        RootContext(server::console::ConsoleApplication& app)
            : m_app(app)
            { }
        virtual bool call(const String_t& cmd, interpreter::Arguments args, server::console::Parser& parser, std::auto_ptr<afl::data::Value>& result)
            {
                // ex RootContext::processCommand
                if (server::console::ContextFactory* f = m_app.getContextFactoryByName(cmd)) {
                    std::auto_ptr<server::console::Context> ctx(f->create());
                    if (args.getNumArgs() > 0) {
                        return ctx->call(server::toString(args.getNext()), args, parser, result);
                    } else {
                        m_app.pushNewContext(ctx.release());
                        return true;
                    }
                } else {
                    return false;
                }
            }
        String_t getName()
            { return "c2console-ng"; }
     private:
        server::console::ConsoleApplication& m_app;
    };

}


// Constructor.
server::console::ConsoleApplication::ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
    : Application(env, fs),
      ConfigurationHandler(log(), "console"),
      m_networkStack(net),
      m_environment(),
      m_contextStack(),
      m_macros(m_environment)
{
    // Active contexts
    m_contextStack.pushBackNew(new RootContext(*this));

    // Available contexts
    m_availableContexts.pushBackNew(new ConnectionContextFactory("doc", DOC_PORT, m_networkStack));
    m_availableContexts.pushBackNew(new ConnectionContextFactory("file", FILE_PORT, m_networkStack));
    m_availableContexts.pushBackNew(new ConnectionContextFactory("format", FORMAT_PORT, m_networkStack));
    m_availableContexts.pushBackNew(new ConnectionContextFactory("host", HOST_PORT, m_networkStack));
    m_availableContexts.pushBackNew(new ConnectionContextFactory("hostfile", HOSTFILE_PORT, m_networkStack));
    m_availableContexts.pushBackNew(new ConnectionContextFactory("mailout", MAILOUT_PORT, m_networkStack));
    m_availableContexts.pushBackNew(new ConnectionContextFactory("redis", DB_PORT, m_networkStack));
    m_availableContexts.pushBackNew(new ConnectionContextFactory("talk", TALK_PORT, m_networkStack));
    m_availableContexts.pushBackNew(new ConnectionContextFactory("user", USER_PORT, m_networkStack));
    m_availableContexts.pushBackNew(new RouterContextFactory("router", m_networkStack));

    // Be quiet by default.
    consoleLogger().setConfiguration("*@-Info=hide", translator());
}

// Destructor.
server::console::ConsoleApplication::~ConsoleApplication()
{ }

// Application enty point.
void
server::console::ConsoleApplication::appMain()
{
    // ex planetscentral/console/console.cc:main (sort-of)
    // Parse args
    afl::string::Translator& tx = translator();
    afl::base::Ref<afl::sys::Environment::CommandLine_t> commandLine(environment().getCommandLine());
    afl::sys::StandardCommandLineParser commandLineParser(commandLine);
    String_t p;
    bool opt;
    afl::base::Optional<String_t> command;
    while (commandLineParser.getNext(opt, p)) {
        if (opt) {
            if (p == "h" || p == "help") {
                help();
            } else if (p == "log") {
                consoleLogger().setConfiguration(commandLineParser.getRequiredParameter("log"), tx);
            } else if (p == "proxy") {
                String_t url = commandLineParser.getRequiredParameter(p);
                if (!m_networkStack.add(url)) {
                    throw std::runtime_error(Format(tx("Unrecognized proxy URL: \"%s\""), url));
                }
            } else if (handleCommandLineOption(p, commandLineParser)) {
                // ok
            } else {
                errorExit(Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            String_t::size_type eq = p.find('=');
            if (eq != String_t::npos) {
                m_environment.setNew(String_t(p, 0, eq), Environment::ValuePtr_t(makeStringValue(String_t(p, eq+1))));
            } else {
                command = p;
                break;
            }
        }
    }

    // Load/process configuration
    loadConfigurationFile(environment(), fileSystem());

    // At this point, command is either empty (interactive mode),
    // or it is nonempty and the command line may contain more parameters.
    if (const String_t* verb = command.get()) {
        // Command mode
        afl::data::Segment cmd;

        String_t s;
        while (commandLine->getNextElement(s)) {
            cmd.pushBackString(s);
        }

        // Execute
        PipeTerminal term(standardOutput(), errorOutput());
        try {
            Parser p(m_environment, term, fileSystem(), *this);
            interpreter::Arguments args(cmd, 0, cmd.size());
            ValuePtr_t result;
            if (!call(*verb, args, p, result)) {
                throw std::runtime_error("Unknown command '" + *verb + "'");
            }
            if (result.get()) {
                term.printResultPrefix();
                showValue(standardOutput(), result.get(), String_t());
                term.printResultSuffix();
            }
        }
        catch (std::exception& e) {
            term.printError(e.what());
            exit(1);
        }
    } else {
        // Define a terminal
        std::auto_ptr<Terminal> term;
        if (IS_INTERACTIVE_TERMINAL()) {
            String_t terminalType = environment().getEnvironmentVariable("TERM");
            if (terminalType.empty() || terminalType.find("emacs") != String_t::npos) {
                term.reset(new DumbTerminal(standardOutput(), errorOutput()));
            } else {
                term.reset(new ColorTerminal(standardOutput(), errorOutput()));
            }
        } else {
            term.reset(new PipeTerminal(standardOutput(), errorOutput()));
        }
        term->printBanner();

        // Read interactively from standard input
        afl::base::Ref<afl::io::TextReader> input = environment().attachTextReader(afl::sys::Environment::Input);
        evaluateInteractive(*term, *input);
    }
}

// Get a ContextFactory, given a name.
server::console::ContextFactory*
server::console::ConsoleApplication::getContextFactoryByName(String_t name)
{
    for (size_t i = 0, n = m_availableContexts.size(); i < n; ++i) {
        if (name == m_availableContexts[i]->getCommandName()) {
            return m_availableContexts[i];
        }
    }
    return 0;
}

// Enter a new context.
void
server::console::ConsoleApplication::pushNewContext(Context* ctx)
{
    if (ctx != 0) {
        m_contextStack.pushBackNew(ctx);
    }
}

// Display help and exit.
void
server::console::ConsoleApplication::help()
{
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(Format(tx("PCC2 Console v%s - (c) 2017-2023 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-h]\n"
                            "  %$0s [--config=FILE] [-DKEY=VALUE] [ENV=VALUE] [COMMAND...]\n"
                            "\n"
                            "Options:\n"
                            "%s"
                            "\n"
                            "Report bugs to <Streu@gmx.de>").c_str(),
                         environment().getInvocationName(),
                         util::formatOptions(ConfigurationHandler::getHelp() +
                                             tx("--log=CONFIG\tSet logger configuration\n"
                                                "--proxy=URL\tAdd network proxy\n"
                                                "ENV=VALUE\tSet script environment variable\n"
                                                "COMMAND...\tCommand to execute (interactive if none)\n"))));
    out.flush();
    exit(0);
}

void
server::console::ConsoleApplication::evaluateInteractive(Terminal& term, afl::io::TextReader& in)
{
    // ex planetscentral/console/console.cc:evalStream
    while (!m_contextStack.empty()) {
        std::auto_ptr<afl::data::Value> lastResult;
        term.printPrimaryPrompt(m_contextStack);
        try {
            Parser p(m_environment, term, fileSystem(), *this);
            switch (p.evaluate(in, lastResult)) {
             case Parser::End:
                return;

             case Parser::BlankLine:
                break;

             case Parser::Command:
                if (lastResult.get() != 0) {
                    term.printResultPrefix();
                    showValue(standardOutput(), lastResult.get(), String_t());
                    term.printResultSuffix();
                }
                break;
            }
        }
        catch (std::exception& e) {
            term.printError(e.what());
            // FIXME: on PipeTerminal, c2console-classic would print the failing command
        }
    }
}

bool
server::console::ConsoleApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    bool ok = false;
    for (size_t i = 0, n = m_availableContexts.size(); i < n; ++i) {
        if (m_availableContexts[i]->handleConfiguration(key, value)) {
            ok = true;
        }
    }
    return ok;
}

bool
server::console::ConsoleApplication::call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result)
{
    // ex planetscentral/console/console.cc:evalCommand

    // If we have no context, we have seen an "exit" command or similar, but our caller didn't notice.
    // Exit as quickly as possible without spending precious CPU cycles.
    if (m_contextStack.empty()) {
        return true;
    }

    // Process global commands implemented in different CommandHandler's
    if (m_macros.call(cmd, args, parser, result)
        || FundamentalCommandHandler(m_environment).call(cmd, args, parser, result)
        || ArcaneCommandHandler(m_environment, *this).call(cmd, args, parser, result)
        || IntegerCommandHandler().call(cmd, args, parser, result)
        || StringCommandHandler().call(cmd, args, parser, result)
        || FileCommandHandler(fileSystem()).call(cmd, args, parser, result))
    {
        return true;
    }

    // Process global commands that need the ConsoleApplication environment
    if (cmd == ".." || cmd == "up" || cmd == "exit") {
        /* @q .. (Global Console Command), up (Global Console Command), exit (Global Console Command)
           Exit current context.
           If you are in the topmost context, exit c2console.
           @since PCC2 1.99.18, PCC2 2.40.3 */
        args.checkArgumentCount(0);
        m_contextStack.popBack();
        return true;
    }

    if (cmd == "load") {
        /* @q load FILE:Str... (Global Console Command)
           Load and execute command files.
           @since PCC2 1.99.19, PCC2 2.40.3 */
        while (args.getNumArgs() > 0) {
            afl::base::Ref<afl::io::Stream> s = fileSystem().openFile(toString(args.getNext()), afl::io::FileSystem::OpenRead);
            afl::io::TextFile tf(*s);
            PipeTerminal term(standardOutput(), errorOutput());
            evaluateInteractive(term, tf);
        }
        return true;
    }

    if (cmd == "die") {
        /* @q die TEXT:Str... (Global Console Command)
           Exit console.
           @since PCC2 1.99.19, PCC2 2.40.3 */
        String_t message;
        while (args.getNumArgs() > 0) {
            message += toString(args.getNext());
            if (args.getNumArgs() != 0) {
                message += ' ';
            }
        }
        parser.terminal().printError(message);
        exit(1);
        return true;
    }

    if (cmd == "fatal") {
        /* @q fatal COMMAND... (Global Console Command)
           Execute a command.
           If the command produces an error, terminates the console
           (default would be to log the error and proceed).
           @since PCC2 1.99.18, PCC2 2.40.3 */
        try {
            args.checkArgumentCountAtLeast(1);
            if (!call(toString(args.getNext()), args, parser, result)) {
                throw std::runtime_error("Unknown command");
            }
        }
        catch (std::exception& e) {
            parser.terminal().printError(e.what());
            exit(1);
        }
        return true;
    }

    if (cmd == "noerror") {
        /* @q noerror COMMAND... (Global Console Command)
           Execute a command, ignore errors.
           @since PCC2 1.99.18, PCC2 2.40.3 */
        if (args.getNumArgs() > 0) {
            try {
                call(toString(args.getNext()), args, parser, result);
            }
            catch (std::exception&)
            { }
        }
        return true;
    }
    if (cmd == "silent") {
        /* @q silent COMMAND... (Global Console Command)
           Execute a command, and suppress its result output.
           @since PCC2 1.99.18, PCC2 2.40.3 */
        args.checkArgumentCountAtLeast(1);

        std::auto_ptr<afl::data::Value> suppressedResult;
        return call(toString(args.getNext()), args, parser, suppressedResult);
    }

    return m_contextStack.back()->call(cmd, args, parser, result);
}
