/**
  *  \file server/play/commandhandler.hpp
  *  \brief Interface server::play::CommandHandler
  */
#ifndef C2NG_SERVER_PLAY_COMMANDHANDLER_HPP
#define C2NG_SERVER_PLAY_COMMANDHANDLER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "interpreter/arguments.hpp"

namespace server { namespace play {

    class PackerList;

    /** Interface for handling commands. */
    class CommandHandler : public afl::base::Deletable {
     public:
        /** Process a command.
            @param [in]     cmd    Command verb
            @param [in,out] args   Parameters; function consumes these
            @param [in,out] objs   If command causes objects to be changed, add appropriate packers here */
        virtual void processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs) = 0;
    };

} }

#endif
