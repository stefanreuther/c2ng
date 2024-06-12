/**
  *  \file game/interface/vcrsideproperty.cpp
  *  \brief Enum game::interface::VcrSideProperty
  */

#include "game/interface/vcrsideproperty.hpp"
#include "afl/string/format.hpp"
#include "game/vcr/object.hpp"
#include "interpreter/values.hpp"

using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using interpreter::makeStringValue;

namespace {
    /** Convert status flags to script value.
        \param battle Battle
        \param side Side to query
        \param config Host configuration
        \param shipList Ship list */
    afl::data::Value* makeVcrResult(game::vcr::Battle& battle,
                                    size_t side,
                                    const game::config::HostConfiguration& config,
                                    const game::spec::ShipList& shipList)
    {
        if (battle.getPlayability(config, shipList) != game::vcr::Battle::IsPlayable) {
            return makeStringValue("Invalid");
        } else {
            /* If all units survived, we declare this a Stalemate.
               This also applies to "Timeout" and "Disabled" results which are not in the generic interface.
               If this is a regular complete fight that we report as Survived for this guy, someone else will die or get captured. */
            int status = battle.getOutcome(config, shipList, side);
            if (status == 0) {
                for (size_t i = 0, n = battle.getNumObjects(); i < n; ++i) {
                    if (battle.getOutcome(config, shipList, i) != 0) {
                        return makeStringValue("Survived");
                    }
                }
                return makeStringValue("Stalemate");
            } else if (status < 0) {
                return makeStringValue("Exploded");
            } else {
                return makeStringValue("Captured");
            }
        }
    }
}

