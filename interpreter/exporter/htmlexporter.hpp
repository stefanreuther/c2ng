/**
  *  \file interpreter/exporter/htmlexporter.hpp
  *  \brief Class interpreter::exporter::HtmlExporter
  */
#ifndef C2NG_INTERPRETER_EXPORTER_HTMLEXPORTER_HPP
#define C2NG_INTERPRETER_EXPORTER_HTMLEXPORTER_HPP

#include "afl/io/textwriter.hpp"
#include "interpreter/exporter/exporter.hpp"

namespace interpreter { namespace exporter {

    /** Export HTML table. */
    class HtmlExporter : public Exporter {
     public:
        /** Constructor.
            \param file Target file. Note that the HTML only uses US-ASCII; Unicode characters are always escaped.
                        Therefore, the character encoding of this text file is not relevant. */
        explicit HtmlExporter(afl::io::TextWriter& file);

        // Exporter methods:
        virtual void startTable(const FieldList& fields, afl::base::Memory<const TypeHint> types);
        virtual void startRecord();
        virtual void addField(afl::data::Value* value, const String_t& name, TypeHint type);
        virtual void endRecord();
        virtual void endTable();

     private:
        afl::io::TextWriter& m_file;       ///< Text file to write to.

        void writeTag(const char* tagName, const String_t& content);
    };

} }

#endif
