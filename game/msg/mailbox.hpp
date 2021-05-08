/**
  *  \file game/msg/mailbox.hpp
  */
#ifndef C2NG_GAME_MSG_MAILBOX_HPP
#define C2NG_GAME_MSG_MAILBOX_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/playerlist.hpp"

namespace game { namespace msg {

    class Configuration;

    /** Mailbox.

        FIXME: this interface is temporary.
        Ideally, we'd have multiple mailboxes that can ultimately model things like PMs received on a website.
        Having to pass a translator and player list is cumbersome, but required as we cannot pass them to a constructor:
        the PlayerList lives in a Root, whereas Mailboxes live in a Game, which have different lifetimes. */
    class Mailbox : public afl::base::Deletable {
     public:
        enum Flag {
            Confirmed
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        enum Action {
            ToggleConfirmed
        };
        typedef afl::bits::SmallSet<Action> Actions_t;

        /** Get number of messages in this mailbox. */
        virtual size_t getNumMessages() const = 0;

        /** Get text of a message.
            \param index message number, [0, getNumMessages()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \return message text */
        virtual String_t getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& players) const = 0;

        /** Get heading of a message.
            The heading is used for subject-sorting and the kill filter.
            \param index message number, [0, getNumMessages()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \return message heading */
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players) const = 0;

        /** Get turn of a message.
            \param index message number, [0, getCount()).
            \return turn number; can be 0. */
        virtual int getMessageTurnNumber(size_t index) const = 0;

        /** Check whether message is filtered.
            \param index message number, [0, getNumMessages()).
            \param tx Translator for internationalizable parts
            \param players Player list for player names
            \param config Message configuration
            \return true if message is filtered (should be hidden) */
        virtual bool isMessageFiltered(size_t index, afl::string::Translator& tx, const PlayerList& players, const Configuration& config) const = 0;

        /** Get message flags.
            \param index message number, [0, getNumMessages()).
            \return flags */
        virtual Flags_t getMessageFlags(size_t index) const = 0;

        /** Get permitted message actions.
            \param index message number, [0, getNumMessages()).
            \return set of actions */
        virtual Actions_t getMessageActions(size_t index) const = 0;

        /** Perform action on a message.
            \param index message number, [0, getNumMessages()).
            \param a     action, taken from getMessageActions() */
        virtual void performMessageAction(size_t index, Action a) = 0;
    };

} }

#endif
