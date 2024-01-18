/**
  *  \file test/game/v3/commandtest.cpp
  *  \brief Test for game::v3::Command
  */

#include "game/v3/command.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include <memory>

using game::v3::Command;

namespace {
    /** Test one command.
        \param cmd      Command to test
        \param type     Expected command type
        \param id       Expected command Id
        \param arg      Expected command argument
        \param result   [optional] Expected stringification result
        \param file_only [optional] if true, this is a file-only command that shall not be recognized when not parsing a cmd.txt file */
    void testOneCommand(afl::test::Assert a, const char* cmd, Command::Type type, game::Id_t id, const char* arg, const char* result = 0, bool file_only = false)
    {
        afl::test::Assert aa(a(cmd));
        if (!arg) {
            arg = cmd;
        }

        aa.check("01. isMessageIntroducer", !Command::isMessageIntroducer(cmd));

        std::auto_ptr<Command> p(Command::parseCommand(cmd, true, false));
        if (type == Command::Other) {
            aa.checkNull("11. parseCommand", p.get());
        } else {
            aa.checkNonNull("12. parseCommand", p.get());
            aa.checkEqual("13. getCommand", p->getCommand(), type);
            aa.checkEqual("14. getId", p->getId(), id);
            aa.checkEqual("15. getArg", p->getArg(), arg);
            if (result) {
                aa.checkEqual("16. getCommandText", p->getCommandText(), result);
            }
        }

        p.reset(Command::parseCommand(cmd, false, false));
        if (file_only || type == Command::Other) {
            aa.checkNull("21. parseCommand", p.get());
        } else {
            aa.checkNonNull("22. parseCommand", p.get());
            aa.checkEqual("23. getCommand", p->getCommand(), type);
            aa.checkEqual("24. getId", p->getId(), id);
            aa.checkEqual("25. getArg", p->getArg(), arg);
            if (result) {
                aa.checkEqual("26. getCommandText", p->getCommandText(), result);
            }
        }
    }

    /** Test behaviour of proto-commands.
        \param cmd      Command to test
        \param type     Expected command type
        \param id       Expected command Id
        \param isFull   true: this is a full command and must be recognized as proto-command and full command;
                        false: this is a proto-command and must not be recognized as a full command */
    void testProtoCommand(afl::test::Assert a, const char* cmd, Command::Type type, game::Id_t id, bool isFull)
    {
        afl::test::Assert aa(a(cmd));
        std::auto_ptr<Command> normal, proto;
        AFL_CHECK_SUCCEEDS(a("01. parse normal"), normal.reset(Command::parseCommand(cmd, false, false)));
        AFL_CHECK_SUCCEEDS(a("02. parse proto"),  proto.reset(Command::parseCommand(cmd, false, true)));

        // All commands must parse as a proto-command
        aa.checkNonNull("11. proto", proto.get());
        aa.checkEqual("12. getCommand", proto->getCommand(), type);
        aa.checkEqual("13. getId", proto->getId(), id);

        // Full commands (and only those) must also parse
        if (isFull) {
            aa.checkNonNull("21. normal", normal.get());
            aa.checkEqual("22. getCommand", normal->getCommand(), type);
            aa.checkEqual("23. getId", normal->getId(), id);
        } else {
            aa.checkNull("24. normal", normal.get());
        }
    }
}

