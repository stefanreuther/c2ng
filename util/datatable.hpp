/**
  *  \file util/datatable.hpp
  *  \brief Class util::DataTable
  */
#ifndef C2NG_UTIL_DATATABLE_HPP
#define C2NG_UTIL_DATATABLE_HPP

#include "afl/base/inlineoptional.hpp"
#include "afl/base/memory.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/functional/binaryfunction.hpp"
#include "afl/string/string.hpp"
#include "util/range.hpp"
#include "util/vector.hpp"

namespace util {

    /** Table of data values.
        Stores a number of rows containing a list of values.
        Each row has a (not necessarily unique) Id to find it.

        Values in each row are of type Value_t and can therefore be unset.
        Indexes into a row (=column numbers) are integers starting at 0.

        In addition to row data, a DataTable can contain names for the columns. */
    class DataTable {
     public:
        /** Type for value. */
        typedef afl::base::InlineOptional<int32_t,0x80000000> Value_t;

        /** Row.
            Rows are created using DataTable::addRow(); users cannot create rows on their own. */
        class Row {
         public:
            /** Set single value.
                \param column Column number (0...)
                \param value Value */
            void set(int column, Value_t value);

            /** Set list of values.
                \param column Starting column number (0...)
                \param values Values to store in this column and following */
            void set(int column, afl::base::Memory<const int32_t> values);

            /** Set list of values.
                \param column Starting row number (0...)
                \param values Values to store in this column and following */
            void set(int column, afl::base::Memory<const Value_t> values);

            /** Get value.
                \param column Column number (0...)
                \return value; Nothing if not set */
            Value_t get(int column) const;

            /** Get range of values.
                Determines minimum/maximum of the values.
                \return range containing all values */
            Range<int32_t> getValueRange() const;

            /** Get number of columns in this row.
                \return number such that all known values have a strictly lower column number */
            int getNumColumns() const;

            /** Set name of this row.
                \param name New name */
            void setName(String_t name);

            /** Get name of this row.
                \return name */
            String_t getName() const;

            /** Combine by adding.
                Adds each value of the other row to the value in the same column of this one.
                If either value is unset, treats it as 0;
                only if both values are unset, the resulting value is unset.
                \param other Other row */
            void add(const Row& other);

            /** Combine by multiplying/adding.
                Adds each value of the other row multiplied by the scale factor to the value in the same column of this one.
                If either value is unset, treats it as 0;
                only if both values are unset, the resulting value is unset.
                \param other Other row */
            void add(int scale, const Row& other);

            /** Get Id.
                \return Id used for creating this row. */
            int getId() const;

            /** Get index.
                \return index such that parent's getRow(index) returns this row */
            size_t getIndex() const;

         private:
            friend class DataTable;
            Row(int id, size_t index);

            const int m_id;
            size_t m_index;                   // Must be modifiable for appendMove
            Vector<Value_t,int> m_values;
            String_t m_name;
        };


        /** Constructor.
            Makes an empty table. */
        DataTable();

        /** Destructor. */
        ~DataTable();

        /** Add a row.
            \param id Id to use (need not be unique)
            \return Newly-created row */
        Row& addRow(int id);

        /** Find first row with a given Id.
            \param id Id
            \return Row if any; null if none found */
        Row* findRowById(int id);

        /** Find first row with a given Id (const overload).
            \param id Id
            \return Row if any; null if none found */
        const Row* findRowById(int id) const;

        /** Find next row with same Id.
            \param p Previously-found row
            \return Next row with same Id if any; null if none found or p is null. */
        Row* findNextRowById(const Row* p);

        /** Find next row with same Id (const overload).
            \param p Previously-found row
            \return Next row with same Id if any; null if none found or p is null. */
        const Row* findNextRowById(const Row* p) const;

        /** Get row by index.
            \param index Index [0,getNumRows())
            \return Row; null if index out of range */
        Row* getRow(size_t index);

        /** Get row by index (const overload).
            \param index Index [0,getNumRows())
            \return Row; null if index out of range */
        const Row* getRow(size_t index) const;

        /** Get number of rows.
            \return Number of rows */
        size_t getNumRows() const;

        /** Set column name.
            \param column Column number (0...)
            \param name Column name */
        void setColumnName(int column, String_t name);

        /** Get column name.
            \param column Column number (0...)
            \return Column name, empty if none set */
        String_t getColumnName(int column) const;

        /** Get range of values.
            \return range containing all values of all rows/columns */
        Range<int32_t> getValueRange() const;

        /** Get maximum column number.
            \return number such that all known values have a strictly lower column number */
        int getNumColumns() const;

        /** Stack rows atop each other to build a stacked area chart.
            Adds values such that
            - second row contains the sum of first+second
            - third row contains the sum of first+second+third
            etc.
            Modifies this DataTable in-place. */
        void stack();

        /** Append other DataTable by copying.
            Copies the other table's values and creates new rows.
            \param other Other table */
        void appendCopy(const DataTable& other);

        /** Append other DataTable by moving.
            Moves the other table's rows here and removes them from the other table.
            The other table will be empty afterwards.
            \param other Other table */
        void appendMove(DataTable& other);

        /** Copy column names from another DataTable.
            \param other Other table */
        void copyColumnNames(const DataTable& other);

        /** Add values from another table, multiplying by a scale factor.
            Rows are matched by index, not Id.
            Only rows present in both tables are processed:
            - if other has fewer rows than this one, the excess in this are left unprocessed (not modified)
            - if other has more rows than this one, the excess in other are left unprocessed (no new rows created)
            \param scale Scale factor
            \param other Other table */
        void add(int scale, const DataTable& other);

        /** Sort rows by predicate.
            Rearranges the columns without changing their content.
            \param fcn Comparison predicate */
        void sortRows(const afl::functional::BinaryFunction<const Row&, const Row&, bool>& fcn);

     private:
        afl::container::PtrVector<Row> m_rows;
        Vector<String_t,int> m_columnNames;

        Row* findNextRowById(size_t pos, int id) const;
    };

}

#endif
