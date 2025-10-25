/**
  *  \file game/vcr/classic/eventrecorder.cpp
  *  \brief Class game::vcr::classic::EventRecorder
  */

#include "game/vcr/classic/eventrecorder.hpp"

namespace {
    // Instruction opcodes
    enum {
        iPlaceObject,
        iUpdateTime,
        iStartFighter,
        iLandFighter,
        iKillFighter,
        iFireBeam,
        iFireTorpedo,
        iUpdateBeam,
        iUpdateLauncher,
        iMoveObject,
        iMoveFighter,
        iKillObject,
        iUpdateObject,
        iUpdateAmmo,
        iUpdateFighter,
        iSetResult,
        iRemoveAnimations
    };
}

// Constructor.
game::vcr::classic::EventRecorder::EventRecorder()
    : m_content()
{ }

// Destructor.
game::vcr::classic::EventRecorder::~EventRecorder()
{ }

// Swap content.
void
game::vcr::classic::EventRecorder::swapContent(util::StringInstructionList& content)
{
    m_content.swap(content);
}

// Replay content.
void
game::vcr::classic::EventRecorder::replay(EventListener& listener)
{
    util::StringInstructionList::Instruction_t insn;
    util::StringInstructionList::Iterator it(m_content);
    while (it.readInstruction(insn)) {
        switch (insn) {
         case iPlaceObject: {
            int32_t side, mass, shield, damage, crew, id, owner, race, picture, beamType, numBeams, torpedoType, numLaunchers, numTorpedoes, numBays, numFighters, isPlanet, position, relation;
            String_t name, ownerName, beamName, launcherName;
            if (it.readParameter(side)
                && it.readParameter(mass)
                && it.readParameter(shield)
                && it.readParameter(damage)
                && it.readParameter(crew)
                && it.readParameter(id)
                && it.readParameter(owner)
                && it.readParameter(race)
                && it.readParameter(picture)
                && it.readParameter(beamType)
                && it.readParameter(numBeams)
                && it.readParameter(torpedoType)
                && it.readParameter(numLaunchers)
                && it.readParameter(numTorpedoes)
                && it.readParameter(numBays)
                && it.readParameter(numFighters)
                && it.readParameter(isPlanet)
                && it.readStringParameter(name)
                && it.readParameter(position)
                && it.readStringParameter(ownerName)
                && it.readParameter(relation)
                && it.readStringParameter(beamName)
                && it.readStringParameter(launcherName))
            {
                UnitInfo ui;
                ui.object.setMass(mass);
                ui.object.setShield(shield);
                ui.object.setDamage(damage);
                ui.object.setCrew(crew);
                ui.object.setId(id);
                ui.object.setOwner(owner);
                ui.object.setRace(race);
                ui.object.setPicture(picture);
                ui.object.setBeamType(beamType);
                ui.object.setNumBeams(numBeams);
                ui.object.setTorpedoType(torpedoType);
                ui.object.setNumLaunchers(numLaunchers);
                ui.object.setNumTorpedoes(numTorpedoes);
                ui.object.setNumBays(numBays);
                ui.object.setNumFighters(numFighters);
                ui.object.setIsPlanet(isPlanet != 0);
                ui.object.setName(name);
                ui.position = position;
                ui.ownerName = ownerName;
                ui.relation = TeamSettings::Relation(relation);
                ui.beamName = beamName;
                ui.launcherName = launcherName;
                listener.placeObject(Side(side), ui);
            }
            break;
         }
         case iUpdateTime: {
            int32_t time, distance;
            if (it.readParameter(time) && it.readParameter(distance)) {
                listener.updateTime(time, distance);
            }
            break;
         }
         case iStartFighter: {
            int32_t side, track, position, distance, fighterDiff;
            if (it.readParameter(side) && it.readParameter(track) && it.readParameter(position) && it.readParameter(distance) && it.readParameter(fighterDiff)) {
                listener.startFighter(Side(side), track, position, distance, fighterDiff);
            }
            break;
         }
         case iLandFighter: {
            int32_t side, track, fighterDiff;
            if (it.readParameter(side) && it.readParameter(track) && it.readParameter(fighterDiff)) {
                listener.landFighter(Side(side), track, fighterDiff);
            }
            break;
         }
         case iKillFighter: {
            int32_t side, track;
            if (it.readParameter(side) && it.readParameter(track)) {
                listener.killFighter(Side(side), track);
            }
            break;
         }
         case iFireBeam: {
            int32_t side, track, target, hit, damage, kill, damageDone, crewKilled, shieldLost;
            if (it.readParameter(side)
                && it.readParameter(track)
                && it.readParameter(target)
                && it.readParameter(hit)
                && it.readParameter(damage)
                && it.readParameter(kill)
                && it.readParameter(damageDone)
                && it.readParameter(crewKilled)
                && it.readParameter(shieldLost))
            {
                HitEffect eff;
                eff.damageDone = damageDone;
                eff.crewKilled = crewKilled;
                eff.shieldLost = shieldLost;
                listener.fireBeam(Side(side), track, target, hit, damage, kill, eff);
            }
            break;
         }
         case iFireTorpedo: {
            int32_t side, hit, launcher, torpedoDiff, damageDone, crewKilled, shieldLost;
            if (it.readParameter(side)
                && it.readParameter(hit)
                && it.readParameter(launcher)
                && it.readParameter(torpedoDiff)
                && it.readParameter(damageDone)
                && it.readParameter(crewKilled)
                && it.readParameter(shieldLost))
            {
                HitEffect eff;
                eff.damageDone = damageDone;
                eff.crewKilled = crewKilled;
                eff.shieldLost = shieldLost;
                listener.fireTorpedo(Side(side), hit, launcher, torpedoDiff, eff);
            }
            break;
         }
         case iUpdateBeam: {
            int32_t side, id, value;
            if (it.readParameter(side) && it.readParameter(id) && it.readParameter(value)) {
                listener.updateBeam(Side(side), id, value);
            }
            break;
         }
         case iUpdateLauncher: {
            int32_t side, id, value;
            if (it.readParameter(side) && it.readParameter(id) && it.readParameter(value)) {
                listener.updateLauncher(Side(side), id, value);
            }
            break;
         }
         case iMoveObject: {
            int32_t side, pos;
            if (it.readParameter(side) && it.readParameter(pos)) {
                listener.moveObject(Side(side), pos);
            }
            break;
         }
         case iMoveFighter: {
            int32_t side, track, pos, distance, status;
            if (it.readParameter(side) && it.readParameter(track) && it.readParameter(pos) && it.readParameter(distance) && it.readParameter(status)) {
                listener.moveFighter(Side(side), track, pos, distance, FighterStatus(status));
            }
            break;
         }
         case iKillObject: {
            int32_t side;
            if (it.readParameter(side)) {
                listener.killObject(Side(side));
            }
            break;
         }
         case iUpdateObject: {
            int32_t side, damage, crew, shield;
            if (it.readParameter(side) && it.readParameter(damage) && it.readParameter(crew) && it.readParameter(shield)) {
                listener.updateObject(Side(side), damage, crew, shield);
            }
            break;
         }
         case iUpdateAmmo: {
            int32_t side, numTorpedoes, numFighters;
            if (it.readParameter(side) && it.readParameter(numTorpedoes) && it.readParameter(numFighters)) {
                listener.updateAmmo(Side(side), numTorpedoes, numFighters);
            }
            break;
         }
         case iUpdateFighter: {
            int32_t side, track, pos, distance, status;
            if (it.readParameter(side) && it.readParameter(track) && it.readParameter(pos) && it.readParameter(distance) && it.readParameter(status)) {
                listener.updateFighter(Side(side), track, pos, distance, FighterStatus(status));
            }
            break;
         }
         case iSetResult: {
            int32_t result;
            if (it.readParameter(result)) {
                listener.setResult(BattleResult_t::fromInteger(result));
            }
            break;
         }
         case iRemoveAnimations:
            listener.removeAnimations();
            break;
        }
    }
}

