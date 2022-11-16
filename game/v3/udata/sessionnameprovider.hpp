/**
  *  \file game/v3/udata/sessionnameprovider.hpp
  *  \brief Class game::v3::udata::SessionNameProvider
  */
#ifndef C2NG_GAME_V3_UDATA_SESSIONNAMEPROVIDER_HPP
#define C2NG_GAME_V3_UDATA_SESSIONNAMEPROVIDER_HPP

#include "game/v3/udata/nameprovider.hpp"
#include "game/session.hpp"

namespace game { namespace v3 { namespace udata {

    /** Implementation of NameProvider for a game session.
        Supplies all names from a Session, provided it has all required sub-objects set. */
    class SessionNameProvider : public NameProvider {
     public:
        /** Constructor.
            @param session */
        explicit SessionNameProvider(Session& session);

        // NameProvider:
        virtual String_t getName(Type type, int id) const;

     private:
        Session& m_session;
    };

} } }

#endif
