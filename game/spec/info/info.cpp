/**
  *  \file game/spec/info/info.cpp
  */

#include "game/spec/info/info.hpp"
#include "afl/string/format.hpp"
#include "game/spec/info/picturenamer.hpp"
#include "util/string.hpp"
#include "game/spec/fighter.hpp"
#include "afl/string/nulltranslator.hpp"

namespace {
    using afl::string::Format;
    using game::config::HostConfiguration;
    using util::addListItem;

    int getTorpDamageScale(const game::Root& root)
    {
        return root.hostVersion().hasDoubleTorpedoPower(root.hostConfiguration()) ? 2 : 1;
    }
}

void
game::spec::info::describeHull(PageContent& content, Id_t id, const ShipList& shipList, bool withCost, const PictureNamer& picNamer, const Root& root, int viewpointPlayer, afl::string::Translator& tx)
{
    if (const Hull* h = shipList.hulls().get(id)) {
        util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();

        // Header
        content.title = h->getName(shipList.componentNamer());
        content.pictureName = picNamer.getHullPicture(*h);

        // Content
        content.attributes.push_back(Attribute(tx("Mass"),          Format(tx("%d kt"), fmt.formatNumber(h->getMass()))));
        content.attributes.push_back(Attribute(tx("Cargo"),         Format(tx("%d kt"), fmt.formatNumber(h->getMaxCargo()))));
        content.attributes.push_back(Attribute(tx("Fuel"),          Format(tx("%d kt"), fmt.formatNumber(h->getMaxFuel()))));
        content.attributes.push_back(Attribute(tx("Engines"),       fmt.formatNumber(h->getNumEngines())));
        content.attributes.push_back(Attribute(tx("Crew"),          fmt.formatNumber(h->getMaxCrew())));

        // Weapons
        String_t weapons;
        if (h->getMaxBeams() != 0) {
            addListItem(weapons, ", ", Format(tx("%d beam%!1{s%}"), fmt.formatNumber(h->getMaxBeams())));
        }
        if (h->getMaxLaunchers() != 0) {
            addListItem(weapons, ", ", Format(tx("%d torpedo launcher%!1{s%}"), fmt.formatNumber(h->getMaxLaunchers())));
        }
        if (h->getNumBays() != 0) {
            addListItem(weapons, ", ", Format(tx("%d fighter bay%!1{s%}"), fmt.formatNumber(h->getNumBays())));
        }
        if (weapons.empty()) {
            weapons = tx("none");
        }
        content.attributes.push_back(Attribute(tx("Weapons"), weapons));

        // Mine hit damage
        content.attributes.push_back(Attribute(tx("Mine hit damage"), Format("%d%%", h->getMineHitDamage(viewpointPlayer, false, root.hostVersion(), root.hostConfiguration()))));

        // Fuel burn
        if (root.hostVersion().isEugeneGame(root.hostConfiguration())) {
            content.attributes.push_back(Attribute(tx("Fuel burn"),
                                                   Format(tx("%d kt/turn, %d kt/fight"),
                                                          h->getTurnFuelUsage(viewpointPlayer, false, root.hostConfiguration()),
                                                          h->getTurnFuelUsage(viewpointPlayer, true,  root.hostConfiguration()))));
        }

        // Cost
        if (withCost) {
            content.attributes.push_back(Attribute(tx("Cost"), h->cost().format(tx, fmt)));
            content.attributes.push_back(Attribute(tx("Tech level"), fmt.formatNumber(h->getTechLevel())));
        }

        // Abilities
        HullFunctionList hfList;
        shipList.enumerateHullFunctions(hfList, id, root.hostConfiguration(), PlayerSet_t::allUpTo(MAX_PLAYERS), ExperienceLevelSet_t::allUpTo(MAX_EXPERIENCE_LEVELS),
                                        true /* includeNewShip */, false /* includeRacialAbilities */);
        hfList.simplify();
        hfList.sortForNewShip(PlayerSet_t(viewpointPlayer));
        describeHullFunctions(content.abilities, hfList, shipList, picNamer, root, tx);

        // Players
        content.players = shipList.hullAssignments().getPlayersForHull(root.hostConfiguration(), id);
    }
}

