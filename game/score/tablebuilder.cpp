/**
  *  \file game/score/tablebuilder.cpp
  *  \brief Class game::score::TableBuilder
  */

#include "game/score/tablebuilder.hpp"
#include "afl/string/format.hpp"

using util::DataTable;

game::score::TableBuilder::TableBuilder(const TurnScoreList& scores,
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
      m_byTeam(false),
      m_difference(false),
      m_turnIndex(0),
      m_otherTurnIndex(0)
{
    init(SingleBuilder(*this, scores, teams, host, config), config.isPBPGame());
}

void
game::score::TableBuilder::setByTeam(bool flag)
{
    m_byTeam = flag;
}

void
game::score::TableBuilder::setTurnIndex(size_t index)
{
    m_difference = false;
    m_turnIndex = index;
    m_otherTurnIndex = index;
}

void
game::score::TableBuilder::setTurnDifferenceIndexes(size_t first, size_t second)
{
    m_difference = true;
    m_turnIndex = first;
    m_otherTurnIndex = second;
}

std::auto_ptr<util::DataTable>
game::score::TableBuilder::build() const
{
    std::auto_ptr<DataTable> result(new DataTable());
    if (const TurnScore* turn = m_scores.getTurnByIndex(m_turnIndex)) {
        // Build regular data
        buildTurn(*result, *turn);
    }

    if (m_difference) {
        if (const TurnScore* turn = m_scores.getTurnByIndex(m_otherTurnIndex)) {
            // Build differences
            DataTable tmp;
            buildTurn(tmp, *turn);
            result->add(-1, tmp);
        }
    }

    return result;
}

void
game::score::TableBuilder::buildTurn(util::DataTable& out, const TurnScore& in) const
{
    if (m_byTeam) {
        // Build chart for each team
        const PlayerSet_t allPlayers = m_players.getAllPlayers();
        for (int teamNr = 1; teamNr <= MAX_PLAYERS; ++teamNr) {
            PlayerSet_t teamPlayers = m_teams.getTeamPlayers(teamNr) & allPlayers;
            if (!teamPlayers.empty()) {
                // Add row
                DataTable::Row& r = out.addRow(teamNr);
                r.setName(m_teams.getTeamName(teamNr, m_translator));

                // Fill it
                for (size_t i = 0, n = getNumVariants(); i < n; ++i) {
                    r.set(static_cast<int>(i), getVariant(i)->score.get(in, teamPlayers));
                }
            }
        }
    } else {
        // Build row for each player
        for (Player* p = m_players.getFirstPlayer(); p != 0; p = m_players.getNextPlayer(p)) {
            if (p->isReal()) {
                // Add row
                const int playerId = p->getId();
                DataTable::Row& r = out.addRow(playerId);
                r.setName(p->getName(Player::ShortName, m_translator));

                // Fill it
                for (size_t i = 0, n = getNumVariants(); i < n; ++i) {
                    r.set(static_cast<int>(i), getVariant(i)->score.get(in, playerId));
                }
            }
        }
    }

    // Label the rows
    for (size_t i = 0, n = getNumVariants(); i < n; ++i) {
        out.setColumnName(static_cast<int>(i), getVariant(i)->name);
    }
}

void
game::score::TableBuilder::init(const SingleBuilder& b, bool isPBPGame)
{
    // ex WScoreTable::addDefaultColumns
    // Note that this function is very similar to game::score::ChartBuilder::init(),
    // but uses shortened names.

    // Predefined scores
    addVariant(m_translator("Score"), CompoundScore(m_scores, CompoundScore::TimScore), 0, 0, -1);
    b.add(m_translator("Planets"),    ScoreId_Planets);
    b.add(m_translator("Fr."),        ScoreId_Freighters);
    b.add(m_translator("Cap."),       ScoreId_Capital);
    b.add(m_translator("Bases"),      ScoreId_Bases);

    // Build points can be PBPs or PAL
    b.add(isPBPGame ? m_translator("PBPs") : m_translator("PAL"), ScoreId_BuildPoints);

    // Add remaining scores
    for (size_t i = 0, n = m_scores.getNumScores(); i < n; ++i) {
        ScoreId_t id;
        if (m_scores.getScoreByIndex(i).get(id)) {
            String_t name;
            if (id == ScoreId_MinesAllowed) {
                name = m_translator("MF all.");
            } else if (id == ScoreId_MinesLaid) {
                name = m_translator("MFs");
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
