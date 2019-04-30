/**
  *  \file server/interface/userdataserver.hpp
  *  \brief Class server::interface::UserDataServer
  */
#ifndef C2NG_SERVER_INTERFACE_USERDATASERVER_HPP
#define C2NG_SERVER_INTERFACE_USERDATASERVER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class UserData;

    /** Server for UserData interface. */
    class UserDataServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            \param impl Implementation */
        explicit UserDataServer(UserData& impl);

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        UserData& m_implementation;
    };

} }

#endif