/** Test parsing and stringification. */
AFL_TEST("game.v3.Command:commands", a)
{
    // ex GameCommandTestSuite::testCommands
    testOneCommand(a, "language klingon",   Command::Language, 0, "klingon", "language klingon");
    testOneCommand(a, "lanGuaGE   klingon", Command::Language, 0, "klingon", "language klingon");
    testOneCommand(a, "l k",                Command::Language, 0, "k", "language k");
    testOneCommand(a, "la k",               Command::Language, 0, "k", "language k");
    testOneCommand(a, "li k",               Command::Other, 0, 0);
    testOneCommand(a, "phost: la k",        Command::Language, 0, "k", "language k");

    testOneCommand(a, "send config",        Command::SendConfig, 0, "", "send config");
    testOneCommand(a, "s c",                Command::SendConfig, 0, "", "send config");
    testOneCommand(a, "se CO",              Command::SendConfig, 0, "", "send config");
    testOneCommand(a, "send fcodes",        Command::SendFCodes, 0, "", "send fcodes");
    testOneCommand(a, "send f",             Command::SendFCodes, 0, "", "send fcodes");
    testOneCommand(a, "send racenames",     Command::SendRaceNames, 0, "", "send racenames");
    testOneCommand(a, "s r",                Command::SendRaceNames, 0, "", "send racenames");
    testOneCommand(a, "send money",         Command::Other, 0, 0);

    testOneCommand(a, "racename long Klingons", Command::SetRaceName, Command::LongName, "Klingons", "race long Klingons");
    testOneCommand(a, "ra       long    Klingons", Command::SetRaceName, Command::LongName, "Klingons", "race long Klingons");
    testOneCommand(a, "ra long Spacey   Klingons", Command::SetRaceName, Command::LongName, "Spacey   Klingons", "race long Spacey   Klingons");
    testOneCommand(a, "ra short Frogs",     Command::SetRaceName, Command::ShortName, "Frogs", "race short Frogs");
    testOneCommand(a, "ra s Frogs",         Command::SetRaceName, Command::ShortName, "Frogs", "race short Frogs");
    testOneCommand(a, "ra a Frogs",         Command::SetRaceName, Command::AdjectiveName, "Frogs", "race adj Frogs");
    testOneCommand(a, "ra adjective Frogs", Command::SetRaceName, Command::AdjectiveName, "Frogs", "race adj Frogs");
    testOneCommand(a, "r adj Foo",          Command::Other, 0, 0);

    testOneCommand(a, "filter y",           Command::Filter, 0, "y", "filter y");
    testOneCommand(a, "f y",                Command::Filter, 0, "y", "filter y");

    testOneCommand(a, "allies config 7 +c", Command::ConfigAlly, 7, "+c", "allies config 7 +c");
    testOneCommand(a, "a c 7 +c",           Command::ConfigAlly, 7, "+c", "allies config 7 +c");

    testOneCommand(a, "allies add 7",       Command::AddDropAlly, 7, "add", "allies add 7");
    testOneCommand(a, "a a 7",              Command::AddDropAlly, 7, "a", "allies a 7");
    testOneCommand(a, "a frob 7",           Command::Other, 0, 0);

    testOneCommand(a, "give ship 9 to 2",   Command::GiveShip, 9, "2", "give ship 9 to 2");
    testOneCommand(a, "g s 9 2",            Command::GiveShip, 9, "2", "give ship 9 to 2");
    testOneCommand(a, "give planet 9 2",    Command::GivePlanet, 9, "2", "give planet 9 to 2");
    testOneCommand(a, "g p 9 to 2",         Command::GivePlanet, 9, "2", "give planet 9 to 2");
    testOneCommand(a, "give foo 9 to 2",    Command::Other, 0, 0);
    testOneCommand(a, "give planet 9",      Command::Other, 0, 0);

    testOneCommand(a, "remote control 333", Command::RemoteControl, 333, "control", "remote control 333");
    testOneCommand(a, "re c 333",           Command::RemoteControl, 333, "c", "remote c 333");
    testOneCommand(a, "remote allow   333", Command::RemoteControl, 333, "allow", "remote allow 333");
    testOneCommand(a, "re a 333",           Command::RemoteControl, 333, "a", "remote a 333");
    testOneCommand(a, "remote forbid 333",  Command::RemoteControl, 333, "forbid", "remote forbid 333");
    testOneCommand(a, "re f 333",           Command::RemoteControl, 333, "f", "remote f 333");
    testOneCommand(a, "remote drop 333",    Command::RemoteControl, 333, "drop", "remote drop 333");
    testOneCommand(a, "re d 333",           Command::RemoteControl, 333, "d", "remote d 333");
    testOneCommand(a, "re blurb 333",       Command::Other, 0, 0);

    testOneCommand(a, "remote allow default", Command::RemoteDefault, 0, "allow", "remote allow default");
    testOneCommand(a, "re a d",             Command::RemoteDefault, 0, "a", "remote a default");
    testOneCommand(a, "remote forbid default", Command::RemoteDefault, 0, "forbid", "remote forbid default");
    // testOneCommand(a, "remote drop default", Command::Other, 0, 0);

    testOneCommand(a, "beamup 999 c100 nmax", Command::BeamUp, 999, "c100 nmax", "beamup 999 c100 nmax");
    testOneCommand(a, "be  999 c100 nmax",  Command::BeamUp, 999, "c100 nmax", "beamup 999 c100 nmax");
    testOneCommand(a, "beamup 140 T999 D999 M999 S999 C999 $999 N99",  Command::BeamUp, 140, "T999 D999 M999 S999 C999 $999 N99", "be 140 T999 D999 M999 S999 C999 $999 N99");
    testOneCommand(a, "beamup 140 T999 D99 M99 S999 C999 $999 N99",  Command::BeamUp, 140, "T999 D99 M99 S999 C999 $999 N99", "beam 140 T999 D99 M99 S999 C999 $999 N99");
    testOneCommand(a, "b 999 c100 nmax",    Command::Other, 0, 0);

    testOneCommand(a, "$thost-allies ee9ffa", Command::TAlliance, 0, "ee9ffa", "$thost-allies ee9ffa", true);

    testOneCommand(a, "$send-file foo",     Command::SendFile, 0, "foo", "$send-file foo", true);
    testOneCommand(a, "$send-f foo",        Command::SendFile, 0, "foo", "$send-file foo", true);
    testOneCommand(a, "$send- foo",         Command::Other, 0, 0);

    testOneCommand(a, "enemies add 8",      Command::Enemies, 8, "add", "enemies add 8");
    testOneCommand(a, "e a 8",              Command::Other, 0, 0);    /* interpreted as bogus 'extmission' */
    testOneCommand(a, "ene a 8",            Command::Enemies, 8, "a", "enemies a 8");
    testOneCommand(a, "ene d 8",            Command::Enemies, 8, "d", "enemies d 8");
    testOneCommand(a, "ene x 8",            Command::Other, 0, 0);    /* not a valid 'enemies' subcommand */

    testOneCommand(a, "unload 42 $10",      Command::Unload, 42, "$10", "unload 42 $10");
    testOneCommand(a, "unload 140 T999 D999 M999 S999 C999 $999 N99",  Command::Unload, 140, "T999 D999 M999 S999 C999 $999 N99", "unl 140 T999 D999 M999 S999 C999 $999 N99");

    testOneCommand(a, "transfer 42 $10",    Command::Transfer, 42, "$10", "transfer 42 $10");
    testOneCommand(a, "transfer 140 T999 D999 M999 S999 C999 $999 N99",  Command::Transfer, 140, "T999 D999 M999 S999 C999 $999 N99", "tra 140 T999 D999 M999 S999 C999 $999 N99");

    testOneCommand(a, "show ship 99 to 7",     Command::ShowShip, 99, "7");
    testOneCommand(a, "show ship 99 to 3 4 5", Command::ShowShip, 99, "3 4 5");
    testOneCommand(a, "sh s 99 t 1 2 3",       Command::ShowShip, 99, "1 2 3");
    testOneCommand(a, "sh s 99 7 8 9",         Command::ShowShip, 99, "7 8 9", "show ship 99 7 8 9");

    testOneCommand(a, "show planet 99 to 7",     Command::ShowPlanet, 99, "7");
    testOneCommand(a, "show planet 99 to 3 4 5", Command::ShowPlanet, 99, "3 4 5");
    testOneCommand(a, "sh p 99 t 1 2 3",         Command::ShowPlanet, 99, "1 2 3");
    testOneCommand(a, "sh p 99 7 8 9",           Command::ShowPlanet, 99, "7 8 9", "show planet 99 7 8 9");

    testOneCommand(a, "show minefield 42 to 7",     Command::ShowMinefield, 42, "7");
    testOneCommand(a, "show minefield 42 to 3 4 5", Command::ShowMinefield, 42, "3 4 5");
    testOneCommand(a, "sh m 42 t 1 2 3",            Command::ShowMinefield, 42, "1 2 3");
    testOneCommand(a, "sh m 42 7 8 9",              Command::ShowMinefield, 42, "7 8 9", "show minefield 42 7 8 9");
    testOneCommand(a, "sh m 10000 11",              Command::ShowMinefield, 10000, "11");

    // failing versions of "show ..."
    testOneCommand(a, "s s 99 7",            Command::Other, 0, 0);  // 's' actually means 'send', which
    testOneCommand(a, "s m 99 7",            Command::Other, 0, 0);  // ...has valid subcommands 'fcodes',
    testOneCommand(a, "s p 99 7",            Command::Other, 0, 0);  // 'racenames', 'config'
    testOneCommand(a, "sh a 99 7",           Command::Other, 0, 0);

    testOneCommand(a, "refit 123 1 2 3 4 5", Command::Refit, 123, "1 2 3 4 5");
    testOneCommand(a, "refit 123 1 2 3 4 5", Command::Refit, 123, "1 2 3 4 5");
    testOneCommand(a, "ref 999 1",           Command::Refit, 999, "1", "refit 999 1");
    testOneCommand(a, "re 999 1",            Command::Other, 0, 0);  // 're' actually means 'remote'
    testOneCommand(a, "r 999 1",             Command::Other, 0, 0);  // 'r' is not defined
}

