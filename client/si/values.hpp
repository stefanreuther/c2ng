/**
  *  \file client/si/values.hpp
  */
#ifndef C2NG_CLIENT_SI_VALUES_HPP
#define C2NG_CLIENT_SI_VALUES_HPP

#include "ui/widgets/framegroup.hpp"

namespace client { namespace si {

    String_t formatFrameType(ui::FrameType type);
    
    bool parseFrameType(ui::FrameType& type, String_t value);

} }

#endif
