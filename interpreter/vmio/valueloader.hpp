/**
  *  \file interpreter/vmio/valueloader.hpp
  *  \brief Class interpreter::vmio::ValueLoader
  */
#ifndef C2NG_INTERPRETER_VMIO_VALUELOADER_HPP
#define C2NG_INTERPRETER_VMIO_VALUELOADER_HPP

#include "afl/base/types.hpp"
#include "afl/charset/charset.hpp"
#include "afl/data/namemap.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/value.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "interpreter/tagnode.hpp"

namespace interpreter { namespace vmio {

    class LoadContext;

    /** Value loader.
        This is the core building block for loading (deserializing) individual values and data segments.

        ValueLoader itself only loads scalar data; nonscalar data is handled by a LoadContext.
        - use NullLoadContext to only load scalars.
        - use game::interface::LoadContext to load game objects.
        - use ObjectLoader to load non-scalar data; give it a WorldLoadContext to be able to load processes.

        A data segment consists of a sequence of 48-bit tag nodes, followed by the concatenated auxiliary information.
        The load() method loads such a stream, the loadValue() method builds a value from a tag and aux info. */
    class ValueLoader {
     public:
        /** Constructor.
            \param cs Character set. For game data, typically the game character set.
            \param ctx Load context to load non-scalar data
            \param tx Translator (for error messages) */
        ValueLoader(afl::charset::Charset& cs, LoadContext& ctx, afl::string::Translator& tx);

        /** Load data segment.
            \param data       [out] Segment to store data into
            \param in         [in]  Stream to read from
            \param firstIndex [in]  First slot in \c data to load from
            \param slots      [in]  Number of slots to load

            This method will modify the slots [firstIndex, firstIndex+slots) of \c data;
            all other slots will remain unchanged. */
        void load(afl::data::Segment& data, afl::io::Stream& in, size_t firstIndex, size_t slots);

        /** Load single value.
            This method is the inverse to SaveVisitor() resp. BaseValue::store().
            \param tag [in] deserialized tag
            \param aux [in] auxiliary data
            \return newly-created object. Can be null if tag describes a null value.
            \throw FileFormatException if value cannot be interpreted */
        afl::data::Value* loadValue(const TagNode& tag, afl::io::Stream& aux);

        /** Load a name table.
            \param names [out] Name table. Names are appended, so this should normally be empty
            \param in    [in]  Input stream
            \param n     [in]  Number of names to load */
        void loadNames(afl::data::NameMap& names, afl::io::Stream& in, uint32_t n);

     private:
        afl::charset::Charset& m_charset;
        LoadContext& m_context;
        afl::string::Translator& m_translator;

        String_t loadPascalString(uint32_t flag, afl::io::Stream& aux);
        String_t loadLongString(uint32_t length, afl::io::Stream& aux);
        double loadFloat(uint32_t value);
        double loadFloat48(const TagNode& tag);

        afl::data::Value* makeBlobValue(uint32_t size, afl::io::Stream& aux);
    };

} }

#endif
