/**
  *  \file gfx/gen/application.cpp
  *  \brief Class gfx::gen::Application
  */

#include <stdexcept>
#include "gfx/gen/application.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "afl/sys/time.hpp"
#include "gfx/gen/explosionrenderer.hpp"
#include "gfx/gen/orbitconfig.hpp"
#include "gfx/gen/planetconfig.hpp"
#include "gfx/gen/shieldrenderer.hpp"
#include "gfx/gen/spaceviewconfig.hpp"
#include "gfx/gen/texture.hpp"
#include "gfx/save.hpp"
#include "util/randomnumbergenerator.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::string::Format;
using afl::string::strToInteger;

namespace gfx { namespace gen { namespace {
    void executeTextureCommand(Texture& tex, const String_t& command, util::RandomNumberGenerator& rng)
    {
        util::StringParser p(command);
        bool ok;
        if (p.parseString("fill(")) {
            // "fill(color)"
            ColorQuad_t color;
            if (parseColor(p, color) && p.parseString(")") && p.parseEnd()) {
                tex.fill(color);
                ok = true;
            } else {
                ok = false;
            }
        } else if (p.parseString("noise(")) {
            // "noise(range)"
            ColorRange r;
            if (r.parse(p) && p.parseString(")") && p.parseEnd()) {
                tex.fillNoise(r, rng);
                ok = true;
            } else {
                ok = false;
            }
        } else if (p.parseString("brush(")) {
            // "brush(range[,angle=N,n=N])"
            ColorRange r;
            int angle = 0;
            int n = tex.pixmap().getWidth() * tex.pixmap().getHeight() / 200;
            ok = r.parse(p);
            while (ok) {
                if (p.parseString(",n=")) {
                    ok = p.parseInt(n) && n > 0;
                } else if (p.parseString(",angle=")) {
                    ok = p.parseInt(angle) && n >= 0;
                } else if (p.parseString(")")) {
                    ok = p.parseEnd();
                    break;
                } else {
                    ok = false;
                }
            }
            if (ok) {
                tex.renderBrush(r, n, angle, rng);
            }
        } else if (p.parseString("circ(")) {
            // "circ(range,x,y,r[,noise])"
            ColorRange r;
            int x, y;
            int radius;
            int noise = 0;
            ok = (r.parse(p) && p.parseCharacter(',')
                  && p.parseInt(x) && p.parseCharacter(',')
                  && p.parseInt(y) && p.parseCharacter(',')
                  && p.parseInt(radius));
            if (ok && p.parseCharacter(',')) {
                ok = p.parseInt(noise);
            }
            if (ok) {
                ok = p.parseCharacter(')') && p.parseEnd();
            }
            if (ok) {
                tex.renderCircularGradient(Point(x, y), radius, r, rng, noise);
            }
        } else {
            ok = false;
        }
        if (!ok) {
            throw std::runtime_error(Format("Command syntax error at '%s'", p.getRemainder().substr(0, 20)));
        }
    }
} } }



struct gfx::gen::Application::CommonOptions {
    afl::base::Optional<String_t> outputFileName;
    util::RandomNumberGenerator rng;
    int w;
    int h;

    CommonOptions()
        : outputFileName(),
          rng(afl::sys::Time::getTickCounter()),
          w(640),
          h(480)
        { }
};


