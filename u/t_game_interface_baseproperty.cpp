/**
  *  \file u/t_game_interface_baseproperty.cpp
  *  \brief Test for game::interface::BaseProperty
  */

#include "game/interface/baseproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"
#include "game/test/shiplist.hpp"
#include "afl/data/integervalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/arguments.hpp"
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
void
TestGameInterfaceBaseProperty::testIt()
{
    Environment env;
    game::map::Planet pl(33);
    configurePlanet(env, pl, true);

    verifyNewInteger("ibpBaseDamage",      getBaseProperty(pl, game::interface::ibpBaseDamage,      env.tx, env.root, env.shipList, env.turn), 7);
    verifyNewInteger("ibpBaseDefense",     getBaseProperty(pl, game::interface::ibpBaseDefense,     env.tx, env.root, env.shipList, env.turn), 10);
    verifyNewInteger("ibpBaseDefenseMax",  getBaseProperty(pl, game::interface::ibpBaseDefenseMax,  env.tx, env.root, env.shipList, env.turn), 200);
    verifyNewInteger("ibpBaseFighters",    getBaseProperty(pl, game::interface::ibpBaseFighters,    env.tx, env.root, env.shipList, env.turn), 5);
    verifyNewInteger("ibpBaseFightersMax", getBaseProperty(pl, game::interface::ibpBaseFightersMax, env.tx, env.root, env.shipList, env.turn), 60);
    verifyNewInteger("ibpBeamTech",        getBaseProperty(pl, game::interface::ibpBeamTech,        env.tx, env.root, env.shipList, env.turn), 4);
    verifyNewInteger("ibpBuildBeam",       getBaseProperty(pl, game::interface::ibpBuildBeam,       env.tx, env.root, env.shipList, env.turn), 3);
    verifyNewInteger("ibpBuildBeamCount",  getBaseProperty(pl, game::interface::ibpBuildBeamCount,  env.tx, env.root, env.shipList, env.turn), 4);
    verifyNewInteger("ibpBuildEngine",     getBaseProperty(pl, game::interface::ibpBuildEngine,     env.tx, env.root, env.shipList, env.turn), 9);
    verifyNewBoolean("ibpBuildFlag",       getBaseProperty(pl, game::interface::ibpBuildFlag,       env.tx, env.root, env.shipList, env.turn), true);
    verifyNewInteger("ibpBuildHull",       getBaseProperty(pl, game::interface::ibpBuildHull,       env.tx, env.root, env.shipList, env.turn), game::test::ANNIHILATION_HULL_ID);
    verifyNewString ("ibpBuildHullName",   getBaseProperty(pl, game::interface::ibpBuildHullName,   env.tx, env.root, env.shipList, env.turn), "ANNIHILATION CLASS BATTLESHIP");
    verifyNewString ("ibpBuildHullShort",  getBaseProperty(pl, game::interface::ibpBuildHullShort,  env.tx, env.root, env.shipList, env.turn), "Anni");
    verifyNewInteger("ibpBuildQueuePos",   getBaseProperty(pl, game::interface::ibpBuildQueuePos,   env.tx, env.root, env.shipList, env.turn), 17);
    verifyNewInteger("ibpBuildTorp",       getBaseProperty(pl, game::interface::ibpBuildTorp,       env.tx, env.root, env.shipList, env.turn), 5);
    verifyNewInteger("ibpBuildTorpCount",  getBaseProperty(pl, game::interface::ibpBuildTorpCount,  env.tx, env.root, env.shipList, env.turn), 6);
    verifyNewInteger("ibpEngineTech",      getBaseProperty(pl, game::interface::ibpEngineTech,      env.tx, env.root, env.shipList, env.turn), 1);
    verifyNewInteger("ibpHullTech",        getBaseProperty(pl, game::interface::ibpHullTech,        env.tx, env.root, env.shipList, env.turn), 3);
    verifyNewInteger("ibpMission",         getBaseProperty(pl, game::interface::ibpMission,         env.tx, env.root, env.shipList, env.turn), 6);
    verifyNewString ("ibpMissionName",     getBaseProperty(pl, game::interface::ibpMissionName,     env.tx, env.root, env.shipList, env.turn), "Force surrender");
    verifyNewInteger("ibpTorpedoTech",     getBaseProperty(pl, game::interface::ibpTorpedoTech,     env.tx, env.root, env.shipList, env.turn), 5);

    // Abnormal case: No ship list
    verifyNewNull("Null ibpBuildHull",       getBaseProperty(pl, game::interface::ibpBuildHull,       env.tx, env.root, 0, env.turn));
    verifyNewNull("Null ibpBuildHullName",   getBaseProperty(pl, game::interface::ibpBuildHullName,   env.tx, env.root, 0, env.turn));
    verifyNewNull("Null ibpBuildHullShort",  getBaseProperty(pl, game::interface::ibpBuildHullShort,  env.tx, env.root, 0, env.turn));
    verifyNewNull("Null ibpBeamStorage",     getBaseProperty(pl, game::interface::ibpBeamStorage,     env.tx, env.root, 0, env.turn));
    verifyNewNull("Null ibpEngineStorage",   getBaseProperty(pl, game::interface::ibpEngineStorage,   env.tx, env.root, 0, env.turn));
    verifyNewNull("Null ibpHullStorage",     getBaseProperty(pl, game::interface::ibpHullStorage,     env.tx, env.root, 0, env.turn));
    verifyNewNull("Null ibpLauncherStorage", getBaseProperty(pl, game::interface::ibpLauncherStorage, env.tx, env.root, 0, env.turn));
    verifyNewNull("Null ibpAmmoStorage",     getBaseProperty(pl, game::interface::ibpAmmoStorage,     env.tx, env.root, 0, env.turn));

    // ibpEngineStorage
    {
        ArrayVerifier verif("ibpEngineStorage", getBaseProperty(pl, game::interface::ibpEngineStorage, env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger("ibpEngineStorage(9)",    verif.getUnary(interpreter::makeIntegerValue(9)), 19);
        verifyNewInteger("ibpEngineStorage(0)",    verif.getUnary(interpreter::makeIntegerValue(0)), 135);  // 11+12+13+14+15+16+17+18+19
        verifyNewNull   ("ibpEngineStorage(null)", verif.getUnary(0));
        verifyNewNull   ("ibpEngineStorage(777)",  verif.getUnary(interpreter::makeIntegerValue(777)));
        TS_ASSERT_THROWS(verif.getUnary(interpreter::makeStringValue("X")), interpreter::Error);
        TS_ASSERT_THROWS(verif.getNullary(), interpreter::Error);
        TS_ASSERT_THROWS(verif.setUnary(interpreter::makeIntegerValue(9), 1), interpreter::Error);
        TS_ASSERT_THROWS(verif.indexable().makeFirstContext(), interpreter::Error);
        TS_ASSERT_EQUALS(verif.indexable().getDimension(0), 1);
        TS_ASSERT_EQUALS(verif.indexable().getDimension(1), 10);
    }

    // ibpHullStorage
    {
        ArrayVerifier verif("ibpHullStorage", getBaseProperty(pl, game::interface::ibpHullStorage, env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger("ibpHullStorage(ANNI)", verif.getUnary(interpreter::makeIntegerValue(game::test::ANNIHILATION_HULL_ID)), 23);
        verifyNewInteger("ibpHullStorage(0)",    verif.getUnary(interpreter::makeIntegerValue(0)), 66);     // 21+22+23, because getMaxIndex() == HULL_SLOT
        verifyNewNull   ("ibpHullStorage(null)", verif.getUnary(0));
        verifyNewInteger("ibpHullStorage(777)",  verif.getUnary(interpreter::makeIntegerValue(777)), 0);    // Not null, because we know to have zero of unbuildable hull
        TS_ASSERT_THROWS(verif.getUnary(interpreter::makeStringValue("X")), interpreter::Error);
        TS_ASSERT_THROWS(verif.getNullary(), interpreter::Error);
        TS_ASSERT_THROWS(verif.setUnary(interpreter::makeIntegerValue(9), 1), interpreter::Error);
        TS_ASSERT_THROWS(verif.indexable().makeFirstContext(), interpreter::Error);
        TS_ASSERT_EQUALS(verif.indexable().getDimension(0), 1);
        TS_ASSERT_EQUALS(verif.indexable().getDimension(1), game::test::ANNIHILATION_HULL_ID+1);
    }

    // ibpBeamStorage
    {
        ArrayVerifier verif("ibpBeamStorage", getBaseProperty(pl, game::interface::ibpBeamStorage, env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger("ibpBeamStorage(9)",    verif.getUnary(interpreter::makeIntegerValue(9)), 39);
        verifyNewInteger("ibpBeamStorage(0)",    verif.getUnary(interpreter::makeIntegerValue(0)), 355);    // 31+32+33+34+35+36+37+38+39+40
        verifyNewNull   ("ibpBeamStorage(null)", verif.getUnary(0));
        verifyNewNull   ("ibpBeamStorage(777)",  verif.getUnary(interpreter::makeIntegerValue(777)));
        TS_ASSERT_THROWS(verif.getUnary(interpreter::makeStringValue("X")), interpreter::Error);
        TS_ASSERT_THROWS(verif.getNullary(), interpreter::Error);
        TS_ASSERT_THROWS(verif.setUnary(interpreter::makeIntegerValue(9), 1), interpreter::Error);
        TS_ASSERT_THROWS(verif.indexable().makeFirstContext(), interpreter::Error);
        TS_ASSERT_EQUALS(verif.indexable().getDimension(0), 1);
        TS_ASSERT_EQUALS(verif.indexable().getDimension(1), 11);
    }

    // ibpLauncherStorage
    {
        ArrayVerifier verif("ibpLauncherStorage", getBaseProperty(pl, game::interface::ibpLauncherStorage, env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger("ibpLauncherStorage(9)",    verif.getUnary(interpreter::makeIntegerValue(9)), 49);
        verifyNewInteger("ibpLauncherStorage(0)",    verif.getUnary(interpreter::makeIntegerValue(0)), 455); // 41+42+43+44+45+46+47+48+49+50
        verifyNewNull   ("ibpLauncherStorage(null)", verif.getUnary(0));
        verifyNewNull   ("ibpLauncherStorage(777)",  verif.getUnary(interpreter::makeIntegerValue(777)));
        TS_ASSERT_THROWS(verif.getUnary(interpreter::makeStringValue("X")), interpreter::Error);
        TS_ASSERT_THROWS(verif.getNullary(), interpreter::Error);
        TS_ASSERT_THROWS(verif.setUnary(interpreter::makeIntegerValue(9), 1), interpreter::Error);
        TS_ASSERT_THROWS(verif.indexable().makeFirstContext(), interpreter::Error);
        TS_ASSERT_EQUALS(verif.indexable().getDimension(0), 1);
        TS_ASSERT_EQUALS(verif.indexable().getDimension(1), 11);
    }

    // ibpAmmoStorage
    {
        ArrayVerifier verif("ibpAmmoStorage", getBaseProperty(pl, game::interface::ibpAmmoStorage, env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger("ibpAmmoStorage(9)",    verif.getUnary(interpreter::makeIntegerValue(9)), 59);     // Mk7 Torps
        verifyNewInteger("ibpAmmoStorage(9)",    verif.getUnary(interpreter::makeIntegerValue(11)), 5);     // Fighters
        verifyNewInteger("ibpAmmoStorage(0)",    verif.getUnary(interpreter::makeIntegerValue(0)), 560);    // 51+52+53+54+55+56+57+58+59+60 + 5
        verifyNewNull   ("ibpAmmoStorage(null)", verif.getUnary(0));
        verifyNewNull   ("ibpAmmoStorage(777)",  verif.getUnary(interpreter::makeIntegerValue(777)));
        TS_ASSERT_THROWS(verif.getUnary(interpreter::makeStringValue("X")), interpreter::Error);
        TS_ASSERT_THROWS(verif.getNullary(), interpreter::Error);
        TS_ASSERT_THROWS(verif.setUnary(interpreter::makeIntegerValue(9), 1), interpreter::Error);
        TS_ASSERT_THROWS(verif.indexable().makeFirstContext(), interpreter::Error);
        TS_ASSERT_EQUALS(verif.indexable().getDimension(0), 1);
        TS_ASSERT_EQUALS(verif.indexable().getDimension(1), 12);
    }
}

/** General test on planet without base. */
void
TestGameInterfaceBaseProperty::testNoBase()
{
    Environment env;
    game::map::Planet pl(33);
    configurePlanet(env, pl, false);

    verifyNewNull("ibpBaseDamage",      getBaseProperty(pl, game::interface::ibpBaseDamage,      env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBaseDefense",     getBaseProperty(pl, game::interface::ibpBaseDefense,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBaseDefenseMax",  getBaseProperty(pl, game::interface::ibpBaseDefenseMax,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBaseFighters",    getBaseProperty(pl, game::interface::ibpBaseFighters,    env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBaseFightersMax", getBaseProperty(pl, game::interface::ibpBaseFightersMax, env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBeamTech",        getBaseProperty(pl, game::interface::ibpBeamTech,        env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildBeam",       getBaseProperty(pl, game::interface::ibpBuildBeam,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildBeamCount",  getBaseProperty(pl, game::interface::ibpBuildBeamCount,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildEngine",     getBaseProperty(pl, game::interface::ibpBuildEngine,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildFlag",       getBaseProperty(pl, game::interface::ibpBuildFlag,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildHull",       getBaseProperty(pl, game::interface::ibpBuildHull,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildHullName",   getBaseProperty(pl, game::interface::ibpBuildHullName,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildHullShort",  getBaseProperty(pl, game::interface::ibpBuildHullShort,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildQueuePos",   getBaseProperty(pl, game::interface::ibpBuildQueuePos,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildTorp",       getBaseProperty(pl, game::interface::ibpBuildTorp,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildTorpCount",  getBaseProperty(pl, game::interface::ibpBuildTorpCount,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpEngineTech",      getBaseProperty(pl, game::interface::ibpEngineTech,      env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpHullTech",        getBaseProperty(pl, game::interface::ibpHullTech,        env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpMission",         getBaseProperty(pl, game::interface::ibpMission,         env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpMissionName",     getBaseProperty(pl, game::interface::ibpMissionName,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpTorpedoTech",     getBaseProperty(pl, game::interface::ibpTorpedoTech,     env.tx, env.root, env.shipList, env.turn));

    verifyNewNull("ibpEngineStorage",   getBaseProperty(pl, game::interface::ibpEngineStorage,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpHullStorage",     getBaseProperty(pl, game::interface::ibpHullStorage,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBeamStorage",     getBaseProperty(pl, game::interface::ibpBeamStorage,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpLauncherStorage", getBaseProperty(pl, game::interface::ibpLauncherStorage, env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpAmmoStorage",     getBaseProperty(pl, game::interface::ibpAmmoStorage,     env.tx, env.root, env.shipList, env.turn));
}

/** General test on unplayed planet. */
void
TestGameInterfaceBaseProperty::testNoPlanet()
{
    Environment env;
    game::map::Planet pl(33);

    verifyNewNull("ibpBaseDamage",      getBaseProperty(pl, game::interface::ibpBaseDamage,      env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBaseDefense",     getBaseProperty(pl, game::interface::ibpBaseDefense,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBaseDefenseMax",  getBaseProperty(pl, game::interface::ibpBaseDefenseMax,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBaseFighters",    getBaseProperty(pl, game::interface::ibpBaseFighters,    env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBaseFightersMax", getBaseProperty(pl, game::interface::ibpBaseFightersMax, env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBeamTech",        getBaseProperty(pl, game::interface::ibpBeamTech,        env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildBeam",       getBaseProperty(pl, game::interface::ibpBuildBeam,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildBeamCount",  getBaseProperty(pl, game::interface::ibpBuildBeamCount,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildEngine",     getBaseProperty(pl, game::interface::ibpBuildEngine,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildFlag",       getBaseProperty(pl, game::interface::ibpBuildFlag,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildHull",       getBaseProperty(pl, game::interface::ibpBuildHull,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildHullName",   getBaseProperty(pl, game::interface::ibpBuildHullName,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildHullShort",  getBaseProperty(pl, game::interface::ibpBuildHullShort,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildQueuePos",   getBaseProperty(pl, game::interface::ibpBuildQueuePos,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildTorp",       getBaseProperty(pl, game::interface::ibpBuildTorp,       env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBuildTorpCount",  getBaseProperty(pl, game::interface::ibpBuildTorpCount,  env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpEngineTech",      getBaseProperty(pl, game::interface::ibpEngineTech,      env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpHullTech",        getBaseProperty(pl, game::interface::ibpHullTech,        env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpMission",         getBaseProperty(pl, game::interface::ibpMission,         env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpMissionName",     getBaseProperty(pl, game::interface::ibpMissionName,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpTorpedoTech",     getBaseProperty(pl, game::interface::ibpTorpedoTech,     env.tx, env.root, env.shipList, env.turn));

    verifyNewNull("ibpEngineStorage",   getBaseProperty(pl, game::interface::ibpEngineStorage,   env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpHullStorage",     getBaseProperty(pl, game::interface::ibpHullStorage,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpBeamStorage",     getBaseProperty(pl, game::interface::ibpBeamStorage,     env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpLauncherStorage", getBaseProperty(pl, game::interface::ibpLauncherStorage, env.tx, env.root, env.shipList, env.turn));
    verifyNewNull("ibpAmmoStorage",     getBaseProperty(pl, game::interface::ibpAmmoStorage,     env.tx, env.root, env.shipList, env.turn));
}

/** Test shipyard properties. */
void
TestGameInterfaceBaseProperty::testShipyard()
{
    // Default
    {
        Environment env;
        game::map::Planet pl(33);
        configurePlanet(env, pl, true);
        verifyNewNull   ("Empty ibpShipyardAction",  getBaseProperty(pl, game::interface::ibpShipyardAction,  env.tx, env.root, env.shipList, env.turn));
        verifyNewInteger("Empty ibpShipyardId",      getBaseProperty(pl, game::interface::ibpShipyardId,      env.tx, env.root, env.shipList, env.turn), 0);
        verifyNewNull   ("Empty ibpShipyardName",    getBaseProperty(pl, game::interface::ibpShipyardName,    env.tx, env.root, env.shipList, env.turn));
        verifyNewNull   ("Empty ibpShipyardStr",     getBaseProperty(pl, game::interface::ibpShipyardStr,     env.tx, env.root, env.shipList, env.turn));
    }

    // Fix
    {
        Environment env;
        game::map::Planet pl(33);
        configurePlanet(env, pl, true);

        game::map::Ship& sh = *env.turn->universe().ships().create(17);
        sh.setName("Fixee");
        pl.setBaseShipyardOrder(game::FixShipyardAction, 17);

        verifyNewString ("Fix ibpShipyardAction",  getBaseProperty(pl, game::interface::ibpShipyardAction,  env.tx, env.root, env.shipList, env.turn), "Fix");
        verifyNewInteger("Fix ibpShipyardId",      getBaseProperty(pl, game::interface::ibpShipyardId,      env.tx, env.root, env.shipList, env.turn), 17);
        verifyNewString ("Fix ibpShipyardName",    getBaseProperty(pl, game::interface::ibpShipyardName,    env.tx, env.root, env.shipList, env.turn), "Fixee");
        verifyNewString ("Fix ibpShipyardStr",     getBaseProperty(pl, game::interface::ibpShipyardStr,     env.tx, env.root, env.shipList, env.turn), "Fix Fixee");
    }

    // Recycle
    {
        Environment env;
        game::map::Planet pl(33);
        configurePlanet(env, pl, true);

        game::map::Ship& sh = *env.turn->universe().ships().create(99);
        sh.setName("Scrap");
        pl.setBaseShipyardOrder(game::RecycleShipyardAction, 99);

        verifyNewString ("Recycle ibpShipyardAction",  getBaseProperty(pl, game::interface::ibpShipyardAction,  env.tx, env.root, env.shipList, env.turn), "Recycle");
        verifyNewInteger("Recycle ibpShipyardId",      getBaseProperty(pl, game::interface::ibpShipyardId,      env.tx, env.root, env.shipList, env.turn), 99);
        verifyNewString ("Recycle ibpShipyardName",    getBaseProperty(pl, game::interface::ibpShipyardName,    env.tx, env.root, env.shipList, env.turn), "Scrap");
        verifyNewString ("Recycle ibpShipyardStr",     getBaseProperty(pl, game::interface::ibpShipyardStr,     env.tx, env.root, env.shipList, env.turn), "Recycle Scrap");
    }

    // No base
    {
        Environment env;
        game::map::Planet pl(33);
        configurePlanet(env, pl, false);
        verifyNewNull   ("NoBase ibpShipyardAction",  getBaseProperty(pl, game::interface::ibpShipyardAction,  env.tx, env.root, env.shipList, env.turn));
        verifyNewNull   ("NoBase ibpShipyardId",      getBaseProperty(pl, game::interface::ibpShipyardId,      env.tx, env.root, env.shipList, env.turn));
        verifyNewNull   ("NoBase ibpShipyardName",    getBaseProperty(pl, game::interface::ibpShipyardName,    env.tx, env.root, env.shipList, env.turn));
        verifyNewNull   ("NoBase ibpShipyardStr",     getBaseProperty(pl, game::interface::ibpShipyardStr,     env.tx, env.root, env.shipList, env.turn));
    }

    // Not played
    {
        Environment env;
        game::map::Planet pl(33);
        verifyNewNull   ("NoBase ibpShipyardAction",  getBaseProperty(pl, game::interface::ibpShipyardAction,  env.tx, env.root, env.shipList, env.turn));
        verifyNewNull   ("NoBase ibpShipyardId",      getBaseProperty(pl, game::interface::ibpShipyardId,      env.tx, env.root, env.shipList, env.turn));
        verifyNewNull   ("NoBase ibpShipyardName",    getBaseProperty(pl, game::interface::ibpShipyardName,    env.tx, env.root, env.shipList, env.turn));
        verifyNewNull   ("NoBase ibpShipyardStr",     getBaseProperty(pl, game::interface::ibpShipyardStr,     env.tx, env.root, env.shipList, env.turn));
    }
}

/** Test setBaseProperty(). */
void
TestGameInterfaceBaseProperty::testSet()
{
    // Base present
    {
        Environment env;
        game::map::Planet pl(33);
        configurePlanet(env, pl, true);

        // Successful assignment
        const afl::data::IntegerValue iv(1);
        TS_ASSERT_THROWS_NOTHING(setBaseProperty(pl, game::interface::ibpMission, &iv));
        TS_ASSERT_EQUALS(pl.getBaseMission().orElse(-1), 1);

        // Failing assignment
        TS_ASSERT_THROWS(setBaseProperty(pl, game::interface::ibpBaseDamage, &iv), interpreter::Error);
    }

    // No base present
    {
        Environment env;
        game::map::Planet pl(33);
        configurePlanet(env, pl, false);

        // Successful assignment
        const afl::data::IntegerValue iv(1);
        TS_ASSERT_THROWS(setBaseProperty(pl, game::interface::ibpMission, &iv), interpreter::Error);

        // Failing assignment
        TS_ASSERT_THROWS(setBaseProperty(pl, game::interface::ibpBaseDamage, &iv), interpreter::Error);
    }
}

