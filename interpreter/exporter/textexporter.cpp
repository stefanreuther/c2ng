/**
  *  \file interpreter/exporter/textexporter.cpp
  *  \brief Class interpreter::exporter::TextExporter
  *
  *  PCC2 Comment:
  *
  *  This implements the 'text' and 'table' output formats.
  *  It accumulates a complete line at a time, then trims
  *  whitespace, and outputs the result.
  *
  *  Modelled partially after PCC 1.x export.pas::CTextExporter
  *  (PCC 1.x doesn't have 'text', only 'table').
  */

#include "interpreter/exporter/textexporter.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/values.hpp"

namespace {
    /** Get default width of a particular column type. */
    size_t getDefaultWidth(interpreter::TypeHint th)
    {
        switch (th) {
         case interpreter::thBool:
            return 3;
         case interpreter::thInt:
            return 10;
         case interpreter::thFloat:
            return 10;
         case interpreter::thString:
            return 30;
         case interpreter::thNone:
            return 100;
         default:
            return 30;
        }
    }
}

interpreter::exporter::TextExporter::TextExporter(afl::io::TextWriter& file, bool boxes)
    : m_file(file),
      m_boxes(boxes),
      m_line(),
      m_fieldNumber(0),
      m_widths(),
      m_totalWidth(0),
      m_lineNr()
{ }

void
interpreter::exporter::TextExporter::startTable(const FieldList& fields, afl::base::Memory<const TypeHint> types)
{
    // ex IntTextExporter::startTable
    // Populate widths
    m_totalWidth = 0;
    for (FieldList::Index_t i = 0; i < fields.size(); ++i) {
        size_t ele;
        if (fields.getFieldWidth(i) != 0) {
            ele = std::abs(fields.getFieldWidth(i));
        } else if (const TypeHint* pth = types.at(i)) {
            ele = getDefaultWidth(*pth);
        } else {
            ele = getDefaultWidth(thNone);
        }
        m_totalWidth += ele;
        m_widths.push_back(ele);
    }

    // Write headings
    startLine();
    for (FieldList::Index_t i = 0; i < fields.size(); ++i) {
        const TypeHint* pth = types.at(i);
        addValue(fields.getFieldName(i), pth == 0 || (*pth != thInt && *pth != thFloat));
    }
    endLine();
    m_lineNr = 0;
}

void
interpreter::exporter::TextExporter::startRecord()
{
    // ex IntTextExporter::startRecord
    // Divider
    if (m_lineNr == 0 || (m_boxes && (m_lineNr % 10 == 0))) {
        writeDivider();
    }
    ++m_lineNr;

    startLine();
}

void
interpreter::exporter::TextExporter::addField(afl::data::Value* value, const String_t& /*name*/, TypeHint /*type*/)
{
    // ex IntTextExporter::addField
    bool left = dynamic_cast<afl::data::IntegerValue*>(value) == 0 && dynamic_cast<afl::data::FloatValue*>(value) == 0;
    addValue(toString(value, false), left);
}

void
interpreter::exporter::TextExporter::endRecord()
{
    // ex IntTextExporter::endRecord
    endLine();
}

void
interpreter::exporter::TextExporter::endTable()
{
    // ex IntTextExporter::endTable
    if (m_boxes) {
        writeDivider();
    }
}


// /** Start a new data line. */
void
interpreter::exporter::TextExporter::startLine()
{
    m_line.clear();
    m_fieldNumber = 0;
    if (m_boxes) {
        m_line.append("| ");
    }
}

// /** Add a value.
//     \param value Formatted value
//     \param left true to left-justify, false to right-justify. */
void
interpreter::exporter::TextExporter::addValue(String_t value, bool left)
{
    // Remove newlines
    String_t::size_type nn = value.find('\n');
    if (nn != value.npos) {
        value.erase(nn);
        value.append("...");
    }

    // Format into field
    std::size_t n = afl::charset::Utf8().length(value);
    std::size_t width = m_widths[m_fieldNumber];
    if (n != width) {
        if (n < width) {
            if (!left) {
                m_line.append(width - n, ' ');
            }
            m_line.append(value);
            if (left) {
                m_line.append(width - n, ' ');
            }
        } else {
            if (left) {
                m_line.append(afl::charset::Utf8().substr(value, 0, width));
            } else {
                m_line.append(afl::charset::Utf8().substr(value, n - width, width));
            }
        }
    } else {
        m_line.append(value);
    }

    if (m_boxes) {
        m_line.append(" | ");
    } else {
        m_line.append(" ");
    }

    m_fieldNumber++;
}

// /** End a line. This outputs the line. */
void
interpreter::exporter::TextExporter::endLine()
{
    m_file.writeLine(afl::string::strRTrim(m_line));
}

// /** Write a divider line. */
void
interpreter::exporter::TextExporter::writeDivider()
{
    size_t total = m_totalWidth;
    if (m_boxes) {
        // 2 spaces in each field, plus one '|' per field, plus one extra '|'
        total += 3*m_widths.size() + 1;
    } else {
        // 1 extra space per field, except for last one
        total += m_widths.size() - 1;
    }
    m_file.writeLine(String_t(total, '-'));
}
