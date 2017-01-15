/**
  *  \file interpreter/tagnode.hpp
  *  \brief Struct interpreter::TagNode
  */
#ifndef C2NG_INTERPRETER_TAGNODE_HPP
#define C2NG_INTERPRETER_TAGNODE_HPP

#include "afl/base/types.hpp"

namespace interpreter {

    /** Serialized form of a value.
        This is the same structure as used in PCC 1.x.
        A serialized data segment consists of a sequence of these 48-bit tag nodes, followed by (optional) auxiliary data.
        The tag nodes contain
        - 32-bit integer or bool
        - string tag (actual string data in auxiliary data field)
        - 48-bit floating point values (Turbo Pascal REAL)
        - 32-bit floating point values (IEEE single precision)
        - other tag or Id for nonscalars or contexts (may have auxiliary data depending on type)

        Turbo Pascal has an entirely-software emulated floating point type REAL.
        Those have the convenient property of having an effective value of 0.0 if their first byte, the exponent, is zero.
        All the other bytes are irrelevant in this case
        (except for the 47th bit, the sign bit, which distinguishes positive and negative zeroes, to be precise).
        PCC 1.x therefore makes sure that actual floating-point zeroes are stored as all-48-bits-zero,
        and uses the "non-standard" zeroes to store tag/value pairs.
        The tags thus need to have zero in the lower 8 bits.

        Non-scalar values require additional data in the auxiliary data field.
        This data follows the tag nodes in sequence.
        This has the drawback that random access is not possible, just sequential reading,
        and it also means that there is no way to skip over data added by potential future tags, i.e. non-scalars after those future values are lost.
        PCC 1.x, in anticipation of PCC2, implements a little forward compatibility by reading 32-bit floats and long strings, although it never writes them.

        Although 32-bit floats were explicitly intended to be used with PCC2,
        it turned out that they have too little precision for our needs, so we don't use them as well.
        PCC2 serializes floats as 48-bit REAL. */
    struct TagNode {
        static const uint16_t Tag_FPZero     = 0x0000;       ///< Floating-point zero.
        static const uint16_t Tag_Empty      = 0x0100;       ///< EMPTY (null, unassigned, ...).
        static const uint16_t Tag_Integer    = 0x0200;       ///< Integer. value is actual value.
        static const uint16_t Tag_Boolean    = 0x0300;       ///< Boolean. value is 0 or 1.
        static const uint16_t Tag_String     = 0x0400;       ///< String. value is flag: 0=empty string, 1=Pascal string follows in auxiliary data.
        static const uint16_t Tag_32bitFP    = 0x0500;       ///< 32-bit floating point. value is IEEE single precision value. Supported since PCC 1.0.9.
        static const uint16_t Tag_LongString = 0x0600;       ///< String. value is number of bytes following in auxiliary data. Supported since PCC 1.0.18.
        static const uint16_t Tag_BCO        = 0x0700;       ///< Bytecode object. Only valid within VM files. value is the BCO's id.
        static const uint16_t Tag_Array      = 0x0800;       ///< Array. Only valid within VM files. value is array Id.
        static const uint16_t Tag_Blob       = 0x0900;       ///< Byte string. Works like Tag_LongString, but content is byte string, not text, for file I/O.
        static const uint16_t Tag_FileHandle = 0x0A00;       ///< File handle. Works like Tag_Integer, but content is file handle ("#10").
        static const uint16_t Tag_Hash       = 0x0B00;       ///< Hash. Only valid within VM files. value is hash Id.
        static const uint16_t Tag_Struct     = 0x0C00;       ///< Structure. Only valid within VM files. value is struct Id.
        static const uint16_t Tag_StructType = 0x0D00;       ///< Structure type. Only valid within VM files. value is struct type Id.

        /* Tag values for serialized contexts.
           Those must have their lower 8 bits zero.
           By convention, to avoid collision with standard (non-context) values, they also have their highest bit set. */
        static const uint16_t Tag_Ship      = 0x8000;        ///< Ship(value).
        static const uint16_t Tag_Planet    = 0x8100;        ///< Planet(value).
        static const uint16_t Tag_Minefield = 0x8200;        ///< Minefield(value).
        static const uint16_t Tag_Ion       = 0x8300;        ///< IonStorm(value).
        static const uint16_t Tag_Hull      = 0x8400;        ///< Hull(value).
        static const uint16_t Tag_Engine    = 0x8500;        ///< Engine(value).
        static const uint16_t Tag_Beam      = 0x8600;        ///< Beam(value).
        static const uint16_t Tag_Torpedo   = 0x8700;        ///< Torpedo(value).
        static const uint16_t Tag_Launcher  = 0x8800;        ///< Launcher(value).
        static const uint16_t Tag_Frame     = 0x8900;        ///< Stack frame context. Value is the age of the frame (0=oldest/bottom-most).
        static const uint16_t Tag_Global    = 0x8A00;        ///< Global context.
        static const uint16_t Tag_Mutex     = 0x8B00;        ///< Mutex. Value is the "ownership" flag, aux contains 2 uint32_t's (string lengths) followed by two strings (name and note).
        static const uint16_t Tag_Iterator  = 0x8C00;        ///< Iterator(value).
        static const uint16_t Tag_Player    = 0x8D00;        ///< Player(value).

        /** Type tag. */
        uint16_t tag;

        /** Type-dependant values. */
        uint32_t value;
    };

}

#endif
