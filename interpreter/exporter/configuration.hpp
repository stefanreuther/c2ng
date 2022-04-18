/**
  *  \file interpreter/exporter/configuration.hpp
  *  \brief Class interpreter::exporter::Configuration
  */
#ifndef C2NG_INTERPRETER_EXPORTER_CONFIGURATION_HPP
#define C2NG_INTERPRETER_EXPORTER_CONFIGURATION_HPP

#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/string/translator.hpp"
#include "interpreter/context.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/exporter/format.hpp"
#include "util/charsetfactory.hpp"

namespace interpreter { namespace exporter {

    /** Configuration for Exporter. */
    class Configuration {
     public:
        /** Constructor. Set defaults. */
        Configuration();

        /** Destructor. */
        ~Configuration();

        /** Set character set by index.
            \param index Index (see util::CharsetFactory) */
        void setCharsetIndex(util::CharsetFactory::Index_t index);

        /** Set character set by name.
            \param name Name
            \param tx Translator (for exception message)
            \throw std::runtime_error Name is not a known charset */
        void setCharsetByName(const String_t& name, afl::string::Translator& tx);

        /** Get character set index.
            \return index */
        util::CharsetFactory::Index_t getCharsetIndex() const;

        /** Create configured character set.
            \return newly allocated Charset object; caller assumes ownership */
        afl::charset::Charset* createCharset() const;

        /** Set format.
            \param fmt Format */
        void setFormat(Format fmt);

        /** Set format by name.
            \param name Name
            \param tx Translator (for exception message)
            \throw std::runtime_error Name is not a known format */
        void setFormatByName(const String_t& name, afl::string::Translator& tx);

        /** Get format.
            \return format */
        Format getFormat() const;

        /** Access field list.
            \return field list. */
        FieldList& fieldList();
        const FieldList& fieldList() const;

        /** Read configuration from stream.
            \param in Stream
            \param tx Translator (for exception message)
            \throw afl::except::FileProblemException Syntax or I/O error */
        void load(afl::io::Stream& in, afl::string::Translator& tx);

        /** Write configuration to stream.
            Produces a file that load() can read to restore this state into an empty Configuration.
            \param out Stream */
        void save(afl::io::Stream& out);

        /** Perform export in a text format.
            Honors the configured format, but not the character set.
            Character set needs to be handled by the TextWriter.
            \param ctx Context looking at the first object to possibly export.
            \param out Text output stream
            \return true on success, false if requested format does not support text output */
        bool exportText(Context& ctx, afl::io::TextWriter& out) const;

        /** Perform output into a file.
            Honors all configured parameters.
            \param ctx Context looking at the first object to possibly export.
            \param out Output stream */
        void exportFile(Context& ctx, afl::io::Stream& out) const;

     private:
        util::CharsetFactory::Index_t m_charsetIndex;
        Format m_format;
        FieldList m_fieldList;
    };

} }

#endif
