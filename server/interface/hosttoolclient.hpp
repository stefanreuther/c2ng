/**
  *  \file server/interface/hosttoolclient.hpp
  *  \brief Class server::interface::HostToolClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTOOLCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTTOOLCLIENT_HPP

#include "afl/data/segment.hpp"
#include "afl/net/commandhandler.hpp"
#include "server/interface/hosttool.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Client for host tool access.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class HostToolClient : public HostTool {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the HostToolClient.
            @param area           Area to access */
        HostToolClient(afl::net::CommandHandler& commandHandler, Area area);

        /** Destructor. */
        ~HostToolClient();

        // HostTool virtuals:
        virtual void add(String_t id, String_t path, String_t program, String_t kind);
        virtual void set(String_t id, String_t key, String_t value);
        virtual String_t get(String_t id, String_t key);
        virtual bool remove(String_t id);
        virtual void getAll(std::vector<Info>& result);
        virtual void copy(String_t sourceId, String_t destinationId);
        virtual void setDefault(String_t id);
        virtual int32_t getDifficulty(String_t id);
        virtual void clearDifficulty(String_t id);
        virtual int32_t setDifficulty(String_t id, afl::base::Optional<int32_t> value, bool use);

        /** Unpack a serialized Info structure.
            @param p Value received from server
            @return Info */
        static Info unpackInfo(const Value_t* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
        const Area m_area;

        void addCommand(afl::data::Segment& seg, const char* suffix) const;
    };

} }

#endif
