/**
  *  \file u/t_server_file_racenames.cpp
  *  \brief Test for server::file::RaceNames
  */

#include "server/file/racenames.hpp"

#include "t_server_file.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"
#include "u/files.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "afl/io/constmemorystream.hpp"

/** Various tests. */
void
TestServerFileRaceNames::testIt()
{
    // Codepage for everything
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);

    // Standard case for buffer version
    {
        server::file::RaceNames_t result;
        server::file::loadRaceNames(result, getDefaultRaceNames(), cs);
        TS_ASSERT_EQUALS(result.get(1), "The Solar Federation");
        TS_ASSERT_EQUALS(result.get(7), "The Crystal Confederation");
        TS_ASSERT_EQUALS(result.get(11), "The Missing Colonies of Man");
    }

    // Error case for buffer version
    {
        server::file::RaceNames_t result;
        TS_ASSERT_THROWS(server::file::loadRaceNames(result, afl::base::Nothing, cs), afl::except::FileTooShortException);
    }

    // Standard case for directory version
    {
        server::file::RaceNames_t result;
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("x");
        dir->addStream("race.nm", *new afl::io::ConstMemoryStream(getDefaultRaceNames()));
        server::file::loadRaceNames(result, *dir, cs);
        TS_ASSERT_EQUALS(result.get(1), "The Solar Federation");
        TS_ASSERT_EQUALS(result.get(7), "The Crystal Confederation");
        TS_ASSERT_EQUALS(result.get(11), "The Missing Colonies of Man");
    }

    // Error-file case for directory version
    {
        server::file::RaceNames_t result;
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("x");
        dir->addStream("race.nm", *new afl::io::ConstMemoryStream(afl::base::Nothing));
        TS_ASSERT_THROWS(server::file::loadRaceNames(result, *dir, cs), afl::except::FileTooShortException);
    }

    // Missing-file case for directory version
    {
        server::file::RaceNames_t result;
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("x");
        TS_ASSERT_THROWS(server::file::loadRaceNames(result, *dir, cs), afl::except::FileProblemException);
    }
}
