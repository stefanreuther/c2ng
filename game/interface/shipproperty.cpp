/**
  *  \file game/interface/shipproperty.cpp
  */

#include <cmath>
#include "game/interface/shipproperty.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/cargospec.hpp"
#include "game/exception.hpp"
#include "game/interface/inboxsubsetvalue.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/map/fleet.hpp"
#include "game/map/fleetmember.hpp"
#include "game/map/shippredictor.hpp"
#include "game/map/shiputils.hpp"
#include "game/root.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/mission.hpp"
#include "game/stringverifier.hpp"
#include "game/tables/headingname.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/functionvalue.hpp"
#include "interpreter/values.hpp"
#include "util/math.hpp"

using interpreter::makeStringValue;
using interpreter::makeIntegerValue;
using interpreter::makeFloatValue;
using interpreter::makeBooleanValue;
using interpreter::makeOptionalIntegerValue;
using interpreter::makeOptionalStringValue;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;

namespace {
    class ShipArrayProperty : public interpreter::FunctionValue {
     public:
        enum Type {
            Score,
            HasFunction
        };
        ShipArrayProperty(Type type,
                          const game::map::Ship& ship,
                          afl::base::Ref<const game::Game> game,
                          afl::base::Ref<const game::Root> root,
                          afl::base::Ref<const game::spec::ShipList> shipList);

        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual ShipArrayProperty* clone() const;

     private:
        const Type m_type;
        const game::map::Ship& m_ship;
        afl::base::Ref<const game::Game> m_game;
        afl::base::Ref<const game::Root> m_root;
        afl::base::Ref<const game::spec::ShipList> m_shipList;
    };

    /** Classify ship. This yields the ship's category as a string, 0 if unknown. */
    const char* classifyShip(const game::map::Ship& sh, const game::spec::ShipList& shipList)
    {
        // ex shipacc.pas:ShipType (sort-of)
        game::IntegerProperty_t beams = sh.getNumBeams();
        game::IntegerProperty_t tubes = sh.getNumLaunchers();
        game::IntegerProperty_t bays = sh.getNumBays();

        int hullNr;
        if (sh.getHull().get(hullNr)) {
            if (const game::spec::Hull* pHull = shipList.hulls().get(hullNr)) {
                beams = beams.orElse(pHull->getMaxBeams());
                tubes = tubes.orElse(pHull->getMaxLaunchers());
                bays = bays.orElse(pHull->getNumBays());
            }
        }

        int n;
        if (bays.get(n) && n > 0) {
            return "Carrier";
        } else if (tubes.get(n) && n > 0) {
            return "Torpedo Ship";
        } else if (beams.get(n) && n > 0) {
            return "Beam Weapons";
        } else if (tubes.isSame(0) && beams.isSame(0) && bays.isSame(0)) {
            return "Freighter";
        } else {
            return 0;
        }
    }

    struct FunctionMap {
        char ch;
        int basicFunction : 8;
    };
    const FunctionMap functions[] = {
        {'C', game::spec::BasicHullFunction::Cloak},
        {'C', game::spec::BasicHullFunction::AdvancedCloak},
        {'C', game::spec::BasicHullFunction::HardenedCloak},
        {'H', game::spec::BasicHullFunction::Hyperdrive},
        {'G', game::spec::BasicHullFunction::Gravitonic},
        {'B', game::spec::BasicHullFunction::Bioscan},
        {'B', game::spec::BasicHullFunction::FullBioscan},
        {'A', game::spec::BasicHullFunction::MerlinAlchemy},
        {'A', game::spec::BasicHullFunction::AriesRefinery},
        {'A', game::spec::BasicHullFunction::NeutronicRefinery},
    };

    String_t getSpecialFunctionsString(const game::map::Ship& sh,
                                       const game::UnitScoreDefinitionList& scoreDefinitions,
                                       const game::spec::ShipList& shipList,
                                       const game::config::HostConfiguration& config)
    {
        String_t result;
        if (sh.isVisible()) {
            afl::base::Memory<const FunctionMap> fs(functions);
            char last = '\0';
            while (const FunctionMap* f = fs.eat()) {
                if (f->ch != last && sh.hasSpecialFunction(f->basicFunction, scoreDefinitions, shipList, config)) {
                    result += f->ch;
                    last = f->ch;
                }
            }
        }
        return result;
    }
}

/*************************** ShipArrayProperty ***************************/

ShipArrayProperty::ShipArrayProperty(Type type,
                                     const game::map::Ship& ship,
                                     afl::base::Ref<const game::Game> game,                // needed for shipScores (Score)
                                     afl::base::Ref<const game::Root> root,                // needed for hostConfiguration (HasFunction)
                                     afl::base::Ref<const game::spec::ShipList> shipList)  // needed for hull functions
    : m_type(type),
      m_ship(ship),
      m_game(game),
      m_root(root),
      m_shipList(shipList)
{ }

