/**
  *  \file game/v3/udata/parser.cpp
  *
  *  FIXME: this is a semi-direct port of PCC2's two classes, GUtilParser and GUtilMessageParser.
  *  That separation was probably for a hypothetical util.dat viewer built from the same source.
  *  The split would have to be a little different in c2ng because we do more via MessageInformation.
  *  We can probably improve a lot upon this in terms of testability, re-usability, etc.
  */

#include "game/v3/udata/parser.hpp"
#include "afl/string/format.hpp"
#include "game/alliance/offer.hpp"
#include "game/score/scoreid.hpp"
#include "game/turn.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/structures.hpp"

namespace gt = game::v3::structures;
namespace gp = game::parser;
using afl::base::fromObject;
using game::alliance::Offer;
using game::parser::MessageInformation;

namespace {
    const char*const LOG_NAME = "game.v3.udata";

    template<typename Content>
    class Eater {
     public:
        Eater(afl::base::ConstBytes_t& data)
            {
                afl::base::ConstBytes_t bite = data.split(sizeof(Content));
                if (bite.size() == sizeof(Content)) {
                    fromObject(m_content).copyFrom(bite);
                    m_ok = true;
                } else {
                    m_ok = false;
                }
            }

        operator bool() const
            { return m_ok; }
        const Content* operator->() const
            { return &m_content; }
        const Content& operator*() const
            { return m_content; }

     private:
        Content m_content;
        bool m_ok;
    };
}


game::v3::udata::Parser::Parser(Game& game,
                                int playerNr,
                                game::config::HostConfiguration& config,
                                game::spec::ModifiedHullFunctionList& modList,
                                afl::charset::Charset& cs,
                                afl::string::Translator& tx,
                                afl::sys::LogListener& log)
    : m_game(game),
      m_player(playerNr),
      m_hostConfiguration(config),
      m_modList(modList),
      m_charset(cs),
      m_translator(tx),
      m_log(log)
{ }

