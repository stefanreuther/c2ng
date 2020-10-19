/**
  *  \file interpreter/exporter/separatedtextexporter.cpp
  *  \brief Class interpreter::exporter::SeparatedTextExporter
  *
  *  PCC2 comment:
  *
  *  This implements comma-separated values and derivatives.
  *
  *  Modelled partially after PCC 1.x export.pas::CTextExporter.
  */

#include "interpreter/exporter/separatedtextexporter.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/values.hpp"

interpreter::exporter::SeparatedTextExporter::SeparatedTextExporter(afl::io::TextWriter& tf, char sep)
    : m_file(tf),
      m_separator(sep),
      m_firstField(true)
{
    // ex IntSeparatedTextExporter::IntSeparatedTextExporter
}

void
interpreter::exporter::SeparatedTextExporter::startTable(const FieldList& fields, afl::base::Memory<const TypeHint> /*types*/)
{
    // ex IntSeparatedTextExporter::startTable
    /* Write a line with field names. Always quote field names.
       Otherwise, if the first field is "ID" (probably a common
       case), Excel would mistake the files as SYLK, not CSV. */
    for (FieldList::Index_t i = 0; i < fields.size(); ++i) {
        if (i != 0) {
            m_file.writeText(String_t(1, m_separator));
        }
        m_file.writeText("\"");
        m_file.writeText(fields.getFieldName(i));
        m_file.writeText("\"");
    }
    m_file.writeLine();
}

void
interpreter::exporter::SeparatedTextExporter::startRecord()
{
    // ex IntSeparatedTextExporter::startRecord
    m_firstField = true;
}

void
interpreter::exporter::SeparatedTextExporter::addField(afl::data::Value* value, const String_t& /*name*/, TypeHint /*type*/)
{
    // Separator
    if (!m_firstField) {
        m_file.writeText(String_t(1, m_separator));
    }
    m_firstField = false;

    // Value. Remove newlines.
    String_t s = toString(value, false);
    String_t::size_type n = s.find('\n');
    if (n != s.npos) {
        s.erase(n);
        s.append("...");
    }

    // Quote by doubling quotes; this seems to be the standard way to quote in CSV.
    // PCC1.x quoted by using backslashes.
    if (s.find('"') != s.npos || s.find(m_separator) != s.npos) {
        for (String_t::size_type i = s.find('"'); i != s.npos; i = s.find('"', i+2)) {
            s.insert(i, 1, '"');
        }
        s.insert(0, "\"");
        s.append("\"");
    }
    m_file.writeText(s);
}

void
interpreter::exporter::SeparatedTextExporter::endRecord()
{
    // ex IntSeparatedTextExporter::endRecord
    m_file.writeLine();
}

void
interpreter::exporter::SeparatedTextExporter::endTable()
{
    // ex IntSeparatedTextExporter::endTable
}
