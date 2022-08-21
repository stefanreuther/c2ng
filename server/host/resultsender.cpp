/**
  *  \file server/host/resultsender.cpp
  *  \brief Class server::host::ResultSender
  */

#include "server/host/resultsender.hpp"
#include "afl/base/countof.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/parsedtime.hpp"
#include "server/host/game.hpp"
#include "server/host/installer.hpp"
#include "server/host/root.hpp"
#include "server/host/schedule.hpp"
#include "server/host/user.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/hostschedule.hpp"
#include "server/interface/mailqueue.hpp"

using afl::string::Format;
using server::host::Game;
using server::interface::BaseClient;
using server::interface::FileBase;
using server::interface::FileBaseClient;
using server::interface::HostGame;

namespace {
    typedef std::map<String_t, game::PlayerSet_t> PlayerMap_t;

    void addPlayers(PlayerMap_t& out, const afl::data::StringList_t& in, int slot)
    {
        for (size_t i = 0; i < in.size(); ++i) {
            out[in[i]] += slot;
        }
    }

    void installResults(Game& g, server::host::Root& root, const PlayerMap_t& allPlayers)
    {
        for (PlayerMap_t::const_iterator i = allPlayers.begin(); i != allPlayers.end(); ++i) {
            const String_t userGameDir = g.getPlayerConfig(i->first, "gameDir");
            if (!userGameDir.empty()) {
                server::host::Installer(root).installGameData(g, i->second, i->first, userGameDir);
            }
        }
    }


    // FIXME: Merge with server::talk::LinkFormatter somehow?
    String_t makeGameUrl(int32_t gameId, String_t gameName)
    {
        String_t raw = Format("%d-%s", gameId, gameName);
        String_t result;
        bool needDash = false;
        for (String_t::size_type i = 0; i < raw.size(); ++i) {
            char c = raw[i];
            if (afl::string::charIsAlphanumeric(c)) {
                if (needDash) {
                    result += '-';
                }
                result += c;
                needDash = false;
            } else {
                needDash = true;
            }
        }
        return result;
    }


    // FIXME: can we recycle this from somewhere else?
    uint32_t getNextHostDate(Game& game, bool& hostEarly, server::host::Schedule::Type_t& type)
    {
        // Figure out last host date
        int32_t lastHost = game.lastHostTime().get();
        int32_t turn     = game.turnNumber().get();

        // Process all schedules
        afl::net::redis::Subtree sroot(game.getSchedule());
        afl::net::redis::StringListKey schedules(sroot.stringListKey("list"));
        int32_t numSchedules = schedules.size();
        int32_t currentSchedule = 0;
        while (currentSchedule < numSchedules) {
            // Process one schedule
            server::host::Schedule sched;
            sched.loadFrom(sroot.hashKey(schedules[currentSchedule]));
            if (!sched.isExpired(turn, lastHost)) {
                hostEarly = sched.getHostEarly();
                type = sched.getType();
                return sched.getNextHost(lastHost);
            }
            ++currentSchedule;
        }
        hostEarly = false;
        type = server::interface::HostSchedule::Stopped;
        return 0;
    }

    void describeNextHostDate(server::host::Root& root,
                              server::interface::MailQueue& mail_connection,
                              uint32_t nextHost,
                              bool nextHostEarly,
                              server::host::Schedule::Type_t nextHostType)
    {
        String_t suffix = nextHostEarly ? "_early" : "";
        if (nextHost != 0) {
            afl::sys::ParsedTime pt;
            root.getSystemTimeFromTime(nextHost).unpack(pt, afl::sys::Time::UniversalTime);
            mail_connection.addParameter("next_schedule", "day" + suffix);
            mail_connection.addParameter("next_day_time", pt.format("%d/%b/%Y %H:%M GMT"));
        } else {
            if (nextHostType == server::interface::HostSchedule::Manual) {
                mail_connection.addParameter("next_schedule", "manual" + suffix);
            } else if (nextHostType == server::interface::HostSchedule::Stopped) {
                mail_connection.addParameter("next_schedule", "stop" + suffix);
            } else {
                mail_connection.addParameter("next_schedule", "quick" + suffix);
            }
        }
    }

