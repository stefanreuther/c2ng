/**
  *  \file test/game/interface/basepropertytest.cpp
  *  \brief Test for game::interface::BaseProperty
  */

#include "game/interface/baseproperty.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"

using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    const int PLAYER = 7;
    const int TURN_NR = 10;
    const int HULL_SLOT = 3;

    struct Environment {
        afl::string::NullTranslator tx;
        afl::base::Ref<game::Root> root;
        afl::base::Ptr<game::spec::ShipList> shipList;
        afl::base::Ref<game::Turn> turn;

        Environment()
            : tx(), root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0)))),
              shipList(new game::spec::ShipList()),
              turn(*new game::Turn())
            {
                game::test::addTranswarp(*shipList);
                game::test::initStandardBeams(*shipList);
                game::test::initStandardTorpedoes(*shipList);
                game::test::addAnnihilation(*shipList);
                shipList->hullAssignments().add(PLAYER, HULL_SLOT, game::test::ANNIHILATION_HULL_ID);
                shipList->hulls().get(game::test::ANNIHILATION_HULL_ID)->setShortName("Anni");
            }
    };

    void configurePlanet(Environment& env, game::map::Planet& pl, bool withBase)
    {
        // Planet
        game::map::PlanetData pd;
        pd.owner             = PLAYER;
        pd.friendlyCode      = "jkl";
        pd.numMines          = 20;
        pd.numFactories      = 30;
        pd.numDefensePosts   = 15;
        pd.minedNeutronium   = 120;
        pd.minedTritanium    = 84;
        pd.minedDuranium     = 76;
        pd.minedMolybdenum   = 230;
        pd.colonistClans     = 1200;
        pd.supplies          = 31;
        pd.money             = 15000;
        pd.groundNeutronium  = 1092;
        pd.groundTritanium   = 9102;
        pd.groundDuranium    = 349;
        pd.groundMolybdenum  = 781;
        pd.densityNeutronium = 14;
        pd.densityTritanium  = 87;
        pd.densityDuranium   = 29;
        pd.densityMolybdenum = 7;
        pd.colonistTax       = 3;
        pd.nativeTax         = 12;
        pd.colonistHappiness = 97;
        pd.nativeHappiness   = 76;
        pd.nativeGovernment  = 4;
        pd.nativeClans       = 7821;
        pd.nativeRace        = 3;
        pd.temperature       = 53;
        pd.baseFlag          = 1;

        pl.setPosition(game::map::Point(1030, 2700));
        pl.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
        pl.setName("Earth 2");
        pl.setPlayability(game::map::Object::Playable);
        if (withBase) {
            game::map::BaseData bd;
            bd.numBaseDefensePosts           = 10;
            bd.damage                        = 7;
            bd.techLevels[game::HullTech]    = 3;
            bd.techLevels[game::EngineTech]  = 1;
            bd.techLevels[game::BeamTech]    = 4;
            bd.techLevels[game::TorpedoTech] = 5;
            for (int i = 1; i <= 10; ++i) {
                bd.engineStorage.set(i, 10+i);
                bd.hullStorage.set(i, 20+i);
                bd.beamStorage.set(i, 30+i);
                bd.launcherStorage.set(i, 40+i);
                bd.torpedoStorage.set(i, 50+i);
            }
            bd.numFighters    = 5;
            bd.shipyardId     = 0;
            bd.shipyardAction = 0;
            bd.mission        = 6;
            bd.shipBuildOrder.setHullIndex(HULL_SLOT);
            bd.shipBuildOrder.setEngineType(9);
            bd.shipBuildOrder.setBeamType(3);
            bd.shipBuildOrder.setNumBeams(4);
            bd.shipBuildOrder.setTorpedoType(5);
            bd.shipBuildOrder.setNumLaunchers(6);
            pl.addCurrentBaseData(bd, game::PlayerSet_t(PLAYER));
        }
        pl.setBaseQueuePosition(17);

        {
            game::map::Configuration mapConfig;
            afl::sys::Log log;
            pl.internalCheck(mapConfig, game::PlayerSet_t(PLAYER), TURN_NR, env.tx, log);
        }
    }

    /*
     *  Helper for verifying array property
     */

    interpreter::IndexableValue& mustBeIndexable(afl::test::Assert a, afl::data::Value* v)
    {
        interpreter::IndexableValue* iv = dynamic_cast<interpreter::IndexableValue*>(v);
        a.check("not null", iv != 0);

        interpreter::test::ValueVerifier verif(*iv, a);
        verif.verifyBasics();
        verif.verifyNotSerializable();

        return *iv;
    }

    class ArrayVerifier {
     public:
        ArrayVerifier(afl::test::Assert a, afl::data::Value* v)
            : m_assert(a),
              m_value(v),
              m_indexable(mustBeIndexable(a("indexable"), v))
            { }

        interpreter::IndexableValue& indexable()
            { return m_indexable; }

        afl::data::Value* getUnary(afl::data::Value* index)
            {
                afl::data::Segment seg;
                seg.pushBackNew(index);
                interpreter::Arguments args(seg, 0, 1);
                return m_indexable.get(args);
            }

        afl::data::Value* getNullary()
            {
                afl::data::Segment seg;
                interpreter::Arguments args(seg, 0, 0);
                return m_indexable.get(args);
            }

        void setUnary(afl::data::Value* index, int value)
            {
                afl::data::IntegerValue iv(value);
                afl::data::Segment seg;
                seg.pushBackNew(index);
                interpreter::Arguments args(seg, 0, 1);
                m_indexable.set(args, &iv);
            }

     private:
        afl::test::Assert m_assert;
        std::auto_ptr<afl::data::Value> m_value;
        interpreter::IndexableValue& m_indexable;
    };
}

