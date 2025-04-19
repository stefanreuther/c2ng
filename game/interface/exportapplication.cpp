/**
  *  \file game/interface/exportapplication.cpp
  *  \brief Class game::interface::ExportApplication
  */

#include "game/interface/exportapplication.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/limits.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/specificationloader.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "game/v3/rootloader.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/exporter/configuration.hpp"
#include "interpreter/metacontext.hpp"
#include "util/charsetfactory.hpp"
#include "util/profiledirectory.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::base::Optional;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;
using interpreter::Context;

namespace {
    const char LOG_NAME[] = "export";
}

void
game::interface::ExportApplication::appMain()
{
    util::ProfileDirectory profile(environment(), fileSystem());
    afl::string::Translator& tx = translator();

    // Parse args
    interpreter::exporter::Configuration config;

    Optional<String_t> arg_array;
    Optional<String_t> arg_gamedir;
    Optional<String_t> arg_rootdir;
    Optional<String_t> arg_outfile;
    int arg_race = 0;
    bool opt_fields = false;
    std::auto_ptr<afl::charset::Charset> gameCharset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
    bool hadCharsetOption = false;

    int i;
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    consoleLogger().setConfiguration("*@Warn+=raw:*=drop", translator());
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "C") {
                if (afl::charset::Charset* cs = util::CharsetFactory().createCharset(commandLine.getRequiredParameter(p))) {
                    gameCharset.reset(cs);
                } else {
                    errorExit(tx("the specified character set is not known"));
                }
            } else if (p == "f") {
                String_t pp = commandLine.getRequiredParameter(p);
                try {
                    config.fieldList().addList(pp);
                }
                catch (interpreter::Error& e) {
                    errorExit(Format("'-f %s': %s", pp, e.what()));
                }
            } else if (p == "F") {
                opt_fields = true;
            } else if (p == "S") {
                arg_array = "SHIP";
            } else if (p == "P") {
                arg_array = "PLANET";
            } else if (p == "A") {
                arg_array = commandLine.getRequiredParameter(p);
            } else if (p == "t") {
                config.setFormatByName(commandLine.getRequiredParameter(p), tx);
            } else if (p == "o") {
                arg_outfile = commandLine.getRequiredParameter(p);
            } else if (p == "O") {
                config.setCharsetByName(commandLine.getRequiredParameter(p), tx);
                hadCharsetOption = true;
            } else if (p == "c") {
                Ref<Stream> file = fileSystem().openFile(commandLine.getRequiredParameter(p), FileSystem::OpenRead);
                interpreter::exporter::Configuration tmpConfig(config);
                tmpConfig.load(*file, tx);
                config = tmpConfig;
            } else if (p == "v" || p == "verbose") {
                consoleLogger().setConfiguration("*=raw", tx);
            } else if (p == "h" || p == "help") {
                help();
            } else {
                errorExit(Format(tx("invalid option specified. Use '%s -h' for help."), environment().getInvocationName()));
            }
        } else {
            if (arg_race == 0 && afl::string::strToInteger(p, i) && i > 0 && i <= MAX_PLAYERS) {
                arg_race = i;
            } else if (!arg_gamedir.isValid()) {
                arg_gamedir = p;
            } else if (!arg_rootdir.isValid()) {
                arg_rootdir = p;
            } else {
                errorExit(tx("too many arguments"));
            }
        }
    }

    // Validate args
    const String_t*const parg_array = arg_array.get();
    if (parg_array == 0) {
        errorExit(tx("please specify the object type to export ('-P', '-S', '-A'). Use '-h' for help."));
    }

    // Default field set
    if (config.fieldList().size() == 0) {
        if (opt_fields) {
            config.fieldList().addList("NAME@-30,TYPE@-10");
        } else {
            config.fieldList().addList("ID@5,NAME@-30");
        }
    }

    // Set up game directories
    FileSystem& fs = fileSystem();
    const String_t defaultRoot = fs.makePathName(fs.makePathName(environment().getInstallationDirectoryName(), "share"), "specs");
    game::v3::RootLoader loader(fs.openDirectory(arg_rootdir.orElse(defaultRoot)), &profile, 0 /* pCallback */, translator(), log(), fs);

    // Check game data
    // Keep using default config
    const String_t usedGameDir = fs.getAbsolutePathName(arg_gamedir.orElse("."));
    const game::config::UserConfiguration uc;
    const afl::base::Ptr<Root> root = loader.load(fs.openDirectory(usedGameDir), *gameCharset, uc, false);
    if (root.get() == 0 || root->getTurnLoader().get() == 0) {
        errorExit(Format(tx("no game data found in directory \"%s\""), usedGameDir));
    }

    // Check player number
    if (arg_race != 0) {
        String_t extra;
        if (!root->getTurnLoader()->getPlayerStatus(arg_race, extra, translator()).contains(TurnLoader::Available)) {
            errorExit(Format(tx("no game data available for player %d"), arg_race));
        }
    } else {
        arg_race = root->getTurnLoader()->getDefaultPlayer(root->playerList().getAllPlayers());
        if (arg_race == 0) {
            errorExit(tx("please specify the player number"));
        }
    }

    // Make a session and load it
    bool ok = false;
    Session session(translator(), fs);
    session.setGame(new Game());
    session.setRoot(root);
    session.setShipList(new game::spec::ShipList());
    root->specificationLoader().loadShipList(*session.getShipList(), *root, game::makeResultTask(ok))->call();
    if (!ok) {
        throw Exception(tx("unable to load ship list"));
    }

    ok = false;
    root->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), arg_race, *root, session, makeResultTask(ok))->call();
    if (!ok) {
        throw Exception(tx("unable to load turn"));
    }

    session.postprocessTurn(session.getGame()->currentTurn(), game::PlayerSet_t(arg_race), game::PlayerSet_t(arg_race), game::map::Object::ReadOnly);

    // What do we want to export?
    std::auto_ptr<Context> array(findArray(*parg_array, session.world()));
    if (opt_fields) {
        array.reset(interpreter::MetaContext::create(*array));
        if (array.get() == 0) {
            errorExit(Format(tx("object of type '%s' has no fields"), *parg_array));
        }
    }

    // Do it.
    if (const String_t*const outfile = arg_outfile.get()) {
        // Output to file
        Ref<Stream> s = fs.openFile(*outfile, FileSystem::Create);
        config.exportFile(*array, *s);
    } else {
        // Output to console. The console performs character set conversion.
        if (hadCharsetOption) {
            log().write(afl::sys::LogListener::Warn, LOG_NAME, tx("WARNING: Option '-O' has been ignored because standard output is being used."));
        }
        if (!config.exportText(*array, standardOutput())) {
            errorExit(tx("the selected format needs an output file name ('-o')"));
        }
    }
}

