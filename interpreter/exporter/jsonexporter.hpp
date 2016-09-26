/**
  *  \file interpreter/exporter/jsonexporter.hpp
  */
#ifndef C2NG_INTERPRETER_EXPORTER_JSONEXPORTER_HPP
#define C2NG_INTERPRETER_EXPORTER_JSONEXPORTER_HPP

#include "interpreter/exporter/exporter.hpp"
#include "afl/io/textwriter.hpp"

namespace interpreter { namespace exporter {

    /** Export to JSON text.
        Generates output as an array of hashes. */
    class JsonExporter : public Exporter {
     public:
        JsonExporter(afl::io::TextWriter& file);
        virtual void startTable(const FieldList& fields, afl::base::Memory<const TypeHint> types);
        virtual void startRecord();
        virtual void addField(afl::data::Value* value, const String_t& name, TypeHint type);
        virtual void endRecord();
        virtual void endTable();

     private:
        afl::io::TextWriter& m_file;
        bool m_firstField;
        bool m_firstRecord;
    };

} }

#endif
