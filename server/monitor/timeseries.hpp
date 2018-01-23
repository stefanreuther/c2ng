/**
  *  \file server/monitor/timeseries.hpp
  */
#ifndef C2NG_SERVER_MONITOR_TIMESERIES_HPP
#define C2NG_SERVER_MONITOR_TIMESERIES_HPP

#include <vector>
#include "afl/sys/time.hpp"

namespace server { namespace monitor {

    class TimeSeries {
     public:
        TimeSeries();

        ~TimeSeries();

        void add(afl::sys::Time time, bool valid, int32_t value);

        size_t size() const;

        bool get(size_t index, afl::sys::Time& timeOut, bool& validOut, int32_t& valueOut) const;

        bool get(size_t index, afl::sys::Time& timeOut, int32_t& valueOut) const;

        void compact(size_t start, size_t count, size_t factor);

        String_t render(int width, int height) const;

     private:
        struct Item {
            afl::sys::Time time;
            bool valid;
            int32_t value;

            Item(afl::sys::Time time, bool valid, int32_t value)
                : time(time), valid(valid), value(value)
                { }
        };

        std::vector<Item> m_items;

        void getMinMax(int32_t& min, int32_t& max) const;

        size_t findLimit(size_t top) const;
    };

} }

#endif