afl::data::Value*
ShipArrayProperty::get(interpreter::Arguments& args)
{
    // ex shipint.pas:CShipContext.ResolveFuncall (sort-of)
    switch (m_type) {
     case Score:
        /* @q Score:Int() (Ship Property, Planet Property)
           Get unit's score of a given type.

           PHost can associate various scores with ships and planets (utilX.dat records 49 and 50).
           This property makes these scores available to scripts.
           Valid parameters can be found in the PHost documentation.
           As of PHost 4.1, the following values are valid:

           - Score(1): experience level (same as {Level}).
           - Score(2): experience points.

           This property yields EMPTY if the respective score does not exist or is not known.
           @since PCC2 1.99.21, PCC 1.1.16 */
        {
            args.checkArgumentCount(1);
            int32_t id;
            if (!checkIntegerArg(id, args.getNext(), 0, 0x7FFF)) {
                return 0;
            }

            game::UnitScoreDefinitionList::Index_t index;
            int16_t value, turn;
            if (m_game->shipScores().lookup(int16_t(id), index)
                && m_ship.unitScores().get(index, value, turn))
            {
                return makeIntegerValue(value);
            } else {
                return 0;
            }
        }

     case HasFunction:
        /* @q HasFunction:Bool() (Ship Property)
           True if the ship has the specified hull function used as index.
           The index is either the name ("Gravitonic") or number (7) of the function,
           as it can be used in SHIPLIST.TXT.

           This property considers all functions assigned to this ship type as well as to this individual ship,
           and honors level restrictions.
           @since PCC2 1.99.21, PCC 1.1.15 */
        {
            args.checkArgumentCount(1);
            String_t func;
            if (!checkStringArg(func, args.getNext())) {
                return 0;
            }
            int funcId;
            if (afl::string::strToInteger(func, funcId)) {
                // accept
            } else if (const game::spec::BasicHullFunction* hf = m_shipList->basicHullFunctions().getFunctionByName(func, false)) {
                funcId = hf->getId();
            } else {
                throw interpreter::Error("Invalid hull function name");
            }
            return makeBooleanValue(m_ship.hasSpecialFunction(funcId, m_game->shipScores(), *m_shipList, m_root->hostConfiguration()));
        }
    }
    return 0;
}

ShipArrayProperty*
ShipArrayProperty::clone() const
{
    // ex IntShipArrayProperty::clone
    return new ShipArrayProperty(*this);
}

