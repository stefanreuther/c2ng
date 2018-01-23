/**
  *  \file gfx/gen/application.cpp
  *  \brief Class gfx::gen::Application
  */

#include "gfx/gen/application.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "afl/sys/time.hpp"
#include "gfx/gen/planetconfig.hpp"
#include "gfx/gen/spaceviewconfig.hpp"
#include "gfx/save.hpp"
#include "util/randomnumbergenerator.hpp"
#include "util/string.hpp"
#include "util/translation.hpp"
#include "version.hpp"

using afl::string::Format;

gfx::gen::Application::Application(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : util::Application(env, fs)
{ }

void
gfx::gen::Application::appMain()
{
    afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl = environment().getCommandLine();
    String_t verb;
    if (!cmdl->getNextElement(verb)) {
        errorExit(Format(_("no command specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
    }

    if (verb.size() > 1 && verb[0] == '-' && verb[1] == '-') {
        verb.erase(0, 1);
    }
    if (verb == "-h" || verb == "-help" || verb == "help") {
        showHelp();
    } else if (verb == "space") {
        doSpace(*cmdl);
    } else if (verb == "planet") {
        doPlanet(*cmdl);
    } else {
        errorExit(Format(_("invalid command \"%s\" specified. Use \"%s -h\" for help").c_str(), verb, environment().getInvocationName()));
    }
}

void
gfx::gen::Application::showHelp()
{
    afl::io::TextWriter& w = standardOutput();
    w.writeLine(afl::string::Format(_("Graphics Generator v%s - (c) 2017-2018 Stefan Reuther").c_str(), PCC2_VERSION));
    w.writeText(afl::string::Format(_("\n"
                                      "Usage:\n"
                                      "  %s [-h]\n"
                                      "  %0$s COMMAND [-OPTS]\n\n"
                                      "%s"
                                      "\n"
                                      "Report bugs to <Streu@gmx.de>\n").c_str(),
                                    environment().getInvocationName(),
                                    util::formatOptions(_("Command \"space\":\n"
                                                          "-w WIDTH\tSet width\n"
                                                          "-h HEIGHT\tSet height\n"
                                                          "-s SUNS\tSet number of suns\n"
                                                          "-p PROB\tSet star probability\n"
                                                          "-S SEED\tSet seed\n"
                                                          "-o FILE.bmp\tSet output file (mandatory)\n"
                                                          "\n"
                                                          "Command \"planet\":\n"
                                                          "-w WIDTH\tSet width\n"
                                                          "-h HEIGHT\tSet height\n"
                                                          "-x NN, -y NN\tSet planet position (percentage)\n"
                                                          "-r NN\tSet planet radius (percentage)\n"
                                                          "-t NN\tSet planet temperature\n"
                                                          "-X NN, -Y NN, -Z NN\tSet (invisible) sun position (percentage)\n"
                                                          "-S SEED\tSet seed\n"
                                                          "-o FILE.bmp\tSet output file (mandatory)\n"))));
    exit(0);
}

void
gfx::gen::Application::doSpace(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    // Configuration area
    afl::base::Optional<String_t> outputFileName;
    util::RandomNumberGenerator rng(afl::sys::Time::getTickCounter());
    SpaceViewConfig config;
    int w = 640;
    int h = 480;

    // Parse command line
    afl::sys::StandardCommandLineParser parser(cmdl);
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (!option) {
            errorExit(_("This command does not take positional parameters"));
        }
        if (text == "w") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), w) || w <= 0) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "h") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), h) || h <= 0) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "s") {
            int n;
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), n) || n < 0) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
            config.setNumSuns(n);
        } else if (text == "p") {
            int p;
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), p) || p < 0 || p >= 100) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
            config.setStarProbability(p);
        } else if (text == "S") {
            uint32_t seed = 0;
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), seed)) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
            rng.setSeed(seed);
        } else if (text == "o") {
            outputFileName = parser.getRequiredParameter(text);
        } else {
            errorExit(Format(_("invalid option specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
        }
    }

    // Finalize
    const String_t* pOutputFileName = outputFileName.get();
    if (pOutputFileName == 0) {
        errorExit(_("output file name (\"-o\") not specified"));
    }
    config.setSize(Point(w, h));

    // Generate
    afl::base::Ref<RGBAPixmap> result(config.render(rng));

    // Save
    saveCanvas(*result->makeCanvas(), *fileSystem().openFile(*pOutputFileName, afl::io::FileSystem::Create));
}

void
gfx::gen::Application::doPlanet(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    // Configuration area
    afl::base::Optional<String_t> outputFileName;
    util::RandomNumberGenerator rng(afl::sys::Time::getTickCounter());
    int w = 640;
    int h = 480;
    int px = 50, py = 50, pr = 40, pt = 50;
    int sx = -1000, sy = -1000, sz = -1000;

    // Parse command line
    afl::sys::StandardCommandLineParser parser(cmdl);
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (!option) {
            errorExit(_("This command does not take positional parameters"));
        }
        if (text == "w") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), w) || w <= 0) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "h") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), h) || h <= 0) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "x") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), px)) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "y") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), py)) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "r") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), pr) || pr <= 0) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "t") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), pt) || pt < 0 || pt > 100) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "X") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), sx)) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "Y") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), sy)) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "Z") {
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), sz)) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
        } else if (text == "S") {
            uint32_t seed = 0;
            if (!afl::string::strToInteger(parser.getRequiredParameter(text), seed)) {
                errorExit(Format(_("parameter for \"-%s\" is invalid").c_str(), text));
            }
            rng.setSeed(seed);
        } else if (text == "o") {
            outputFileName = parser.getRequiredParameter(text);
        } else {
            errorExit(Format(_("invalid option specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
        }
    }

    // Finalize
    const String_t* pOutputFileName = outputFileName.get();
    if (pOutputFileName == 0) {
        errorExit(_("output file name (\"-o\") not specified"));
    }

    PlanetConfig config;
    config.setSize(Point(w, h));
    config.setPlanetPosition(px, py);
    config.setPlanetRadius(pr);
    config.setPlanetTemperature(pt);
    config.setSunPosition(sx, sy, sz);

    // Generate
    afl::base::Ref<RGBAPixmap> result(config.render(rng));

    // Save
    saveCanvas(*result->makeCanvas(), *fileSystem().openFile(*pOutputFileName, afl::io::FileSystem::Create));
}
