/**
  *  \file server/interface/filebase.cpp
  *  \brief Interface server::interface::FileBase
  */

#include <memory>
#include <stdexcept>
#include "server/interface/filebase.hpp"
#include "server/types.hpp"
#include "afl/data/stringvalue.hpp"


int32_t
server::interface::FileBase::getDirectoryIntegerProperty(String_t dirName, String_t propName)
{
    // ex FileClient::getIntProperty (sort-of)
    afl::data::StringValue sv(getDirectoryProperty(dirName, propName));
    return toInteger(&sv);
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
