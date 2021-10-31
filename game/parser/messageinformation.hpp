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
#include "game/alliance/offer.hpp"

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
            Ufo,                 ///< Target is Ufo given by Id.
            Wormhole,            ///< Target is Wormhole given by Id.
            Explosion,           ///< This message contains an explosion.
            Configuration,       ///< This message contains configuration information.
            PlayerScore,         ///< This message contains player scores. Id is optional and gives the util.dat score Id.
            Alliance,            ///< This message contains alliances.
            NoObject
        };

        typedef afl::container::PtrVector<MessageValueBase> Values_t;
        typedef Values_t::const_iterator Iterator_t, ConstIterator_t;

        /** Constructor.
            \param type Target object type
            \param id Target object Id, dependant on type
            \param turn Turn number */
        MessageInformation(Type type, int32_t id, int turn);

        /** Destructor. */
        ~MessageInformation();

        /** Add string value.
            \param si String value index
            \param s String value */
        void addValue(MessageStringIndex si, const String_t& s);

        /** Add integer value.
            \param ii Integer value index
            \param i Integer value */
        void addValue(MessageIntegerIndex ii, int32_t i);

        /** Add configuration value.
            \pre getObjectType() == Configuration
            \param key Configuration key
            \param value Value */
        void addConfigurationValue(String_t key, String_t value);

        /** Add score value.
            \pre getObjectType() == PlayerScore
            \param player Player number
            \param value Score value */
        void addScoreValue(int player, int32_t value);

        /** Add alliance value.
            \pre getObjectType() == Alliance
            \param id Alliance level identifier
            \param offer Alliance offer */
        void addAllianceValue(String_t id, const game::alliance::Offer& offer);

        /** Get string value.
            \param si  [in] String value index
            \param out [out] Result
            \retval true Value was found, \c out updated
            \retval false Value not found, \c out unchanged */
        bool getValue(MessageStringIndex si, String_t& out) const;

        /** Get integer value.
            \param ii [in] Integer value index
            \param out [out] Result
            \retval true Value was found, \c out updated
            \retval false Value not found, \c out unchanged */
        bool getValue(MessageIntegerIndex ii, int32_t& out) const;

        /** Get iterator to first contained value.
            \return iterator */
        Iterator_t begin() const;

        /** Get iterator to last contained value.
            \return iterator */
        Iterator_t end() const;

        /** Get target object type.
            \return target object type */
        Type getObjectType() const;

        /** Get target object Id.
            \return target object Id */
        int32_t getObjectId() const;

        /** Get turn number.
            \return turn number */
        int getTurnNumber() const;

     private:
        const Type m_type;
        const int32_t m_id;
        const int m_turnNumber;
        Values_t m_values;
    };

} }


// Get iterator to first contained value.
inline game::parser::MessageInformation::Iterator_t
game::parser::MessageInformation::begin() const
{
    return m_values.begin();
}

// Get iterator to last contained value.
inline game::parser::MessageInformation::Iterator_t
game::parser::MessageInformation::end() const
{
    return m_values.end();
}

// Get target object type.
inline game::parser::MessageInformation::Type
game::parser::MessageInformation::getObjectType() const
{
    return m_type;
}

// Get target object Id.
inline int32_t
game::parser::MessageInformation::getObjectId() const
{
    return m_id;
}

// Get turn number.
inline int
game::parser::MessageInformation::getTurnNumber() const
{
    return m_turnNumber;
}

#endif
