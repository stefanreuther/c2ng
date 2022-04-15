/**
  *  \file game/map/info/info.hpp
  *  \brief Information Summary Rendering
  *
  *  Information summaries (Imperial Statistics) show a bunch of tables.
  *  This module provides a number of functions to render those tables, one per function.
  *  For each function, caller is epected to provide a TagNode representing an empty <table> tag.
  *  The functions will add rows/columns.
  *
  *  Most functions compute all required data on-the-fly.
  *  For some functions, you need to explicitly compute the data ahead
  *  because it is shared input between multiple functions.
  *
  *  Design thoughts:
  *  - having each function produce a single table allows future handling with these tables,
  *    e.g. export as CSV, collapse, etc.
  *  - do not peek into the produced tables to retrieve values; instead, add appropriate accessors to Universe etc.
  *  - do not peek into the TotalsInfo, StarchartInfo structures; these ought to be private for this module.
  */
#ifndef C2NG_GAME_MAP_INFO_INFO_HPP
#define C2NG_GAME_MAP_INFO_INFO_HPP

#include "afl/string/translator.hpp"
#include "game/cargospec.hpp"
#include "game/element.hpp"
#include "game/map/info/types.hpp"
#include "game/map/universe.hpp"
#include "game/playerarray.hpp"
#include "game/teamsettings.hpp"
#include "game/turn.hpp"
#include "util/numberformatter.hpp"

namespace game { namespace map { namespace info {

    class LinkBuilder;

    /*
     *  Data Acquisition
     */

    /** Ad-hoc summary information for TotalsPage.
        This structure is created using computeTotalsInfo(),
        and serves as input to renderMineralTotals(), renderColoniesTotals(), renderProductionTotals(). */
    struct TotalsInfo {
        CargoSpec availableResources;
        CargoSpec groundResources;
        CargoSpec producedResources;
        CargoSpec maxProducedResources;
        int32_t totalFactories;
        int32_t totalDefense;
        int32_t totalMines;

        TotalsInfo()
            : availableResources(), groundResources(), producedResources(), maxProducedResources(),
              totalFactories(), totalDefense(), totalMines()
            { }
    };

    /** Ad-hoc summary information for StarchartPage.
        This structure is created using computeStarchartInfo(),
        and serves as input to renderStarchartEmpireSummary(), renderStarchartForeignSummary(). */
    struct StarchartInfo {
        PlayerArray<int> numCurrentPlanets;
        PlayerArray<int> numOldPlanets;
        PlayerArray<int> numCurrentShips;
        PlayerArray<int> numOldShips;
        PlayerArray<int> numMinefields;
        int totalPlanets;
        int totalTargets;
        int numOwnMinefields, numTeamMinefields, numEnemyMinefields;
        StarchartInfo()
            : numCurrentPlanets(), numOldPlanets(), numCurrentShips(), numOldShips(), numMinefields(),
              totalPlanets(), totalTargets(),
              numOwnMinefields(), numTeamMinefields(), numEnemyMinefields()
            { }
    };

    /** Compute TotalsInfo structure.
        @param [in]  univ   Universe
        @param [in]  config Host configuration (production configuration)
        @param [in]  host   Host version (production formulas)
        @return result */
    TotalsInfo computeTotalsInfo(const Universe& univ, const game::config::HostConfiguration& config, const game::HostVersion& host);

    /** Compute StarchartInfo structure.
        @param [in]  turn   Turn
        @param [in]  teams  Team settings (viewpoint player, player relations)
        @return result */
    StarchartInfo computeStarchartInfo(const Turn& turn, const TeamSettings& teams);


    /*
     *  Rendering
     */

    /** Render unit totals (part of TotalsPage).
        Reports player's unit numbers.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator */
    void renderUnitTotals(TagNode& tab,
                          const Universe& univ,
                          util::NumberFormatter fmt,
                          afl::string::Translator& tx);

    /** Render mineral totals (part of TotalsPage).
        Reports player's available/ground mineral counts.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   t            Precomputed "totals" object
        @param [in]   shipList     Ship List (currently required for naming things; Element::getName())
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator */
    void renderMineralTotals(TagNode& tab,
                             const TotalsInfo& t,
                             const game::spec::ShipList& shipList,
                             util::NumberFormatter fmt,
                             afl::string::Translator& tx);

