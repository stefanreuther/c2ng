/**
  *  \file game/score/scorebuilderbase.cpp
  *  \brief Class game::score::ScoreBuilderBase
  */

#include "game/score/scorebuilderbase.hpp"

game::score::ScoreBuilderBase::ScoreBuilderBase()
    : m_variants()
{ }

game::score::ScoreBuilderBase::~ScoreBuilderBase()
{ }

size_t
game::score::ScoreBuilderBase::getNumVariants() const
{
    return m_variants.size();
}

const game::score::ScoreBuilderBase::Variant*
game::score::ScoreBuilderBase::getVariant(size_t index) const
{
    if (index < m_variants.size()) {
        return &m_variants[index];
    } else {
        return 0;
    }
}

const game::score::ScoreBuilderBase::Variant*
game::score::ScoreBuilderBase::findVariant(const CompoundScore& score, size_t* pIndex) const
{
    for (size_t i = 0, n = m_variants.size(); i < n; ++i) {
        if (m_variants[i].score == score) {
            if (pIndex != 0) {
                *pIndex = i;
            }
            return &m_variants[i];
        }
    }
    return 0;
}

const game::score::ScoreBuilderBase::Variants_t&
game::score::ScoreBuilderBase::getVariants() const
{
    return m_variants;
}

void
game::score::ScoreBuilderBase::addVariant(String_t name, CompoundScore score, ScoreId_t scoreId, int decay, int32_t winLimit)
{
    if (score.isValid()) {
        bool known = false;
        for (size_t i = 0, n = m_variants.size(); i < n; ++i) {
            if (m_variants[i].score == score) {
                known = true;
                break;
            }
        }
        if (!known) {
            m_variants.push_back(Variant(name, score, scoreId, decay, winLimit));
        }
    }
}

game::score::ScoreBuilderBase::SingleBuilder::SingleBuilder(ScoreBuilderBase& parent, const TurnScoreList& scores, const TeamSettings& team, const HostVersion& host, const game::config::HostConfiguration& config)
    : m_parent(parent),
      m_scores(scores),
      m_teamSettings(team),
      m_hostVersion(host),
      m_config(config)
{ }

void
game::score::ScoreBuilderBase::SingleBuilder::add(String_t name, ScoreId_t scoreId) const
{
    // Determine win-limit from score definition
    int32_t winLimit = -1;
    if (const TurnScoreList::Description* desc = m_scores.getDescription(scoreId)) {
        winLimit = desc->winLimit;
    }

    // Determine decay
    int decay = 0;
    if (scoreId == ScoreId_BuildPoints && m_hostVersion.isPHost()) {
        decay = m_config[game::config::HostConfiguration::PALDecayPerTurn](m_teamSettings.getViewpointPlayer());
    }

    m_parent.addVariant(name, CompoundScore(m_scores, scoreId, 1), scoreId, decay, winLimit);
}
