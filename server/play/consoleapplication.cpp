/**
  *  \file server/play/consoleapplication.cpp
  */

#include "server/play/consoleapplication.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/net/line/linesink.hpp"
#include "afl/net/url.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/game.hpp"
#include "game/limits.hpp"
#include "game/session.hpp"
#include "game/specificationloader.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "game/v3/rootloader.hpp"
#include "server/interface/gameaccess.hpp"
#include "server/interface/gameaccessserver.hpp"
#include "server/play/fs/directory.hpp"
#include "server/play/fs/session.hpp"
#include "server/play/gameaccess.hpp"
#include "server/play/mainpacker.hpp"
#include "server/ports.hpp"
#include "util/charsetfactory.hpp"
#include "util/messagecollector.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::string::Format;

struct server::play::ConsoleApplication::Parameters {
    afl::base::Optional<String_t> arg_gamedir;  // -G
    afl::base::Optional<String_t> arg_rootdir;  // -R
    std::auto_ptr<afl::charset::Charset> gameCharset;
    int playerNumber;

    Parameters()
        : arg_gamedir(),
          arg_rootdir(),
          gameCharset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1)),
          playerNumber(0)
        { }
};


server::play::ConsoleApplication::ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
    : Application(env, fs),
      m_network(net)
{ }

void
server::play::ConsoleApplication::appMain()
{
    afl::string::Translator& tx = translator();

    // Parameters
    Parameters params;

    // Parser
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "h" || p == "help") {
                help();
            } else if (p == "C") {
                // character set
                if (afl::charset::Charset* cs = util::CharsetFactory().createCharset(commandLine.getRequiredParameter(p))) {
                    params.gameCharset.reset(cs);
                } else {
                    errorExit(tx("the specified character set is not known"));
                }
            } else if (p == "R" || p == "W") {
                // session conflict management; skip those
                commandLine.getRequiredParameter(p);
            } else if (p == "D") {
                // property
                String_t key = commandLine.getRequiredParameter(p);
                String_t value;
                String_t::size_type eq = key.find('=');
                if (eq != String_t::npos) {
                    value.assign(key, eq+1, String_t::npos);
                    key.erase(eq);
                }
                m_properties[key] = value;
            } else {
                errorExit(Format(tx("invalid option '%s' specified. Use '%s -h' for help."), p, environment().getInvocationName()));
            }
        } else {
            int n;
            if (afl::string::strToInteger(p, n) && n > 0 && n <= game::MAX_PLAYERS) {
                if (params.playerNumber != 0) {
                    errorExit(tx("only one player number allowed"));
                }
                params.playerNumber = n;
            } else if (!params.arg_gamedir.isValid()) {
                params.arg_gamedir = p;
            } else if (!params.arg_rootdir.isValid()) {
                params.arg_rootdir = p;
            } else {
                errorExit(tx("too many arguments"));
            }
        }
    }

    if (params.playerNumber == 0) {
        errorExit(tx("missing player number"));
    }

    String_t gameDir;
    if (!params.arg_gamedir.get(gameDir)) {
        errorExit(tx("missing directory name"));
    }

    // Central logger
    util::MessageCollector logCollector;

    // Make a session
    game::Session session(tx, fileSystem());
    session.log().addListener(logCollector);

    // Check game data
    afl::base::Ptr<game::Root> root = loadRoot(gameDir, params, session.log());
    if (root.get() == 0 || root->getTurnLoader().get() == 0) {
        errorExit(tx("no game data found"));
    }

    String_t extra;
    if (!root->getTurnLoader()->getPlayerStatus(params.playerNumber, extra, tx).contains(game::TurnLoader::Available)) {
        errorExit(Format(tx.translateString("no game data available for player %d").c_str(), params.playerNumber));
    }

    // Make a session and load it
    bool ok = false;
    session.setGame(new game::Game());
    session.setRoot(root);
    session.setShipList(new game::spec::ShipList());
    root->specificationLoader().loadShipList(*session.getShipList(), *root, game::makeResultTask(ok))->call();
    if (!ok) {
        errorExit(tx("unable to load ship list"));
    }

    ok = false;
    root->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), params.playerNumber, *root, session, game::makeResultTask(ok))->call();
    if (!ok) {
        errorExit(tx("unable to load turn"));
    }

    session.getGame()->setViewpointPlayer(params.playerNumber);
    // FIXME? sync teams from alliances
    session.setEditableAreas(game::Session::AreaSet_t(session.LocalDataArea) + session.CommandArea);
    session.postprocessTurn(session.getGame()->currentTurn(), game::PlayerSet_t(params.playerNumber), game::PlayerSet_t(params.playerNumber), game::map::Object::Playable);

    // Store properties in session
    getSessionProperties(session) = m_properties;

    // Interact
    class Sink : public afl::net::line::LineSink {
     public:
        Sink(afl::io::TextWriter& out)
            : m_out(out)
            { }
        virtual void handleLine(const String_t& line)
            { m_out.writeLine(line); }
     private:
        afl::io::TextWriter& m_out;
    };

    afl::base::Ref<afl::io::TextReader> reader = environment().attachTextReader(afl::sys::Environment::Input);

    Sink sink(standardOutput());
    GameAccess impl(session, logCollector);
    server::interface::GameAccessServer server(impl);
    bool stop = server.handleOpening(sink);
    while (!stop) {
        standardOutput().flush();
        String_t line;
        if (reader->readLine(line)) {
            stop = server.handleLine(line, sink);
        } else {
            server.handleConnectionClose();
            stop = true;
        }
    }

    impl.save();
}