    /** Render colonies totals (part of TotalsPage).
        Reports player's economy totals (colonists, buildings, cash).

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   t            Precomputed "totals" object
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator */
    void renderColoniesTotals(TagNode& tab,
                              const TotalsInfo& t,
                              util::NumberFormatter fmt,
                              afl::string::Translator& tx);

    /** Render production totals (part of TotalsPage).
        Reports player's production totals for all resources.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   t            Precomputed "totals" object
        @param [in]   shipList     Ship List (currently required for naming things; Element::getName())
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator */
    void renderProductionTotals(TagNode& tab,
                                const TotalsInfo& t,
                                const game::spec::ShipList& shipList,
                                util::NumberFormatter fmt,
                                afl::string::Translator& tx);

    /** Render table of top-mineral planets.
        Builds a table with effectively three columns: planet name, total, mined amount.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   sortByTotal  true to sort by total amount, false to sort by mined amount
        @param [in]   limit        Maximum number of planets to report
        @param [in]   el           Mineral to report
        @param [in]   shipList     Ship List (currently required for naming things; Element::getName())
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderTopMineralPlanets(TagNode& tab,
                                 const Universe& univ,
                                 bool sortByTotal,
                                 size_t limit,
                                 Element::Type el,
                                 const game::spec::ShipList& shipList,
                                 util::NumberFormatter fmt,
                                 afl::string::Translator& tx,
                                 const LinkBuilder& link);

    /** Render table of top-resource planets.
        Builds a table with two: planet name, amount.
        This is therefore a slightly simpler version than renderTopMineralPlanets.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   limit        Maximum number of planets to report
        @param [in]   el           Resource to report
        @param [in]   shipList     Ship List (currently required for naming things; Element::getName())
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderTopResourcePlanets(TagNode& tab,
                                  const Universe& univ,
                                  size_t limit,
                                  Element::Type el,
                                  const game::spec::ShipList& shipList,
                                  util::NumberFormatter fmt,
                                  afl::string::Translator& tx,
                                  const LinkBuilder& link);


    /** Render number of planets (part of PlanetsPage).

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator */
    void renderPlanetNumber(TagNode& tab, const Universe& univ,
                            util::NumberFormatter fmt,
                            afl::string::Translator& tx);

    /** Render planet natives summary (part of PlanetsPage).
        Shows planets by native race or population.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   sortOrder    Sort order (Planets_SortByRace etc.)
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderPlanetNativeSummary(TagNode& tab, const Universe& univ,
                                   uint8_t sortOrder,
                                   util::NumberFormatter fmt,
                                   afl::string::Translator& tx,
                                   const LinkBuilder& link);

    /** Render planet climate summary (part of PlanetsPage).
        Shows planets by climate.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderPlanetClimateSummary(TagNode& tab, const Universe& univ,
                                    util::NumberFormatter fmt,
                                    afl::string::Translator& tx,
                                    const LinkBuilder& link);

    /** Render planet defense summary (part of PlanetsPage).
        Shows planets by defense status.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   config       Host configuration (determines DefenseForUndetectable)
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderPlanetDefenseSummary(TagNode& tab,
                                    const Universe& univ,
                                    const game::config::HostConfiguration& config,
                                    util::NumberFormatter fmt,
                                    afl::string::Translator& tx,
                                    const LinkBuilder& link);

    /** Render starbase summary (part of StarbasePage).
        Shows counts of starbases that stand out somehow.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderStarbaseSummary(TagNode& tab,
                               const Universe& univ,
                               util::NumberFormatter fmt,
                               afl::string::Translator& tx,
                               const LinkBuilder& link);

    /** Render starbase ship building summary (part of StarbasePage).
        Shows ships being built by starbases.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   sortOrder    Sort order (Ships_SortByXX)
        @param [in]   shipList     Ship list (for hulls being built)
        @param [in]   config       Host configuration (for resolving truehull)
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderStarbaseShipBuildSummary(TagNode& tab,
                                        const Universe& univ,
                                        uint8_t sortOrder,
                                        const game::spec::ShipList& shipList,
                                        const game::config::HostConfiguration& config,
                                        util::NumberFormatter fmt,
                                        afl::string::Translator& tx,
                                        const LinkBuilder& link);

    /** Render starship summary (part of StarshipPage).
        Shows ships that stand out.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   withFreighters Include freighters in list?
        @param [in]   shipScores   Ship score definitions (for resolving hullfuncs)
        @param [in]   shipList     Ship list (for hulls, hullfuncs)
        @param [in]   config       Host configuration (for resolving hullfuncs)
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderShipSummary(TagNode& tab,
                           const Universe& univ,
                           bool withFreighters,
                           const UnitScoreDefinitionList& shipScores,
                           const game::spec::ShipList& shipList,
                           const game::config::HostConfiguration& config,
                           util::NumberFormatter fmt,
                           afl::string::Translator& tx,
                           const LinkBuilder& link);

    /** Render starship experience level summary (part of StarshipPage).
        Shows ships by experience level.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   withFreighters Include freighters in list?
        @param [in]   shipScores   Ship score definitions (for experience levels and for resolving hullfuncs)
        @param [in]   config       Host configuration (for experience levels)
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderShipExperienceSummary(TagNode& tab,
                                     const Universe& univ,
                                     bool withFreighters,
                                     const UnitScoreDefinitionList& shipScores,
                                     const game::config::HostConfiguration& config,
                                     util::NumberFormatter fmt,
                                     afl::string::Translator& tx,
                                     const LinkBuilder& link);

    /** Render starship type summary (part of StarshipPage).
        Shows ships by type.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   sortOrder    Sort order (Ships_SortByXX)
        @param [in]   withFreighters Include freighters in list?
        @param [in]   shipList     Ship list (for hulls)
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderShipTypeSummary(TagNode& tab,
                               const Universe& univ,
                               uint8_t sortOrder,
                               bool withFreighters,
                               const game::spec::ShipList& shipList,
                               util::NumberFormatter fmt,
                               afl::string::Translator& tx,
                               const LinkBuilder& link);

    /** Render starchart summary, own empire (part of StarchartPage).
        Reports size and content of starchart.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   t            StarchartInfo object produced by computeStarchartInfo()
        @param [in]   univ         Universe
        @param [in]   teams        Teams (viewpoint player, player relations)
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator */
    void renderStarchartEmpireSummary(TagNode& tab,
                                      const StarchartInfo& t,
                                      const Universe& univ,
                                      const TeamSettings& teams,
                                      util::NumberFormatter fmt,
                                      afl::string::Translator& tx);

