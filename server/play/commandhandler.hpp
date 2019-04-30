/**
  *  \file server/play/commandhandler.hpp
  */
#ifndef C2NG_SERVER_PLAY_COMMANDHANDLER_HPP
#define C2NG_SERVER_PLAY_COMMANDHANDLER_HPP

#include "interpreter/arguments.hpp"
#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"

namespace server { namespace play {

    class PackerList;

    class CommandHandler : public afl::base::Deletable {
     public:
        virtual void processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs) = 0;
    };

} }

#endif
