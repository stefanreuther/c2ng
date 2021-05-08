/**
  *  \file game/score/chartbuilder.cpp
  *  \brief Class game::score::ChartBuilder
  */

#include "game/score/chartbuilder.hpp"
#include "afl/string/format.hpp"
#include "game/score/scoreid.hpp"

using util::DataTable;

game::score::ChartBuilder::ChartBuilder(const TurnScoreList& scores,
                                        const PlayerList& players,
                                        const TeamSettings& teams,
                                        const HostVersion& host,
                                        const game::config::HostConfiguration& config,
                                        afl::string::Translator& tx)
    : ScoreBuilderBase(),
      m_scores(scores),
      m_players(players),
      m_teams(teams),
      m_translator(tx),
      m_currentVariant(0),
      m_byTeam(false),
      m_cumulative(false)
{
    init(SingleBuilder(*this, scores, teams, host, config), host.isPBPGame(config));
}

void
game::score::ChartBuilder::setVariantIndex(size_t index)
{
    m_currentVariant = index;
}

void
game::score::ChartBuilder::setByTeam(bool flag)
{
    m_byTeam = flag;
}

void
game::score::ChartBuilder::setCumulativeMode(bool flag)
{
    m_cumulative = flag;
}

std::auto_ptr<util::DataTable>
game::score::ChartBuilder::build() const
{
    std::auto_ptr<DataTable> result(new DataTable());
    if (const Variant* variant = getVariant(m_currentVariant)) {
        // Build basic table
        const int firstTurn = m_scores.getFirstTurnNumber();
        if (m_byTeam) {
            // Build chart for each team
            const PlayerSet_t allPlayers = m_players.getAllPlayers();
            for (int teamNr = 1; teamNr <= MAX_PLAYERS; ++teamNr) {
                PlayerSet_t teamPlayers = m_teams.getTeamPlayers(teamNr) & allPlayers;
                if (!teamPlayers.empty()) {
                    // Add row
                    DataTable::Row& r = result->addRow(teamNr);
                    r.setName(m_teams.getTeamName(teamNr, m_translator));

                    // Fill it
                    for (size_t turnIndex = 0, numTurns = m_scores.getNumTurns(); turnIndex < numTurns; ++turnIndex) {
                        if (const TurnScore* turn = m_scores.getTurnByIndex(turnIndex)) {
                            r.set(turn->getTurnNumber() - firstTurn, variant->score.get(*turn, teamPlayers));
                        }
                    }
                }
            }
        } else {
            // Build chart for each player
            for (Player* p = m_players.getFirstPlayer(); p != 0; p = m_players.getNextPlayer(p)) {
                if (p->isReal()) {
                    // Add row
                    const int playerId = p->getId();
                    DataTable::Row& r = result->addRow(playerId);
                    r.setName(p->getName(Player::ShortName));

                    // Fill it
                    for (size_t turnIndex = 0, numTurns = m_scores.getNumTurns(); turnIndex < numTurns; ++turnIndex) {
                        if (const TurnScore* turn = m_scores.getTurnByIndex(turnIndex)) {
                            r.set(turn->getTurnNumber() - firstTurn, variant->score.get(*turn, playerId));
                        }
                    }
                }
            }
        }

        // Conver to cumulative if desired
        if (m_cumulative) {
            result->stack();
        }

        // Label the columns
        for (size_t turnIndex = 0, numTurns = m_scores.getNumTurns(); turnIndex < numTurns; ++turnIndex) {
            if (const TurnScore* turn = m_scores.getTurnByIndex(turnIndex)) {
                result->setColumnName(turn->getTurnNumber() - firstTurn, afl::string::Format(m_translator("Turn %d"), turn->getTurnNumber()));
            }
        }
    }
    return result;
}

void
game::score::ChartBuilder::init(const SingleBuilder& b, bool isPBPGame)
{
    // ex client/scr-score.cc:generateScoreTabs (part)
    // Predefined scores
    addVariant(m_translator("Score"),       CompoundScore(m_scores, CompoundScore::TimScore),   0, 0, -1);
    b.add(m_translator("Planets"),          ScoreId_Planets);
    b.add(m_translator("Freighters"),       ScoreId_Freighters);
    b.add(m_translator("Capital Ships"),    ScoreId_Capital);
    addVariant(m_translator("Total Ships"), CompoundScore(m_scores, CompoundScore::TotalShips), 0, 0, -1);
    b.add(m_translator("Bases"),            ScoreId_Bases);

    // Build points can be PBPs or PAL
    b.add(isPBPGame ? m_translator("PBPs") : m_translator("PAL"), ScoreId_BuildPoints);

    // Add remaining scores
    for (size_t i = 0, n = m_scores.getNumScores(); i < n; ++i) {
        ScoreId_t id;
        if (m_scores.getScoreByIndex(i, id)) {
            String_t name;
            if (id == ScoreId_MinesAllowed) {
                name = m_translator("Minefields Allowed");
            } else if (id == ScoreId_MinesLaid) {
                name = m_translator("Minefields Laid");
            } else if (const TurnScoreList::Description* desc = m_scores.getDescription(id)) {
                name = desc->name;
            }
            if (name.empty()) {
                name = afl::string::Format(m_translator("Score #%d"), id);
            }
            b.add(name, id);
        }
    }
}
