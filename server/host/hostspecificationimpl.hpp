/**
  *  \file server/host/hostspecificationimpl.hpp
  *  \brief Class server::host::HostSpecificationImpl
  */
#ifndef C2NG_SERVER_HOST_HOSTSPECIFICATIONIMPL_HPP
#define C2NG_SERVER_HOST_HOSTSPECIFICATIONIMPL_HPP

#include "afl/data/hash.hpp"
#include "server/host/spec/publisher.hpp"
#include "server/interface/hostspecification.hpp"

namespace server { namespace host {

    class Root;
    class Session;

    /** Implementation of HostSpecification.
        This class implements the command processing, parameter validation and output formatting.
        Actual data acquisition is in interface server::host::spec::Publisher. */
    class HostSpecificationImpl : public server::interface::HostSpecification {
     public:
        /** Constructor.
            @param session Session;
            @param root    Service root;
            @param pub     Specification publisher. Normally, same as root.specPublisher(), but can be different for testing */
        HostSpecificationImpl(const Session& session, Root& root, server::host::spec::Publisher& pub);

        // HostSpecification:
        virtual Value_t* getShiplistData(String_t shiplistId, Format format, const afl::data::StringList_t& keys);
        virtual Value_t* getGameData(int32_t gameId, Format format, const afl::data::StringList_t& keys);

     private:
        const Session& m_session;
        Root& m_root;
        server::host::spec::Publisher& m_publisher;

        Value_t* formatResult(afl::data::Hash::Ref_t result, Format fmt);
        Value_t* getShiplistDataWithFlak(String_t shiplistId, Format format, const afl::data::StringList_t& keys, String_t flakTool);

    };

} }

#endif