// Get approximation of size of content.
size_t
game::vcr::classic::EventRecorder::size() const
{
    return m_content.size();
}

void
game::vcr::classic::EventRecorder::placeObject(Side side, const UnitInfo& info)
{
    m_content.addInstruction(iPlaceObject)
        .addParameter(side)
        .addParameter(info.object.getMass())
        .addParameter(info.object.getShield())
        .addParameter(info.object.getDamage())
        .addParameter(info.object.getCrew())
        .addParameter(info.object.getId())
        .addParameter(info.object.getOwner())
        .addParameter(info.object.getRace())
        .addParameter(info.object.getPicture())
        .addParameter(info.object.getBeamType())
        .addParameter(info.object.getNumBeams())
        .addParameter(info.object.getTorpedoType())
        .addParameter(info.object.getNumLaunchers())
        .addParameter(info.object.getNumTorpedoes())
        .addParameter(info.object.getNumBays())
        .addParameter(info.object.getNumFighters())
        .addParameter(info.object.isPlanet())
        .addStringParameter(info.object.getName())
        .addParameter(info.position)
        .addStringParameter(info.ownerName)
        .addParameter(info.relation)
        .addStringParameter(info.beamName)
        .addStringParameter(info.launcherName);
}

void
game::vcr::classic::EventRecorder::updateTime(Time_t time, int32_t distance)
{
    m_content.addInstruction(iUpdateTime)
        .addParameter(time)
        .addParameter(distance);
}

