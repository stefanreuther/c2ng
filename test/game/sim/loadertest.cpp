/**
  *  \file test/game/sim/loadertest.cpp
  *  \brief Test for game::sim::Loader
  *
  *  These test cases use actual .ccb files created by PCC/CCBSim/PCC2/PlayVCR.
  */

#include "game/sim/loader.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/test/files.hpp"

namespace {
    /* Prepare a setup for the "save" tests */
    void prepare(game::sim::Setup& setup)
    {
        game::sim::Ship* sh = setup.addShip();
        sh->setName("Boat");
        sh->setOwner(3);
        sh->setTorpedoType(7);
        sh->setNumLaunchers(4);
        sh->setAmmo(180);
        sh->setCrew(17);
        sh->setFriendlyCode("abc");
        sh->setEngineType(8);

        game::sim::Planet* pl = setup.addPlanet();
        pl->setOwner(4);
        pl->setDefense(61);
        pl->setFriendlyCode("xyz");
        pl->setBaseBeamTech(6);
        pl->setBaseTorpedoTech(9);
        pl->setBaseDefense(12);
    }
}

/** Test V0 file format (PCC 0.99.10). */
AFL_TEST("game.sim.Loader:load:V0", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);
    afl::io::ConstMemoryStream stream(game::test::getSimFileV0());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    a.checkEqual("01. getNumShips",   result.getNumShips(), 2U);
    a.checkEqual("02. getNumObjects", result.getNumObjects(), 3U);
    a.check("03. hasPlanet",          result.hasPlanet());

    // First ship
    game::sim::Ship* sh = result.getShip(0);
    a.check("11. ship 0", sh);
    a.checkEqual("12. getName",            sh->getName(), "C.C.S.S. Joker");
    a.checkEqual("13. getHullType",        sh->getHullType(), 61);  // Emerald
    a.checkEqual("14. getOwner",           sh->getOwner(), 7);
    a.checkEqual("15. getId",              sh->getId(), 117);
    a.checkEqual("16. getFriendlyCode",    sh->getFriendlyCode(), "NTP");
    a.checkEqual("17. getDamage",          sh->getDamage(), 0);
    a.checkEqual("18. getCrew",            sh->getCrew(), 258);
    // a.checkEqual("19. getMass", sh->getMass(), 180); // not set, needs ship list
    a.checkEqual("20. getNumBeams",        sh->getNumBeams(), 8);
    a.checkEqual("21. getBeamType",        sh->getBeamType(), 7);
    a.checkEqual("22. getNumLaunchers",    sh->getNumLaunchers(), 3);
    a.checkEqual("23. getTorpedoType",     sh->getTorpedoType(), 10);
    a.checkEqual("24. getNumBays",         sh->getNumBays(), 0);
    a.checkEqual("25. getAmmo",            sh->getAmmo(), 40);
    a.checkEqual("26. getEngineType",      sh->getEngineType(), 7);
    a.checkEqual("27. getAggressiveness",  sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    a.checkEqual("28. getFlags",           sh->getFlags(), 0);
    a.checkEqual("29. getInterceptId",     sh->getInterceptId(), 0);
    a.checkEqual("30. getExperienceLevel", sh->getExperienceLevel(), 0);

    // Second ship
    sh = result.getShip(1);
    a.check("31. ship 1", sh);
    a.checkEqual("32. getName",            sh->getName(), "C.C.S.S. Claudrin II");
    a.checkEqual("33. getHullType",        sh->getHullType(), 22);  // LCC
    a.checkEqual("34. getOwner",           sh->getOwner(), 7);
    a.checkEqual("35. getId",              sh->getId(), 9);
    a.checkEqual("36. getFriendlyCode",    sh->getFriendlyCode(), "NTP");
    a.checkEqual("37. getDamage",          sh->getDamage(), 0);
    a.checkEqual("38. getCrew",            sh->getCrew(), 430);
    a.checkEqual("39. getNumBeams",        sh->getNumBeams(), 4);
    a.checkEqual("40. getBeamType",        sh->getBeamType(), 6);
    a.checkEqual("41. getNumLaunchers",    sh->getNumLaunchers(), 3);
    a.checkEqual("42. getTorpedoType",     sh->getTorpedoType(), 6);
    a.checkEqual("43. getNumBays",         sh->getNumBays(), 0);
    a.checkEqual("44. getAmmo",            sh->getAmmo(), 50);
    a.checkEqual("45. getEngineType",      sh->getEngineType(), 9);
    a.checkEqual("46. getAggressiveness",  sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    a.checkEqual("47. getFlags",           sh->getFlags(), 0);
    a.checkEqual("48. getInterceptId",     sh->getInterceptId(), 0);
    a.checkEqual("49. getExperienceLevel", sh->getExperienceLevel(), 0);

    // Planet
    game::sim::Planet* pl = result.getPlanet();
    a.check("51. planet", pl);
    a.checkEqual("52. getId",                 pl->getId(), 1);
    a.checkEqual("53. getOwner",              pl->getOwner(), 2);
    a.checkEqual("54. getFriendlyCode",       pl->getFriendlyCode(), "i9m");
    a.checkEqual("55. getDefense",            pl->getDefense(), 62);
    a.checkEqual("56. getFlags",              pl->getFlags(), 0);
    a.checkEqual("57. getExperienceLevel",    pl->getExperienceLevel(), 0);
    a.checkEqual("58. getBaseBeamTech",       pl->getBaseBeamTech(), 0);
    // a.checkEqual("59. getNumBaseFighters", pl->getNumBaseFighters(), 0); // not set
    // a.checkEqual("60. getBaseDefense",     pl->getBaseDefense(), 0);     // not set
    // a.checkEqual("61. getBaseTorpedoTech", pl->getBaseTorpedoTech(), 0); // not set
}

/** Test V1 file format (PCC 1.0). */
AFL_TEST("game.sim.Loader:load:V1", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);
    afl::io::ConstMemoryStream stream(game::test::getSimFileV1());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    a.checkEqual("01. getNumShips",   result.getNumShips(), 1U);
    a.checkEqual("02. getNumObjects", result.getNumObjects(), 2U);
    a.check("03. hasPlanet",          result.hasPlanet());

    // Ship
    game::sim::Ship* sh = result.getShip(0);
    a.check("11. ship 0", sh);
    a.checkEqual("12. getName",            sh->getName(), "Ship 201");
    a.checkEqual("13. getHullType",        sh->getHullType(), 76);  // SSC
    a.checkEqual("14. getOwner",           sh->getOwner(), 8);
    a.checkEqual("15. getId",              sh->getId(), 201);
    a.checkEqual("16. getFriendlyCode",    sh->getFriendlyCode(), "?""?""?");
    a.checkEqual("17. getDamage",          sh->getDamage(), 0);
    a.checkEqual("18. getCrew",            sh->getCrew(), 352);
    // a.checkEqual("19. getMass",         sh->getMass(), 180); // not set, needs ship list
    a.checkEqual("20. getNumBeams",        sh->getNumBeams(), 6);
    a.checkEqual("21. getBeamType",        sh->getBeamType(), 6);
    a.checkEqual("22. getNumLaunchers",    sh->getNumLaunchers(), 0);
    a.checkEqual("23. getTorpedoType",     sh->getTorpedoType(), 0);
    a.checkEqual("24. getNumBays",         sh->getNumBays(), 4);
    a.checkEqual("25. getAmmo",            sh->getAmmo(), 85);
    a.checkEqual("26. getEngineType",      sh->getEngineType(), 9);
    a.checkEqual("27. getAggressiveness",  sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    a.checkEqual("28. getFlags",           sh->getFlags(), 0);
    a.checkEqual("29. getInterceptId",     sh->getInterceptId(), 0);
    a.checkEqual("30. getExperienceLevel", sh->getExperienceLevel(), 0);

    // Planet
    game::sim::Planet* pl = result.getPlanet();
    a.check("31. planet", pl);
    a.checkEqual("32. getId",              pl->getId(), 459);
    a.checkEqual("33. getOwner",           pl->getOwner(), 6);
    a.checkEqual("34. getFriendlyCode",    pl->getFriendlyCode(), "NUK");
    a.checkEqual("35. getDefense",         pl->getDefense(), 129);
    a.checkEqual("36. getFlags",           pl->getFlags(), 0);
    a.checkEqual("37. getExperienceLevel", pl->getExperienceLevel(), 0);
    a.checkEqual("38. getBaseBeamTech",    pl->getBaseBeamTech(), 1);
    a.checkEqual("39. getNumBaseFighters", pl->getNumBaseFighters(), 22);
    a.checkEqual("40. getBaseDefense",     pl->getBaseDefense(), 150);
    a.checkEqual("41. getBaseTorpedoTech", pl->getBaseTorpedoTech(), 1);
}

/** Test V2 file format (PCC 1.0.7). */
AFL_TEST("game.sim.Loader:load:V2", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);
    afl::io::ConstMemoryStream stream(game::test::getSimFileV2());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    a.checkEqual("01. getNumShips",   result.getNumShips(), 2U);
    a.checkEqual("02. getNumObjects", result.getNumObjects(), 3U);
    a.check("03. hasPlanet",          result.hasPlanet());

    // First ship
    game::sim::Ship* sh = result.getShip(0);
    a.check("11. ship 0", sh);
    a.checkEqual("12. getName",            sh->getName(), "Ship 4");
    a.checkEqual("13. getHullType",        sh->getHullType(), 1);  // Outrider
    a.checkEqual("14. getOwner",           sh->getOwner(), 12);
    a.checkEqual("15. getId",              sh->getId(), 4);
    a.checkEqual("16. getFriendlyCode",    sh->getFriendlyCode(), "?""?""?");
    a.checkEqual("17. getDamage",          sh->getDamage(), 0);
    a.checkEqual("18. getCrew",            sh->getCrew(), 180);
    // a.checkEqual("19. getMass",         sh->getMass(), 75); // not set, needs ship list
    a.checkEqual("20. getNumBeams",        sh->getNumBeams(), 1);
    a.checkEqual("21. getBeamType",        sh->getBeamType(), 10);
    a.checkEqual("22. getNumLaunchers",    sh->getNumLaunchers(), 0);
    a.checkEqual("23. getTorpedoType",     sh->getTorpedoType(), 0);
    a.checkEqual("24. getNumBays",         sh->getNumBays(), 0);
    a.checkEqual("25. getAmmo",            sh->getAmmo(), 0);
    a.checkEqual("26. getEngineType",      sh->getEngineType(), 9);
    a.checkEqual("27. getAggressiveness",  sh->getAggressiveness(), game::sim::Ship::agg_NoFuel);
    a.checkEqual("28. getFlags",           sh->getFlags(), 0);
    a.checkEqual("29. getInterceptId",     sh->getInterceptId(), 0);
    a.checkEqual("30. getExperienceLevel", sh->getExperienceLevel(), 0);

    // Second ship
    sh = result.getShip(1);
    a.check("31. ship 1", sh);
    a.checkEqual("32. getName",            sh->getName(), "Ship 5");
    a.checkEqual("33. getHullType",        sh->getHullType(), 73);  // Mig Scout
    a.checkEqual("34. getOwner",           sh->getOwner(), 8);
    a.checkEqual("35. getId",              sh->getId(), 5);
    a.checkEqual("36. getFriendlyCode",    sh->getFriendlyCode(), "123");
    a.checkEqual("37. getDamage",          sh->getDamage(), 0);
    a.checkEqual("38. getCrew",            sh->getCrew(), 10);
    a.checkEqual("39. getNumBeams",        sh->getNumBeams(), 2);
    a.checkEqual("40. getBeamType",        sh->getBeamType(), 10);
    a.checkEqual("41. getNumLaunchers",    sh->getNumLaunchers(), 0);
    a.checkEqual("42. getTorpedoType",     sh->getTorpedoType(), 0);
    a.checkEqual("43. getNumBays",         sh->getNumBays(), 0);
    a.checkEqual("44. getAmmo",            sh->getAmmo(), 0);
    a.checkEqual("45. getEngineType",      sh->getEngineType(), 9);
    a.checkEqual("46. getAggressiveness",  sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    a.checkEqual("47. getFlags",           sh->getFlags(), 0);
    a.checkEqual("48. getInterceptId",     sh->getInterceptId(), 0);
    a.checkEqual("49. getExperienceLevel", sh->getExperienceLevel(), 0);

    // Planet
    game::sim::Planet* pl = result.getPlanet();
    a.check("51. planet", pl);
    a.checkEqual("52. getId",                 pl->getId(), 1);
    a.checkEqual("53. getOwner",              pl->getOwner(), 12);
    a.checkEqual("54. getFriendlyCode",       pl->getFriendlyCode(), "NUK");
    a.checkEqual("55. getDefense",            pl->getDefense(), 10);
    a.checkEqual("56. getFlags",              pl->getFlags(), 0);
    a.checkEqual("57. getExperienceLevel",    pl->getExperienceLevel(), 0);
    a.checkEqual("58. getBaseBeamTech",       pl->getBaseBeamTech(), 0);
    // a.checkEqual("59. getNumBaseFighters", pl->getNumBaseFighters(), 0); // not set
    // a.checkEqual("60. getBaseDefense",     pl->getBaseDefense(), 0);     // not set
    // a.checkEqual("61. getBaseTorpedoTech", pl->getBaseTorpedoTech(), 0); // not set
}

/** Test V3 file format (PCC 1.0.16, PCC2 1.99.2). */
AFL_TEST("game.sim.Loader:load:V3", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);
    afl::io::ConstMemoryStream stream(game::test::getSimFileV3());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    a.checkEqual("01. getNumShips",   result.getNumShips(), 3U);
    a.checkEqual("02. getNumObjects", result.getNumObjects(), 4U);
    a.check("03. hasPlanet",          result.hasPlanet());

    // First ship
    game::sim::Ship* sh = result.getShip(0);
    a.check("11. ship 0", sh);
    a.checkEqual("12. getName",            sh->getName(), "Ultra Elite Alien");
    a.checkEqual("13. getHullType",        sh->getHullType(), 1);  // Outrider
    a.checkEqual("14. getOwner",           sh->getOwner(), 12);
    a.checkEqual("15. getId",              sh->getId(), 1);
    a.checkEqual("16. getFriendlyCode",    sh->getFriendlyCode(), "?""?""?");
    a.checkEqual("17. getDamage",          sh->getDamage(), 0);
    a.checkEqual("18. getCrew",            sh->getCrew(), 58);
    // a.checkEqual("19. getMass", sh->getMass(), 75); // not set, needs ship list
    a.checkEqual("20. getNumBeams",        sh->getNumBeams(), 1);
    a.checkEqual("21. getBeamType",        sh->getBeamType(), 10);
    a.checkEqual("22. getNumLaunchers",    sh->getNumLaunchers(), 0);
    a.checkEqual("23. getTorpedoType",     sh->getTorpedoType(), 0);
    a.checkEqual("24. getNumBays",         sh->getNumBays(), 0);
    a.checkEqual("25. getAmmo",            sh->getAmmo(), 0);
    a.checkEqual("26. getEngineType",      sh->getEngineType(), 9);
    a.checkEqual("27. getAggressiveness",  sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    a.checkEqual("28. getFlags",           sh->getFlags(), game::sim::Object::fl_CommanderSet + game::sim::Object::fl_Commander);
    a.checkEqual("29. getInterceptId",     sh->getInterceptId(), 0);
    a.checkEqual("30. getExperienceLevel", sh->getExperienceLevel(), 4);

    // Second ship
    sh = result.getShip(1);
    a.check("31. ship 1", sh);
    a.checkEqual("32. getName",            sh->getName(), "Recruit Alien");
    a.checkEqual("33. getHullType",        sh->getHullType(), 1);  // Outrider
    a.checkEqual("34. getOwner",           sh->getOwner(), 12);
    a.checkEqual("35. getId",              sh->getId(), 2);
    a.checkEqual("36. getFriendlyCode",    sh->getFriendlyCode(), "?""?""?");
    a.checkEqual("37. getDamage",          sh->getDamage(), 0);
    a.checkEqual("38. getCrew",            sh->getCrew(), 58);
    // a.checkEqual("39. getMass", sh->getMass(), 75); // not set, needs ship list
    a.checkEqual("40. getNumBeams",        sh->getNumBeams(), 1);
    a.checkEqual("41. getBeamType",        sh->getBeamType(), 10);
    a.checkEqual("42. getNumLaunchers",    sh->getNumLaunchers(), 0);
    a.checkEqual("43. getTorpedoType",     sh->getTorpedoType(), 0);
    a.checkEqual("44. getNumBays",         sh->getNumBays(), 0);
    a.checkEqual("45. getAmmo",            sh->getAmmo(), 0);
    a.checkEqual("46. getEngineType",      sh->getEngineType(), 9);
    a.checkEqual("47. getAggressiveness",  sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    a.checkEqual("48. getFlags",           sh->getFlags(), 0);
    a.checkEqual("49. getInterceptId",     sh->getInterceptId(), 0);
    a.checkEqual("50. getExperienceLevel", sh->getExperienceLevel(), 0);

    // Third ship
    sh = result.getShip(2);
    a.check("51. ship 2", sh);
    a.checkEqual("52. getName",            sh->getName(), "Recruit Borg");
    a.checkEqual("53. getHullType",        sh->getHullType(), 58);  // Quietus
    a.checkEqual("54. getOwner",           sh->getOwner(), 6);
    a.checkEqual("55. getId",              sh->getId(), 3);
    a.checkEqual("56. getFriendlyCode",    sh->getFriendlyCode(), "?""?""?");
    a.checkEqual("57. getDamage",          sh->getDamage(), 0);
    a.checkEqual("58. getCrew",            sh->getCrew(), 517);
    a.checkEqual("59. getNumBeams",        sh->getNumBeams(), 9);
    a.checkEqual("60. getBeamType",        sh->getBeamType(), 10);
    a.checkEqual("61. getNumLaunchers",    sh->getNumLaunchers(), 9);
    a.checkEqual("62. getTorpedoType",     sh->getTorpedoType(), 10);
    a.checkEqual("63. getNumBays",         sh->getNumBays(), 0);
    a.checkEqual("64. getAmmo",            sh->getAmmo(), 260);
    a.checkEqual("65. getEngineType",      sh->getEngineType(), 9);
    a.checkEqual("66. getAggressiveness",  sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    a.checkEqual("67. getFlags",           sh->getFlags(), 0);
    a.checkEqual("68. getInterceptId",     sh->getInterceptId(), 0);
    a.checkEqual("69. getExperienceLevel", sh->getExperienceLevel(), 0);

    // Planet
    game::sim::Planet* pl = result.getPlanet();
    a.check("71. planet", pl);
    a.checkEqual("72. getId",              pl->getId(), 1);
    a.checkEqual("73. getOwner",           pl->getOwner(), 12);
    a.checkEqual("74. getFriendlyCode",    pl->getFriendlyCode(), "?""?""?");
    a.checkEqual("75. getDefense",         pl->getDefense(), 10);
    a.checkEqual("76. getFlags",           pl->getFlags(), 0);
    a.checkEqual("77. getExperienceLevel", pl->getExperienceLevel(), 0);
    a.checkEqual("78. getBaseBeamTech",    pl->getBaseBeamTech(), 0);
}

/** Test V4 file format (PCC 1.1.11.6, PCC2 1.99.2). */
AFL_TEST("game.sim.Loader:load:V4", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);
    afl::io::ConstMemoryStream stream(game::test::getSimFileV4());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    a.checkEqual("01. getNumShips",   result.getNumShips(), 1U);
    a.checkEqual("02. getNumObjects", result.getNumObjects(), 1U);
    a.check("03. hasPlanet",         !result.hasPlanet());

    // The ship
    game::sim::Ship* sh = result.getShip(0);
    a.check("11. ship 0", sh);
    a.checkEqual("12. getName",                     sh->getName(), "Ship 1");
    a.checkEqual("13. getHullType",                 sh->getHullType(), 1);  // Outrider
    a.checkEqual("14. getOwner",                    sh->getOwner(), 12);
    a.checkEqual("15. getId",                       sh->getId(), 1);
    a.checkEqual("16. getFriendlyCode",             sh->getFriendlyCode(), "?""?""?");
    a.checkEqual("17. getDamage",                   sh->getDamage(), 0);
    a.checkEqual("18. getCrew",                     sh->getCrew(), 58);
    // a.checkEqual("19. getMass", sh->getMass(), 75); // not set, needs ship list
    a.checkEqual("20. getNumBeams",                 sh->getNumBeams(), 1);
    a.checkEqual("21. getBeamType",                 sh->getBeamType(), 10);
    a.checkEqual("22. getNumLaunchers",             sh->getNumLaunchers(), 0);
    a.checkEqual("23. getTorpedoType",              sh->getTorpedoType(), 0);
    a.checkEqual("24. getNumBays",                  sh->getNumBays(), 0);
    a.checkEqual("25. getAmmo",                     sh->getAmmo(), 0);
    a.checkEqual("26. getEngineType",               sh->getEngineType(), 9);
    a.checkEqual("27. getAggressiveness",           sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    a.checkEqual("28. getFlags",                    sh->getFlags(), game::sim::Ship::fl_RatingOverride);
    a.checkEqual("29. getInterceptId",              sh->getInterceptId(), 0);
    a.checkEqual("30. getExperienceLevel",          sh->getExperienceLevel(), 0);
    a.checkEqual("31. getFlakRatingOverride",       sh->getFlakRatingOverride(), 240);
    a.checkEqual("32. getFlakCompensationOverride", sh->getFlakCompensationOverride(), 23);
}

/** Test V5 file format (PCC2 1.99.22). */
AFL_TEST("game.sim.Loader:load:V5", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);
    afl::io::ConstMemoryStream stream(game::test::getSimFileV5());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    a.checkEqual("01. getNumShips",   result.getNumShips(), 2U);
    a.checkEqual("02. getNumObjects", result.getNumObjects(), 2U);
    a.check("03. hasPlanet",         !result.hasPlanet());

    // First ship
    game::sim::Ship* sh = result.getShip(0);
    a.check("11. ship 0", sh);
    a.checkEqual("12. getName",            sh->getName(), "Mike Oldfield");
    a.checkEqual("13. getHullType",        sh->getHullType(), 16);  // MDSF
    a.checkEqual("14. getOwner",           sh->getOwner(), 9);
    a.checkEqual("15. getId",              sh->getId(), 1);
    a.checkEqual("16. getFriendlyCode",    sh->getFriendlyCode(), "_{=");
    a.checkEqual("17. getDamage",          sh->getDamage(), 0);
    a.checkEqual("18. getCrew",            sh->getCrew(), 6);
    a.checkEqual("19. getNumBeams",        sh->getNumBeams(), 0);
    a.checkEqual("20. getBeamType",        sh->getBeamType(), 0);
    a.checkEqual("21. getNumLaunchers",    sh->getNumLaunchers(), 0);
    a.checkEqual("22. getTorpedoType",     sh->getTorpedoType(), 0);
    a.checkEqual("23. getNumBays",         sh->getNumBays(), 0);
    a.checkEqual("24. getAmmo",            sh->getAmmo(), 0);
    a.checkEqual("25. getEngineType",      sh->getEngineType(), 8);
    a.checkEqual("26. getAggressiveness",  sh->getAggressiveness(), game::sim::Ship::agg_Passive);
    a.checkEqual("27. getFlags",           sh->getFlags(), 0);
    a.checkEqual("28. getInterceptId",     sh->getInterceptId(), 0);
    a.checkEqual("29. getExperienceLevel", sh->getExperienceLevel(), 0);

    // Second ship
    sh = result.getShip(1);
    a.check("31. ship 1", sh);
    a.checkEqual("32. getName",            sh->getName(), "Ma Baker");
    a.checkEqual("33. getHullType",        sh->getHullType(), 17);  // LDSF
    a.checkEqual("34. getOwner",           sh->getOwner(), 9);
    a.checkEqual("35. getId",              sh->getId(), 6);
    a.checkEqual("36. getFriendlyCode",    sh->getFriendlyCode(), "4R{");
    a.checkEqual("37. getDamage",          sh->getDamage(), 0);
    a.checkEqual("38. getCrew",            sh->getCrew(), 102);
    a.checkEqual("39. getNumBeams",        sh->getNumBeams(), 0);
    a.checkEqual("40. getBeamType",        sh->getBeamType(), 0);
    a.checkEqual("41. getNumLaunchers",    sh->getNumLaunchers(), 0);
    a.checkEqual("42. getTorpedoType",     sh->getTorpedoType(), 0);
    a.checkEqual("43. getNumBays",         sh->getNumBays(), 0);
    a.checkEqual("44. getAmmo",            sh->getAmmo(), 0);
    a.checkEqual("45. getEngineType",      sh->getEngineType(), 9);
    a.checkEqual("46. getAggressiveness",  sh->getAggressiveness(), game::sim::Ship::agg_Passive);
    a.checkEqual("47. getFlags",           sh->getFlags(), game::sim::Object::fl_Elusive + game::sim::Object::fl_ElusiveSet);
    a.checkEqual("48. getInterceptId",     sh->getInterceptId(), 0);
    a.checkEqual("49. getExperienceLevel", sh->getExperienceLevel(), 0);
}

/** Test error behaviour. */
AFL_TEST("game.sim.Loader:load:error", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);
    game::sim::Setup result;

    // v0
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x73, 0x69, 0x6d, 0x1a, 0x02, 0x80, 0x43, 0x2e, 0x43, 0x2e,
        };
        afl::io::ConstMemoryStream stream(FILE);
        AFL_CHECK_THROWS(a("01. load V0"), testee.load(stream, result), afl::except::FileFormatException);
    }

    // v1
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x30, 0x1a, 0x01, 0x80, 0x53, 0x68,
        };
        afl::io::ConstMemoryStream stream(FILE);
        AFL_CHECK_THROWS(a("11. load V1"), testee.load(stream, result), afl::except::FileFormatException);
    }

    // v2
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x31, 0x1a, 0x02, 0x80, 0x53, 0x68,
        };
        afl::io::ConstMemoryStream stream(FILE);
        AFL_CHECK_THROWS(a("21. load V2"), testee.load(stream, result), afl::except::FileFormatException);
    }

    // v3
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x32, 0x1a, 0x03, 0x80, 0x55, 0x6c,
        };
        afl::io::ConstMemoryStream stream(FILE);
        AFL_CHECK_THROWS(a("31. load V3"), testee.load(stream, result), afl::except::FileFormatException);
    }

    // v4
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x33, 0x1a, 0x01, 0x00, 0x53, 0x68,
        };
        afl::io::ConstMemoryStream stream(FILE);
        AFL_CHECK_THROWS(a("41. load V4"), testee.load(stream, result), afl::except::FileFormatException);
    }

    // v5
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x34, 0x1a, 0x02, 0x00, 0x4d, 0x69,
        };
        afl::io::ConstMemoryStream stream(FILE);
        AFL_CHECK_THROWS(a("51. load V5"), testee.load(stream, result), afl::except::FileFormatException);
    }

    // truncated signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x34,
        };
        afl::io::ConstMemoryStream stream(FILE);
        AFL_CHECK_THROWS(a("61. truncated signature"), testee.load(stream, result), afl::except::FileFormatException);
    }

    // future signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x39, 0x1a,
        };
        afl::io::ConstMemoryStream stream(FILE);
        AFL_CHECK_THROWS(a("71. future signature"), testee.load(stream, result), afl::except::FileFormatException);
    }

    // bad signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x30, 0x00,
        };
        afl::io::ConstMemoryStream stream(FILE);
        AFL_CHECK_THROWS(a("81. bad signature"), testee.load(stream, result), afl::except::FileFormatException);
    }

    // bad signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43,
        };
        afl::io::ConstMemoryStream stream(FILE);
        AFL_CHECK_THROWS(a("91. bad signature"), testee.load(stream, result), afl::except::FileFormatException);
    }

    // empty file
    {
        afl::io::ConstMemoryStream stream((afl::base::ConstBytes_t()));
        AFL_CHECK_THROWS(a("101. empty file"), testee.load(stream, result), afl::except::FileFormatException);
    }
}

