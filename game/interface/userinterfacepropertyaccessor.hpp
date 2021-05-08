/**
  *  \file game/interface/userinterfacepropertyaccessor.hpp
  */
#ifndef C2NG_GAME_INTERFACE_USERINTERFACEPROPERTYACCESSOR_HPP
#define C2NG_GAME_INTERFACE_USERINTERFACEPROPERTYACCESSOR_HPP

#include <memory>
#include "afl/base/deletable.hpp"
#include "afl/data/value.hpp"
#include "game/interface/userinterfaceproperty.hpp"

namespace game { namespace interface {

    class UserInterfacePropertyAccessor : public afl::base::Deletable {
     public:
        virtual bool get(UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result) = 0;
        virtual bool set(UserInterfaceProperty prop, const afl::data::Value* p) = 0;
    };

} }

#endif
