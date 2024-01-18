/**
  *  \file test/ui/res/residtest.cpp
  *  \brief Test for ui::res::ResId
  */

#include "ui/res/resid.hpp"
#include "afl/test/testrunner.hpp"

/** Test makeResourceId. */
AFL_TEST("ui.res.ResId:makeResourceId", a)
{
    a.checkEqual("01", ui::res::makeResourceId("foo", 1), "foo.1");
    a.checkEqual("02", ui::res::makeResourceId("foo", 1, 2), "foo.1.2");
}

/** Test generalizeResourceId. */

// Regular case
AFL_TEST("ui.res.ResId:generalizeResourceId:normal", a)
{
    String_t id = "ship.34.105";
    a.check     ("01", ui::res::generalizeResourceId(id));
    a.checkEqual("02", id, "ship.34");
    a.check     ("03", ui::res::generalizeResourceId(id));
    a.checkEqual("04", id, "ship");
    a.check     ("05", !ui::res::generalizeResourceId(id));
}

// Boundary cases
AFL_TEST("ui.res.ResId:generalizeResourceId:empty", a)
{
    String_t id = "";
    a.check("", !ui::res::generalizeResourceId(id));
}

AFL_TEST("ui.res.ResId:generalizeResourceId:dots", a)
{
    String_t id = "..";
    a.check     ("01", ui::res::generalizeResourceId(id));
    a.checkEqual("02", id, ".");
    a.check     ("03", ui::res::generalizeResourceId(id));
    a.checkEqual("04", id, "");
    a.check     ("05", !ui::res::generalizeResourceId(id));
}

AFL_TEST("ui.res.ResId:generalizeResourceId:trailing-dot", a)
{
    String_t id = "foo.";
    a.check     ("01", ui::res::generalizeResourceId(id));
    a.checkEqual("02", id, "foo");
    a.check     ("03", !ui::res::generalizeResourceId(id));
}


/** Test matchResourceId. */
AFL_TEST("ui.res.ResId:matchResourceId", a)
{
    int aa = 0, bb = 0;
    a.check("01", ui::res::matchResourceId("ship.33.15", "ship", aa, bb));
    a.checkEqual("02", aa, 33);
    a.checkEqual("03", bb, 15);

    a.check("11", !ui::res::matchResourceId("ship.33.15", "ship", aa));
    a.check("12", !ui::res::matchResourceId("ship.33", "ship", aa, bb));

    a.check("21", ui::res::matchResourceId("ship.33.15", "ship.33", aa));
    a.checkEqual("22", aa, 15);
}
