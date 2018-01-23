/**
  *  \file server/monitor/timeserieswriter.cpp
  *  \brief Class server::monitor::TimeSeriesWriter
  */

#include "server/monitor/timeserieswriter.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/time.hpp"
#include "server/monitor/timeseries.hpp"

using afl::string::Format;
using afl::sys::Time;

// Constructor.
server::monitor::TimeSeriesWriter::TimeSeriesWriter()
    : m_names(),
      m_series()
{ }

// Destructor.
server::monitor::TimeSeriesWriter::~TimeSeriesWriter()
{ }

// Add TimeSeries to output job.
void
server::monitor::TimeSeriesWriter::add(const String_t& name, const TimeSeries& ts)
{
    m_names.push_back(name);
    m_series.push_back(&ts);
}

// Save to file.
void
server::monitor::TimeSeriesWriter::save(afl::io::Stream& out) const
{
    afl::io::TextFile tf(out);
    const Time epoch = Time::fromUnixTime(0);
    for (size_t i = 0, n = m_series.size(); i < n; ++i) {
        // Name
        tf.writeLine(Format("[%s]", m_names[i]));

        // Time series
        const TimeSeries& ts = *m_series[i];
        for (size_t ii = 0, nn = ts.size(); ii < nn; ++ii) {
            Time time;
            bool valid = false;
            int32_t value = 0;
            if (ts.get(ii, time, valid, value)) {
                tf.writeLine(Format("%d\t%d\t%d", (time - epoch).getMilliseconds(), int(valid), value));
            }
        }
    }
    tf.flush();
}
