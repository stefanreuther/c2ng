/**
  *  \file u/t_ui_res_resid.cpp
  *  \brief Test for ui::res::ResId
  */

#include "ui/res/resid.hpp"

#include "t_ui_res.hpp"

/** Test makeResourceId. */
void
TestUiResResId::testMake()
{
    TS_ASSERT_EQUALS(ui::res::makeResourceId("foo", 1), "foo.1");
    TS_ASSERT_EQUALS(ui::res::makeResourceId("foo", 1, 2), "foo.1.2");
}

/** Test generalizeResourceId. */
void
TestUiResResId::testGeneralize()
{
    // Regular case
    {
        String_t id = "ship.34.105";
        TS_ASSERT(ui::res::generalizeResourceId(id));
        TS_ASSERT_EQUALS(id, "ship.34");
        TS_ASSERT(ui::res::generalizeResourceId(id));
        TS_ASSERT_EQUALS(id, "ship");
        TS_ASSERT(!ui::res::generalizeResourceId(id));
    }

    // Boundary cases
    {
        String_t id = "";
        TS_ASSERT(!ui::res::generalizeResourceId(id));
    }
    {
        String_t id = "..";
        TS_ASSERT(ui::res::generalizeResourceId(id));
        TS_ASSERT_EQUALS(id, ".");
        TS_ASSERT(ui::res::generalizeResourceId(id));
        TS_ASSERT_EQUALS(id, "");
        TS_ASSERT(!ui::res::generalizeResourceId(id));
    }
    {
        String_t id = "foo.";
        TS_ASSERT(ui::res::generalizeResourceId(id));
        TS_ASSERT_EQUALS(id, "foo");
        TS_ASSERT(!ui::res::generalizeResourceId(id));
    }
}

/** Test matchResourceId. */
void
TestUiResResId::testMatch()
{
    int a = 0, b = 0;
    TS_ASSERT(ui::res::matchResourceId("ship.33.15", "ship", a, b));
    TS_ASSERT_EQUALS(a, 33);
    TS_ASSERT_EQUALS(b, 15);

    TS_ASSERT(!ui::res::matchResourceId("ship.33.15", "ship", a));
    TS_ASSERT(!ui::res::matchResourceId("ship.33", "ship", a, b));

    TS_ASSERT(ui::res::matchResourceId("ship.33.15", "ship.33", a));
    TS_ASSERT_EQUALS(a, 15);
}