void
game::spec::info::describeHullFunctions(Abilities_t& out, const HullFunctionList& hfList, const ShipList& shipList, const PictureNamer& picNamer, const Root& root, afl::string::Translator& tx)
{
    // ex drawHullFunctionList (sort-of)
    // FIXME: implement have/damaged/will-have/will-not-have annotations
    for (HullFunctionList::Iterator_t it = hfList.begin(), e = hfList.end(); it != e; ++it) {
        String_t pictureName;
        String_t info;
        if (const BasicHullFunction* hf = shipList.basicHullFunctions().getFunctionById(it->getBasicFunctionId())) {
            pictureName = picNamer.getAbilityPicture(hf->getPictureName());
            info = hf->getDescription();
        } else {
            info = Format(tx("Hull Function #%d"), it->getBasicFunctionId());
        }

        // Annotations
        String_t annot;
        util::addListItem(annot, "; ", formatPlayerSet(it->getPlayers(), root.playerList(), tx));
        util::addListItem(annot, "; ", formatExperienceLevelSet(it->getLevels(), root.hostVersion(), root.hostConfiguration(), tx));
        if (it->getKind() == HullFunction::AssignedToShip) {
            util::addListItem(annot, "; ", tx("ship"));
        }

        // Build total
        if (!annot.empty()) {
            info += " (";
            info += annot;
            info += ")";
        }

        out.push_back(Ability(info, pictureName));
    }
}

void
game::spec::info::describeEngine(PageContent& content, Id_t id, const ShipList& shipList, bool withCost, const PictureNamer& picNamer, const Root& root, int viewpointPlayer, afl::string::Translator& tx)
{
    if (const Engine* e = shipList.engines().get(id)) {
        util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();
        content.title = e->getName(shipList.componentNamer());
        content.pictureName = picNamer.getEnginePicture(*e);
        content.attributes.push_back(Attribute(tx("Max Efficient Warp"), Format("%d", e->getMaxEfficientWarp())));

        int esbRate = root.hostConfiguration()[HostConfiguration::AllowEngineShieldBonus]()
            ? root.hostConfiguration()[HostConfiguration::EngineShieldBonusRate](viewpointPlayer)
            : 0;
        if (esbRate != 0) {
            content.attributes.push_back(Attribute(tx("Shield Bonus"), Format(tx("%d kt"), fmt.formatNumber(e->cost().get(Cost::Money) * esbRate / 100))));
        }

        if (withCost) {
            content.attributes.push_back(Attribute(tx("Cost"), e->cost().format(tx, fmt)));
            content.attributes.push_back(Attribute(tx("Tech level"), Format("%d", e->getTechLevel())));
        }
    }
}

void
game::spec::info::describeBeam(PageContent& content, Id_t id, const ShipList& shipList, bool withCost, const PictureNamer& picNamer, const Root& root, int viewpointPlayer, afl::string::Translator& tx)
{
    if (const Beam* b = shipList.beams().get(id)) {
        util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();
        content.title = b->getName(shipList.componentNamer());
        content.pictureName = picNamer.getBeamPicture(*b);
        if (root.hostVersion().hasDeathRays()) {
            content.attributes.push_back(Attribute(tx("Type"), b->isDeathRay(root.hostVersion()) ? tx("death ray") : tx("normal")));
        }
        content.attributes.push_back(Attribute(tx("Kill"),          fmt.formatNumber(b->getKillPower())));
        content.attributes.push_back(Attribute(tx("Destroy"),       fmt.formatNumber(b->getDamagePower())));
        content.attributes.push_back(Attribute(tx("Recharge time"), Format("%ds", fmt.formatNumber(b->getRechargeTime(viewpointPlayer, root.hostVersion(), root.hostConfiguration())))));
        content.attributes.push_back(Attribute(tx("Hit"),           Format("%d%%", fmt.formatNumber(b->getHitOdds(viewpointPlayer, root.hostVersion(), root.hostConfiguration())))));

        int minesSwept = b->getNumMinesSwept(viewpointPlayer, false, root.hostConfiguration());
        int websSwept  = b->getNumMinesSwept(viewpointPlayer, true,  root.hostConfiguration());
        if (minesSwept == websSwept) {
            content.attributes.push_back(Attribute(tx("Sweep"), Format(tx("%d mines"), fmt.formatNumber(minesSwept))));
        } else {
            content.attributes.push_back(Attribute(tx("Sweep"), Format(tx("%d mines, %d webs"), fmt.formatNumber(minesSwept), fmt.formatNumber(websSwept))));
        }

        content.attributes.push_back(Attribute(tx("Mass"), Format(tx("%d kt"), fmt.formatNumber(b->getMass()))));
        if (withCost) {
            content.attributes.push_back(Attribute(tx("Cost"), b->cost().format(tx, fmt)));
            content.attributes.push_back(Attribute(tx("Tech level"), fmt.formatNumber(b->getTechLevel())));
        }
    }
}

