/**
  *  \file game/score/chartbuilder.hpp
  *  \brief Class game::score::ChartBuilder
  */
#ifndef C2NG_GAME_SCORE_CHARTBUILDER_HPP
#define C2NG_GAME_SCORE_CHARTBUILDER_HPP

#include <memory>
#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/playerlist.hpp"
#include "game/score/compoundscore.hpp"
#include "game/score/scorebuilderbase.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/teamsettings.hpp"
#include "util/datatable.hpp"

namespace game { namespace score {

    /** Data formatting for score charts.
        Provides a list of available charts from a TurnScoreList instance
        and the ability to format these charts into a DataTable. */
    class ChartBuilder : public ScoreBuilderBase {
     public:
        /** Constructor.
            Constructs the object and determines possible variants.
            \param scores    Scores to interpret. Lifetime must exceed that of the ChartBuilder.
            \param players   Player list (for chart names)
            \param teams     Team settings (for by-team mode)
            \param host      Host version (for naming of ScoreId_BuildPoints)
            \param config    Host configuration (for naming of ScoreId_BuildPoints)
            \param tx        Translator */
        ChartBuilder(const TurnScoreList& scores,
                     const PlayerList& players,
                     const TeamSettings& teams,
                     const HostVersion& host,
                     const game::config::HostConfiguration& config,
                     afl::string::Translator& tx);

        /** Select variant for build().
            \param index Index */
        void setVariantIndex(size_t index);

        /** Select by-team mode for build().
            \param flag false: build scores by player (default); true: build scores by team */
        void setByTeam(bool flag);

        /** Select cumulative mode for build().
            \param flag false: build individual scores (default); true: build cumulative (stacked) scores */
        void setCumulativeMode(bool flag);

        /** Build chart according to given parameters.
            The table will contain
            - rows for each player or team, with the Id being the player/team Id.
              Row name are player or team names.
            - columns with the first ciolumn corresponding to the first turn stored in the data; gaps when there are gaps in the data.
              Column names are "Turn XXX".
            \return Newly-allocated DataTable */
        std::auto_ptr<util::DataTable> build() const;

     private:
        const TurnScoreList& m_scores;
        const PlayerList& m_players;
        const TeamSettings& m_teams;
        afl::string::Translator& m_translator;

        size_t m_currentVariant;
        bool m_byTeam;
        bool m_cumulative;

        void init(const SingleBuilder& b, bool isPBPGame);
    };

} }

#endif
