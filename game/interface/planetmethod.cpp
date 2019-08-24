/**
  *  \file game/interface/planetmethod.cpp
  */

#include "game/interface/planetmethod.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/value.hpp"
#include "game/actions/basefixrecycle.hpp"
#include "game/actions/buildammo.hpp"
#include "game/actions/buildparts.hpp"
#include "game/actions/buildship.hpp"
#include "game/actions/buildstarbase.hpp"
#include "game/actions/buildstructures.hpp"
#include "game/actions/preconditions.hpp"
#include "game/actions/techupgrade.hpp"
#include "game/exception.hpp"
#include "game/interface/baseproperty.hpp"
#include "game/interface/cargomethod.hpp"
#include "game/interface/objectcommand.hpp"
#include "game/interface/planetproperty.hpp"
#include "game/limits.hpp"
#include "game/map/planetformula.hpp"
#include "game/map/planetstorage.hpp"
#include "game/map/reverter.hpp"
#include "game/map/shipstorage.hpp"
#include "game/root.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"

using game::Exception;

namespace {
    void setBaseShipyardOrder(game::map::Planet& pl,
                              game::ShipyardAction action,
                              interpreter::Arguments& args,
                              game::Turn& turn)
    {
        // Parse args
        int32_t n;
        args.checkArgumentCount(1);
        if (!interpreter::checkIntegerArg(n, args.getNext())) {
            return;
        }

        // Find associated ship
        game::map::Ship* ship;
        if (n == 0) {
            action = game::NoShipyardAction;
            ship = 0;
        } else {
            ship = turn.universe().ships().get(n);
            if (ship == 0) {
                throw interpreter::Error::rangeError();
            }
        }

        // Try it
        if (!game::actions::BaseFixRecycle(pl).set(action, ship)) {
            // FIXME: PCC2 would also generate ePos if positions are different
            throw Exception(Exception::ePerm);
        }
    }

    void doBuildBase(game::map::Planet& planet, interpreter::Arguments& args, game::Session& session, game::Root& root)
    {
        // ex IFPlanetBuildBase
        // Check arguments
        bool wantBase = true;
        args.checkArgumentCount(0, 1);
        if (args.getNumArgs() > 0) {
            if (!interpreter::checkBooleanArg(wantBase, args.getNext())) {
                return;
            }
        }

        // Do it
        game::config::HostConfiguration& config = root.hostConfiguration();
        game::map::PlanetStorage container(planet, session.interface(), config);
        game::actions::BuildStarbase action(planet, container, wantBase, session.translator(), config);
        action.commit();
    }

    void doAutobuild(game::map::Planet& planet, interpreter::Arguments& args, game::Session& session, game::Root& root)
    {
        // ex IFPlanetAutoBuild
        args.checkArgumentCount(0);

        game::config::HostConfiguration& config = root.hostConfiguration();
        game::map::PlanetStorage container(planet, session.interface(), config);
        game::actions::BuildStructures action(planet, container, config);
        action.doStandardAutoBuild();
        action.commit();
    }

    void doBuildStructures(game::map::Planet& planet,
                           interpreter::Process& process,
                           interpreter::Arguments& args,
                           game::Session& session,
                           game::Turn& turn,
                           game::Root& root,
                           const game::PlanetaryBuilding type)
    {
        // ex int/if/planetif.cc:doStructureBuild
        args.checkArgumentCount(1, 2);
        int32_t count = 0;
        int32_t flag = 0;
        if (!interpreter::checkIntegerArg(count, args.getNext(), -game::MAX_NUMBER, +game::MAX_NUMBER)) {
            return;
        }
        interpreter::checkFlagArg(flag, 0, args.getNext(), "N");

        game::config::HostConfiguration& config = root.hostConfiguration();
        game::map::PlanetStorage container(planet, session.interface(), config);
        game::actions::BuildStructures action(planet, container, config);
        action.setUndoInformation(turn.universe());

        int32_t built = action.addLimitCash(type, count);
        if (flag != 0) {
            // We permit partial build, place remainder in BUILD.REMAINDER
            afl::data::IntegerValue iv(count - built);
            process.setVariable("BUILD.REMAINDER", &iv);
            action.commit();
        } else {
            // We do not permit partial build, so refuse it
            if (built != count) {
                throw game::Exception(game::Exception::ePerm);
            }
            action.commit();
        }
    }

