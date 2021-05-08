/**
  *  \file u/t_game_spec_standardcomponentnameprovider.cpp
  *  \brief Test for game::spec::StandardComponentNameProvider
  */

#include "game/spec/standardcomponentnameprovider.hpp"

#include "t_game_spec.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/sys/log.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test it. */
void
TestGameSpecStandardComponentNameProvider::testIt()
{
    // Environment
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    static const char FILE1[] =
        "[hulls.short]\n"
        "# a = b\n"
        "emerald class cruiser = emerald\n"
        "[something else]\n"
        "whatever\n"
        "[engines.short]\n"
        "warp drive = wd\n"
        "[error\n"
        "warp drive = wd40\n";
    static const char FILE2[] =
        "[beams.short]\n"
        "Laser=Las\n"
        "Nerf=Ner\n"
        "Error\n"
        "[TORPS.SHORT]\n"
        "Mark 1 Photon = Mk1\n";
    dir->addStream("names.usr", *new afl::io::ConstMemoryStream(afl::string::toBytes(FILE1)));
    dir->addStream("names.cc", *new afl::io::ConstMemoryStream(afl::string::toBytes(FILE2)));

    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Test it
    game::spec::StandardComponentNameProvider testee;
    testee.load(*dir, tx, log);

    // Verify result
    TS_ASSERT_EQUALS(testee.getName(game::spec::ComponentNameProvider::Hull, 7, "emerald class cruiser"), "emerald class cruiser");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Hull, 7, "emerald class cruiser", ""), "emerald");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Hull, 7, "EMERALD CLASS CRUISER", ""), "emerald");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Hull, 7, "emerald class cruiser", "emmy"), "emmy");

    TS_ASSERT_EQUALS(testee.getName(game::spec::ComponentNameProvider::Engine, 3, "warp drive"), "warp drive");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Engine, 3, "warp drive", ""), "wd");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Engine, 3, "Warp Drive", ""), "wd");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Engine, 3, "warp drive", "wa"), "wa");

    TS_ASSERT_EQUALS(testee.getName(game::spec::ComponentNameProvider::Beam, 3, "Laser"), "Laser");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Beam, 3, "Laser", ""), "Las");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Beam, 3, "LASER", ""), "Las");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Beam, 4, "Nerf", ""), "Ner");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Beam, 4, "Nerf Gun", ""), "Nerf Gun");

    TS_ASSERT_EQUALS(testee.getName(game::spec::ComponentNameProvider::Torpedo, 8, "Mark 1 Photon"), "Mark 1 Photon");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Torpedo, 8, "Mark 1 Photon", ""), "Mk1");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Torpedo, 8, "Mark 1 Photon", ""), "Mk1");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Torpedo, 8, "Mark 1 Photon", "M1P"), "M1P");

    // Looking up name of wrong type:
    TS_ASSERT_EQUALS(testee.getName(game::spec::ComponentNameProvider::Hull, 3, "Laser"), "Laser");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Hull, 3, "Laser", ""), "Laser");

    // Comments were ignored, so there is no mapping "# a" -> "b"
    TS_ASSERT_EQUALS(testee.getName(game::spec::ComponentNameProvider::Hull, 9, "# a"), "# a");

    // Clear resets
    testee.clear();
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Engine, 3, "warp drive", ""), "warp drive");
}

/** Test language specific. */
void
TestGameSpecStandardComponentNameProvider::testLanguage()
{
    // Environment
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    static const char FILE1[] =
        "[hulls.short]\n"
        "small deep space freighter = Kleiner Frachter\n";
    dir->addStream("names_de.cc", *new afl::io::ConstMemoryStream(afl::string::toBytes(FILE1)));

    class Translator : public afl::string::Translator {
        virtual String_t translate(afl::string::ConstStringMemory_t in) const
            {
                String_t result = afl::string::fromMemory(in);
                if (result == "{languageCode}") {
                    result = "de";
                }
                return result;
            }
    };
    Translator tx;
    afl::sys::Log log;

    // Test it
    game::spec::StandardComponentNameProvider testee;
    testee.load(*dir, tx, log);

    // Verify result
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Hull, 7, "Small Deep Space Freighter", ""), "Kleiner Frachter");
}

