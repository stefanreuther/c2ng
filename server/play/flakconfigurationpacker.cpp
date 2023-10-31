/**
  *  \file server/play/flakconfigurationpacker.cpp
  *  \brief Class server::play::FlakConfigurationPacker
  */

#include "server/play/flakconfigurationpacker.hpp"
#include "afl/base/ref.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "game/vcr/flak/configuration.hpp"

using afl::base::Ref;
using afl::data::Hash;
using afl::data::HashValue;

namespace {
    void addIntegerValue(Hash& h, const char* name, int value)
    {
        h.setNew(name, server::makeIntegerValue(value));
    }
}

server::play::FlakConfigurationPacker::FlakConfigurationPacker(const game::Root& root)
    : m_root(root)
{ }

server::Value_t*
server::play::FlakConfigurationPacker::buildValue() const
{
    const game::vcr::flak::Configuration& config = m_root.flakConfiguration();
    Ref<Hash> hv(Hash::create());

    addIntegerValue(*hv, "RatingBeamScale",            config.RatingBeamScale);
    addIntegerValue(*hv, "RatingTorpScale",            config.RatingTorpScale);
    addIntegerValue(*hv, "RatingBayScale",             config.RatingBayScale);
    addIntegerValue(*hv, "RatingMassScale",            config.RatingMassScale);
    addIntegerValue(*hv, "RatingPEBonus",              config.RatingPEBonus);
    addIntegerValue(*hv, "RatingFullAttackBonus",      config.RatingFullAttackBonus);
    addIntegerValue(*hv, "RatingRandomBonus",          config.RatingRandomBonus);
    addIntegerValue(*hv, "StartingDistanceShip",       config.StartingDistanceShip);
    addIntegerValue(*hv, "StartingDistancePlanet",     config.StartingDistancePlanet);
    addIntegerValue(*hv, "StartingDistancePerPlayer",  config.StartingDistancePerPlayer);
    addIntegerValue(*hv, "StartingDistancePerFleet",   config.StartingDistancePerFleet);
    addIntegerValue(*hv, "CompensationShipScale",      config.CompensationShipScale);
    addIntegerValue(*hv, "CompensationBeamScale",      config.CompensationBeamScale);
    addIntegerValue(*hv, "CompensationTorpScale",      config.CompensationTorpScale);
    addIntegerValue(*hv, "CompensationFighterScale",   config.CompensationFighterScale);
    addIntegerValue(*hv, "CompensationLimit",          config.CompensationLimit);
    addIntegerValue(*hv, "CompensationMass100KTScale", config.CompensationMass100KTScale);
    addIntegerValue(*hv, "CompensationAdjust",         config.CompensationAdjust);
    addIntegerValue(*hv, "CyborgDebrisRate",           config.CyborgDebrisRate);
    addIntegerValue(*hv, "MaximumFleetSize",           config.MaximumFleetSize);
    addIntegerValue(*hv, "SendUtilData",               config.SendUtilData);

    return new HashValue(hv);
}

String_t
server::play::FlakConfigurationPacker::getName() const
{
    return "flakconfig";
}
