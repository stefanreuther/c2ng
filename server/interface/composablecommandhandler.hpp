/**
  *  \file server/interface/composablecommandhandler.hpp
  *  \brief Class server::interface::ComposableCommandHandler
  */
#ifndef C2NG_SERVER_INTERFACE_COMPOSABLECOMMANDHANDLER_HPP
#define C2NG_SERVER_INTERFACE_COMPOSABLECOMMANDHANDLER_HPP

#include <memory>
#include "afl/net/commandhandler.hpp"
#include "interpreter/arguments.hpp"

namespace server { namespace interface {

    /** Composable CommandHandler.
        A normal CommandHandler's only way to refuse a command is by throwing an exception.
        This makes it hard to build an interface that dispatches commands into multiple CommandHandler's.
        In addition, each of these CommandHandler's probably contains the same boilerplate code to extract the command verb.

        ComposableCommandHandler allows to build such a dispatcher without having to rely on exceptions.
        The dispatcher will probably look like this
        <code>
          bool MyCCH::handleCommand(const String_t& uc, interpreter::Arguments& as, std::auto_ptr<Value_t>& result) {
              return first.handleCommand(uc, as, result)
                  || second.handleCommand(uc, as, result)
                  || third.handleCommand(uc, as, result);
          }
        </code>

        Each CommandHandler can also be used as a CommandHandler on its own. */
    class ComposableCommandHandler : public afl::net::CommandHandler {
     public:
        /** Handle a command.
            \param upcasedCommand [in] Command verb, in upper case
            \param args [in] Arguments
            \param result [out] Command result. Should be null on call.
            \retval true Command was recognized. \c args have possibly been used, \c result has been set.
            \retval false Command was not recognized. \c args and \c result are unchanged. */
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result) = 0;

        // CommandHandler:
        virtual Value_t* call(const Segment_t& command);
        virtual void callVoid(const Segment_t& command);
    };

} }

#endif
