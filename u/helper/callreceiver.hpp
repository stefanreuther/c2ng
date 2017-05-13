/**
  *  \file u/helper/callreceiver.hpp
  */
#ifndef C2NG_U_HELPER_CALLRECEIVER_HPP
#define C2NG_U_HELPER_CALLRECEIVER_HPP

#include <list>
#include <cxxtest/TestSuite.h>
#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"
#include "afl/container/ptrvector.hpp"

class CallReceiver {
 public:
    CallReceiver();
    ~CallReceiver();

    void expectCall(String_t call);
    void checkCall(String_t call);
    void checkFinish();

    template<typename T>
    void provideReturnValue(T t)
        { m_returnValues.pushBackNew(new Value<T>(t)); }

    template<typename T>
    T& consumeReturnValue()
        {
            TS_ASSERT(m_nextReturnValue < m_returnValues.size());
            Value<T>* p = dynamic_cast<Value<T>*>(m_returnValues[m_nextReturnValue++]);
            TS_ASSERT(p != 0);
            return p->get();
        }

 private:
    std::list<String_t> m_queue;

    template<typename T>
    class Value : public afl::base::Deletable {
     public:
        Value(T t)
            : m_t(t)
            { }
        T& get()
            { return m_t; }
     private:
        T m_t;
    };

    afl::container::PtrVector<afl::base::Deletable> m_returnValues;
    size_t m_nextReturnValue;
};

#endif