// Reader:
bool
game::v3::udata::Parser::handleRecord(uint16_t recordId, afl::base::ConstBytes_t data)
{
    // ex GUtilParser::process, GUtilMessageParser::parseRecord
    switch (recordId) {
     case 0:
     case 46:
        // Minefield
        // Variable size: planetId, scanReason are optional
        if (data.size() >= 14) {
            gt::Util0Minefield mf;
            fromObject(mf).copyFrom(data);

            MessageInformation info(MessageInformation::Minefield, mf.minefieldId, getTurnNumber());
            info.addValue(gp::mi_X, mf.x);
            info.addValue(gp::mi_Y, mf.y);
            info.addValue(gp::mi_Owner, mf.owner);
            info.addValue(gp::mi_MineUnits, mf.units);
            info.addValue(gp::mi_Type, mf.type);
            if (data.size() >= 18) {
                // Scan reason: PHost's reasons are only laid/swept/scanned, so they are off-by-one.
                static_assert(game::map::Minefield::MinefieldLaid == 1, "MinefieldLaid");
                static_assert(game::map::Minefield::MinefieldSwept == 2, "MinefieldSwept");
                static_assert(game::map::Minefield::MinefieldScanned == 3, "MinefieldScanned");
                info.addValue(gp::mi_MineScanReason, mf.scanReason+1);
            }
            processMessageInformation(info);
        }
        break;

     case 1:
        // Explosion
        // Variable size: name is optional
        if (data.size() >= 6) {
            // FIXME: This only updates the ship. We must generate a bang marker, too.
            gt::Util1Bang bang;
            fromObject(bang).copyFrom(data);

            MessageInformation info(MessageInformation::Ship, bang.shipId, getTurnNumber());
            info.addValue(gp::mi_Damage, 999);
            info.addValue(gp::mi_X,      bang.x);
            info.addValue(gp::mi_Y,      bang.y);
            if (data.size() >= 26) {
                info.addValue(gp::ms_Name, m_charset.decode(bang.shipName));
            }
            processMessageInformation(info);
        }
        break;

     case 2:
        // Mine hit
        // Variable size: ship name is optional
        if (data.size() >= 8) {
            gt::Util2MineHit hit;
            fromObject(hit).copyFrom(data);

            MessageInformation info(MessageInformation::Ship, hit.shipId, getTurnNumber());
            info.addValue(gp::mi_X,      hit.x);
            info.addValue(gp::mi_Y,      hit.y);
            info.addValue(gp::mi_Damage, hit.damage);
            if (data.size() >= 28) {
                info.addValue(gp::ms_Name, m_charset.decode(hit.shipName));
            }
            processMessageInformation(info);
        }
        break;

     case 3:
        // Dark Sense
        if (Eater<gt::Util3DarkSense> report = data) {
            MessageInformation info(MessageInformation::Planet, report->planetId, getTurnNumber());
            info.addValue(gp::mi_Owner,         report->owner);
            info.addValue(gp::mi_PlanetTotalN,  report->totalOre[gt::Neutronium]);
            info.addValue(gp::mi_PlanetTotalT,  report->totalOre[gt::Tritanium]);
            info.addValue(gp::mi_PlanetTotalD,  report->totalOre[gt::Duranium]);
            info.addValue(gp::mi_PlanetTotalM,  report->totalOre[gt::Molybdenum]);
            info.addValue(gp::mi_PlanetCash,    report->money);
            info.addValue(gp::mi_PlanetHasBase, report->baseFlag);
            processMessageInformation(info);
        }
        break;

     case 4:
        // Super Spy
        // Variable size: supplies are optional
        if (data.size() >= 31) {
            gt::Util4SuperSpy report;
            fromObject(report).copyFrom(data);

            MessageInformation info(MessageInformation::Planet, report.planetId, getTurnNumber());
            info.addValue(gp::mi_PlanetMines,     report.numMines);
            info.addValue(gp::mi_PlanetFactories, report.numFactories);
            info.addValue(gp::mi_PlanetDefense,   report.numDefensePosts);
            info.addValue(gp::ms_FriendlyCode,    m_charset.decode(report.friendlyCode));
            info.addValue(gp::mi_PlanetTotalN,    report.totalOre[gt::Neutronium]);
            info.addValue(gp::mi_PlanetTotalT,    report.totalOre[gt::Tritanium]);
            info.addValue(gp::mi_PlanetTotalD,    report.totalOre[gt::Duranium]);
            info.addValue(gp::mi_PlanetTotalM,    report.totalOre[gt::Molybdenum]);
            info.addValue(gp::mi_PlanetCash,      report.money);
            if (data.size() >= 35) {
                info.addValue(gp::mi_PlanetSupplies, report.supplies);
            }
            processMessageInformation(info);
        }
        break;

     case 5:
        // Planet Exploration
        if (Eater<gt::Util5Planet> report = data) {
            MessageInformation info(MessageInformation::Planet, report->planetId, getTurnNumber());
            info.addValue(gp::mi_PlanetTemperature, report->temperature);
            info.addValue(gp::mi_Owner,             report->owner);
            info.addValue(gp::mi_PlanetColonists,   report->numColonists / 100);
            info.addValue(gp::mi_PlanetHasBase,     report->baseFlag);
            processMessageInformation(info);
        }
        break;

     case 6:
        // Sensor Sweep
        if (Eater<gt::Util6SensorSweep> report = data) {
            MessageInformation info(MessageInformation::Planet, report->planetId, getTurnNumber());
            info.addValue(gp::mi_Owner,          report->owner);
            info.addValue(gp::mi_PlanetActivity, report->activity);
            processMessageInformation(info);
        }
        break;

        // TODO: TUtil7Battle

     case 8:
        // Meteor
        if (Eater<gt::Util8Meteor> report = data) {
            MessageInformation info(MessageInformation::Planet, report->planetId, getTurnNumber());
            info.addValue(gp::mi_PlanetAddedN, report->addedOre[gt::Neutronium]);
            info.addValue(gp::mi_PlanetAddedT, report->addedOre[gt::Tritanium]);
            info.addValue(gp::mi_PlanetAddedD, report->addedOre[gt::Duranium]);
            info.addValue(gp::mi_PlanetAddedM, report->addedOre[gt::Molybdenum]);
            processMessageInformation(info);
        }
        break;

     case 9:
        // Meteorite
        /* This could be handled like a Meteor, but does not provide useful information:
           it is only sent to the planet owner who knows the planet's content anyway,
           and even if we had these reports for foreign planets, the added amounts are
           usually minor compared to the existing amounts. Hence we ignore it. */
        break;

     case 10:
        // Target.
        // This produces reliable targets, so we pass a nonempty playerset.
        if (Eater<gt::ShipTarget> target = data) {
            Loader(m_charset, m_translator, m_log).addTarget(m_game.currentTurn().universe(), *target, PlayerSet_t(m_player), getTurnNumber());
        }
        break;

     case 11:
        // Allied base
        if (Eater<gt::Util11AlliedBase> report = data) {
            MessageInformation info(MessageInformation::Planet, report->baseId, getTurnNumber());
            info.addValue(gp::mi_Owner, report->owner);
            info.addValue(gp::mi_PlanetHasBase, 1);
            processMessageInformation(info);
        }
        break;

     case 12:
        /* Allied planet */
        if (Eater<gt::Util12AlliedPlanet> report = data) {
            MessageInformation info(MessageInformation::Planet, report->planetId, getTurnNumber());
            info.addValue(gp::mi_Owner,             report->owner);
            info.addValue(gp::mi_PlanetTemperature, report->temperature);
            info.addValue(gp::mi_PlanetNativeRace,  report->nativeRace);
            info.addValue(gp::mi_PlanetNativeGov,   report->nativeGovernment);
            info.addValue(gp::mi_PlanetNatives,     report->numNatives / 100);
            info.addValue(gp::mi_PlanetMinedN,      report->minedOre[gt::Neutronium]);
            info.addValue(gp::mi_PlanetMinedT,      report->minedOre[gt::Tritanium]);
            info.addValue(gp::mi_PlanetMinedD,      report->minedOre[gt::Duranium]);
            info.addValue(gp::mi_PlanetMinedM,      report->minedOre[gt::Molybdenum]);
            info.addValue(gp::mi_PlanetColonists,   report->numColonists / 100);
            info.addValue(gp::mi_PlanetSupplies,    report->supplies);
            info.addValue(gp::mi_PlanetCash,        report->money);
            processMessageInformation(info);
        }
        break;

     case 13:
        // Control
        if (data.size() >= sizeof(gt::Util13ControlMinimal)) {
            gt::Util13Control control;
            fromObject(control).copyFrom(data);
            if (m_game.currentTurn().getTimestamp() != control.base.timestamp || m_game.currentTurn().getTurnNumber() != control.base.turnNumber) {
                m_log.write(afl::sys::LogListener::Error, LOG_NAME, m_translator.translateString("util.dat is from a different turn. File will be ignored."));
                return false;
            }
            if (m_player != control.base.playerId) {
                m_log.write(afl::sys::LogListener::Error, LOG_NAME, m_translator.translateString("util.dat belongs to a different player. File will be ignored."));
                return false;
            }

            // @change PCC2 would figure out the host version here. We don't; we already did that before.

            /* Remaining items, not yet checked:
               - spec digests
               - game name */
        } else {
            m_log.write(afl::sys::LogListener::Error, LOG_NAME, m_translator.translateString("util.dat control record too short. File is possibly damaged and will be ignored."));
            return false;
        }
        break;

    //  FIXME: case 14:
    //     /* Wormhole. 4.0e adds new fields to the wormhole structure, so we must
    //        also accept structures where they are missing. */
    //     // FIXME: it would be cleaner to parse this message into our regular format,
    //     // and have the Ufo container eat that.
    //     if (data.size() >= 10) {
    //         TUtil14Wormhole wh;
    //         getPartialStructure(data, wh, size);
    //         if (size < 12)
    //             wh.ufo_id = -1;
    //         if (size < 14)
    //             wh.bidir = -1;
    //         univ.ty_ufos.addWormholeData(wh);
    //     }
    //     break;

     case 15:
        // Wormhole travel */
        if (Eater<gt::Util15WormholeTravel> report = data) {
            // The only useful information we get from this is the ship's new damage,
            // which we may not know if the ship is under remote control
            MessageInformation info(MessageInformation::Ship, report->shipId, getTurnNumber());
            info.addValue(gp::mi_Damage, report->damage);
            processMessageInformation(info);
        }
        break;

     case 16:        // Ship recycled
     case 18:        // Ship colonized
        if (Eater<gt::Util16Recycled> report = data) {
            MessageInformation info(MessageInformation::Ship, report->shipId, getTurnNumber());
            info.addValue(gp::mi_Damage, 999);
            processMessageInformation(info);
        }
        break;

     case 17:
        // Ion storm
        if (Eater<gt::Util17Storm> report = data) {
            // ex GIonStorm::addUtilData (sort of)
            MessageInformation info(MessageInformation::IonStorm, report->stormId, getTurnNumber());
            info.addValue(gp::mi_X,          report->x);
            info.addValue(gp::mi_Y,          report->y);
            info.addValue(gp::mi_IonVoltage, report->voltage);
            info.addValue(gp::mi_Heading,    report->heading);
            info.addValue(gp::mi_Speed,      report->warpFactor);
            info.addValue(gp::mi_Radius,     report->radius);
            info.addValue(gp::mi_IonStatus,  report->growthFlag);
            // Ignore stormClass; we compute that internally
            processMessageInformation(info);
        }
        break;

     case 19:
        // Ship surrendered
        if (Eater<gt::Util19Surrender> report = data) {
            // This yields two parts of information:
            // - shipId now belongs to newOwner
            // - newOwner has a base on planet baseId
            MessageInformation info1(MessageInformation::Ship, report->shipId, getTurnNumber());
            info1.addValue(gp::mi_Owner, report->newOwner);
            processMessageInformation(info1);

            MessageInformation info2(MessageInformation::Planet, report->baseId, getTurnNumber());
            info2.addValue(gp::mi_Owner, report->newOwner);
            info2.addValue(gp::mi_PlanetHasBase, 1);
            processMessageInformation(info2);
        }
        break;

        // TODO: TUtil20ShipBuilt. Can we use it?

     case 21:
        // Ship trade
        if (Eater<gt::Util21ShipGiven> report = data) {
            MessageInformation info(MessageInformation::Ship, report->shipId, getTurnNumber());
            info.addValue(gp::mi_Owner, report->newOwner);
            processMessageInformation(info);
        }
        break;

     case 22:
        // Alliances
        // Variable size: conditional flags
        if (data.size() >= 22) {
            gt::Util22Alliance allies;
            fromObject(allies).fill(0);
            fromObject(allies).copyFrom(data);
            processAlliances(allies);
        }
        break;

     case 23:
        // Bioscan
        if (Eater<gt::Util23Bioscan> report = data) {
            MessageInformation info(MessageInformation::Planet, report->planetId, getTurnNumber());
            info.addValue(gp::mi_PlanetNativeRace,  report->nativeRace);
            info.addValue(gp::mi_PlanetNatives,     report->numNatives / 100);
            info.addValue(gp::mi_PlanetTemperature, report->temperature);
            processMessageInformation(info);
        }
        break;

     case 24:
        // Glory device
        if (Eater<gt::Util24GD> report = data) {
            // FIXME: right now, this only generates information that the ship exploded. We should also generate a marker?
            MessageInformation info(MessageInformation::Ship, report->shipId, getTurnNumber());
            info.addValue(gp::mi_Damage, 999);
            processMessageInformation(info);
        }
        break;

     case 25:
        // Glory damage
        // Variable size: optonal hull type, name
        if (data.size() >= 10) {
            gt::Util25GDHit report;
            fromObject(report).copyFrom(data);

            MessageInformation info(MessageInformation::Ship, report.shipId, getTurnNumber());
            info.addValue(gp::mi_X, report.x);
            info.addValue(gp::mi_Y, report.y);
            info.addValue(gp::mi_Damage, report.damage);
            info.addValue(gp::mi_Owner, report.owner);
            if (data.size() >= 12) {
                info.addValue(gp::mi_ShipHull, report.hullType);
            }
            if (data.size() >= 32) {
                info.addValue(gp::ms_Name, m_charset.decode(report.name));
            }
            processMessageInformation(info);
        }
        break;

     case 26:
        // Boarding
        // Variable size: optionally boarding ship Id
        if (data.size() >= 6) {
            // This generates information about the new owner of shipId, and, if present, the owner of boardingShipId.
            gt::Util26Boarded report;
            fromObject(report).copyFrom(data);

            MessageInformation info(MessageInformation::Ship, report.shipId, getTurnNumber());
            info.addValue(gp::mi_Owner, report.newOwner);
            processMessageInformation(info);

            if (data.size() >= 8) {
                MessageInformation boarderInfo(MessageInformation::Ship, report.boardingShipId, getTurnNumber());
                boarderInfo.addValue(gp::mi_Owner, report.newOwner);
                processMessageInformation(boarderInfo);
            }
        }
        break;

        // TODO: 27 (old FTP)

     case 28:
        // Ground attack
        if (Eater<gt::Util28GroundCombat> report = data) {
            MessageInformation info(MessageInformation::Planet, report->planetId, getTurnNumber());
            info.addValue(gp::mi_Owner,
                          report->result == 0
                          ? report->owner
                          : report->result == 1
                          ? report->attacker
                          : 0);
            processMessageInformation(info);
        }
        break;

     case 29:
        // Mines destroy mines
        if (Eater<gt::Util29MinesExplode> report = data) {
            // This information cannot generate new minefield scans, but it can
            // invalidate old ones (coordinate mismatch) or modify existing ones.
            MessageInformation info1(MessageInformation::Minefield, report->id1, getTurnNumber());
            info1.addValue(gp::mi_X, report->x1);
            info1.addValue(gp::mi_Y, report->y1);
            info1.addValue(gp::mi_MineUnitsRemoved, report->explodedUnits);
            processMessageInformation(info1);

            MessageInformation info2(MessageInformation::Minefield, report->id2, getTurnNumber());
            info2.addValue(gp::mi_X, report->x2);
            info2.addValue(gp::mi_Y, report->y2);
            info2.addValue(gp::mi_MineUnitsRemoved, report->explodedUnits);
            processMessageInformation(info2);
        }
        break;

     case 30:
        // EOF, ignored
        break;

     case 31:
        // Mine scoop
        // Variable size: optional unitsBefore
        if (data.size() >= 10) {
            gt::Util31MineScoop report;
            fromObject(report).copyFrom(data);

            MessageInformation info(MessageInformation::Minefield, report.mineId, getTurnNumber());
            info.addValue(gp::mi_MineUnitsRemoved, report.unitsSwept);
            if (data.size() >= 14) {
                info.addValue(gp::mi_MineUnits, report.unitsBefore - report.unitsSwept);
            }
            processMessageInformation(info);
        }
        break;

     case 32:
        // Pillage
        // Variable size: optional shipOwner
        if (data.size() >= 10) {
            // Recent PHost includes the ship owner, but we cannot do anything
            // with that information (useful for player diplomacy, but not history)
            gt::Util32Pillage report;
            fromObject(report).copyFrom(data);

            MessageInformation info(MessageInformation::Planet, report.planetId, getTurnNumber());
            info.addValue(gp::mi_PlanetColonists, report.colonists);
            info.addValue(gp::mi_PlanetNatives,   report.natives);
        }
        break;

     case 33:
        // General Object
        if (Eater<gt::Util33GO> report = data) {
            MessageInformation info(MessageInformation::Ufo, report->ufoId, getTurnNumber());
            info.addValue(gp::mi_X,        report->x);
            info.addValue(gp::mi_Y,        report->y);
            info.addValue(gp::mi_UfoColor, report->color);
            info.addValue(gp::mi_Radius,   report->radius);
            info.addValue(gp::mi_Speed,    report->warpFactor);
            if (report->heading >= 0) {
                info.addValue(gp::mi_Heading, report->heading);
            }
            info.addValue(gp::ms_Name,     m_charset.decode(report->name));
            info.addValue(gp::ms_UfoInfo1, m_charset.decode(report->info1));
            info.addValue(gp::ms_UfoInfo2, m_charset.decode(report->info2));
            info.addValue(gp::mi_Type,     report->typeCode);

            if (game::map::Ufo* pUfo = m_game.currentTurn().universe().ufos().addUfo(report->ufoId, report->typeCode, report->color)) {
                pUfo->addMessageInformation(info);
                pUfo->setIsSeenThisTurn(true);
            }
        }
        break;

        // FIXME: Util34FTP

     case 35:
        // Cloak failure. Can we do anything?
        break;

     case 36:
        // Loki decloak
        // Variable size: optional beforeMovementFlag
        if (data.size() >= 8) {
            gt::Util36Loki report;
            fromObject(report).copyFrom(data);

            MessageInformation info(MessageInformation::Ship, report.shipId, getTurnNumber());
            info.addValue(gp::mi_X, report.x);
            info.addValue(gp::mi_Y, report.y);
            info.addValue(gp::mi_Owner, report.owner);
            // FIXME: what to do with 'before_movement'? Could be used to adjust the turn number.
            processMessageInformation(info);
        }
        break;

     case 37:
        // Remote
        while (Eater<gt::Util37RemoteEntry> report = data) {
            MessageInformation info(MessageInformation::Ship, report->shipId, getTurnNumber());
            info.addValue(gp::mi_ShipRemoteFlag, report->flag);
            processMessageInformation(info);
        }
        break;

     case 38:
        // PAL
        if (Eater<gt::Util38PAL> report = data) {
            MessageInformation info(MessageInformation::PlayerScore, game::score::ScoreId_BuildPoints, getTurnNumber());
            info.addScoreValue(m_player, report->total);
            processMessageInformation(info);
        }
        break;

     case 39:
        // Build queue entry
        while (Eater<gt::Util39Queue> report = data) {
            MessageInformation info(MessageInformation::Planet, report->baseId, getTurnNumber());
            info.addValue(gp::mi_BaseQueuePos, report->queuePosition);
            info.addValue(gp::mi_BaseQueuePriority, report->priority);
            processMessageInformation(info);
        }
        break;

     case 40:
        // Web drain complete
        if (Eater<gt::Util40WebDrainComplete> report = data) {
            // Web drain complete reports that the ship is out of fuel.
            // This is a little risky for ramscoopers.
            MessageInformation info(MessageInformation::Ship, report->shipId, getTurnNumber());
            info.addValue(gp::mi_ShipFuel, 0);
            processMessageInformation(info);
        }
        break;

     case 41:
        // RGA
        // Variable size: optional shipOwner
        if (data.size() >= 4) {
            gt::Util41RGA report;
            fromObject(report).copyFrom(data);

            MessageInformation info(MessageInformation::Planet, report.planetId, getTurnNumber());
            info.addValue(gp::mi_PlanetHasNatives, report.hasNativesFlag);
        }
        break;

        // TODO: gt::Util42GODestroyed

     case 43:
        // Minefield quotas
        if (Eater<gt::Util43MinefieldQuota> report = data) {
            MessageInformation quotaInfo(MessageInformation::PlayerScore, game::score::ScoreId_MinesAllowed, getTurnNumber());
            MessageInformation usedInfo(MessageInformation::PlayerScore, game::score::ScoreId_MinesLaid, getTurnNumber());
            for (int pl = 1; pl <= gt::NUM_PLAYERS; ++pl) {
                int16_t quota = report->allowed[pl-1];
                int16_t used = report->used[pl-1];
                if (quota >= 0) {
                    quotaInfo.addScoreValue(pl, quota);
                }
                if (used >= 0) {
                    usedInfo.addScoreValue(pl, used);
                }
            }
            processMessageInformation(quotaInfo);
            processMessageInformation(usedInfo);
        }
        break;

        // TODO: gt::Util44Failure

     case 45:
        // Planet trade
        if (Eater<gt::Util45PlanetGiven> report = data) {
            MessageInformation info(MessageInformation::Planet, report->planetId, getTurnNumber());
            info.addValue(gp::mi_Owner, report->newOwner);
            processMessageInformation(info);
        }
        break;

        // Case 46 handled above

     case 47:
        // Non-existant planets
        while (Eater<gt::Int16_t> pid = data) {
            if (game::map::Planet* p = m_game.currentTurn().universe().planets().get(*pid)) {
                p->setKnownToNotExist(true);
            }
        }
        break;

     case 48:
        // PAL summary
        if (Eater<gt::Util48PALSummary> report = data) {
            MessageInformation info(MessageInformation::PlayerScore, game::score::ScoreId_BuildPoints, getTurnNumber());
            for (int pl = 1; pl <= gt::NUM_PLAYERS; ++pl) {
                info.addScoreValue(pl, report->scores[pl-1]);
            }
            processMessageInformation(info);
        }
        break;

     case 49:
        // Ship score
        processScoreRecord(data, ShipScope, m_game.shipScores());
        break;

     case 50:
        // Planet score
        processScoreRecord(data, PlanetScope, m_game.planetScores());
        break;

     case 51:
        // Player score
        if (Eater<gt::Util51PlayerScore> report = data) {
            MessageInformation info(MessageInformation::PlayerScore, report->scoreId, getTurnNumber());
            info.addValue(gp::ms_Name, m_charset.decode(report->name));
            info.addValue(gp::mi_ScoreTurnLimit, report->turnLimit);
            if (report->winLimit >= 0) {
                info.addValue(gp::mi_ScoreWinLimit, report->winLimit);
            }
            for (int pl = 1; pl <= gt::NUM_PLAYERS; ++pl) {
                int32_t value = report->scores[pl-1];
                if (value >= 0) {
                    info.addScoreValue(pl, value);
                }
            }
            processMessageInformation(info);
        }
        break;

     case 52:
        // Special functions assigned to ship
        // FIXME: this relies on PHost sending the #57's before the #52's.
        // We need to process the #57's first, because otherwise getFunctionIdFromHostId() will not know what we're talking about.
        if (Eater<gt::Int16_t> id = data) {
            if (game::map::Ship* pShip = m_game.currentTurn().universe().ships().get(*id)) {
                while (Eater<gt::Int16_t> func = data) {
                    pShip->addShipSpecialFunction(m_modList.getFunctionIdFromHostId(*func));
                }
            }
        }
        break;

     case 53:
        // Single minefield explosion
        if (Eater<gt::Util53OneMineExplodes> report = data) {
            MessageInformation info(MessageInformation::Minefield, report->mineId, getTurnNumber());
            info.addValue(gp::mi_X, report->x);
            info.addValue(gp::mi_Y, report->y);
            info.addValue(gp::mi_MineUnitsRemoved, report->explodedUnits);
            processMessageInformation(info);
        }
        break;

     case 54:
        // Enemies
        if (Eater<gt::UInt16_t> enemies = data) {
            processEnemies(*enemies);
        }
        break;

     case 55:
        // Production report. Can we do anything with these?
        break;

     case 56:
        // Repair report. Can we do anything with these?
        break;

     case 57:
        // Special function definition
        if (Eater<gt::Util57Special> report = data) {
            game::spec::HullFunction func(report->basicId, ExperienceLevelSet_t::fromInteger(report->experienceMask));
            func.setHostId(report->functionId);
            m_modList.getFunctionIdFromDefinition(func);
        }
        break;

     case 58:
        // Minefield explosion.
        // FIXME: should generate an explosion marker
        break;
    }
    return true;
}

