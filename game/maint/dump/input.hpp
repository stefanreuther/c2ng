/**
  *  \file game/maint/dump/input.hpp
  *  \brief Class game::maint::dump::Input
  */
#ifndef C2NG_GAME_MAINT_DUMP_INPUT_HPP
#define C2NG_GAME_MAINT_DUMP_INPUT_HPP

#include "afl/base/memory.hpp"
#include "afl/charset/charset.hpp"
#include "afl/string/string.hpp"

namespace game { namespace maint { namespace dump {

    class Output;

    /** Input for dump utility.
        Provides a byte stream and methods to consume it.
        To use,
        - load a file into a Buffer
        - construct Input
        - call parser function, that calls readXXX() functions to consume data */
    class Input {
     public:
        /** Constructor.
            @param input    data block to dump
            @param charset  character set */
        Input(afl::base::ConstBytes_t input, afl::charset::Charset& charset)
            : m_input(input), m_charset(charset)
            { }

        /** Construct from other input.
            Extracts the first @c size bytes from @c other and makes it available for reading.
            @param [in,out] other   Source
            @param [in]     size    Number of bytes to consume */
        Input(Input& other, size_t size);

        /** Read unsigned byte.
            @return formatted value */
        String_t readByte();

        /** Read unsigned word (16-bit).
            @return formatted value */
        String_t readWord();

        /** Read unsigned long (32-bit).
            @return formatted value */
        String_t readLong();

        /** Read coordinate pair (2x 16 bit).
            @return formatted value */
        String_t readCoordinate();

        /** Read fixed-size string.
            @param length Number of bytes
            @return formatted value */
        String_t readString(size_t length);

        /** Read Pascal string.
            @return formatted value */
        String_t readPascalString();

        /** Get number of remaining bytes.
            @return number of bytes */
        size_t getRemainingSize() const;

        /** Read unparsed bytes.
            @param limit Number of bytes
            @return formatted value (list of hex bytes) */
        String_t readUnparsed(size_t limit);

        /** Read bytes without consuming them.
            @param mem Memory buffer
            @return number of bytes found */
        size_t peek(afl::base::Bytes_t mem);

        /** Read bytes, consuming them.
            @param mem Memory buffer
            @return number of bytes found */
        size_t read(afl::base::Bytes_t mem);

        /** Consume bytes.
            @param size Number of bytes to skip
            @return number of bytes skipped (<= size) */
        size_t skip(size_t size);

        /** Dump remainder of data to output.
            Consumes all remaining bytes.
            @param out Output */
        void dumpRemainder(Output& out);

     private:
        afl::base::ConstBytes_t m_input;
        afl::charset::Charset& m_charset;
    };

} } }

#endif
