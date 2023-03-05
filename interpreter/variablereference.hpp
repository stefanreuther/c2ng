/**
  *  \file interpreter/variablereference.hpp
  *  \brief Class interpreter::VariableReference
  */
#ifndef C2NG_INTERPRETER_VARIABLEREFERENCE_HPP
#define C2NG_INTERPRETER_VARIABLEREFERENCE_HPP

#include "afl/base/types.hpp"
#include "afl/data/value.hpp"
#include "afl/string/string.hpp"
#include "interpreter/process.hpp"

namespace interpreter {

    class ProcessList;

    /** Symbolic reference to a variable.
        We sometimes need to pass values from a script, through UI, to a script.
        Scalar values or values with simple structure are copied.
        However, we don't want to do that with values that have a complex structure
        because we cannot ensure that UI and script side do not examine or modify those in parallel.

        To solve this, we store such values in variables in a variable in a process.
        Usage sequence:
        - in the script/game thread, create a VariableReference::Maker, and call Maker::make() for every value that needs storing;
        - pass the resulting VariableReference objects through the UI thread;
        - in a new task in the script/game thread, use get() to resolve the references and access the values.

        This gives us automatic lifetime control for the values:
        a VariableReference is invalidated when the referenced process terminates.
        Also, because values are stored in a dummy frame, continuing the process will lose the values.
        This is appropriate for processes that perform UI interaction and are stopped during the interaction.

        As an escape mechanism, a VariableReference can also be created for a process/variable-name pair,
        and thus, for example, refer to the UI.RESULT variable. */
    class VariableReference {
     public:
        /** VariableReference factory.
            Creates a (number of) VariableReference objects by storing their values in temporary frames. */
        class Maker {
         public:
            /** Constructor.
                @param proc Process */
            explicit Maker(Process& proc);

            /** Create a variable reference.
                @param name    Name (should be unique; if re-used, it is undefined what happens to the previous value)
                @param value   Value (will be cloned, can be null)
                @return VariableReference such that get() returns (a clone of) the given value */
            VariableReference make(const String_t& name, const afl::data::Value* value);

         private:
            Process& m_process;
            Process::Frame& m_frame;
        };

        /** Create null VariableReference. */
        VariableReference();

        /** Create VariableReference from a process/name.
            @param proc  Process
            @param name  Variable name
            @return VariableReference */
        static VariableReference fromProcess(const Process& proc, const String_t& name);

        /** Resolve a VariableReference.
            @param list Process List
            @return Value; caller takes responsibility. Null if reference is stale or null. */
        std::auto_ptr<afl::data::Value> get(const ProcessList& list) const;

     private:
        friend class Maker;

        VariableReference(uint32_t processId, const String_t& name);

        String_t m_name;
        uint32_t m_processId;
    };

}

#endif
