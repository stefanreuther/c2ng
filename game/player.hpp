/**
  *  \file game/player.hpp
  *  \brief Class game::Player
  */
#ifndef C2NG_GAME_PLAYER_HPP
#define C2NG_GAME_PLAYER_HPP

#include "afl/string/string.hpp"
#include "afl/base/types.hpp"

namespace game {

    /** Information about a player slot.
        A PlayerList stores an array of these.

        Each Player describes a fixed position in a game, that need not play.
        For example, player 5 typically is Privateers, regardless of whether they play in the game or not.
        Which player positions exist is determined by the host version,
        but which player slots are actually used can normally not be found out without server help.

        A player slot has a multitude of names giving information about that slot.
        Conventionally, that is the short/long/adjective name;
        we allow storing some additional information as well as host/user versions of the names.

        A Player can be real (default) or not.
        A real player corresponds to an actual race slot.
        An unreal player is either slot 0 (used as owner for unowned items),
        or slot 12 in classic games (used as owner for non-game units).
        The respective player slots will be used to allow naming of appropriate units,
        but marked as unreal so players cannot send messages to these.

        Player provides a change flag which is used by PlayerList::notifyListeners(). */
    class Player {
     public:
        /** Possible name fields. */
        enum Name {
            ShortName,              ///< Short name. Use for sentences like "This ship belongs to 'The Frogs'".
            AdjectiveName,          ///< Adjective. Use for sentences like "This is a 'Frog' ship".
            LongName,               ///< Long name. Generally only used in headings.
            OriginalShortName,      ///< Short name, host's version. Used for message parsing.
            OriginalAdjectiveName,  ///< Adjective, host's version. Used for message parsing.
            OriginalLongName,       ///< Long name, host's version. Used for message parsing.
            UserName,               ///< User login name, if known. Used to refer to the user, e.g. in URLs or addresses ('@jrluser').
            NickName,               ///< User display name, if known. Used to refer to the user in text ('J.R.Luser').
            EmailAddress            ///< User email address, if known.
        };
        static const size_t NUM_NAMES = EmailAddress+1;

        /** Constructor.
            \param id Player number. */
        explicit Player(int id);

        /** Get player number.
            \return player number */
        int getId() const;

        /** Set player status.
            \param flag true: this is a real player that takes part in the game; false: not a real player
            \post isReal() == flag */
        void setIsReal(bool flag);

        /** Get player status.
            \retval true this is a real player that takes part in the game
            \retval false not a real player */
        bool isReal() const;

        /** Set name.
            \param which Which name to set
            \param name New name
            \post getName(which) == name */
        void setName(Name which, String_t name);

        /** Set original names.
            Copies the current names (LongName etc.) into the original names (OriginalLongName) slots. */
        void setOriginalNames();

        /** Get name.
            \param which Which name to get
            \return name */
        const String_t& getName(Name which) const;

        /** Initialize for standard "unowned" slot.
            Sets name appropriately for slot 0 (unowned units). */
        void initUnowned();

        /** Initialize for standard "aliens" slot.
            Sets name appropriately for slot 12 (PCC/Jumpgate/... aliens). */
        void initAlien();

        /** Mark this player changed.
            \param state New state. Default value is true to mark this player changed for the next notifyListeners iteration. */
        void markChanged(bool state = true);

        /** Check whether this player was changed.
            \return true if player was changed */
        bool isChanged() const;

     private:
        const int m_id;
        bool m_isReal;
        bool m_changed;
        String_t m_names[NUM_NAMES];
    };

}

#endif