/** Test Command::isMessageIntroducer. */
AFL_TEST("game.v3.Command:isMessageIntroducer", a)
{
    // ex GameCommandTestSuite::testMI
    a.check("01", Command::isMessageIntroducer("message 1 2 3"));
    a.check("02", Command::isMessageIntroducer("message 1"));
    a.check("03", Command::isMessageIntroducer("message u"));
    a.check("04", Command::isMessageIntroducer("m u"));
    a.check("05", Command::isMessageIntroducer("rumor u"));
    a.check("06", Command::isMessageIntroducer("rumour u"));
    a.check("07", Command::isMessageIntroducer("ru u"));
    a.check("08", !Command::isMessageIntroducer("r u"));
}

/** Test parsing of proto-commands. */
AFL_TEST("game.v3.Command:proto-commands", a)
{
    testProtoCommand(a, "send config",             Command::SendConfig,      0, true);
    testProtoCommand(a, "send racenames",          Command::SendRaceNames,   0, true);
    testProtoCommand(a, "send fcodes",             Command::SendFCodes,      0, true);
    testProtoCommand(a, "language",                Command::Language,        0, false);
    testProtoCommand(a, "language tlh",            Command::Language,        0, true);
    testProtoCommand(a, "filter",                  Command::Filter,          0, false);
    testProtoCommand(a, "filter yes",              Command::Filter,          0, true);
    testProtoCommand(a, "give ship 1",             Command::GiveShip,        1, false);
    testProtoCommand(a, "give ship 1 to",          Command::GiveShip,        1, false);
    testProtoCommand(a, "give ship 1 to 3",        Command::GiveShip,        1, true);
    testProtoCommand(a, "give planet 7",           Command::GivePlanet,      7, false);
    testProtoCommand(a, "give planet 7 to",        Command::GivePlanet,      7, false);
    testProtoCommand(a, "give planet 7 to 3",      Command::GivePlanet,      7, true);
    testProtoCommand(a, "allies config 4",         Command::ConfigAlly,      4, false);
    testProtoCommand(a, "allies config 4 +c",      Command::ConfigAlly,      4, true);
    testProtoCommand(a, "allies add 3",            Command::AddDropAlly,     3, true);
    testProtoCommand(a, "allies drop 3",           Command::AddDropAlly,     3, true);
    testProtoCommand(a, "remote whatever default", Command::RemoteDefault,   0, true /* sic! happens to be recognized as full command. */);
    testProtoCommand(a, "remote control default",  Command::RemoteDefault,   0, true);
    testProtoCommand(a, "remote whatever 4",       Command::RemoteControl,   4, false);
    testProtoCommand(a, "remote control 4",        Command::RemoteControl,   4, true);
    testProtoCommand(a, "beamup 30",               Command::BeamUp,         30, true);
    testProtoCommand(a, "beamup 30 N10",           Command::BeamUp,         30, true);
    testProtoCommand(a, "unload 42",               Command::Unload,         42, true);
    testProtoCommand(a, "unload 42 N10",           Command::Unload,         42, true);
    testProtoCommand(a, "transfer 99",             Command::Transfer,       99, true);
    testProtoCommand(a, "transfer 99 N10 to 97",   Command::Transfer,       99, true);
    testProtoCommand(a, "race long",               Command::SetRaceName, Command::LongName, true);
    testProtoCommand(a, "race long Karl-Theodor",  Command::SetRaceName, Command::LongName, true);
    testProtoCommand(a, "race short",              Command::SetRaceName, Command::ShortName, true);
    testProtoCommand(a, "race short KT",           Command::SetRaceName, Command::ShortName, true);
    testProtoCommand(a, "race adj",                Command::SetRaceName, Command::AdjectiveName, true);
    testProtoCommand(a, "race adj KT",             Command::SetRaceName, Command::AdjectiveName, true);
    testProtoCommand(a, "enemies whatever 9",      Command::Enemies,         9, false);
    testProtoCommand(a, "enemies add 9",           Command::Enemies,         9, true);
    testProtoCommand(a, "show minefield 150",      Command::ShowMinefield, 150, true /* sic! happens to be recognized as full command. */);
    testProtoCommand(a, "show minefield 150 3",    Command::ShowMinefield, 150, true);
    testProtoCommand(a, "show ship 499",           Command::ShowShip,      499, true /* sic! happens to be recognized as full command. */);
    testProtoCommand(a, "show ship 499 1 2 3",     Command::ShowShip,      499, true);
    testProtoCommand(a, "show planet 363",         Command::ShowPlanet,    363, true /* sic! happens to be recognized as full command. */);
    testProtoCommand(a, "show planet 363 9",       Command::ShowPlanet,    363, true);
    testProtoCommand(a, "refit 9",                 Command::Refit,           9, true /* sic! happens to be recognized as full command. */);
    testProtoCommand(a, "refit 9 8 8 8",           Command::Refit,           9, true);
}

