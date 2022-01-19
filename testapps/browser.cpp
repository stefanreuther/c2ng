/**
  *  \file test_apps/browser.cpp
  */

#include <iostream>
#include "game/browser/browser.hpp"
#include "afl/string/nulltranslator.hpp"
#include "util/consolelogger.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/browser/directoryhandler.hpp"
#include "game/browser/accountmanager.hpp"
#include "afl/sys/environment.hpp"
#include "util/profiledirectory.hpp"
#include "game/browser/usercallback.hpp"
#include "afl/data/stringvalue.hpp"
#include "game/turnloader.hpp"
#include "game/registrationkey.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/pcc/browserhandler.hpp"
#include "afl/net/http/client.hpp"
#include "afl/sys/thread.hpp"
#include "afl/net/http/manager.hpp"
#include "afl/net/http/defaultconnectionprovider.hpp"

using game::browser::Task_t;
using game::browser::LoadGameRootTask_t;

namespace {
    class MyUserCallback : public game::browser::UserCallback {
     public:
        virtual bool askInput(String_t title, const std::vector<Element>& question, afl::data::Segment& values)
            {
                std::string n;
                std::cout << "-- Input request: " << title << " --\n";
             again:
                size_t index = 0;
                for (size_t i = 0; i < question.size(); ++i) {
                    switch (question[i].type) {
                     case AskString:
                     case AskPassword:
                        std::cout << question[i].prompt << "? " << std::flush;
                        if (getline(std::cin, n)) {
                            values.setNew(index++, new afl::data::StringValue(n));
                        }
                        break;
                     case ShowInfo:
                        std::cout << question[i].prompt << "\n";
                        break;
                    }
                }
                while (1) {
                    std::cout << "Accept (y=yes, n=no and start again, c=cancel) " << std::flush;
                    if (!getline(std::cin, n) || n == "c") {
                        return false;
                    } else if (n == "y") {
                        return true;
                    } else if (n == "n") {
                        goto again;
                    } else {
                        // again
                    }
                }
            }
    };
}

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::string::NullTranslator tx;
    util::ConsoleLogger log;
    MyUserCallback userCB;
    log.attachWriter(false, env.attachTextWriter(env.Output).asPtr());
    log.attachWriter(true,  env.attachTextWriter(env.Error).asPtr());

    afl::net::http::Client client;
    afl::sys::Thread clientThread("http", client);
    client.setNewConnectionProvider(new afl::net::http::DefaultConnectionProvider(client, afl::net::NetworkStack::getInstance()));
    clientThread.start();
    afl::net::http::Manager httpManager(client);

    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    util::ProfileDirectory profile(env, fs, tx, log);
    game::browser::AccountManager acc(profile, tx, log);
    acc.load();
    game::browser::Browser b(fs, tx, log, acc, profile, userCB);
    afl::base::Ref<afl::io::Directory> defaultSpecDirectory = fs.openDirectory(fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs"));
    b.addNewHandler(new game::browser::DirectoryHandler(b, defaultSpecDirectory, profile));
    b.addNewHandler(new game::pcc::BrowserHandler(b, httpManager, defaultSpecDirectory, profile));
    b.addNewHandler(new game::nu::BrowserHandler(b, httpManager, defaultSpecDirectory));
    String_t cmd;
    size_t n;
    while (std::cout << b.currentFolder().getName() << "> ", getline(std::cin, cmd)) {
        if (cmd == "") {
            // ok
        } else if (cmd == "pwd") {
            const afl::container::PtrVector<game::browser::Folder>& path = b.path();
            for (size_t i = 0, n = path.size(); i < n; ++i) {
                std::cout << afl::string::Format("%3d. %s", i, path[i]->getName()) << "\n";
            }
        } else if (cmd == "ls") {
            b.loadContent(std::auto_ptr<Task_t>(Task_t::makeNull()))->call();
            const afl::container::PtrVector<game::browser::Folder>& content = b.content();
            for (size_t i = 0, n = content.size(); i < n; ++i) {
                std::cout << afl::string::Format("%3d. %s", i, content[i]->getName()) << "\n";
            }
        } else if (cmd.substr(0, 5) == "open ") {
            b.openFolder(cmd.substr(5));
        } else if (cmd.substr(0, 3) == "cd " && afl::string::strToInteger(cmd.substr(3), n)) {
            b.openChild(n);
        } else if (cmd == "up") {
            b.openParent();
        } else if (cmd == "info") {
            game::config::UserConfiguration config;
            b.currentFolder().loadConfiguration(config);
            afl::base::Ptr<game::Root> root;

            class Task : public LoadGameRootTask_t {
             public:
                Task(afl::base::Ptr<game::Root>& root)
                    : m_root(root)
                    { }
                void call(afl::base::Ptr<game::Root> root)
                    { m_root = root; }
             private:
                afl::base::Ptr<game::Root>& m_root;
            };
            b.currentFolder().loadGameRoot(config, std::auto_ptr<LoadGameRootTask_t>(new Task(root)))->call();
            if (root.get() != 0) {
                if (root->getTurnLoader().get() != 0) {
                    std::cout << "Turn loader present.\n";
                    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
                        if (game::Player* pl = root->playerList().get(i)) {
                            String_t extra;
                            game::TurnLoader::PlayerStatusSet_t st = root->getTurnLoader()->getPlayerStatus(i, extra, tx);
                            if (!st.empty() || !extra.empty()) {
                                std::cout << "Player " << i << ", "
                                          << pl->getName(pl->ShortName);
                                if (st.contains(game::TurnLoader::Available)) {
                                    std::cout << ", available";
                                }
                                if (st.contains(game::TurnLoader::Playable)) {
                                    std::cout << ", playable";
                                }
                                if (st.contains(game::TurnLoader::Primary)) {
                                    std::cout << ", primary";
                                }
                                if (!extra.empty()) {
                                    std::cout << ", " << extra;
                                }
                                std::cout << "\n";
                            }
                        }
                    }
                } else {
                    std::cout << "No turn loader.\n";
                }
                switch (root->registrationKey().getStatus()) {
                 case game::RegistrationKey::Unknown:      std::cout << "Unknown registration key.\n"; break;
                 case game::RegistrationKey::Unregistered: std::cout << "Unregistered.\n"; break;
                 case game::RegistrationKey::Registered:   std::cout << "Registered: " << root->registrationKey().getLine(game::RegistrationKey::Line1) << ".\n"; break;
                }
                std::cout << "Host version: " << root->hostVersion().toString(tx) << "\n";
            } else {
                std::cout << "No game.\n";
            }
        } else {
            // huh?
            std::cout << "Invalid command.\n";
        }
    }
}