gfx::gen::Application::Application(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : util::Application(env, fs)
{ }

void
gfx::gen::Application::appMain()
{
    afl::string::Translator& tx = translator();
    afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl = environment().getCommandLine();
    String_t verb;
    if (!cmdl->getNextElement(verb)) {
        errorExit(Format(tx("no command specified. Use \"%s -h\" for help"), environment().getInvocationName()));
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
    } else if (verb == "orbit") {
        doOrbit(*cmdl);
    } else if (verb == "explosion") {
        doExplosion(*cmdl);
    } else if (verb == "shield") {
        doShield(*cmdl);
    } else if (verb == "texture") {
        doTexture(*cmdl);
    } else {
        errorExit(Format(tx("invalid command \"%s\" specified. Use \"%s -h\" for help"), verb, environment().getInvocationName()));
    }
}

void
gfx::gen::Application::showHelp()
{
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& w = standardOutput();
    w.writeLine(Format(tx("PCC2 Procedural Graphics Generator v%s - (c) 2017-2022 Stefan Reuther"), PCC2_VERSION));
    w.writeText(Format(tx("\n"
                          "Usage:\n"
                          "  %s [-h]\n"
                          "  %0$s COMMAND [-OPTS]\n\n"
                          "%s"
                          "\n"
                          "Report bugs to <Streu@gmx.de>\n"),
                       environment().getInvocationName(),
                       util::formatOptions(tx("Common options:\n"
                                              "-w WIDTH\tSet width\n"
                                              "-h HEIGHT\tSet height\n"
                                              "-S SEED\tSet seed\n"
                                              "-o FILE.bmp\tSet output file (mandatory)\n"
                                              "\n"
                                              "Command \"space\": space view/starfield/nebula\n"
                                              "-s SUNS\tSet number of suns\n"
                                              "-p PROB\tSet star probability\n"
                                              "\n"
                                              "Command \"planet\": single planet\n"
                                              "-x NN, -y NN\tSet planet position (percentage)\n"
                                              "-r NN\tSet planet radius (percentage)\n"
                                              "-t NN\tSet planet temperature\n"
                                              "-X NN, -Y NN, -Z NN\tSet (invisible) sun position (percentage)\n"
                                              "\n"
                                              "Command \"orbit\": space view with planet in foreground\n"
                                              "-x NN, -y NN\tSet planet position (percentage)\n"
                                              "-r NN\tSet planet radius (percentage)\n"
                                              "-n NN\tSet number of stars\n"
                                              "\n"
                                              "Command \"explosion\": generic explosion\n"
                                              "-n NN\tSet size (number of hotspots)\n"
                                              "-v NN\tSet speed\n"
                                              "\n"
                                              "Command \"shield\": shield effect\n"
                                              "-n NN\tSet size (number of hotspots)\n"
                                              "-a NN\tAngle (0-7)\n"
                                              "\n"
                                              "Command \"texture\": textures\n"
                                              "fill(COLOR)\tFill with solid color\n"
                                              "noise(RANGE)\tFill with noise\n"
                                              "brush(RANGE[,angle=N,n=N])\tAdd brushed metal effect\n"
                                              "circ(RANGE,X,Y,R[,NOISE])\tAdd circular gradient effect\n"))));
    exit(0);
}

void
gfx::gen::Application::doSpace(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    // Configuration area
    CommonOptions opts;
    SpaceViewConfig config;

    // Parse command line
    afl::sys::StandardCommandLineParser parser(cmdl);
    afl::string::Translator& tx = translator();
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (!option) {
            errorExit(tx("This command does not take positional parameters"));
        }
        if (handleCommonOption(opts, text, parser)) {
            // ok
        } else if (text == "s") {
            int n = 0;
            if (!strToInteger(parser.getRequiredParameter(text), n) || n < 0) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
            config.setNumSuns(n);
        } else if (text == "p") {
            int p = 0;
            if (!strToInteger(parser.getRequiredParameter(text), p) || p < 0 || p >= 100) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
            config.setStarProbability(p);
        } else {
            errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
        }
    }

    // Finalize
    const String_t* pOutputFileName = opts.outputFileName.get();
    if (pOutputFileName == 0) {
        errorExit(tx("output file name (\"-o\") not specified"));
    }
    config.setSize(Point(opts.w, opts.h));

    // Generate
    afl::base::Ref<RGBAPixmap> result(config.render(opts.rng));

    // Save
    saveCanvas(*result->makeCanvas(), *fileSystem().openFile(*pOutputFileName, afl::io::FileSystem::Create));
}