void
game::v3::udata::Parser::handleError(afl::io::Stream& in)
{
    // ex GUtilParser::failure
    // same message as FileTooShortException
    m_log.write(afl::sys::LogListener::Warn, LOG_NAME, afl::string::Format("%s: %s", in.getName(), m_translator.translateString("File too short")));
}

int
game::v3::udata::Parser::getTurnNumber() const
{
    return m_game.currentTurn().getTurnNumber();
}

void
game::v3::udata::Parser::processAlliances(const game::v3::structures::Util22Alliance& allies)
{
    // ex GPHostAllianceHandler::processAlliances
    // FIXME: this duplicates the PHost alliance level names
    static const char*const MAIN_ID = "phost.ally";
    static const uint8_t MAIN_FLAG = 0x20;
    static const size_t NUM_LEVELS = 5;
    static const char*const LEVEL_IDS[NUM_LEVELS] = { "phost.s", "phost.p", "phost.m", "phost.c", "phost.v", };
    static const uint8_t LEVEL_FLAGS[NUM_LEVELS] = { 0x01, 0x02, 0x04, 0x08, 0x10, };

    MessageInformation info(MessageInformation::Alliance, m_player, getTurnNumber());

    // Alliance offers
    Offer mainOffers;
    for (int i = 1; i <= gt::NUM_PLAYERS; ++i) {
        Offer::Type theirOffer = ((allies.offeredFrom[i-1] & MAIN_FLAG) != 0 ? Offer::Yes : Offer::No);
        Offer::Type ourOffer   = ((allies.offeredTo[i-1]   & MAIN_FLAG) != 0 ? Offer::Yes : Offer::No);
        mainOffers.theirOffer.set(i, theirOffer);
        mainOffers.oldOffer.set(i, ourOffer);
        mainOffers.newOffer.set(i, ourOffer);
    }
    info.addAllianceValue(MAIN_ID, mainOffers);

    // Level offers
    for (size_t lvl = 0; lvl < NUM_LEVELS; ++lvl) {
        Offer levelOffers;
        for (int i = 1; i <= gt::NUM_PLAYERS; ++i) {
            const uint8_t flag = LEVEL_FLAGS[lvl];
            Offer::Type theirOffer = ((allies.offeredFrom[i-1] & flag) != 0
                                      ? (allies.conditionalFrom[i-1] & flag) != 0
                                      ? Offer::Conditional
                                      : Offer::Yes
                                      : Offer::No);
            Offer::Type ourOffer   = ((allies.offeredTo[i-1] & flag) != 0
                                      ? (allies.conditionalTo[i-1] & flag) != 0
                                      ? Offer::Conditional
                                      : Offer::Yes
                                      : Offer::No);
            levelOffers.theirOffer.set(i, theirOffer);
            levelOffers.oldOffer.set(i, ourOffer);
            levelOffers.newOffer.set(i, ourOffer);
        }
        info.addAllianceValue(LEVEL_IDS[lvl], levelOffers);
    }

    processMessageInformation(info);
}

