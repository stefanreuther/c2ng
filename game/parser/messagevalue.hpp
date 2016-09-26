/**
  *  \file game/parser/messagevalue.hpp
  */
#ifndef C2NG_GAME_PARSER_MESSAGEVALUE_HPP
#define C2NG_GAME_PARSER_MESSAGEVALUE_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"
#include "afl/string/translator.hpp"

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
        MessageValue(Index index, Value value);
        virtual ~MessageValue();

        Index getIndex() const
            { return m_index; }

        Value getValue() const
            { return m_value; }

        void setValue(Value v)
            { this->m_value = v; }
     public:
        const Index m_index;
        Value m_value;
    };


    /** Message index for object string attributes. */
    enum MessageStringIndex {
        ms_Name,
        ms_FriendlyCode,
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

        /* Minefields: */
        mi_MineUnits,
        mi_MineScanReason,
        mi_MineType,
        mi_MineUnitsRemoved,

        /* Ships: */
        mi_ShipHull,
        mi_ShipFuel,
        mi_ShipRemoteFlag,

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
        mi_PlanetCash,
        mi_PlanetSupplies,
        mi_PlanetHasBase,
        mi_PlanetMines,
        mi_PlanetFactories,
        mi_PlanetDefense,
        mi_PlanetTemperature,
        mi_PlanetColonists,
        mi_PlanetActivity,
        mi_PlanetNativeRace,
        mi_PlanetNativeGov,
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

        // TODO: TUtil7Battle
        // TODO: TUtil10Target
        // TODO: TUtil14Wormhole
        // TODO: TUtil17Ion
        // TODO: TUtil33GO
        // TODO: TUtil34FTP
        // TODO: TUtil38PAL
        // TODO: TUtil42GODestroyed
        // TODO: TUtil43MinefieldQuota
        // TODO: TUtil47NonExistantPlanets
        // TODO: Scores
        // TODO: TUtil57Special

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
        The GAllianceOffer is partially-filled in. */
    // FIXME: missing
    // typedef GMessageValue<string_t,GAllianceOffer> GMessageAllianceValue;


    String_t getNameFromIndex(MessageStringIndex si, afl::string::Translator& tx);
    String_t getNameFromIndex(MessageIntegerIndex ii, afl::string::Translator& tx);

    MessageStringIndex getStringIndexFromKeyword(String_t kw);
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
