/**
  *  \file server/host/hostkey.hpp
  *  \brief Class server::host::HostKey
  */
#ifndef C2NG_SERVER_HOST_HOSTKEY_HPP
#define C2NG_SERVER_HOST_HOSTKEY_HPP

#include "server/interface/hostkey.hpp"

namespace server { namespace host {

    class Session;
    class Root;

    /** Implementation of HostKey interface.
        This interface implements KEY commands. */
    class HostKey : public server::interface::HostKey {
     public:
        /** Constructor.
            \param session Session
            \param root    Service root */
        HostKey(Session& session, Root& root);

        // Interface methods:
        void listKeys(Infos_t& out);
        String_t getKey(String_t keyId);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
