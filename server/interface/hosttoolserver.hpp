/**
  *  \file server/interface/hosttoolserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTOOLSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTTOOLSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hosttool.hpp"

namespace server { namespace interface {

    class HostToolServer : public ComposableCommandHandler {
     public:
        HostToolServer(HostTool& impl, HostTool::Area area);
        ~HostToolServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packTool(const HostTool::Info& tool);

     private:
        HostTool& m_implementation;
        const HostTool::Area m_area;

        bool isCommand(const String_t& upcasedCommand, const char* suffix) const;
    };

} }

#endif
