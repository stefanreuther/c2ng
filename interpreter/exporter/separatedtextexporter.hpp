/**
  *  \file interpreter/exporter/separatedtextexporter.hpp
  */
#ifndef C2NG_INTERPRETER_EXPORTER_SEPARATEDTEXTEXPORTER_HPP
#define C2NG_INTERPRETER_EXPORTER_SEPARATEDTEXTEXPORTER_HPP

#include "interpreter/exporter/exporter.hpp"
#include "afl/io/textwriter.hpp"

namespace interpreter { namespace exporter {

    /** Export separated text.
        Writes simple plaintext files with a configurable field separator.
        The first line contains field names.
        This can be easily imported into spreadsheets. */
    class SeparatedTextExporter : public Exporter {
     public:
        SeparatedTextExporter(afl::io::TextWriter& tf, char sep);

        // Exporter:
        virtual void startTable(const FieldList& fields, afl::base::Memory<const TypeHint> types);
        virtual void startRecord();
        virtual void addField(afl::data::Value* value, const String_t& name, TypeHint type);
        virtual void endRecord();
        virtual void endTable();

     private:
        afl::io::TextWriter& m_file;
        char m_separator;
        bool m_firstField;
    };

} }

#endif