afl::data::Value*
game::interface::getShipProperty(const game::map::Ship& sh, ShipProperty isp,
                                 Session& session,                                     // needed for names, HasTask
                                 afl::base::Ref<const Root> root,                      // needed for configuration
                                 afl::base::Ref<const game::spec::ShipList> shipList,  // needed for spec access
                                 afl::base::Ref<const Game> game,                      // needed for ship scores
                                 afl::base::Ref<const Turn> turn)                      // needed for location names
{
    // ex int/if/shipif.h:getShipProperty
    /* Combat participant properties often share names and meaning with ship properties,
       and are therefore documented here as well for brevity. Documenting them separately
       in getVcrSideProperty() would generate new documentation fragments and disambiguations
       for every item. */

    int n;
    game::map::Point pt;
    switch (isp) {
     case ispAuxId:
        /* @q Aux$:Int (Ship Property, Combat Participant Property)
           Type of secondary weapon.
           - 1..10 for torpedoes
           - 11 for fighters
           - EMPTY if no secondary weapon, or not known. */
        if (sh.getNumBays().get(n) && n > 0) {
            return makeIntegerValue(shipList->launchers().size() + 1);
        } else if (sh.getTorpedoType().get(n) && n > 0) {
            return makeIntegerValue(n);
        } else {
            return 0;
        }
     case ispAuxAmmo:
        /* @q Aux.Ammo:Int (Ship Property, Combat Participant Property)
           Number of fighters/torpedoes. */
        return makeOptionalIntegerValue(sh.getAmmo());
     case ispAuxCount:
        /* @q Aux.Count:Int (Ship Property, Combat Participant Property)
           Number of fighter bays/torpedo launchers. */
        if (sh.getNumBays().get(n) && n > 0) {
            return makeIntegerValue(n);
        } else if (sh.getTorpedoType().get(n) && n > 0 && sh.getNumLaunchers().get(n)) {
            return makeIntegerValue(n);
        } else {
            return 0;
        }
     case ispAuxShort:
        /* @q Aux.Short:Str (Ship Property, Combat Participant Property)
           Secondary weapon type, short name.
           @see Aux (Ship Property) */
        if (sh.getNumBays().get(n) && n > 0) {
            return makeStringValue("Ftr");
        } else if (sh.getTorpedoType().get(n) && n > 0) {
            return makeOptionalStringValue(shipList->launchers().shortNames(shipList->componentNamer())(n));
        } else {
            return 0;
        }
     case ispAuxName:
        /* @q Aux:Str (Ship Property, Combat Participant Property)
           Secondary weapon type, full name.
           Either a torpedo system name, "Fighters", or EMPTY. */
        if (sh.getNumBays().get(n) && n > 0) {
            return makeStringValue("Fighters");
        } else if (sh.getTorpedoType().get(n) && n > 0) {
            return makeOptionalStringValue(shipList->launchers().names(shipList->componentNamer())(n));
        } else {
            return 0;
        }
     case ispBeamId:
        /* @q Beam$:Int (Ship Property, Combat Participant Property)
           Beam type. 0 if none, EMPTY if not known. */
        return makeOptionalIntegerValue(sh.getBeamType());
     case ispBeamCount:
        /* @q Beam.Count:Int (Ship Property, Combat Participant Property)
           Number of beams. */
        return makeOptionalIntegerValue(sh.getNumBeams());
     case ispBeamShort:
        /* @q Beam.Short:Str (Ship Property, Combat Participant Property)
           Beam type, short name. */
        return makeOptionalStringValue(shipList->beams().shortNames(shipList->componentNamer())(sh.getBeamType()));
     case ispBeamName:
        /* @q Beam:Str (Ship Property, Combat Participant Property)
           Beam type, full name. */
        return makeOptionalStringValue(shipList->beams().names(shipList->componentNamer())(sh.getBeamType()));
     case ispCargoColonists:
        /* @q Cargo.Colonists:Int (Ship Property)
           Number of colonists aboard this ship. */
        return makeOptionalIntegerValue(sh.getCargo(Element::Colonists));
     case ispCargoD:
        /* @q Cargo.D:Int (Ship Property)
           Duranium aboard this ship, kilotons. */
        return makeOptionalIntegerValue(sh.getCargo(Element::Duranium));
     case ispCargoFree:
        /* @q Cargo.Free:Int (Ship Property)
           Free cargo room. */
        return makeOptionalIntegerValue(sh.getFreeCargo(*shipList));
     case ispCargoM:
        /* @q Cargo.M:Int (Ship Property)
           Molybdenum aboard this ship, kilotons. */
        return makeOptionalIntegerValue(sh.getCargo(Element::Molybdenum));
     case ispCargoMoney:
        /* @q Cargo.Money:Int (Ship Property)
           Money aboard this ship. */
        return makeOptionalIntegerValue(sh.getCargo(Element::Money));
     case ispCargoN:
        /* @q Cargo.N:Int (Ship Property)
           Neutronium aboard this ship, kilotons. */
        return makeOptionalIntegerValue(sh.getCargo(Element::Neutronium));
     case ispCargoStr:
        /* @q Cargo.Str:Cargo (Ship Property)
           Cargo aboard this ship.
           String containing amounts of minerals, supplies, colonists, cash, and torpedoes/fighters. */
     {
         CargoSpec cs;
         bool haveAny = false;
         int32_t n;
         int tt;
         if (sh.getCargo(Element::Neutronium).get(n)) {
             cs.set(CargoSpec::Neutronium, n);
             haveAny = true;
         }
         if (sh.getCargo(Element::Tritanium).get(n)) {
             cs.set(CargoSpec::Tritanium, n);
             haveAny = true;
         }
         if (sh.getCargo(Element::Duranium).get(n)) {
             cs.set(CargoSpec::Duranium, n);
             haveAny = true;
         }
         if (sh.getCargo(Element::Molybdenum).get(n)) {
             cs.set(CargoSpec::Molybdenum, n);
             haveAny = true;
         }
         if (sh.getCargo(Element::Supplies).get(n)) {
             cs.set(CargoSpec::Supplies, n);
             haveAny = true;
         }
         if (sh.getCargo(Element::Money).get(n)) {
             cs.set(CargoSpec::Money, n);
             haveAny = true;
         }
         if (sh.getCargo(Element::Colonists).get(n)) {
             cs.set(CargoSpec::Colonists, n);
             haveAny = true;
         }
         if (sh.getCargo(Element::Fighters).get(n)) {
             cs.set(CargoSpec::Fighters, n);
             haveAny = true;
         }
         if (sh.getTorpedoType().get(tt) && tt > 0 && sh.getAmmo().get(n)) {
             cs.set(CargoSpec::Torpedoes, n);
             haveAny = true;
         }

         if (haveAny) {
             return makeStringValue(cs.toCargoSpecString());
         } else {
             return 0;
         }
     }
     case ispCargoSupplies:
        /* @q Cargo.Supplies:Int (Ship Property)
           Supplies aboard this ship, kilotons. */
        return makeOptionalIntegerValue(sh.getCargo(Element::Supplies));
     case ispCargoT:
        /* @q Cargo.T:Int (Ship Property)
           Tritanium aboard this ship, kilotons. */
        return makeOptionalIntegerValue(sh.getCargo(Element::Tritanium));
     case ispCrew:
        /* @q Crew:Int (Ship Property)
           Current crew size. */
        return makeOptionalIntegerValue(sh.getCrew());
     case ispDamage:
        /* @q Damage:Int (Ship Property, Combat Participant Property)
           Damage level in percent. */
        return makeOptionalIntegerValue(sh.getDamage());
     case ispEnemyId:
        /* @q Enemy$:Int (Ship Property)
           Primary Enemy. 0=none, or a player number.
           @assignable
           @see SetEnemy (Ship Command) */
        return makeOptionalIntegerValue(sh.getPrimaryEnemy());
     case ispEngineId:
        /* @q Engine$:Int (Ship Property)
           Type of engine. */
        return makeOptionalIntegerValue(sh.getEngineType());
     case ispEngineName:
        /* @q Engine:Str (Ship Property)
           Type of engine, full name. */
        return makeOptionalStringValue(shipList->engines().names(shipList->componentNamer())(sh.getEngineType()));
     case ispFCode:
        /* @q FCode:Str (Ship Property)
           Friendly code.
           @assignable
           @see SetFCode (Ship Command) */
        return makeOptionalStringValue(sh.getFriendlyCode());
     case ispFighterBays:
        /* @q Fighter.Bays:Int (Ship Property, Combat Participant Property)
           Number of fighter bays. */
        return makeOptionalIntegerValue(sh.getNumBays());
     case ispFighterCount:
        /* @q Fighter.Count:Int (Ship Property, Combat Participant Property)
           Number of fighters. */
         if (sh.getNumBays().get(n) && n > 0) {
             return makeOptionalIntegerValue(sh.getAmmo());
         } else {
             return 0;
         }
     case ispFleetId:
        /* @q Fleet$:Int (Ship Property)
           Id of fleet this ship is in.
           @assignable
           @see SetFleet (Ship Command) */
        return makeIntegerValue(sh.getFleetNumber());
     case ispFleetName:
        /* @q Fleet.Name:Str (Ship Property)
           Name of fleet this ship is leader of.
           Has a value, and is assignable, only for ships that actually are fleet leaders
           (i.e. <tt>Fleet$ = Id$</tt>).
           @assignable */
        return makeStringValue(sh.getFleetName());
     case ispFleetStatus:
        /* @q Fleet.Status:Str (Ship Property)
           Fleet status. One of
           - "leader"
           - "member"
           - "-" */
        if (int fid = sh.getFleetNumber()) {
            if (fid == sh.getId()) {
                return makeStringValue("leader");
            } else {
                return makeStringValue("member");
            }
        } else {
            return makeStringValue("-");
        }
     case ispFleet:
        /* @q Fleet:Str (Ship Property)
           Name of fleet this ship is in.
           If this ship is leader of a fleet, and the fleet has a name ({Fleet.Name}), returns that.
           Otherwise, returns the name ({Name (Ship Property)|Name}) of the leader.
           If the ship is not member of a fleet, this property is EMPTY. */
        if (int fid = sh.getFleetNumber()) {
            if (game::map::Ship* leader = turn->universe().ships().get(fid)) {
                String_t result = leader->getFleetName();
                if (result.empty()) {
                    result = leader->getName(LongName, session.translator(), session.interface());
                }
                return makeStringValue(result);
            }
        }
        return 0;
     case ispHeadingAngle:
        /* @q Heading$:Int (Ship Property)
           Current angle of movement, in degrees.
           EMPTY if the ship is not moving, or the angle is not known. */
        return makeOptionalIntegerValue(sh.getHeading());
     case ispHeadingName:
        /* @q Heading:Str (Ship Property)
           Current angle of movement, as compass direction. */
        return makeOptionalStringValue(game::tables::HeadingName()(sh.getHeading()));
     case ispHullSpecial:
        /* @q Hull.Special:Str (Ship Property)
           Special function summary.
           This is a string identifying the major special functions of this ship.
           The string will contain each letter if and only if the ship
           has the respective ability assigned for all players.
           - "C" (Cloak, including Advanced and Hardened Cloak)
           - "H" (Hyperdrive)
           - "G" (Gravitonic accelerator)
           - "B" (Bioscan, including Full Bioscan)
           - "A" (Alchemy, including Neutronic/Aries Refinery) */
        return makeStringValue(getSpecialFunctionsString(sh, game->shipScores(), *shipList, root->hostConfiguration()));
     case ispId:
        /* @q Id:Int (Ship Property)
           Ship Id. */
        return makeIntegerValue(sh.getId());
     case ispLevel:
        /* @q Level:Int (Ship Property)
           Ship's experience level.
           If the experience system is not enabled, or the level is not known, yields EMPTY. */
        {
            UnitScoreList::Index_t index;
            int16_t value, turn;
            if (game->shipScores().lookup(ScoreId_ExpLevel, index) && sh.unitScores().get(index, value, turn)) {
                return makeIntegerValue(value);
            } else {
                return 0;
            }
        }
     case ispLocX:
        /* @q Loc.X:Int (Ship Property)
           X location of ship. */
        if (sh.getPosition(pt)) {
            return makeIntegerValue(pt.getX());
        } else {
            return 0;
        }
     case ispLocY:
        /* @q Loc.Y:Int (Ship Property)
           Y location of ship. */
        if (sh.getPosition(pt)) {
            return makeIntegerValue(pt.getY());
        } else {
            return 0;
        }
     case ispLoc:
        /* @q Loc:Str (Ship Property)
           Location of ship, as a human-readable string.
           If the ship is at a planet, returns that planet's name and Id.
           In deep space, returns an (X,Y) pair. */
        if (sh.isVisible() && sh.getPosition(pt)) {
            return makeStringValue(turn->universe().findLocationName(pt, 0, game->mapConfiguration(), root->hostConfiguration(), root->hostVersion(), session.translator()));
        } else {
            return 0;
        }
     case ispMarked:
        /* @q Marked:Bool (Ship Property)
           True if ship is marked. */
        return makeBooleanValue(sh.isMarked());
     case ispMass:
        /* @q Mass:Int (Ship Property)
           Mass of ship (hull, components, and cargo). */
        return makeOptionalIntegerValue(sh.getMass(*shipList));
     case ispMessages:
        /* @q Messages:Obj() (Ship Property)
           If this ship has any messages, this property is non-null and contains an array of messages.
           Individual messages have the same form as the inbox messages (InMsg()).
           @see int:index:group:incomingmessageproperty|Incoming Message Properties
           @since PCC2 2.0.3, PCC2 2.40.10 */
        return InboxSubsetValue::create(sh.messages().get(), session.translator(), root, game);
     case ispMissionId:
        /* @q Mission$:Int (Ship Property)
           Mission number.
           @assignable
           @see SetMission (Ship Command) */
        return makeOptionalIntegerValue(sh.getMission());
     case ispMissionIntercept:
        /* @q Mission.Intercept:Int (Ship Property)
           Mission "Intercept" parameter.
           @assignable
           @see SetMission (Ship Command) */
        return makeOptionalIntegerValue(sh.getMissionParameter(InterceptParameter));
     case ispMissionShort:
        /* @q Mission.Short:Str (Ship Property)
           Mission, short name. */
        if (const game::spec::Mission* msn = getShipMission(sh, root->hostConfiguration(), shipList->missions())) {
            return makeStringValue(msn->getShortName());
        } else {
            int m;
            if (sh.getMission().get(m)) {
                return makeStringValue(afl::string::Format("MIT %d", m));
            } else {
                return 0;
            }
        }
     case ispMissionTow:
        /* @q Mission.Tow:Int (Ship Property)
           Mission "Tow" parameter.
           @assignable
           @see SetMission (Ship Command) */
        return makeOptionalIntegerValue(sh.getMissionParameter(TowParameter));
     case ispMissionName:
        /* @q Mission:Str (Ship Property)
           Mission, full name. */
        if (const game::spec::Mission* msn = getShipMission(sh, root->hostConfiguration(), shipList->missions())) {
            return makeStringValue(msn->getName());
        } else {
            int m, i, t;
            if (sh.getMission().get(m)
                && sh.getMissionParameter(InterceptParameter).get(i)
                && sh.getMissionParameter(TowParameter).get(t))
            {
                return makeStringValue(afl::string::Format("M.I.T. %d (%d,%d)", m, i, t));
            } else {
                return 0;
            }
        }
     case ispMoveETA:
        /* @q Move.ETA:Int (Ship Property)
           Estimated time of arrival at waypoint (number of turns). */
        if (sh.getShipKind() == sh.CurrentShip) {
            game::map::ShipPredictor pred(turn->universe(),
                                          sh.getId(),
                                          game->shipScores(),
                                          *shipList,
                                          game->mapConfiguration(),
                                          root->hostConfiguration(),
                                          root->hostVersion(),
                                          root->registrationKey());
            pred.addTowee();
            pred.computeMovement();
            return makeIntegerValue(pred.getNumTurns());
        } else {
            return 0;
        }
     case ispMoveFuel:
        /* @q Move.Fuel:Int (Ship Property)
           Predicted fuel useage for movement, in kilotons. */
        if (sh.getShipKind() == sh.CurrentShip) {
            game::map::ShipPredictor pred(turn->universe(),
                                          sh.getId(),
                                          game->shipScores(),
                                          *shipList,
                                          game->mapConfiguration(),
                                          root->hostConfiguration(),
                                          root->hostVersion(),
                                          root->registrationKey());
            pred.addTowee();
            pred.computeMovement();
            return makeIntegerValue(pred.getMovementFuelUsed());
        } else {
            return 0;
        }
     case ispName:
        /* @q Name:Str (Ship Property)
           Ship name.
           @assignable
           @see SetName (Ship Command) */
        if (sh.isVisible()) {
            return makeStringValue(sh.getName());
        } else {
            return 0;
        }
     case ispOrbitId:
        /* @q Orbit$:Int (Ship Property)
           Id of planet this ship is orbiting. 0 if none. */
        if (sh.getPosition(pt)) {
            return makeIntegerValue(turn->universe().findPlanetAt(pt));
        } else {
            return 0;
        }
     case ispOrbitName:
        /* @q Orbit:Str (Ship Property)
           Name of planet this ship is orbiting. EMPTY if none. */
        if (sh.getPosition(pt)) {
            if (const Id_t pid = turn->universe().findPlanetAt(pt)) {
                if (const game::map::Planet* p = turn->universe().planets().get(pid)) {
                    return makeStringValue(p->getName(session.translator()));
                }
            }
        }
        return 0;
     case ispPlayed:
        /* @q Played:Bool (Ship Property)
           True if this ship is played.
           @since PCC 1.1.19 */
        return makeBooleanValue(sh.isPlayable(sh.Playable));
     case ispRealOwner:
        /* @q Owner.Real:Int (Ship Property)
           Real owner of this ship, player number.
           The real owner can differ from the {Owner (Ship Property)|Owner} reported normally
           when the ship is under remote control. */
        return makeOptionalIntegerValue(sh.getRealOwner());
     case ispReference:
        /* @q Ref:Reference (Ship Property)
           Symbolic reference to this ship.
           If given an object of unknown type, this can be used to identify this object as a ship.
           @since PCC2 2.40.13 */
        return new ReferenceContext(Reference(Reference::Ship, sh.getId()), session);
     case ispSpeedId:
        /* @q Speed$:Int (Ship Property)
           Speed (warp factor).
           @assignable
           @see SetSpeed (Ship Command) */
        return makeOptionalIntegerValue(sh.getWarpFactor());
     case ispSpeedName:
        /* @q Speed:Str (Ship Property)
           Speed, as human-readable string.
           If the hyperdrive is active, reports "Hyperdrive", otherwise "Warp x". */
        if (sh.getWarpFactor().get(n)) {
            if (sh.isHyperdriving(game->shipScores(), *shipList, root->hostConfiguration())) {
                return makeStringValue("Hyperdrive");
            } else {
                return makeStringValue(afl::string::Format("Warp %d", n));
            }
        } else {
            return 0;
        }
     case ispTask:
        /* @q Task:Bool (Ship Property)
           True if this ship has an auto task. */
        return makeBooleanValue(session.interface().hasTask(InterpreterInterface::Ship, sh.getId()));
     case ispTorpId:
        /* @q Torp$:Int (Ship Property, Combat Participant Property)
           Torpedo type. */
        return makeOptionalIntegerValue(sh.getTorpedoType());
     case ispTorpCount:
        /* @q Torp.Count:Int (Ship Property, Combat Participant Property)
           Number of torpedoes on this ship. 0 if the ship has no torpedoes. */
        if (sh.getTorpedoType().get(n)) {
            if (n > 0) {
                return makeOptionalIntegerValue(sh.getAmmo());
            } else {
                return makeIntegerValue(0);
            }
        } else {
            return 0;
        }
     case ispTorpLCount:
        /* @q Torp.LCount:Int (Ship Property, Combat Participant Property)
           Number of torpedo launchers on this ship. */
        return makeOptionalIntegerValue(sh.getNumLaunchers());
     case ispTorpShort:
        /* @q Torp.Short:Str (Ship Property, Combat Participant Property)
           Torpedo type, short name. */
        return makeOptionalStringValue(shipList->launchers().shortNames(shipList->componentNamer())(sh.getTorpedoType()));
     case ispTorpName:
        /* @q Torp:Str (Ship Property, Combat Participant Property)
           Torpedo type, full name. */
        return makeOptionalStringValue(shipList->launchers().names(shipList->componentNamer())(sh.getTorpedoType()));
     case ispTransferShipColonists:
        /* @q Transfer.Ship.Colonists:Int (Ship Property)
           Number of colonists being transferred to another ship. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.TransferTransporter, Element::Colonists));
     case ispTransferShipD:
        /* @q Transfer.Ship.D:Int (Ship Property)
           Amount of Duranium being transferred to another ship. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.TransferTransporter, Element::Duranium));
     case ispTransferShipId:
        /* @q Transfer.Ship.Id:Int (Ship Property)
           Id of cargo transfer target ship. */
        return makeOptionalIntegerValue(sh.getTransporterTargetId(sh.TransferTransporter));
     case ispTransferShipM:
        /* @q Transfer.Ship.M:Int (Ship Property)
           Amount of Molybdenum being transferred to another ship. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.TransferTransporter, Element::Molybdenum));
     case ispTransferShipN:
        /* @q Transfer.Ship.N:Int (Ship Property)
           Amount of Neutronium being transferred to another ship. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.TransferTransporter, Element::Neutronium));
     case ispTransferShipName:
        /* @q Transfer.Ship.Name:Str (Ship Property)
           Name of cargo transfer target ship. */
        if (sh.getTransporterTargetId(sh.TransferTransporter).get(n)) {
            if (const game::map::Ship* otherShip = turn->universe().ships().get(n)) {
                return makeStringValue(otherShip->getName());
            }
        }
        return 0;
     case ispTransferShipT:
        /* @q Transfer.Ship.T:Int (Ship Property)
           Amount of Tritanium being transferred to another ship. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.TransferTransporter, Element::Tritanium));
     case ispTransferShipSupplies:
        /* @q Transfer.Ship.Supplies:Int (Ship Property)
           Amount of Supplies being transferred to another ship. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.TransferTransporter, Element::Supplies));
     case ispTransferShip:
        /* @q Transfer.Ship:Bool (Ship Property)
           True if cargo is being transported to another ship. */
        return sh.getShipKind() == sh.CurrentShip
            ? makeBooleanValue(sh.isTransporterActive(sh.TransferTransporter))
            : 0;
     case ispTransferUnloadColonists:
        /* @q Transfer.Unload.Colonists:Int (Ship Property)
           Number of colonists being unloaded to a planet or deep space. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.UnloadTransporter, Element::Colonists));
     case ispTransferUnloadD:
        /* @q Transfer.Unload.D:Int (Ship Property)
           Amount of Duranium being unloaded to a planet or deep space. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.UnloadTransporter, Element::Duranium));
     case ispTransferUnloadId:
        /* @q Transfer.Unload.Id:Int (Ship Property)
           Id of planet cargo is being unloaded to. 0 for jettison. */
        return makeOptionalIntegerValue(sh.getTransporterTargetId(sh.UnloadTransporter));
     case ispTransferUnloadM:
        /* @q Transfer.Unload.M:Int (Ship Property)
           Amount of Molybdenum being unloaded to a planet or deep space. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.UnloadTransporter, Element::Molybdenum));
     case ispTransferUnloadN:
        /* @q Transfer.Unload.N:Int (Ship Property)
           Amount of Neutronium being unloaded to a planet or deep space. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.UnloadTransporter, Element::Neutronium));
     case ispTransferUnloadName:
        /* @q Transfer.Unload.Name:Int (Ship Property)
           Name of planet cargo is being unloaded to. "Jettison" for jettison. */
        if (sh.isTransporterActive(sh.TransferTransporter) && sh.getTransporterTargetId(sh.TransferTransporter).get(n)) {
            if (n == 0) {
                return makeStringValue("Jettison");
            }
            if (const game::map::Planet* pl = turn->universe().planets().get(n)) {
                return makeStringValue(pl->getName(session.translator()));
            }
        }
        return 0;
     case ispTransferUnloadT:
        /* @q Transfer.Unload.T:Int (Ship Property)
           Amount of Tritanium being unloaded to a planet or deep space. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.UnloadTransporter, Element::Tritanium));
     case ispTransferUnloadSupplies:
        /* @q Transfer.Unload.Supplies:Int (Ship Property)
           Amount of Supplies being unloaded to a planet or deep space. */
        return makeOptionalIntegerValue(sh.getTransporterCargo(sh.UnloadTransporter, Element::Supplies));
     case ispTransferUnload:
        /* @q Transfer.Unload:Bool (Ship Property)
           True if cargo is being unloaded to a planet or deep space. */
        return sh.getShipKind() == sh.CurrentShip
            ? makeBooleanValue(sh.isTransporterActive(sh.UnloadTransporter))
            : 0;
     case ispTypeChar:
        /* @q Type.Short:Str (Ship Property)
           Classification of ship, short.
           This is the first letter of the {Type (Ship Property)|Type}, see there. */
        if (const char* p = classifyShip(sh, *shipList)) {
            return makeStringValue(String_t(p, 1));
        } else {
            return 0;
        }
     case ispTypeStr:
        /* @q Type:Str (Ship Property)
           Classification of ship. Possible values are:
           - "Carrier"
           - "Torpedo Ship"
           - "Beam Weapons"
           - "Freighter" */
        if (const char* p = classifyShip(sh, *shipList)) {
            return makeStringValue(p);
        } else {
            return 0;
        }
     case ispWaypointDistance:
        /* @q Waypoint.Dist:Num (Ship Property)
           Distance to waypoint, in ly.
           This can be a fractional number. */
     {
         int dx, dy;
         if (sh.getWaypointDX().get(dx) && sh.getWaypointDY().get(dy)) {
             return makeFloatValue(std::sqrt(double(util::squareInteger(dx) + util::squareInteger(dy))));
         } else {
             return 0;
         }
     }
     case ispWaypointDX:
        /* @q Waypoint.DX:Int (Ship Property)
           X distance to waypoint. */
        return makeOptionalIntegerValue(sh.getWaypointDX());
     case ispWaypointDY:
        /* @q Waypoint.DY:Int (Ship Property)
           Y distance to waypoint. */
        return makeOptionalIntegerValue(sh.getWaypointDY());
     case ispWaypointPlanetId:
        /* @q Waypoint.Planet:Int (Ship Property)
           Id of planet at waypoint.
           @see PlanetAt() */
        if (sh.getWaypoint().get(pt)) {
            return makeIntegerValue(turn->universe().findPlanetAt(game->mapConfiguration().getCanonicalLocation(pt)));
        } else {
            return 0;
        }
     case ispWaypointX:
        /* @q Waypoint.X:Int (Ship Property)
           X location of waypoint. */
        if (sh.getWaypoint().get(pt)) {
            return makeIntegerValue(pt.getX());
        } else {
            return 0;
        }
     case ispWaypointY:
        /* @q Waypoint.Y:Int (Ship Property)
           Y location of waypoint. */
        if (sh.getWaypoint().get(pt)) {
            return makeIntegerValue(pt.getY());
        } else {
            return 0;
        }
     case ispWaypointName:
        /* @q Waypoint:Str (Ship Property)
           Waypoint, as a human-readable string. */
        if (sh.getWaypoint().get(pt)) {
            // FIXME: PCC 1.x also handles Intercept here
            if (sh.getWaypointDX().isSame(0) && sh.getWaypointDY().isSame(0)) {
                return makeStringValue("(Location)");
            } else {
                return makeStringValue(turn->universe().findLocationName(pt, 0, game->mapConfiguration(), root->hostConfiguration(), root->hostVersion(), session.translator()));
            }
        } else {
            return 0;
        }

     case ispScore:
        return new ShipArrayProperty(ShipArrayProperty::Score, sh, game, root, shipList);

     case ispHasFunction:
        return new ShipArrayProperty(ShipArrayProperty::HasFunction, sh, game, root, shipList);
    }
    return 0;
}

