/**
  *  \file game/parser/informationconsumer.cpp
  *  \brief Base Class game::parser::InformationConsumer
  */

#include "game/parser/informationconsumer.hpp"

void
game::parser::InformationConsumer::addMessageInformation(const afl::container::PtrVector<MessageInformation>& info)
{
    for (size_t i = 0, n = info.size(); i < n; ++i) {
        if (const MessageInformation* p = info[i]) {
            addMessageInformation(*p);
        }
    }
}
