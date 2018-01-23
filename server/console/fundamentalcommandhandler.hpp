/**
  *  \file server/console/fundamentalcommandhandler.hpp
  *  \brief Class server::console::FundamentalCommandHandler
  */
#ifndef C2NG_SERVER_CONSOLE_FUNDAMENTALCOMMANDHANDLER_HPP
#define C2NG_SERVER_CONSOLE_FUNDAMENTALCOMMANDHANDLER_HPP

#include "server/console/commandhandler.hpp"

namespace server { namespace console {

    class Environment;

    /** Fundamental commands.
        This includes
        - environment manipulation
        - control structures */
    class FundamentalCommandHandler : public CommandHandler {
     public:
        /** Constructor.
            \param env Environment */
        explicit FundamentalCommandHandler(Environment& env);

        /** Destructor. */
        ~FundamentalCommandHandler();

        // CommandHandler:
        virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result);

     private:
        Environment& m_environment;
    };

} }

#endif
