/**
  *  \file server/monitor/timeseriesloader.hpp
  *  \brief Class server::monitor::TimeSeriesLoader
  */
#ifndef C2NG_SERVER_MONITOR_TIMESERIESLOADER_HPP
#define C2NG_SERVER_MONITOR_TIMESERIESLOADER_HPP

#include <vector>
#include "afl/io/stream.hpp"
#include "afl/io/stream.hpp"

namespace server { namespace monitor {

    class TimeSeries;

    /** Load a list of TimeSeries from a file.
        To use,
        - call add() to add all TimeSeries you want to load.
        - call load() to load

        @see TimeSeriesWriter */
    class TimeSeriesLoader {
     public:
        /** Constructor.
            Makes a default, empty object. */
        TimeSeriesLoader();

        /** Destructor. */
        ~TimeSeriesLoader();

        /** Add TimeSeries to load job.
            \param name Name of job (must be unique)
            \param ts   TimeSeries. Object must out-live the TimeSeriesWriter.
                        Should be empty to avoid producing out-of-order data. */
        void add(const String_t& name, TimeSeries& ts);

        /** Load file.
            \param in File */
        void load(afl::io::Stream& in) const;

     private:
        std::vector<String_t> m_names;
        std::vector<TimeSeries*> m_series;

        TimeSeries* get(const String_t& name) const;
        bool parseSectionDelimiter(const String_t& line, TimeSeries*& pCurrent) const;
        bool parseData(const String_t& line, TimeSeries& current) const;
    };

} }

#endif
