/**
  *  \file game/spec/fighter.cpp
  *  \brief Class game::spec::Fighter
  */

#include "game/spec/fighter.hpp"
#include "afl/string/format.hpp"

// Constructor.
game::spec::Fighter::Fighter(int id,
                             const game::config::HostConfiguration& config,
                             const PlayerList& players,
                             afl::string::Translator& tx)
    : Weapon(ComponentNameProvider::Fighter, id)
{
    // ex GFighter::GFighter(int id)
    setKillPower(config[config.FighterBeamKill](id));
    setDamagePower(config[config.FighterBeamExplosive](id));
    cost() = config[config.BaseFighterCost](id);    
    setName(afl::string::Format(tx("%s fighter"), players.getPlayerName(id, Player::AdjectiveName)));
    setMass(1);
    setTechLevel(1);
    setShortName(tx("Ftr"));
}