    void doSellSupplies(game::map::Planet& pl,
                        interpreter::Process& process,
                        interpreter::Arguments& args,
                        game::Turn& turn)
    {
        // ex int/if/planetif.cc:IFPlanetSellSupplies
        // Fetch arguments
        args.checkArgumentCount(1, 2);

        int32_t amount;
        if (!interpreter::checkIntegerArg(amount, args.getNext())) {
            return;
        }

        int32_t flag = 0;
        interpreter::checkFlagArg(flag, 0, args.getNext(), "N");

        // Planet must be played
        game::actions::mustBePlayed(pl);
        int32_t availableSupplies = pl.getCargo(game::Element::Supplies).orElse(0);
        int32_t availableMoney    = pl.getCargo(game::Element::Money).orElse(0);

        // Do it
        int32_t undid;
        if (amount == 0) {
            // No change
            undid = 0;
        } else if (amount > 0) {
            // Sell supplies.
            // FIXME: PCC2 has a transaction for that. We don't.
            int32_t suppliesToConvert = std::min(amount, availableSupplies);
            undid = amount - suppliesToConvert;
            if (flag == 0 && undid != 0) {
                throw game::Exception(game::Exception::ePerm);
            }

            pl.setCargo(game::Element::Supplies, availableSupplies - suppliesToConvert);
            pl.setCargo(game::Element::Money,    availableMoney    + suppliesToConvert);
        } else {
            // Buy supples. We do not yet have a transaction for that. FIXME.
            int32_t buyLimit = 0;
            if (game::map::Reverter* rev = turn.universe().getReverter()) {
                buyLimit = std::min(availableMoney, rev->getSuppliesAllowedToBuy(pl.getId()));
            }
            int32_t suppliesToConvert = std::min(-amount, buyLimit);
            undid = suppliesToConvert + amount;
            if (flag == 0 && undid != 0) {
                throw game::Exception(game::Exception::ePerm);
            }

            pl.setCargo(game::Element::Supplies, availableSupplies + suppliesToConvert);
            pl.setCargo(game::Element::Money,    availableMoney    - suppliesToConvert);
        }

        // Set variable
        if (flag != 0) {
            afl::data::IntegerValue iv(undid);
            process.setVariable("BUILD.REMAINDER", &iv);
        }
    }

    void doSetTech(game::map::Planet& pl,
                   interpreter::Arguments& args,
                   game::Session& session,
                   game::Turn& turn,
                   game::Root& root)
    {
        // ex IFBaseSetTech
        args.checkArgumentCount(2);

        // Fetch arguments
        int32_t area, tech;
        if (!interpreter::checkIntegerArg(area, args.getNext(), 1, int32_t(game::NUM_TECH_AREAS))) {
            return;
        }
        if (!interpreter::checkIntegerArg(tech, args.getNext(), 1, 10)) {
            return;
        }
        --area;

        // Fetch ship list
        game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);

        // Create tech upgrade action (checks preconditions)
        game::map::PlanetStorage container(pl, session.interface(), root.hostConfiguration());
        game::actions::TechUpgrade action(pl, container, shipList, root);
        action.setUndoInformation(turn.universe());

        // Do the rules permit this?
        if (!action.setTechLevel(game::TechLevel(area), tech)) {
            throw game::Exception(game::Exception::ePerm);
        }