/** Test getAffectedShip(). */
AFL_TEST("game.v3.Command:getAffectedShip", a)
{
    a.checkEqual("01", Command(Command::Language,      0, "en").getAffectedShip(), 0);
    a.checkEqual("02", Command(Command::SendConfig,    0, ""  ).getAffectedShip(), 0);
    a.checkEqual("03", Command(Command::SendRaceNames, 9, ""  ).getAffectedShip(), 0);
    a.checkEqual("04", Command(Command::SetRaceName,   1, "Ho").getAffectedShip(), 0);
    a.checkEqual("05", Command(Command::Filter,        0, "no").getAffectedShip(), 0);
    a.checkEqual("06", Command(Command::ConfigAlly,    9, "+c").getAffectedShip(), 0);
    a.checkEqual("07", Command(Command::AddDropAlly,   9, "a" ).getAffectedShip(), 0);
    a.checkEqual("08", Command(Command::GiveShip,     12, "11").getAffectedShip(), 12);
    a.checkEqual("09", Command(Command::GivePlanet,   17, "10").getAffectedShip(), 0);
    a.checkEqual("10", Command(Command::RemoteControl, 4, "a" ).getAffectedShip(), 4);
    a.checkEqual("11", Command(Command::RemoteDefault, 0, "d" ).getAffectedShip(), 0);
    a.checkEqual("12", Command(Command::BeamUp,       77, "M7").getAffectedShip(), 77);
    a.checkEqual("13", Command(Command::TAlliance,     0, "ff").getAffectedShip(), 0);
    a.checkEqual("14", Command(Command::SendFCodes,    0, ""  ).getAffectedShip(), 0);
    a.checkEqual("15", Command(Command::SendFile,      0, "ab").getAffectedShip(), 0);
    a.checkEqual("16", Command(Command::Enemies,       4, "a" ).getAffectedShip(), 0);
    a.checkEqual("17", Command(Command::Unload,       33, "$5").getAffectedShip(), 33);
    a.checkEqual("18", Command(Command::Transfer,    150, "N3").getAffectedShip(), 150);
    a.checkEqual("19", Command(Command::ShowShip,    259, "3" ).getAffectedShip(), 259);
    a.checkEqual("20", Command(Command::ShowPlanet,  149, "4" ).getAffectedShip(), 0);
    a.checkEqual("21", Command(Command::ShowMinefield, 1, "5" ).getAffectedShip(), 0);
    a.checkEqual("22", Command(Command::Refit,       451, "12").getAffectedShip(), 451);
    a.checkEqual("23", Command(Command::Other,         0, "Yo").getAffectedShip(), 0);

    a.checkEqual("31", Command(Command::GiveShip, 12, "11").getAffectedUnit().getType(), game::Reference::Ship);
    a.checkEqual("32", Command(Command::GiveShip, 12, "11").getAffectedUnit().getId(), 12);

    a.checkEqual("41", Command(Command::SendFCodes, 0, "").getAffectedUnit().isSet(), false);
}

