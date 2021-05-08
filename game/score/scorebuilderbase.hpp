/**
  *  \file game/score/scorebuilderbase.hpp
  *  \brief Class game::score::ScoreBuilderBase
  */
#ifndef C2NG_GAME_SCORE_SCOREBUILDERBASE_HPP
#define C2NG_GAME_SCORE_SCOREBUILDERBASE_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "game/score/compoundscore.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/hostversion.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/teamsettings.hpp"

namespace game { namespace score {

    /** Base class for score builders.
        Manages a variant list.

        This class is intended as a base class and not for standalone usage. */
    class ScoreBuilderBase {
     public:
        /** Definition of a variant (=a possible chart). */
        struct Variant {
            String_t name;              /**< Name (human-readable, translated). */
            CompoundScore score;        /**< Definition. */
            ScoreId_t scoreId;          /**< ScoreId, if any. */
            int decay;                  /**< Decay rate (PALDecayPerTurn). */
            int32_t winLimit;           /**< Win limit if configured; otherwise, -1. */

            Variant(String_t name, CompoundScore score, ScoreId_t scoreId, int decay, int32_t winLimit)
                : name(name), score(score), scoreId(scoreId), decay(decay), winLimit(winLimit)
                { }
        };
        typedef std::vector<game::score::ScoreBuilderBase::Variant> Variants_t;

        /** Default constructor. */
        ScoreBuilderBase();

        /** Get number of available variants (possible charts).
            \return Number */
        size_t getNumVariants() const;

        /** Get variant by index.
            \param index Index [0,getNumVariants())
            \return variant; null if parameter is out of range */
        const Variant* getVariant(size_t index) const;

        /** Find variant by definition.
            \param [in]  score  Score definition to find
            \param [out] pIndex If specified, index is placed here on success
            \return variant; null if not found */
        const Variant* findVariant(const CompoundScore& score, size_t* pIndex) const;

        /** Access variants.
            \return Variant vector */
        const Variants_t& getVariants() const;

     protected:
        /** Utility class for derived classes to add single-component score entries with appropriate metadata.
            This is intended to be a very short-lived class just for initialisation:
            - make temporary instance
            - call add() to add a single-component score */
        class SingleBuilder {
         public:
            SingleBuilder(ScoreBuilderBase& parent, const TurnScoreList& scores, const TeamSettings& team, const HostVersion& host, const game::config::HostConfiguration& config);
            void add(String_t name, ScoreId_t scoreId) const;

         private:
            ScoreBuilderBase& m_parent;
            const TurnScoreList& m_scores;
            const TeamSettings& m_teamSettings;
            const HostVersion& m_hostVersion;
            const game::config::HostConfiguration& m_config;
        };
        friend class SingleBuilder;

        ~ScoreBuilderBase();
        void addVariant(String_t name, CompoundScore score, ScoreId_t scoreId, int decay, int32_t winLimit);

     private:
        Variants_t m_variants;
    };

} }

#endif