        // Execute
        action.commit();
    }

    void doBuildComponents(game::map::Planet& pl,
                           interpreter::Process& process,
                           game::Session& session,
                           game::Turn& turn,
                           game::Root& root,
                           game::TechLevel area,
                           int slot,
                           int amount,
                           bool flag)
    {
        // ex int/if/baseif.cc:doBaseBuildComponents
        // Fetch ship list
        game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);

        // Create action
        game::map::PlanetStorage container(pl, session.interface(), root.hostConfiguration());
        game::actions::BuildParts action(pl, container, shipList, root);
        action.setUndoInformation(turn.universe());

        // Do it
        int did = action.add(area, slot, amount, flag);
        if (flag) {
            // Try to back out if we have too little cash, then commit (this will throw if there is a different error).
            while (did > 0 && !action.costAction().isValid() && action.add(area, slot, -1, false) != 0) {
                --did;
            }
            action.commit();

            // Tell user about remainder
            afl::data::IntegerValue iv(amount - did);
            process.setVariable("BUILD.REMAINDER", &iv);
        } else {
            // We are not permitted to do a partial build.
            if (did != amount) {
                throw game::Exception(game::Exception::eNoResource);
            }
            action.commit();
        }
    }

    void doBuildEngines(game::map::Planet& pl,
                        interpreter::Process& process,
                        interpreter::Arguments& args,
                        game::Session& session,
                        game::Turn& turn,
                        game::Root& root)
    {
        // ex int/if/baseif.cc:IFBaseBuildEngines
        args.checkArgumentCount(2, 3);

        game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);

        int32_t type;
        if (!interpreter::checkIntegerArg(type, args.getNext(), 1, shipList.engines().size())) {
            return;
        }

        int32_t amount;
        if (!interpreter::checkIntegerArg(amount, args.getNext(), -game::MAX_NUMBER, +game::MAX_NUMBER)) {
            return;
        }

        int32_t flag = 0;
        interpreter::checkFlagArg(flag, 0, args.getNext(), "N");

        doBuildComponents(pl, process, session, turn, root, game::EngineTech, type, amount, (flag != 0));
    }

    void doBuildHulls(game::map::Planet& pl,
                      interpreter::Process& process,
                      interpreter::Arguments& args,
                      game::Session& session,
                      game::Turn& turn,
                      game::Root& root)
    {
        // ex int/if/baseif.cc:IFBaseBuildHulls
        args.checkArgumentCount(2, 3);

        game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);

        // Fetch planet owner. This will not fail (and if it does, getIndexFromHull will refuse it).
        int planetOwner = 0;
        pl.getOwner(planetOwner);

        int32_t type;
        if (!interpreter::checkIntegerArg(type, args.getNext(), 1, shipList.hulls().size())) {
            return;
        }

        int32_t amount;
        if (!interpreter::checkIntegerArg(amount, args.getNext(), -game::MAX_NUMBER, +game::MAX_NUMBER)) {
            return;
        }

        int32_t flag = 0;
        interpreter::checkFlagArg(flag, 0, args.getNext(), "N");

        // Can we build this hull?
        int slot = shipList.hullAssignments().getIndexFromHull(root.hostConfiguration(), planetOwner, type);
        if (slot == 0) {
            if (amount != 0) {
                throw game::Exception(game::Exception::ePerm);
            }
        } else {
            doBuildComponents(pl, process, session, turn, root, game::HullTech, slot, amount, (flag != 0));
        }
    }

    void doBuildLaunchers(game::map::Planet& pl,
                          interpreter::Process& process,
                          interpreter::Arguments& args,
                          game::Session& session,
                          game::Turn& turn,
                          game::Root& root)
    {
        // ex int/if/baseif.cc:IFBaseBuildLaunchers
        args.checkArgumentCount(2, 3);

        game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);

        int32_t type;
        if (!interpreter::checkIntegerArg(type, args.getNext(), 1, shipList.launchers().size())) {
            return;
        }

        int32_t amount;
        if (!interpreter::checkIntegerArg(amount, args.getNext(), -game::MAX_NUMBER, +game::MAX_NUMBER)) {
            return;
        }

        int32_t flag = 0;
        interpreter::checkFlagArg(flag, 0, args.getNext(), "N");

        doBuildComponents(pl, process, session, turn, root, game::TorpedoTech, type, amount, (flag != 0));
    }

    void doBuildBeams(game::map::Planet& pl,
                      interpreter::Process& process,
                      interpreter::Arguments& args,
                      game::Session& session,
                      game::Turn& turn,
                      game::Root& root)
    {
        // ex int/if/baseif.cc:IFBaseBuildBeams
        args.checkArgumentCount(2, 3);

        game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);

        int32_t type;
        if (!interpreter::checkIntegerArg(type, args.getNext(), 1, shipList.beams().size())) {
            return;
        }

        int32_t amount;
        if (!interpreter::checkIntegerArg(amount, args.getNext(), -game::MAX_NUMBER, +game::MAX_NUMBER)) {
            return;
        }

        int32_t flag = 0;
        interpreter::checkFlagArg(flag, 0, args.getNext(), "N");

        doBuildComponents(pl, process, session, turn, root, game::BeamTech, type, amount, (flag != 0));
    }

    void doBuildShip(game::map::Planet& pl,
                     interpreter::Arguments& args,
                     game::Session& session,
                     game::Root& root)
    {
        // ex int/if/baseif.cc:IFBaseBuildShip
        // Parse args
        game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);
        game::ShipBuildOrder o;
        if (!game::interface::parseBuildShipCommand(args, o, shipList)) {
            return;
        }

        // Get planet
        game::actions::mustHavePlayedBase(pl);

        // Check for cancellation
        if (o.getHullIndex() == 0) {
            pl.setBaseBuildOrder(o);
            return;
        }

        // Make a transaction and fire it
        game::map::PlanetStorage container(pl, session.interface(), root.hostConfiguration());
        game::actions::BuildShip a(pl, container, shipList, root);
        a.setUsePartsFromStorage(true);
        a.setBuildOrder(o);
        a.commit();
    }

    void doBuildAmmo(game::map::Planet& pl,
                     interpreter::Process& process,
                     game::Session& session,
                     game::Turn& turn,
                     game::Root& root,
                     game::Element::Type type,
                     int32_t amount,
                     bool partial,
                     int32_t shipId)
    {
        // Resolve optional ship Id and build receiver
        std::auto_ptr<game::CargoContainer> pReceiver;
        if (shipId != 0) {
            // Ship must exist...
            game::map::Ship* pShip = turn.universe().ships().get(shipId);
            if (pShip == 0) {
                throw Exception(Exception::eRange);
            }

            // ...be played... (this check redundant; also in ShipStorage)
            game::actions::mustBePlayed(*pShip);

            // ...at same place.
            game::map::Point planetPos, shipPos;
            if (!pl.getPosition(planetPos) || !pShip->getPosition(shipPos) || planetPos != shipPos) {
                throw Exception(Exception::ePos);
            }

            // ok
            pReceiver.reset(new game::map::ShipStorage(*pShip, session.interface(), game::actions::mustHaveShipList(session)));
        } else {
            // No ship; use planet
            pReceiver.reset(new game::map::PlanetStorage(pl, session.interface(), root.hostConfiguration()));
        }

        // Build remainder
        game::map::PlanetStorage financier(pl, session.interface(), root.hostConfiguration());
        game::actions::BuildAmmo action(pl, financier, *pReceiver, game::actions::mustHaveShipList(session), root);
        action.setUndoInformation(turn.universe());

        // Do it
        if (partial) {
            int32_t done = action.addLimitCash(type, amount);
            action.commit();

            // Tell user about remainder
            afl::data::IntegerValue iv(amount - done);
            process.setVariable("BUILD.REMAINDER", &iv);
        } else {
            if (action.add(type, amount, false) != amount) {
                throw Exception(Exception::eRange);
            }
            action.commit();
        }
    }

    void doBuildTorpedoes(game::map::Planet& pl,
                          interpreter::Process& process,
                          interpreter::Arguments& args,
                          game::Session& session,
                          game::Turn& turn,
                          game::Root& root)
    {
        // ex int/if/baseif.cc:IFBaseBuildTorps
        args.checkArgumentCount(2, 3);
        game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);

        int32_t type;
        if (!interpreter::checkIntegerArg(type, args.getNext(), 1, shipList.launchers().size())) {
            return;
        }

        int32_t amount;
        if (!interpreter::checkIntegerArg(amount, args.getNext(), -game::MAX_NUMBER, game::MAX_NUMBER)) {
            return;
        }

        int32_t flag = 0;
        int32_t sid = 0;
        interpreter::checkFlagArg(flag, &sid, args.getNext(), "N");

        doBuildAmmo(pl, process, session, turn, root, game::Element::fromTorpedoType(type), amount, (flag != 0), sid);
    }

    void doBuildFighters(game::map::Planet& pl,
                         interpreter::Process& process,
                         interpreter::Arguments& args,
                         game::Session& session,
                         game::Turn& turn,
                         game::Root& root)
    {
        // ex int/if/baseif.cc:IFBaseBuildFighters
        args.checkArgumentCount(1, 2);

        int32_t amount;
        if (!interpreter::checkIntegerArg(amount, args.getNext(), -game::MAX_NUMBER, game::MAX_NUMBER)) {
            return;
        }

        int32_t flag = 0;
        int32_t sid = 0;
        interpreter::checkFlagArg(flag, &sid, args.getNext(), "N");

        doBuildAmmo(pl, process, session, turn, root, game::Element::Fighters, amount, (flag != 0), sid);
    }

    void doAutoTaxColonists(game::map::Planet& pl, const game::Root& root)
    {
        // ex IFPlanetAutoTaxColonists
        game::actions::mustBePlayed(pl);
        int mines, factories;
        if (pl.getNumBuildings(game::MineBuilding).get(mines) && pl.getNumBuildings(game::FactoryBuilding).get(factories)) {
            int tax;
            if (game::map::getColonistSafeTax(pl, root.hostConfiguration(), root.hostVersion(), mines + factories).get(tax)) {
                pl.setColonistTax(tax);
            }
        }
    }

    void doAutoTaxNatives(game::map::Planet& pl, const game::Root& root)
    {
        // ex IFPlanetAutoTaxNatives
        game::actions::mustBePlayed(pl);
        int mines, factories;
        if (pl.getNumBuildings(game::MineBuilding).get(mines) && pl.getNumBuildings(game::FactoryBuilding).get(factories)) {
            int tax;
            if (game::map::getNativeSafeTax(pl, root.hostConfiguration(), root.hostVersion(), mines + factories).get(tax)) {
                pl.setNativeTax(tax);
            }
        }
    }
}

