/**
  *  \file test/game/v3/resultfiletest.cpp
  *  \brief Test for game::v3::ResultFile
  */

#include "game/v3/resultfile.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"

/** Test v3.0 result file. */
AFL_TEST("game.v3.ResultFile:v30", a)
{
    afl::io::ConstMemoryStream file(game::test::getResultFile30());
    afl::string::NullTranslator tx;
    game::v3::ResultFile result(file, tx);

    // Simple queries
    a.check("01. ShipSection",     result.hasSection(result.ShipSection));
    a.check("02. TargetSection",   result.hasSection(result.TargetSection));
    a.check("03. PlanetSection",   result.hasSection(result.PlanetSection));
    a.check("04. BaseSection",     result.hasSection(result.BaseSection));
    a.check("05. MessageSection",  result.hasSection(result.MessageSection));
    a.check("06. ShipXYSection",   result.hasSection(result.ShipXYSection));
    a.check("07. GenSection",      result.hasSection(result.GenSection));
    a.check("08. VcrSection",      result.hasSection(result.VcrSection));
    a.check("09. KoreSection",    !result.hasSection(result.KoreSection));
    a.check("10. LeechSection",   !result.hasSection(result.LeechSection));
    a.check("11. SkoreSection",   !result.hasSection(result.SkoreSection));
    a.checkEqual("12. getFile",   &result.getFile(), &file);
    a.checkEqual("13. getVersion", result.getVersion(), -1);

    // Offset queries
    afl::io::Stream::FileSize_t offset = 0;
    a.check     ("21. ShipSection",    result.getSectionOffset(result.ShipSection, offset));
    a.checkEqual("22. offset",         offset, 0x0021U);
    a.check     ("23. TargetSection",  result.getSectionOffset(result.TargetSection, offset));
    a.checkEqual("24. offset",         offset, 0x00F9U);
    a.check     ("25. PlanetSection",  result.getSectionOffset(result.PlanetSection, offset));
    a.checkEqual("26. offset",         offset, 0x00FBU);
    a.check     ("27. BaseSection",    result.getSectionOffset(result.BaseSection, offset));
    a.checkEqual("28. offset",         offset, 0x01FCU);
    a.check     ("29. MessageSection", result.getSectionOffset(result.MessageSection, offset));
    a.checkEqual("30. offset",         offset, 0x029AU);
    a.check     ("31. ShipXYSection",  result.getSectionOffset(result.ShipXYSection, offset));
    a.checkEqual("32. offset",         offset, 0x095EU);
    a.check     ("33. GenSection",     result.getSectionOffset(result.GenSection, offset));
    a.checkEqual("34. offset",         offset, 0x2896U);
    a.check     ("35. VcrSection",     result.getSectionOffset(result.VcrSection, offset));
    a.checkEqual("36. offset",         offset, 0x2926U);
    a.check     ("37. KoreSection",   !result.getSectionOffset(result.KoreSection, offset));
    a.check     ("38. LeechSection",  !result.getSectionOffset(result.LeechSection, offset));
    a.check     ("39. SkoreSection",  !result.getSectionOffset(result.SkoreSection, offset));

    // Test seeking
    AFL_CHECK_SUCCEEDS(a("41. seekToSection"), result.seekToSection(result.ShipSection));
    a.checkEqual("42. getPos", file.getPos(), 0x0021U);
    AFL_CHECK_THROWS(a("43. seekToSection"), result.seekToSection(result.KoreSection), afl::except::FileProblemException);
}

/** Test v3.5 result file. */
AFL_TEST("game.v3.ResultFile:v35", a)
{
    afl::io::ConstMemoryStream file(game::test::getResultFile35());
    afl::string::NullTranslator tx;
    game::v3::ResultFile result(file, tx);

    // Simple queries
    a.check("01. ShipSection",     result.hasSection(result.ShipSection));
    a.check("02. TargetSection",   result.hasSection(result.TargetSection));
    a.check("03. PlanetSection",   result.hasSection(result.PlanetSection));
    a.check("04. BaseSection",     result.hasSection(result.BaseSection));
    a.check("05. MessageSection",  result.hasSection(result.MessageSection));
    a.check("06. ShipXYSection",   result.hasSection(result.ShipXYSection));
    a.check("07. GenSection",      result.hasSection(result.GenSection));
    a.check("08. VcrSection",      result.hasSection(result.VcrSection));
    a.check("09. KoreSection",     result.hasSection(result.KoreSection));
    a.check("10. LeechSection",   !result.hasSection(result.LeechSection));
    a.check("11. SkoreSection",    result.hasSection(result.SkoreSection));
    a.checkEqual("12. getVersion", result.getVersion(), 1);

    // Offset queries
    afl::io::Stream::FileSize_t offset = 0;
    a.check     ("21. ShipSection",    result.getSectionOffset(result.ShipSection, offset));
    a.checkEqual("22. offset",         offset, 0x0060U);
    a.check     ("23. TargetSection",  result.getSectionOffset(result.TargetSection, offset));
    a.checkEqual("24. offset",         offset, 0x01A3U);
    a.check     ("25. PlanetSection",  result.getSectionOffset(result.PlanetSection, offset));
    a.checkEqual("26. offset",         offset, 0x01A5U);
    a.check     ("27. BaseSection",    result.getSectionOffset(result.BaseSection, offset));
    a.checkEqual("28. offset",         offset, 0x02FBU);
    a.check     ("29. MessageSection", result.getSectionOffset(result.MessageSection, offset));
    a.checkEqual("30. offset",         offset, 0x0399U);
    a.check     ("31. ShipXYSection",  result.getSectionOffset(result.ShipXYSection, offset));
    a.checkEqual("32. offset",         offset, 0x0AD1U);
    a.check     ("33. GenSection",     result.getSectionOffset(result.GenSection, offset));
    a.checkEqual("34. offset",         offset, 0x2A09U);
    a.check     ("35. VcrSection",     result.getSectionOffset(result.VcrSection, offset));
    a.checkEqual("36. offset",         offset, 0x2A99U);
    a.check     ("37. KoreSection",    result.getSectionOffset(result.KoreSection, offset));
    a.checkEqual("38. offset",         offset, 0x2A9BU);
    a.check     ("39. LeechSection",  !result.getSectionOffset(result.LeechSection, offset));
    a.check     ("40. SkoreSection",   result.getSectionOffset(result.SkoreSection, offset));
    a.checkEqual("41. offset",         offset, 0x5E85U);

    // Test seeking
    AFL_CHECK_SUCCEEDS(a("51. seekToSection"), result.seekToSection(result.ShipSection));
    a.checkEqual("52. getPos", file.getPos(), 0x0060U);
    AFL_CHECK_THROWS(a("53. seekToSection"), result.seekToSection(result.LeechSection), afl::except::FileProblemException);
}
