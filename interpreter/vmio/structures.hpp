/**
  *  \file interpreter/vmio/structures.hpp
  *  \brief interpreter::vmio Structures
  */
#ifndef C2NG_INTERPRETER_VMIO_STRUCTURES_HPP
#define C2NG_INTERPRETER_VMIO_STRUCTURES_HPP

#include "afl/base/staticassert.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "interpreter/process.hpp"

namespace interpreter { namespace vmio { namespace structures {

    /*
     *  Data Types
     */

    /** Type descriptor for Process::ProcessKind. */
    struct ProcessKind {
        typedef uint8_t Bytes_t[1];
        typedef interpreter::Process::ProcessKind Word_t;

        static Word_t unpack(const Bytes_t& bytes);
        static void pack(Bytes_t& bytes, Word_t word);
    };

    /** Representation type for 16-bit integer. */
    typedef afl::bits::Value<afl::bits::UInt16LE> UInt16_t;

    /** Representation type for 32-bit integer. */
    typedef afl::bits::Value<afl::bits::UInt32LE> UInt32_t;

    /** Representation type for Process::ProcessKind. */
    typedef afl::bits::Value<ProcessKind> ProcessKind_t;


    /*
     *  VM File Format
     */

    /** VM File Format: Object Types. */
    const uint32_t otyp_Bytecode        = 1;    ///< Bytecode object (procedure).
    const uint32_t otyp_Process         = 2;    ///< Process object.
    const uint32_t otyp_Frame           = 3;    ///< Stack frame object.
    const uint32_t otyp_DataArray       = 4;    ///< Data object: array.
    const uint32_t otyp_DataHash        = 5;    ///< Data object: hash.
    const uint32_t otyp_DataStructValue = 6;    ///< Data object: structure value.
    const uint32_t otyp_DataStructType  = 7;    ///< Data object: structure type.

    /** Tag in serialized data segment.
        \see interpreter::TagNode */
    struct Tag {
        afl::bits::Value<afl::bits::UInt16LE> packedTag;
        afl::bits::Value<afl::bits::UInt32LE> packedValue;
    };
    static_assert(sizeof(Tag) == 6, "sizeof Tag");

    /** Header property for BCO. */
    struct BCOHeader {
        UInt16_t flags;
        UInt16_t minArgs;
        UInt16_t maxArgs;
        UInt16_t numLabels;

        static const uint16_t ProcedureFlag = 1;  ///< ex bcofl_Procedure; Set if this is a procedure, clear if it is a function.
        static const uint16_t VarargsFlag = 2;    ///< ex bcofl_Varargs; Set if this routine takes a variable number of arguments.
    };
    static_assert(sizeof(BCOHeader) == 8, "sizeof BCOHeader");

    /** Header property for Frame. */
    struct FrameHeader {
        UInt32_t bcoRef;
        UInt32_t pc;
        UInt32_t contextSP;
        UInt32_t exceptionSP;
        UInt32_t flags;

        static const uint32_t WantResult = 1;     ///< ex frfl_WantResult; set if caller wants a result
    };
    static_assert(sizeof(FrameHeader) == 20, "sizeof FrameHeader");

    /** Header property for Process. */
    struct ProcessHeader {
        uint8_t priority;
        ProcessKind_t kind;
        UInt16_t contextTOS;
    };
    static_assert(sizeof(ProcessHeader) == 4, "sizeof ProcessHeader");

    /** Object header for an object pool file. */
    struct ObjectHeader {
        UInt32_t type;
        UInt32_t id;
        UInt32_t size;
        UInt32_t numProperties;
    };
    static_assert(sizeof(ObjectHeader) == 16, "sizeof ObjectHeader");

    /** Object file header. */
    struct ObjectFileHeader {
        char magic[6];                           ///< Magic number.
        uint8_t version;                         ///< Version (=OBJECT_FILE_VERSION).
        uint8_t zero;                            ///< Zero for now.
        UInt16_t headerSize;                     ///< Size of following header (=OBJECT_FILE_HEADER_SIZE).
        UInt32_t entry;                          ///< ID of entry point (=ID of a BCO).
    };
    static_assert(sizeof(ObjectFileHeader) == 14, "sizeof ObjectFileHeader");

    const char OBJECT_FILE_MAGIC[6]       = {'C','C','o','b','j',26};
    const uint8_t OBJECT_FILE_VERSION     = 100;
    const uint8_t OBJECT_FILE_HEADER_SIZE = 4;

} } }

#endif
