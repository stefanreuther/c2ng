/**
  *  \file server/host/hostturn.cpp
  *  \brief Class server::host::HostTurn
  */

#include <stdexcept>
#include "server/host/hostturn.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/turnfile.hpp"
#include "server/errors.hpp"
#include "server/host/exporter.hpp"
#include "server/host/game.hpp"
#include "server/host/gamearbiter.hpp"
#include "server/host/installer.hpp"
#include "server/host/keystore.hpp"
#include "server/host/root.hpp"
#include "server/host/schedule.hpp"
#include "server/host/session.hpp"
#include "server/host/user.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/hostgame.hpp"

using server::interface::HostGame;

namespace server { namespace host { namespace {
    const char LOG_NAME[] = "host.turn";

    // FIXME: duplicated from hostplayer.cpp
    size_t indexOf(const afl::data::StringList_t& list, const String_t& ele)
    {
        size_t i = 0;
        size_t size = list.size();
        while (i < size && list[i] != ele) {
            ++i;
        }
        return i;
    }

    /* Check whether to suggest "allow temporary".
       This is not a hard condition, just a suggestion; we accept TRNMARKTEMP at all times but it may not always make sense.
       We suggest "allow temporary" if
       - schedule is "run when all turns are in" or "run at fixed schedule, but allow to run earlier"
       - hostDelay is >= 5 minutes (so there's actually time to do it) */
    bool checkAllowTemp(Game& g)
    {
        afl::net::redis::Subtree schedule = g.getSchedule();
        String_t currentSchedule = schedule.stringListKey("list")[0];
        if (currentSchedule.empty()) {
            return false;
        }

        Schedule sch;
        sch.loadFrom(schedule.hashKey(currentSchedule));

        return (sch.getType() == server::interface::HostSchedule::Quick
                || sch.getHostEarly())
            && sch.getHostDelay() >= 5;
    }

    void rememberKey(Root& root, String_t userId, int32_t gameId, const game::v3::TurnFile& trn)
    {
        game::v3::RegistrationKey key(std::auto_ptr<afl::charset::Charset>(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1)));
        key.unpackFromBytes(afl::base::fromObject(trn.getDosTrailer().registrationKey));

        KeyStore(User(root, userId).keyStore(), root.config()).
            addKey(key, root.getTime(), gameId);
    }

} } }

