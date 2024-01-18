/**
  *  \file test/game/timestamptest.cpp
  *  \brief Test for game::Timestamp
  */

#include "game/timestamp.hpp"
#include "afl/test/testrunner.hpp"

using game::Timestamp;

/** Test initialisations. */

// Null timestamp is not valid
AFL_TEST("game.Timestamp:init:null", a)
{
    Timestamp ts;
    a.check("01. isValid", !ts.isValid());
}

// Assigning a correct timestamp
AFL_TEST("game.Timestamp:init:data", a)
{
    // 12-24-1988 Nakatomi Plaza - Never Forget
    static const uint8_t data[18] = {'1','2','-','2','4','-','1','9','8','8','2','0',':','1','5',':','3','1'};
    Timestamp ts(data);
    a.check("01. isValid", ts.isValid());
    a.checkEqual("02. getTimestampAsString", ts.getTimestampAsString(), "12-24-198820:15:31");
    a.checkEqual("03. getTimeAsString", ts.getTimeAsString(), "20:15:31");
    a.checkEqual("04. getDateAsString", ts.getDateAsString(), "12-24-1988");

    // Compare with copy-out
    uint8_t data2[18];
    ts.storeRawData(data2);
    a.checkEqualContent("11. storeRawData", afl::base::ConstBytes_t(data), afl::base::ConstBytes_t(data2));

    // Compare directly
    a.checkEqualContent("12. getRawData",   afl::base::ConstBytes_t(data), afl::base::ConstBytes_t(ts.getRawData()));
}

// Components
AFL_TEST("game.Timestamp:init:parts", a)
{
    // 04-05-2063, Day of first contact
    Timestamp ts(2063, 4, 5, 11, 50, 0);
    a.check("01. isValid", ts.isValid());
    a.checkEqual("02. getTimestampAsString", ts.getTimestampAsString(), "04-05-206311:50:00");
}


/** Test relations/comparisons. */
AFL_TEST("game.Timestamp:compare", a)
{
    static const uint8_t data[18]       = {'1','2','-','2','4','-','1','9','8','8','2','0',':','1','5',':','3','1'};
    static const uint8_t prevYear[18]   = {'1','2','-','2','4','-','1','9','8','7','2','0',':','1','5',':','3','1'};
    static const uint8_t prevMonth[18]  = {'1','1','-','2','4','-','1','9','8','8','2','0',':','1','5',':','3','1'};
    static const uint8_t prevDay[18]    = {'1','2','-','2','3','-','1','9','8','8','2','0',':','1','5',':','3','1'};
    static const uint8_t prevHour[18]   = {'1','2','-','2','4','-','1','9','8','8','1','9',':','1','5',':','3','1'};
    static const uint8_t prevMinute[18] = {'1','2','-','2','4','-','1','9','8','8','2','0',':','1','4',':','3','1'};
    static const uint8_t prevSecond[18] = {'1','2','-','2','4','-','1','9','8','8','2','0',':','1','5',':','3','0'};

    // Timestamp is not earlier than itself
    a.check("01", !Timestamp(data).isEarlierThan(Timestamp(data)));

    // Verify relations between reference date and dates that differ in one component
    a.check("11", !Timestamp(data).isEarlierThan(Timestamp(prevYear)));
    a.check("12", !Timestamp(data).isEarlierThan(Timestamp(prevMonth)));
    a.check("13", !Timestamp(data).isEarlierThan(Timestamp(prevDay)));
    a.check("14", !Timestamp(data).isEarlierThan(Timestamp(prevHour)));
    a.check("15", !Timestamp(data).isEarlierThan(Timestamp(prevMinute)));
    a.check("16", !Timestamp(data).isEarlierThan(Timestamp(prevSecond)));

    a.check("21", Timestamp(prevYear).isEarlierThan(Timestamp(data)));
    a.check("22", Timestamp(prevMonth).isEarlierThan(Timestamp(data)));
    a.check("23", Timestamp(prevDay).isEarlierThan(Timestamp(data)));
    a.check("24", Timestamp(prevHour).isEarlierThan(Timestamp(data)));
    a.check("25", Timestamp(prevMinute).isEarlierThan(Timestamp(data)));
    a.check("26", Timestamp(prevSecond).isEarlierThan(Timestamp(data)));

    // Multiple differences
    a.check("31", Timestamp(prevYear).isEarlierThan(Timestamp(prevSecond)));
    a.check("32", Timestamp(prevYear).isEarlierThan(Timestamp(prevMinute)));
    a.check("33", Timestamp(prevYear).isEarlierThan(Timestamp(prevHour)));
    a.check("34", Timestamp(prevYear).isEarlierThan(Timestamp(prevDay)));
    a.check("35", Timestamp(prevYear).isEarlierThan(Timestamp(prevMonth)));

    a.check("41", !Timestamp(prevSecond).isEarlierThan(Timestamp(prevYear)));
    a.check("42", !Timestamp(prevMinute).isEarlierThan(Timestamp(prevYear)));
    a.check("43", !Timestamp(prevHour).isEarlierThan(Timestamp(prevYear)));
    a.check("44", !Timestamp(prevDay).isEarlierThan(Timestamp(prevYear)));
    a.check("45", !Timestamp(prevMonth).isEarlierThan(Timestamp(prevYear)));

    // Equalities
    a.checkEqual("51", Timestamp(data) == Timestamp(data), true);
    a.checkEqual("52", Timestamp(data) != Timestamp(data), false);
    a.checkEqual("53", Timestamp(data) == Timestamp(prevDay), false);
    a.checkEqual("54", Timestamp(data) != Timestamp(prevDay), true);

    a.checkEqual("61", Timestamp(data) == data, true);
    a.checkEqual("62", Timestamp(data) != data, false);
    a.checkEqual("63", Timestamp(data) == prevDay, false);
    a.checkEqual("64", Timestamp(data) != prevDay, true);
}
