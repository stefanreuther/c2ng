/**
  *  \file game/interface/baseproperty.cpp
  *  \brief Enum game::interface::BaseProperty
  */

#include "game/interface/baseproperty.hpp"
#include "afl/string/format.hpp"
#include "game/map/planetformula.hpp"
#include "game/map/ship.hpp"
#include "game/tables/basemissionname.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/values.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using interpreter::Arguments;
using interpreter::Error;
using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using interpreter::makeOptionalIntegerValue;
using interpreter::makeOptionalStringValue;
using interpreter::makeStringValue;

namespace {
    /*
     *  Implementation of an Array property
     */
    enum BaseArrayPropertyIdentifier {
        bapEngineStorage,
        bapHullStorage,
        bapBeamStorage,
        bapLauncherStorage,
        bapAmmoStorage
    };

    class BaseArrayProperty : public interpreter::IndexableValue {
     public:
        BaseArrayProperty(const game::map::Planet& planet,
                          const Ref<const game::Root>& root,
                          const Ref<const game::spec::ShipList>& shipList,
                          BaseArrayPropertyIdentifier property);

        // IndexableValue:
        virtual afl::data::Value* get(Arguments& args);
        virtual void set(Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual size_t getDimension(size_t which) const;
        virtual interpreter::Context* makeFirstContext();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        // Value:
        virtual BaseArrayProperty* clone() const;

     private:
        typedef game::IntegerProperty_t Function_t(const game::map::Planet&, const game::spec::ShipList&, int);
        afl::data::Value* performArrayReference(Function_t func, int limit, int32_t arg, bool hull);

        const game::map::Planet& m_planet;
        Ref<const game::Root> m_root;
        Ref<const game::spec::ShipList> m_shipList;
        const BaseArrayPropertyIdentifier m_property;
    };

    game::IntegerProperty_t getBaseEngineStore(const game::map::Planet& p, const game::spec::ShipList& /*shipList*/, int n)
    {
        return p.getBaseStorage(game::EngineTech, n);
    }

    game::IntegerProperty_t getBaseHullStoreSlot(const game::map::Planet& p, const game::spec::ShipList& /*shipList*/, int n)
    {
        return p.getBaseStorage(game::HullTech, n);
    }

    game::IntegerProperty_t getBaseBeamStore(const game::map::Planet& p, const game::spec::ShipList& /*shipList*/, int n)
    {
        return p.getBaseStorage(game::BeamTech, n);
    }

    game::IntegerProperty_t getBaseLauncherStore(const game::map::Planet& p, const game::spec::ShipList& /*shipList*/, int n)
    {
        return p.getBaseStorage(game::TorpedoTech, n);
    }

    game::IntegerProperty_t getBaseAmmoStore(const game::map::Planet& p, const game::spec::ShipList& shipList, int n)
    {
        // ex planint.pas:fetch_storage (part), GPlanet::getBaseAmmoStore
        int numLaunchers = shipList.launchers().size();
        if (n < 0 || n > numLaunchers+1) {
            return afl::base::Nothing;
        } else if (n > numLaunchers) {
            return p.getCargo(game::Element::Fighters);
        } else {
            return p.getCargo(game::Element::fromTorpedoType(n));
        }
    }

