/**
  *  \file game/playerlist.hpp
  *  \brief Class game::PlayerList
  */
#ifndef C2NG_GAME_PLAYERLIST_HPP
#define C2NG_GAME_PLAYERLIST_HPP

#include "afl/base/signal.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"
#include "game/player.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"

namespace game {

    /** List of players.
        Manages a list of Player objects for a game.

        The list is indexed by player number.
        Player numbers start at 1 for regular players.

        Player 0 is reserved for "no player / unowned".
        Player 0 always exists in the PlayerList. */
    class PlayerList {
     public:
        /** Default constructor.
            Makes an empty list. */
        PlayerList();

        /** Destructor. */
        ~PlayerList();

        /** Create a player slot.
            If the slot does not exist, creates it.
            Otherwise, returns the existing slot.
            \param id Slot
            \return player; may be null if the \c id is not a valid slot number */
        Player* create(int id);

        /** Get player slot.
            Returns the existing player object, if any.
            \param id Slot
            \return player; may be null if the \c id is not a valid slot number or the slot has not been created */
        Player* get(int id) const;

        /** Reset this object.
            Discards all player objects and resets the content to the same state as after construction. */
        void clear();

        /** Get size.
            This is for informative purposes; use getFirstPlayer()/getNextPlayer() to iterate.
            \return upper bound for player numbers; there will be no player number greater than size() for which get() returns a value. */
        int size() const;

        /** Get player object, given a player character.
            \param ch player character (0-9, A-Z).
            \return player; null if invalid character given or slot was not created */
        Player* getPlayerFromCharacter(char ch) const;

        /** Get character from player number.
            \param id Player number
            \return character; '\0' if number cannot be represented */
        static char getCharacterFromPlayer(int id);

        /** Expand names in string template.
            The string can contain placeholders
            - "%X" for short name
            - "%-X" for adjective
            - "%%" for a literal percent sign
            Use this to format friendly codes, missions, etc.
            \param tpl Template
            \param useOriginalNames if true, use original names (OriginalAdjectiveName), otherwise, use normal names (AdjectiveName).
            \param tx Translator (for fallback names)
            \return expanded string */
        String_t expandNames(String_t tpl, bool useOriginalNames, afl::string::Translator& tx) const;

        /** Get set of all players.
            \return set of players where Player::isReal() is true */
        PlayerSet_t getAllPlayers() const;

        /** Get first player.
            \return first slot. Slot 0 is never returned, but otherwise non-real players can be returned. Null if no slot is occupied. */
        Player* getFirstPlayer() const;

        /** Get next player.
            \param p Player to start search from
            \return first slot after the given one. Null if end of list reached. */
        Player* getNextPlayer(const Player* p) const;

        /** Get next player.
            \param id Player number to start search from
            \return first slot after the given one. Null if end of list reached. */
        Player* getNextPlayer(int id) const;

        /** Get name of a player.
            Equivalent to get(id)->getName(which), but handles the case that get(id) returns null.
            \param id Slot
            \param which Which name to get
            \param tx Translator (for fallback names)
            \return name */
        String_t getPlayerName(int id, Player::Name which, afl::string::Translator& tx) const;

        /** Get names of all players.
            \param which Which names to get
            \param tx Translator (for fallback names)
            \return array of names. Values for empty or out-of-range indexes are empty. */
        PlayerArray<String_t> getPlayerNames(Player::Name which, afl::string::Translator& tx) const;

        /** Notify listeners.
            Call eventually after modifying players.
            If any player has its change flag set, resets it and invokes sig_change. */
        void notifyListeners();

        /** Callback. \see notifyListeners */
        afl::base::Signal<void()> sig_change;

     private:
        afl::container::PtrVector<Player> m_players;

        Player* findNextPlayer(int nr) const;
    };

}

#endif
