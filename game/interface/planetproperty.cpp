/**
  *  \file game/interface/planetproperty.cpp
  */

#include "game/interface/planetproperty.hpp"
#include "interpreter/values.hpp"
#include "interpreter/error.hpp"
#include "interpreter/arguments.hpp"
#include "game/tables/nativeracename.hpp"
#include "game/tables/nativegovernmentname.hpp"
#include "game/tables/industrylevel.hpp"
#include "game/tables/temperaturename.hpp"
#include "game/tables/happinessname.hpp"
#include "game/cargospec.hpp"
#include "game/map/planetformula.hpp"
#include "game/tables/happinesschangename.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/turn.hpp"
#include "interpreter/indexablevalue.hpp"
#include "game/root.hpp"
#include "game/stringverifier.hpp"

using interpreter::makeIntegerValue;
using interpreter::makeBooleanValue;
using interpreter::makeStringValue;
using interpreter::makeOptionalIntegerValue;
using interpreter::makeOptionalStringValue;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;
using interpreter::Error;
using interpreter::Arguments;

namespace {
    class PlanetArrayProperty : public interpreter::IndexableValue {
     public:
        PlanetArrayProperty(const game::map::Planet& planet, afl::base::Ref<game::Game> game, game::interface::PlanetProperty property);

        // IndexableValue:
        virtual afl::data::Value* get(Arguments& args);
        virtual void set(Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        // Value:
        virtual PlanetArrayProperty* clone() const;

     private:
        const game::map::Planet& m_planet;
        afl::base::Ref<game::Game> m_game;
        const game::interface::PlanetProperty m_property;
    };
}

PlanetArrayProperty::PlanetArrayProperty(const game::map::Planet& planet, afl::base::Ref<game::Game> game, game::interface::PlanetProperty property)
    : interpreter::IndexableValue(),
      m_planet(planet),
      m_game(game),
      m_property(property)
{ }

// IndexableValue:
afl::data::Value*
PlanetArrayProperty::get(Arguments& args)
{
    // ex PlanetArrayProperty::get
    switch (m_property) {
     case game::interface::ippScore: {
        // FIXME: /* Documented in shipproperty.cc */
        int32_t id;
        game::UnitScoreList::Index_t index;
        int16_t value, turn;
        args.checkArgumentCount(1);
        if (checkIntegerArg(id, args.getNext(), 0, 0x7FFF)
            && m_game->planetScores().lookup(id, index)
            && m_planet.unitScores().get(index, value, turn))
        {
            return makeIntegerValue(value);
        } else {
            return 0;
        }
     }

     default:
        return 0;
    }
}

void
PlanetArrayProperty::set(Arguments& /*args*/, afl::data::Value* /*value*/)
{
    // ex PlanetArrayProperty::set
    throw Error::notAssignable();
}

// CallableValue:
int32_t
PlanetArrayProperty::getDimension(int32_t /*which*/) const
{
    // ex PlanetArrayProperty::getDimension
    return 0;
}

interpreter::Context*
PlanetArrayProperty::makeFirstContext()
{
    // ex PlanetArrayProperty::makeFirstContext
    throw Error::typeError(Error::ExpectIterable);
}

// BaseValue:
String_t
PlanetArrayProperty::toString(bool /*readable*/) const
{
    // ex PlanetArrayProperty::toString
    return "#<array>";
}

void
PlanetArrayProperty::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex PlanetArrayProperty::store
    throw Error::notSerializable();
}

// Value:
PlanetArrayProperty*
PlanetArrayProperty::clone() const
{
    // ex PlanetArrayProperty::clone
    return new PlanetArrayProperty(m_planet, m_game, m_property);
}



