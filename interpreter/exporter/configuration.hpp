/**
  *  \file interpreter/exporter/configuration.hpp
  *  \brief Class interpreter::exporter::Configuration
  */
#ifndef C2NG_INTERPRETER_EXPORTER_CONFIGURATION_HPP
#define C2NG_INTERPRETER_EXPORTER_CONFIGURATION_HPP

#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
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
            \throw std::runtime_error Name is not a known charset */
        void setCharsetByName(const String_t& name);

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
            \throw std::runtime_error Name is not a known format */
        void setFormatByName(const String_t& name);

        /** Get format.
            \return format */
        Format getFormat() const;

        /** Access field list.
            \return field list. */
        FieldList& fieldList();
        const FieldList& fieldList() const;

        /** Read configuration from stream.
            \param in Stream
            \throw afl::except::FileProblemException Syntax or I/O error */
        void load(afl::io::Stream& in);

     private:
        util::CharsetFactory::Index_t m_charsetIndex;
        Format m_format;
        FieldList m_fieldList;
    };

} }

#endif