void
game::interface::setShipProperty(game::map::Ship& sh, ShipProperty isp, const afl::data::Value* value,
                                 Root& root,
                                 const game::spec::ShipList& shipList,
                                 const game::map::Configuration& mapConfig,
                                 Turn& turn)
{
    // ex int/if/shipif.h:setShipProperty

    // Everything is only assignable for own ships. As an exception, the name is also assignable for targets.
    if (!sh.isPlayable(game::map::Object::Playable)) {
        // \change PCC2 tests: if (sh.getShipKind() != GShip::CurrentShip)
        if (isp == ispName && sh.getShipKind() != sh.NoShip) {
            // accept
        } else {
            throw interpreter::Error::notAssignable();
        }
    }

    String_t sv;
    int32_t iv;
    switch (isp) {
     case ispFCode:
        if (checkStringArg(sv, value)) {
            if (!root.stringVerifier().isValidString(StringVerifier::FriendlyCode, sv)) {
                throw interpreter::Error::rangeError();
            }
            sh.setFriendlyCode(sv);
        }
        break;
     case ispMissionId:
     case ispMissionIntercept:
     case ispMissionTow:
        if (checkIntegerArg(iv, value, 0, MAX_NUMBER)) {
            // FIXME: this changes other values to 0 if they were unknown (PCC2 probably crashes)
            int m = (isp == ispMissionId        ? iv : sh.getMission().orElse(0));
            int i = (isp == ispMissionIntercept ? iv : sh.getMissionParameter(InterceptParameter).orElse(0));
            int t = (isp == ispMissionTow       ? iv : sh.getMissionParameter(TowParameter).orElse(0));
            if (!game::map::FleetMember(turn.universe(), sh, mapConfig).setMission(m, i, t, root.hostConfiguration(), shipList)) {
                throw Exception(Exception::eFleet);
            }
        }
        break;
     case ispName:
        if (checkStringArg(sv, value)) {
            if (!root.stringVerifier().isValidString(StringVerifier::ShipName, sv)) {
                throw interpreter::Error::rangeError();
            }
            sh.setName(sv);
        }
        break;
     case ispSpeedId:
        if (checkIntegerArg(iv, value, 0, game::spec::Engine::MAX_WARP)) {
            if (!game::map::FleetMember(turn.universe(), sh, mapConfig).setWarpFactor(iv, root.hostConfiguration(), shipList)) {
                throw Exception(Exception::eFleet);
            }
        }
        break;
     case ispEnemyId:
        if (checkIntegerArg(iv, value, 0, MAX_PLAYERS)) {
            // \change allow setting PE to all players from playerList, including aliens.
            // PHost allows [0,12], Tim-Host has no restriction.
            if (root.playerList().get(iv) == 0) {
                throw interpreter::Error::rangeError();
            }
            sh.setPrimaryEnemy(iv);
        }
        break;
     case ispFleetName:
        if (checkStringArg(sv, value)) {
            if (!game::map::FleetMember(turn.universe(), sh, mapConfig).setFleetName(sv)) {
                throw interpreter::Error::notAssignable();
            }
        }
        break;
     case ispFleetId:
        if (checkIntegerArg(iv, value)) {
            if (!game::map::FleetMember(turn.universe(), sh, mapConfig).setFleetNumber(iv, root.hostConfiguration(), shipList)) {
                throw interpreter::Error::rangeError();
            }
        }
        break;
     default:
        throw interpreter::Error::notAssignable();
    }
}