afl::data::Value*
game::interface::getVcrSideProperty(game::vcr::Battle& battle, size_t side, VcrSideProperty ivs,
                                    afl::string::Translator& tx,
                                    const game::spec::ShipList& shipList,
                                    const game::config::HostConfiguration& config,
                                    const PlayerList& players)
{
    // ex int/if/vcrif.h:getVcrSideProperty

    /* Note: this implements the owner properties directly instead of relying on
       getPlayerProperty() like most others. This makes it possible to use VCR
       properties without having a turn loaded. */
    /* FIXME: this does not implement PlanetsHaveTubes; for planets with tubes,
       it always returns the fighters. */
    const game::vcr::Object* obj = battle.getObject(side, false);
    if (obj == 0) {
        return 0;
    }

    switch (ivs) {
     case ivsAuxAmmo:
        /* Aux.Ammo documented in getShipProperty */
        /* @q Left.Aux.Ammo:Int (Combat Property), Right.Aux.Ammo:Int (Combat Property)
           Number of fighters or torpedoes on this unit. */
        if (obj->getNumBays() > 0) {
            return makeIntegerValue(obj->getNumFighters());
        } else if (obj->getNumLaunchers() > 0 && shipList.launchers().get(obj->getTorpedoType()) != 0) {
            return makeIntegerValue(obj->getNumTorpedoes());
        } else {
            return makeIntegerValue(0);
        }
     case ivsAuxCount:
        /* Aux.Count documented in getShipProperty */
        /* @q Left.Aux.Count:Int (Combat Property), Right.Aux.Count:Int (Combat Property)
           Number of fighter bays or torpedo launchers on this unit. */
        if (obj->getNumBays() > 0) {
            return makeIntegerValue(obj->getNumBays());
        } else if (obj->getNumLaunchers() > 0 && shipList.launchers().get(obj->getTorpedoType()) != 0) {
            return makeIntegerValue(obj->getNumLaunchers());
        } else {
            return 0;
        }
     case ivsAuxId:
        /* Aux$ documented in getShipProperty */
        /* @q Left.Aux$:Int (Combat Property), Right.Aux$:Int (Combat Property)
           Type of secondary weapon.
           - 1..10 for torpedoes
           - 11 for fighters
           - EMPTY if no secondary weapon. */
        if (obj->getNumBays() > 0) {
            return makeIntegerValue(shipList.launchers().size()+1);
        } else if (obj->getNumLaunchers() > 0 && shipList.launchers().get(obj->getTorpedoType()) != 0) {
            return makeIntegerValue(obj->getTorpedoType());
        } else {
            return 0;
        }
     case ivsAuxName:
        /* Aux documented in getShipProperty */
        /* @q Left.Aux:Str (Combat Property), Right.Aux:Str (Combat Property)
           Secondary weapon type, full name.
           Either a torpedo system name, "Fighters", or EMPTY. */
        if (obj->getNumBays() > 0) {
            return makeStringValue("Fighters");
        } else if (obj->getNumLaunchers() > 0) {
            if (const game::spec::Component* comp = shipList.launchers().get(obj->getTorpedoType())) {
                return makeStringValue(comp->getName(shipList.componentNamer()));
            } else {
                return 0;
            }
        } else {
            return 0;
        }
     case ivsAuxShort:
        /* Aux.Short documented in getShipProperty */
        /* @q Left.Aux.Short:Str (Combat Property), Right.Aux.Short:Str (Combat Property)
           Secondary weapon type, short name.
           @see Left.Aux (Combat Property) */
        if (obj->getNumBays() > 0) {
            return makeStringValue("Ftr");
        } else if (obj->getNumLaunchers() > 0) {
            if (const game::spec::Component* comp = shipList.launchers().get(obj->getTorpedoType())) {
                return makeStringValue(comp->getShortName(shipList.componentNamer()));
            } else {
                return 0;
            }
        } else {
            return 0;
        }
     case ivsFighterBays:
        /* Fighter.Bays documented in getShipProperty */
        /* @q Left.Fighter.Bays:Int (Combat Property), Right.Fighter.Bays:Int (Combat Property)
           Number of fighter bays. */
        return makeIntegerValue(obj->getNumBays());
     case ivsFighterCount:
        /* Fighter.Count documented in getShipProperty */
        /* @q Left.Fighter.Count:Int (Combat Property), Right.Fighter.Count:Int (Combat Property)
           Number of fighters. */
        return makeIntegerValue(obj->getNumBays() > 0 ? obj->getNumFighters() : 0);
     case ivsTorpId:
        /* Torp$ documented in getShipProperty */
        /* @q Left.Torp$:Int (Combat Property), Right.Torp$:Int (Combat Property)
           Torpedo type. */
        if (obj->getNumLaunchers() > 0 && shipList.launchers().get(obj->getTorpedoType()) != 0) {
            return makeIntegerValue(obj->getTorpedoType());
        } else {
            return 0;
        }
     case ivsTorpCount:
        /* Torp.Count documented in getShipProperty */
        /* @q Left.Torp.Count:Int (Combat Property), Right.Torp.Count:Int (Combat Property)
           Number of torpedoes. 0 if the ship has no torpedoes. */
        if (obj->getNumLaunchers() > 0 && shipList.launchers().get(obj->getTorpedoType()) != 0) {
            return makeIntegerValue(obj->getNumTorpedoes());
        } else {
            return makeIntegerValue(0);
        }

     case ivsTorpLCount:
        /* Torp.LCount documented in getShipProperty */
        /* @q Left.Torp.LCount:Int (Combat Property), Right.Torp.LCount:Int (Combat Property)
           Number of torpedo launchers on this ship. */
        if (obj->getNumLaunchers() > 0 && shipList.launchers().get(obj->getTorpedoType()) != 0) {
            return makeIntegerValue(obj->getNumLaunchers());
        } else {
            return makeIntegerValue(0);
        }
     case ivsTorpShort:
        /* Torp.Short documented in getShipProperty */
        /* @q Left.Torp.Short:Str (Combat Property), Right.Torp.Short:Str (Combat Property)
           Torpedo type, short name. */
        if (obj->getNumLaunchers() > 0) {
            if (const game::spec::Component* comp = shipList.launchers().get(obj->getTorpedoType())) {
                return makeStringValue(comp->getShortName(shipList.componentNamer()));
            } else {
                return 0;
            }
        } else {
            return 0;
        }
     case ivsTorpName:
        /* Torp documented in getShipProperty */
        /* @q Left.Torp:Str (Combat Property), Right.Torp:Str (Combat Property)
           Torpedo type, full name. */
        if (obj->getNumLaunchers() > 0) {
            if (const game::spec::Component* comp = shipList.launchers().get(obj->getTorpedoType())) {
                return makeStringValue(comp->getName(shipList.componentNamer()));
            } else {
                return 0;
            }
        } else {
            return 0;
        }
     case ivsBeamCount:
        /* Beam.Count documented in getShipProperty */
        /* @q Left.Beam.Count:Int (Combat Property), Right.Beam.Count:Int (Combat Property)
           Number of beams. */
        return makeIntegerValue(obj->getNumBeams());
     case ivsBeamId:
        /* Beam$ documented in getShipProperty */
        /* @q Left.Beam$:Int (Combat Property), Right.Beam$:Int (Combat Property)
           Beam type. 0 if none, EMPTY if not known. */
        return makeIntegerValue(obj->getBeamType());
     case ivsBeamName:
        /* Beam documented in getShipProperty */
        /* @q Left.Beam:Str (Combat Property), Right.Beam:Str (Combat Property)
           Beam type, full name. */
        if (const game::spec::Beam* beam = shipList.beams().get(obj->getBeamType())) {
            return makeStringValue(beam->getName(shipList.componentNamer()));
        } else {
            return 0;
        }
     case ivsBeamShort:
        /* Beam.Short documented in getShipProperty */
        /* @q Left.Beam.Short:Str (Combat Property), Right.Beam.Short:Str (Combat Property)
           Beam type, short name. */
        if (const game::spec::Beam* beam = shipList.beams().get(obj->getBeamType())) {
            return makeStringValue(beam->getShortName(shipList.componentNamer()));
        } else {
            return 0;
        }
     case ivsCrew:
        /* @q Crew:Int (Combat Participant Property)
           @q Left.Crew:Int (Combat Property), Right.Crew:Int (Combat Property)
           Crew on this ship. EMPTY if this is a planet. */
        if (obj->isPlanet()) {
            return 0;
        } else {
            return makeIntegerValue(obj->getCrew());
        }
     case ivsCrewRaw:
        /* @q Crew$:Int (Combat Participant Property)
           @q Left.Crew$:Int (Combat Property), Right.Crew$:Int (Combat Property)
           Crew.
           This returns the raw, unfiltered value of the %Crew field within the VCR data structure.
           This field normally has a meaning only for ships.
           @see Crew (Combat Participant Property) */
        return makeIntegerValue(obj->getCrew());
     case ivsDamage:
        /* Damage documented in getShipProperty */
        /* @q Left.Damage:Int (Combat Property), Right.Damage:Int (Combat Property)
           Initial damage in percent. */
        return makeIntegerValue(obj->getDamage());
     case ivsId:
        /* @q Id:Int (Combat Participant Property)
           @q Left.Id:Int (Combat Property), Right.Id:Int (Combat Property)
           Id of this ship or planet. */
        return makeIntegerValue(obj->getId());
     case ivsMass:
        /* @q Mass:Int (Combat Participant Property)
           @q Left.Mass:Int (Combat Property), Right.Mass:Int (Combat Property)
           Combat mass of this unit.
           This mass includes the hull weight and optional bonuses, such as the Engine-Shield-Bonus,
           but not the ship's cargo, equipment or ammo. It therefore cannot be meaningfully compared
           to a {Mass (Ship Property)|ship's mass}. */
        return makeIntegerValue(obj->getMass());
     case ivsName:
        /* @q Name:Int (Combat Participant Property)
           @q Left.Name:Int (Combat Property), Right.Name:Int (Combat Property)
           Name of ship or planet. */
        return makeStringValue(obj->getName());
     case ivsNameFull:
        /* @q Name.Full:Int (Combat Participant Property)
           @q Left:Int (Combat Property), Right:Int (Combat Property)
           Name and type of this combat participant.
           A string of the form "name (Planet #Id)" resp. "name (Ship #Id)". */
        return makeStringValue(afl::string::Format(obj->isPlanet() ? tx("%s (Planet #%d)") : tx("%s (Ship #%d)"),
                                                   obj->getName(),
                                                   obj->getId()));
     case ivsOwnerAdj:
        /* @q Owner.Adj:Str (Combat Participant Property)
           @q Left.Owner.Adj:Str (Combat Property), Right.Owner.Adj:Str (Combat Property)
           Adjective name of this player. */
        return makeStringValue(players.getPlayerName(obj->getOwner(), Player::AdjectiveName, tx));
     case ivsOwnerId:
        /* @q Owner$:Int (Combat Participant Property)
           @q Left.Owner$:Int (Combat Property), Right.Owner$:Int (Combat Property)
           Player number. */
        return makeIntegerValue(obj->getOwner());
     case ivsOwnerShort:
        /* @q Owner:Str (Combat Participant Property)
           @q Left.Owner:Str (Combat Property), Right.Owner:Str (Combat Property)
           Short name of this player. */
        return makeStringValue(players.getPlayerName(obj->getOwner(), Player::ShortName, tx));
     case ivsShield:
        /* @q Shield:Int (Combat Participant Property)
           @q Left.Shield:Int (Combat Property), Right.Shield:Int (Combat Property)
           Initial shield level in percent. */
        return makeIntegerValue(obj->getShield());
     case ivsType:
     case ivsTypeShort:
        /* @q Type:Str (Combat Participant Property)
           @q Left.Type:Str (Combat Property), Right.Type:Str (Combat Property)
           Classification of this unit. Possible values are:
           - "Planet"
           - "Carrier"
           - "Torpedo Ship"
           - "Beam Weapons"
           - "Freighter" */
        /* @q Type.Short:Str (Combat Participant Property)
           @q Left.Type.Short:Str (Combat Property), Right.Type.Short:Str (Combat Property)
           Classification of this unit, short.
           This is the first letter of the {Type (Combat Participant Property)|Type}, see there. */
        {
            const char* kind;
            if (obj->isPlanet()) {
                kind = "Planet";
            } else if (obj->getNumBays() > 0) {
                kind = "Carrier";
            } else if (obj->getNumLaunchers() > 0 && shipList.launchers().get(obj->getTorpedoType()) != 0) {
                kind = "Torpedo Ship";
            } else if (obj->getNumBeams() > 0) {
                kind = "Beam Weapons";
            } else {
                kind = "Freighter";
            }
            if (ivs == ivsTypeShort) {
                return makeStringValue(String_t(kind, 1));
            } else {
                return makeStringValue(kind);
            }
        }
        break;
     case ivsHullName:
     case ivsHullId:
        /* @q Hull:Str (Combat Participant Property)
           @q Left.Hull:Str (Combat Property), Right.Hull:Str (Combat Property)
           Hull name.
           EMPTY if the hull cannot be determined, or this is a planet. */
        /* @q Hull$:Int (Combat Participant Property)
           @q Left.Hull$:Int (Combat Property), Right.Hull$:Int (Combat Property)
           Hull number.
           EMPTY if the hull cannot be determined, or this is a planet. */
        if (int h = obj->getGuessedHull(shipList.hulls())) {
            if (ivs == ivsHullId) {
                return makeIntegerValue(h);
            } else {
                if (const game::spec::Hull* hull = shipList.hulls().get(h)) {
                    return makeStringValue(hull->getName(shipList.componentNamer()));
                } else {
                    return 0;
                }
            }
        } else {
            return 0;
        }
     case ivsImage:
        /* @q Image:Int (Combat Participant Property)
           @q Left.Image:Int (Combat Property), Right.Image:Int (Combat Property)
           Number of ship picture.
           If the hull of the ship can be determined, your changes to the picture assignment will be honored,
           otherwise, the host-provided picture is returned.
           For planets, this property is 0. */
        return makeIntegerValue(obj->getGuessedShipPicture(shipList.hulls()));
     case ivsLevel:
        /* @q Level:Int (Combat Participant Property)
           @q Left.Level:Int (Combat Property), Right.Level:Int (Combat Property)
           Experience level for this unit.
           0 if the fight does not include experience levels (because experience is not enabled, maybe). */
        return makeIntegerValue(obj->getExperienceLevel());
     case ivsStatus:
        /* @q Status:Str (Combat Participant Property)
           @q Left.Status:Str (Combat Property), Right.Status:Str (Combat Property)
           Battle result, from the point-of-view of this unit.
           - "Survived" if the unit survived the battle and captured or destroyed its opponent.
           - "Captured" if the unit was captured by an enemy.
           - "Exploded" if the unit was destroyed.
           - "Invalid" if the battle is not playable in PCC.
           - "Stalemate" if the battle ended without a victor (PCC2 only).
           - "Disabled" if the battle ended because combatants didn't have offensive capabilities left (PCC 1.x only).
           - "Timeout" if the battle timed out (PCC 1.x only).

           Computing the value for this property may involve playing the whole VCR,
           and thus take a considerable amount of time.
           Results are cached, so you'll only have to wait once. */
        return makeVcrResult(battle, side, config, shipList);
     case ivsStatusRaw:
        /* @q Status$:Int (Combat Participant Property)
           @q Left.Status$:Int (Combat Property), Right.Status$:Int (Combat Property)
           Battle result, from the point-of-view of this unit.
           This is an integer:
           - -1: this unit was destroyed.
           - 0: this unit survived the battle.
           - other: this unit was captured in battle, the value is the new owner's player number. */
        return makeIntegerValue(battle.getOutcome(config, shipList, side));
     case ivsIsPlanet:
        /* @q IsPlanet:Bool (Combat Participant Property)
           True if this is a planet. */
        return makeBooleanValue(obj->isPlanet());

     case ivsBeamKillRate:
        /* @q Config.BeamKillRate:Int (Combat Participant Property)
           Beam kill rate for this unit (3 for Privateers, otherwise 1).
           @since PCC2 1.99.23 */
        return makeIntegerValue(obj->getBeamKillRate());

     case ivsBeamChargeRate:
        /* @q Config.BeamChargeRate:Int (Combat Participant Property)
           Beam charge rate boost. This value is generated only by NuHost.
           In particular, it is not used by PHost.
           It can be used for PHost in PCC2's simulator, where it scales up the effective BeamRechargeRate
           computed from PConfig.
           @since PCC2 1.99.23 */
        return makeIntegerValue(obj->getBeamChargeRate());

     case ivsTorpMissRate:
        /* @q Config.TorpMissRate:Int (Combat Participant Property)
           Torpedo miss rate. This value is generated only by NuHost.
           In particular, it is not used by PHost and has no relation to the TorpHitOdds PConfig option.
           @since PCC2 1.99.23 */
        return makeIntegerValue(obj->getTorpMissRate());

     case ivsTorpChargeRate:
        /* @q Config.TorpChargeRate:Int (Combat Participant Property)
           Torpedo charge rate boost. This value is generated only by NuHost.
           In particular, it is not used by PHost.
           It can be used for PHost in PCC2's simulator, where it scales up the effective TubeRechargeRate
           computed from PConfig.
           @since PCC2 1.99.23 */
        return makeIntegerValue(obj->getTorpChargeRate());

     case ivsCrewDefenseRate:
        /* @q Config.CrewDefenseRate:Int (Combat Participant Property)
           Crew defense rate. This value is generated only by NuHost.
           In particular, it is not used by PHost.
           It can be used for PHost in PCC2's simulator, where it scales down the effective CrewKillScaling
           computed from PConfig (a CrewDefenseRate of 100 reduces the CrewKillScaling to 0).
           @since PCC2 1.99.23 */
        return makeIntegerValue(obj->getCrewDefenseRate());

     case ivsRole:
        /* @q Role:Str (Combat Participant Property)
           Role.
           One of "aggressor", "opponent".
           This value is typically not known for host-generated battles.
           @since PCC2 2.0.12, PCC2 2.40.11 */
        switch (obj->getRole()) {
         case game::vcr::Object::NoRole:        break;
         case game::vcr::Object::AggressorRole: return makeStringValue("aggressor");
         case game::vcr::Object::OpponentRole:  return makeStringValue("opponent");
        }
        return 0;
    }
    return 0;
}