    enum {
        Result,
        ResultPlayerFiles,
        Zip,
        ZipPlayerFiles,
        Info,
        NumFormats
    };

    struct ResultMailInfo {
        int32_t gameId;
        String_t gameName;
        String_t gameUrl;
        String_t gameDir;
        int32_t gameTurn;
        bool nextHostEarly;
        bool finalTurn;
        bool endChanged;
        bool configChanged;
        bool scheduleChanged;
        server::host::Schedule::Type_t nextHostType;
        uint32_t nextHostTime;

        void init(Game& game);
        void describeGame(server::host::Root& root, server::interface::MailQueue& mailer) const;
        void sendResults(server::host::Root& root, server::interface::MailQueue& mailer, Game& game, int slot, afl::data::StringList_t (&playersByFormat)[NumFormats]);
    };

    void ResultMailInfo::init(Game& game)
    {
        gameId          = game.getId();
        gameName        = game.getName();
        gameUrl         = makeGameUrl(gameId, gameName);
        gameDir         = game.getDirectory();
        gameTurn        = game.turnNumber().get();
        nextHostTime    = getNextHostDate(game, nextHostEarly, nextHostType);
        finalTurn       = game.getState() == HostGame::Finished;
        endChanged      = game.endChanged().get();
        configChanged   = game.configChanged().get();
        scheduleChanged = game.scheduleChanged().get();
    }

    void ResultMailInfo::describeGame(server::host::Root& root, server::interface::MailQueue& mailer) const
    {
        mailer.addParameter("gameid", Format("%d", gameId));
        mailer.addParameter("gamename", gameName);
        mailer.addParameter("gameurl", gameUrl);
        mailer.addParameter("gameturn", Format("%d", gameTurn));
        mailer.addParameter("endChanged", Format("%d", int(endChanged)));
        mailer.addParameter("configChanged", Format("%d", int(configChanged)));
        mailer.addParameter("scheduleChanged", Format("%d", int(scheduleChanged)));
        describeNextHostDate(root, mailer, nextHostTime, nextHostEarly, nextHostType);
    }

    void ResultMailInfo::sendResults(server::host::Root& root, server::interface::MailQueue& mailer, Game& game, int slot, afl::data::StringList_t (&playersByFormat)[NumFormats])
    {
        static const char*const suffixes[] = {
            "-rst",
            "-rst-pf",
            "",
            "-pf",
            "-info",
        };

        for (int fmt = 0; fmt < NumFormats; ++fmt) {
            String_t id = Format("result-%d-%d%s", gameId, slot, suffixes[fmt]);
            if (playersByFormat[fmt].empty()) {
                // Nobody wants this format, just cancel the previous mail.
                mailer.cancelMessage(id);
            } else {
                // Someone wants this format, so generate the message
                String_t tpl;
                if (finalTurn) {
                    tpl += "last-";
                }
                if (fmt == Info) {
                    tpl += "result-info";
                } else {
                    tpl += "result";
                }
                mailer.startMessage(tpl, id);
                mailer.addParameter("slot", Format("%d", slot));
                describeGame(root, mailer);

                if (finalTurn) {
                    // A slot is marked dead only when the last player resigns.
                    // That is, if we actually have a player here, the slot is alive,
                    // and the slot will have a nonzero rank assigned.
                    mailer.addParameter("rank", Format("%d", game.getSlot(slot).rank().get()));
                }

                if (fmt == Zip || fmt == ZipPlayerFiles) {
                    mailer.addAttachment(Format("c2file://%s:%s/%s/out/%d/player%d.zip")
                                         << root.config().hostFileAddress.getName()
                                         << root.config().hostFileAddress.getService()
                                         << gameDir << slot << slot);
                }
                if (fmt == Result || fmt == ResultPlayerFiles) {
                    // Send everything but the ZIP file.
                    // Compare actions.cpp:importFileHistory which intersects out/<pl> with backups/pre-<turn> to effectively suppress the .zip.
                    try {
                        const String_t pathName = Format("%s/out/%d", gameDir, slot);
                        FileBase::ContentInfoMap_t files;
                        BaseClient(root.hostFile()).setUserContext(String_t());
                        FileBaseClient(root.hostFile()).getDirectoryContent(pathName, files);
                        for (FileBase::ContentInfoMap_t::iterator it = files.begin(); it != files.end(); ++it) {
                            if (const FileBase::Info* p = it->second) {
                                if (p->type == FileBase::IsFile
                                    && (it->first.size() <= 4 || it->first.compare(it->first.size()-4, 4, ".zip", 4) != 0))
                                {
                                    mailer.addAttachment(Format("c2file://%s:%s/%s/%s")
                                                         << root.config().hostFileAddress.getName()
                                                         << root.config().hostFileAddress.getService()
                                                         << pathName
                                                         << it->first);
                                }
                            }
                        }
                    }
                    catch (std::exception&) {
                        // Ignore errors accessing the file server
                    }
                }
                if (fmt == ZipPlayerFiles || fmt == ResultPlayerFiles) {
                    mailer.addAttachment(Format("c2file://%s:%s/%s/out/all/playerfiles.zip")
                                         << root.config().hostFileAddress.getName()
                                         << root.config().hostFileAddress.getService()
                                         << gameDir);
                }

                // Send it
                mailer.send(playersByFormat[fmt]);
            }
        }
    }

