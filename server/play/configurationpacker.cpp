/**
  *  \file server/play/configurationpacker.cpp
  *  \brief Class server::play::ConfigurationPacker
  */

#include "server/play/configurationpacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "game/actions/preconditions.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/root.hpp"

using afl::base::Ref;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using game::config::Configuration;
using game::config::HostConfiguration;
using game::spec::Cost;

namespace {
    server::Value_t* packOption(const game::config::ConfigurationOption& opt)
    {
        if (const game::config::IntegerOption* io = dynamic_cast<const game::config::IntegerOption*>(&opt)) {
            return server::makeIntegerValue((*io)());
        } else if (const game::config::GenericIntegerArrayOption* iao = dynamic_cast<const game::config::GenericIntegerArrayOption*>(&opt)) {
            Ref<Vector> vec = Vector::create();
            afl::base::Memory<const int32_t> content = iao->getArray();
            while (const int32_t* p = content.eat()) {
                vec->pushBackInteger(*p);
            }
            return new VectorValue(vec);
        } else if (const game::config::StringOption* so = dynamic_cast<const game::config::StringOption*>(&opt)) {
            return server::makeStringValue((*so)());
        } else if (const game::config::CostArrayOption* co = dynamic_cast<const game::config::CostArrayOption*>(&opt)) {
            Ref<Vector> vec = Vector::create();
            for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
                Ref<Hash> hv(Hash::create());
                const Cost& pc = (*co)(i);
                hv->setNew("T", server::makeIntegerValue(pc.get(Cost::Tritanium)));
                hv->setNew("D", server::makeIntegerValue(pc.get(Cost::Duranium)));
                hv->setNew("M", server::makeIntegerValue(pc.get(Cost::Molybdenum)));
                hv->setNew("MC", server::makeIntegerValue(pc.get(Cost::Money)));
                hv->setNew("SUPPLIES", server::makeIntegerValue(pc.get(Cost::Supplies)));
                vec->pushBackNew(new HashValue(hv));
            }
            return new VectorValue(vec);
        } else {
            // what?
            return 0;
        }
    }

    template<typename Desc>
    void addOption(Hash& hv, const Configuration& config, const Desc& desc)
    {
        String_t name = afl::string::strUCase(desc.m_name);
        if (server::Value_t* value = packOption(config[desc])) {
            hv.setNew(name.c_str(), value);
        }
    }
}


server::play::ConfigurationPacker::ConfigurationPacker(game::Session& session, int slice)
    : m_session(session),
      m_slice(slice)
{ }

