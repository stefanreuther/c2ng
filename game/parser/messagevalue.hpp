/**
  *  \file game/parser/messagevalue.hpp
  *  \brief Template class game::parser::MessageValue and related functions
  */
#ifndef C2NG_GAME_PARSER_MESSAGEVALUE_HPP
#define C2NG_GAME_PARSER_MESSAGEVALUE_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"
#include "afl/string/translator.hpp"
#include "game/alliance/offer.hpp"

namespace game { namespace parser {

    /** Base class of a message value.
        For future extensibility. */
    typedef afl::base::Deletable MessageValueBase;


    /** Message value.
        A message value contains an index that identifies the value, and the actual value.
        \param Index type of index
        \param Value type of value */
    template<typename Index, typename Value>
    class MessageValue : public MessageValueBase {
     public:
        typedef Index Index_t;
        typedef Value Value_t;

        /** Constructor.
            \param index Index
            \param value Initial value */
        MessageValue(Index index, Value value);

        /** Destructor. */
        virtual ~MessageValue();

        /** Get index.
            \return index */
        Index getIndex() const
            { return m_index; }

        /** Get current value.
            \return value */
        Value getValue() const
            { return m_value; }

        /** Set new value.
            \param v Value */
        void setValue(Value v)
            { this->m_value = v; }
     public:
        const Index m_index;
        Value m_value;
    };


    /** Message index for object string attributes. */
    enum MessageStringIndex {
        /* Generic: */
        ms_Name,
        ms_FriendlyCode,

        /* Ufo: */
        ms_UfoInfo1,
        ms_UfoInfo2,

        /* Drawing: */
        ms_DrawingComment,
        ms_DrawingTag,

        ms_Max
    };

    /** Message index for object integer attributes. */
    enum MessageIntegerIndex {
        /* Generic: */
        mi_X,
        mi_Y,
        mi_Radius,
        mi_Owner,
        mi_Damage,
        mi_Heading,
        mi_Speed,
        mi_Type,                    // Mines, Ufos
        mi_Mass,
        mi_Color,                   // Ufos, drawings
        mi_EndX,
        mi_EndY,

        /* Minefields: */
        mi_MineUnits,
        mi_MineScanReason,
        mi_MineUnitsRemoved,

        /* Ships: */
        mi_ShipHull,
        mi_ShipFuel,
        mi_ShipRemoteFlag,
        mi_ShipWaypointDX,
        mi_ShipWaypointDY,
        mi_ShipEngineType,
        mi_ShipBeamType,
        mi_ShipNumBeams,
        mi_ShipNumBays,
        mi_ShipLauncherType,
        mi_ShipAmmo,
        mi_ShipNumLaunchers,
        mi_ShipMission,
        mi_ShipTow,
        mi_ShipIntercept,
        mi_ShipEnemy,
        mi_ShipCrew,
        mi_ShipColonists,
        mi_ShipSupplies,
        mi_ShipCargoT,
        mi_ShipCargoD,
        mi_ShipCargoM,
        mi_ShipMoney,

        /* Planets: */
        mi_PlanetTotalN,            // Total minerals, as in Dark Sense or Super Spy
        mi_PlanetTotalT,
        mi_PlanetTotalD,
        mi_PlanetTotalM,
        mi_PlanetAddedN,            // Added minerals, as in Meteor
        mi_PlanetAddedT,
        mi_PlanetAddedD,
        mi_PlanetAddedM,
        mi_PlanetMinedN,            // Mined minerals, as in Allied Planet
        mi_PlanetMinedT,
        mi_PlanetMinedD,
        mi_PlanetMinedM,
        mi_PlanetDensityN,
        mi_PlanetDensityT,
        mi_PlanetDensityD,
        mi_PlanetDensityM,
        mi_PlanetCash,
        mi_PlanetSupplies,
        mi_PlanetHasBase,
        mi_PlanetMines,
        mi_PlanetFactories,
        mi_PlanetDefense,
        mi_PlanetTemperature,
        mi_PlanetColonists,
        mi_PlanetColonistHappiness,
        mi_PlanetColonistTax,
        mi_PlanetActivity,
        mi_PlanetNativeRace,
        mi_PlanetNativeGov,
        mi_PlanetNativeHappiness,
        mi_PlanetNativeTax,
        mi_PlanetNatives,
        mi_PlanetHasNatives,

        /* Bases: */
        mi_BaseQueuePos,
        mi_BaseQueuePriority,

        /* Score: */
        mi_ScoreWinLimit,
        mi_ScoreTurnLimit,

        /* Storm: */
        mi_IonVoltage,
        mi_IonStatus,

        // TODO: TUtil34FTP
        // TODO: TUtil42GODestroyed

        /* Ufo: */
        mi_UfoRealId,               // also as Ufo Id for a wormhole
        mi_UfoSpeedX,
        mi_UfoSpeedY,
        mi_UfoPlanetRange,
        mi_UfoShipRange,

        /* Wormhole: */
        mi_WormholeStabilityCode,
        mi_WormholeBidirFlag,

        /* Explosion: */
        mi_ExplodedShipId,

        /* Drawing: */
        mi_DrawingShape,
        mi_DrawingExpire,

        mi_Max
    };

    /** Object string attribute.
        An arbitrary history information for game objects. */
    typedef MessageValue<MessageStringIndex,String_t> MessageStringValue_t;

    /** Object integer attribute.
        An arbitrary history information for game objects. */
    typedef MessageValue<MessageIntegerIndex,int32_t> MessageIntegerValue_t;

    /** Configuration value. For use with MessageConfig.
        The index is the configuration item name,
        the value is the configuration value. */
    typedef MessageValue<String_t,String_t> MessageConfigurationValue_t;

    /** Score value. For use with MessagePlayerScore.
        The index is the player number,
        the value is the score. */
    typedef MessageValue<int,int32_t> MessageScoreValue_t;

    /** Alliance value. For use with MessageAlliance.
        The string is the alliance identifier.
        The Offer is partially-filled in. */
    typedef MessageValue<String_t,game::alliance::Offer> MessageAllianceValue_t;

    /** Get human-readable name, given a string index.
        \param si String index
        \param tx Translator
        \return Human-readable, translated name */
    String_t getNameFromIndex(MessageStringIndex si, afl::string::Translator& tx);

    /** Get human-readable name, given an integer index.
        \param ii Integer index
        \param tx Translator
        \return Human-readable, translated name */
    String_t getNameFromIndex(MessageIntegerIndex ii, afl::string::Translator& tx);

    /** Get string index, given a keyword.
        \param kw Keyword in upper case
        \return String index; ms_Max if keyword not recognized */
    MessageStringIndex getStringIndexFromKeyword(String_t kw);

    /** Get integer index, given a keyword.
        \param kw Keyword in upper case
        \return Integer index; mi_Max if keyword not recognized */
    MessageIntegerIndex getIntegerIndexFromKeyword(String_t kw);

} }

template<typename Index, typename Value>
inline
game::parser::MessageValue<Index,Value>::MessageValue(Index index, Value value)
    : m_index(index),
      m_value(value)
{ }

template<typename Index, typename Value>
inline
game::parser::MessageValue<Index,Value>::~MessageValue()
{ }

#endif
