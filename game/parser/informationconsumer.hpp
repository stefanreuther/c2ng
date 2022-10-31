/**
  *  \file game/parser/informationconsumer.hpp
  *  \brief Base Class game::parser::InformationConsumer
  */
#ifndef C2NG_GAME_PARSER_INFORMATIONCONSUMER_HPP
#define C2NG_GAME_PARSER_INFORMATIONCONSUMER_HPP

#include "afl/container/ptrvector.hpp"

namespace game { namespace parser {

    class MessageInformation;

    /** Abstract base class for a MessageInformation consumer. */
    class InformationConsumer {
     public:
        /** Virtual destructor. */
        virtual ~InformationConsumer()
            { }

        /** Consume a single MessageInformation object.
            Derived classes override this.
            @param info Object */
        virtual void addMessageInformation(const MessageInformation& info) = 0;

        /** Consume a pack of MessageInformation objects.
            Calls the single-object receiver function for each object in the pack.
            @param info Objects */
        void addMessageInformation(const afl::container::PtrVector<MessageInformation>& info);
    };

} }

#endif
