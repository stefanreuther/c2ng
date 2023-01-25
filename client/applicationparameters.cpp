/**
  *  \file client/applicationparameters.cpp
  *  \brief Class client::ApplicationParameters
  */

#include "client/applicationparameters.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/longcommandlineparser.hpp"
#include "game/limits.hpp"
#include "util/string.hpp"

client::ApplicationParameters::ApplicationParameters(gfx::Application& app, const String_t& programTitle)
    : m_app(app),
      m_programTitle(programTitle),
      m_windowParameters(),
      m_traceConfig(),
      m_gameDirectory(),
      m_proxyAddress(),
      m_password(),
      m_commandLineResources(),
      m_requestThreadDelay(0),
      m_playerNumber(0),
      m_directoryMode(OpenGame)
{
    m_windowParameters.title        = app.translator()("Planets Command Center II (c2ng)");
    m_windowParameters.size         = gfx::Point(800, 600);
    m_windowParameters.bitsPerPixel = 32;
}

void
client::ApplicationParameters::parse(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    afl::string::Translator& tx = m_app.translator();
    afl::sys::LongCommandLineParser parser(cmdl);
    bool option;
    String_t text;
    while (parser.getNext(option, text)) {
        if (option) {
            if (handleWindowParameterOption(m_windowParameters, text, parser, tx)) {
                // ok
            } else if (text == "debug-request-delay") {
                int value = 0;
                if (!afl::string::strToInteger(parser.getRequiredParameter(text), value) || value < 0) {
                    throw afl::except::CommandLineException(afl::string::Format(tx("Invalid argument to command line parameter \"-%s\""), text));
                }
                m_requestThreadDelay = value;
            } else if (text == "dir") {
                m_directoryMode = OpenBrowser;
            } else if (text == "help") {
                doHelp();
            } else if (text == "log") {
                util::addListItem(m_traceConfig, ":", parser.getRequiredParameter(text));
            } else if (text == "password") {
                m_password = parser.getRequiredParameter(text);
            } else if (text == "proxy") {
                m_proxyAddress = parser.getRequiredParameter(text);
            } else if (text == "resource") {
                m_commandLineResources.push_back(parser.getRequiredParameter(text));
            } else {
                throw afl::except::CommandLineException(afl::string::Format(tx("Unknown command line parameter \"-%s\""), text));
            }
        } else {
            int n;
            if (m_playerNumber == 0 && afl::string::strToInteger(text, n) && n > 0 && n < game::MAX_PLAYERS) {
                m_playerNumber = n;
            } else if (!m_gameDirectory.isValid()) {
                m_gameDirectory = text;
            } else {
                throw afl::except::CommandLineException(afl::string::Format(tx("Excess parameter \"%s\""), text));
            }
        }
    }
}

void
client::ApplicationParameters::doHelp()
{
    afl::string::Translator& tx = m_app.translator();
    String_t help = m_programTitle;
    help += "\n\n";
    help += tx("Usage: c2ng [-options] gamedir [player]");
    help += "\n\n";
    help += tx("Options:");
    help += "\n";
    help += util::formatOptions(tx("-dir\tOpen browser\n"
                                   "-log=CONFIG\tConfigure log output\n"
                                   "-password=PASS\tResult file password\n"
                                   "-proxy=URL\tSet network proxy\n"
                                   "-resource=NAME\tAdd resource provider\n")
                                + gfx::getWindowParameterHelp(tx));
    help += "\n";
    help += tx("(c) copyright 2017-2023 Stefan Reuther <streu@gmx.de>");
    help += "\n";
    m_app.dialog().showInfo(help, m_programTitle);
    m_app.exit(0);
}
