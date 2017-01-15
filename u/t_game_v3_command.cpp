/**
  *  \file u/t_game_command.cpp
  *  \brief Test for game::v3::Command
  */

#include <memory>
#include "game/v3/command.hpp"

#include "t_game_v3.hpp"

namespace {
    /** Test one command.
        \param cmd      Command to test
        \param type     Expected command type
        \param id       Expected command Id
        \param arg      Expected command argument
        \param result   [optional] Expected stringification result
        \param file_only [optional] if true, this is a file-only command that shall not be recognized when not parsing a cmd.txt file */
    void testOneCommand(const char* cmd, game::v3::Command::Type type, game::Id_t id, const char* arg, const char* result = 0, bool file_only = false)
    {
        if (!arg) {
            arg = cmd;
        }
        try {
            TSM_ASSERT(cmd, !game::v3::Command::isMessageIntroducer(cmd));

            game::v3::Command* p = game::v3::Command::parseCommand(cmd, true);
            if (type == game::v3::Command::phc_Other) {
                TSM_ASSERT(cmd, !p);
            } else {
                TSM_ASSERT(cmd, p);
                TSM_ASSERT_EQUALS(cmd, p->getCommand(), type);
                TSM_ASSERT_EQUALS(cmd, p->getId(), id);
                TSM_ASSERT_EQUALS(cmd, p->getArg(), arg);
                if (result)
                    TSM_ASSERT_EQUALS(cmd, p->getCommandText(), result);
                delete p;
            }

            p = game::v3::Command::parseCommand(cmd, false);
            if (file_only || type == game::v3::Command::phc_Other) {
                TSM_ASSERT(cmd, !p);
            } else {
                TSM_ASSERT(cmd, p);
                TSM_ASSERT_EQUALS(cmd, p->getCommand(), type);
                TSM_ASSERT_EQUALS(cmd, p->getId(), id);
                TSM_ASSERT_EQUALS(cmd, p->getArg(), arg);
                if (result)
                    TSM_ASSERT_EQUALS(cmd, p->getCommandText(), result);
                delete p;
            }
        }
        catch (...) {
            TS_FAIL("exception!");
        }
    }

    /** Test behaviour of proto-commands.
        \param cmd      Command to test
        \param type     Expected command type
        \param id       Expected command Id
        \param isFull   true: this is a full command and must be recognized as proto-command and full command;
                        false: this is a proto-command and must not be recognized as a full command */
    void testProtoCommand(const char* cmd, game::v3::Command::Type type, game::Id_t id, bool isFull)
    {
        std::auto_ptr<game::v3::Command> normal, proto;
        TSM_ASSERT_THROWS_NOTHING(cmd, normal.reset(game::v3::Command::parseCommand(cmd, false, false)));
        TSM_ASSERT_THROWS_NOTHING(cmd, proto.reset(game::v3::Command::parseCommand(cmd, false, true)));

        // All commands must parse as a proto-command
        TSM_ASSERT(cmd, proto.get() != 0);
        TSM_ASSERT_EQUALS(cmd, proto->getCommand(), type);
        TSM_ASSERT_EQUALS(cmd, proto->getId(), id);

        // Full commands (and only those) must also parse
        if (isFull) {
            TSM_ASSERT(cmd, normal.get() != 0);
            TSM_ASSERT_EQUALS(cmd, normal->getCommand(), type);
            TSM_ASSERT_EQUALS(cmd, normal->getId(), id);
        } else {
            TSM_ASSERT(cmd, normal.get() == 0);
        }
    }
}

