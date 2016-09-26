/**
  *  \file u/t_game_timestamp.cpp
  *  \brief Test for game::Timestamp
  */

#include "game/timestamp.hpp"

#include "t_game.hpp"

/** Test initialisations. */
void
TestGameTimestamp::testInit()
{
    using game::Timestamp;

    // Null timestamp is not valid
    {
        Timestamp ts;
        TS_ASSERT(!ts.isValid());
    }

    // Assigning a correct timestamp
    {
        // 12-24-1988 Nakatomi Plaza - Never Forget
        const char data[18] = {'1','2','-','2','4','-','1','9','8','8','2','0',':','1','5',':','3','1'};
        Timestamp ts(data);
        TS_ASSERT(ts.isValid());
        TS_ASSERT_EQUALS(ts.getTimestampAsString(), "12-24-198820:15:31");
        TS_ASSERT_EQUALS(ts.getTimeAsString(), "20:15:31");
        TS_ASSERT_EQUALS(ts.getDateAsString(), "12-24-1988");

        // Compare with copy-out
        char data2[18];
        ts.storeRawData(data2);
        TS_ASSERT_SAME_DATA(data, data2, 18);

        // Compare directly
        TS_ASSERT_SAME_DATA(data, ts.getRawData(), 18);
    }

    // Components
    {
        // 04-05-2063, Day of first contact
        Timestamp ts(2063, 4, 5, 11, 50, 0);
        TS_ASSERT(ts.isValid());
        TS_ASSERT_EQUALS(ts.getTimestampAsString(), "04-05-206311:50:00");
    }
}

/** Test relations/comparisons. */
void
TestGameTimestamp::testRelation()
{
    using game::Timestamp;

    const char data[18]       = {'1','2','-','2','4','-','1','9','8','8','2','0',':','1','5',':','3','1'};
    const char prevYear[18]   = {'1','2','-','2','4','-','1','9','8','7','2','0',':','1','5',':','3','1'};
    const char prevMonth[18]  = {'1','1','-','2','4','-','1','9','8','8','2','0',':','1','5',':','3','1'};
    const char prevDay[18]    = {'1','2','-','2','3','-','1','9','8','8','2','0',':','1','5',':','3','1'};
    const char prevHour[18]   = {'1','2','-','2','4','-','1','9','8','8','1','9',':','1','5',':','3','1'};
    const char prevMinute[18] = {'1','2','-','2','4','-','1','9','8','8','2','0',':','1','4',':','3','1'};
    const char prevSecond[18] = {'1','2','-','2','4','-','1','9','8','8','2','0',':','1','5',':','3','0'};

    // Timestamp is not earlier than itself
    TS_ASSERT(!Timestamp(data).isEarlierThan(Timestamp(data)));

    // Verify relations between reference date and dates that differ in one component
    TS_ASSERT(!Timestamp(data).isEarlierThan(Timestamp(prevYear)));
    TS_ASSERT(!Timestamp(data).isEarlierThan(Timestamp(prevMonth)));
    TS_ASSERT(!Timestamp(data).isEarlierThan(Timestamp(prevDay)));
    TS_ASSERT(!Timestamp(data).isEarlierThan(Timestamp(prevHour)));
    TS_ASSERT(!Timestamp(data).isEarlierThan(Timestamp(prevMinute)));
    TS_ASSERT(!Timestamp(data).isEarlierThan(Timestamp(prevSecond)));

    TS_ASSERT(Timestamp(prevYear).isEarlierThan(Timestamp(data)));
    TS_ASSERT(Timestamp(prevMonth).isEarlierThan(Timestamp(data)));
    TS_ASSERT(Timestamp(prevDay).isEarlierThan(Timestamp(data)));
    TS_ASSERT(Timestamp(prevHour).isEarlierThan(Timestamp(data)));
    TS_ASSERT(Timestamp(prevMinute).isEarlierThan(Timestamp(data)));
    TS_ASSERT(Timestamp(prevSecond).isEarlierThan(Timestamp(data)));

    // Multiple differences
    TS_ASSERT(Timestamp(prevYear).isEarlierThan(Timestamp(prevSecond)));
    TS_ASSERT(Timestamp(prevYear).isEarlierThan(Timestamp(prevMinute)));
    TS_ASSERT(Timestamp(prevYear).isEarlierThan(Timestamp(prevHour)));
    TS_ASSERT(Timestamp(prevYear).isEarlierThan(Timestamp(prevDay)));
    TS_ASSERT(Timestamp(prevYear).isEarlierThan(Timestamp(prevMonth)));

    TS_ASSERT(!Timestamp(prevSecond).isEarlierThan(Timestamp(prevYear)));
    TS_ASSERT(!Timestamp(prevMinute).isEarlierThan(Timestamp(prevYear)));
    TS_ASSERT(!Timestamp(prevHour).isEarlierThan(Timestamp(prevYear)));
    TS_ASSERT(!Timestamp(prevDay).isEarlierThan(Timestamp(prevYear)));
    TS_ASSERT(!Timestamp(prevMonth).isEarlierThan(Timestamp(prevYear)));

    // Equalities
    TS_ASSERT_EQUALS(Timestamp(data) == Timestamp(data), true);
    TS_ASSERT_EQUALS(Timestamp(data) != Timestamp(data), false);
    TS_ASSERT_EQUALS(Timestamp(data) == Timestamp(prevDay), false);
    TS_ASSERT_EQUALS(Timestamp(data) != Timestamp(prevDay), true);

    TS_ASSERT_EQUALS(Timestamp(data) == data, true);
    TS_ASSERT_EQUALS(Timestamp(data) != data, false);
    TS_ASSERT_EQUALS(Timestamp(data) == prevDay, false);
    TS_ASSERT_EQUALS(Timestamp(data) != prevDay, true);
}

