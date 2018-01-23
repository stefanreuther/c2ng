/**
  *  \file server/console/environment.hpp
  *  \brief Class server::console::Environment
  */
#ifndef C2NG_SERVER_CONSOLE_ENVIRONMENT_HPP
#define C2NG_SERVER_CONSOLE_ENVIRONMENT_HPP

#include <memory>
#include "afl/data/namemap.hpp"
#include "afl/data/segment.hpp"

namespace server { namespace console {

    /** Script environment for console.
        Scripts have access to an environment containing arbitrary values.
        Variables are addressed by name, e.g. "$a" or "${name}".
        Values cannot be null (empty).

        Macros receive positional parameters.
        Positional parameters are accessed using 1-based numeric names, e.g. "$1" or "${20}".

        Class Environment provides storage for these values.

        Use-Cases:
        - set a variable: setNew()
        - temporarily shadow a variable, e.g. for a loop induction variable: pushNew() to set the new values.
          You receive the previous value in return; store it and restore it using popNew() later.
        - set positional parameters: pushPositionalParameters() to set the new values.
          You receive the previous value in return; store it and restore it using popPositionalParameters() later. */
    class Environment {
     public:
        typedef std::auto_ptr<afl::data::Value> ValuePtr_t;
        typedef std::auto_ptr<afl::data::Segment> SegmentPtr_t;

        /** Constructor.
            Makes an empty environment. */
        Environment();

        /** Destructor. */
        ~Environment();

        /** Set environment variable.
            \param name Name of variable (must not be all-numeric, i.e. a positional parameter)
            \param value Value */
        void setNew(String_t name, ValuePtr_t value);

        /** Temporarily replace an environment variable.
            \param name Name of variable (must not be all-numeric, i.e. a positional parameter)
            \param value Value
            \return old value. Pass this to popNew later. */
        ValuePtr_t pushNew(String_t name, ValuePtr_t value);

        /** End replacement of an environment variable.
            \param name Name of variable (must not be all-numeric, i.e. a positional parameter)
            \param value Old value to restore */
        void popNew(String_t name, ValuePtr_t value);

        /** Set positional parameters.
            \param seg New positional parameters. The segment will be looted (emptied); the values are moved, not copied.
            \return old positional parameters. Pass this to popPositionalParameters later. */
        SegmentPtr_t pushPositionalParameters(afl::data::Segment& seg);

        /** Restore positional parameters.
            Undoes a previous pushPositionalParameters().
            \param ptr Old positional parameters */
        void popPositionalParameters(SegmentPtr_t ptr);

        /** Get value of an environment variable.
            \param name Name of environment variable or positional parameter
            \return value (null if unset) */
        afl::data::Value* get(String_t name);

        /** List all variables.
            Appends all names and values (alternating) to the given result,
            for all environment variables and current positional parameters.
            \param result [out] Result */
        void listContent(afl::data::Segment& result);

     private:
        afl::data::NameMap m_names;
        afl::data::Segment m_values;

        afl::data::Segment m_positionalParameters;
    };

} }

#endif
