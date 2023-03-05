/**
  *  \file interpreter/exporter/separatedtextexporter.hpp
  *  \brief Class interpreter::exporter::SeparatedTextExporter
  */
#ifndef C2NG_INTERPRETER_EXPORTER_SEPARATEDTEXTEXPORTER_HPP
#define C2NG_INTERPRETER_EXPORTER_SEPARATEDTEXTEXPORTER_HPP

#include "afl/io/textwriter.hpp"
#include "interpreter/exporter/exporter.hpp"

namespace interpreter { namespace exporter {

    /** Export separated text.
        Writes simple plaintext files with a configurable field separator.
        This implements comma-separated values and derivatives.

        The first line contains field names.
        This can be easily imported into spreadsheets. */
    class SeparatedTextExporter : public Exporter {
     public:
        /** Constructor.
            @param tf   Text file to write to
            @param sep  Field separator character (e.g. ';') */
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
