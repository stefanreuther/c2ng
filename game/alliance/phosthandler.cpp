/**
  *  \file game/alliance/phosthandler.cpp
  */

#include "game/alliance/phosthandler.hpp"
#include "afl/string/char.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/root.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"
#include "game/v3/structures.hpp"
#include "util/translation.hpp"

using afl::string::charToLower;
using game::config::HostConfiguration;
using game::v3::Command;
using game::v3::CommandContainer;
using game::v3::CommandExtra;
using game::alliance::Offer;

namespace {
    /* Player limit */
    const int NUM_PLAYERS = game::v3::structures::NUM_PLAYERS;

    /* Main */
    const char*const MAIN_ID = "phost.ally";

    /* Levels */
    const size_t NUM_LEVELS = 5;

    const char*const LEVEL_IDS[NUM_LEVELS] = {
        "phost.s",
        "phost.p",
        "phost.m",
        "phost.c",
        "phost.v",
    };

    const char*const LEVEL_NAMES[NUM_LEVELS] = {
        N_("Ship alliance"),
        N_("Planet alliance"),
        N_("Minefield alliance"),
        N_("Combat alliance"),
        N_("Vision alliance"),
    };

    const char LEVEL_LETTERS[NUM_LEVELS] = { 's', 'p', 'm', 'c', 'v' };

    /* Enemies */
    const char ENEMY_ID[] = "phost.enemy";

    /** Clear all new offers for a given offer Id.
        \param allies Alliance object
        \param id Identifier (MAIN_ID etc.) */
    void clearAll(game::alliance::Container& allies, const char* id)
    {
        if (Offer* pOffer = allies.getMutableOffer(allies.find(id))) {
            pOffer->newOffer = pOffer->oldOffer;
        }
    }

    /** Convert "add"/"drop" to an offer type.
        Used for "allies" and "enemies". */
    Offer::Type convertFromAddDrop(const String_t& name)
    {
        if (name.size() > 0 && afl::string::charToLower(name[0]) == 'a') {
            return Offer::Yes;
        } else {
            return Offer::No;
        }
    }

    /** Convert offer type to "add"/"drop". */
    inline const char* convertToAddDrop(Offer::Type type)
    {
        if (type == Offer::Yes || type == Offer::Conditional) {
            return "add";
        } else {
            return "drop";
        }
    }

    /** Process level offer ("+p") into an alliance object.
        \param allies Alliance object
        \param player Target player
        \param word word ("+p") from command */
    void processLevelOffer(game::alliance::Container& allies, int player, const String_t& word)
    {
        // Must at least have a control character and a level letter
        if (word.size() < 2) {
            return;
        }

        // Determine mode
        Offer::Type type;
        switch (word[0]) {
         case '+': type = Offer::Yes;         break;
         case '-': type = Offer::No;          break;
         case '~': type = Offer::Conditional; break;
         default:  return;
        }

        // Determine level
        for (size_t i = 0; i < NUM_LEVELS; ++i) {
            if (LEVEL_LETTERS[i] == afl::string::charToLower(word[1])) {
                if (Offer* pOffer = allies.getMutableOffer(allies.find(LEVEL_IDS[i]))) {
                    pOffer->newOffer.set(player, type);
                }
                break;
            }
        }
    }
}


game::alliance::PHostHandler::PHostHandler(int32_t version, Turn& turn, Session& session, int player)
    : m_version(version),
      m_turn(turn),
      m_session(session),
      m_player(player)
{ }

game::alliance::PHostHandler::~PHostHandler()
{ }

void
game::alliance::PHostHandler::init(Container& allies)
{
    // ex GPHostAllianceHandler::processVersion (sort-of)
    afl::base::Ptr<Root> root = m_session.getRoot();
    if (root.get() != 0) {
        if (root->hostConfiguration()[HostConfiguration::CPEnableAllies]()) {
            // Add the main alliance level
            allies.addLevel(Level(_("Alliance offer"), MAIN_ID, Level::Flags_t(Level::IsOffer)));

            // Add the sub levels
            for (size_t i = 0; i < NUM_LEVELS; ++i) {
                allies.addLevel(Level(_(LEVEL_NAMES[i]), LEVEL_IDS[i], Level::Flags_t(Level::NeedsOffer) + Level::AllowConditional));
            }
        }

        if (m_version >= MKVERSION(4,0,8) && root->hostConfiguration()[HostConfiguration::CPEnableEnemies]()) {
            // Add the enemies, if supported by host
            allies.addLevel(Level(_("Enemy"), ENEMY_ID, Level::Flags_t(Level::IsEnemy)));
        }
    }   
}

