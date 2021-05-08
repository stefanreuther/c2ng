/**
  *  \file game/score/tablebuilder.hpp
  *  \brief Class game::score::TableBuilder
  */
#ifndef C2NG_GAME_SCORE_TABLEBUILDER_HPP
#define C2NG_GAME_SCORE_TABLEBUILDER_HPP

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

    class TableBuilder : public ScoreBuilderBase {
     public:
        /** Constructor.
            Constructs the object and determines possible variants.
            \param scores    Scores to interpret. Lifetime must exceed that of the TableBuilder.
            \param players   Player list (for chart names)
            \param teams     Team settings (for by-team mode)
            \param host      Host version (for naming of ScoreId_BuildPoints)
            \param config    Host configuration (for naming of ScoreId_BuildPoints)
            \param tx        Translator */
        TableBuilder(const TurnScoreList& scores,
                     const PlayerList& players,
                     const TeamSettings& teams,
                     const HostVersion& host,
                     const game::config::HostConfiguration& config,
                     afl::string::Translator& tx);

        /** Select by-team mode for build().
            \param flag false: build scores by player (default); true: build scores by team */
        void setByTeam(bool flag);

        /** Select turn to report in build().
            \param index Index
            \see game::score::TurnScoreList::getTurnByIndex() */
        void setTurnIndex(size_t index);

        /** Select turn pair to report differences in build().
            \param first Index of turn to display
            \param second Index of turn to subtract
            \see game::score::TurnScoreList::getTurnByIndex() */
        void setTurnDifferenceIndexes(size_t first, size_t second);

        /** Build chart according to given parameters.
            The table will contain
            - row for each player or team, with the Id being the player/team Id.
              Row names are player or team names.
            - columns containing the scores in the order as listed in getVariant().
              Column names correspond to the names in the variant list.
            - cells contain scores or score differences.
            \return Newly-allocated DataTable */
        std::auto_ptr<util::DataTable> build() const;

     private:
        const TurnScoreList& m_scores;
        const PlayerList& m_players;
        const TeamSettings& m_teams;
        afl::string::Translator& m_translator;

        bool m_byTeam;
        bool m_difference;
        size_t m_turnIndex;
        size_t m_otherTurnIndex;

        void buildTurn(util::DataTable& out, const TurnScore& in) const;
        void init(const SingleBuilder& b, bool isPBPGame);
    };

} }

#endif
