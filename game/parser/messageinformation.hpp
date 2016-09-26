/**
  *  \file game/parser/messageinformation.hpp
  *  \brief Class game::parser::MessageInformation
  *
  *  In addition to information produced by regular data files (PDATA, VCR, etc.), information comes in mixed ad-hoc forms.
  *  This information is converted into a uniform structure before being assimilated by the particular components:
  *
  *  - each message produces one or more MessageInformation's
  *  - each MessageInformation contains information about a single target object
  *  - each MessageInformation can contain multiple values for that target object
  *  - each MessageInformation can contain information about a single turn
  *
  *  Messages can also produce empty MessageInformation to just relate the message to an object.
  */
#ifndef C2NG_GAME_PARSER_MESSAGEINFORMATION_HPP
#define C2NG_GAME_PARSER_MESSAGEINFORMATION_HPP

#include "game/parser/messagevalue.hpp"
#include "afl/container/ptrvector.hpp"

namespace game { namespace parser {

    /** Message information.
        Collects a set of values for a single target object. */
    class MessageInformation {
     public:
        /** Target object type of a message information.
            Together with an Id, determines the target of the information. */
        enum Type {
            // ex GMessageObject
            Ship,                ///< Target is ship given by Id.
            Planet,              ///< Target is planet given by Id.
            Starbase,            ///< Target is starbase given by Id.
            Minefield,           ///< Target is minefield given by Id.
            IonStorm,            ///< Target is ion storm given by Id.
            Explosion,           ///< This message contains an explosion.
            Configuration,       ///< This message contains configuration information.
            PlayerScore,         ///< This message contains player scores. Id is optional and gives the util.dat score Id.
            Alliance,            ///< This message contains alliances.
            NoObject
        };

        typedef afl::container::PtrVector<MessageValueBase> Values_t;
        typedef Values_t::const_iterator Iterator_t, ConstIterator_t;

        MessageInformation(Type type, int32_t id, int turn);
        ~MessageInformation();

        void addValue(MessageStringIndex si, const String_t& s);
        void addValue(MessageIntegerIndex ii, int32_t i);
        void addConfigurationValue(String_t key, String_t value);
        void addScoreValue(int player, int32_t value);
        // FIXME: missing
        // void addAllianceValue(string_t id, const GAllianceOffer& offer);

        Iterator_t begin() const
            { return m_values.begin(); }
        Iterator_t end() const
            { return m_values.end(); }

        Type getObjectType() const
            { return m_type; }
        int32_t getObjectId() const
            { return m_id; }
        int getTurnNumber() const
            { return m_turnNumber; }

     private:
        const Type m_type;
        const int32_t m_id;
        const int m_turnNumber;
        Values_t m_values;
    };

} }

#endif
