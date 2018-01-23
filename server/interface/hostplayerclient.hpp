/**
  *  \file server/interface/hostplayerclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTPLAYERCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTPLAYERCLIENT_HPP

#include "server/interface/hostplayer.hpp"
#include "afl/net/commandhandler.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class HostPlayerClient : public HostPlayer {
     public:
        explicit HostPlayerClient(afl::net::CommandHandler& commandHandler);
        ~HostPlayerClient();

        virtual void join(int32_t gameId, int32_t slot, String_t userId);
        virtual void substitute(int32_t gameId, int32_t slot, String_t userId);
        virtual void resign(int32_t gameId, int32_t slot, String_t userId);
        virtual void add(int32_t gameId, String_t userId);
        virtual void list(int32_t gameId, bool all, std::map<int,Info>& result);
        virtual Info getInfo(int32_t gameId, int32_t slot);
        virtual void setDirectory(int32_t gameId, String_t userId, String_t dirName);
        virtual String_t getDirectory(int32_t gameId, String_t userId);
        virtual FileStatus checkFile(int32_t gameId, String_t userId, String_t fileName, afl::base::Optional<String_t> dirName);

        static Info unpackInfo(const Value_t* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