void
gfx::gen::Application::doPlanet(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    // Configuration area
    CommonOptions opts;
    int px = 50, py = 50, pr = 40, pt = 50;
    int sx = -1000, sy = -1000, sz = -1000;

    // Parse command line
    afl::sys::StandardCommandLineParser parser(cmdl);
    afl::string::Translator& tx = translator();
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (!option) {
            errorExit(tx("This command does not take positional parameters"));
        }
        if (handleCommonOption(opts, text, parser)) {
            // ok
        } else if (text == "x") {
            if (!strToInteger(parser.getRequiredParameter(text), px)) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "y") {
            if (!strToInteger(parser.getRequiredParameter(text), py)) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "r") {
            if (!strToInteger(parser.getRequiredParameter(text), pr) || pr <= 0) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "t") {
            if (!strToInteger(parser.getRequiredParameter(text), pt) || pt < 0 || pt > 100) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "X") {
            if (!strToInteger(parser.getRequiredParameter(text), sx)) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "Y") {
            if (!strToInteger(parser.getRequiredParameter(text), sy)) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "Z") {
            if (!strToInteger(parser.getRequiredParameter(text), sz)) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else {
            errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
        }
    }

    // Finalize
    const String_t* pOutputFileName = opts.outputFileName.get();
    if (pOutputFileName == 0) {
        errorExit(tx("output file name (\"-o\") not specified"));
    }

    PlanetConfig config;
    config.setSize(Point(opts.w, opts.h));
    config.setPlanetPosition(px, py);
    config.setPlanetRadius(pr);
    config.setPlanetTemperature(pt);
    config.setSunPosition(sx, sy, sz);

    // Generate
    afl::base::Ref<RGBAPixmap> result(config.render(opts.rng));

    // Save
    saveCanvas(*result->makeCanvas(), *fileSystem().openFile(*pOutputFileName, afl::io::FileSystem::Create));
}

void
gfx::gen::Application::doOrbit(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    // Configuration area
    CommonOptions opts;
    int px = 100, py = 500, pr = 415;
    int n = 5;

    // Parse command line
    afl::sys::StandardCommandLineParser parser(cmdl);
    afl::string::Translator& tx = translator();
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (!option) {
            errorExit(tx("This command does not take positional parameters"));
        }
        if (handleCommonOption(opts, text, parser)) {
            // ok
        } else if (text == "x") {
            if (!strToInteger(parser.getRequiredParameter(text), px)) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "y") {
            if (!strToInteger(parser.getRequiredParameter(text), py)) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "r") {
            if (!strToInteger(parser.getRequiredParameter(text), pr) || pr <= 0) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "n") {
            if (!strToInteger(parser.getRequiredParameter(text), n) || n < 0) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else {
            errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
        }
    }

    // Finalize
    const String_t* pOutputFileName = opts.outputFileName.get();
    if (pOutputFileName == 0) {
        errorExit(tx("output file name (\"-o\") not specified"));
    }

    OrbitConfig config;
    config.setSize(Point(opts.w, opts.h));
    config.setPlanetPosition(px, py);
    config.setPlanetRadius(pr);
    config.setNumStars(n);

    // Generate
    afl::base::Ref<RGBAPixmap> result(config.render(opts.rng));

    // Save
    saveCanvas(*result->makeCanvas(), *fileSystem().openFile(*pOutputFileName, afl::io::FileSystem::Create));
}

void
gfx::gen::Application::doExplosion(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    CommonOptions opts;
    int size = 50;
    int speed = 1;
    
    // Parse command line
    afl::sys::StandardCommandLineParser parser(cmdl);
    afl::string::Translator& tx = translator();
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (!option) {
            errorExit(tx("This command does not take positional parameters"));
        }
        if (handleCommonOption(opts, text, parser)) {
            // ok
        } else if (text == "n") {
            if (!strToInteger(parser.getRequiredParameter(text), size) || size <= 0) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "v") {
            if (!strToInteger(parser.getRequiredParameter(text), speed) || speed <= 0) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else {
            errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
        }
    }

    // Finalize
    const String_t* pOutputFileName = opts.outputFileName.get();
    if (pOutputFileName == 0) {
        errorExit(tx("output file name (\"-o\") not specified"));
    }

    // Generate
    ExplosionRenderer renderer(Point(opts.w, opts.h), size, speed, opts.rng);
    afl::base::Ref<Canvas> result(renderer.renderAll());

    // Save
    saveCanvas(*result, *fileSystem().openFile(*pOutputFileName, afl::io::FileSystem::Create));
}

