/**
  *  \file server/file/item.hpp
  *  \brief Class server::file::Item
  */
#ifndef C2NG_SERVER_FILE_ITEM_HPP
#define C2NG_SERVER_FILE_ITEM_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace server { namespace file {

    /** Base class for an item in the file system. */
    class Item : public afl::base::Deletable {
     public:
        /** Constructor.
            @param name Name of this object (basename) */
        explicit Item(String_t name);

        /** Destructor. */
        ~Item();

        /** Get name.
            @return name (basename) */
        const String_t& getName() const;

     private:
        String_t m_name;
    };

} }

#endif
