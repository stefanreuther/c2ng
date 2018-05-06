/**
  *  \file game/interface/completionlist.hpp
  */
#ifndef C2NG_GAME_INTERFACE_COMPLETIONLIST_HPP
#define C2NG_GAME_INTERFACE_COMPLETIONLIST_HPP

#include <set>
#include "afl/string/string.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class CompletionList {
     public:
        typedef std::set<String_t> Container_t;
        typedef Container_t::const_iterator Iterator_t;

        CompletionList(const String_t& stem = String_t());

        ~CompletionList();

        void setStem(const String_t& stem);

        const String_t& getStem() const;

        void clear();

        void addCandidate(const String_t& candidate);

        String_t getImmediateCompletion() const;

        Iterator_t begin() const;

        Iterator_t end() const;

        bool isEmpty() const;

     private:
        Container_t m_data;
        String_t m_stem;
    };

    void buildCompletionList(CompletionList& out,
                             const String_t& text,
                             Session& session,
                             bool onlyCommands,
                             const afl::container::PtrVector<interpreter::Context>& contexts);

} }

#endif