afl::data::Value*
game::interface::getPlanetProperty(const game::map::Planet& pl, PlanetProperty ipp,
                                   afl::string::Translator& tx,
                                   const game::HostVersion& host,
                                   const game::config::HostConfiguration& config,
                                   InterpreterInterface& iface,
                                   afl::base::Ref<Game> game)
{
    // ex int/if/planetif.h:getPlanetProperty
    // FIXME: check lifetime issues. If this gives out an array property, that one must keep config/shipList alive.
    int32_t n;
    switch (ipp) {
     case ippBaseDefenseWanted:
        /* @q Defense.Base.Want:Int (Planet Property)
           Auto-build goal for starbase defense.
           @assignable */
        return makeIntegerValue(pl.getAutobuildGoal(BaseDefenseBuilding));
     case ippBaseFlag:
        /* @q Base.YesNo:Bool (Planet Property)
           True if this planet has a base. */
        return makeBooleanValue(pl.hasBase());
     case ippBaseBuildFlag:
        /* @q Base.Building:Bool (Planet Property)
           True if this planet is building a base. */
        if (pl.hasFullPlanetData()) {
            return makeBooleanValue(pl.isBuildingBase());
        } else {
            return 0;
        }
     case ippBaseStr:
        /* @q Base:Str (Planet Property)
           Starbase status, human-readable.
           One of
           - "present"
           - "being built"
           - "-" */
        if (pl.hasBase()) {
            return makeStringValue("present");
        } else if (pl.isBuildingBase()) {
            return makeStringValue("being built");
        } else {
            return makeStringValue("-");
        }
     case ippColonistChange:
        /* @q Colonists.Change$:Int (Planet Property)
           Colonist happiness change, numeric value. */
        return makeOptionalIntegerValue(getColonistChange(pl, config, host));
     case ippColonistChangeStr:
        /* @q Colonists.Change:Str (Planet Property)
           Colonist happiness change, text. */
        return makeOptionalStringValue(game::tables::HappinessChangeName(tx)(getColonistChange(pl, config, host)));
     case ippColonistHappy:
        /* @q Colonists.Happy$:Int (Planet Property)
           Colonist happiness, numeric value. */
        return makeOptionalIntegerValue(pl.getColonistHappiness());
     case ippColonistHappyStr:
        /* @q Colonists.Happy:Str (Planet Property)
           Colonist happiness, text. */
        return makeOptionalStringValue(game::tables::HappinessName(tx)(pl.getColonistHappiness()));
     case ippColonistSupported:
        /* @q Colonists.Supported:Int (Planet Property)
           Maximum colonist clans supported by planet's climate.
           @since PCC 1.1.16, PCC2 1.99.8 */
        return makeOptionalIntegerValue(getMaxSupportedColonists(pl, config, host));
     case ippColonistTax:
        /* @q Colonists.Tax:Int (Planet Property)
           Colonist tax.
           @assignable
           @see SetColonistTax (Planet Command) */
        return makeOptionalIntegerValue(pl.getColonistTax());
     case ippColonistTaxIncome: {
        /* @q Colonists.Tax.Income:Int (Planet Property)
           Tax income from colonists, megacredits.
           @since PCC2 1.99.15 */
        int tax;
        if (pl.getColonistTax().get(tax)) {
            return makeOptionalIntegerValue(getColonistDue(pl, config, host, tax));
        } else {
            return 0;
        }
     }
     case ippColonists:
        /* @q Colonists:Int (Planet Property)
           Colonist population, number of clans. */
        return makeOptionalIntegerValue(pl.getCargo(Element::Colonists));
     case ippDefense:
        /* @q Defense:Int (Planet Property)
           Number of planetary defense posts. */
        return makeOptionalIntegerValue(pl.getNumBuildings(DefenseBuilding));
     case ippDefenseMax:
        /* @q Defense.Max:Int (Planet Property)
           Maximum number of planetary defense posts. */
        return makeOptionalIntegerValue(getMaxBuildings(pl, DefenseBuilding, config));
     case ippDefenseWanted:
        /* @q Defense.Want:Int (Planet Property)
           Auto-build goal for defense posts.
           @assignable */
        return makeIntegerValue(pl.getAutobuildGoal(DefenseBuilding));
     case ippDensityD:
        /* @q Density.D:Int (Planet Property)
           Density of Duranium in planet core. */
        return makeOptionalIntegerValue(pl.getOreDensity(Element::Duranium));
     case ippDensityM:
        /* @q Density.M:Int (Planet Property)
           Density of Molybdenum in planet core. */
        return makeOptionalIntegerValue(pl.getOreDensity(Element::Molybdenum));
     case ippDensityN:
        /* @q Density.N:Int (Planet Property)
           Density of Neutronium in planet core. */
        return makeOptionalIntegerValue(pl.getOreDensity(Element::Neutronium));
     case ippDensityT:
        /* @q Density.T:Int (Planet Property)
           Density of Tritanium in planet core. */
        return makeOptionalIntegerValue(pl.getOreDensity(Element::Tritanium));
     case ippFCode:
        /* @q FCode:Str (Planet Property)
           Friendly code.
           @assignable
           @see SetFCode (Planet Command) */
        return makeOptionalStringValue(pl.getFriendlyCode());
     case ippFactories:
        /* @q Factories:Int (Planet Property)
           Number of factories on planet. */
        return makeOptionalIntegerValue(pl.getNumBuildings(FactoryBuilding));
     case ippFactoriesMax:
        /* @q Factories.Max:Int (Planet Property)
           Maximum number of factories on planet. */
        return makeOptionalIntegerValue(getMaxBuildings(pl, FactoryBuilding, config));
     case ippFactoriesWanted:
        /* @q Factories.Want:Int (Planet Property)
           Auto-build goal for factories.
           @assignable */
        return makeIntegerValue(pl.getAutobuildGoal(FactoryBuilding));
     case ippGroundD:
        /* @q Ground.D:Int (Planet Property)
           Amount of Duranium in ground, kilotons. */
        return makeOptionalIntegerValue(pl.getOreGround(Element::Duranium));
     case ippGroundM:
        /* @q Ground.M:Int (Planet Property)
           Amount of Molybdenum in ground, kilotons. */
        return makeOptionalIntegerValue(pl.getOreGround(Element::Molybdenum));
     case ippGroundN:
        /* @q Ground.N:Int (Planet Property)
           Amount of Neutronium in ground, kilotons. */
        return makeOptionalIntegerValue(pl.getOreGround(Element::Neutronium));
     case ippGroundT:
        /* @q Ground.T:Int (Planet Property)
           Amount of Tritanium in ground, kilotons. */
        return makeOptionalIntegerValue(pl.getOreGround(Element::Tritanium));
     case ippId:
        /* @q Id:Int (Planet Property)
           Planet Id. */
        return makeIntegerValue(pl.getId());
     case ippIndustry:
        /* @q Industry:Str (Planet Property)
           Planetary industry level, human-readable.
           @see Industry$ (Planet Property) */
        return makeOptionalStringValue(game::tables::IndustryLevel(tx)(pl.getIndustryLevel(host)));
     case ippIndustryCode:
        /* @q Industry$:Int (Planet Property)
           Planetary industry level code.
           <table>
            <tr><th width="3" align="left">Ind$</th><th width="7" align="left">Ind</th></tr>
            <tr><td>0</td><td>Minimal</td></tr>
            <tr><td>1</td><td>Light</td></tr>
            <tr><td>2</td><td>Moderate</td></tr>
            <tr><td>3</td><td>Substantial</td></tr>
            <tr><td>4</td><td>Heavy</td></tr>
           </table> */
        return makeOptionalIntegerValue(pl.getIndustryLevel(host));
     case ippLevel: {
        /* @q Level:Int (Planet Property)
           Planet's experience level.
           If the experience system is not enabled, or the level is not known, yields EMPTY. */
        UnitScoreList::Index_t index;
        int16_t value, turn;
        if (game->planetScores().lookup(ScoreId_ExpLevel, index) && pl.unitScores().get(index, value, turn)) {
            return makeIntegerValue(value);
        } else {
            return 0;
        }
     }
     case ippLocX: {
        /* @q Loc.X:Int (Planet Property)
           Planet X location. */
        game::map::Point pos;
        if (pl.getPosition(pos)) {
            return makeIntegerValue(pos.getX());
        } else {
            return 0;
        }
     }
     case ippLocY: {
        /* @q Loc.Y:Int (Planet Property)
           Planet Y location. */
        game::map::Point pos;
        if (pl.getPosition(pos)) {
            return makeIntegerValue(pos.getY());
        } else {
            return 0;
        }
     }
     case ippMarked:
        /* @q Marked:Bool (Planet Property)
           True if planet is marked. */
        return makeBooleanValue(pl.isMarked());
     case ippMinedD:
        /* @q Mined.D:Int (Planet Property)
           Mined Duranium, in kilotons. */
        return makeOptionalIntegerValue(pl.getCargo(Element::Duranium));
     case ippMinedM:
        /* @q Mined.M:Int (Planet Property)
           Mined Molybdenum, in kilotons. */
        return makeOptionalIntegerValue(pl.getCargo(Element::Molybdenum));
     case ippMinedN:
        /* @q Mined.N:Int (Planet Property)
           Mined Neutronium, in kilotons. */
        return makeOptionalIntegerValue(pl.getCargo(Element::Neutronium));
     case ippMinedStr:
        /* @q Mined.Str:Cargo (Planet Property)
           Mined minerals, as a string. */
     {
         CargoSpec cs;
         bool haveAny = false;
         int32_t n;
         if (pl.getCargo(Element::Neutronium).get(n)) {
             cs.set(CargoSpec::Neutronium, n);
             haveAny = true;
         }
         if (pl.getCargo(Element::Tritanium).get(n)) {
             cs.set(CargoSpec::Tritanium, n);
             haveAny = true;
         }
         if (pl.getCargo(Element::Duranium).get(n)) {
             cs.set(CargoSpec::Duranium, n);
             haveAny = true;
         }
         if (pl.getCargo(Element::Molybdenum).get(n)) {
             cs.set(CargoSpec::Molybdenum, n);
             haveAny = true;
         }
         if (haveAny) {
             return makeStringValue(cs.toCargoSpecString());
         } else {
             return 0;
         }
     }
     case ippMinedT:
        /* @q Mined.T:Int (Planet Property)
           Mined Tritanium, in kilotons. */
        return makeOptionalIntegerValue(pl.getCargo(Element::Tritanium));
     case ippMines:
        /* @q Mines:Int (Planet Property)
           Number of mineral mines. */
        return makeOptionalIntegerValue(pl.getNumBuildings(MineBuilding));
     case ippMinesMax:
        /* @q Mines.Max:Int (Planet Property)
           Maximum number of mineral mines. */
        return makeOptionalIntegerValue(getMaxBuildings(pl, MineBuilding, config));
     case ippMinesWanted:
        /* @q Mines.Want:Int (Planet Property)
           Auto-build goal for mineral mines.
           @assignable */
        return makeIntegerValue(pl.getAutobuildGoal(MineBuilding));
     case ippMoney:
        /* @q Money:Int (Planet Property)
           Money (megacredits) on planet. */
        return makeOptionalIntegerValue(pl.getCargo(Element::Money));
     case ippName:
        /* @q Name:Str (Planet Property)
           Name of planet. */
        return makeStringValue(pl.getName(pl.PlainName, tx, iface));
     case ippNativeChange:
        /* @q Natives.Change$:Int (Planet Property)
           Native happiness change, numeric value. */
        return makeOptionalIntegerValue(getNativeChange(pl, host));
     case ippNativeChangeStr:
        /* @q Natives.Change:Str (Planet Property)
           Native happiness change, text. */
        return makeOptionalStringValue(game::tables::HappinessChangeName(tx)(getNativeChange(pl, host)));
     case ippNativeGov:
        /* @q Natives.Gov:Str (Planet Property)
           Native government name. */
        if (pl.getNatives().get(n) && n > 0) {
            return makeOptionalStringValue(game::tables::NativeGovernmentName(tx)(pl.getNativeGovernment()));
        } else {
            return 0;
        }
     case ippNativeGovCode:
        /* @q Natives.Gov$:Int (Planet Property)
           Native government code. */
        if (pl.getNatives().get(n) && n > 0) {
            return makeOptionalIntegerValue(pl.getNativeGovernment());
        } else {
            return 0;
        }
     case ippNativeHappy:
        /* @q Natives.Happy$:Int (Planet Property)
           Native happiness, numeric value. */
        if (pl.getNatives().get(n) && n > 0) {
            return makeOptionalIntegerValue(pl.getNativeHappiness());
        } else {
            return 0;
        }
     case ippNativeHappyStr:
        /* @q Natives.Happy:Str (Planet Property)
           Native happiness, text. */
        if (pl.getNatives().get(n) && n > 0) {
            return makeOptionalStringValue(game::tables::HappinessName(tx)(pl.getNativeHappiness()));
        } else {
            return 0;
        }
     case ippNativeRace:
        /* @q Natives.Race:Str (Planet Property)
           Native race, name. */
        if (pl.getNatives().get(n) && n > 0) {
            return makeOptionalStringValue(game::tables::NativeRaceName(tx)(pl.getNativeRace()));
        } else {
            return 0;
        }
     case ippNativeRaceCode:
        /* @q Natives.Race$:Int (Planet Property)
           Native race, numeric value. */
        if (pl.getNatives().isValid()) {
            return makeOptionalIntegerValue(pl.getNativeRace());
        } else {
            return 0;
        }
     case ippNativeTax:
        /* @q Natives.Tax:Int (Planet Property)
           Native tax level.
           @assignable
           @see SetNativeTax (Planet Command) */
        if (pl.getNatives().get(n) && n > 0) {
            return makeOptionalIntegerValue(pl.getNativeTax());
        } else {
            return 0;
        }
     case ippNativeTaxBase:
        /* @q Natives.Tax.Base:Int (Planet Property)
           Natives base tax level.
           This is the tax level at which happiness does not change.
           @since PCC2 1.99.15 */
        return makeOptionalIntegerValue(getNativeBaseTax(pl, config, host, 0));
     case ippNativeTaxMax:
        /* @q Natives.Tax.Max:Int (Planet Property)
           Natives maximum tax level.
           This is the tax level at which happiness changes by -30.
           @since PCC2 1.99.15 */
        return makeOptionalIntegerValue(getNativeBaseTax(pl, config, host, -30));
     case ippNativeTaxIncome:
        /* @q Natives.Tax.Income:Int (Planet Property)
           Tax income from natives, megacredits.
           @since PCC2 1.99.15 */
        if (pl.getNativeTax().get(n)) {
            // FIXME: PCC2 returns null when there are no natives, this return 0.
            return makeOptionalIntegerValue(getNativeDue(pl, config, host, n));
        } else {
            return 0;
        }
     case ippNatives:
        /* @q Natives:Int (Planet Property)
           Native population size, clans. */
        return makeOptionalIntegerValue(pl.getNatives());
     case ippOrbitingEnemies: {
        /* @q Orbit.Enemy:Int (Planet Property)
           Number of enemy (=not own) ships in orbit of this planet. */
        game::map::Point pt;
        if (pl.getPosition(pt)) {
            return makeIntegerValue(game::map::AnyShipType(game->currentTurn().universe()).countObjectsAt(pt, PlayerSet_t::allUpTo(MAX_PLAYERS) - game->getViewpointPlayer()));
        } else {
            return 0;
        }
     }
     case ippOrbitingOwn: {
        /* @q Orbit.Own:Int (Planet Property)
           Number of own ships in orbit of this planet. */
        game::map::Point pt;
        if (pl.getPosition(pt)) {
            return makeIntegerValue(game::map::AnyShipType(game->currentTurn().universe()).countObjectsAt(pt, PlayerSet_t(game->getViewpointPlayer())));
        } else {
            return 0;
        }
     }
     case ippOrbitingShips: {
        /* @q Orbit:Int (Planet Property)
           Total number of ships in orbit of this planet. */
        game::map::Point pt;
        if (pl.getPosition(pt)) {
            return makeIntegerValue(game::map::AnyShipType(game->currentTurn().universe()).countObjectsAt(pt, PlayerSet_t::allUpTo(MAX_PLAYERS)));
        } else {
            return 0;
        }
     }
     case ippPlayed:
        /* @q Played:Bool (Planet Property)
           True if this planet is played.
           @since PCC 1.1.19 */
        return makeBooleanValue(pl.isPlayable(pl.Playable));
     case ippSupplies:
        /* @q Supplies:Int (Planet Property)
           Supplies on this planet. */
        return makeOptionalIntegerValue(pl.getCargo(Element::Supplies));
     case ippTask:
        /* @q Task:Bool (Planet Property)
           True if this planet has an Auto Task. */
        return makeBooleanValue(iface.hasTask(iface.Planet, pl.getId()));
     case ippTaskBase:
        /* @q Task.Base:Bool (Planet Property)
           True if this planet's starbase has an Auto Task. */
        return makeBooleanValue(iface.hasTask(iface.Base, pl.getId()));
     case ippTemp:
        /* @q Temp$:Int (Planet Property)
           Temperature, numeric value. */
        return makeOptionalIntegerValue(pl.getTemperature());
     case ippTempStr:
        /* @q Temp:Int (Planet Property)
           Temperature class, human-readable. */
        return makeOptionalStringValue(game::tables::TemperatureName(tx)(pl.getTemperature()));
     case ippTypeChar:
        /* @q Type.Short:Str (Planet Property)
           Always "P" for planets.
           @see Type.Short (Ship Property), Type.Short (Combat Participant Property)
           @since PCC2 1.99.21, PCC 1.1.20 */
        return makeStringValue("P");
     case ippTypeStr:
        /* @q Type:Str (Planet Property)
           Always "Planet" for planets.
           @see Type (Ship Property), Type (Combat Participant Property)
           @since PCC2 1.99.21, PCC 1.1.20 */
        return makeStringValue("Planet");

     case ippScore:
        return new PlanetArrayProperty(pl, game, ipp);
    }
    return 0;
}

