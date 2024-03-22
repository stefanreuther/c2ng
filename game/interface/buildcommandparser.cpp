/**
  *  \file game/interface/buildcommandparser.cpp
  *  \brief Class game::interface::BuildCommandParser
  */

#include "game/interface/buildcommandparser.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/string/format.hpp"
#include "game/actions/buildparts.hpp"
#include "game/actions/buildship.hpp"
#include "game/actions/buildstarbase.hpp"
#include "game/actions/buildstructures.hpp"
#include "game/exception.hpp"
#include "game/interface/planetmethod.hpp"
#include "game/map/planetstorage.hpp"
#include "interpreter/taskeditor.hpp"
#include "interpreter/values.hpp"

using afl::string::Format;
using game::map::PlanetStorage;
using game::actions::BuildParts;
using game::actions::BuildShip;
using game::actions::BuildStarbase;
using game::actions::BuildStructures;

namespace {
    void renderAmount(afl::data::StringList_t& info, int n, int todo, int added, afl::string::Translator& tx)
    {
        // We might be half-way in a build action
        if (todo != n) {
            info.push_back(Format(tx("To build: %d/%d"), todo, n));
        } else {
            info.push_back(Format(tx("To build: %d"), n));
        }

        // - We might exceed supported amount (which means command will never finish)
        if (added != todo) {
            info.push_back(Format(tx("Only %d more supported!"), added));
        }
    }
}

game::interface::BuildCommandParser::BuildCommandParser(game::map::Planet& pl,
                                                        game::spec::ShipList& shipList,
                                                        Root& root,
                                                        afl::string::Translator& tx)
    : m_planet(pl),
      m_shipList(shipList),
      m_root(root),
      m_translator(tx),
      m_limit(),
      m_result()
{ }

// Set limit for commands that build multiple items (e.g. "BuildFactoriesWait").
void
game::interface::BuildCommandParser::setLimit(int n)
{
    m_limit = n;
}

// Check task for applicable limit.
void
game::interface::BuildCommandParser::loadLimit(const interpreter::TaskEditor& editor, size_t pc)
{
    int v = 0;
    if (editor.getPC() == pc && editor.isInSubroutineCall()) {
        std::auto_ptr<afl::data::Value> value(editor.process().getVariable("BUILD.REMAINDER"));
        if (afl::data::ScalarValue* sv = dynamic_cast<afl::data::ScalarValue*>(value.get())) {
            v = sv->getValue();
        }
    }
    setLimit(v);
}

// Get result.
std::auto_ptr<game::interface::BuildCommandParser::Result>
game::interface::BuildCommandParser::getResult()
{
    return m_result;
}

// TaskPredictor:
bool
game::interface::BuildCommandParser::predictInstruction(const String_t& name, interpreter::Arguments& args)
{
    try {
        if (name == "BUILDSHIP" || name == "ENQUEUESHIP") {
            handleBuildShip(args);
        } else if (name == "BUILDBASE" || name == "BUILDBASEWAIT") {
            handleBuildBase(args);
        } else if (name == "BUILDDEFENSE" || name == "BUILDDEFENSEWAIT") {
            handleBuildStructure(args, DefenseBuilding);
        } else if (name == "BUILDFACTORIES" || name == "BUILDFACTORIESWAIT") {
            handleBuildStructure(args, FactoryBuilding);
        } else if (name == "BUILDBASEDEFENSE" || name == "BUILDBASEDEFENSEWAIT") {
            handleBuildStructure(args, BaseDefenseBuilding);
        } else if (name == "BUILDMINES" || name == "BUILDMINESWAIT") {
            handleBuildStructure(args, MineBuilding);
        } else if (name == "BUILDENGINES" || name == "BUILDENGINESWAIT") {
            handleBuildParts(args, EngineTech);
        } else if (name == "BUILDHULLS" || name == "BUILDHULLSWAIT") {
            handleBuildParts(args, HullTech);
        } else if (name == "BUILDBEAMS" || name == "BUILDBEAMSWAIT") {
            handleBuildParts(args, BeamTech);
        } else if (name == "BUILDLAUNCHERS" || name == "BUILDLAUNCHERSWAIT") {
            handleBuildParts(args, TorpedoTech);
        }
    }
    catch (Exception&) {
        // TaskPredictor only guards against interpreter::Error; let's also guard against game::Exception.
    }
    return true;
}