void
game::interface::ExportApplication::help()
{
    afl::io::TextWriter& out = standardOutput();
    afl::string::Translator& tx = translator();
    out.writeLine(Format(tx("PCC2 Export v%s - (c) 2017-2025 Stefan Reuther"), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-h]\n"
                            "  %$0s [-opts] [-f F@W...] [-S|-P|-A OBJECT] [-t TYPE] DIR [ROOT] PLAYER\n\n"
                            "%s"
                            "\n"
                            "Report bugs to <Streu@gmx.de>"),
                         environment().getInvocationName(),
                         util::formatOptions(tx("Options:\n"
                                                "-C CHARSET\tSet game character set\n"
                                                "-f FIELD@WIDTH\tAdd field to report\n"
                                                "-S\tExport ships (same as '-A SHIP')\n"
                                                "-P\tExport planets (same as '-A PLANET')\n"
                                                "-A OBJECT\tExport specified object type (CCScript array name)\n"
                                                "-t TYPE\tSet output file format/type\n"
                                                "-o FILE\tSet output file name (default: stdout)\n"
                                                "-O CHARSET\tSet output file character set (default: UTF-8)\n"
                                                "-F\tExport list of fields instead of game data\n"
                                                "-c FILE\tRead configuration from file\n"
                                                "-v\tShow log messages (verbose mode)\n"
                                                "\n"
                                                "Types:\n"
                                                "dbf\tdBASE file (needs '-o')\n"
                                                "text\tsimple text table, default\n"
                                                "table\tboxy text table\n"
                                                "csv, tsv, ssv\tcomma/tab/semicolon-separated values\n"
                                                "json\tJSON (JavaScript)\n"
                                                "html\tHTML\n"))));
    out.flush();
    exit(0);
}

interpreter::Context*
game::interface::ExportApplication::findArray(const String_t& name, interpreter::World& world)
{
    // Look up name
    afl::data::NameMap::Index_t i = world.globalPropertyNames().getIndexByName(afl::string::strUCase(name));
    if (i == afl::data::NameMap::nil) {
        errorExit(Format(translator()("unknown object type '%s'"), name));
    }

    // Check for array
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(world.globalValues()[i]);
    if (!cv) {
        errorExit(Format(translator()("unknown object type '%s'"), name));
    }

    // Check for content
    try {
        Context* ctx = cv->makeFirstContext();
        if (!ctx) {
            errorExit(Format(translator()("this game does not contain any objects of type '%s'"), name));
        }
        return ctx;
    }
    catch (interpreter::Error& /*e*/) {
        // This happens when they do something like '-A CADD',
        // because CAdd refuses makeFirstContext() with
        // Error::typeError. No need to display the very
        // error message; it's not a known object type, period.
        errorExit(Format(translator()("unknown object type '%s'"), name));
        return 0;
    }
}
