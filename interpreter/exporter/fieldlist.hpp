/**
  *  \file interpreter/exporter/fieldlist.hpp
  */
#ifndef C2NG_INTERPRETER_EXPORTER_FIELDLIST_HPP
#define C2NG_INTERPRETER_EXPORTER_FIELDLIST_HPP

#include "afl/string/string.hpp"

namespace interpreter { namespace exporter {

    /** Export field list.
        This defines the user's setup of an export layout. */
    class FieldList {
     public:
        typedef size_t Index_t;

        FieldList();
        ~FieldList();

        void addList(String_t spec);
        void add(String_t spec);
        void add(Index_t line, String_t name, int width);
        void swap(Index_t a, Index_t b);
        void remove(Index_t line);

        void setFieldName(Index_t line, String_t name);
        void setFieldWidth(Index_t line, int width);
        bool getField(Index_t line, String_t& name, int& width) const;
        const String_t& getFieldName(Index_t line) const;
        int getFieldWidth(Index_t line) const;

        Index_t size() const;

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
