/**
  *  \file game/alliance/hosthandler.cpp
  */

#include "game/alliance/hosthandler.hpp"
#include "afl/string/char.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"
#include "game/v3/structures.hpp"
#include "util/string.hpp"

using game::v3::Command;
using game::v3::CommandContainer;
using game::v3::CommandExtra;

namespace {
    /* Player limit */
    const int NUM_PLAYERS = game::v3::structures::NUM_PLAYERS;

    const char ALLIANCE_ID[] = "thost.ally";
    const char STRONG_ID[] = "thost.ff";
}

game::alliance::HostHandler::HostHandler(int32_t version, Turn& turn, int player)
    : m_version(version),
      m_turn(turn),
      m_player(player)
{ }

game::alliance::HostHandler::~HostHandler()
{ }

void
game::alliance::HostHandler::init(Container& allies, afl::string::Translator& tx)
{
    // ex GTHostAllianceHandler::processVersion
    // We pretend all host versions have alliances.
    // This is the same as PCC 1.x.
    allies.addLevel(Level(tx("Standard alliance"), ALLIANCE_ID, Level::Flags_t(Level::IsOffer)));
    if (m_version == 0 || m_version >= MKVERSION(3,22,39)) {
        allies.addLevel(Level(tx("Vision alliance"), STRONG_ID, Level::Flags_t(Level::NeedsOffer)));
    }
}

void
game::alliance::HostHandler::postprocess(Container& allies)
{
    // ex GTHostAllianceHandler::postprocess
    // Find offers
    Offer* pAlliance = allies.getMutableOffer(allies.find(ALLIANCE_ID));
    Offer* pStrong   = allies.getMutableOffer(allies.find(STRONG_ID));

    // Reset to defaults
    if (pAlliance) {
        pAlliance->newOffer = pAlliance->oldOffer;
    }
    if (pStrong) {
        pStrong->newOffer = pStrong->oldOffer;
    }

    // Check the command messages
    if (CommandContainer* cc = CommandExtra::get(m_turn, m_player)) {
        if (const Command* cmd = cc->getCommand(Command::phc_TAlliance, 0)) {
            const String_t& arg = cmd->getArg();
            for (size_t i = 0; i+2 < arg.size(); i += 3) {
                // Determine race
                int player = 0;
                if (util::parsePlayerCharacter(arg[i+2], player) && player > 0 && player <= NUM_PLAYERS) {
                    // Set result
                    if (pAlliance) {
                        pAlliance->newOffer.set(player, arg[i] == 'e' ? Offer::No : Offer::Yes);
                    }
                    if (pStrong) {
                        pStrong->newOffer.set(player, arg[i] == 'F' ? Offer::Yes : Offer::No);
                    }
                }
            }
        }
    }
}

void
game::alliance::HostHandler::handleChanges(const Container& allies)
{
    // ex GTHostAllianceHandler::handleChanges
    const Offer* pAlliance = allies.getOffer(allies.find(ALLIANCE_ID));
    const Offer* pStrong   = allies.getOffer(allies.find(STRONG_ID));

    // Build fcode list
    String_t codes;
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        bool oldAlliance = pAlliance != 0 && Offer::isOffer(pAlliance->oldOffer.get(i));
        bool newAlliance = pAlliance != 0 && Offer::isOffer(pAlliance->newOffer.get(i));
        bool oldStrong   = pStrong   != 0 && Offer::isOffer(pStrong->oldOffer.get(i));
        bool newStrong   = pStrong   != 0 && Offer::isOffer(pStrong->newOffer.get(i));
        
        if (oldAlliance != newAlliance || (newAlliance && oldStrong != newStrong)) {
            char code = (!newAlliance ? 'e' : newStrong ? 'F' : 'f');
            codes += code;
            codes += code;
            codes += afl::string::charToLower(PlayerList::getCharacterFromPlayer(i));
        }
    }

    // Make the command
    CommandContainer& cc = CommandExtra::create(m_turn).create(m_player);
    if (codes.empty()) {
        cc.removeCommand(Command::phc_TAlliance, 0);
    } else {
        cc.addCommand(Command::phc_TAlliance, 0, codes);
    }
}
