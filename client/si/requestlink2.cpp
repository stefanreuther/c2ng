/**
  *  \file client/si/requestlink2.cpp
  */

#include "client/si/requestlink2.hpp"
#include "afl/string/format.hpp"

String_t
client::si::RequestLink2::toString() const
{
    if (m_isValid) {
        return afl::string::Format("RequestLink(%d)", m_pid);
    } else {
        return "RequestLink(null)";
    }
}
