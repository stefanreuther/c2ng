/**
  *  \file server/host/hostplayer.hpp
  *  \brief Class server::host::HostPlayer
  */
#ifndef C2NG_SERVER_HOST_HOSTPLAYER_HPP
#define C2NG_SERVER_HOST_HOSTPLAYER_HPP

#include "server/interface/hostplayer.hpp"

namespace server { namespace host {

    class Root;
    class Session;

    /** Implementation of HostPlayer interface.
        This interface implements PLAYER commands. */
    class HostPlayer : public server::interface::HostPlayer {
     public:
        /** Constructor.
            \param session Session
            \param root    Service root */
        HostPlayer(Session& session, Root& root);

        // HostPlayer virtuals:
        virtual void join(int32_t gameId, int32_t slot, String_t userId);
        virtual void substitute(int32_t gameId, int32_t slot, String_t userId);
        virtual void resign(int32_t gameId, int32_t slot, String_t userId);
        virtual void add(int32_t gameId, String_t userId);
        virtual void list(int32_t gameId, bool all, std::map<int,Info>& result);
        virtual Info getInfo(int32_t gameId, int32_t slot);
        virtual void setDirectory(int32_t gameId, String_t userId, String_t dirName);
        virtual String_t getDirectory(int32_t gameId, String_t userId);
        virtual FileStatus checkFile(int32_t gameId, String_t userId, String_t fileName, afl::base::Optional<String_t> dirName);
        virtual void set(int32_t gameId, String_t userId, String_t key, String_t value);
        virtual String_t get(int32_t gameId, String_t userId, String_t key);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
