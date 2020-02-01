/**
  *  \file game/test/root.cpp
  */

#include "game/test/root.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"

game::test::Root::Root(HostVersion host, RegistrationKey::Status status, int maxTech)
    : game::Root(afl::io::InternalDirectory::create("<empty>"),
                 *new SpecificationLoader(),
                 host,
                 std::auto_ptr<game::RegistrationKey>(new RegistrationKey(status, maxTech)),
                 std::auto_ptr<game::StringVerifier>(new StringVerifier()),
                 std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                 Actions_t())
{ }

