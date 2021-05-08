/**
  *  \file game/test/counter.hpp
  *  \brief Class game::test::Counter
  */
#ifndef C2NG_GAME_TEST_COUNTER_HPP
#define C2NG_GAME_TEST_COUNTER_HPP

namespace game { namespace test {

    /** Simple counter.
        This class' increment() method can be used as an event listener.
        It counts the number of callbacks. */
    class Counter {
     public:
        Counter()
            : m_counter(0)
            { }

        void increment()
            { ++m_counter; }

        int get() const
            { return m_counter; }
     private:
        int m_counter;
    };

} }

#endif