/** Test parsing and stringification. */
void
TestGameV3Command::testCommands()
{
    // ex GameCommandTestSuite::testCommands
    testOneCommand("language klingon",   game::v3::Command::phc_Language, 0, "klingon", "language klingon");
    testOneCommand("lanGuaGE   klingon", game::v3::Command::phc_Language, 0, "klingon", "language klingon");
    testOneCommand("l k",                game::v3::Command::phc_Language, 0, "k", "language k");
    testOneCommand("la k",               game::v3::Command::phc_Language, 0, "k", "language k");
    testOneCommand("li k",               game::v3::Command::phc_Other, 0, 0);
    testOneCommand("phost: la k",        game::v3::Command::phc_Language, 0, "k", "language k");

    testOneCommand("send config",        game::v3::Command::phc_SendConfig, 0, "", "send config");
    testOneCommand("s c",                game::v3::Command::phc_SendConfig, 0, "", "send config");
    testOneCommand("se CO",              game::v3::Command::phc_SendConfig, 0, "", "send config");
    testOneCommand("send fcodes",        game::v3::Command::phc_SendFcodes, 0, "", "send fcodes");
    testOneCommand("send f",             game::v3::Command::phc_SendFcodes, 0, "", "send fcodes");
    testOneCommand("send racenames",     game::v3::Command::phc_SendRacenames, 0, "", "send racenames");
    testOneCommand("s r",                game::v3::Command::phc_SendRacenames, 0, "", "send racenames");
    testOneCommand("send money",         game::v3::Command::phc_Other, 0, 0);

    testOneCommand("racename long Klingons", game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Long, "Klingons", "race long Klingons");
    testOneCommand("ra       long    Klingons", game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Long, "Klingons", "race long Klingons");
    testOneCommand("ra long Spacey   Klingons", game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Long, "Spacey   Klingons", "race long Spacey   Klingons");
    testOneCommand("ra short Frogs",     game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Short, "Frogs", "race short Frogs");
    testOneCommand("ra s Frogs",         game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Short, "Frogs", "race short Frogs");
    testOneCommand("ra a Frogs",         game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Adjective, "Frogs", "race adj Frogs");
    testOneCommand("ra adjective Frogs", game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Adjective, "Frogs", "race adj Frogs");
    testOneCommand("r adj Foo",          game::v3::Command::phc_Other, 0, 0);

    testOneCommand("filter y",           game::v3::Command::phc_Filter, 0, "y", "filter y");
    testOneCommand("f y",                game::v3::Command::phc_Filter, 0, "y", "filter y");

    testOneCommand("allies config 7 +c", game::v3::Command::phc_ConfigAlly, 7, "+c", "allies config 7 +c");
    testOneCommand("a c 7 +c",           game::v3::Command::phc_ConfigAlly, 7, "+c", "allies config 7 +c");

    testOneCommand("allies add 7",       game::v3::Command::phc_AddDropAlly, 7, "add", "allies add 7");
    testOneCommand("a a 7",              game::v3::Command::phc_AddDropAlly, 7, "a", "allies a 7");
    testOneCommand("a frob 7",           game::v3::Command::phc_Other, 0, 0);

    testOneCommand("give ship 9 to 2",   game::v3::Command::phc_GiveShip, 9, "2", "give ship 9 to 2");
    testOneCommand("g s 9 2",            game::v3::Command::phc_GiveShip, 9, "2", "give ship 9 to 2");
    testOneCommand("give planet 9 2",    game::v3::Command::phc_GivePlanet, 9, "2", "give planet 9 to 2");
    testOneCommand("g p 9 to 2",         game::v3::Command::phc_GivePlanet, 9, "2", "give planet 9 to 2");
    testOneCommand("give foo 9 to 2",    game::v3::Command::phc_Other, 0, 0);
    testOneCommand("give planet 9",      game::v3::Command::phc_Other, 0, 0);

    testOneCommand("remote control 333", game::v3::Command::phc_RemoteControl, 333, "control", "remote control 333");
    testOneCommand("re c 333",           game::v3::Command::phc_RemoteControl, 333, "c", "remote c 333");
    testOneCommand("remote allow   333", game::v3::Command::phc_RemoteControl, 333, "allow", "remote allow 333");
    testOneCommand("re a 333",           game::v3::Command::phc_RemoteControl, 333, "a", "remote a 333");
    testOneCommand("remote forbid 333",  game::v3::Command::phc_RemoteControl, 333, "forbid", "remote forbid 333");
    testOneCommand("re f 333",           game::v3::Command::phc_RemoteControl, 333, "f", "remote f 333");
    testOneCommand("remote drop 333",    game::v3::Command::phc_RemoteControl, 333, "drop", "remote drop 333");
    testOneCommand("re d 333",           game::v3::Command::phc_RemoteControl, 333, "d", "remote d 333");
    testOneCommand("re blurb 333",       game::v3::Command::phc_Other, 0, 0);

    testOneCommand("remote allow default", game::v3::Command::phc_RemoteDefault, 0, "allow", "remote allow default");
    testOneCommand("re a d",             game::v3::Command::phc_RemoteDefault, 0, "a", "remote a default");
    testOneCommand("remote forbid default", game::v3::Command::phc_RemoteDefault, 0, "forbid", "remote forbid default");
    // testOneCommand("remote drop default", game::v3::Command::phc_Other, 0, 0);

    testOneCommand("beamup 999 c100 nmax", game::v3::Command::phc_Beamup, 999, "c100 nmax", "beamup 999 c100 nmax");
    testOneCommand("be  999 c100 nmax",  game::v3::Command::phc_Beamup, 999, "c100 nmax", "beamup 999 c100 nmax");
    testOneCommand("beamup 140 T999 D999 M999 S999 C999 $999 N99",  game::v3::Command::phc_Beamup, 140, "T999 D999 M999 S999 C999 $999 N99", "be 140 T999 D999 M999 S999 C999 $999 N99");
    testOneCommand("beamup 140 T999 D99 M99 S999 C999 $999 N99",  game::v3::Command::phc_Beamup, 140, "T999 D99 M99 S999 C999 $999 N99", "beam 140 T999 D99 M99 S999 C999 $999 N99");
    testOneCommand("b 999 c100 nmax",    game::v3::Command::phc_Other, 0, 0);

    testOneCommand("$thost-allies ee9ffa", game::v3::Command::phc_TAlliance, 0, "ee9ffa", "$thost-allies ee9ffa", true);

    testOneCommand("$send-file foo",     game::v3::Command::phc_SendFile, 0, "foo", "$send-file foo", true);
    testOneCommand("$send-f foo",        game::v3::Command::phc_SendFile, 0, "foo", "$send-file foo", true);
    testOneCommand("$send- foo",         game::v3::Command::phc_Other, 0, 0);

    testOneCommand("enemies add 8",      game::v3::Command::phc_Enemies, 8, "add", "enemies add 8");
    testOneCommand("e a 8",              game::v3::Command::phc_Other, 0, 0);    /* interpreted as bogus 'extmission' */
    testOneCommand("ene a 8",            game::v3::Command::phc_Enemies, 8, "a", "enemies a 8");
    testOneCommand("ene d 8",            game::v3::Command::phc_Enemies, 8, "d", "enemies d 8");
    testOneCommand("ene x 8",            game::v3::Command::phc_Other, 0, 0);    /* not a valid 'enemies' subcommand */

    testOneCommand("unload 42 $10",      game::v3::Command::phc_Unload, 42, "$10", "unload 42 $10");
    testOneCommand("unload 140 T999 D999 M999 S999 C999 $999 N99",  game::v3::Command::phc_Unload, 140, "T999 D999 M999 S999 C999 $999 N99", "unl 140 T999 D999 M999 S999 C999 $999 N99");

    testOneCommand("transfer 42 $10",    game::v3::Command::phc_Transfer, 42, "$10", "transfer 42 $10");
    testOneCommand("transfer 140 T999 D999 M999 S999 C999 $999 N99",  game::v3::Command::phc_Transfer, 140, "T999 D999 M999 S999 C999 $999 N99", "tra 140 T999 D999 M999 S999 C999 $999 N99");

    testOneCommand("show ship 99 to 7",     game::v3::Command::phc_ShowShip, 99, "7");
    testOneCommand("show ship 99 to 3 4 5", game::v3::Command::phc_ShowShip, 99, "3 4 5");
    testOneCommand("sh s 99 t 1 2 3",       game::v3::Command::phc_ShowShip, 99, "1 2 3");
    testOneCommand("sh s 99 7 8 9",         game::v3::Command::phc_ShowShip, 99, "7 8 9", "show ship 99 7 8 9");

    testOneCommand("show planet 99 to 7",     game::v3::Command::phc_ShowPlanet, 99, "7");
    testOneCommand("show planet 99 to 3 4 5", game::v3::Command::phc_ShowPlanet, 99, "3 4 5");
    testOneCommand("sh p 99 t 1 2 3",         game::v3::Command::phc_ShowPlanet, 99, "1 2 3");
    testOneCommand("sh p 99 7 8 9",           game::v3::Command::phc_ShowPlanet, 99, "7 8 9", "show planet 99 7 8 9");

    testOneCommand("show minefield 42 to 7",     game::v3::Command::phc_ShowMinefield, 42, "7");
    testOneCommand("show minefield 42 to 3 4 5", game::v3::Command::phc_ShowMinefield, 42, "3 4 5");
    testOneCommand("sh m 42 t 1 2 3",            game::v3::Command::phc_ShowMinefield, 42, "1 2 3");
    testOneCommand("sh m 42 7 8 9",              game::v3::Command::phc_ShowMinefield, 42, "7 8 9", "show minefield 42 7 8 9");
    testOneCommand("sh m 10000 11",              game::v3::Command::phc_ShowMinefield, 10000, "11");

    // failing versions of "show ..."
    testOneCommand("s s 99 7",            game::v3::Command::phc_Other, 0, 0);  // 's' actually means 'send', which
    testOneCommand("s m 99 7",            game::v3::Command::phc_Other, 0, 0);  // ...has valid subcommands 'fcodes',
    testOneCommand("s p 99 7",            game::v3::Command::phc_Other, 0, 0);  // 'racenames', 'config'
    testOneCommand("sh a 99 7",           game::v3::Command::phc_Other, 0, 0);

    testOneCommand("refit 123 1 2 3 4 5", game::v3::Command::phc_Refit, 123, "1 2 3 4 5");
    testOneCommand("refit 123 1 2 3 4 5", game::v3::Command::phc_Refit, 123, "1 2 3 4 5");
    testOneCommand("ref 999 1",           game::v3::Command::phc_Refit, 999, "1");
    testOneCommand("re 999 1",            game::v3::Command::phc_Other, 0, 0);  // 're' actually means 'remote'
    testOneCommand("r 999 1",             game::v3::Command::phc_Other, 0, 0);  // 'r' is not defined
}

