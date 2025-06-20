/**
  *  \file game/browser/testapplet.cpp
  *  \brief Class game::browser::TestApplet
  */

#include "game/browser/testapplet.hpp"

#include "afl/io/textreader.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/net/http/client.hpp"
#include "afl/net/http/defaultconnectionprovider.hpp"
#include "afl/net/http/manager.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/thread.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/directoryhandler.hpp"
#include "game/browser/usercallback.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/limits.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/pcc/browserhandler.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"
#include "game/turnloader.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::container::PtrVector;
using afl::io::Directory;
using afl::io::FileSystem;
using afl::io::TextReader;
using afl::io::TextWriter;
using afl::net::http::Client;
using afl::net::http::Manager;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::Environment;
using afl::sys::LogListener;
using afl::sys::Thread;
using game::config::UserConfiguration;
using util::ProfileDirectory;

class game::browser::TestApplet::MyUserCallback : public UserCallback {
 public:
    MyUserCallback(TextReader& in, TextWriter& out)
        : m_in(in), m_out(out)
        { }
    virtual void askPassword(const PasswordRequest& req)
        {
            m_out.writeLine("-- Password request: " + req.accountName);
            m_out.writeText("Password? ");
            m_out.flush();

            PasswordResponse resp;
            String_t str;
            if (m_in.readLine(str)) {
                resp.canceled = false;
                resp.password = str;
            } else {
                resp.canceled = true;
            }
            sig_passwordResult.raise(resp);
        }

 private:
    TextReader& m_in;
    TextWriter& m_out;
};

game::browser::TestApplet::TestApplet(afl::net::NetworkStack& net)
    : m_networkStack(net)
{ }

int
game::browser::TestApplet::run(util::Application& app, afl::sys::Environment::CommandLine_t& /*cmdl*/)
{
    Environment& env = app.environment();
    Translator& tx = app.translator();
    LogListener& log = app.log();

    Ref<TextReader> in = env.attachTextReader(Environment::Input);
    TextWriter& out = app.standardOutput();

    MyUserCallback userCB(*in, out);

    Client client;
    Thread clientThread("http", client);
    client.setNewConnectionProvider(new afl::net::http::DefaultConnectionProvider(client, m_networkStack));
    clientThread.start();
    Manager httpManager(client);

    FileSystem& fs = app.fileSystem();
    ProfileDirectory profile(env, fs);
    AccountManager acc(profile, tx, log);
    acc.load();
    Browser b(fs, tx, log, acc, profile, userCB);
    Ref<Directory> defaultSpecDirectory = fs.openDirectory(fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs"));
    b.addNewHandler(new DirectoryHandler(b, defaultSpecDirectory, profile));
    b.addNewHandler(new game::pcc::BrowserHandler(b, httpManager, defaultSpecDirectory, profile));
    b.addNewHandler(new game::nu::BrowserHandler(b, httpManager, defaultSpecDirectory));
    String_t cmd;

    while (out.writeText(b.currentFolder().getName() + "> "), out.flush(), in->readLine(cmd)) {
        size_t n;
        if (cmd == "") {
            // ok
        } else if (cmd == "pwd") {
            const PtrVector<Folder>& path = b.path();
            for (size_t i = 0, n = path.size(); i < n; ++i) {
                out.writeLine(Format("%3d. %s", i, path[i]->getName()));
            }
        } else if (cmd == "ls") {
            b.loadContent(std::auto_ptr<Task_t>(Task_t::makeNull()))->call();
            const PtrVector<Folder>& content = b.content();
            for (size_t i = 0, n = content.size(); i < n; ++i) {
                out.writeLine(Format("%3d. %s", i, content[i]->getName()));
            }
        } else if (cmd.substr(0, 5) == "open ") {
            b.openFolder(cmd.substr(5));
        } else if (cmd.substr(0, 3) == "cd " && afl::string::strToInteger(cmd.substr(3), n)) {
            b.openChild(n);
        } else if (cmd == "up") {
            b.openParent();
        } else if (cmd == "info") {
            UserConfiguration config;
            b.currentFolder().loadConfiguration(config);
            Ptr<Root> root;

            class Task : public LoadGameRootTask_t {
             public:
                Task(Ptr<Root>& root)
                    : m_root(root)
                    { }
                void call(Ptr<Root> root)
                    { m_root = root; }
             private:
                Ptr<Root>& m_root;
            };
            b.currentFolder().loadGameRoot(config, std::auto_ptr<LoadGameRootTask_t>(new Task(root)))->call();
            if (root.get() != 0) {
                if (root->getTurnLoader().get() != 0) {
                    out.writeLine("Turn loader present.");
                    for (int i = 1; i <= MAX_PLAYERS; ++i) {
                        if (const Player* pl = root->playerList().get(i)) {
                            String_t extra;
                            TurnLoader::PlayerStatusSet_t st = root->getTurnLoader()->getPlayerStatus(i, extra, tx);
                            if (!st.empty() || !extra.empty()) {
                                out.writeText(Format("Player %d, %s", i, pl->getName(Player::ShortName, tx)));
                                if (st.contains(TurnLoader::Available)) {
                                    out.writeText(", available");
                                }
                                if (st.contains(TurnLoader::Playable)) {
                                    out.writeText(", playable");
                                }
                                if (st.contains(TurnLoader::Primary)) {
                                    out.writeText(", primary");
                                }
                                if (!extra.empty()) {
                                    out.writeText(", " + extra);
                                }
                                out.writeLine();
                            }
                        }
                    }
                } else {
                    out.writeLine("No turn loader.");
                }
                switch (root->registrationKey().getStatus()) {
                 case RegistrationKey::Unknown:      out.writeLine("Unknown registration key."); break;
                 case RegistrationKey::Unregistered: out.writeLine("Unregistered."); break;
                 case RegistrationKey::Registered:   out.writeLine(Format("Registered: %s.", root->registrationKey().getLine(RegistrationKey::Line1))); break;
                }
                out.writeLine(Format("Host version: %s", root->hostVersion().toString()));
            } else {
                out.writeLine("No game.");
            }
        } else {
            // huh?
            out.writeLine("Invalid command.");
        }
    }
    return 0;
}
