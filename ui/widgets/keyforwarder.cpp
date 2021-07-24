/**
  *  \file ui/widgets/keyforwarder.cpp
  *  \brief Class ui::widgets::KeyForwarder
  */

#include "ui/widgets/keyforwarder.hpp"

ui::widgets::KeyForwarder::KeyForwarder(gfx::KeyEventConsumer& consumer)
    : InvisibleWidget(), m_consumer(consumer)
{ }

bool
ui::widgets::KeyForwarder::handleKey(util::Key_t key, int prefix)
{
    return m_consumer.handleKey(key, prefix);
}
