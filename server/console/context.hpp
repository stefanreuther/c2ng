/**
  *  \file server/console/context.hpp
  *  \brief Interface server::console::Context
  */
#ifndef C2NG_SERVER_CONSOLE_CONTEXT_HPP
#define C2NG_SERVER_CONSOLE_CONTEXT_HPP

#include <memory>
#include "afl/base/deletable.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/value.hpp"
#include "afl/container/ptrvector.hpp"
#include "interpreter/arguments.hpp"

namespace server { namespace console {

    class Parser;

    /** Console context.
        Represents a state the user is in, and possible commands to accept.
        Context is transient/short-lived. */
    class Context : public afl::base::Deletable {
     public:
        /** Call a command.
            \param cmd    [in] Command verb
            \param args   [in] Command arguments
            \param parser [in] Invoking parser; can be used to recursively evaluate commands
            \param result [out] Result produced here. Pass in null here.
            \retval true Command was accepted, result has been produced
            \retval false Command not accepted, result not changed. Caller must generate error message. */
        virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result) = 0;

        /** Get name of this context.
            This is used to form prompts.
            \return name, should not be empty */
        virtual String_t getName() = 0;
    };

    typedef afl::container::PtrVector<Context> ContextStack_t;

} }

#endif
