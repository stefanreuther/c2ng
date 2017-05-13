/**
  *  \file server/file/item.hpp
  */
#ifndef C2NG_SERVER_FILE_ITEM_HPP
#define C2NG_SERVER_FILE_ITEM_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace server { namespace file {

    class Item : public afl::base::Deletable {
     public:
        Item(String_t name);
        ~Item();
        const String_t& getName() const;
     private:
        String_t m_name;
    };

} }

#endif
