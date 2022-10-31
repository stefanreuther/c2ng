/**
  *  \file game/msg/browser.hpp
  *  \brief Class game::msg::Browser
  */
#ifndef C2NG_GAME_MSG_BROWSER_HPP
#define C2NG_GAME_MSG_BROWSER_HPP

#include "game/playerlist.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace msg {

    class Mailbox;
    class Configuration;

    /** Browsing operations on a mailbox.

        For now, mailbox operations take multiple parameters (tx, players, config).
        This gathers operations working on those in a little more convenient interface.
        A Browser object is intended to be short-lived. */
    class Browser {
     public:
        /** Browser mode. */
        enum Mode {
            First,
            Last,
            Previous,
            Next
        };

        /** Summary information. */
        struct SummaryEntry {
            size_t index;
            size_t count;
            bool isFiltered;
            String_t heading;

            SummaryEntry(size_t index, size_t count, bool isFiltered, String_t heading)
                : index(index), count(count), isFiltered(isFiltered), heading(heading)
                { }
        };
        typedef std::vector<SummaryEntry> Summary_t;

        /** Search result. */
        struct Result {
            size_t index;         ///< Resulting index. Always valid.
            bool found;           ///< true if index is result of a match; false if fallback was used because no matching item was found.
            Result(size_t index, bool found)
                : index(index), found(found)
                { }
        };

        /** Constructor.
            \param mailbox   Mailbox to work on
            \param tx        Translator
            \param players   Player list
            \param config    Message configuration. If given as null, message filters are not used */
        Browser(const Mailbox& mailbox, afl::string::Translator& tx, const PlayerList& players, const Configuration* config);

        /** Check whether message is filtered.
            \param index Index, [0, mailbox.getNumMessages())
            \return true message is filtered
            \see Configuration::isHeadingFiltered */
        bool isMessageFiltered(size_t index) const;

        /** Find first message.
            This may not be the first message in the mailbox if some are filtered.
            \return Message index */
        size_t findFirstMessage() const;

        /** Find last message.
            This may not be the last message in the mailbox if some are filtered.
            \return Message index */
        size_t findLastMessage() const;

        /** Browse.
            \param index   Starting index (not used for First, Last)
            \param mode    Mode
            \param amount  Number of messages to skip (not used for First, Last)
            \return updated message index */
        size_t browse(size_t index, Mode mode, int amount) const;

        /** Browse.
            \param index   Starting index (not used for First, Last)
            \param mode    Mode
            \param amount  Number of messages to skip (not used for First, Last)
            \param needle  Search text
            \return updated message index and success flag */
        Result search(size_t index, Mode mode, int amount, const String_t& needle) const;

        /** Get summary.
            Retrieves a list of subject lines.
            \param [out] summary Summary list */
        void buildSummary(Summary_t& summary);

     private:
        class Acceptor;
        class BrowseAcceptor;
        class SearchAcceptor;

        const Mailbox& m_mailbox;
        afl::string::Translator& m_translator;
        const PlayerList& m_playerList;
        const Configuration* m_pConfig;

        Result findFirstMessage(const Acceptor& a) const;
        Result findLastMessage(const Acceptor& a) const;
        Result browse(size_t index, Mode mode, int amount, const Acceptor& a) const;
    };

} }

#endif