void
game::spec::info::describeTorpedo(PageContent& content, Id_t id, const ShipList& shipList, bool withCost, const PictureNamer& picNamer, const Root& root, int viewpointPlayer, afl::string::Translator& tx)
{
    if (const TorpedoLauncher* p = shipList.launchers().get(id)) {
        util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();
        int factor = getTorpDamageScale(root);
        content.title = p->getName(shipList.componentNamer());
        content.pictureName = picNamer.getLauncherPicture(*p);
        if (root.hostVersion().hasDeathRays()) {
            content.attributes.push_back(Attribute(tx("Type"), p->isDeathRay(root.hostVersion()) ? tx("death ray") : tx("normal")));
        }
        content.attributes.push_back(Attribute(tx("Kill"),          fmt.formatNumber(factor * p->getKillPower())));
        content.attributes.push_back(Attribute(tx("Destroy"),       fmt.formatNumber(factor * p->getDamagePower())));
        content.attributes.push_back(Attribute(tx("Recharge time"), Format("%ds", fmt.formatNumber(p->getRechargeTime(viewpointPlayer, root.hostVersion(), root.hostConfiguration())))));
        content.attributes.push_back(Attribute(tx("Hit"),           Format("%d%%", fmt.formatNumber(p->getHitOdds(viewpointPlayer, root.hostVersion(), root.hostConfiguration())))));
        content.attributes.push_back(Attribute(tx("Torp Cost"),     p->torpedoCost().format(tx, fmt)));

        Cost mineCost;
        if (p->getMinefieldCost(viewpointPlayer, 1000, false, root.hostConfiguration(), mineCost)) {
            content.attributes.push_back(Attribute(tx("1000 mines"), mineCost.format(tx, fmt)));
        }

        content.attributes.push_back(Attribute(tx("Launcher Mass"), Format(tx("%d kt"), fmt.formatNumber(p->getMass()))));
        if (withCost) {
            content.attributes.push_back(Attribute(tx("Launcher Cost"), p->cost().format(tx, fmt)));
            content.attributes.push_back(Attribute(tx("Tech level"), fmt.formatNumber(p->getTechLevel())));
        }
    }
}

void
game::spec::info::describeFighter(PageContent& content, int player, const ShipList& shipList, bool withCost, const PictureNamer& picNamer, const Root& root, afl::string::Translator& tx)
{
    // ex WFighterInfo::drawContent
    // Modelled after the torpedo view, because it might overlay it:
    //   Type:    fighter
    //   Kill:
    //   Destroy:
    //   Recharge: (recharge time in seconds)
    //   Strikes:
    //   Fighter Cost:

    if (player != 0) {
        util::NumberFormatter fmt = root.userConfiguration().getNumberFormatter();
        Fighter ftr(player, root.hostConfiguration(), root.playerList(), tx);

        content.title = ftr.getName(shipList.componentNamer());
        content.pictureName = picNamer.getFighterPicture(root.hostConfiguration().getPlayerRaceNumber(player), player);
        if (root.hostVersion().hasDeathRays()) {
            // This 'if' is to make it match with the torpedoes
            content.attributes.push_back(Attribute(tx("Type"),     tx("fighter")));
        }
        content.attributes.push_back(Attribute(tx("Kill"),     fmt.formatNumber(ftr.getKillPower())));
        content.attributes.push_back(Attribute(tx("Destroy"),  fmt.formatNumber(ftr.getDamagePower())));
        content.attributes.push_back(Attribute(tx("Recharge"), util::toString(ftr.getRechargeTime(root.hostVersion(), root.hostConfiguration()), Fighter::Range_t(0, Fighter::MAX_INTERVAL), false, fmt, tx)));
        content.attributes.push_back(Attribute(tx("Strikes"),  util::toString(ftr.getNumStrikes  (root.hostVersion(), root.hostConfiguration()), Fighter::Range_t(0, Fighter::MAX_INTERVAL), false, fmt, tx)));
        if (withCost) {
            content.attributes.push_back(Attribute(tx("Fighter Cost"), ftr.cost().format(tx, fmt)));
        }
    }
}


