/**
  *  \file test/game/spec/standardcomponentnameprovidertest.cpp
  *  \brief Test for game::spec::StandardComponentNameProvider
  */

#include "game/spec/standardcomponentnameprovider.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"

/** Test it. */
AFL_TEST("game.spec.StandardComponentNameProvider:normal", a)
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
    a.checkEqual("01", testee.getName(game::spec::ComponentNameProvider::Hull, 7, "emerald class cruiser"), "emerald class cruiser");
    a.checkEqual("02", testee.getShortName(game::spec::ComponentNameProvider::Hull, 7, "emerald class cruiser", ""), "emerald");
    a.checkEqual("03", testee.getShortName(game::spec::ComponentNameProvider::Hull, 7, "EMERALD CLASS CRUISER", ""), "emerald");
    a.checkEqual("04", testee.getShortName(game::spec::ComponentNameProvider::Hull, 7, "emerald class cruiser", "emmy"), "emmy");

    a.checkEqual("11", testee.getName(game::spec::ComponentNameProvider::Engine, 3, "warp drive"), "warp drive");
    a.checkEqual("12", testee.getShortName(game::spec::ComponentNameProvider::Engine, 3, "warp drive", ""), "wd");
    a.checkEqual("13", testee.getShortName(game::spec::ComponentNameProvider::Engine, 3, "Warp Drive", ""), "wd");
    a.checkEqual("14", testee.getShortName(game::spec::ComponentNameProvider::Engine, 3, "warp drive", "wa"), "wa");

    a.checkEqual("21", testee.getName(game::spec::ComponentNameProvider::Beam, 3, "Laser"), "Laser");
    a.checkEqual("22", testee.getShortName(game::spec::ComponentNameProvider::Beam, 3, "Laser", ""), "Las");
    a.checkEqual("23", testee.getShortName(game::spec::ComponentNameProvider::Beam, 3, "LASER", ""), "Las");
    a.checkEqual("24", testee.getShortName(game::spec::ComponentNameProvider::Beam, 4, "Nerf", ""), "Ner");
    a.checkEqual("25", testee.getShortName(game::spec::ComponentNameProvider::Beam, 4, "Nerf Gun", ""), "Nerf Gun");

    a.checkEqual("31", testee.getName(game::spec::ComponentNameProvider::Torpedo, 8, "Mark 1 Photon"), "Mark 1 Photon");
    a.checkEqual("32", testee.getShortName(game::spec::ComponentNameProvider::Torpedo, 8, "Mark 1 Photon", ""), "Mk1");
    a.checkEqual("33", testee.getShortName(game::spec::ComponentNameProvider::Torpedo, 8, "Mark 1 Photon", ""), "Mk1");
    a.checkEqual("34", testee.getShortName(game::spec::ComponentNameProvider::Torpedo, 8, "Mark 1 Photon", "M1P"), "M1P");

    // Looking up name of wrong type:
    a.checkEqual("41", testee.getName(game::spec::ComponentNameProvider::Hull, 3, "Laser"), "Laser");
    a.checkEqual("42", testee.getShortName(game::spec::ComponentNameProvider::Hull, 3, "Laser", ""), "Laser");

    // Comments were ignored, so there is no mapping "# a" -> "b"
    a.checkEqual("51", testee.getName(game::spec::ComponentNameProvider::Hull, 9, "# a"), "# a");

    // Clear resets
    testee.clear();
    a.checkEqual("61", testee.getShortName(game::spec::ComponentNameProvider::Engine, 3, "warp drive", ""), "warp drive");
}

/** Test language specific. */
AFL_TEST("game.spec.StandardComponentNameProvider:language", a)
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
    a.checkEqual("01", testee.getShortName(game::spec::ComponentNameProvider::Hull, 7, "Small Deep Space Freighter", ""), "Kleiner Frachter");
}