/** General test on planet with base. */
AFL_TEST("game.interface.BaseProperty:normal", a)
{
    Environment env;
    game::map::Planet pl(33);
    configurePlanet(env, pl, true);

    verifyNewInteger(a("ibpBaseDamage"),      getBaseProperty(pl, game::interface::ibpBaseDamage,      env.tx, env.root, env.shipList, env.turn), 7);
    verifyNewInteger(a("ibpBaseDefense"),     getBaseProperty(pl, game::interface::ibpBaseDefense,     env.tx, env.root, env.shipList, env.turn), 10);
    verifyNewInteger(a("ibpBaseDefenseMax"),  getBaseProperty(pl, game::interface::ibpBaseDefenseMax,  env.tx, env.root, env.shipList, env.turn), 200);
    verifyNewInteger(a("ibpBaseFighters"),    getBaseProperty(pl, game::interface::ibpBaseFighters,    env.tx, env.root, env.shipList, env.turn), 5);
    verifyNewInteger(a("ibpBaseFightersMax"), getBaseProperty(pl, game::interface::ibpBaseFightersMax, env.tx, env.root, env.shipList, env.turn), 60);
    verifyNewInteger(a("ibpBeamTech"),        getBaseProperty(pl, game::interface::ibpBeamTech,        env.tx, env.root, env.shipList, env.turn), 4);
    verifyNewInteger(a("ibpBuildBeam"),       getBaseProperty(pl, game::interface::ibpBuildBeam,       env.tx, env.root, env.shipList, env.turn), 3);
    verifyNewInteger(a("ibpBuildBeamCount"),  getBaseProperty(pl, game::interface::ibpBuildBeamCount,  env.tx, env.root, env.shipList, env.turn), 4);
    verifyNewInteger(a("ibpBuildEngine"),     getBaseProperty(pl, game::interface::ibpBuildEngine,     env.tx, env.root, env.shipList, env.turn), 9);
    verifyNewBoolean(a("ibpBuildFlag"),       getBaseProperty(pl, game::interface::ibpBuildFlag,       env.tx, env.root, env.shipList, env.turn), true);
    verifyNewInteger(a("ibpBuildHull"),       getBaseProperty(pl, game::interface::ibpBuildHull,       env.tx, env.root, env.shipList, env.turn), game::test::ANNIHILATION_HULL_ID);
    verifyNewString (a("ibpBuildHullName"),   getBaseProperty(pl, game::interface::ibpBuildHullName,   env.tx, env.root, env.shipList, env.turn), "ANNIHILATION CLASS BATTLESHIP");
    verifyNewString (a("ibpBuildHullShort"),  getBaseProperty(pl, game::interface::ibpBuildHullShort,  env.tx, env.root, env.shipList, env.turn), "Anni");
    verifyNewInteger(a("ibpBuildQueuePos"),   getBaseProperty(pl, game::interface::ibpBuildQueuePos,   env.tx, env.root, env.shipList, env.turn), 17);
    verifyNewInteger(a("ibpBuildTorp"),       getBaseProperty(pl, game::interface::ibpBuildTorp,       env.tx, env.root, env.shipList, env.turn), 5);
    verifyNewInteger(a("ibpBuildTorpCount"),  getBaseProperty(pl, game::interface::ibpBuildTorpCount,  env.tx, env.root, env.shipList, env.turn), 6);
    verifyNewInteger(a("ibpEngineTech"),      getBaseProperty(pl, game::interface::ibpEngineTech,      env.tx, env.root, env.shipList, env.turn), 1);
    verifyNewInteger(a("ibpHullTech"),        getBaseProperty(pl, game::interface::ibpHullTech,        env.tx, env.root, env.shipList, env.turn), 3);
    verifyNewInteger(a("ibpMission"),         getBaseProperty(pl, game::interface::ibpMission,         env.tx, env.root, env.shipList, env.turn), 6);
    verifyNewString (a("ibpMissionName"),     getBaseProperty(pl, game::interface::ibpMissionName,     env.tx, env.root, env.shipList, env.turn), "Force surrender");
    verifyNewInteger(a("ibpTorpedoTech"),     getBaseProperty(pl, game::interface::ibpTorpedoTech,     env.tx, env.root, env.shipList, env.turn), 5);

    // Abnormal case: No ship list
    verifyNewNull(a("Null ibpBuildHull"),       getBaseProperty(pl, game::interface::ibpBuildHull,       env.tx, env.root, 0, env.turn));
    verifyNewNull(a("Null ibpBuildHullName"),   getBaseProperty(pl, game::interface::ibpBuildHullName,   env.tx, env.root, 0, env.turn));
    verifyNewNull(a("Null ibpBuildHullShort"),  getBaseProperty(pl, game::interface::ibpBuildHullShort,  env.tx, env.root, 0, env.turn));
    verifyNewNull(a("Null ibpBeamStorage"),     getBaseProperty(pl, game::interface::ibpBeamStorage,     env.tx, env.root, 0, env.turn));
    verifyNewNull(a("Null ibpEngineStorage"),   getBaseProperty(pl, game::interface::ibpEngineStorage,   env.tx, env.root, 0, env.turn));
    verifyNewNull(a("Null ibpHullStorage"),     getBaseProperty(pl, game::interface::ibpHullStorage,     env.tx, env.root, 0, env.turn));
    verifyNewNull(a("Null ibpLauncherStorage"), getBaseProperty(pl, game::interface::ibpLauncherStorage, env.tx, env.root, 0, env.turn));
    verifyNewNull(a("Null ibpAmmoStorage"),     getBaseProperty(pl, game::interface::ibpAmmoStorage,     env.tx, env.root, 0, env.turn));

    // ibpEngineStorage
    {
        ArrayVerifier verif("ibpEngineStorage", getBaseProperty(pl, game::interface::ibpEngineStorage, env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger(a("ibpEngineStorage(9)"),    verif.getUnary(interpreter::makeIntegerValue(9)), 19);
        verifyNewInteger(a("ibpEngineStorage(0)"),    verif.getUnary(interpreter::makeIntegerValue(0)), 135);  // 11+12+13+14+15+16+17+18+19
        verifyNewNull   (a("ibpEngineStorage(null)"), verif.getUnary(0));
        verifyNewNull   (a("ibpEngineStorage(777)"),  verif.getUnary(interpreter::makeIntegerValue(777)));
        AFL_CHECK_THROWS(a("ibpEngineStorage('X')"),  verif.getUnary(interpreter::makeStringValue("X")), interpreter::Error);
        AFL_CHECK_THROWS(a("ibpEngineStorage()"),     verif.getNullary(), interpreter::Error);
        AFL_CHECK_THROWS(a("set ibpEngineStorage"),   verif.setUnary(interpreter::makeIntegerValue(9), 1), interpreter::Error);
        AFL_CHECK_THROWS(a("first ibpEngineStorage"), verif.indexable().makeFirstContext(), interpreter::Error);
        a.checkEqual("ibpEngineStorage dim(0)",       verif.indexable().getDimension(0), 1);
        a.checkEqual("ibpEngineStorage dim(1)",       verif.indexable().getDimension(1), 10);
    }

    // ibpHullStorage
    {
        ArrayVerifier verif("ibpHullStorage", getBaseProperty(pl, game::interface::ibpHullStorage, env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger(a("ibpHullStorage(ANNI)"), verif.getUnary(interpreter::makeIntegerValue(game::test::ANNIHILATION_HULL_ID)), 23);
        verifyNewInteger(a("ibpHullStorage(0)"),    verif.getUnary(interpreter::makeIntegerValue(0)), 66);     // 21+22+23, because getMaxIndex() == HULL_SLOT
        verifyNewNull   (a("ibpHullStorage(null)"), verif.getUnary(0));
        verifyNewInteger(a("ibpHullStorage(777)"),  verif.getUnary(interpreter::makeIntegerValue(777)), 0);    // Not null, because we know to have zero of unbuildable hull
        AFL_CHECK_THROWS(a("ibpHullStorage('X')"),  verif.getUnary(interpreter::makeStringValue("X")), interpreter::Error);
        AFL_CHECK_THROWS(a("ibpHullStorage()"),     verif.getNullary(), interpreter::Error);
        AFL_CHECK_THROWS(a("set ibpHullStorage"),   verif.setUnary(interpreter::makeIntegerValue(9), 1), interpreter::Error);
        AFL_CHECK_THROWS(a("first ibpHullStorage"), verif.indexable().makeFirstContext(), interpreter::Error);
        a.checkEqual("ibpHullStorage dim(0)",       verif.indexable().getDimension(0), 1);
        a.checkEqual("ibpHullStorage dim(1)",       verif.indexable().getDimension(1), game::test::ANNIHILATION_HULL_ID+1);
    }

    // ibpBeamStorage
    {
        ArrayVerifier verif("ibpBeamStorage", getBaseProperty(pl, game::interface::ibpBeamStorage, env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger(a("ibpBeamStorage(9)"),    verif.getUnary(interpreter::makeIntegerValue(9)), 39);
        verifyNewInteger(a("ibpBeamStorage(0)"),    verif.getUnary(interpreter::makeIntegerValue(0)), 355);    // 31+32+33+34+35+36+37+38+39+40
        verifyNewNull   (a("ibpBeamStorage(null)"), verif.getUnary(0));
        verifyNewNull   (a("ibpBeamStorage(777)"),  verif.getUnary(interpreter::makeIntegerValue(777)));
        AFL_CHECK_THROWS(a("ibpBeamStorage('X')"),  verif.getUnary(interpreter::makeStringValue("X")), interpreter::Error);
        AFL_CHECK_THROWS(a("ibpBeamStorage()"),     verif.getNullary(), interpreter::Error);
        AFL_CHECK_THROWS(a("set ibpBeamStorage"),   verif.setUnary(interpreter::makeIntegerValue(9), 1), interpreter::Error);
        AFL_CHECK_THROWS(a("first ibpBeamStorage"), verif.indexable().makeFirstContext(), interpreter::Error);
        a.checkEqual("ibpBeamStorage dim(0)",       verif.indexable().getDimension(0), 1);
        a.checkEqual("ibpBeamStorage dim(1)",       verif.indexable().getDimension(1), 11);
    }

    // ibpLauncherStorage
    {
        ArrayVerifier verif("ibpLauncherStorage", getBaseProperty(pl, game::interface::ibpLauncherStorage, env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger(a("ibpLauncherStorage(9)"),    verif.getUnary(interpreter::makeIntegerValue(9)), 49);
        verifyNewInteger(a("ibpLauncherStorage(0)"),    verif.getUnary(interpreter::makeIntegerValue(0)), 455); // 41+42+43+44+45+46+47+48+49+50
        verifyNewNull   (a("ibpLauncherStorage(null)"), verif.getUnary(0));
        verifyNewNull   (a("ibpLauncherStorage(777)"),  verif.getUnary(interpreter::makeIntegerValue(777)));
        AFL_CHECK_THROWS(a("ibpLauncherStorage('X')"),  verif.getUnary(interpreter::makeStringValue("X")), interpreter::Error);
        AFL_CHECK_THROWS(a("ibpLauncherStorage()"),     verif.getNullary(), interpreter::Error);
        AFL_CHECK_THROWS(a("set ibpLauncherStorage"),   verif.setUnary(interpreter::makeIntegerValue(9), 1), interpreter::Error);
        AFL_CHECK_THROWS(a("first ibpLauncherStorage"), verif.indexable().makeFirstContext(), interpreter::Error);
        a.checkEqual("ibpLauncherStorage dim(0)",       verif.indexable().getDimension(0), 1);
        a.checkEqual("ibpLauncherStorage dim(1)",       verif.indexable().getDimension(1), 11);
    }

    // ibpAmmoStorage
    {
        ArrayVerifier verif("ibpAmmoStorage", getBaseProperty(pl, game::interface::ibpAmmoStorage, env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger(a("ibpAmmoStorage(9)"),    verif.getUnary(interpreter::makeIntegerValue(9)), 59);     // Mk7 Torps
        verifyNewInteger(a("ibpAmmoStorage(9)"),    verif.getUnary(interpreter::makeIntegerValue(11)), 5);     // Fighters
        verifyNewInteger(a("ibpAmmoStorage(0)"),    verif.getUnary(interpreter::makeIntegerValue(0)), 560);    // 51+52+53+54+55+56+57+58+59+60 + 5
        verifyNewNull   (a("ibpAmmoStorage(null)"), verif.getUnary(0));
        verifyNewNull   (a("ibpAmmoStorage(777)"),  verif.getUnary(interpreter::makeIntegerValue(777)));
        AFL_CHECK_THROWS(a("ibpAmmoStorage('X')"),  verif.getUnary(interpreter::makeStringValue("X")), interpreter::Error);
        AFL_CHECK_THROWS(a("ibpAmmoStorage()"),     verif.getNullary(), interpreter::Error);
        AFL_CHECK_THROWS(a("set ibpAmmoStorage"),   verif.setUnary(interpreter::makeIntegerValue(9), 1), interpreter::Error);
        AFL_CHECK_THROWS(a("first ibpAmmoStorage"), verif.indexable().makeFirstContext(), interpreter::Error);
        a.checkEqual("ibpAmmoStorage dim(0)",       verif.indexable().getDimension(0), 1);
        a.checkEqual("ibpAmmoStorage dim(1)",       verif.indexable().getDimension(1), 12);
    }
}

/** General test on planet without base. */
AFL_TEST("game.interface.BaseProperty:no-base", a)
{
    Environment env;
    game::map::Planet pl(33);
    configurePlanet(env, pl, false);

    verifyNewNull(a("ibpBaseDamage"),      getBaseProperty(pl, game::interface::ibpBaseDamage,      env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBaseDefense"),     getBaseProperty(pl, game::interface::ibpBaseDefense,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBaseDefenseMax"),  getBaseProperty(pl, game::interface::ibpBaseDefenseMax,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBaseFighters"),    getBaseProperty(pl, game::interface::ibpBaseFighters,    env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBaseFightersMax"), getBaseProperty(pl, game::interface::ibpBaseFightersMax, env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBeamTech"),        getBaseProperty(pl, game::interface::ibpBeamTech,        env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildBeam"),       getBaseProperty(pl, game::interface::ibpBuildBeam,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildBeamCount"),  getBaseProperty(pl, game::interface::ibpBuildBeamCount,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildEngine"),     getBaseProperty(pl, game::interface::ibpBuildEngine,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildFlag"),       getBaseProperty(pl, game::interface::ibpBuildFlag,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildHull"),       getBaseProperty(pl, game::interface::ibpBuildHull,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildHullName"),   getBaseProperty(pl, game::interface::ibpBuildHullName,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildHullShort"),  getBaseProperty(pl, game::interface::ibpBuildHullShort,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildQueuePos"),   getBaseProperty(pl, game::interface::ibpBuildQueuePos,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildTorp"),       getBaseProperty(pl, game::interface::ibpBuildTorp,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildTorpCount"),  getBaseProperty(pl, game::interface::ibpBuildTorpCount,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpEngineTech"),      getBaseProperty(pl, game::interface::ibpEngineTech,      env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpHullTech"),        getBaseProperty(pl, game::interface::ibpHullTech,        env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpMission"),         getBaseProperty(pl, game::interface::ibpMission,         env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpMissionName"),     getBaseProperty(pl, game::interface::ibpMissionName,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpTorpedoTech"),     getBaseProperty(pl, game::interface::ibpTorpedoTech,     env.tx, env.root, env.shipList, env.turn));

    verifyNewNull(a("ibpEngineStorage"),   getBaseProperty(pl, game::interface::ibpEngineStorage,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpHullStorage"),     getBaseProperty(pl, game::interface::ibpHullStorage,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBeamStorage"),     getBaseProperty(pl, game::interface::ibpBeamStorage,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpLauncherStorage"), getBaseProperty(pl, game::interface::ibpLauncherStorage, env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpAmmoStorage"),     getBaseProperty(pl, game::interface::ibpAmmoStorage,     env.tx, env.root, env.shipList, env.turn));
}

/** General test on unplayed planet. */
AFL_TEST("game.interface.BaseProperty:unplayed", a)
{
    Environment env;
    game::map::Planet pl(33);

    verifyNewNull(a("ibpBaseDamage"),      getBaseProperty(pl, game::interface::ibpBaseDamage,      env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBaseDefense"),     getBaseProperty(pl, game::interface::ibpBaseDefense,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBaseDefenseMax"),  getBaseProperty(pl, game::interface::ibpBaseDefenseMax,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBaseFighters"),    getBaseProperty(pl, game::interface::ibpBaseFighters,    env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBaseFightersMax"), getBaseProperty(pl, game::interface::ibpBaseFightersMax, env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBeamTech"),        getBaseProperty(pl, game::interface::ibpBeamTech,        env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildBeam"),       getBaseProperty(pl, game::interface::ibpBuildBeam,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildBeamCount"),  getBaseProperty(pl, game::interface::ibpBuildBeamCount,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildEngine"),     getBaseProperty(pl, game::interface::ibpBuildEngine,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildFlag"),       getBaseProperty(pl, game::interface::ibpBuildFlag,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildHull"),       getBaseProperty(pl, game::interface::ibpBuildHull,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildHullName"),   getBaseProperty(pl, game::interface::ibpBuildHullName,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildHullShort"),  getBaseProperty(pl, game::interface::ibpBuildHullShort,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildQueuePos"),   getBaseProperty(pl, game::interface::ibpBuildQueuePos,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildTorp"),       getBaseProperty(pl, game::interface::ibpBuildTorp,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBuildTorpCount"),  getBaseProperty(pl, game::interface::ibpBuildTorpCount,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpEngineTech"),      getBaseProperty(pl, game::interface::ibpEngineTech,      env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpHullTech"),        getBaseProperty(pl, game::interface::ibpHullTech,        env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpMission"),         getBaseProperty(pl, game::interface::ibpMission,         env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpMissionName"),     getBaseProperty(pl, game::interface::ibpMissionName,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpTorpedoTech"),     getBaseProperty(pl, game::interface::ibpTorpedoTech,     env.tx, env.root, env.shipList, env.turn));

    verifyNewNull(a("ibpEngineStorage"),   getBaseProperty(pl, game::interface::ibpEngineStorage,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpHullStorage"),     getBaseProperty(pl, game::interface::ibpHullStorage,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpBeamStorage"),     getBaseProperty(pl, game::interface::ibpBeamStorage,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpLauncherStorage"), getBaseProperty(pl, game::interface::ibpLauncherStorage, env.tx, env.root, env.shipList, env.turn));
    verifyNewNull(a("ibpAmmoStorage"),     getBaseProperty(pl, game::interface::ibpAmmoStorage,     env.tx, env.root, env.shipList, env.turn));
}

/*
 *  Shipyard properties
 */

// Default
AFL_TEST("game.interface.BaseProperty:shipyard:default", a)
{
    Environment env;
    game::map::Planet pl(33);
    configurePlanet(env, pl, true);
    verifyNewNull   (a("ibpShipyardAction"),  getBaseProperty(pl, game::interface::ibpShipyardAction,  env.tx, env.root, env.shipList, env.turn));
    verifyNewInteger(a("ibpShipyardId"),      getBaseProperty(pl, game::interface::ibpShipyardId,      env.tx, env.root, env.shipList, env.turn), 0);
    verifyNewNull   (a("ibpShipyardName"),    getBaseProperty(pl, game::interface::ibpShipyardName,    env.tx, env.root, env.shipList, env.turn));
    verifyNewNull   (a("ibpShipyardStr"),     getBaseProperty(pl, game::interface::ibpShipyardStr,     env.tx, env.root, env.shipList, env.turn));
}

// Fix
AFL_TEST("game.interface.BaseProperty:shipyard:fix", a)
{
    Environment env;
    game::map::Planet pl(33);
    configurePlanet(env, pl, true);

    game::map::Ship& sh = *env.turn->universe().ships().create(17);
    sh.setName("Fixee");
    pl.setBaseShipyardOrder(game::FixShipyardAction, 17);

    verifyNewString (a("ibpShipyardAction"),  getBaseProperty(pl, game::interface::ibpShipyardAction,  env.tx, env.root, env.shipList, env.turn), "Fix");
    verifyNewInteger(a("ibpShipyardId"),      getBaseProperty(pl, game::interface::ibpShipyardId,      env.tx, env.root, env.shipList, env.turn), 17);
    verifyNewString (a("ibpShipyardName"),    getBaseProperty(pl, game::interface::ibpShipyardName,    env.tx, env.root, env.shipList, env.turn), "Fixee");
    verifyNewString (a("ibpShipyardStr"),     getBaseProperty(pl, game::interface::ibpShipyardStr,     env.tx, env.root, env.shipList, env.turn), "Fix Fixee");
}

// Recycle
AFL_TEST("game.interface.BaseProperty:shipyard:recycle", a)
{
    Environment env;
    game::map::Planet pl(33);
    configurePlanet(env, pl, true);

    game::map::Ship& sh = *env.turn->universe().ships().create(99);
    sh.setName("Scrap");
    pl.setBaseShipyardOrder(game::RecycleShipyardAction, 99);

    verifyNewString (a("ibpShipyardAction"),  getBaseProperty(pl, game::interface::ibpShipyardAction,  env.tx, env.root, env.shipList, env.turn), "Recycle");
    verifyNewInteger(a("ibpShipyardId"),      getBaseProperty(pl, game::interface::ibpShipyardId,      env.tx, env.root, env.shipList, env.turn), 99);
    verifyNewString (a("ibpShipyardName"),    getBaseProperty(pl, game::interface::ibpShipyardName,    env.tx, env.root, env.shipList, env.turn), "Scrap");
    verifyNewString (a("ibpShipyardStr"),     getBaseProperty(pl, game::interface::ibpShipyardStr,     env.tx, env.root, env.shipList, env.turn), "Recycle Scrap");
}

// No base
AFL_TEST("game.interface.BaseProperty:shipyard:no-base", a)
{
    Environment env;
    game::map::Planet pl(33);
    configurePlanet(env, pl, false);
    verifyNewNull   (a("ibpShipyardAction"),  getBaseProperty(pl, game::interface::ibpShipyardAction,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull   (a("ibpShipyardId"),      getBaseProperty(pl, game::interface::ibpShipyardId,      env.tx, env.root, env.shipList, env.turn));
    verifyNewNull   (a("ibpShipyardName"),    getBaseProperty(pl, game::interface::ibpShipyardName,    env.tx, env.root, env.shipList, env.turn));
    verifyNewNull   (a("ibpShipyardStr"),     getBaseProperty(pl, game::interface::ibpShipyardStr,     env.tx, env.root, env.shipList, env.turn));
}

// Not played
AFL_TEST("game.interface.BaseProperty:shipyard:not-played", a)
{
    Environment env;
    game::map::Planet pl(33);
    verifyNewNull   (a("ibpShipyardAction"),  getBaseProperty(pl, game::interface::ibpShipyardAction,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull   (a("ibpShipyardId"),      getBaseProperty(pl, game::interface::ibpShipyardId,      env.tx, env.root, env.shipList, env.turn));
    verifyNewNull   (a("ibpShipyardName"),    getBaseProperty(pl, game::interface::ibpShipyardName,    env.tx, env.root, env.shipList, env.turn));
    verifyNewNull   (a("ibpShipyardStr"),     getBaseProperty(pl, game::interface::ibpShipyardStr,     env.tx, env.root, env.shipList, env.turn));
}

/*
 *   setBaseProperty()
 */

    // Base present
AFL_TEST("game.interface.BaseProperty:setBaseProperty:success", a)
{
    Environment env;
    game::map::Planet pl(33);
    configurePlanet(env, pl, true);

    // Successful assignment
    const afl::data::IntegerValue iv(1);
    AFL_CHECK_SUCCEEDS(a("ibpMission"), setBaseProperty(pl, game::interface::ibpMission, &iv));
    a.checkEqual("getBaseMission", pl.getBaseMission().orElse(-1), 1);

    // Failing assignment
    AFL_CHECK_THROWS(a("ibpBaseDamage"), setBaseProperty(pl, game::interface::ibpBaseDamage, &iv), interpreter::Error);
}

// No base present
AFL_TEST("game.interface.BaseProperty:setBaseProperty:no-base", a)
{
    Environment env;
    game::map::Planet pl(33);
    configurePlanet(env, pl, false);

    // Assignable, but inaccessible
    const afl::data::IntegerValue iv(1);
    AFL_CHECK_THROWS(a("ibpMission"), setBaseProperty(pl, game::interface::ibpMission, &iv), interpreter::Error);

    // Failing assignment
    AFL_CHECK_THROWS(a("ibpBaseDamage"), setBaseProperty(pl, game::interface::ibpBaseDamage, &iv), interpreter::Error);
}