/** Test getAffectedPlanet(). */
AFL_TEST("game.v3.Command:getAffectedPlanet", a)
{
    a.checkEqual("01", Command(Command::Language,      0, "en").getAffectedPlanet(), 0);
    a.checkEqual("02", Command(Command::SendConfig,    0, ""  ).getAffectedPlanet(), 0);
    a.checkEqual("03", Command(Command::SendRaceNames, 9, ""  ).getAffectedPlanet(), 0);
    a.checkEqual("04", Command(Command::SetRaceName,   1, "Ho").getAffectedPlanet(), 0);
    a.checkEqual("05", Command(Command::Filter,        0, "no").getAffectedPlanet(), 0);
    a.checkEqual("06", Command(Command::ConfigAlly,    9, "+c").getAffectedPlanet(), 0);
    a.checkEqual("07", Command(Command::AddDropAlly,   9, "a" ).getAffectedPlanet(), 0);
    a.checkEqual("08", Command(Command::GiveShip,     12, "11").getAffectedPlanet(), 0);
    a.checkEqual("09", Command(Command::GivePlanet,   17, "10").getAffectedPlanet(), 17);
    a.checkEqual("10", Command(Command::RemoteControl, 4, "a" ).getAffectedPlanet(), 0);
    a.checkEqual("11", Command(Command::RemoteDefault, 0, "d" ).getAffectedPlanet(), 0);
    a.checkEqual("12", Command(Command::BeamUp,       77, "M7").getAffectedPlanet(), 0);
    a.checkEqual("13", Command(Command::TAlliance,     0, "ff").getAffectedPlanet(), 0);
    a.checkEqual("14", Command(Command::SendFCodes,    0, ""  ).getAffectedPlanet(), 0);
    a.checkEqual("15", Command(Command::SendFile,      0, "ab").getAffectedPlanet(), 0);
    a.checkEqual("16", Command(Command::Enemies,       4, "a" ).getAffectedPlanet(), 0);
    a.checkEqual("17", Command(Command::Unload,       33, "$5").getAffectedPlanet(), 0);
    a.checkEqual("18", Command(Command::Transfer,    150, "N3").getAffectedPlanet(), 0);
    a.checkEqual("19", Command(Command::ShowShip,    259, "3" ).getAffectedPlanet(), 0);
    a.checkEqual("20", Command(Command::ShowPlanet,  149, "4" ).getAffectedPlanet(), 149);
    a.checkEqual("21", Command(Command::ShowMinefield, 1, "5" ).getAffectedPlanet(), 0);
    a.checkEqual("22", Command(Command::Refit,       451, "12").getAffectedPlanet(), 0);
    a.checkEqual("23", Command(Command::Other,         0, "Yo").getAffectedPlanet(), 0);

    a.checkEqual("31", Command(Command::GivePlanet, 12, "11").getAffectedUnit().getType(), game::Reference::Planet);
    a.checkEqual("32", Command(Command::GivePlanet, 17, "10").getAffectedUnit().getId(), 17);
}