    void collectPlayers(server::host::Root& root, Game& game, afl::data::StringList_t& players, afl::data::StringList_t (&playersByFormat)[NumFormats])
    {
        // Get mail formats.
        for (size_t i = 0; i < players.size(); ++i) {
            String_t fmt = game.getPlayerConfig(players[i], "mailgametype");
            bool hasPF = game.getPlayerConfigInt(players[i], "hasPlayerFiles");
            if (!hasPF) {
                game.setPlayerConfigInt(players[i], "hasPlayerFiles", true);
            }
            if (fmt.empty() || fmt == "default") {
                fmt = server::host::User(root, players[i]).getProfileString("mailgametype");
            }
            if (fmt == "none") {
                // User does not want mail
            } else if (fmt == "rst") {
                playersByFormat[Result + !hasPF].push_back("user:" + players[i]);
            } else if (fmt == "info") {
                playersByFormat[Info].push_back("user:" + players[i]);
            } else {
                playersByFormat[Zip + !hasPF].push_back("user:" + players[i]);
            }
        }
    }
}


server::host::ResultSender::ResultSender(Root& root, Game& game)
    : m_root(root),
      m_game(game)
{ }

void
server::host::ResultSender::sendAllResults()
{
    // ex planetscentral/host/exec.cc:sendResultFiles
    ResultMailInfo rmi;
    rmi.init(m_game);

    std::map<String_t, game::PlayerSet_t> allPlayers;
    for (int slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
        // List players
        afl::data::StringList_t playersByFormat[NumFormats];
        afl::data::StringList_t players;
        m_game.listPlayers(slot, players);
        addPlayers(allPlayers, players, slot);

        // Send results in all formats
        collectPlayers(m_root, m_game, players, playersByFormat);
        rmi.sendResults(m_root, m_root.mailQueue(), m_game, slot, playersByFormat);
    }

    // Distribute results to local directories
    installResults(m_game, m_root, allPlayers);

    // Clear status flags
    m_game.endChanged().remove();
    m_game.configChanged().remove();
    m_game.scheduleChanged().remove();
}

void
server::host::ResultSender::installAllResults()
{
    std::map<String_t, game::PlayerSet_t> allPlayers;
    for (int slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
        afl::data::StringList_t players;
        m_game.listPlayers(slot, players);
        addPlayers(allPlayers, players, slot);
    }

    installResults(m_game, m_root, allPlayers);
}
