/**
  *  \file util/datatable.cpp
  *  \brief Class util::DataTable
  */

#include <algorithm>
#include "util/datatable.hpp"

namespace {
    /* PtrVector::sort() wants a copyable predicate */
    class SortAdaptor {
     public:
        SortAdaptor(const afl::functional::BinaryFunction<const util::DataTable::Row&, const util::DataTable::Row&, bool>& fcn)
            : m_fcn(fcn)
            { }
        bool operator()(const util::DataTable::Row& a, const util::DataTable::Row& b) const
            { return m_fcn(a, b); }
     private:
        const afl::functional::BinaryFunction<const util::DataTable::Row&, const util::DataTable::Row&, bool>& m_fcn;
    };
}

void
util::DataTable::Row::set(int column, Value_t value)
{
    m_values.set(column, value);
}

void
util::DataTable::Row::set(int column, afl::base::Memory<const int32_t> values)
{
    while (const int32_t* p = values.eat()) {
        m_values.set(column, *p);
        ++column;
    }
}

void
util::DataTable::Row::set(int column, afl::base::Memory<const Value_t> values)
{
    while (const Value_t* p = values.eat()) {
        m_values.set(column, *p);
        ++column;
    }
}

util::DataTable::Value_t
util::DataTable::Row::get(int column) const
{
    return m_values.get(column);
}

util::Range<int32_t>
util::DataTable::Row::getValueRange() const
{
    Range<int32_t> result;
    for (int i = 0, n = m_values.size(); i < n; ++i) {
        int32_t v;
        if (m_values.get(i).get(v)) {
            result.include(v);
        }
    }
    return result;
}

int
util::DataTable::Row::getNumColumns() const
{
    int result = m_values.size();
    while (result > 0 && !m_values.get(result-1).isValid()) {
        --result;
    }
    return result;
}

void
util::DataTable::Row::setName(String_t name)
{
    m_name = name;
}

String_t
util::DataTable::Row::getName() const
{
    return m_name;
}

void
util::DataTable::Row::add(const Row& other)
{
    add(1, other);
}

void
util::DataTable::Row::add(int scale, const Row& other)
{
    for (int i = 0, n = std::max(m_values.size(), other.m_values.size()); i < n; ++i) {
        int32_t meVal = 0, themVal = 0;
        bool meOK = m_values.get(i).get(meVal);
        bool themOK = other.m_values.get(i).get(themVal);
        if (meOK || themOK) {
            m_values.set(i, meVal + themVal * scale);
        }
    }
}

int
util::DataTable::Row::getId() const
{
    return m_id;
}

size_t
util::DataTable::Row::getIndex() const
{
    return m_index;
}

inline
util::DataTable::Row::Row(int id, size_t index)
    : m_id(id), m_index(index), m_values(), m_name()
{ }


/*
 *  DataTable
 */

util::DataTable::DataTable()
    : m_rows(),
      m_columnNames()
{ }

util::DataTable::~DataTable()
{ }

util::DataTable::Row&
util::DataTable::addRow(int id)
{
    size_t index = m_rows.size();
    return *m_rows.pushBackNew(new Row(id, index));
}

util::DataTable::Row*
util::DataTable::findRowById(int id)
{
    return findNextRowById(0, id);
}

const util::DataTable::Row*
util::DataTable::findRowById(int id) const
{
    return const_cast<DataTable*>(this)->findRowById(id);
}

util::DataTable::Row*
util::DataTable::findNextRowById(const Row* p)
{
    if (p != 0) {
        return findNextRowById(p->m_index + 1, p->m_id);
    } else {
        return 0;
    }
}

const util::DataTable::Row*
util::DataTable::findNextRowById(const Row* p) const
{
    return const_cast<DataTable*>(this)->findNextRowById(p);
}

util::DataTable::Row*
util::DataTable::getRow(size_t index)
{
    if (index < m_rows.size()) {
        return m_rows[index];
    } else {
        return 0;
    }
}

const util::DataTable::Row*
util::DataTable::getRow(size_t index) const
{
    return const_cast<DataTable*>(this)->getRow(index);
}

size_t
util::DataTable::getNumRows() const
{
    return m_rows.size();
}

void
util::DataTable::setColumnName(int column, String_t name)
{
    m_columnNames.set(column, name);
}

String_t
util::DataTable::getColumnName(int column) const
{
    return m_columnNames.get(column);
}

util::Range<int32_t>
util::DataTable::getValueRange() const
{
    Range<int32_t> result;
    for (size_t i = 0, n = m_rows.size(); i < n; ++i) {
        result.include(m_rows[i]->getValueRange());
    }
    return result;
}

int
util::DataTable::getNumColumns() const
{
    int result = 0;
    for (size_t i = 0, n = m_rows.size(); i < n; ++i) {
        result = std::max(result, m_rows[i]->getNumColumns());
    }
    return result;
}

void
util::DataTable::stack()
{
    for (size_t i = 1, n = m_rows.size(); i < n; ++i) {
        m_rows[i]->add(*m_rows[i-1]);
    }
}

void
util::DataTable::appendCopy(const DataTable& other)
{
    for (size_t i = 0, n = other.m_rows.size(); i < n; ++i) {
        Row& c = addRow(other.m_rows[i]->m_id);
        c.m_values = other.m_rows[i]->m_values;
        c.m_name = other.m_rows[i]->m_name;
    }
}

void
util::DataTable::appendMove(DataTable& other)
{
    try {
        for (size_t i = 0, n = other.m_rows.size(); i < n; ++i) {
            size_t index = m_rows.size();
            m_rows.pushBackNew(other.m_rows.extractElement(i))
                ->m_index = index;
        }
        other.m_rows.clear();
    }
    catch (...) {
        // At this time, other.m_rows may contain null elements which are not allowed.
        // Discard it entirely to keep the object's invariant.
        other.m_rows.clear();
        throw;
    }
}

void
util::DataTable::copyColumnNames(const DataTable& other)
{
    m_columnNames = other.m_columnNames;
}

void
util::DataTable::add(int scale, const DataTable& other)
{
    for (size_t i = 0, n = std::min(m_rows.size(), other.m_rows.size()); i < n; ++i) {
        m_rows[i]->add(scale, *other.m_rows[i]);
    }
}

void
util::DataTable::sortRows(const afl::functional::BinaryFunction<const Row&, const Row&, bool>& fcn)
{
    // Sort
    m_rows.sort(SortAdaptor(fcn));

    // Renumber
    for (size_t i = 0, n = m_rows.size(); i < n; ++i) {
        m_rows[i]->m_index = i;
    }
}

util::DataTable::Row*
util::DataTable::findNextRowById(size_t pos, int id) const
{
    while (pos < m_rows.size()) {
        if (m_rows[pos]->m_id == id) {
            return m_rows[pos];
        }
        ++pos;
    }
    return 0;
}