void
game::interface::BuildCommandParser::handleBuildShip(interpreter::Arguments& args)
{
    afl::base::Optional<ShipBuildOrder> order = parseBuildShipCommand(args, m_shipList);
    if (const ShipBuildOrder* p = order.get()) {
        // Action
        PlanetStorage container(m_planet, m_root.hostConfiguration());
        BuildShip action(m_planet, container, m_shipList, m_root);
        action.setUsePartsFromStorage(false);
        action.setBuildOrder(*p);

        // Result
        m_result.reset(new Result());
        p->describe(m_result->info, m_shipList, m_translator);
        m_result->cost = action.costAction().getCost();
        m_result->missingAmount = action.costAction().getMissingAmountAsCost();
        m_result->type = OrderType_Ship;
    }
}

void
game::interface::BuildCommandParser::handleBuildBase(interpreter::Arguments& args)
{
    // Only handle "BuildBase" or "BuildBase 1" commands (not "BuildBase 0")
    args.checkArgumentCount(0, 1);
    afl::data::Value* p = args.getNext();
    if (p == 0 || interpreter::getBooleanValue(p) > 0) {
        // Action
        PlanetStorage container(m_planet, m_root.hostConfiguration());
        BuildStarbase action(m_planet, container, true, m_root.hostConfiguration());

        // Result
        m_result.reset(new Result());
        m_result->info.push_back(m_translator("Starbase"));
        m_result->cost = action.costAction().getCost();
        m_result->missingAmount = action.costAction().getMissingAmountAsCost();
    }
}

void
game::interface::BuildCommandParser::handleBuildStructure(interpreter::Arguments& args, PlanetaryBuilding type)
{
    args.checkArgumentCount(1);
    int32_t n;
    if (!interpreter::checkIntegerArg(n, args.getNext(), 0, MAX_NUMBER)) {
        return;
    }

    const int todo = getLimitedAmount(n);;
    if (todo > 0) {
        // Action
        PlanetStorage container(m_planet, m_root.hostConfiguration());
        BuildStructures action(m_planet, container, m_root.hostConfiguration());

        // Result
        const int added = action.add(type, todo, true);
        m_result.reset(new Result());
        m_result->info.push_back(m_translator(BuildStructures::describe(type).untranslatedBuildingName));
        renderAmount(m_result->info, n, todo, added, m_translator);

        // Cost. Note that this will be partial cost if we're not supporting enough.
        m_result->cost = action.costAction().getCost();
        m_result->missingAmount = action.costAction().getMissingAmountAsCost();
    }
}

void
game::interface::BuildCommandParser::handleBuildParts(interpreter::Arguments& args, TechLevel area)
{
    // Parse args
    // (Ignore arg #3, which is the optional "N" flag)
    args.checkArgumentCount(2, 3);
    int32_t type, amount;
    if (!interpreter::checkIntegerArg(type, args.getNext(), 0, MAX_NUMBER)) {
        return;
    }
    if (!interpreter::checkIntegerArg(amount, args.getNext(), -MAX_NUMBER, +MAX_NUMBER)) {
        return;
    }

    // Validate
    const game::spec::Component* comp = 0;
    int slot = type;
    switch (area) {
     case HullTech:
        slot = m_shipList.hullAssignments().getIndexFromHull(m_root.hostConfiguration(), m_planet.getOwner().orElse(0), type);
        comp = m_shipList.hulls().get(type);
        break;

     case EngineTech:
        comp = m_shipList.engines().get(type);
        break;

     case BeamTech:
        comp = m_shipList.beams().get(type);
        break;

     case TorpedoTech:
        comp = m_shipList.launchers().get(type);
        break;
    }

    const int todo = getLimitedAmount(amount);
    if (slot != 0 && comp != 0 && todo > 0) {
        // Action
        PlanetStorage container(m_planet, m_root.hostConfiguration());
        BuildParts action(m_planet, container, m_shipList, m_root);

        // Result
        const int added = action.add(area, slot, todo, true);
        m_result.reset(new Result());
        m_result->info.push_back(comp->getName(m_shipList.componentNamer()));
        renderAmount(m_result->info, amount, todo, added, m_translator);
        m_result->cost = action.costAction().getCost();
        m_result->missingAmount = action.costAction().getMissingAmountAsCost();
    }
}

int
game::interface::BuildCommandParser::getLimitedAmount(int requested) const
{
    if (m_limit > 0 && m_limit < requested) {
        return m_limit;
    } else {
        return requested;
    }
}