/** Test getAffectedMinefield(). */
AFL_TEST("game.v3.Command:getAffectedMinefield", a)
{
    a.checkEqual("01", Command(Command::Language,      0, "en").getAffectedMinefield(), 0);
    a.checkEqual("02", Command(Command::SendConfig,    0, ""  ).getAffectedMinefield(), 0);
    a.checkEqual("03", Command(Command::SendRaceNames, 9, ""  ).getAffectedMinefield(), 0);
    a.checkEqual("04", Command(Command::SetRaceName,   1, "Ho").getAffectedMinefield(), 0);
    a.checkEqual("05", Command(Command::Filter,        0, "no").getAffectedMinefield(), 0);
    a.checkEqual("06", Command(Command::ConfigAlly,    9, "+c").getAffectedMinefield(), 0);
    a.checkEqual("07", Command(Command::AddDropAlly,   9, "a" ).getAffectedMinefield(), 0);
    a.checkEqual("08", Command(Command::GiveShip,     12, "11").getAffectedMinefield(), 0);
    a.checkEqual("09", Command(Command::GivePlanet,   17, "10").getAffectedMinefield(), 0);
    a.checkEqual("10", Command(Command::RemoteControl, 4, "a" ).getAffectedMinefield(), 0);
    a.checkEqual("11", Command(Command::RemoteDefault, 0, "d" ).getAffectedMinefield(), 0);
    a.checkEqual("12", Command(Command::BeamUp,       77, "M7").getAffectedMinefield(), 0);
    a.checkEqual("13", Command(Command::TAlliance,     0, "ff").getAffectedMinefield(), 0);
    a.checkEqual("14", Command(Command::SendFCodes,    0, ""  ).getAffectedMinefield(), 0);
    a.checkEqual("15", Command(Command::SendFile,      0, "ab").getAffectedMinefield(), 0);
    a.checkEqual("16", Command(Command::Enemies,       4, "a" ).getAffectedMinefield(), 0);
    a.checkEqual("17", Command(Command::Unload,       33, "$5").getAffectedMinefield(), 0);
    a.checkEqual("18", Command(Command::Transfer,    150, "N3").getAffectedMinefield(), 0);
    a.checkEqual("19", Command(Command::ShowShip,    259, "3" ).getAffectedMinefield(), 0);
    a.checkEqual("20", Command(Command::ShowPlanet,  149, "4" ).getAffectedMinefield(), 0);
    a.checkEqual("21", Command(Command::ShowMinefield, 1, "5" ).getAffectedMinefield(), 1);
    a.checkEqual("22", Command(Command::Refit,       451, "12").getAffectedMinefield(), 0);
    a.checkEqual("23", Command(Command::Other,         0, "Yo").getAffectedMinefield(), 0);

    a.checkEqual("31", Command(Command::ShowMinefield, 300, "4").getAffectedUnit().getType(), game::Reference::Minefield);
    a.checkEqual("32", Command(Command::ShowMinefield, 300, "4").getAffectedUnit().getId(), 300);
}

