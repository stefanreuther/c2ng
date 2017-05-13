/**
  *  \file u/helper/callreceiver.cpp
  */

#include "u/helper/callreceiver.hpp"

CallReceiver::CallReceiver()
    : m_queue(),
      m_returnValues(),
      m_nextReturnValue(0)
{ }

CallReceiver::~CallReceiver()
{ }

void
CallReceiver::expectCall(String_t call)
{
    m_queue.push_back(call);
}

void
CallReceiver::checkCall(String_t call)
{
    TS_ASSERT(!m_queue.empty());
    TS_ASSERT_EQUALS(m_queue.front(), call);
    m_queue.pop_front();
}

void
CallReceiver::checkFinish()
{
    TS_ASSERT(m_queue.empty());
    TS_ASSERT_EQUALS(m_nextReturnValue, m_returnValues.size());
}
