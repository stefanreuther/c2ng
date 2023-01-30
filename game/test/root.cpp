/**
  *  \file game/test/root.cpp
  *  \brief Test Root
  */

#include "game/test/root.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"

afl::base::Ref<game::Root>
game::test::makeRoot(HostVersion host, RegistrationKey::Status status, int maxTech)
{
    return *new game::Root(afl::io::InternalDirectory::create("<empty>"),
                           *new SpecificationLoader(),
                           host,
                           std::auto_ptr<game::RegistrationKey>(new RegistrationKey(status, maxTech)),
                           std::auto_ptr<game::StringVerifier>(new StringVerifier()),
                           std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                           Root::Actions_t());
}
