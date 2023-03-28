/**
  *  \file game/interface/userinterfacepropertyaccessor.hpp
  *  \brief Interface game::interface::UserInterfaceProperty
  */
#ifndef C2NG_GAME_INTERFACE_USERINTERFACEPROPERTYACCESSOR_HPP
#define C2NG_GAME_INTERFACE_USERINTERFACEPROPERTYACCESSOR_HPP

#include <memory>
#include "afl/base/deletable.hpp"
#include "afl/data/value.hpp"
#include "game/interface/userinterfaceproperty.hpp"

namespace game { namespace interface {

    /** User interface property access.
        Interface to get and set UserInterfaceProperty values. */
    class UserInterfacePropertyAccessor : public afl::base::Deletable {
     public:
        /** Get property.
            @param [in]  prop    Property to get
            @param [out] result  Result
            @return true if property was provided, result was set; false if property was not provided, result unchanged */
        virtual bool get(UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result) = 0;

        /** Set property.
            @param [in]  prop    Property to get
            @param [in]  p       Value, owned by caller
            @return true if property was set; false if property was not set
            @throw interpreter::Error if property value is invalid */
        virtual bool set(UserInterfaceProperty prop, const afl::data::Value* p) = 0;
    };

} }

#endif