/** Test ordering constraints. */
AFL_TEST("game.v3.Command:getCommandOrder", a)
{
    // SetRaceName then SendRaceName
    a.checkLessThan("01. set before send",      Command::getCommandOrder(Command::SetRaceName), Command::getCommandOrder(Command::SendRaceNames));

    // Filter,Language then SendConfig
    a.checkLessThan("11. filter before send",   Command::getCommandOrder(Command::Filter),      Command::getCommandOrder(Command::SendConfig));
    a.checkLessThan("12. language before send", Command::getCommandOrder(Command::Language),    Command::getCommandOrder(Command::SendConfig));

    // AddDropAlly then ConfigAlly then RemoteControl
    a.checkLessThan("21. add before config",    Command::getCommandOrder(Command::AddDropAlly), Command::getCommandOrder(Command::ConfigAlly));
    a.checkLessThan("22. config before remote", Command::getCommandOrder(Command::ConfigAlly),  Command::getCommandOrder(Command::RemoteControl));
}

/** Test getCommandInfo(). */
AFL_TEST("game.v3.Command:getCommandInfo", a)
{
    afl::string::NullTranslator tx;

    for (int i = 0; i <= Command::Other; ++i) {
        a.check("01", !Command::getCommandInfo(Command::Type(i), tx).empty());
    }

    a.check("11", !Command::getCommandInfo(Command::GiveShip, tx).empty());
    a.check("12", !Command::getCommandInfo(Command::Other, tx).empty());
}
