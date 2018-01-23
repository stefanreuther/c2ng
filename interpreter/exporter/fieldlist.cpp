/**
  *  \file interpreter/exporter/fieldlist.cpp
  */

#include <algorithm>
#include "interpreter/exporter/fieldlist.hpp"
#include "afl/string/parse.hpp"
#include "interpreter/error.hpp"
#include "afl/string/format.hpp"
#include "interpreter/tokenizer.hpp"
#include "afl/string/char.hpp"

interpreter::exporter::FieldList::FieldList()
    : m_items()
{ }

interpreter::exporter::FieldList::~FieldList()
{ }

/** Add list of fields.
    \param spec Comma-separated list of field specifications
    \throw IntError on error */
void
interpreter::exporter::FieldList::addList(String_t spec)
{
    // ex IntExportFieldList::addList
    String_t::size_type n = 0;
    String_t::size_type i = spec.find(',');
    while (i != spec.npos) {
        add(afl::string::strTrim(String_t(spec, n, i-n)));
        n = i+1;
        i = spec.find(',', n);
    }
    add(afl::string::strTrim(String_t(spec, n)));
}

/** Add field.
    \param spec Field definition (field name, optionally with '@' and width)
    \throw IntError on error */
void
interpreter::exporter::FieldList::add(String_t spec)
{
    // ex IntExportFieldList::add
    String_t::size_type m = spec.find('@');
    int width = 0;
    if (m != spec.npos) {
        if (!afl::string::strToInteger(String_t(spec, m+1), width)) {
            throw Error("Syntax error");
        }
        spec.erase(m);
    }
    const String_t s = afl::string::strUCase(afl::string::strTrim(spec));
    if (!Tokenizer::isValidUppercaseIdentifier(s)) {
        throw Error("Syntax error");
    }
    add(m_items.size(), s, width);
}

/** Add field.
    \param line Add before this line number
    \param name Name of field
    \param width Width of field (0=use default) */
void
interpreter::exporter::FieldList::add(Index_t line, String_t name, int width)
{
    m_items.insert(m_items.begin() + std::min(m_items.size(), line), Item(afl::string::strUCase(name), width));
}

/** Swap fields.
    \param a,b Positions of fields to swap */
void
interpreter::exporter::FieldList::swap(Index_t a, Index_t b)
{
    // ex IntExportFieldList::swap
    if (a < m_items.size() && b < m_items.size()) {
        iter_swap(m_items.begin()+a, m_items.begin()+b);
    }
}

/** Delete a field.
    \param line Line to delete. */
void
interpreter::exporter::FieldList::remove(Index_t line)
{
    // ex IntExportFieldList::remove
    if (line < m_items.size()) {
        m_items.erase(m_items.begin()+line);
    }        
}

/** Change name of a field.
    \param line Position of field
    \param name New name */
void
interpreter::exporter::FieldList::setFieldName(Index_t line, String_t name)
{
    // ex IntExportFieldList::setFieldName
    if (line < m_items.size()) {
        m_items[line].name = name;
    }
}

/** Change width of a field.
    \param line Position of field
    \param width New width (0=use default) */
void
interpreter::exporter::FieldList::setFieldWidth(Index_t line, int width)
{
    // ex IntExportFieldList::setFieldWidth
    if (line < m_items.size()) {
        m_items[line].width = width;
    }
}

bool
interpreter::exporter::FieldList::getField(Index_t line, String_t& name, int& width) const
{
    if (line < m_items.size()) {
        name = m_items[line].name;
        width = m_items[line].width;
        return true;
    } else {
        return false;
    }
}

/** Get name of a field.
    \param line Position of field */
const String_t&
interpreter::exporter::FieldList::getFieldName(Index_t line) const
{
    // ex IntExportFieldList::getFieldName
    // FIXME: rework to "return bool" style
    return m_items[line].name;
}

/** Get width of a field.
    \param line Position of field */
int
interpreter::exporter::FieldList::getFieldWidth(Index_t line) const
{
    // ex IntExportFieldList::getFieldWidth
    // FIXME: rework to "return bool" style
    return m_items[line].width;
}

/** Get number of fields. */
interpreter::exporter::FieldList::Index_t
interpreter::exporter::FieldList::size() const
{
    return m_items.size();
}

/** Convert field definitions to string. This string can be fed into
    addList() to restore this field list. */
String_t
interpreter::exporter::FieldList::toString() const
{
    String_t result;
    for (Index_t i = 0; i < m_items.size(); ++i) {
        if (i != 0) {
            result += ',';
        }
        result += m_items[i].name;
        if (m_items[i].width != 0) {
            result += afl::string::Format("@%d", m_items[i].width);
        }
    }
    return result;
}