server::Value_t*
server::play::ConfigurationPacker::buildValue() const
{
    // ex ServerConfigWriter::write
    const HostConfiguration& config = game::actions::mustHaveRoot(m_session).hostConfiguration();
    Ref<Hash> hv(Hash::create());

    switch (m_slice) {
     case 0:
     {
         Configuration::OptionInfo_t oi;
         Ref<Configuration::Enumerator_t> oe = config.getOptions();
         while (oe->getNextElement(oi)) {
             if (oi.second != 0) {
                 addValueNew(*hv, packOption(*oi.second), afl::string::strUCase(oi.first).c_str());
             }
         }
         break;
     }

     case 1:
        // Options that potentially affect building and taxation on planets
        addOption(*hv, config, HostConfiguration::StarbaseCost);
        addOption(*hv, config, HostConfiguration::AllowScienceMissions);
        addOption(*hv, config, HostConfiguration::ClimateLimitsPopulation);
        addOption(*hv, config, HostConfiguration::CrystalsPreferDeserts);
        addOption(*hv, config, HostConfiguration::DefenseForUndetectable);
        addOption(*hv, config, HostConfiguration::DefenseToBlockBioscan);
        addOption(*hv, config, HostConfiguration::FactoriesForDetectable);
        addOption(*hv, config, HostConfiguration::MaxColTempSlope);
        addOption(*hv, config, HostConfiguration::MaxShipsHissing);
        addOption(*hv, config, HostConfiguration::MinesForDetectable);
        addOption(*hv, config, HostConfiguration::NativeClimateDeathRate);
        addOption(*hv, config, HostConfiguration::StructureDecayOnUnowned);
        addOption(*hv, config, HostConfiguration::TransuraniumDecayRate);
        addOption(*hv, config, HostConfiguration::AllowEatingSupplies);
        addOption(*hv, config, HostConfiguration::ClimateDeathRate);
        addOption(*hv, config, HostConfiguration::ColonistTaxRate);
        addOption(*hv, config, HostConfiguration::FighterSweepRange);
        addOption(*hv, config, HostConfiguration::GroundDefenseFactor);
        addOption(*hv, config, HostConfiguration::GroundKillFactor);
        addOption(*hv, config, HostConfiguration::HissEffectRate);
        addOption(*hv, config, HostConfiguration::MaxPlanetaryIncome);
        addOption(*hv, config, HostConfiguration::MaximumDefenseOnBase);
        addOption(*hv, config, HostConfiguration::NativeTaxRate);
        addOption(*hv, config, HostConfiguration::ProductionRate);
        addOption(*hv, config, HostConfiguration::RaceGrowthRate);
        addOption(*hv, config, HostConfiguration::RaceMiningRate);
        addOption(*hv, config, HostConfiguration::StructureDecayPerTurn);
        addOption(*hv, config, HostConfiguration::TerraformRate);
        break;

     case 2:
        // Combat-related options
        addOption(*hv, config, HostConfiguration::AllowAlternativeCombat);
        addOption(*hv, config, HostConfiguration::BayLaunchInterval);
        addOption(*hv, config, HostConfiguration::BayRechargeBonus);
        addOption(*hv, config, HostConfiguration::BayRechargeRate);
        addOption(*hv, config, HostConfiguration::BeamFiringRange);
        addOption(*hv, config, HostConfiguration::BeamHitBonus);
        addOption(*hv, config, HostConfiguration::BeamHitFighterCharge);
        addOption(*hv, config, HostConfiguration::BeamHitFighterRange);
        addOption(*hv, config, HostConfiguration::BeamHitOdds);
        addOption(*hv, config, HostConfiguration::BeamHitShipCharge);
        addOption(*hv, config, HostConfiguration::BeamRechargeBonus);
        addOption(*hv, config, HostConfiguration::BeamRechargeRate);
        addOption(*hv, config, HostConfiguration::CrewKillScaling);
        addOption(*hv, config, HostConfiguration::EModBayRechargeBonus);
        addOption(*hv, config, HostConfiguration::EModBayRechargeRate);
        addOption(*hv, config, HostConfiguration::EModBeamHitBonus);
        addOption(*hv, config, HostConfiguration::EModBeamHitFighterCharge);
        addOption(*hv, config, HostConfiguration::EModBeamHitOdds);
        addOption(*hv, config, HostConfiguration::EModBeamRechargeBonus);
        addOption(*hv, config, HostConfiguration::EModBeamRechargeRate);
        addOption(*hv, config, HostConfiguration::EModCrewKillScaling);
        addOption(*hv, config, HostConfiguration::EModFighterBeamExplosive);
        addOption(*hv, config, HostConfiguration::EModFighterBeamKill);
        addOption(*hv, config, HostConfiguration::EModFighterMovementSpeed);
        addOption(*hv, config, HostConfiguration::EModHullDamageScaling);
        addOption(*hv, config, HostConfiguration::EModMaxFightersLaunched);
        addOption(*hv, config, HostConfiguration::EModShieldDamageScaling);
        addOption(*hv, config, HostConfiguration::EModShieldKillScaling);
        addOption(*hv, config, HostConfiguration::EModStrikesPerFighter);
        addOption(*hv, config, HostConfiguration::EModTorpHitBonus);
        addOption(*hv, config, HostConfiguration::EModTorpHitOdds);
        addOption(*hv, config, HostConfiguration::EModTubeRechargeBonus);
        addOption(*hv, config, HostConfiguration::EModTubeRechargeRate);
        addOption(*hv, config, HostConfiguration::ExperienceLevelNames);
        addOption(*hv, config, HostConfiguration::FighterBeamExplosive);
        addOption(*hv, config, HostConfiguration::FighterBeamKill);
        addOption(*hv, config, HostConfiguration::FighterFiringRange);
        addOption(*hv, config, HostConfiguration::FighterKillOdds);
        addOption(*hv, config, HostConfiguration::FighterMovementSpeed);
        addOption(*hv, config, HostConfiguration::FireOnAttackFighters);
        addOption(*hv, config, HostConfiguration::HullDamageScaling);
        addOption(*hv, config, HostConfiguration::MaxFightersLaunched);
        addOption(*hv, config, HostConfiguration::NumExperienceLevels);
        addOption(*hv, config, HostConfiguration::PlayerRace);
        addOption(*hv, config, HostConfiguration::ShieldDamageScaling);
        addOption(*hv, config, HostConfiguration::ShieldKillScaling);
        addOption(*hv, config, HostConfiguration::ShipMovementSpeed);
        addOption(*hv, config, HostConfiguration::StandoffDistance);
        addOption(*hv, config, HostConfiguration::StrikesPerFighter);
        addOption(*hv, config, HostConfiguration::TorpFiringRange);
        addOption(*hv, config, HostConfiguration::TorpHitBonus);
        addOption(*hv, config, HostConfiguration::TorpHitOdds);
        addOption(*hv, config, HostConfiguration::TubeRechargeBonus);
        addOption(*hv, config, HostConfiguration::TubeRechargeRate);
        break;

     case 3:
        addOption(*hv, config, HostConfiguration::BaseFighterCost);
        addOption(*hv, config, HostConfiguration::BaseTechCost);
        addOption(*hv, config, HostConfiguration::MaximumFightersOnBase);
        addOption(*hv, config, HostConfiguration::MaximumDefenseOnBase);
        break;
    }

    return new HashValue(hv);
}

String_t
server::play::ConfigurationPacker::getName() const
{
    return afl::string::Format("cfg%d", m_slice);
}