/** Test saving in default format (V3). */
AFL_TEST("game.sim.Loader:save:default", a)
{
    game::sim::Setup setup;
    prepare(setup);

    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);

    afl::io::InternalStream stream;
    testee.save(stream, setup);

    static const uint8_t EXPECTED[] = {
        // Header (10)
        'C','C','b','s','i','m','2',26,1,0x80,

        // Ship (57)
        'B','o','a','t',32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,    // 20
        0,0, 17,0, 1,0, 3,0, 0,0, 0,0, 0,0, 0,0, 7,0, 180,0, 4,0,           // 22
        8,0, 0,0, 100,0, 'a','b','c', 0,0, 100,0, 0,0,

        // Planet (57)
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,                   // 20
        0,0, 0,0, 1,0, 4,0, 0,0, 6,0, 0,0, 0,0, 0,0, 0,0, 9,0,              // 22
        12,0, 61,0, 100,0, 'x','y','z', 0xFF,0xFF, 0,0, 0,0,
        // Note::   ^^^^^=shield,       ^^^^^^^^^=aggressiveness; this is not contractual
    };

    a.checkEqual("01. file size", stream.getContent().size(), sizeof(EXPECTED));
    a.checkEqualContent("02. file content", stream.getContent(), afl::base::ConstBytes_t(EXPECTED));
}

