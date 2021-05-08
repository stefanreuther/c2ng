/**
  *  \file game/msg/subsetmailbox.hpp
  *  \brief Class game::msg::SubsetMailbox
  */
#ifndef C2NG_GAME_MSG_SUBSETMAILBOX_HPP
#define C2NG_GAME_MSG_SUBSETMAILBOX_HPP

#include <vector>
#include <cstddef>
#include "game/msg/mailbox.hpp"
#include "afl/base/optional.hpp"

namespace game { namespace msg {

    /** Mailbox containing a subset of another.

        Publishes messages from another mailbox, given a vector of indexes.
        For example, when instantiated with a vector {0, 5, 6, 7}, it will
        report a mailbox with four messages, namely the first (index 0) and 6.-8. (index 5-7)of the other mailbox. */
    class SubsetMailbox : public Mailbox {
     public:
        /** Constructor.
            \param parent  Other mailbox; must live longer than the SubsetMailbox
            \param indexes Indexes of messages to report */
        SubsetMailbox(Mailbox& parent, const std::vector<size_t>& indexes);
        ~SubsetMailbox();

        afl::base::Optional<size_t> find(size_t outerIndex);
        size_t getOuterIndex(size_t index) const;

        // Mailbox:
        virtual size_t getNumMessages() const;
        virtual String_t getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual int getMessageTurnNumber(size_t index) const;
        virtual bool isMessageFiltered(size_t index, afl::string::Translator& tx, const PlayerList& players, const Configuration& config) const;
        virtual Flags_t getMessageFlags(size_t index) const;
        virtual Actions_t getMessageActions(size_t index) const;
        virtual void performMessageAction(size_t index, Action a);

     private:
        Mailbox& m_parent;
        std::vector<size_t> m_indexes;
    };

} }

#endif