    BaseArrayProperty* makeArrayProperty(const game::map::Planet& pl,
                                         const Ref<const game::Root>& root,
                                         const Ptr<const game::spec::ShipList>& shipList,
                                         BaseArrayPropertyIdentifier id)
    {
        if (shipList.get() != 0) {
            return new BaseArrayProperty(pl, root, *shipList, id);
        } else {
            return 0;
        }
    }
}

BaseArrayProperty::BaseArrayProperty(const game::map::Planet& planet,
                                     const Ref<const game::Root>& root,
                                     const Ref<const game::spec::ShipList>& shipList,
                                     BaseArrayPropertyIdentifier property)
    : m_planet(planet),
      m_root(root),
      m_shipList(shipList),
      m_property(property)
{ }

// IndexableValue:
afl::data::Value*
BaseArrayProperty::get(Arguments& args)
{
    // ex IntBaseArrayProperty::get
    int32_t arg;
    args.checkArgumentCount(1);
    if (!interpreter::checkIntegerArg(arg, args.getNext())) {
        return 0;
    }

    int owner;
    switch (m_property) {
     case bapEngineStorage:
        return performArrayReference(getBaseEngineStore, m_shipList->engines().size(), arg, false);
     case bapHullStorage:
        if (m_planet.getOwner().get(owner)) {
            return performArrayReference(getBaseHullStoreSlot, m_shipList->hullAssignments().getMaxIndex(m_root->hostConfiguration(), owner), arg, true);
        } else {
            return 0;
        }
     case bapBeamStorage:
        return performArrayReference(getBaseBeamStore, m_shipList->beams().size(), arg, false);
     case bapLauncherStorage:
        return performArrayReference(getBaseLauncherStore, m_shipList->launchers().size(), arg, false);
     case bapAmmoStorage:
        return performArrayReference(getBaseAmmoStore, m_shipList->launchers().size()+1, arg, false);
    }
    return 0;
}

void
BaseArrayProperty::set(Arguments& args, const afl::data::Value* value)
{
    // ex IntBaseArrayProperty::set
    rejectSet(args, value);
}


// CallableValue:
size_t
BaseArrayProperty::getDimension(size_t which) const
{
    // ex IntBaseArrayProperty::getDimension
    if (which == 0) {
        // Property has 1 dimension
        return 1;
    } else {
        // Get that dimension
        // Note: must add "+1" because nonexistant 0th element is counted as well!
        // "dim a(10)" produces an array that has "dim(a) = 10", and indexes 0..9.
        switch (m_property) {
         case bapEngineStorage:
            return m_shipList->engines().size() + 1;
         case bapHullStorage:
            return m_shipList->hulls().size() + 1;
         case bapBeamStorage:
            return m_shipList->beams().size() + 1;
         case bapLauncherStorage:
            return m_shipList->launchers().size() + 1;
         case bapAmmoStorage:
            return m_shipList->launchers().size() + 1 + 1;
        }
        return 0;
    }
}

interpreter::Context*
BaseArrayProperty::makeFirstContext()
{
    // ex IntBaseArrayProperty::makeFirstContext
    return rejectFirstContext();
}

// BaseValue:
String_t
BaseArrayProperty::toString(bool /*readable*/) const
{
    // ex IntBaseArrayProperty::toString
    return "#<array>";
}

void
BaseArrayProperty::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    // ex IntBaseArrayProperty::store
    rejectStore(out, aux, ctx);
}

// Value:
BaseArrayProperty*
BaseArrayProperty::clone() const
{
    return new BaseArrayProperty(m_planet, m_root, m_shipList, m_property);
}

/** Perform array reference.
    This implements the special abilities of starbase array properties:
    - index 0 counts all items
    - index 1..limit returns that item's count
    - other values yield empty (not an error!)
    \param func Function to access the property in raw form
    \param limit Maximum index
    \param arg User-specified argument
    \param hull True for hulls
    \return resulting value */
afl::data::Value*
BaseArrayProperty::performArrayReference(Function_t func, int limit, int32_t arg, bool hull)
{
    // ex IntBaseArrayProperty::performArrayReference
    // ex planint.pas:ArrayRef
    if (arg == 0) {
        int32_t sum = 0;
        for (int i = 1; i <= limit; ++i) {
            int value;
            if (func(m_planet, *m_shipList, i).get(value)) {
                sum += value;
            } else {
                return 0;
            }
        }
        return makeIntegerValue(sum);
    } else if (hull) {
        int owner;
        if (m_planet.getOwner().get(owner)) {
            if (int slot = m_shipList->hullAssignments().getIndexFromHull(m_root->hostConfiguration(), owner, arg)) {
                return makeOptionalIntegerValue(func(m_planet, *m_shipList, slot));
            } else if (m_shipList->hulls().get(arg) == 0) {
                return 0;
            } else {
                return makeIntegerValue(0);
            }
        } else {
            return 0;
        }
    } else {
        return makeOptionalIntegerValue(func(m_planet, *m_shipList, arg));
    }
}

/*
 *  Public Entry Points
 */