game::spec::info::OptionalInt_t
game::spec::info::getHullAttribute(const Hull& h, FilterAttribute att)
{
    switch (att) {
     // Supported values
     case Range_CostD:            return h.cost().get(Cost::Duranium);
     case Range_CostM:            return h.cost().get(Cost::Molybdenum);
     case Range_CostMC:           return h.cost().get(Cost::Money);
     case Range_CostT:            return h.cost().get(Cost::Tritanium);
     case Range_IsArmed:          return (h.getMaxBeams() != 0 || h.getMaxLaunchers() != 0 || h.getNumBays() != 0);
     case Range_Mass:             return h.getMass();
     case Range_MaxBeams:         return h.getMaxBeams();
     case Range_MaxCargo:         return h.getMaxCargo();
     case Range_MaxCrew:          return h.getMaxCrew();
     case Range_MaxFuel:          return h.getMaxFuel();
     case Range_MaxLaunchers:     return h.getMaxLaunchers();
     case Range_NumBays:          return h.getNumBays();
     case Range_NumEngines:       return h.getNumEngines();
     case Range_Id:               return h.getId();
     case Range_Tech:             return h.getTechLevel();

     // Unsupported values
     case Range_DamagePower:      case Range_HitOdds:        case Range_KillPower:
     case Range_MaxEfficientWarp: case Range_NumMinesSwept:  case Range_RechargeTime:
     case Range_TorpCost:         case Value_Hull:           case Value_Player:
     case Value_Category:         case Value_Origin:         case ValueRange_ShipAbility:
     case Range_IsDeathRay:       case String_Name:
        break;
    }
    return afl::base::Nothing;
}

game::spec::info::OptionalInt_t
game::spec::info::getEngineAttribute(const Engine& engine, FilterAttribute att)
{
    switch (att) {
     // Supported values
     case Range_CostD:            return engine.cost().get(Cost::Duranium);
     case Range_CostM:            return engine.cost().get(Cost::Molybdenum);
     case Range_CostMC:           return engine.cost().get(Cost::Money);
     case Range_CostT:            return engine.cost().get(Cost::Tritanium);
     case Range_MaxEfficientWarp: return engine.getMaxEfficientWarp();
     case Range_Id:               return engine.getId();
     case Range_Tech:             return engine.getTechLevel();

     // Unsupported values
     case Range_DamagePower:            case Range_HitOdds:            case Range_IsArmed:
     case Range_KillPower:              case Range_Mass:               case Range_MaxBeams:
     case Range_MaxCargo:               case Range_MaxCrew:            case Range_MaxFuel:
     case Range_MaxLaunchers:           case Range_NumBays:            case Range_NumEngines:
     case Range_NumMinesSwept:          case Range_RechargeTime:       case Range_TorpCost:
     case Value_Hull:                   case Value_Player:             case Value_Category:
     case Value_Origin:                 case ValueRange_ShipAbility:   case Range_IsDeathRay:
     case String_Name:
        break;
    }
    return afl::base::Nothing;
}

