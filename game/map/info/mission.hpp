/**
  *  \file game/map/info/mission.hpp
  *  \brief Formatting Mission-Related Information
  */
#ifndef C2NG_GAME_MAP_INFO_MISSION_HPP
#define C2NG_GAME_MAP_INFO_MISSION_HPP

#include "afl/string/translator.hpp"
#include "game/map/info/types.hpp"
#include "game/map/shippredictor.hpp"
#include "game/playerlist.hpp"

namespace game { namespace map { namespace info {

    /** Render chunnel failure reasons.
        This is the rich-text/XML version of game::map::formatChunnelFailureReasons().

        @param [out] list      Output target (empty <li> tag)
        @param [in]  failures  Chunnel failure reasons, return value of ChunnelMission::getFailureReasons()
        @param [in]  tx        Translator */
    void renderChunnelFailureReasons(TagNode& list, int failures, afl::string::Translator& tx);

    /** Render used properties of a ship prediction.

        If the ship predictor has used a mission, and we want to show the full name ("Towing USS Excalibur"),
        the name must be explicitly passed in because we cannot compute it in compiled code.
        If no mission name is given, the plain name of the mission is used.

        @param [out] list        Output target (empty <li> tag)
        @param [in]  pred        Ship predictor
        @param [in]  missionName Ship mission name
        @param [in]  playerList  Player list (for obtaining friendly code names)
        @param [in]  tx          Translator */
    void renderShipPredictorUsedProperties(TagNode& list, const ShipPredictor& pred, const String_t& missionName, const PlayerList& playerList, afl::string::Translator& tx);

} } }

#endif
