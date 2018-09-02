/**
  *  \file u/t_game_v3_resultfile.cpp
  *  \brief Test for game::v3::ResultFile
  */

#include "game/v3/resultfile.hpp"

#include "u/t_game_v3.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "u/files.hpp"

/** Test v3.0 result file. */
void
TestGameV3ResultFile::test30()
{
    afl::io::ConstMemoryStream file(getResultFile30());
    afl::string::NullTranslator tx;
    game::v3::ResultFile result(file, tx);

    // Simple queries
    TS_ASSERT(result.hasSection(result.ShipSection));
    TS_ASSERT(result.hasSection(result.TargetSection));
    TS_ASSERT(result.hasSection(result.PlanetSection));
    TS_ASSERT(result.hasSection(result.BaseSection));
    TS_ASSERT(result.hasSection(result.MessageSection));
    TS_ASSERT(result.hasSection(result.ShipXYSection));
    TS_ASSERT(result.hasSection(result.GenSection));
    TS_ASSERT(result.hasSection(result.VcrSection));
    TS_ASSERT(!result.hasSection(result.KoreSection));
    TS_ASSERT(!result.hasSection(result.LeechSection));
    TS_ASSERT(!result.hasSection(result.SkoreSection));
    TS_ASSERT_EQUALS(&result.getFile(), &file);
    TS_ASSERT_EQUALS(result.getVersion(), -1);

    // Offset queries
    afl::io::Stream::FileSize_t offset = 0;
    TS_ASSERT(result.getSectionOffset(result.ShipSection, offset));
    TS_ASSERT_EQUALS(offset, 0x0021U);
    TS_ASSERT(result.getSectionOffset(result.TargetSection, offset));
    TS_ASSERT_EQUALS(offset, 0x00F9U);
    TS_ASSERT(result.getSectionOffset(result.PlanetSection, offset));
    TS_ASSERT_EQUALS(offset, 0x00FBU);
    TS_ASSERT(result.getSectionOffset(result.BaseSection, offset));
    TS_ASSERT_EQUALS(offset, 0x01FCU);
    TS_ASSERT(result.getSectionOffset(result.MessageSection, offset));
    TS_ASSERT_EQUALS(offset, 0x029AU);
    TS_ASSERT(result.getSectionOffset(result.ShipXYSection, offset));
    TS_ASSERT_EQUALS(offset, 0x095EU);
    TS_ASSERT(result.getSectionOffset(result.GenSection, offset));
    TS_ASSERT_EQUALS(offset, 0x2896U);
    TS_ASSERT(result.getSectionOffset(result.VcrSection, offset));
    TS_ASSERT_EQUALS(offset, 0x2926U);
    TS_ASSERT(!result.getSectionOffset(result.KoreSection, offset));
    TS_ASSERT(!result.getSectionOffset(result.LeechSection, offset));
    TS_ASSERT(!result.getSectionOffset(result.SkoreSection, offset));

    // Test seeking
    TS_ASSERT_THROWS_NOTHING(result.seekToSection(result.ShipSection));
    TS_ASSERT_EQUALS(file.getPos(), 0x0021U);
    TS_ASSERT_THROWS(result.seekToSection(result.KoreSection), afl::except::FileProblemException);
}

/** Test v3.5 result file. */
void
TestGameV3ResultFile::test35()
{
    afl::io::ConstMemoryStream file(getResultFile35());
    afl::string::NullTranslator tx;
    game::v3::ResultFile result(file, tx);

    // Simple queries
    TS_ASSERT(result.hasSection(result.ShipSection));
    TS_ASSERT(result.hasSection(result.TargetSection));
    TS_ASSERT(result.hasSection(result.PlanetSection));
    TS_ASSERT(result.hasSection(result.BaseSection));
    TS_ASSERT(result.hasSection(result.MessageSection));
    TS_ASSERT(result.hasSection(result.ShipXYSection));
    TS_ASSERT(result.hasSection(result.GenSection));
    TS_ASSERT(result.hasSection(result.VcrSection));
    TS_ASSERT(result.hasSection(result.KoreSection));
    TS_ASSERT(!result.hasSection(result.LeechSection));
    TS_ASSERT(result.hasSection(result.SkoreSection));
    TS_ASSERT_EQUALS(result.getVersion(), 1);

    // Offset queries
    afl::io::Stream::FileSize_t offset = 0;
    TS_ASSERT(result.getSectionOffset(result.ShipSection, offset));
    TS_ASSERT_EQUALS(offset, 0x0060U);
    TS_ASSERT(result.getSectionOffset(result.TargetSection, offset));
    TS_ASSERT_EQUALS(offset, 0x01A3U);
    TS_ASSERT(result.getSectionOffset(result.PlanetSection, offset));
    TS_ASSERT_EQUALS(offset, 0x01A5U);
    TS_ASSERT(result.getSectionOffset(result.BaseSection, offset));
    TS_ASSERT_EQUALS(offset, 0x02FBU);
    TS_ASSERT(result.getSectionOffset(result.MessageSection, offset));
    TS_ASSERT_EQUALS(offset, 0x0399U);
    TS_ASSERT(result.getSectionOffset(result.ShipXYSection, offset));
    TS_ASSERT_EQUALS(offset, 0x0AD1U);
    TS_ASSERT(result.getSectionOffset(result.GenSection, offset));
    TS_ASSERT_EQUALS(offset, 0x2A09U);
    TS_ASSERT(result.getSectionOffset(result.VcrSection, offset));
    TS_ASSERT_EQUALS(offset, 0x2A99U);
    TS_ASSERT(result.getSectionOffset(result.KoreSection, offset));
    TS_ASSERT_EQUALS(offset, 0x2A9BU);
    TS_ASSERT(!result.getSectionOffset(result.LeechSection, offset));
    TS_ASSERT(result.getSectionOffset(result.SkoreSection, offset));
    TS_ASSERT_EQUALS(offset, 0x5E85U);

    // Test seeking
    TS_ASSERT_THROWS_NOTHING(result.seekToSection(result.ShipSection));
    TS_ASSERT_EQUALS(file.getPos(), 0x0060U);
    TS_ASSERT_THROWS(result.seekToSection(result.LeechSection), afl::except::FileProblemException);
}