void
game::interface::callPlanetMethod(game::map::Planet& pl,
                                  PlanetMethod ipm,
                                  interpreter::Arguments& args,
                                  interpreter::Process& process,
                                  Session& session,
                                  Turn& turn,
                                  Root& root)
{
    switch (ipm) {
     case ipmMark:
        IFObjMark(pl, args);
        break;

     case ipmUnmark:
        IFObjUnmark(pl, args);
        break;

     case ipmSetComment:
        /* @q SetComment s:Str (Planet Command)
           Set planet comment.
           @see Comment (Planet Property)
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex IFPlanetSetComment
        args.checkArgumentCount(1);
        if (afl::data::Value* value = args.getNext()) {
            if (afl::data::Segment* seg = session.world().planetProperties().create(pl.getId())) {
                seg->setNew(interpreter::World::pp_Comment, interpreter::makeStringValue(interpreter::toString(value, false)));
            }
            pl.markDirty();
        }
        break;

     case ipmFixShip:
        /* @q FixShip sid:Int (Planet Command)
           Fix (repair) a ship. The %sid is a ship Id, or 0 to cancel a pending shipyard order.
           @since PCC2 1.99.9, PCC 1.0.5, PCC2 2.40.1 */
        // ex IFBaseFixShip
        setBaseShipyardOrder(pl, FixShipyardAction, args, turn);
        break;

     case ipmRecycleShip:
        /* @q RecycleShip sid:Int (Planet Command)
           Recycle a ship. The %sid is a ship Id, or 0 to cancel a pending shipyard order.
           @since PCC2 1.99.9, PCC 1.0.5, PCC2 2.40.1 */
        // ex IFBaseRecycleShip
        setBaseShipyardOrder(pl, RecycleShipyardAction, args, turn);
        break;

     case ipmBuildBase:
        /* @q BuildBase Optional flag:Bool (Planet Command)
           Build a starbase.
           If the parameter is specified as True or not at all, builds the base.
           If the parameter is specified as False, cancels a previous build order.
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.3 */
        doBuildBase(pl, args, session, root);
        break;

     case ipmAutoBuild:
        /* @q AutoBuild (Planet Command)
           Perform a standard auto-build operation.
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.3 */
        doAutobuild(pl, args, session, root);
        break;

     case ipmBuildDefense:
        /* @q BuildDefense amount:Int, Optional flag:Str (Planet Command)
           Build defense posts.
           Build the the specified number of structures, or scraps them if %amount is negative.
           Fails if you don't own the planet, don't have the required resources,
           or if the new amount of structures is not allowed by the rules.

           If the %flag is specified as <tt>"n"</tt>, the command will not fail due to lacking resources.
           Instead, it will build as many structures as possible, and set the variable {Build.Remainder}
           to the amount not built.
           @see BuildDefenseWait
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.3 */
        // ex IFPlanetBuildDefense
        doBuildStructures(pl, process, args, session, turn, root, DefenseBuilding);
        break;

     case ipmBuildFactories:
        /* @q BuildFactories amount:Int, Optional flag:Str (Planet Command)
           Build factories.
           Build the the specified number of structures, or scraps them if %amount is negative.
           Fails if you don't own the planet, don't have the required resources,
           or if the new amount of structures is not allowed by the rules.

           If the %flag is specified as <tt>"n"</tt>, the command will not fail due to lacking resources.
           Instead, it will build as many structures as possible, and set the variable {Build.Remainder}
           to the amount not built.
           @see BuildFactoriesWait
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.3 */
        // ex IFPlanetBuildFactories
        doBuildStructures(pl, process, args, session, turn, root, FactoryBuilding);
        break;

     case ipmBuildMines:
        /* @q BuildMines amount:Int, Optional flag:Str (Planet Command)
           Build mineral mines.
           Build the the specified number of structures, or scraps them if %amount is negative.
           Fails if you don't own the planet, don't have the required resources,
           or if the new amount of structures is not allowed by the rules.

           If the %flag is specified as <tt>"n"</tt>, the command will not fail due to lacking resources.
           Instead, it will build as many structures as possible, and set the variable {Build.Remainder}
           to the amount not built.
           @see BuildMinesWait
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.3 */
        // ex IFPlanetBuildMines
        doBuildStructures(pl, process, args, session, turn, root, MineBuilding);
        break;

     case ipmSetColonistTax:
        /* @q SetColonistTax n:Int (Planet Command)
           Set colonist tax.
           @see Colonists.Tax
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex IFPlanetSetColonistTax
        args.checkArgumentCount(1);
        setPlanetProperty(pl, ippColonistTax, args.getNext(), root);
        break;

     case ipmSetNativeTax:
        /* @q SetNativeTax n:Int (Planet Command)
           Set native tax.
           @see Natives.Tax
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex IFPlanetSetNativeTax
        args.checkArgumentCount(1);
        setPlanetProperty(pl, ippNativeTax, args.getNext(), root);
        break;

     case ipmSetFCode:
        /* @q SetFCode fc:Str (Planet Command)
           Set planet friendly code.
           @see FCode (Planet Property)
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex IFPlanetSetFCode
        args.checkArgumentCount(1);
        setPlanetProperty(pl, ippFCode, args.getNext(), root);
        break;

     case ipmSetMission:
        /* @q SetMission number:Int (Planet Command)
           Set starbase mission.
           @since PCC2 1.99.9, PCC 1.0.5, PCC2 2.40.1 */
        // ex IFBaseSetMission
        args.checkArgumentCount(1);
        setBaseProperty(pl, ibpMission, args.getNext());
        break;

     case ipmBuildBaseDefense:
        /* @q BuildBaseDefense amount:Int, Optional flag:Str (Planet Command)
           Build starbase defense.
           Build the the specified number of structures, or scraps them if %amount is negative.
           Fails if you don't own the planet, don't have the required resources,
           or if the new amount of structures is not allowed by the rules.

           If the %flag is specified as <tt>"n"</tt>, the command will not fail due to lacking resources.
           Instead, it will build as many structures as possible, and set the variable {Build.Remainder}
           to the amount not built.
           @see BuildBaseDefenseWait
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.3 */
        // ex IFPlanetBuildBaseDefense
        doBuildStructures(pl, process, args, session, turn, root, BaseDefenseBuilding);
        break;

     case ipmSetTech:
        /* @q SetTech area:Int, level:Int (Planet Command)
           Set starbase tech level. %area is 1 for engines, 2 for hulls, 3
           for beams, 4 for torpedoes. %level is the new tech level.

           Note that when you build a ship, PCC automatically upgrades tech.
           You can raise tech levels, and lower them again when you have not
           yet used them.

           @since PCC2 1.99.9, PCC 1.1, PCC2 2.40.3 */
        doSetTech(pl, args, session, turn, root);
        break;

     case ipmBuildFighters:
        /* @q BuildFighters amount:Int, Optional flagAndShipId:Any (Planet Command)
           Build fighters.

           Attempts to build %amount fighters. The amount can be negative to
           scrap fighters. The %flagAndShipId can be "N" to permit partial
           builds. If not all of the requested amount can be built, the
           command will report the amount not built in the variable
           %Build.Remainder instead of failing.

           %flagAndShipId can also contain a ship Id, to place the
           newly-built fighters on that ship.

           @since PCC2 1.99.9, PCC 1.1.5, PCC2 2.40.3 */
        doBuildFighters(pl, process, args, session, turn, root);
        break;

     case ipmBuildEngines:
        /* @q BuildEngines type:Int, amount:Int, Optional flag:Str (Planet Command)
           Build engines.

           Attempts to build %amount engines of the given %type. The amount
           can be negative to scrap engines. The tech levels is automatically
           raised as necessary. The %flag can be "N" to permit partial
           builds. If not all of the requested amount can be built, the
           command will report the amount not built in the variable
           %Build.Remainder instead of failing.

           @since PCC2 1.99.9, PCC 1.1.16, PCC2 2.40.3 */
        doBuildEngines(pl, process, args, session, turn, root);
        break;

     case ipmBuildHulls:
        /* @q BuildHulls type:Int, amount:Int, Optional flag:Str (Planet Command)
           Build starship hulls.

           Attempts to build %amount hulls of the given %type. The amount
           can be negative to scrap hulls. The tech levels is automatically
           raised as necessary. The %flag can be "N" to permit partial
           builds. If not all of the requested amount can be built, the
           command will report the amount not built in the variable
           %Build.Remainder instead of failing.

           The %type is a hull Id. You can not build all hulls; the command will
           fail if you try to build one you cannot build.

           @since PCC2 1.99.9, PCC 1.1.16, PCC2 2.40.3 */
        doBuildHulls(pl, process, args, session, turn, root);
        break;

     case ipmBuildLaunchers:
        /* @q BuildLaunchers type:Int, amount:Int, Optional flag:Str (Planet Command)
           Build torpedo launchers.

           Attempts to build %amount torpedo launchers of the given %type.
           The amount can be negative to scrap launchers. The tech levels is
           automatically raised as necessary. The %flag can be "N" to permit
           partial builds. If not all of the requested amount can be built,
           the command will report the amount not built in the variable
           %Build.Remainder instead of failing.

           @since PCC2 1.99.9, PCC 1.1.16, PCC2 2.40.3 */
        doBuildLaunchers(pl, process, args, session, turn, root);
        break;

     case ipmBuildBeams:
        /* @q BuildBeams type:Int, amount:Int, Optional flag:Str (Planet Command)
           Build beam weapons.

           Attempts to build %amount beams of the given %type. The amount
           can be negative to scrap beams. The tech levels is automatically
           raised as necessary. The %flag can be "N" to permit partial
           builds. If not all of the requested amount can be built, the
           command will report the amount not built in the variable
           %Build.Remainder instead of failing.

           @since PCC2 1.99.9, PCC 1.1.16, PCC2 2.40.3 */
        doBuildBeams(pl, process, args, session, turn, root);
        break;

     case ipmBuildTorps:
        /* @q BuildTorps type:Int, amount:Int, Optional flagAndShipId:Any (Planet Command)
           Build torpedoes.

           Attempts to build %amount torpedoes of the given type. The amount
           can be negative to scrap torpedoes. The %flagAndShipId can be "N"
           to permit partial builds. If not all of the requested amount can
           be built, the command will report the amount not built in the
           variable %Build.Remainder instead of failing.

           %flagAndShipId can also contain a ship Id, to place the
           newly-built torpedoes on that ship.

           @since PCC2 1.99.9, PCC 1.1.5, PCC2 2.40.3 */
        doBuildTorpedoes(pl, process, args, session, turn, root);
        break;

     case ipmSellSupplies:
        /* @q SellSupplies amount:Int, Optional flags:Str (Planet Command)
           Sell or buy supplies.
           Sells the specified number of supplies (for one megacredit each), or buys supplies if %count is negative.
           You can only buy back supplies you sold this turn.
           Fails if you don't own the planet, or the rules forbid you to buy/sell the specified amount
           (because you don't have enough, maybe).

           Optionally, you can specify the flag "n", as in
           | SellSupplies 1000, "n"
           When you can't sell/buy the specified amount, this will sell as much as possible instead of failing.
           The variable {Build.Remainder} will be set to the amount that was not sold.
           For example, if the planet on which you run the above command only has 650 supplies,
           %Build.Remainder will be set to 350.
           @since PCC 1.0.19, PCC2 1.99.9, PCC2 2.40.3 */
        doSellSupplies(pl, process, args, turn);
        break;

     case ipmBuildShip:
        /* @q BuildShip hull:Int, Optional engine:Int, beamtype:Int, beamcount:Int, torptype:Int, torpcount:Int (Planet Command)
           Submit a starship build order.
           If %hull is zero, cancels a pending order.
           Otherwise, builds a ship.
           In this case, %engine must be specified, and the others should better be specified as well to
           avoid building a ship without weapons.

           Required tech levels and parts are bought automatically.

           @since PCC2 1.99.16, PCC 1.0.6, PCC2 2.40.3 */
        doBuildShip(pl, args, session, root);
        break;

     case ipmCargoTransfer:
        doCargoTransfer(pl, process, args, session, turn, root);
        break;

     case ipmAutoTaxColonists:
        /* @q AutoTaxColonists (Planet Command)
           Auto-tax for colonists.
           @since PCC2 1.99.15, PCC2 2.40.3 */
        args.checkArgumentCount(0);
        doAutoTaxColonists(pl, root);
        break;

     case ipmAutoTaxNatives:
        /* @q AutoTaxNatives (Planet Command)
           Auto-tax for natives.
           @since PCC2 1.99.15, PCC2 2.40.3 */
        args.checkArgumentCount(0);
        doAutoTaxNatives(pl, root);
        break;
    }
}


