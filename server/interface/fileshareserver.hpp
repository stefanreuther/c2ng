/**
  *  \file server/interface/fileshareserver.hpp
  *  \brief Class server::interface::FileShareServer
  */
#ifndef C2NG_SERVER_INTERFACE_FILESHARESERVER_HPP
#define C2NG_SERVER_INTERFACE_FILESHARESERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/fileshare.hpp"

namespace server { namespace interface {

    /** Server for file sharing information.
        Implements a ComposableCommandHandler and dispatches received commands to a FileShare implementation. */
    class FileShareServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit FileShareServer(FileShare& impl);

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Pack information into a value.
            @param info Information
            @return newly-allocated value */
        static afl::data::Value* packInfo(const FileShare::Info& info);

     private:
        FileShare& m_implementation;
    };

} }

#endif
