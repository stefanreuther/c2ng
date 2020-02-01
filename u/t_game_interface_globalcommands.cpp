/**
  *  \file u/t_game_interface_globalcommands.cpp
  *  \brief Test for game::interface::GlobalCommands
  */

#include "game/interface/globalcommands.hpp"

#include "t_game_interface.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/error.hpp"

/** Test checkPlayerSetArg: null.
    A: call checkPlayerSetArg with a null argument.
    E: result must be 0. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgNull()
{
    game::PlayerSet_t result;
    TS_ASSERT_EQUALS(game::interface::checkPlayerSetArg(result, 0), false);
}

/** Test checkPlayerSetArg: wrong type.
    A: call checkPlayerSetArg with a wrong type argument.
    E: must throw exception. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgWrong()
{
    afl::data::StringValue value("hi");
    game::PlayerSet_t result;
    TS_ASSERT_THROWS(game::interface::checkPlayerSetArg(result, &value), interpreter::Error);
}

/** Test checkPlayerSetArg: integer.
    A: call checkPlayerSetArg with integer argument.
    E: must return correct value. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgInt()
{
    {
        afl::data::IntegerValue value(8);
        game::PlayerSet_t result;
        TS_ASSERT_EQUALS(game::interface::checkPlayerSetArg(result, &value), true);
        TS_ASSERT_EQUALS(result, game::PlayerSet_t(8));
    }
    {
        afl::data::IntegerValue value(0);
        game::PlayerSet_t result;
        TS_ASSERT_EQUALS(game::interface::checkPlayerSetArg(result, &value), true);
        TS_ASSERT_EQUALS(result, game::PlayerSet_t(0));
    }
}

/** Test checkPlayerSetArg: array.
    A: call checkPlayerSetArg with array argument.
    E: must return correct value. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgArray()
{
    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->content.setNew(0, new afl::data::IntegerValue(4));
    ad->content.setNew(2, new afl::data::IntegerValue(7));
    interpreter::ArrayValue av(ad);

    game::PlayerSet_t result;
    TS_ASSERT_EQUALS(game::interface::checkPlayerSetArg(result, &av), true);
    TS_ASSERT_EQUALS(result, game::PlayerSet_t() + 4 + 7);
}

/** Test checkPlayerSetArg: out of range integer.
    A: call checkPlayerSetArg with out-of-range integer.
    E: must throw exception. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgIntRange()
{
    afl::data::IntegerValue value(-1);
    game::PlayerSet_t result;
    TS_ASSERT_THROWS(game::interface::checkPlayerSetArg(result, &value), interpreter::Error);
}

/** Test checkPlayerSetArg: out of range integer.
    A: call checkPlayerSetArg with an array containing out-of-range argument.
    E: must throw exception. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgArrayRange()
{
    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->content.setNew(0, new afl::data::IntegerValue(44));
    interpreter::ArrayValue av(ad);

    game::PlayerSet_t result;
    TS_ASSERT_THROWS(game::interface::checkPlayerSetArg(result, &av), interpreter::Error);
}

/** Test checkPlayerSetArg: vector.
    A: call checkPlayerSetArg with afl::data::Vector argument.
    E: must return correct value. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgVector()
{
    afl::base::Ref<afl::data::Vector> vd = afl::data::Vector::create();
    vd->setNew(0, new afl::data::IntegerValue(9));
    vd->setNew(2, new afl::data::IntegerValue(1));
    afl::data::VectorValue vv(vd);

    game::PlayerSet_t result;
    TS_ASSERT_EQUALS(game::interface::checkPlayerSetArg(result, &vv), true);
    TS_ASSERT_EQUALS(result, game::PlayerSet_t() + 9 + 1);
}

