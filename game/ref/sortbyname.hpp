/**
  *  \file game/ref/sortbyname.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYNAME_HPP
#define C2NG_GAME_REF_SORTBYNAME_HPP

#include "game/ref/sortpredicate.hpp"
#include "game/session.hpp"

namespace game { namespace ref {

    class SortByName : public SortPredicate {
     public:
        SortByName(Session& session);

        virtual int compare(const Reference& a, const Reference& b) const;
        virtual String_t getClass(const Reference& a) const;

        String_t getName(const Reference& a) const;

     private:
        Session& m_session;
    };

} }

#endif
