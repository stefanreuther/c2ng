/**
  *  \file game/turnloader.hpp
  *  \brief Base class game::TurnLoader
  */
#ifndef C2NG_GAME_TURNLOADER_HPP
#define C2NG_GAME_TURNLOADER_HPP

#include <memory>
#include "afl/base/closure.hpp"
#include "afl/base/deletable.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/charset/charset.hpp"
#include "afl/string/translator.hpp"
#include "game/playerset.hpp"
#include "game/task.hpp"

namespace game {

    class Turn;
    class Game;
    class Root;
    class Session;

    /** Turn loader.
        Provides an interface to load and save current and historic turns and databases.

        <b>Functions returning a Task:</b>

        The returned Task object is intended to perform the required task, and then emit a callback with the result.

        These functions are permitted to execute the actual operation ahead of time and just return a dummy task.
        However, they must not confirm the operation before the return value has been invoked.

        If the Task requires some interaction, it is permitted to suspend and later resume.
        How this resumption is achieved is out of the scope of TurnLoader.
        The task must be resumed in the same thread that started it.

        Caller may decide to abort the task by destroying it.
        In this case, it needs to stop operating and drop all references.

        For now (20220116), the ability to suspend is primarily intended for user interactions, NOT for I/O.
        Should we enable that for I/O, we'd need to enable console/server programs to support resumption of a task.
        Those currently assume that local I/O does not suspend. */
    class TurnLoader : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Player status.
            \see getPlayerStatus() */
        enum PlayerStatus {
            /** This player's data is available.
                This bit must be set for this player to be accessible.
                If it is not set, other bits are ignored. */
            Available,

            /** This player's data is playable.
                If it is not set, data is only viewable (historic/finished game or alliance data, maybe). */
            Playable,

            /** This is primary data.
                The idea is to point out the default data to load.
                It does not otherwise affect us.
                An example use would be that this is the player's primary race whereas the others are allies. */
            Primary
        };
        typedef afl::bits::SmallSet<PlayerStatus> PlayerStatusSet_t;


        /** History status.
            \see getHistoryStatus() */
        enum HistoryStatus {
            /** History is known to not be available. */
            Negative,

            /** We are optimistic that history is available, but verifying it would be very expensive.
                For example: we need network access to verify it. */
            WeaklyPositive,

            /** We are certain that history is available.
                For example: we checked that the required files exist. */
            StronglyPositive
        };

        /** Property identifier. */
        enum Property {
            LocalFileFormatProperty,           ///< Local file format (System.Local).
            RemoteFileFormatProperty,          ///< Remote (turn) file format (System.Remote).
            RootDirectoryProperty              ///< Root directory (System.RootDirectory).
        };

        /** Get player status.
            Valid player numbers can be taken from the PlayerList.
            If an invalid player number is passed in, this function must return an empty status and empty \c extra.

            Note that "valid player number" means that this number theoretically exists, not that it exists now.
            A VGAP3 game with, say, a current player7.rst and a previous player3.rst will report player 3 as unavailable
            (empty status), but player 3 is on the PlayerList, and TurnLoader can still produce a helpful message in \c extra.

            \param player [in] Player to inquiry.
            \param extra [out] Extra information. Free-form text to describe this player's data (e.g. "RST", "RST+TRN", "Conflict").
            \param tx [in] Translator used to create the \c extra field.
            \return player status */
        virtual PlayerStatusSet_t getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const = 0;

        /** Load current turn.

            The resulting playability game's status will be set by the caller.

            \param turn [out] Turn to load. Should be completely initialized.
            \param game [in/out] Game object. May be updated with planet/ship score definitions, turn scores.
            \param player [in] Player number.
            \param root [in/out] Root. May be updated with configuration.
            \param session [in/out] Session.
            \param then [in] Task to execute after saving; never null.

            \return Newly-allocated task to perform the operation; never null. */
        virtual std::auto_ptr<Task_t> loadCurrentTurn(Turn& turn, Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t> then) = 0;

        /** Save current turn.
            This function produces a task that will save the current game, create and/or upload a turn file, etc.
            It must honor read/write mode restrictions defined by session.getEditableAreas().
            After completion, the task must emit a callback on \c then reporting success/failure.

            \param turn [in] Turn to save.
            \param game [in] Game object.
            \param player [in] Player number.
            \param root [in] Root.
            \param session [in/out] Session.
            \param then [in] Task to execute after saving; never null.

            \return Newly-allocated task to perform the operation; never null. */
        virtual std::auto_ptr<Task_t> saveCurrentTurn(const Turn& turn, const Game& game, int player, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then) = 0;

        /** Get history status.
            This function determines whether a number of turns have history information.

            Multiple turns can be queried at once.
            The \c status parameter provides room for one or more turns.
            It will be populated with status for this turn and following ones.

            \param player [in] Player number.
            \param turn [in] First turn number to inquire.
            \param status [out] Status goes here
            \param root [in] Root FIXME: change to UserConfiguration? */
        virtual void getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root) = 0;

        /** Load history turn.
            \param turn [out] Target turn object
            \param game [in] Game object. May be updated with planet/ship score definitions, turn scores.
            \param player [in] Player number
            \param turnNumber [in] Turn number
            \param root [in] Root object. Should not need updating.
            \param then [in] Task to execute after loading; never null.

            If the task produced by this function fails to load history data, it shall report failure through then \c then task.

            \return Newly-allocated task to perform the operation; never null. */
        virtual std::auto_ptr<Task_t> loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, std::auto_ptr<StatusTask_t> then) = 0;

        /** Get property for script interface.
            These values are published on the script interface (GlobalContext) and are not intended to be used in C++.
            \param p Property index
            \return value */
        virtual String_t getProperty(Property p) = 0;


        /*
         *  Utility Methods
         */

        /** Pick default player.
            Chooses a default player if there is one for this situation.
            \param baseSet set of players to check; pass in PlayerList::getAllPlayers().
            \return default player number if there is one; 0 if there is none or it's ambiguous */
        int getDefaultPlayer(PlayerSet_t baseSet) const;

     protected:
        /** Load current turn databases.
            This method should be called by the loadCurrentTurn() method, with the same parameters,
            to load the databases that are common to all versions:
            - starchart (chartX.cc)
            - scores (scoreX.cc)
            - message configuration
            - teams */
        void loadCurrentDatabases(Turn& turn, Game& game, int player, Root& root, Session& session);

        /** Load history turn databases.
            This method should be called by the loadHistoryTurn() method, with the same parameters,
            to load the databases that are common to all versions:
            - starchart (chartX.cc)
            - scores (scoreX.cc) */
        void loadHistoryDatabases(Turn& turn, Game& game, int player, int turnNumber, Root& root, afl::charset::Charset& charset);

        /** Save current turn databases.
            This method should be called by saveCurrentTurn() method, with the same parameters,
            to save databases that are common to all versions.
            - starchart
            - scores
            - message configuration
            - teams */
        void saveCurrentDatabases(const Turn& turn, const Game& game, int player, const Root& root, Session& session, afl::charset::Charset& charset);
    };

}

#endif
