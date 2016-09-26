/**
  *  \file interpreter/propertyacceptor.hpp
  */
#ifndef C2NG_INTERPRETER_PROPERTYACCEPTOR_HPP
#define C2NG_INTERPRETER_PROPERTYACCEPTOR_HPP

#include "afl/base/deletable.hpp"
#include "interpreter/typehint.hpp"
#include "afl/data/namemap.hpp"
#include "interpreter/nametable.hpp"

namespace interpreter {

    class PropertyAcceptor : public afl::base::Deletable {
     public:
        virtual void addProperty(const String_t& name, TypeHint th) = 0;

        /** Utility function: enumerate a NameMap object.
            \param names Name list to enumerate */
        void enumNames(const afl::data::NameMap& names);

        /** Utility function: enumerate a NameTable array.
            \param tab array */
        void enumTable(afl::base::Memory<const NameTable> tab);
    };

}

#endif
