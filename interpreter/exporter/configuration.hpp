/**
  *  \file interpreter/exporter/configuration.hpp
  */
#ifndef C2NG_INTERPRETER_EXPORTER_CONFIGURATION_HPP
#define C2NG_INTERPRETER_EXPORTER_CONFIGURATION_HPP

#include "util/charsetfactory.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/exporter/format.hpp"
#include "afl/io/stream.hpp"
#include "afl/charset/charset.hpp"

namespace interpreter { namespace exporter {

    class Configuration {
     public:
        Configuration();

        ~Configuration();

        void setCharsetIndex(util::CharsetFactory::Index_t index);
        void setCharsetByName(const String_t& name);
        util::CharsetFactory::Index_t getCharsetIndex() const;
        afl::charset::Charset* createCharset() const;

        void setFormat(Format fmt);
        void setFormatByName(const String_t& name);
        Format getFormat() const;

        FieldList& fieldList();
        const FieldList& fieldList() const;

        void load(afl::io::Stream& in);

     private:
        util::CharsetFactory::Index_t m_charsetIndex;
        Format m_format;
        FieldList m_fieldList;
    };

} }

#endif