    /** Render starchart summary, foreign units (part of StarchartPage).
        Provides a summary of foreign units.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   t            StarchartInfo object produced by computeStarchartInfo()
        @param [in]   teams        Teams (viewpoint player, player relations)
        @param [in]   players      Player names
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderStarchartForeignSummary(TagNode& tab,
                                       const StarchartInfo& t,
                                       const TeamSettings& teams,
                                       const PlayerList& players,
                                       util::NumberFormatter fmt,
                                       afl::string::Translator& tx,
                                       const LinkBuilder& link);

    /** Render universal minefield friendly code (part of StarchartPage).

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   teams        Teams (viewpoint player)
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderUniversalFriendlyCode(TagNode& tab,
                                     const Universe& univ,
                                     const TeamSettings& teams,
                                     afl::string::Translator& tx,
                                     const LinkBuilder& link);

    /** Render beam weapon summary (part of WeaponsPage).
        Reports total numbers of ships with beam weapons, and weapons.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   showAll      true to show all torpedo types, even those you do not have
        @param [in]   shipList     Ship list (weapon names)
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderBeamWeaponSummary(TagNode& tab,
                                 const Universe& univ,
                                 bool showAll,
                                 const game::spec::ShipList& shipList,
                                 util::NumberFormatter fmt,
                                 afl::string::Translator& tx,
                                 const LinkBuilder& link);

    /** Render torpedo weapon summary (part of WeaponsPage).
        Reports total numbers of torpedo ships and torpedoes.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   showAll      true to show all torpedo types, even those you do not have
        @param [in]   shipList     Ship list (weapon names)
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator
        @param [in]   link         LinkBuilder */
    void renderTorpedoWeaponSummary(TagNode& tab,
                                    const Universe& univ,
                                    bool showAll,
                                    const game::spec::ShipList& shipList,
                                    util::NumberFormatter fmt,
                                    afl::string::Translator& tx,
                                    const LinkBuilder& link);

    /** Render misc weapon summary (part of WeaponsPage).
        Reports carriers and unarmed ships.

        @param [out]  tab          Output target (empty <table> tag)
        @param [in]   univ         Universe
        @param [in]   fmt          Number formatter
        @param [in]   tx           Translator */
    void renderOtherWeaponSummary(TagNode& tab,
                                  const Universe& univ,
                                  util::NumberFormatter fmt,
                                  afl::string::Translator& tx);

} } }

#endif
