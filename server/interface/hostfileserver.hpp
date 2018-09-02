/**
  *  \file server/interface/hostfileserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTFILESERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTFILESERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostfile.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vectorvalue.hpp"

namespace server { namespace interface {

    class HostFileServer : public ComposableCommandHandler {
     public:
        explicit HostFileServer(HostFile& impl);
        ~HostFileServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static afl::data::HashValue* packInfo(const HostFile::Info& info);
        static afl::data::VectorValue* packInfos(afl::base::Memory<const HostFile::Info> infos);
     private:
        HostFile& m_implementation;
    };

} }

#endif
