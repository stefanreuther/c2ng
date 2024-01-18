/**
  *  \file test/util/updatertest.cpp
  *  \brief Test for util::Updater
  */

#include "util/updater.hpp"

#include "afl/string/string.hpp"
#include "afl/test/testrunner.hpp"

/** Test some success cases. */
AFL_TEST("util.Updater:true", a)
{
    int ia = 1;
    String_t sb = "x";

    a.check("01", bool(util::Updater().set(ia, 2)));
    a.check("02", bool(util::Updater().set(sb, String_t("y"))));
    a.check("03", bool(util::Updater().set(ia, 3).set(sb, String_t("z"))));
    a.check("04", bool(util::Updater().set(ia, 3).set(sb, String_t("a"))));
    a.check("05", bool(util::Updater().set(ia, 4).set(sb, String_t("a"))));
}

/** Test some non-update cases. */
AFL_TEST("util.Updater:false", a)
{
    int ia = 1;
    String_t sb = "x";

    a.check("01", !bool(util::Updater().set(ia, 1)));
    a.check("02", !bool(util::Updater().set(sb, String_t("x"))));
    a.check("03", !bool(util::Updater().set(ia, 1).set(sb, String_t("x"))));
    a.check("04", !bool(util::Updater()));
}
