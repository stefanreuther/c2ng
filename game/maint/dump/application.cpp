/**
  *  \file game/maint/dump/application.cpp
  *  \brief Class game::maint::dump::Application
  */

#include "game/maint/dump/application.hpp"

#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/maint/dump/input.hpp"
#include "game/maint/dump/output.hpp"
#include "game/maint/dump/parsers.hpp"
#include "game/maint/dump/textoutput.hpp"
#include "game/maint/dump/typedetector.hpp"
#include "util/charsetfactory.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::string::Format;

void
game::maint::dump::Application::appMain()
{
    afl::string::Translator& tx = translator();
    afl::io::FileSystem& fs = fileSystem();

    enum { NoPreference, PlayerFiles, HostFiles } opt_prefs = NoPreference;
    bool hasFiles = false;
    int exitCode = 0;

    TypeDetector tydet;
    std::auto_ptr<afl::charset::Charset> charset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));

    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "t") {
                tydet.setRequiredType(commandLine.getRequiredParameter(p));
            } else if (p == "p") {
                opt_prefs = PlayerFiles;
            } else if (p == "m") {
                opt_prefs = HostFiles;
            } else if (p == "h" || p == "help") {
                help(standardOutput());
                exit(0);
            } else if (p == "C" || p == "charset") {
                if (afl::charset::Charset* cs = util::CharsetFactory().createCharset(commandLine.getRequiredParameter(p))) {
                    charset.reset(cs);
                } else {
                    errorExit(tx("the specified character set is not known"));
                }
            } else {
                errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
            }
        } else {
            // It's a file argument
            hasFiles = true;

            tydet.setFileBaseName(fs.getFileName(p));
            tydet.start();
            tydet.match("bdata",    "bdata",                            Parsers::parseBDataFile);
            tydet.match("beamspec", "beamspec",                         Parsers::parseBeamSpec);
            tydet.match("engspec",  "engspec",                          Parsers::parseEngSpec);
            tydet.match("gendat", opt_prefs == PlayerFiles ? "gen" : 0, Parsers::parseGenFile);
            tydet.match("hullspec", "hullspec",                         Parsers::parseHullSpec);
            tydet.match("names",    "planet",                           Parsers::parseNameList);
            tydet.match("names",    "storm",                            Parsers::parseNameList);
            tydet.match("pdata",    "pdata",                            Parsers::parsePDataFile);
            tydet.match("racenm",   "race",                             Parsers::parseRaceNames);
            tydet.match("ship",     "ship",                             Parsers::parseShipFile);
            tydet.match("target",   "target",                           Parsers::parseTargetFile);
            tydet.match("torpspec", "torpspec",                         Parsers::parseTorpSpec);
            tydet.match("truehull", "truehull",                         Parsers::parseTrueHull);
            tydet.match("xyplan",   "xyplan",                           Parsers::parseXYPlan);
            tydet.match("team",     "team",                             Parsers::parseTeams);
            tydet.match("vcr",      "vcr",                              Parsers::parseVcrFile);

            if (tydet.getNumPossibleTypes() == 0) {
                if (tydet.hasRequiredType()) {
                    reportError(Format(tx("%s: type '%s' is unknown"), p, tydet.getRequiredType()));
                } else {
                    reportError(Format(tx("%s: unable to auto-detect type"), p));
                }
                exitCode = 1;
            } else if (tydet.getNumPossibleTypes() > 1) {
                reportError(Format(tx("%s: file matches multiple types, use '-tTYPE' to specify correct one"), p));
                exitCode = 1;
            } else {
                try {
                    afl::base::Ref<afl::io::Stream> s = fs.openFile(p, afl::io::FileSystem::OpenRead);
                    afl::io::InternalStream is;
                    is.copyFrom(*s);

                    standardOutput().writeLine(Format("=== Dump of file '%s' using type '%s' ===", p, tydet.getType()));

                    Input in(is.getContent(), *charset);
                    TextOutput out(standardOutput());
                    tydet.getParser()(in, out);

                    in.dumpRemainder(out);
                }
                catch (afl::except::FileProblemException& e) {
                    reportError(Format("%s: %s", e.getFileName(), e.what()));
                    exitCode = 1;
                }
                catch (std::exception& e) {
                    reportError(e.what());
                    exitCode = 1;
                }
            }
        }
    }

    if (!hasFiles) {
        errorExit(Format(tx("No file names specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }
    exit(exitCode);
}

void
game::maint::dump::Application::help(afl::io::TextWriter& out)
{
    afl::string::Translator& tx = translator();
    out.writeLine(Format(tx("PCC2 File Dump Utility %s - (c) 2010-2025 Stefan Reuther"), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-tTYPE] FILE [[-tTYPE] FILE...]\n\n"
                            "%s"
                            "\n"
                            "Report bugs to <Streu@gmx.de>\n"),
                         environment().getInvocationName(),
                         util::formatOptions(tx("Options:\n"
                                                "-tTYPE\tSelect type to use to interpret following files\n"
                                                "-p\tAutodetect player files\n"
                                                "-m\tAutodetect host files\n"
                                                "-CCHARSET\tSet character set\n"))));
}

void
game::maint::dump::Application::reportError(String_t msg)
{
    errorOutput().writeLine(msg);
}
