/**
  *  \file server/interface/mailqueue.cpp
  */

#include "server/interface/mailqueue.hpp"

String_t
server::interface::MailQueue::formatAddressStatus(AddressStatus st)
{
    return st == NotSet
        ? String_t()
        : String_t(1, char(st));
}

server::interface::MailQueue::AddressStatus
server::interface::MailQueue::parseAddressStatus(const String_t& st)
{
    return st.empty()
        ? NotSet
        : AddressStatus(st[0]);
}