void
server::play::ConsoleApplication::help()
{
    afl::string::Translator& tx = translator();
    const String_t options =
        util::formatOptions(tx("Options:\n"
                               "-Ccs\tSet game character set\n"
                               "-Rkey, -Wkey\tIgnored; used for session conflict resolution\n"
                               "-Dkey=value\tDefine a property\n"));

    afl::io::TextWriter& out = standardOutput();
    out.writeLine(Format(tx("PCC2 Play Server v%s - (c) 2019-2023 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-h]\n"
                            "  %$0s [-OPTIONS] PLAYER GAMEDIR [ROOTDIR]\n"
                            "\n"
                            "GAMEDIR can be a local directory, or c2file://USER@HOST:PORT/DIR.\n\n"
                            "%s"
                            "\n"
                            "Report bugs to <Streu@gmx.de>").c_str(),
                         environment().getInvocationName(),
                         options));
    exit(0);
}

afl::base::Ptr<game::Root>
server::play::ConsoleApplication::loadRoot(const String_t& gameDir, const Parameters& params, afl::sys::LogListener& log)
{
    afl::io::FileSystem& fs = fileSystem();
    afl::string::Translator& tx = translator();
    String_t defaultRoot = fs.makePathName(fs.makePathName(environment().getInstallationDirectoryName(), "share"), "specs");
    afl::base::Ref<afl::io::Directory> rootDir = fs.openDirectory(params.arg_rootdir.orElse(defaultRoot));

    // Try to parse as URL
    afl::net::Url url;
    if (url.parse(gameDir)) {
        if (url.getScheme() == "c2file") {
            afl::base::Ref<server::play::fs::Session> session(server::play::fs::Session::create(m_network, url.getName(afl::string::Format("%d", FILE_PORT)), url.getUser()));
            return session->createRoot(url.getPath(), tx, log, m_nullFileSystem, rootDir, *params.gameCharset);
        }
    }

    // Default: local play
    // The FileSystem instance is used for accessing backups according to path names generated from configuration.
    // Although, as far as I can tell, these configuration items (Backup.Turn etc.) cannot be accessed in a c2play-server instance,
    // we block this possible hole by passing a NullFileSystem.
    game::v3::RootLoader loader(rootDir, 0 /* profile */, 0 /* callback */, tx, log, m_nullFileSystem);

    // Check game data
    // FIXME: load correct config!
    const game::config::UserConfiguration uc;
    return loader.load(fs.openDirectory(gameDir), *params.gameCharset, uc, false);
}
