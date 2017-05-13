/**
  *  \file server/talk/talksyntax.hpp
  *  \brief Class server::talk::TalkSyntax
  */
#ifndef C2NG_SERVER_TALK_TALKSYNTAX_HPP
#define C2NG_SERVER_TALK_TALKSYNTAX_HPP

#include "server/interface/talksyntax.hpp"
#include "util/syntax/keywordtable.hpp"

namespace server { namespace talk {

    /** Implementation of SYNTAX commands. */
    class TalkSyntax : public server::interface::TalkSyntax {
     public:
        /** Constructor.
            \param table Syntax table to use. */
        explicit TalkSyntax(const util::syntax::KeywordTable& table);

        // TalkSyntax interface:
        virtual String_t get(String_t key);
        virtual afl::base::Ref<afl::data::Vector> mget(afl::base::Memory<const String_t> keys);

     private:
        const util::syntax::KeywordTable& m_table;
    };

} }

#endif