void
game::v3::udata::Parser::processEnemies(uint16_t enemies)
{
    // ex GPHostAllianceHandler::processEnemies
    // FIXME: this duplicates the PHost alliance level names
    Offer offer;
    for (int i = 1; i <= gt::NUM_PLAYERS; ++i) {
        Offer::Type what = (enemies & (1 << i)) != 0 ? Offer::Yes : Offer::No;
        offer.oldOffer.set(i, what);
        offer.newOffer.set(i, what);
    }

    MessageInformation info(MessageInformation::Alliance, m_player, getTurnNumber());
    info.addAllianceValue("phost.enemy", offer);

    processMessageInformation(info);
}

// /** Load score record.
//     \param data   Pointer to record content
//     \param size   Size of record content
//     \param max_id Maximum permitted unit Id
//     \param defs   GUnitScoreDefinitions object describing this class's unit scores
//     \param get    Function to get a unit's scores from a univers object
//     \param univ   Universe */
void
game::v3::udata::Parser::processScoreRecord(afl::base::ConstBytes_t data, Scope scope, UnitScoreDefinitionList& defs)
{
    // ex game/utilparse.cc:loadScoreRecord
    Eater<gt::Util49UnitScoreHeader> record = data;
    if (!record) {
        return;
    }

    // Build definition
    UnitScoreDefinitionList::Definition def;
    def.name = m_charset.decode(record->name);
    def.id = record->scoreType;
    def.limit = record->scoreLimit;

    // Add definition
    UnitScoreDefinitionList::Index_t index = defs.add(def);

    // Read entries
    while (Eater<gt::Util49UnitScoreEntry> entry = data) {
        UnitScoreList* p = 0;
        switch (scope) {
         case ShipScope:
            if (game::map::Ship* pShip = m_game.currentTurn().universe().ships().get(entry->id)) {
                p = &pShip->unitScores();
            }
            break;
         case PlanetScope:
            if (game::map::Planet* pPlanet = m_game.currentTurn().universe().planets().get(entry->id)) {
                p = &pPlanet->unitScores();
            }
            break;
        }
        if (p != 0) {
            p->merge(index, entry->value, static_cast<int16_t>(getTurnNumber()));
        }
    }
}


void
game::v3::udata::Parser::processMessageInformation(const game::parser::MessageInformation& info)
{
    m_game.addMessageInformation(info, m_hostConfiguration);
}
