/**
  *  \file server/interface/hostfileserver.hpp
  *  \brief Class server::interface::HostFileServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTFILESERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTFILESERVER_HPP

#include "afl/data/hashvalue.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostfile.hpp"

namespace server { namespace interface {

    /** Server for host game access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostFile implementation. */
    class HostFileServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit HostFileServer(HostFile& impl);

        /** Destructor. */
        ~HostFileServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Pack HostFile::Info structure into a Value tree.
            @param info Input
            @return newly allocated HashValue; caller assumes ownership */
        static afl::data::HashValue* packInfo(const HostFile::Info& info);

        /** Pack array of HostFile::Info structures into a Value tree.
            @param infos Input
            @return newly allocated VectorValue of name/info pairs; caller assumes ownership */
        static afl::data::VectorValue* packInfos(afl::base::Memory<const HostFile::Info> infos);
     private:
        HostFile& m_implementation;
    };

} }

#endif
