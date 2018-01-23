/**
  *  \file server/dbexport/exportapplication.cpp
  *  \brief Class server::dbexport::ExportApplication
  */

#include "server/dbexport/exportapplication.hpp"
#include "afl/base/optional.hpp"
#include "afl/net/resp/client.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "server/dbexport/dbexporter.hpp"
#include "server/ports.hpp"
#include "util/translation.hpp"
#include "version.hpp"

using afl::string::Format;

server::dbexport::ExportApplication::ExportApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
    : Application(env, fs),
      ConfigurationHandler(log(), "dbexport"),
      m_networkStack(net),
      m_dbAddress(DEFAULT_ADDRESS, DB_PORT)
{
    // Be quiet by default.
    consoleLogger().setConfiguration("*@-Info=hide");
}

server::dbexport::ExportApplication::~ExportApplication()
{ }

// Application enty point.
void
server::dbexport::ExportApplication::appMain()
{
    // Parse args until we obtain a command
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
                consoleLogger().setConfiguration(commandLineParser.getRequiredParameter("log"));
            } else if (handleCommandLineOption(p, commandLineParser)) {
                // ok
            } else {
                errorExit(Format(_("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            command = p;
            break;
        }
    }

    // Did we get a command?
    const String_t* pCommand = command.get();
    if (pCommand == 0) {
        errorExit(_("no command specified"));
    }

    // Load/process configuration
    loadConfigurationFile(environment(), fileSystem());

    // Do it [exception protection provided by caller, Application]
    afl::base::Deleter del;
    if (*pCommand == "db") {
        exportDatabase(standardOutput(), createClient(del, m_dbAddress), commandLineParser);
    } else {
        errorExit(Format(_("unknown command: \"%s\"").c_str(), *pCommand));
    }
}

bool
server::dbexport::ExportApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex planetscentral/dbexport/dbexport.cc:checkConfig1, checkConfig
    if (key == "REDIS.HOST") {
        m_dbAddress.setName(value);
        return true;
    } else if (key == "REDIS.PORT") {
        m_dbAddress.setService(value);
        return true;
    } else {
        return false;
    }
}

void
server::dbexport::ExportApplication::help()
{
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(Format(_("PCC2 Database Export v%s - (c) 2017-2018 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(_("Usage: c2dbexport [--config=FILE] [-DKEY=VALUE] COMMAND [ARGS...]\n"
                           "\n"
                           "Options:\n"
                           "  --config=FILE       Set path to config file\n"
                           "  --log=CONFIG        Set logger configuration\n"
                           "  -DKEY=VALUE         Override config file entry\n"
                           "\n"
                           "Commands:\n"
                           "  db [--delete] WILDCARD...     export database keys\n"
                           "\n"
                           "This utility creates c2console (*.con) scripts to restore\n"
                           "a particular situation / set of data in the same or another\n"
                           "PlanetsCentral database instance.\n"
                           "\n"
                           "Report bugs to <Streu@gmx.de>\n").c_str(),
                         environment().getInvocationName()));
    exit(0);
}

afl::net::CommandHandler&
server::dbexport::ExportApplication::createClient(afl::base::Deleter& del, const afl::net::Name& name)
{
    return del.addNew(new afl::net::resp::Client(m_networkStack, name));
}