/** Test saving with rating override (produces V4 format). */
AFL_TEST("game.sim.Loader:save:rating", a)
{
    game::sim::Setup setup;
    prepare(setup);
    setup.getShip(0)->setFlakRatingOverride(99);
    setup.getShip(0)->setFlags(game::sim::Ship::fl_RatingOverride);

    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);

    afl::io::InternalStream stream;
    testee.save(stream, setup);

    static const uint8_t EXPECTED[] = {
        // Header (10)
        'C','C','b','s','i','m','3',26,1,0x80,

        // Ship (65)
        'B','o','a','t',32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,    // 20
        0,0, 17,0, 1,0, 3,0, 0,0, 0,0, 0,0, 0,0, 7,0, 180,0, 4,0,           // 22
        8,0, 0,0, 100,0, 'a','b','c', 0,0, 100,0, 16,0,
        99,0,0,0, 0,0, 0,0,

        // Planet (65)
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,                   // 20
        0,0, 0,0, 1,0, 4,0, 0,0, 6,0, 0,0, 0,0, 0,0, 0,0, 9,0,              // 22
        12,0, 61,0, 100,0, 'x','y','z', 0xFF,0xFF, 0,0, 0,0,
        0,0,0,0, 0,0, 0,0,
    };

    a.checkEqual("01. file size", stream.getContent().size(), sizeof(EXPECTED));
    a.checkEqualContent("02. file content", stream.getContent(), afl::base::ConstBytes_t(EXPECTED));
}

