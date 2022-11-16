/**
  *  \file game/test/specificationloader.cpp
  *  \brief Class game::test::SpecificationLoader
  */

#include "game/test/specificationloader.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/messages.hpp"

std::auto_ptr<game::Task_t>
game::test::SpecificationLoader::loadShipList(game::spec::ShipList& /*list*/, game::Root& /*root*/, std::auto_ptr<StatusTask_t> then)
{
    return makeConfirmationTask(true, then);
}

afl::base::Ref<afl::io::Stream>
game::test::SpecificationLoader::openSpecificationFile(const String_t& fileName)
{
    throw afl::except::FileProblemException(fileName, afl::string::Messages::fileNotFound());
}
