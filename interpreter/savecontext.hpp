/**
  *  \file interpreter/savecontext.hpp
  *  \brief Interface interpreter::SaveContext
  */
#ifndef C2NG_INTERPRETER_SAVECONTEXT_HPP
#define C2NG_INTERPRETER_SAVECONTEXT_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/data/hash.hpp"

namespace interpreter {

    class BytecodeObject;
    class ArrayData;
    class StructureTypeData;
    class StructureValueData;
    class Process;

    /** Save context, base class.
        Structured data is not immediately flattened when saving the initial reference into a segment.
        Instead, additional data objects are saved into the file.
        Those are referenced using a uint32_t Id, and can preserve sharing, cyclic references, etc.

        When saving a segment (i.e. in a BaseValue::store method), call one of the addXXX methods
        and create a TagNode using the return value as TagNode::value.
        - if we are saving a file that can preserve structured data,
          the caller in combination with the SaveContext will arrange for the structured data to be saved.
        - if structured data cannot be preserved, SaveContext will throw an appropriate exception.

        Objects are identified by their C++ identity (address);
        copying an object and adding it again creates a new instance. */
    class SaveContext : public afl::base::Deletable {
     public:
        /** Add bytecode object.
            \param bco bytecode object
            \return value for Tag_BCO tag
            \throw Error BCO cannot be saved */
        virtual uint32_t addBCO(const BytecodeObject& bco) = 0;

        /** Add hash.
            \param hash hash value object
            \return value for Tag_Hash tag
            \throw Error hash cannot be saved */
        virtual uint32_t addHash(const afl::data::Hash& hash) = 0;

        /** Add array.
            \param array array data object
            \return value for Tag_Array tag
            \throw Error array cannot be saved */
        virtual uint32_t addArray(const ArrayData& array) = 0;

        /** Add structure type object.
            \param type structure type object
            \return value for Tag_StructType tag
            \throw Error structure type cannot be saved */
        virtual uint32_t addStructureType(const StructureTypeData& type) = 0;

        /** Add structure value object.
            \param value structure value
            \return value for Tag_Struct tag
            \throw Error value cannot be saved */
        virtual uint32_t addStructureValue(const StructureValueData& value) = 0;

        /** Check for current process.
            This function is used to serialize mutexes that are part of a process' stack frames.
            It must return true if \c p refers to the process we're currently serializing.
            It must return false if \c p is null, we are not serializing a process, or \c p refers to another process.
            \param p process pointer
            \return true if p is the current process */
        // replaces getCurrentProcess()
        // must handle p==0 and return false!
        virtual bool isCurrentProcess(const Process* p) = 0;
    };

}

#endif