// /** Parse build ship command.
//     \param args [in] arguments
//     \param o [out] build order
//     \retval true correctly parsed
//     \retval false null argument encountered
//     \throws GError or IntError on error */
bool
game::interface::parseBuildShipCommand(interpreter::Arguments& args, ShipBuildOrder& o, const game::spec::ShipList& shipList)
{
    using interpreter::Error;
    args.checkArgumentCount(1, 6);

    // Mandatory arg
    int32_t hull;
    if (!interpreter::checkIntegerArg(hull, args.getNext())) {
        return false;
    }

    // Optional args
    int32_t engine = 0, beam = 0, beamcount = -1, torp = 0, torpcount = -1;
    interpreter::checkIntegerArg(engine, args.getNext());
    interpreter::checkIntegerArg(beam, args.getNext());
    interpreter::checkIntegerArg(beamcount, args.getNext());
    interpreter::checkIntegerArg(torp, args.getNext());
    interpreter::checkIntegerArg(torpcount, args.getNext());

    // Check mandatory arg
    o.setHullIndex(hull);
    if (hull == 0) {
        // Canceling a build
        return true;
    }

    // This is a ship build order. Validate remaining args
    const game::spec::Hull* pHull = shipList.hulls().get(hull);
    if (pHull == 0) {
        throw Error::rangeError();
    }

    // Engine
    if (shipList.engines().get(engine) == 0) {
        throw Error::rangeError();
    }
    o.setEngineType(engine);

    // Beams
    if (beamcount == -1) {
        beamcount = pHull->getMaxBeams();
    }
    if (beam == 0 || beamcount == 0) {
        o.setBeamType(0);
        o.setNumBeams(0);
    } else {
        if (beamcount < 0 || beamcount > pHull->getMaxBeams()) {
            throw Error::rangeError();
        }
        o.setBeamType(beam);
        o.setNumBeams(beamcount);
    }

    // Torps
    if (torpcount == -1) {
        torpcount = pHull->getMaxLaunchers();
    }
    if (torp == 0 || torpcount == 0) {
        o.setLauncherType(0);
        o.setNumLaunchers(0);
    } else {
        if (torpcount < 0 || torpcount > pHull->getMaxLaunchers()) {
            throw Error::rangeError();
        }
        o.setLauncherType(torp);
        o.setNumLaunchers(torpcount);
    }
    return true;
}