/** Test game::v3::Command::isMessageIntroducer. */
void
TestGameV3Command::testMessageIntroducer()
{
    // ex GameCommandTestSuite::testMI
    TS_ASSERT(game::v3::Command::isMessageIntroducer("message 1 2 3"));
    TS_ASSERT(game::v3::Command::isMessageIntroducer("message 1"));
    TS_ASSERT(game::v3::Command::isMessageIntroducer("message u"));
    TS_ASSERT(game::v3::Command::isMessageIntroducer("m u"));
    TS_ASSERT(game::v3::Command::isMessageIntroducer("rumor u"));
    TS_ASSERT(game::v3::Command::isMessageIntroducer("rumour u"));
    TS_ASSERT(game::v3::Command::isMessageIntroducer("ru u"));
    TS_ASSERT(!game::v3::Command::isMessageIntroducer("r u"));
}

/** Test parsing of proto-commands. */
void
TestGameV3Command::testProto()
{
    testProtoCommand("send config",             game::v3::Command::phc_SendConfig,      0, true);
    testProtoCommand("send racenames",          game::v3::Command::phc_SendRacenames,   0, true);
    testProtoCommand("send fcodes",             game::v3::Command::phc_SendFcodes,      0, true);
    testProtoCommand("language",                game::v3::Command::phc_Language,        0, false);
    testProtoCommand("language tlh",            game::v3::Command::phc_Language,        0, true);
    testProtoCommand("filter",                  game::v3::Command::phc_Filter,          0, false);
    testProtoCommand("filter yes",              game::v3::Command::phc_Filter,          0, true);
    testProtoCommand("give ship 1",             game::v3::Command::phc_GiveShip,        1, false);
    testProtoCommand("give ship 1 to",          game::v3::Command::phc_GiveShip,        1, false);
    testProtoCommand("give ship 1 to 3",        game::v3::Command::phc_GiveShip,        1, true);
    testProtoCommand("give planet 7",           game::v3::Command::phc_GivePlanet,      7, false);
    testProtoCommand("give planet 7 to",        game::v3::Command::phc_GivePlanet,      7, false);
    testProtoCommand("give planet 7 to 3",      game::v3::Command::phc_GivePlanet,      7, true);
    testProtoCommand("allies config 4",         game::v3::Command::phc_ConfigAlly,      4, false);
    testProtoCommand("allies config 4 +c",      game::v3::Command::phc_ConfigAlly,      4, true);
    testProtoCommand("allies add 3",            game::v3::Command::phc_AddDropAlly,     3, true);
    testProtoCommand("allies drop 3",           game::v3::Command::phc_AddDropAlly,     3, true);
    testProtoCommand("remote whatever default", game::v3::Command::phc_RemoteDefault,   0, true /* sic! happens to be recognized as full command. */);
    testProtoCommand("remote control default",  game::v3::Command::phc_RemoteDefault,   0, true);
    testProtoCommand("remote whatever 4",       game::v3::Command::phc_RemoteControl,   4, false);
    testProtoCommand("remote control 4",        game::v3::Command::phc_RemoteControl,   4, true);
    testProtoCommand("beamup 30",               game::v3::Command::phc_Beamup,         30, true);
    testProtoCommand("beamup 30 N10",           game::v3::Command::phc_Beamup,         30, true);
    testProtoCommand("unload 42",               game::v3::Command::phc_Unload,         42, true);
    testProtoCommand("unload 42 N10",           game::v3::Command::phc_Unload,         42, true);
    testProtoCommand("transfer 99",             game::v3::Command::phc_Transfer,       99, true);
    testProtoCommand("transfer 99 N10 to 97",   game::v3::Command::phc_Transfer,       99, true);
    testProtoCommand("race long",               game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Long, true);
    testProtoCommand("race long Karl-Theodor",  game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Long, true);
    testProtoCommand("race short",              game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Short, true);
    testProtoCommand("race short KT",           game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Short, true);
    testProtoCommand("race adj",                game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Adjective, true);
    testProtoCommand("race adj KT",             game::v3::Command::phc_SetRaceName, game::v3::Command::phcrn_Adjective, true);
    testProtoCommand("enemies whatever 9",      game::v3::Command::phc_Enemies,         9, false);
    testProtoCommand("enemies add 9",           game::v3::Command::phc_Enemies,         9, true);
    testProtoCommand("show minefield 150",      game::v3::Command::phc_ShowMinefield, 150, true /* sic! happens to be recognized as full command. */);
    testProtoCommand("show minefield 150 3",    game::v3::Command::phc_ShowMinefield, 150, true);
    testProtoCommand("show ship 499",           game::v3::Command::phc_ShowShip,      499, true /* sic! happens to be recognized as full command. */);
    testProtoCommand("show ship 499 1 2 3",     game::v3::Command::phc_ShowShip,      499, true);
    testProtoCommand("show planet 363",         game::v3::Command::phc_ShowPlanet,    363, true /* sic! happens to be recognized as full command. */);
    testProtoCommand("show planet 363 9",       game::v3::Command::phc_ShowPlanet,    363, true);
    testProtoCommand("refit 9",                 game::v3::Command::phc_Refit,           9, true /* sic! happens to be recognized as full command. */);
    testProtoCommand("refit 9 8 8 8",           game::v3::Command::phc_Refit,           9, true);
}
