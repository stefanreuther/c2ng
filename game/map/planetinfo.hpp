/**
  *  \file game/map/planetinfo.hpp
  *  \brief Functions to obtain information about planets
  */
#ifndef C2NG_GAME_MAP_PLANETINFO_HPP
#define C2NG_GAME_MAP_PLANETINFO_HPP

#include "afl/io/xml/node.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/element.hpp"
#include "game/hostversion.hpp"
#include "game/map/planeteffectors.hpp"
#include "game/playerarray.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/types.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace map {

    class Planet;
    class Universe;

    /*
     *  Minerals
     */

    const int MAX_MINING_DURATION = 30;

    /** Information about a mineral.
        All values can be unknown (not set for the Property values; empty for the strings). */
    struct PlanetMineralInfo {
        // Overall status
        enum Status {                             ///< Status.
            Unknown,                              ///< Nothing known about this mineral.
            Scanned,                              ///< Planet was scanned; information might be outdated.
            Reliable                              ///< Planet is reliably known.
        };
        Status status;                            ///< Status.

        // Age
        NegativeProperty_t age;                   ///< Age (0=current, >0=age in turns) if known.
        String_t ageLabel;                        ///< Age formatted as user-friendly string.

        // Amounts
        LongProperty_t minedAmount;               ///< Mined (available) amount. \see Planet::getCargo().
        LongProperty_t groundAmount;              ///< Ground amount. \see Planet::getOreGround().
        IntegerProperty_t density;                ///< Density. \see Planet::getOreDensity().

        String_t groundSummary;                   ///< Summary of ground amount, e.g.\ "very common".
        String_t densitySummary;                  ///< Summary of density, e.g.\ "dispersed".

        // Mining
        IntegerProperty_t miningPerTurn;          ///< Extraction rate per turn.
        IntegerProperty_t miningDuration;         ///< Time to mineral exhaustion in turns, capped at MAX_MINING_DURATION.

        PlanetMineralInfo()
            : status(Unknown),
              age(), ageLabel(),
              minedAmount(), groundAmount(), density(),
              groundSummary(), densitySummary(),
              miningPerTurn(), miningDuration()
            { }
    };

    /** Information about ships unloading clans.
        This information can be provided to alter the textual analysis for the planet
        (ground combat prediction). */
    struct UnloadInfo {
        int32_t hostileUnload;                    ///< Number of clans that are attacking.
        int32_t friendlyUnload;                   ///< Number of clans beaming down friendly (happens with Remote Control).
        bool hostileUnloadIsAssault;              ///< true if hostile unload triggers Imperial Assault.
        bool hostileUnloadIsAssumed;              ///< true if hostile unload is an assumption.

        UnloadInfo()
            : hostileUnload(0),
              friendlyUnload(0),
              hostileUnloadIsAssault(false),
              hostileUnloadIsAssumed(false)
            { }
    };

    /** Ground defense information. */
    struct GroundDefenseInfo {
        PlayerArray<int32_t> strength;            ///< Strength of this race.
        PlayerArray<String_t> name;               ///< Name of race. Empty if player does not exist.
        int defender;                             ///< Player number of defending race.
        bool isPlayable;                          ///< true if this planet is being played.

        GroundDefenseInfo()
            : strength(),
              name(),
              defender(0),
              isPlayable(false)
            { }
    };

    /** Defense textual information. */
    struct DefenseEffectInfo {
        String_t name;                            ///< Name of item.
        int nextAt;                               ///< Number of additional defense posts needed for improvement. 0 if maximum reached.
        bool isAchievable;                        ///< true if nextAt is currently achievable.
        bool isDetail;                            ///< true if this is a detail to the previous item (shown indented).

        DefenseEffectInfo()
            : name(), nextAt(0), isAchievable(false), isDetail(false)
            { }

        DefenseEffectInfo(const String_t& name, int nextAt, bool isAchievable, bool isDetail)
            : name(name), nextAt(nextAt), isAchievable(isAchievable), isDetail(isDetail)
            { }
    };
    typedef std::vector<DefenseEffectInfo> DefenseEffectInfos_t;

    /** Maximum number of lines produced by describePlanetDefenseEffects.
        - 2x beams (type, count)
        - 2x fighters (bays, count)
        - 3x torps (launchers, type, count)
        - 2x fighter resistance (shield, damage) */
    const size_t MAX_DEFENSE_EFFECT_LINES = 9;

    /** Retrieve information about minerals on a planet.
        \param pl            Planet
        \param ele           Mineral to obtain information for (must be one of the minerals;
                             result will be meaningless for other element types)
        \param turnNr        Turn number (used to provide age information)
        \param config        Host configuration (used for production rates, player/race mapping)
        \param host          Host version (used for formulas)
        \param mineOverride  Override number of mines on planet for prediction
        \param tx            Translator
        \return PlanetMineralInfo */
    PlanetMineralInfo packPlanetMineralInfo(const Planet& pl, Element::Type ele,
                                            int turnNr,
                                            const game::config::HostConfiguration& config, const HostVersion& host,
                                            IntegerProperty_t mineOverride,
                                            afl::string::Translator& tx);

    /** Retrieve textual information about planet climate.
        This function describes the climate in textual form.
        It appends XML DOM nodes to a node list, which can eventually be rendered into a ui::rich::Document.

        \param [in,out] nodes  New nodes appended here
        \param pl              Planet
        \param turnNr          Turn number (used to provide age information)
        \param root            Root (used for host configuration, version, user configuration)
        \param viewpointPlayer Viewpoint player
        \param tx              Translator */
    void describePlanetClimate(afl::io::xml::Nodes_t& nodes,
                               const Planet& pl,
                               int turnNr,
                               const Root& root,
                               int viewpointPlayer,
                               afl::string::Translator& tx);

    /** Retrieve textual information about planet natives.
        This function describes the native population in textual form.
        It appends XML DOM nodes to a node list, which can eventually be rendered into a ui::rich::Document.

        \param [in,out] nodes  New nodes appended here
        \param pl              Planet
        \param turnNr          Turn number (used to provide age information)
        \param root            Root (used for host configuration, version, user configuration)
        \param viewpointPlayer Viewpoint player
        \param unload          Unload information (used for assimilation prediction)
        \param tx              Translator */
    void describePlanetNatives(afl::io::xml::Nodes_t& nodes,
                               const Planet& pl,
                               int turnNr,
                               const Root& root,
                               int viewpointPlayer,
                               const UnloadInfo& unload,
                               afl::string::Translator& tx);

    /** Retrieve textual information about planet colony.
        This function describes the colonist population and economy in textual form.
        It appends XML DOM nodes to a node list, which can eventually be rendered into a ui::rich::Document.

        \param [in,out] nodes  New nodes appended here
        \param pl              Planet
        \param turnNr          Turn number (used to provide age information)
        \param root            Root (used for host configuration, version, user configuration)
        \param viewpointPlayer Viewpoint player
        \param unload          Unload information (used for ground combat prediction)
        \param tx              Translator */
    void describePlanetColony(afl::io::xml::Nodes_t& nodes,
                              const Planet& pl,
                              int turnNr,
                              const Root& root,
                              int viewpointPlayer,
                              const UnloadInfo& unload,
                              afl::string::Translator& tx);

    /** Retrieve textual information about planet building effects.
        This function describe the effects on sensor visibility and taxation.

        \param [in,out] nodes  New nodes appended here
        \param pl              Planet. To describe a hypothetical situation, make a copy of the real planet and modify it.
        \param root            Root (used for host configuration, version, user configuration)
        \param tx              Translator */
    void describePlanetBuildingEffects(afl::io::xml::Nodes_t& nodes,
                                       const Planet& pl,
                                       const Root& root,
                                       afl::string::Translator& tx);

    /** Retrieve textual information about planet defense effects.
        This function describes the effects of planet and starbase defense.

        \param [in,out] result Result appended here
        \param pl              Planet. To describe a hypothetical situation, make a copy of the real planet and modify it.
        \param root            Root (used for host configuration, version, user configuration)
        \param shipList        Ship list (used for weapon names)
        \param planetScores    Planet score definitions (used for experience)
        \param tx              Translator */
    void describePlanetDefenseEffects(DefenseEffectInfos_t& result,
                                      const Planet& pl,
                                      const Root& root,
                                      const game::spec::ShipList& shipList,
                                      const UnitScoreDefinitionList& planetScores,
                                      afl::string::Translator& tx);

    /** Prepare unload information for a planet.
        This function produces an UnloadInfo structure that can be given (modified or original) to
        describePlanetNatives, describePlanetColony.

        \param univ              Universe
        \param pid               Planet Id
        \param viewpointPlayer   Viewpoint player (only this player's ships are considered)
        \param scoreDefinitions  Score definitions (required for hull functions)
        \param shipList          Ship list (required for hull functions)
        \param config            Host configuration (required for hull functions)
        \return UnloadInfo */
    UnloadInfo prepareUnloadInfo(const Universe& univ,
                                 Id_t pid,
                                 int viewpointPlayer,
                                 const UnitScoreDefinitionList& scoreDefinitions,
                                 const game::spec::ShipList& shipList,
                                 const game::config::HostConfiguration& config);

    /** Prepare events affecting a planet.
        \param univ              Universe
        \param pid               Planet Id
        \param shipScores        Ship score definitions (required for hull functions)
        \param shipList          Ship list (required for hull functions)
        \param config            Host configuration (required for hull functions)
        \return PlanetEffectors */
    PlanetEffectors preparePlanetEffectors(const Universe& univ,
                                           Id_t pid,
                                           const UnitScoreDefinitionList& shipScores,
                                           const game::spec::ShipList& shipList,
                                           const game::config::HostConfiguration& config);

    /** Retrieve information about ground defense.
        This function describes the current ground defense situation.
        \param pl   Planet
        \param root Root (required for player names, host configuration, version)
        \param tx   Translator */
    GroundDefenseInfo packGroundDefenseInfo(const Planet& pl,
                                            const Root& root,
                                            afl::string::Translator& tx);

} }

#endif
