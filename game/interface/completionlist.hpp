/**
  *  \file game/interface/completionlist.hpp
  *  \brief Class game::interface::CompletionList
  */
#ifndef C2NG_GAME_INTERFACE_COMPLETIONLIST_HPP
#define C2NG_GAME_INTERFACE_COMPLETIONLIST_HPP

#include <set>
#include "afl/string/string.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    /** Command Line Completion Candidate List.
        Maintains a list of possible completions for a given stem.

        This class is intended for CCScript completion.
        Therefore, it is (mostly) case-insensitive and supports (mostly) only ASCII.
        It honors the convention that '$' ends a few property names (which are desired completions),
        but also is part of internal names (which are not desired completions normally).

        Usage:
        - construct with a stem (or use setStem)
        - add candidates using addCandidate()
        - getImmediateCompletion() will return a completion common to all candidates
        - use begin(), end() to iterate through all candidates */
    class CompletionList {
     public:
        typedef std::set<String_t> Container_t;
        typedef Container_t::const_iterator Iterator_t;

        /** Constructor.
            \param stem Stem.
            \see setStem */
        CompletionList(const String_t& stem = String_t());

        /** Destructor. */
        ~CompletionList();

        /** Set stem.
            All completions must start with this text, ignoring case.
            This call implies clear().
            \param stem Stem */
        void setStem(const String_t& stem);

        /** Get stem.
            \return stem */
        const String_t& getStem() const;

        /** Discard completions.
            \post isEmpty() */
        void clear();

        /** Add a completion candidate.
            \param candidate Possible candidate word.
                             CompletionList checks whether it fulfills the conditions and,
                             if possible, adds it to the candidate list. */
        void addCandidate(const String_t& candidate);

        /** Get immediate completion.
            If all candidates continue with the same text after the stem,
            that can be completed without offering the user a list to choose from.
            getImmediateCompletion() checks for such a common text and returns it.

            The return value can be
            - empty (if no candidate matches so far)
            - just the stem (if there are candidates but no immediate completion)
            - a string consisting of the stem and possible extra characters,
              ignoring case, representing the immediate completion

            \return immediate completion, if any */
        String_t getImmediateCompletion() const;

        /** Get first candidate.
            The candidate list/set contains candidates as-is, even if they do not match the stem's case.
            \return iterator */
        Iterator_t begin() const;

        /** Get one-past-last candidate.
            \return iterator */
        Iterator_t end() const;

        /** Check emptiness.
            \retval true Candidate list is empty
            \retval false There are candidates */
        bool isEmpty() const;

     private:
        Container_t m_data;
        String_t m_stem;
    };

    /** Build completions for a command-line.

        Pass in text up to the cursor position as \c text.
        This function will identify a position to perform completion at and determine a possible stem.
        Use out.getStem() to determine the stem; this will be a suffix of \c text.

        \param [out] out          Completions will be produced here
        \param [in]  text         Command-line for which to attempt completion
        \param [in]  session      Session (for script commands, file system, root for configuration)
        \param [in]  onlyCommands true to complete only command names (and nothing when not at command position); false to determine valid types from context
        \param [in]  contexts     Active contexts in addition to global context */
    void buildCompletionList(CompletionList& out,
                             const String_t& text,
                             Session& session,
                             bool onlyCommands,
                             const afl::container::PtrVector<interpreter::Context>& contexts);

} }

#endif