server::host::HostTurn::HostTurn(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

server::interface::HostTurn::Result
server::host::HostTurn::submit(const String_t& blob,
                               afl::base::Optional<int32_t> gameLimit,
                               afl::base::Optional<int32_t> slotLimit,
                               afl::base::Optional<String_t> mail,
                               afl::base::Optional<String_t> info)
{
    // ex doTurnUpload

    // Parse the turn file and complete the parameters
    std::auto_ptr<game::v3::TurnFile> trn;
    afl::charset::CodepageCharset charset(afl::charset::g_codepageLatin1);
    try {
        afl::io::ConstMemoryStream ms(afl::string::toBytes(blob));
        afl::string::NullTranslator tx;
        trn.reset(new game::v3::TurnFile(charset, tx, ms, false));
    }
    catch (std::exception& e) {
        m_root.log().write(afl::sys::LogListener::Warn, LOG_NAME, afl::string::Format("Turn fails to parse: %s", e.what()));
        throw std::runtime_error(INVALID_FILE_FORMAT);
    }

    // slotNumber
    int32_t slotNumber;
    if (const int32_t* p = slotLimit.get()) {
        slotNumber = *p;
    } else {
        slotNumber = trn->getPlayer();
    }

    // gameNumber
    int32_t gameNumber;
    if (const int32_t* p = gameLimit.get()) {
        gameNumber = *p;
    } else {
        gameNumber = m_root.gameRoot().subtree("bytime").intKey(trn->getTimestamp().getTimestampAsString()).get();
    }
    if (gameNumber == 0) {
        throw std::runtime_error(GAME_NOT_FOUND);
    }

    // Obtain critical access: turn upload interferes with a host run
    GameArbiter::Guard guard(m_root.arbiter(), gameNumber, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameNumber);

    afl::data::StringList_t players;
    game.listPlayers(slotNumber, players);

    bool isAdmin = m_session.isAdmin();
    String_t user = m_session.getUser();
    size_t thisPlayerIndex;
    if (const String_t* p = mail.get()) {
        // Identify player by his email address. This is only allowed for admins.
        if (!isAdmin) {
            throw std::runtime_error(PERMISSION_DENIED);
        }

        // Start with out-of-range
        isAdmin = false;
        thisPlayerIndex = players.size();
        for (size_t i = players.size(); i > 0; --i) {
            String_t thisPlayerMail = User(m_root, players[i-1]).getEmailAddress();
            if (afl::string::strCaseCompare(thisPlayerMail, *p) == 0) {
                // Found him, so assume his credentials.
                // (If no match is found, we'll fall through to the "Permission denied" below).
                thisPlayerIndex = i-1;
                user = players[i-1];

                m_root.log().write(afl::sys::LogListener::Info, LOG_NAME,
                                   afl::string::Format("address '%s' resolved to user '%s'", p->c_str(), user.c_str()));
                break;
            }
        }
    } else {
        // Use authenticated player
        thisPlayerIndex = indexOf(players, user);
    }
    if (!isAdmin && thisPlayerIndex >= players.size()) {
        m_root.log().write(afl::sys::LogListener::Info, LOG_NAME,
                           afl::string::Format("game %d, player %d, user %s, mismatch, info %s",
                                               gameNumber,
                                               slotNumber,
                                               user.size() ? user.c_str() : "(none)",
                                               info.orElse("(none)")));
        if (mail.isValid()) {
            throw std::runtime_error(TRN_MAIL_MISMATCH);
        } else {
            throw std::runtime_error(PERMISSION_DENIED);
        }
    }
    if (game.getState() != HostGame::Running) {
        throw std::runtime_error(WRONG_GAME_STATE);
    }

    // Remember the used key
    rememberKey(m_root, user, gameNumber, *trn);

    // Build base directory
    afl::base::Ref<afl::io::DirectoryEntry> workdirEntry =
        m_root.fileSystem().openDirectory(m_root.config().workDirectory)->getDirectoryEntryByName("check");
    try {
        workdirEntry->createAsDirectory();
    }
    catch (std::exception&)
    { }

    // Export
    String_t relative;
    try {
        relative = Exporter(m_root.hostFile(), m_root.fileSystem(), m_root.log()).exportGame(game, m_root, workdirEntry->getPathName());
    }
    catch (std::exception& e) {
        // Convert errors.
        // Export might fail if hostfile does not contain required files (e.g. bin/, defaults/).
        // These 404's should not hit the user, who will interpret them in the context of the host service (i.e. game not found),
        // although they are actually internal errors (comparable to a database error).
        m_root.log().write(afl::sys::LogListener::Error, LOG_NAME, "error during export", e);
        throw std::runtime_error(afl::string::Format("%s [%s]", INTERNAL_ERROR, e.what()));
    }

    // Store turn
    workdirEntry->openDirectory()->openFile(afl::string::Format("%s/in/new/player%d.trn", relative, slotNumber), afl::io::FileSystem::Create)
        ->fullWrite(afl::string::toBytes(blob));

    // Run checkturn
    util::ProcessRunner::Command cmd;
    cmd.command.push_back("/bin/sh");
    cmd.command.push_back("bin/checkturn.sh");
    cmd.command.push_back(relative);
    cmd.command.push_back(afl::string::Format("%d", slotNumber));
    cmd.workDirectory = workdirEntry->getPathName();
    String_t output;
    int32_t code = m_root.checkturnRunner().run(cmd, output);

    // Process result
    Game::Slot slot(game.getSlot(slotNumber));
    int32_t existingState = slot.turnStatus().get();
    int32_t newState = (code + 1);
    if (newState == Game::TurnYellow
        || newState == Game::TurnGreen
        || ((existingState == Game::TurnMissing
             || existingState == Game::TurnDead)
            && newState <= Game::TurnBad))
    {
        // Turn has been kept, or there has not been a previous turn
        slot.turnStatus().set(newState);
    }

    // Log it
    m_root.log().write(afl::sys::LogListener::Info, LOG_NAME,
                       afl::string::Format("game %d, player %d, user %s, status %d, info %s")
                       << gameNumber
                       << slotNumber
                       << (user.size() ? user.c_str() : "(none)")
                       << newState
                       << info.orElse("(none)"));

    // Update
    bool allowTemp = false;
    if (newState == Game::TurnYellow || newState == Game::TurnGreen) {
        // Store the turn
        server::interface::FileBaseClient(m_root.hostFile()).putFile(afl::string::Format("%s/in/player%d.trn", game.getDirectory(), slotNumber), blob);

        // Turn has been kept, so trigger scheduler
        game.lastTurnSubmissionTime().set(m_root.getTime());
        m_root.handleGameChange(gameNumber);

        // Maybe we're taking over again for our replacement
        if (!isAdmin) {
            while (thisPlayerIndex+1 < players.size()) {
                game.popPlayerSlot(slotNumber, m_root);
                players.pop_back();
            }
        }

        // Distribute turn.
        Installer(m_root).installFileMulti(game, players, afl::string::Format("player%d.trn", slotNumber), blob, slotNumber);

        // Check desired status for allowTemp
        allowTemp = checkAllowTemp(game);
    }

    // Mark user active
    // (Note that the player may still be unknown if a turn is submitted using the admin console.)
    if (game.getType() != HostGame::TestGame && user.size() != 0) {
        m_root.activeUsers().add(user);
    }

    // Build protocol result
    Result result;
    result.state         = newState;
    result.output        = output;
    result.gameId        = gameNumber;
    result.slot          = slotNumber;
    result.previousState = existingState;
    result.turnNumber    = game.turnNumber().get();
    result.userId        = user;
    result.gameName      = game.getName();
    result.allowTemp     = allowTemp;
    return result;
}

void
server::host::HostTurn::setTemporary(int32_t gameId, int32_t slotNr, bool flag)
{
    // ex planetscentral/host/cmdplayer.h:doTurnMarkTemporary

    // Obtain critical access: set-temporary interferes with a host run
    // @change Classic did not require these permissions
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);

    if (!game.isSlotInGame(slotNr)) {
        throw std::runtime_error(SLOT_EMPTY);
    }

    Game::Slot slot(game.getSlot(slotNr));
    if (!m_session.isAdmin()) {
        // Not admin: user must be playing this slot
        afl::data::StringList_t players;
        slot.players().getAll(players);
        if (indexOf(players, m_session.getUser()) >= players.size()) {
            throw std::runtime_error(PERMISSION_DENIED);
        }
    }

    // Check existing turn state
    int32_t turnState = slot.turnStatus().get() & Game::TurnStateMask;
    if (turnState == Game::TurnGreen || turnState == Game::TurnYellow) {
        // ok
        if (flag) {
            turnState |= Game::TurnIsTemporary;
        }
        slot.turnStatus().set(turnState);

        // Update last-turn-submitted time so host doesn't immediately run
        if (!flag) {
            game.lastTurnSubmissionTime().set(m_root.getTime());
        }

        m_root.handleGameChange(gameId);
    } else {
        // bad turn state (no turn submitted, bad turn, ...)
        throw std::runtime_error(WRONG_TURN_STATE);
    }

}
