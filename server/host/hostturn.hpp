/**
  *  \file server/host/hostturn.hpp
  *  \brief Class server::host::HostTurn
  */
#ifndef C2NG_SERVER_HOST_HOSTTURN_HPP
#define C2NG_SERVER_HOST_HOSTTURN_HPP

#include "server/interface/hostturn.hpp"

namespace server { namespace host {

    class Session;
    class Root;

    /** Implementation of HostTurn interface.
        This interface implements TURN commands. */
    class HostTurn : public server::interface::HostTurn {
     public:
        /** Constructor.
            \param session Session
            \param root    Service root */
        HostTurn(Session& session, Root& root);

        // HostTurn virtuals:
        virtual Result submit(const String_t& blob,
                              afl::base::Optional<int32_t> game,
                              afl::base::Optional<int32_t> slot,
                              afl::base::Optional<String_t> mail,
                              afl::base::Optional<String_t> info);
        virtual void setTemporary(int32_t gameId, int32_t slotNr, bool flag);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
