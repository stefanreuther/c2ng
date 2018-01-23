/**
  *  \file server/interface/filebase.cpp
  *  \brief Interface server::interface::FileBase
  */

#include <memory>
#include <stdexcept>
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

afl::base::Optional<String_t>
server::interface::FileBase::getFileNT(String_t fileName)
{
    try {
        return getFile(fileName);
    }
    catch (std::exception&) {
        return afl::base::Nothing;
    }
}
