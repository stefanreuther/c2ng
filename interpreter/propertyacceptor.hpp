/**
  *  \file interpreter/propertyacceptor.hpp
  *  \brief Class interpreter::PropertyAcceptor
  */
#ifndef C2NG_INTERPRETER_PROPERTYACCEPTOR_HPP
#define C2NG_INTERPRETER_PROPERTYACCEPTOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/data/namemap.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/typehint.hpp"

namespace interpreter {

    /** Property acceptor.
        This interface is used by Context to provide information about its properties.
        The Context::enumProperties() method calls addProperty() for each property. */
    class PropertyAcceptor : public afl::base::Deletable {
     public:
        /** Add property.
            \param name Name of property
            \param th Type hint */
        virtual void addProperty(const String_t& name, TypeHint th) = 0;

        /** Utility function: enumerate a NameMap object.
            Calls addProperty() for each name from the NameMap.
            \param names Name list to enumerate */
        void enumNames(const afl::data::NameMap& names);

        /** Utility function: enumerate a NameTable array.
            Calls addProperty() for each name from the NameTable array.
            \param tab array */
        void enumTable(afl::base::Memory<const NameTable> tab);
    };

}

#endif