void
game::vcr::classic::EventRecorder::startFighter(Side side, int track, int position, int distance, int fighterDiff)
{
    m_content.addInstruction(iStartFighter)
        .addParameter(side)
        .addParameter(track)
        .addParameter(position)
        .addParameter(distance)
        .addParameter(fighterDiff);
}

void
game::vcr::classic::EventRecorder::landFighter(Side side, int track, int fighterDiff)
{
    m_content.addInstruction(iLandFighter)
        .addParameter(side)
        .addParameter(track)
        .addParameter(fighterDiff);
}

void
game::vcr::classic::EventRecorder::killFighter(Side side, int track)
{
    m_content.addInstruction(iKillFighter)
        .addParameter(side)
        .addParameter(track);
}

void
game::vcr::classic::EventRecorder::fireBeam(Side side, int track, int target, int hit, int damage, int kill, const HitEffect& effect)
{
    m_content.addInstruction(iFireBeam)
        .addParameter(side)
        .addParameter(track)
        .addParameter(target)
        .addParameter(hit)
        .addParameter(damage)
        .addParameter(kill)
        .addParameter(effect.damageDone)
        .addParameter(effect.crewKilled)
        .addParameter(effect.shieldLost);
}

void
game::vcr::classic::EventRecorder::fireTorpedo(Side side, int hit, int launcher, int torpedoDiff, const HitEffect& effect)
{
    m_content.addInstruction(iFireTorpedo)
        .addParameter(side)
        .addParameter(hit)
        .addParameter(launcher)
        .addParameter(torpedoDiff)
        .addParameter(effect.damageDone)
        .addParameter(effect.crewKilled)
        .addParameter(effect.shieldLost);
}

void
game::vcr::classic::EventRecorder::updateBeam(Side side, int id, int value)
{
    m_content.addInstruction(iUpdateBeam)
        .addParameter(side)
        .addParameter(id)
        .addParameter(value);
}

void
game::vcr::classic::EventRecorder::updateLauncher(Side side, int id, int value)
{
    m_content.addInstruction(iUpdateLauncher)
        .addParameter(side)
        .addParameter(id)
        .addParameter(value);
}

void
game::vcr::classic::EventRecorder::moveObject(Side side, int position)
{
    m_content.addInstruction(iMoveObject)
        .addParameter(side)
        .addParameter(position);
}

void
game::vcr::classic::EventRecorder::moveFighter(Side side, int track, int position, int distance, FighterStatus status)
{
    m_content.addInstruction(iMoveFighter)
        .addParameter(side)
        .addParameter(track)
        .addParameter(position)
        .addParameter(distance)
        .addParameter(status);
}

void
game::vcr::classic::EventRecorder::killObject(Side side)
{
    m_content.addInstruction(iKillObject)
        .addParameter(side);
}

void
game::vcr::classic::EventRecorder::updateObject(Side side, int damage, int crew, int shield)
{
    m_content.addInstruction(iUpdateObject)
        .addParameter(side)
        .addParameter(damage)
        .addParameter(crew)
        .addParameter(shield);
}

void
game::vcr::classic::EventRecorder::updateAmmo(Side side, int numTorpedoes, int numFighters)
{
    m_content.addInstruction(iUpdateAmmo)
        .addParameter(side)
        .addParameter(numTorpedoes)
        .addParameter(numFighters);
}

void
game::vcr::classic::EventRecorder::updateFighter(Side side, int track, int position, int distance, FighterStatus status)
{
    m_content.addInstruction(iUpdateFighter)
        .addParameter(side)
        .addParameter(track)
        .addParameter(position)
        .addParameter(distance)
        .addParameter(status);
}

void
game::vcr::classic::EventRecorder::setResult(BattleResult_t result)
{
    m_content.addInstruction(iSetResult)
        .addParameter(result.toInteger());
}

void
game::vcr::classic::EventRecorder::removeAnimations()
{
    m_content.addInstruction(iRemoveAnimations);
}
