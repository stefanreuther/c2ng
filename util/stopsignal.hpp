/**
  *  \file util/stopsignal.hpp
  *  \brief Class util::StopSignal
  */
#ifndef C2NG_UTIL_STOPSIGNAL_HPP
#define C2NG_UTIL_STOPSIGNAL_HPP

#include "afl/sys/atomicinteger.hpp"
#include "afl/base/uncopyable.hpp"

namespace util {

    /** Inter-thread stop signalisation.
        This class is intended to signal a stop request to a worker thread.

        - construct a StopSignal
        - worker thread does something like "while (!sig.get()) ...."
        - control thread does "sig.set()" to cause worker to exit
        - when all workers have stopped/terminated, control thread can use
          "sig.clear()" to return the StopSignal to its original state.

        As of 20201008, this is just a wrapped atomic flag,
        but could be something more elaborate (e.g. semaphore) when needed.

        StopSignal is generally passed by reference; it is uncopyable
        to prevent accidental pass by value. */
    class StopSignal : private afl::base::Uncopyable {
     public:
        /** Constructor.
            Initial state is "stop not requested". */
        StopSignal();

        /** Set signal to "stop requested". */
        void set();

        /** Set signal to "stop not requested".
            Call when all affected threads have stopped/terminated
            (to avoid that one misses the stop request). */
        void clear();

        /** Check for stop request.
            \return true if thread shall stop */
        bool get() const;
     private:
        afl::sys::AtomicInteger m_flag;
    };

}

inline
util::StopSignal::StopSignal()
    : m_flag(0)
{ }

inline void
util::StopSignal::set()
{
    m_flag = 1;
}

inline void
util::StopSignal::clear()
{
    m_flag = 0;
}

inline bool
util::StopSignal::get() const
{
    return m_flag != 0;
}

#endif