void
gfx::gen::Application::doShield(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    CommonOptions opts;
    int size = 16;
    int angle = 0;
    
    // Parse command line
    afl::sys::StandardCommandLineParser parser(cmdl);
    afl::string::Translator& tx = translator();
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (!option) {
            errorExit(tx("This command does not take positional parameters"));
        }
        if (handleCommonOption(opts, text, parser)) {
            // ok
        } else if (text == "n") {
            if (!strToInteger(parser.getRequiredParameter(text), size) || size <= 0) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else if (text == "a") {
            if (!strToInteger(parser.getRequiredParameter(text), angle) || angle < 0) {
                errorExit(Format(tx("parameter for \"-%s\" is invalid"), text));
            }
        } else {
            errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
        }
    }

    // Finalize
    const String_t* pOutputFileName = opts.outputFileName.get();
    if (pOutputFileName == 0) {
        errorExit(tx("output file name (\"-o\") not specified"));
    }

    // Generate
    ShieldRenderer renderer(Point(opts.w, opts.h), angle, size, opts.rng);
    afl::base::Ref<Canvas> result(renderer.renderAll());

    // Save
    saveCanvas(*result, *fileSystem().openFile(*pOutputFileName, afl::io::FileSystem::Create));
}

void
gfx::gen::Application::doTexture(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    CommonOptions opts;
    std::vector<String_t> commands;
    
    // Parse command line
    afl::sys::StandardCommandLineParser parser(cmdl);
    afl::string::Translator& tx = translator();
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (!option) {
            commands.push_back(text);
        } else {
            if (handleCommonOption(opts, text, parser)) {
                // ok
            } else {
                errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
            }
        }
    }

    // Finalize
    const String_t* pOutputFileName = opts.outputFileName.get();
    if (pOutputFileName == 0) {
        errorExit(tx("output file name (\"-o\") not specified"));
    }

    // Create pixmap
    afl::base::Ref<RGBAPixmap> pix(RGBAPixmap::create(opts.w, opts.h));
    Texture tex(*pix);
    for (size_t i = 0, n = commands.size(); i < n; ++i) {
        executeTextureCommand(tex, commands[i], opts.rng);
    }

    // Save
    saveCanvas(*pix->makeCanvas(), *fileSystem().openFile(*pOutputFileName, afl::io::FileSystem::Create));
}

bool
gfx::gen::Application::handleCommonOption(CommonOptions& opt, const String_t& text, afl::sys::CommandLineParser& parser)
{
    if (text == "w") {
        if (!strToInteger(parser.getRequiredParameter(text), opt.w) || opt.w <= 0) {
            errorExit(Format(translator()("parameter for \"-%s\" is invalid"), text));
        }
        return true;
    } else if (text == "h") {
        if (!strToInteger(parser.getRequiredParameter(text), opt.h) || opt.h <= 0) {
            errorExit(Format(translator()("parameter for \"-%s\" is invalid"), text));
        }
        return true;
    } else if (text == "S") {
        uint32_t seed = 0;
        if (!strToInteger(parser.getRequiredParameter(text), seed)) {
            errorExit(Format(translator()("parameter for \"-%s\" is invalid"), text));
        }
        opt.rng.setSeed(seed);
        return true;
    } else if (text == "o") {
        opt.outputFileName = parser.getRequiredParameter(text);
        return true;
    } else {
        return false;
    }
}
