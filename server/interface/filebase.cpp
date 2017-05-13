/**
  *  \file server/interface/filebase.cpp
  */

#include <memory>
#include "server/interface/filebase.hpp"
#include "server/types.hpp"


int32_t
server::interface::FileBase::getDirectoryIntegerProperty(String_t dirName, String_t propName)
{
    // ex FileClient::getIntProperty (sort-of)
    std::auto_ptr<afl::data::Value> p(getDirectoryProperty(dirName, propName));
    return toInteger(p.get());
}

String_t
server::interface::FileBase::getDirectoryStringProperty(String_t dirName, String_t propName)
{
    // ex FileClient::getStringProperty (sort-of)
    std::auto_ptr<afl::data::Value> p(getDirectoryProperty(dirName, propName));
    return toString(p.get());
}
