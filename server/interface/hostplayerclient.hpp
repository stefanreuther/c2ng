/**
  *  \file server/interface/hostplayerclient.hpp
  *  \brief Class server::interface::HostPlayerClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTPLAYERCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTPLAYERCLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/hostplayer.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Client for host player access.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class HostPlayerClient : public HostPlayer {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the HostPlayerClient. */
        explicit HostPlayerClient(afl::net::CommandHandler& commandHandler);

        /** Destructor. */
        ~HostPlayerClient();

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

        /** Unpack a serialized Info structure.
            @param p Value received from server
            @return Info */
        static Info unpackInfo(const Value_t* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
