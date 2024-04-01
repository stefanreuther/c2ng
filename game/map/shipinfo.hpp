/**
  *  \file game/map/shipinfo.hpp
  *  \brief Functions to obtain information about ships
  */
#ifndef C2NG_GAME_MAP_SHIPINFO_HPP
#define C2NG_GAME_MAP_SHIPINFO_HPP

#include <vector>
#include "game/map/point.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/teamsettings.hpp"
#include "game/types.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace map {

    class Configuration;
    class Ship;
    class Universe;

    /** Information about a ship movement request.

        \todo add ship being passive participant of chunnel
        \todo add wormholes */
    struct ShipMovementInfo {
        enum Action {                     ///< Kind of action.
            Movement,                     ///< Regular movement order.
            Tow,                          ///< Ship is being towed by another ship.
            Chunnel,                      ///< Ship is initiating a chunnel.
            FleetLeader                   ///< Relation to fleet leader.
        };

        enum Status {                     ///< Expected result of action.
            Success,                      ///< Expected success (we don't know why it fails).
            InitiatorFails,               ///< Fails due to initiator. Chunnel: ship itself.
            MateFails                     ///< Fails due to mate. Chunnel: chunnel target.
        };

        Action action;                    ///< Kind of action.
        Status status;                    ///< Expected result of action.
        Id_t partner;                     ///< Partner ship Id. Chunnel: chunnel mate.
        Point from;                       ///< Starting location.
        Point to;                         ///< Ending location (nearest alias). @see Configuration::getSimpleNearestAlias.

        /// Default constructor.
        ShipMovementInfo()
            : action(Movement), status(Success), partner(0), from(), to()
            { }

        /// Construct from values.
        ShipMovementInfo(Action action, Status status, Id_t partner, Point from, Point to)
            : action(action), status(status), partner(partner), from(from), to(to)
            { }

        bool operator==(const ShipMovementInfo& other) const;
        bool operator!=(const ShipMovementInfo& other) const;
    };

    typedef std::vector<ShipMovementInfo> ShipMovementInfos_t;

    /** Describe ship movement info.
        Produces a list of movement orders affecting this ship.

        \param [out] result          Result appended here
        \param [in] ship             Ship to get information about
        \param [in] univ             Universe (used for other ships)
        \param [in] scoreDefinitions Ship score definitions (used for hull functions)
        \param [in] mapConfig        Map configuration
        \param [in] teamSettings     Team settings (for allied chunnel)
        \param [in] shipList         Ship list (used for hull functions)
        \param [in] root             Root (used for host configuration) */
    void packShipMovementInfo(ShipMovementInfos_t& result,
                              const Ship& ship,
                              const Universe& univ,
                              const UnitScoreDefinitionList& scoreDefinitions,
                              const Configuration& mapConfig,
                              const TeamSettings& teamSettings,
                              const game::spec::ShipList& shipList,
                              const Root& root);


    /** Textual information about ship cargo.
        This information can be rendered into a table. */
    struct ShipCargoInfo {
        String_t name;                    ///< Name of item / heading.
        String_t value;                   ///< Value as formatted number. Can be empty.
        String_t unit;                    ///< Unit. Can be empty.
        bool isHeading;                   ///< true if this is a section heading (value, unit ignored in this case).
        bool addSpaceBefore;              ///< true if renderer should place some vertical space before this line.

        ShipCargoInfo(String_t name, String_t value, String_t unit, bool isHeading, bool addSpaceBefore)
            : name(name), value(value), unit(unit), isHeading(isHeading), addSpaceBefore(addSpaceBefore)
            { }
    };

    typedef std::vector<ShipCargoInfo> ShipCargoInfos_t;

    /** Describe a ship's last known cargo.
        Produces a list of ShipCargoInfo elements including a heading, describing the cargo.
        If nothing is known, no output is produced.

        \param [out] result     Result appended here
        \param [in] ship        Ship
        \param [in] currentTurn Current turn number (used to format turn numbers)
        \param [in] fmt         Number formatter
        \param [in] shipList    Ship list (used for naming cargo)
        \param [in] tx          Translator */
    void packShipLastKnownCargo(ShipCargoInfos_t& result, const Ship& ship, int currentTurn, util::NumberFormatter fmt, const game::spec::ShipList& shipList, afl::string::Translator& tx);

    /** Describe a ship's mass ranges.
        Produces a list of ShipCargoInfo elements including a heading,
        describing possible mass distributions of cargo, fuel, weapons.
        If nothing is known, no output is produced.

        \param [out] result     Result appended here
        \param [in] ship        Ship
        \param [in] fmt         Number formatter
        \param [in] shipList    Ship list (used for naming components and obtaining masses)
        \param [in] tx          Translator */
    void packShipMassRanges(ShipCargoInfos_t& result, const Ship& ship, util::NumberFormatter fmt, const game::spec::ShipList& shipList, afl::string::Translator& tx);


    /** Information about a ship's position/location. */
    struct ShipLocationInfo {
        int turnNumber;                             ///< Turn number; always set.
        afl::base::Optional<Point> position;        ///< Position, if known.
        String_t positionName;                      ///< Name of position. Can be empty if position is not known.
        IntegerProperty_t mass;                     ///< Mass, if known.
        IntegerProperty_t heading;                  ///< Heading, if known. Unknown if ship did not move.
        IntegerProperty_t warpFactor;               ///< Warp factor, if known.
        afl::base::Optional<double> distanceMoved;  ///< Distance moved, if known.
        ShipLocationInfo(int turnNumber)
            : turnNumber(turnNumber), position(), positionName(), mass(), heading(), warpFactor(), distanceMoved()
            { }
    };

    typedef std::vector<ShipLocationInfo> ShipLocationInfos_t;

    /** Describe a ship's last locations.
        Results are produces starting with the newest data.

        \param [out] result     Result appended here
        \param [in]  ship       Ship
        \param [in]  univ       Universe (for naming locations)
        \param [in]  turnNumber Turn number (for current info)
        \param [in]  mapConfig  Map configuration (for naming locations, distances)
        \param [in]  config     Host configuration (for naming locations)
        \param [in]  host       Host version (for naming locations)
        \param [in]  shipList   Ship list (for current mass)
        \param [in]  tx         Translator */
    void packShipLocationInfo(ShipLocationInfos_t& result,
                              const Ship& ship,
                              const Universe& univ,
                              int turnNumber,
                              const game::map::Configuration& mapConfig,
                              const game::config::HostConfiguration& config,
                              const HostVersion& host,
                              const game::spec::ShipList& shipList,
                              afl::string::Translator& tx);

    /** Information about a ship's experience. */
    struct ShipExperienceInfo {
        IntegerProperty_t level;              ///< Current level, if known.
        IntegerProperty_t points;             ///< Current number of experience points, if known.
        IntegerProperty_t pointGrowth;        ///< Current growth per turn, if known.
    };

    /** Describe a ship's experience status.
        \param ship             Ship
        \param scoreDefinitions Ship score definitions (used for hull functions)
        \param config           Host configuration (for experience parameters)
        \param host             Host version (for hull functions, experience support)
        \param shipList         Ship list (for hulls, missions)
        \return result */
    ShipExperienceInfo packShipExperienceInfo(const Ship& ship,
                                              const UnitScoreDefinitionList& scoreDefinitions,
                                              const game::config::HostConfiguration& config,
                                              const HostVersion& host,
                                              const game::spec::ShipList& shipList);

    /** Get training experience for a ship.
        \param owner      Ship real owner
        \param supplies   Number of supplies to use
        \param isAcademy  true for academy ship
        \param crew       Hull crew (see Hull::getMaxCrew())
        \param config     Host configuration
        \return Experience acquired by training */
    int getShipTrainingExperience(int owner, int supplies, bool isAcademy, int crew, const game::config::HostConfiguration& config);

    /** Get number of turns needed to reach an experience point target.
        Takes as input the a ShipExperienceInfo structure prepared by packShipExperienceInfo().
        \param target     Point target
        \param info       Experience information
        \return Number of turns. 0 if preconditions violated (target reached, growth unknown/negative, etc.) */
    int getNumTurnsUntil(int target, const ShipExperienceInfo& info);

} }

#endif
