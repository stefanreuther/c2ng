/**
  *  \file server/interface/hostplayerserver.cpp
  */

#include <stdexcept>
#include "server/interface/hostplayerserver.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Hash;
using afl::data::HashValue;

server::interface::HostPlayerServer::HostPlayerServer(HostPlayer& impl)
    : m_implementation(impl)
{ }

server::interface::HostPlayerServer::~HostPlayerServer()
{ }

bool
server::interface::HostPlayerServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "PLAYERJOIN") {
        /* @q PLAYERJOIN game:GID slot:Int user:UID (Host Command)
           Join a game.
           Fails if the slot is already taken.

           Permissions: admin and game owner can join everyone; user can join public/unlisted games.

           @err 404 User does not exist (UID does not exist)
           @err 412 Wrong game state (game is not %running or %joining)
           @err 403 Permission denied (game is not %public or %unlisted, and user is not admin or owner)
           @err 403 Permission denied (user already plays on this game)
           @err 409 Slot is not available
           @uses game:$GID:player:$P:users */
        args.checkArgumentCount(3);
        int32_t gameId = toInteger(args.getNext());
        int32_t slotId = toInteger(args.getNext());
        String_t userId = toString(args.getNext());

        m_implementation.join(gameId, slotId, userId);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "PLAYERSUBST") {
        /* @q PLAYERSUBST game:GID slot:Int user:UID (Host Command)
           Set replacement player.
           Arranges for %user to be the current replacement player (=last in {game:$GID:player:$P:users}) for %slot.

           If the caller is owner or admin, just adds them to the back (or removes everyone after them).
           If the caller is already on the slot, discards everyone after the caller and adds the user at the end.

           Permissions: admin and game owner can add everyone, users can add replacements for themselves.

           @err 412 Wrong game state (game is not %running or %joining)
           @err 412 Slot not in use (PLAYERSUBST only works for occupied slots, use {PLAYERJOIN} for empty ones)
           @err 403 Permission denied (caller doesn't have sufficient permissions)
           @uses game:$GID:player:$P:users */
        args.checkArgumentCount(3);
        int32_t gameId = toInteger(args.getNext());
        int32_t slotId = toInteger(args.getNext());
        String_t userId = toString(args.getNext());

        m_implementation.substitute(gameId, slotId, userId);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "PLAYERRESIGN") {
        /* @q PLAYERRESIGN game:GID slot:Int user:UID (Host Command)
           Remove player.
           If the player also has replacements, removes these as well.
           This command can be used by admins and owners to remove everyone,
           by players to remove their replacements.

           Permissions: admin and game owner can drop everyone, users can drop themselves.

           @err 412 Wrong game state (game is not %running or %joining)
           @err 403 Permission denied (%user is not active in this slot)
           @err 403 Permission denied (insufficient permissions)
           @uses game:$GID:player:$P:users */
        args.checkArgumentCount(3);
        int32_t gameId = toInteger(args.getNext());
        int32_t slotId = toInteger(args.getNext());
        String_t userId = toString(args.getNext());

        m_implementation.resign(gameId, slotId, userId);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "PLAYERADD") {
        /* @q PLAYERADD game:GID user:UID (Host Command)
           Add player to game.
           Creates their reference count (and sets it to 0) unless it already exists.
           This allows the player to access private games,
           but does not yet assign them a slot.

           Permissions: config-access to game.

           @err 403 Permission denied (caller is not admin or owner)
           @uses user:$UID:games */
        args.checkArgumentCount(2);
        int32_t gameId = toInteger(args.getNext());
        String_t userId = toString(args.getNext());

        m_implementation.add(gameId, userId);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "PLAYERLS") {
        /* @q PLAYERLS game:GID [ALL] (Host Command)
           Get information about all players.
           By default, reports all current (not dead) slots; with ALL, reports information about all slots.

           Permissions: read-access to game.

           @retval Hash for each slot, one {@type HostPlayerInfo}
           @rettype HostPlayerInfo
           @see PLAYERSTAT
           @uses game:$GID:player:$P:users */
        args.checkArgumentCountAtLeast(1);
        int32_t gameId = toInteger(args.getNext());
        bool allPlayers = false;
        while (args.getNumArgs() > 0) {
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (keyword == "ALL") {
                allPlayers = true;
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        std::map<int,HostPlayer::Info> list;
        m_implementation.list(gameId, allPlayers, list);

        Vector::Ref_t v = Vector::create();
        for (std::map<int,HostPlayer::Info>::const_iterator it = list.begin(), end = list.end(); it != end; ++it) {
            v->pushBackInteger(it->first);
            v->pushBackNew(packInfo(it->second));
        }
        result.reset(new VectorValue(v));
        return true;
    } else if (upcasedCommand == "PLAYERSTAT") {
        /* @q PLAYERSTAT game:GID slot:Int (Host Command)
           Get information about one player slot.

           Permissions: read-access to game.

           @err 412 Slot not in use

           @retval HostPlayerInfo
           @see PLAYERLS
           @uses game:$GID:player:$P:users */
        args.checkArgumentCount(2);
        int32_t gameId = toInteger(args.getNext());
        int32_t slotId = toInteger(args.getNext());

        result.reset(packInfo(m_implementation.getInfo(gameId, slotId)));
        return true;
    } else if (upcasedCommand == "PLAYERSETDIR") {
        /* @q PLAYERSETDIR game:GID user:UID dir:FileName (Host Command)
           Set directory name for online play.
           Every user can configure a directory on the user filer for every game they play in.

           This command configures the directory in the database,
           and sets the property "game" on the directory.

           Permissions: admin or same as %user.

           @err 403 Permission denied (caller is not admin or user, or user is not on game)
           @err 601 Directory in use (directory already configured for another game)
           @see PLAYERGETDIR
           @uses game:$GID:user:$UID, MKDIRHIER (File Command), PROPGET (File Command) */
        args.checkArgumentCount(3);
        int32_t gameId   = toInteger(args.getNext());
        String_t userId  = toString(args.getNext());
        String_t dirName = toString(args.getNext());

        m_implementation.setDirectory(gameId, userId, dirName);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "PLAYERGETDIR") {
        /* @q PLAYERGETDIR game:GID user:UID (Host Command)
           Get directory name for online play.

           Permissions: admin or same as %user.

           @err 403 Permission denied (caller is not admin or user, or user is not on game)

           @retval FileName Directory name or empty string if none configured
           @see PLAYERSETDIR
           @uses game:$GID:user:$UID */
        args.checkArgumentCount(2);
        int32_t gameId  = toInteger(args.getNext());
        String_t userId = toString(args.getNext());

        result.reset(makeStringValue(m_implementation.getDirectory(gameId, userId)));
        return true;
    } else if (upcasedCommand == "PLAYERCHECKFILE") {
        /* @q PLAYERCHECKFILE game:GID user:UID name:Str [DIR dir:FileName] (Host Command)
           Check file creation permission.
           If a directory is managed by the host service because it has been configured for online play
           (see {PLAYERSETDIR}), users are not allowed to manipulate game files in that directory.
           This command checks whether %name is such a game file.
           Optionally, the directory can be passed as well to detect reconfiguration races.

           Permissions: admin or same as %user.

           @retval Str check result:
           - "stale": %dir is not a managed directory for %game
           - "allow": the file is allowed to be uploaded, use {PUT (File Command)} to upload it
           - "trn": this is a turn file, use {TRN (Host Command)} to upload it
           - "refuse": this file is not allowed to be uploaded

           @uses game:$GID:user:$UID */
        args.checkArgumentCountAtLeast(3);
        int32_t gameId    = toInteger(args.getNext());
        String_t userId   = toString(args.getNext());
        String_t fileName = toString(args.getNext());
        afl::base::Optional<String_t> gameDirToCheck;
        while (args.getNumArgs() > 0) {
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (keyword == "DIR") {
                args.checkArgumentCountAtLeast(1);
                gameDirToCheck = toString(args.getNext());
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        HostPlayer::FileStatus status = m_implementation.checkFile(gameId, userId, fileName, gameDirToCheck);
        result.reset(makeStringValue(HostPlayer::formatFileStatus(status)));
        return true;
    } else {
        return false;
    }
}

server::Value_t*
server::interface::HostPlayerServer::packInfo(const HostPlayer::Info& i)
{
    // ex Game::describeSlot (part)
    /* @type HostPlayerInfo
       Information about a player in a game.
       @key long:Str       Long race name
       @key short:Str      Short race name
       @key adj:Str        Race name adjective
       @key users:StrList  {@type UID|User Ids} of all users in this slot.
                           The primary player is on the first slot,
                           replacements on following slots.
       @key editable:Int   Number of editable users (number of users you can call {PLAYERRESIGN} for)
                           at the end of the %users list.
       @key joinable:Int   1=you can join this slot */
    Vector::Ref_t v = Vector::create();
    v->pushBackElements(i.userIds);

    Hash::Ref_t h = Hash::create();
    h->setNew("long", makeStringValue(i.longName));
    h->setNew("short", makeStringValue(i.shortName));
    h->setNew("adj", makeStringValue(i.adjectiveName));
    h->setNew("users", new VectorValue(v));
    h->setNew("editable", makeIntegerValue(i.numEditable));
    h->setNew("joinable", makeIntegerValue(i.joinable));

    return new HashValue(h);
}
