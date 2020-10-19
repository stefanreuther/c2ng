/**
  *  \file interpreter/exporter/fieldlist.hpp
  *  \brief Class interpreter::exporter::FieldList
  */
#ifndef C2NG_INTERPRETER_EXPORTER_FIELDLIST_HPP
#define C2NG_INTERPRETER_EXPORTER_FIELDLIST_HPP

#include <vector>
#include "afl/string/string.hpp"

namespace interpreter { namespace exporter {

    /** Export field list.
        This defines the user's setup of an export layout.
        A field list consists of a number of fields,
        each specified by a field name (upper-case identifier) and width. */
    class FieldList {
     public:
        typedef size_t Index_t;

        /** Constructor.
            Make empty list. */
        FieldList();

        /** Destructor. */
        ~FieldList();

        /** Add list of fields.
            This function is intended to process user input and therefore verifies it.
            \param spec Comma-separated list of field specifications
            \throw Error on error (invalid identifier, invalid width given)
            \see add() */
        void addList(String_t spec);

        /** Add field.
            This function is intended to process user input and therefore verifies it.
            \param spec Field definition (field name, optionally with '@' and width)
            \throw Error on error (invalid identifier, invalid width given) */
        void add(String_t spec);

        /** Add field.
            This function is not intended to process user input and therefore doesn't verify it.
            \param index Add before this index (0=as new first, size()=as new last)
            \param name Name of field
            \param width Width of field (0=use default) */
        void add(Index_t index, String_t name, int width);

        /** Swap fields.
            \param a,b Positions of fields to swap [0,size()) */
        void swap(Index_t a, Index_t b);

        /** Delete a field.
            \param index Index to delete [0,size()) */
        void remove(Index_t index);

        /** Change field name.
            \param index Position of field [0,size())
            \param name New field name */
        void setFieldName(Index_t index, String_t name);

        /** Change width of a field.
            \param index Position of field [0,size())
            \param width New width (0=use default) */
        void setFieldWidth(Index_t index, int width);

        /** Get field by index.
            \param [in] index Position of field [0,size())
            \param [out] name Name returned here
            \param [out] width Width returned here
            \retval true sucess; index was valid, outputs set
            \retval false failure; index was invalid */
        bool getField(Index_t index, String_t& name, int& width) const;

        /** Get field name.
            \param [in] index Position of field [0,size())
            \return Field name (empty if index out of range) */
        String_t getFieldName(Index_t index) const;

        /** Get field width.
            \param [in] index Position of field [0,size())
            \return Field width (0 if index out of range) */
        int getFieldWidth(Index_t index) const;

        /** Get number of fields.
            \return Number of fields */
        Index_t size() const;

        /** Convert field definitions to string.
            This string can be fed into addList() to restore this field list.
            \return string */
        String_t toString() const;

     private:
        struct Item {
            String_t name;
            int width;
            Item(const String_t& name, int width)
                : name(name),
                  width(width)
                { }
        };
        std::vector<Item> m_items;
    };

} }

#endif