void
game::interface::setPlanetProperty(game::map::Planet& pl, PlanetProperty ipp, afl::data::Value* value, Root& root)
{
    // ex int/if/planetif.h:setPlanetProperty
    // We cannot assign to anything other than auto-build goals on non-played planets
    if (!pl.isPlayable(pl.Playable) && (ipp != ippMinesWanted && ipp != ippFactoriesWanted && ipp != ippDefenseWanted && ipp != ippBaseDefenseWanted)) {
        throw Error::notAssignable();
    }

    int32_t iv;
    switch (ipp) {
     case ippMinesWanted:
        if (checkIntegerArg(iv, value, 0, 1000)) {
            pl.setAutobuildGoal(MineBuilding, iv);
        }
        break;
     case ippFactoriesWanted:
        if (checkIntegerArg(iv, value, 0, 1000)) {
            pl.setAutobuildGoal(FactoryBuilding, iv);
        }
        break;
     case ippDefenseWanted:
        if (checkIntegerArg(iv, value, 0, 1000)) {
            pl.setAutobuildGoal(DefenseBuilding, iv);
        }
        break;
     case ippBaseDefenseWanted:
        if (checkIntegerArg(iv, value, 0, 1000)) {
            pl.setAutobuildGoal(BaseDefenseBuilding, iv);
        }
        break;
     case ippColonistTax:
        if (checkIntegerArg(iv, value, 0, 100)) {
            pl.setColonistTax(iv);
        }
        break;
     case ippFCode:
     {
         String_t sv;
         if (checkStringArg(sv, value)) {
             if (!root.stringVerifier().isValidString(StringVerifier::FriendlyCode, sv)) {
                 throw Error::rangeError();
             }
             pl.setFriendlyCode(sv);
         }
         break;
     }
     case ippNativeTax:
        if (checkIntegerArg(iv, value, 0, 100)) {
            if (!pl.getNativeRace().isValid()) {
                throw Error::notAssignable();
            } else {
                pl.setNativeTax(iv);
            }
        }
        break;
     default:
        throw Error::notAssignable();
    }
}