// /** Postprocess after game loading: parse commands into internal state. */
void
game::alliance::PHostHandler::postprocess(Container& allies)
{
    // ex GPHostAllianceHandler::postprocess
    // Clear everything
    clearAll(allies, MAIN_ID);
    for (size_t i = 0; i < NUM_LEVELS; ++i) {
        clearAll(allies, LEVEL_IDS[i]);
    }
    clearAll(allies, ENEMY_ID);

    // Parse commands
    if (CommandContainer* cc = CommandExtra::get(m_turn, m_player)) {
        for (CommandContainer::ConstIterator_t it = cc->begin(), e = cc->end(); it != e; ++it) {
            if (const Command* cmd = *it) {
                switch (cmd->getCommand()) {
                 case Command::phc_AddDropAlly:
                    // Id = player, Arg = "add" or "drop"
                    if (Offer* pOffer = allies.getMutableOffer(allies.find(MAIN_ID))) {
                        pOffer->newOffer.set(cmd->getId(), convertFromAddDrop(cmd->getArg()));
                    }
                    break;

                 case Command::phc_ConfigAlly: {
                    // Id = player, Arg = "+c -s ~m"
                    String_t arg = cmd->getArg();
                    do {
                        processLevelOffer(allies, cmd->getId(), afl::string::strFirst(arg, " "));
                    } while (afl::string::strRemove(arg, " "));
                    break;
                 }

                 case Command::phc_Enemies:
                    // Id = player, Arg = "add" or "drop"
                    if (Offer* pOffer = allies.getMutableOffer(allies.find(ENEMY_ID))) {
                        pOffer->newOffer.set(cmd->getId(), convertFromAddDrop(cmd->getArg()));
                    }
                    break;

                 default:
                    break;
                }
            }
        }
    }
}

// /** Process changes to alliance object: generate command messages. */
void
game::alliance::PHostHandler::handleChanges(const Container& allies)
{
    // ex GPHostAllianceHandler::handleChanges
    if (const Offer* pMainOffer = allies.getOffer(allies.find(MAIN_ID))) {
        // Allies
        CommandContainer& cc = CommandExtra::create(m_turn).create(m_player);
        for (int i = 1; i <= NUM_PLAYERS; ++i) {
            // Transmit main offer
            bool sendLevels = false;
            if (pMainOffer->oldOffer.get(i) != pMainOffer->newOffer.get(i)) {
                cc.addCommand(Command::phc_AddDropAlly, i, convertToAddDrop(pMainOffer->newOffer.get(i)));
                sendLevels = true;
            } else {
                cc.removeCommand(Command::phc_AddDropAlly, i);
            }

            // Transmit levels. We always send a complete level list if anything changes.
            // We also send a complete list if the main offer changed.
            // This is the same which PCC 1.x does; it offers some robustness against things getting out of sync.
            String_t levelStr;
            for (size_t lvl = 0; lvl < NUM_LEVELS; ++lvl) {
                if (const Offer* pLevelOffer = allies.getOffer(allies.find(LEVEL_IDS[lvl]))) {
                    // Register changes
                    if (pLevelOffer->oldOffer.get(i) != pLevelOffer->newOffer.get(i) && pLevelOffer->newOffer.get(i) != Offer::Unknown) {
                        sendLevels = true;
                    }

                    // Build command
                    if (!levelStr.empty()) {
                        levelStr += ' ';
                    }
                    switch (pLevelOffer->newOffer.get(i)) {
                     case Offer::Yes:         levelStr += '+'; break;
                     case Offer::No:          levelStr += '-'; break;
                     case Offer::Conditional: levelStr += '~'; break;
                     case Offer::Unknown:     levelStr += '-'; break;
                    }
                    levelStr += LEVEL_LETTERS[lvl];
                }
            }
            if (sendLevels) {
                cc.addCommand(Command::phc_ConfigAlly, i, levelStr);
            } else {
                cc.removeCommand(Command::phc_ConfigAlly, i);
            }
        }
    }

    if (const Offer* pEnemies = allies.getOffer(allies.find(ENEMY_ID))) {
        // Enemies
        CommandContainer& cc = CommandExtra::create(m_turn).create(m_player);
        for (int i = 1; i <= NUM_PLAYERS; ++i) {
            if (pEnemies->oldOffer.get(i) != pEnemies->newOffer.get(i)) {
                cc.addCommand(Command::phc_Enemies, i, convertToAddDrop(pEnemies->newOffer.get(i)));
            } else {
                cc.removeCommand(Command::phc_Enemies, i);
            }
        }
    }
}