/** Test saving with intercept order (produces V4 format). */
AFL_TEST("game.sim.Loader:save:intercept", a)
{
    game::sim::Setup setup;
    prepare(setup);
    setup.getShip(0)->setInterceptId(12);

    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);

    afl::io::InternalStream stream;
    testee.save(stream, setup);

    static const uint8_t EXPECTED[] = {
        // Header (10)
        'C','C','b','s','i','m','3',26,1,0x80,

        // Ship (65)
        'B','o','a','t',32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,    // 20
        0,0, 17,0, 1,0, 3,0, 0,0, 0,0, 0,0, 0,0, 7,0, 180,0, 4,0,           // 22
        8,0, 0,0, 100,0, 'a','b','c', 0,0, 100,0, 0,0,
        0,0,0,0, 0,0, 12,0,

        // Planet (65)
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,                   // 20
        0,0, 0,0, 1,0, 4,0, 0,0, 6,0, 0,0, 0,0, 0,0, 0,0, 9,0,              // 22
        12,0, 61,0, 100,0, 'x','y','z', 0xFF,0xFF, 0,0, 0,0,
        0,0,0,0, 0,0, 0,0,
    };

    a.checkEqual("01. file size", stream.getContent().size(), sizeof(EXPECTED));
    a.checkEqualContent("02. file content", stream.getContent(), afl::base::ConstBytes_t(EXPECTED));
}