afl::data::Value*
game::interface::getBaseProperty(const game::map::Planet& pl, BaseProperty ibp,
                                 afl::string::Translator& tx,
                                 const afl::base::Ref<const Root>& root,
                                 const afl::base::Ptr<const game::spec::ShipList>& shipList,
                                 const afl::base::Ref<const Turn>& turn)
{
    // ex int/if/baseif.h:getBaseProperty
    if (!pl.isPlayable(game::map::Object::ReadOnly) || !pl.hasBase()) {
        return 0;
    }

    switch (ibp) {
     case ibpBaseDamage:
        /* @q Damage:Int (Planet Property)
           Starbase damage. EMPTY if no base. */
        return makeOptionalIntegerValue(pl.getBaseDamage());
     case ibpBaseDefense:
        /* @q Defense.Base:Int (Planet Property)
           Starbase defense. EMPTY if no base. */
        return makeOptionalIntegerValue(pl.getNumBuildings(BaseDefenseBuilding));
     case ibpBaseDefenseMax:
        /* @q Defense.Base.Max:Int (Planet Property)
           Maximum starbase defense allowed. EMPTY if no base.
           @since PCC 1.1.16, PCC2 1.99.8 */
        return makeOptionalIntegerValue(game::map::getMaxBuildings(pl, BaseDefenseBuilding, root->hostConfiguration()));
     case ibpBaseFighters:
        /* @q Fighters:Int (Planet Property)
           Number of fighters on starbase. EMPTY if no base. */
        return makeOptionalIntegerValue(pl.getCargo(Element::Fighters));
     case ibpBaseFightersMax: {
        /* @q Fighters.Max:Int (Planet Property)
           Maximum number of fighters allowed on starbase. EMPTY if no base.
           @since PCC 1.1.16, PCC2 1.99.8 */
        int owner;
        if (pl.getOwner().get(owner)) {
            return makeIntegerValue(root->hostConfiguration()[game::config::HostConfiguration::MaximumFightersOnBase](owner));
        } else {
            return 0;
        }
     }
     case ibpBeamTech:
        /* @q Tech.Beam:Int (Planet Property)
           Beam tech level on starbase. EMPTY if no base. */
        return makeOptionalIntegerValue(pl.getBaseTechLevel(BeamTech));
     case ibpBuildBeam:
        /* @q Build.Beam$:Int (Planet Property)
           Beam type for ship to build on starbase. EMPTY if no base. */
        return makeIntegerValue(pl.getBaseBuildOrder().getBeamType());
     case ibpBuildBeamCount:
        /* @q Build.Beam.Count:Int (Planet Property)
           Number of beams for ship to build on starbase. EMPTY if no base. */
        return makeIntegerValue(pl.getBaseBuildOrder().getNumBeams());
     case ibpBuildEngine:
        /* @q Build.Engine$:Int (Planet Property)
           Number of engines for ship to build on starbase. EMPTY if no base. */
        return makeIntegerValue(pl.getBaseBuildOrder().getEngineType());
     case ibpBuildFlag:
        /* @q Build.YesNo:Bool (Planet Property)
           Ship build flag. True if this base is building a ship. EMPTY if no base. */
        return makeBooleanValue(pl.getBaseBuildOrder().getHullIndex() != 0);
     case ibpBuildHull:
        /* @q Build.Hull$:Int (Planet Property)
           Type of ship (hull Id) to build on starbase. EMPTY if no base, or no ship being built. */
        if (shipList.get() != 0) {
            return makeOptionalIntegerValue(pl.getBaseBuildHull(root->hostConfiguration(), shipList->hullAssignments()));
        } else {
            return 0;
        }
     case ibpBuildHullName:
        /* @q Build:Str (Planet Property)
           Type of ship (hull name) to build on starbase. EMPTY if no base, or no ship being built.
           @see Name (Hull Property) */
        if (shipList.get() != 0) {
            int nr;
            if (pl.getBaseBuildHull(root->hostConfiguration(), shipList->hullAssignments()).get(nr)) {
                if (const game::spec::Hull* hull = shipList->hulls().get(nr)) {
                    return makeStringValue(hull->getName(shipList->componentNamer()));
                }
            }
        }
        return 0;
     case ibpBuildHullShort:
        /* @q Build.Short:Str (Planet Property)
           Type of ship (short hull name) to build on starbase. EMPTY if no base, or no ship being built.
           @see Name.Short (Hull Property) */
        if (shipList.get() != 0) {
            int nr;
            if (pl.getBaseBuildHull(root->hostConfiguration(), shipList->hullAssignments()).get(nr)) {
                if (const game::spec::Hull* hull = shipList->hulls().get(nr)) {
                    return makeStringValue(hull->getShortName(shipList->componentNamer()));
                }
            }
        }
        return 0;
     case ibpBuildQueuePos:
        /* @q Build.QPos:Int (Planet Property)
           Position of starbase in build queue. EMPTY if no base, or position not known. */
        return makeOptionalIntegerValue(pl.getBaseQueuePosition());
     case ibpBuildTorp:
        /* @q Build.Torp$:Int (Planet Property)
           Torpedo type for ship to build on starbase. EMPTY if no base. */
        return makeIntegerValue(pl.getBaseBuildOrder().getTorpedoType());
     case ibpBuildTorpCount:
        /* @q Build.Torp.Count:Int (Planet Property)
           Number of torpedo tubes for ship to build on starbase. EMPTY if no base. */
        return makeIntegerValue(pl.getBaseBuildOrder().getNumLaunchers());
     case ibpEngineTech:
        /* @q Tech.Engine:Int (Planet Property)
           Engine tech level on starbase. EMPTY if no base. */
        return makeOptionalIntegerValue(pl.getBaseTechLevel(EngineTech));
     case ibpHullTech:
        /* @q Tech.Hull:Int (Planet Property)
           Hull tech level on starbase. EMPTY if no base. */
        return makeOptionalIntegerValue(pl.getBaseTechLevel(HullTech));
     case ibpMission:
        /* @q Mission$:Int (Planet Property)
           Starbase mission number. EMPTY if no base.
           @assignable */
        return makeOptionalIntegerValue(pl.getBaseMission());
     case ibpMissionName:
        /* @q Mission:Str (Planet Property)
           Starbase mission. EMPTY if no base. */
        return makeOptionalStringValue(game::tables::BaseMissionName(tx)(pl.getBaseMission()));
     case ibpShipyardAction: {
        /* @q Shipyard.Action:Str (Planet Property)
           Shipyard action on base. One of <tt>"Fix"</tt> or <tt>"Recycle"</tt>.
           EMPTY if no base, or no shipyard order set. */
        int nr;
        if (pl.getBaseShipyardAction().get(nr)) {
            if (nr == FixShipyardAction) {
                return makeStringValue("Fix");
            }
            if (nr == RecycleShipyardAction) {
                return makeStringValue("Recycle");
            }
        }
        return 0;
     }
     case ibpShipyardId:
        /* @q Shipyard.Id:Int (Planet Property)
           Id of ship being worked on by starbase. EMPTY if no base. */
        return makeOptionalIntegerValue(pl.getBaseShipyardId());
     case ibpShipyardName: {
        /* @q Shipyard.Name:Str (Planet Property)
           Name of ship being worked on by starbase. EMPTY if no base, or no shipyard order set. */
        int id;
        if (pl.getBaseShipyardId().get(id)) {
            if (const game::map::Ship* ship = turn->universe().ships().get(id)) {
                return makeStringValue(ship->getName());
            }
        }
        return 0;
     }
     case ibpShipyardStr: {
        /* @q Shipyard:Str (Planet Property)
           Shipyard order in human-readable form.
           A combination of {Shipyard.Action} and {Shipyard.Name}.
           EMPTY if no base, or no shipyard order set. */
        int id, nr;
        if (pl.getBaseShipyardId().get(id) && pl.getBaseShipyardAction().get(nr)) {
            if (const game::map::Ship* ship = turn->universe().ships().get(id)) {
                if (nr == FixShipyardAction) {
                    return makeStringValue(afl::string::Format("Fix %s", ship->getName()));
                }
                if (nr == RecycleShipyardAction) {
                    return makeStringValue(afl::string::Format("Recycle %s", ship->getName()));
                }
            }
        }
        return 0;
     }
     case ibpTorpedoTech:
        /* @q Tech.Torpedo:Int (Planet Property)
           Torpedo tech level on starbase. EMPTY if no base. */
        return makeOptionalIntegerValue(pl.getBaseTechLevel(TorpedoTech));

     case ibpEngineStorage:
        /* @q Storage.Engines:Int() (Planet Property)
           Number of engines in starbase storage.
           Index can be 0 (=total number of engines) or an engine type (=number of engines of that type).
           EMPTY if no base. */
        return makeArrayProperty(pl, root, shipList, bapEngineStorage);

     case ibpHullStorage:
        /* @q Storage.Hulls:Int() (Planet Property)
           Number of engines in starbase storage.
           Index can be 0 (=total number of hulls) or a hull type (=number of engines of that type).
           EMPTY if no base. */
        return makeArrayProperty(pl, root, shipList, bapHullStorage);

     case ibpBeamStorage:
        /* @q Storage.Beams:Int() (Planet Property)
           Number of beams in starbase storage.
           Index can be 0 (=total number of beams) or a beam type (=number of beams of that type).
           EMPTY if no base. */
        return makeArrayProperty(pl, root, shipList, bapBeamStorage);

     case ibpLauncherStorage:
        /* @q Storage.Launchers:Int() (Planet Property)
           Number of torpedo launchers in starbase storage.
           Index can be 0 (=total number of launchers) or a torpedo type (=number of launchers of that type).
           EMPTY if no base. */
        return makeArrayProperty(pl, root, shipList, bapLauncherStorage);

     case ibpAmmoStorage:
        /* @q Storage.Ammo:Int() (Planet Property)
           Number of torpedoes or fighters in starbase storage.
           Index can be 0 (=total number of weapons), a torpedo type (=number of torpedoes of that type),
           or 11 (=number of fighters, see {Fighters (Planet Property)}.
           EMPTY if no base. */
        return makeArrayProperty(pl, root, shipList, bapAmmoStorage);
    }
    return 0;
}

void
game::interface::setBaseProperty(game::map::Planet& pl, BaseProperty ibp, const afl::data::Value* value)
{
    // ex int/if/baseif.h:setBaseProperty
    if (!pl.hasBase() || !pl.isPlayable(game::map::Planet::Playable)) {
        throw Error::notAssignable();
    }

    int32_t iv;
    switch (ibp) {
     case ibpMission:
        if (interpreter::checkIntegerArg(iv, value, 0, MAX_BASE_MISSION)) {
            pl.setBaseMission(iv);
        }
        break;
     default:
        throw Error::notAssignable();
    }
}
