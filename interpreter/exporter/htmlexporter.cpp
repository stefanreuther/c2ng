/**
  *  \file interpreter/exporter/htmlexporter.cpp
  *  \brief Class interpreter::exporter::HtmlExporter
  */

#include "interpreter/exporter/htmlexporter.hpp"
#include "afl/charset/utf8reader.hpp"
#include "afl/string/format.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/values.hpp"

interpreter::exporter::HtmlExporter::HtmlExporter(afl::io::TextWriter& file)
    : Exporter(),
      m_file(file)
{ }

void
interpreter::exporter::HtmlExporter::startTable(const FieldList& fields, afl::base::Memory<const TypeHint> /*types*/)
{
    m_file.writeLine("<!DOCTYPE html>");
    m_file.writeLine("<html>");
    m_file.writeLine(" <head>");
    m_file.writeLine("  <title>PCC2 export</title>");
    m_file.writeLine(" </head>");
    m_file.writeLine(" <body>");
    m_file.writeLine("  <table>");
    m_file.writeLine("   <tr>");
    for (FieldList::Index_t i = 0, n = fields.size(); i < n; ++i) {
        writeTag("th", fields.getFieldName(i));
    }
    m_file.writeLine("   </tr>");
}

void
interpreter::exporter::HtmlExporter::startRecord()
{
    m_file.writeLine("   <tr>");
}

void
interpreter::exporter::HtmlExporter::addField(afl::data::Value* value, const String_t& /*name*/, TypeHint /*type*/)
{
    writeTag("td", toString(value, false));
}

void
interpreter::exporter::HtmlExporter::endRecord()
{
    m_file.writeLine("   </tr>");
}

void
interpreter::exporter::HtmlExporter::endTable()
{
    m_file.writeLine("  </table>");
    m_file.writeLine(" </body>");
    m_file.writeLine("</html>");
}

void
interpreter::exporter::HtmlExporter::writeTag(const char* tagName, const String_t& content)
{
    String_t escaped;
    afl::charset::Utf8Reader rdr(afl::string::toBytes(content), 0);
    while (rdr.hasMore()) {
        afl::charset::Unichar_t ch = rdr.eat();
        if (ch == '&') {
            escaped.append("&amp;");
        } else if (ch == '<') {
            escaped.append("&lt;");
        } else if (ch == '>') {
            escaped.append("&gt;");
        } else if (ch <= 127) {
            escaped.append(1, char(ch));
        } else {
            escaped.append(afl::string::Format("&#%d;", ch));
        }
    }
    m_file.writeLine(afl::string::Format("    <%s>%s</%0$s>", tagName, escaped));
}