/** Test saving with long flags (produces V5 format). */
AFL_TEST("game.sim.Loader:save:flags", a)
{
    game::sim::Setup setup;
    prepare(setup);
    setup.getPlanet()->setFlags(game::sim::Planet::fl_DoubleBeamChargeSet);

    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    game::sim::Loader testee(cs, tx);

    afl::io::InternalStream stream;
    testee.save(stream, setup);

    static const uint8_t EXPECTED[] = {
        // Header (10)
        'C','C','b','s','i','m','4',26,1,0x80,

        // Ship (67)
        'B','o','a','t',32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,    // 20
        0,0, 17,0, 1,0, 3,0, 0,0, 0,0, 0,0, 0,0, 7,0, 180,0, 4,0,           // 22
        8,0, 0,0, 100,0, 'a','b','c', 0,0, 100,0, 0,0,
        0,0,0,0, 0,0, 0,0, 0,0,

        // Planet (67)
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,                   // 20
        0,0, 0,0, 1,0, 4,0, 0,0, 6,0, 0,0, 0,0, 0,0, 0,0, 9,0,              // 22
        12,0, 61,0, 100,0, 'x','y','z', 0xFF,0xFF, 0,0, 0,0,
        0,0,0,0, 0,0, 0,0, 8,0,
    };

    a.checkEqual("01. file size", stream.getContent().size(), sizeof(EXPECTED));
    a.checkEqualContent("02. file content", stream.getContent(), afl::base::ConstBytes_t(EXPECTED));
}
