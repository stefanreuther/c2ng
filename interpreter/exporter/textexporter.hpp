/**
  *  \file interpreter/exporter/textexporter.hpp
  *  \brief Class interpreter::exporter::TextExporter
  */
#ifndef C2NG_INTERPRETER_EXPORTER_TEXTEXPORTER_HPP
#define C2NG_INTERPRETER_EXPORTER_TEXTEXPORTER_HPP

#include "interpreter/exporter/exporter.hpp"
#include "afl/io/textwriter.hpp"

namespace interpreter { namespace exporter {

    /** Export text table.
        This builds a table using ASCII characters.
        It implements the "text" and "table" output formats. */
    class TextExporter : public Exporter {
     public:
        /** Constructor.
            \param file output file
            \param boxes true to generate a table with borders ("table"), false to generate just a simple table ("text") */
        TextExporter(afl::io::TextWriter& file, bool boxes);

        // Exporter:
        virtual void startTable(const FieldList& fields, afl::base::Memory<const TypeHint> types);
        virtual void startRecord();
        virtual void addField(afl::data::Value* value, const String_t& name, TypeHint type);
        virtual void endRecord();
        virtual void endTable();

     private:
        afl::io::TextWriter& m_file;       ///< Text file to write to.
        const bool m_boxes;                ///< true to create boxy table, false to create plain table.

        String_t m_line;                   ///< Current table line.
        size_t m_fieldNumber;              ///< Next field number.
        std::vector<std::size_t> m_widths; ///< Field widths.
        std::vector<int> m_alignments;     ///< Field alignments.
        size_t m_totalWidth;               ///< Total width of table.
        size_t m_lineNr;                   ///< Current line number.

        void startLine();
        void addValue(String_t value, bool left);
        void endLine();
        void writeDivider();
    };

} }

#endif
