/**
  *  \file interpreter/exporter/fieldlist.cpp
  *  \brief Class interpreter::exporter::FieldList
  */

#include <algorithm>
#include "interpreter/exporter/fieldlist.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tokenizer.hpp"

// Constructor.
interpreter::exporter::FieldList::FieldList()
    : m_items()
{ }

// Destructor.
interpreter::exporter::FieldList::~FieldList()
{ }

// Add list of fields.
void
interpreter::exporter::FieldList::addList(String_t spec)
{
    // ex IntExportFieldList::addList
    // ex export.pas:StringToFieldList
    String_t::size_type n = 0;
    String_t::size_type i = spec.find(',');
    while (i != spec.npos) {
        add(afl::string::strTrim(String_t(spec, n, i-n)));
        n = i+1;
        i = spec.find(',', n);
    }
    add(afl::string::strTrim(String_t(spec, n)));
}

// Add field.
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

// Add field.
void
interpreter::exporter::FieldList::add(Index_t index, String_t name, int width)
{
    m_items.insert(m_items.begin() + std::min(m_items.size(), index), Item(afl::string::strUCase(name), width));
}

// Swap fields.
void
interpreter::exporter::FieldList::swap(Index_t a, Index_t b)
{
    // ex IntExportFieldList::swap
    if (a < m_items.size() && b < m_items.size()) {
        iter_swap(m_items.begin()+a, m_items.begin()+b);
    }
}

// Delete a field.
void
interpreter::exporter::FieldList::remove(Index_t index)
{
    // ex IntExportFieldList::remove
    if (index < m_items.size()) {
        m_items.erase(m_items.begin()+index);
    }
}

// Clear the list.
void
interpreter::exporter::FieldList::clear()
{
    m_items.clear();
}

// Change field name.
void
interpreter::exporter::FieldList::setFieldName(Index_t index, String_t name)
{
    // ex IntExportFieldList::setFieldName
    if (index < m_items.size()) {
        m_items[index].name = afl::string::strUCase(name);
    }
}

// Change width of a field.
void
interpreter::exporter::FieldList::setFieldWidth(Index_t index, int width)
{
    // ex IntExportFieldList::setFieldWidth
    if (index < m_items.size()) {
        m_items[index].width = width;
    }
}

// Change width of a field, relative.
void
interpreter::exporter::FieldList::changeFieldWidth(Index_t index, int delta)
{
    // ex WExportFieldList::changeFieldWidth
    if (index < m_items.size()) {
        /* New field width. Clip into range [-999,+999]. When we pass 0, set it to 0 first. */
        Item& it = m_items[index];
        int ofw = it.width;
        int nfw = std::max(-999, std::min(+999, ofw + delta));
        if (ofw != 0 && (ofw < 0) != (nfw < 0)) {
            nfw = 0;
        }
        it.width = nfw;
    }
}

// Toggle field's alignment.
void
interpreter::exporter::FieldList::toggleFieldAlignment(Index_t index)
{
    if (index < m_items.size()) {
        Item& it = m_items[index];
        it.width = -it.width;
    }
}

// Get field by index.
bool
interpreter::exporter::FieldList::getField(Index_t index, String_t& name, int& width) const
{
    if (index < m_items.size()) {
        name = m_items[index].name;
        width = m_items[index].width;
        return true;
    } else {
        return false;
    }
}

// Get field name.
String_t
interpreter::exporter::FieldList::getFieldName(Index_t index) const
{
    // ex IntExportFieldList::getFieldName
    if (index < m_items.size()) {
        return m_items[index].name;
    } else {
        return String_t();
    }
}

// Get field width.
int
interpreter::exporter::FieldList::getFieldWidth(Index_t index) const
{
    // ex IntExportFieldList::getFieldWidth
    if (index < m_items.size()) {
        return m_items[index].width;
    } else {
        return 0;
    }
}

// Get number of fields.
interpreter::exporter::FieldList::Index_t
interpreter::exporter::FieldList::size() const
{
    return m_items.size();
}

// Convert field definitions to string.
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
