/**
  *  \file u/t_util_updater.cpp
  *  \brief Test for util::Updater
  */

#include "util/updater.hpp"

#include "t_util.hpp"
#include "afl/string/string.hpp"

/** Test some success cases. */
void
TestUtilUpdater::testTrue()
{
    int a = 1;
    String_t b = "x";

    TS_ASSERT(bool(util::Updater().set(a, 2)));
    TS_ASSERT(bool(util::Updater().set(b, String_t("y"))));
    TS_ASSERT(bool(util::Updater().set(a, 3).set(b, String_t("z"))));
    TS_ASSERT(bool(util::Updater().set(a, 3).set(b, String_t("a"))));
    TS_ASSERT(bool(util::Updater().set(a, 4).set(b, String_t("a"))));
}

/** Test some non-update cases. */
void
TestUtilUpdater::testFalse()
{
    int a = 1;
    String_t b = "x";

    TS_ASSERT(!bool(util::Updater().set(a, 1)));
    TS_ASSERT(!bool(util::Updater().set(b, String_t("x"))));
    TS_ASSERT(!bool(util::Updater().set(a, 1).set(b, String_t("x"))));
    TS_ASSERT(!bool(util::Updater()));
}

