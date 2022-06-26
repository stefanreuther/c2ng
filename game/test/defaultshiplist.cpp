/**
  *  \file game/test/defaultshiplist.cpp
  *  \brief Default ship list
  */

#include "game/test/defaultshiplist.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/task.hpp"
#include "game/test/files.hpp"
#include "game/test/root.hpp"
#include "game/v3/specificationloader.hpp"

using afl::base::Ref;
using afl::io::ConstMemoryStream;
using afl::io::InternalDirectory;

void
game::test::initDefaultShipList(game::spec::ShipList& list)
{
    Ref<InternalDirectory> dir(InternalDirectory::create("default"));
    dir->addStream("beamspec.dat", *new ConstMemoryStream(getDefaultBeams()));
    dir->addStream("torpspec.dat", *new ConstMemoryStream(getDefaultTorpedoes()));
    dir->addStream("engspec.dat",  *new ConstMemoryStream(getDefaultEngines()));
    dir->addStream("hullspec.dat", *new ConstMemoryStream(getDefaultHulls()));
    dir->addStream("truehull.dat", *new ConstMemoryStream(getDefaultHullAssignments()));

    Root root(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    std::auto_ptr<afl::charset::Charset> charset(new afl::charset::CodepageCharset(afl::charset::g_codepage437));
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    bool result = false;
    game::v3::SpecificationLoader(dir, charset, tx, log).loadShipList(list, root, std::auto_ptr<game::StatusTask_t>(game::makeResultTask(result)))->call();
    afl::except::checkAssertion(result, "loadDefaultShipList");
}