game::spec::info::OptionalInt_t
game::spec::info::getBeamAttribute(const Beam& beam, FilterAttribute att, const Root& root, int viewpointPlayer)
{
    switch (att) {
     // Supported values
     case Range_CostD:         return beam.cost().get(Cost::Duranium);
     case Range_CostM:         return beam.cost().get(Cost::Molybdenum);
     case Range_CostMC:        return beam.cost().get(Cost::Money);
     case Range_CostT:         return beam.cost().get(Cost::Tritanium);
     case Range_DamagePower:   return beam.getDamagePower();
     case Range_HitOdds:       return beam.getHitOdds(viewpointPlayer, root.hostVersion(), root.hostConfiguration());
     case Range_KillPower:     return beam.getKillPower();
     case Range_Mass:          return beam.getMass();
     case Range_NumMinesSwept: return beam.getNumMinesSwept(viewpointPlayer, false, root.hostConfiguration());
     case Range_RechargeTime:  return beam.getRechargeTime(viewpointPlayer, root.hostVersion(), root.hostConfiguration());
     case Range_Id:            return beam.getId();
     case Range_IsDeathRay:    return beam.isDeathRay(root.hostVersion());
     case Range_Tech:          return beam.getTechLevel();

     // Unsupported values
     case Range_IsArmed:            case Range_MaxBeams:            case Range_MaxCargo:
     case Range_MaxCrew:            case Range_MaxEfficientWarp:    case Range_MaxFuel:
     case Range_MaxLaunchers:       case Range_NumBays:             case Range_NumEngines:
     case Range_TorpCost:           case Value_Hull:                case Value_Player:
     case Value_Category:           case Value_Origin:              case ValueRange_ShipAbility:
     case String_Name:
        break;
    }
    return afl::base::Nothing;
}

game::spec::info::OptionalInt_t
game::spec::info::getTorpedoAttribute(const TorpedoLauncher& torp, FilterAttribute att, const Root& root, int viewpointPlayer)
{
    switch (att) {
     // Supported values
     case Range_CostD:        return torp.cost().get(Cost::Duranium);
     case Range_CostM:        return torp.cost().get(Cost::Molybdenum);
     case Range_CostMC:       return torp.cost().get(Cost::Money);
     case Range_CostT:        return torp.cost().get(Cost::Tritanium);
     case Range_DamagePower:  return torp.getDamagePower() * getTorpDamageScale(root);
     case Range_HitOdds:      return torp.getHitOdds(viewpointPlayer, root.hostVersion(), root.hostConfiguration());
     case Range_KillPower:    return torp.getKillPower() * getTorpDamageScale(root);
     case Range_Mass:         return torp.getMass();
     case Range_RechargeTime: return torp.getRechargeTime(viewpointPlayer, root.hostVersion(), root.hostConfiguration());
     case Range_Tech:         return torp.getTechLevel();
     case Range_TorpCost:     return torp.torpedoCost().get(Cost::Money);
     case Range_Id:           return torp.getId();
     case Range_IsDeathRay:   return torp.isDeathRay(root.hostVersion());

     // Unsupported values
     case Range_IsArmed:       case Range_MaxBeams:           case Range_MaxCargo:
     case Range_MaxCrew:       case Range_MaxEfficientWarp:   case Range_MaxFuel:
     case Range_MaxLaunchers:  case Range_NumBays:            case Range_NumEngines:
     case Range_NumMinesSwept: case Value_Hull:               case Value_Player:
     case Value_Category:      case Value_Origin:             case ValueRange_ShipAbility:
     case String_Name:
        break;
    }
    return afl::base::Nothing;
}

game::spec::info::OptionalInt_t
game::spec::info::getFighterAttribute(const Fighter& ftr, FilterAttribute att, const Root& root)
{
    switch (att) {
     // Supported values
     case Range_CostD:        return ftr.cost().get(Cost::Duranium);
     case Range_CostM:        return ftr.cost().get(Cost::Molybdenum);
     case Range_CostMC:       return ftr.cost().get(Cost::Money);
     case Range_CostT:        return ftr.cost().get(Cost::Tritanium);
     case Range_DamagePower:  return ftr.getDamagePower();
     case Range_KillPower:    return ftr.getKillPower();
     case Range_RechargeTime: return ftr.getRechargeTime(root.hostVersion(), root.hostConfiguration()).min();

     // Unsupported values
     case Range_HitOdds:       case Range_Mass:               case Range_Tech:
     case Range_TorpCost:      case Range_Id:                 case Range_IsDeathRay:
     case Range_IsArmed:       case Range_MaxBeams:           case Range_MaxCargo:
     case Range_MaxCrew:       case Range_MaxEfficientWarp:   case Range_MaxFuel:
     case Range_MaxLaunchers:  case Range_NumBays:            case Range_NumEngines:
     case Range_NumMinesSwept: case Value_Hull:               case Value_Player:
     case Value_Category:      case Value_Origin:             case ValueRange_ShipAbility:
     case String_Name:
        break;
    }
    return afl::base::Nothing;

}
