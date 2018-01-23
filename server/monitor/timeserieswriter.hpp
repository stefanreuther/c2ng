/**
  *  \file server/monitor/timeserieswriter.hpp
  *  \brief Class server::monitor::TimeSeriesWriter
  */
#ifndef C2NG_SERVER_MONITOR_TIMESERIESWRITER_HPP
#define C2NG_SERVER_MONITOR_TIMESERIESWRITER_HPP

#include <vector>
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"

namespace server { namespace monitor {

    class TimeSeries;

    /** Save a list of TimeSeries to a file.
        To use,
        - call add() to add all TimeSeries you want to write
        - call save() to save them to a file

        The file will contain for each TimeSeries:
        - a "[NAME]" delimiter
        - a list of "time<tab>valid<tab>value" lines for each element.
          The time is milliseconds-since-unix-epoch. */
    class TimeSeriesWriter {
     public:
        /** Constructor.
            Makes a default, empty object. */
        TimeSeriesWriter();

        /** Destructor. */
        ~TimeSeriesWriter();

        /** Add TimeSeries to output job.
            \param name Name of job (must be unique)
            \param ts   TimeSeries. Object must out-live the TimeSeriesWriter. */
        void add(const String_t& name, const TimeSeries& ts);

        /** Save to file.
            \param out Target file */
        void save(afl::io::Stream& out) const;

     private:
        std::vector<String_t> m_names;
        std::vector<const TimeSeries*> m_series;
    };

} }

#endif
