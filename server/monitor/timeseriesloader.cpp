/**
  *  \file server/monitor/timeseriesloader.cpp
  *  \brief Class server::monitor::TimeSeriesLoader
  */

#include "server/monitor/timeseriesloader.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "server/monitor/timeseries.hpp"
#include "util/stringparser.hpp"

server::monitor::TimeSeriesLoader::TimeSeriesLoader()
    : m_names(),
      m_series()
{ }

// Destructor.
server::monitor::TimeSeriesLoader::~TimeSeriesLoader()
{ }

// Add TimeSeries to load job.
void
server::monitor::TimeSeriesLoader::add(const String_t& name, TimeSeries& ts)
{
    m_names.push_back(name);
    m_series.push_back(&ts);
}

// Load file.
void
server::monitor::TimeSeriesLoader::load(afl::io::Stream& in) const
{
    TimeSeries* pCurrent = 0;

    afl::io::TextFile tf(in);
    String_t line;
    while (tf.readLine(line)) {
        if (parseSectionDelimiter(line, pCurrent)) {
            // ok
        } else if (pCurrent != 0 && parseData(line, *pCurrent)) {
            // ok
        } else {
            // not ok; ignore line
        }
    }
}

server::monitor::TimeSeries*
server::monitor::TimeSeriesLoader::get(const String_t& name) const
{
    for (size_t i = 0, n = m_series.size(); i < n; ++i) {
        if (m_names[i] == name) {
            return m_series[i];
        }
    }
    return 0;
}

bool
server::monitor::TimeSeriesLoader::parseSectionDelimiter(const String_t& line, TimeSeries*& pCurrent) const
{
    util::StringParser p(line);
    String_t sectionName;
    if (p.parseCharacter('[') && p.parseDelim("]", sectionName) && p.parseCharacter(']')) {
        // "[XXX]"
        pCurrent = get(sectionName);
        return true;
    } else {
        return false;
    }
}

bool
server::monitor::TimeSeriesLoader::parseData(const String_t& line, TimeSeries& current) const
{
    util::StringParser p(line);
    int64_t time = 0;
    int valid = 0;
    int64_t value = 0;

    // parseInt will eat whitespace, so we only need to look for three numbers and need not explicitly mention the '\t'.
    if ((p.parseInt64(time) && p.parseInt(valid) && p.parseInt64(value) && p.parseEnd())
        && (valid == 0 || valid == 1)
        && (value >= -0x7FFFFFFFLL && value <= 0x7FFFFFFFLL))
    {
        current.add(afl::sys::Time::fromUnixTime(0) + afl::sys::Duration::fromMilliseconds(time),
                    valid != 0,
                    int32_t(value));
        return true;
    } else {
        return false;
    }
}
