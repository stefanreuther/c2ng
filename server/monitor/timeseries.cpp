/**
  *  \file server/monitor/timeseries.cpp
  */

#include <algorithm>
#include "server/monitor/timeseries.hpp"
#include "afl/base/memory.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/parsedtime.hpp"

using afl::string::Format;

namespace {
    String_t getAgeName(int64_t age)
    {
        if (age < 1000) {
            return "now";
        }
        age = (age+500)/1000;
        if (age < 60) {
            return Format("-%d s", age);
        }
        age = (age+30)/60;
        if (age < 100) {
            return Format("-%d min", age);
        }
        age = (age+30)/60;
        if (age < 36) {
            return Format("-%d h", age);
        }
        age = (age+12)/24;
        return Format("-%d d", age);
    }

    void adjustLimit(int32_t& limit)
    {
        if (limit < -1000000000 || limit > 1000000000) {
            // Do not try to optimize here
        } else {
            // Try to find a nice number
            int sign = +1;
            if (limit < 0) {
                sign = -1;
            }

            int32_t candidate = 2;
            while (1) {
                // 5, 50, 500, ...
                candidate = 2*candidate + candidate/2;
                if (candidate >= sign*limit) {
                    break;
                }
                // 10, 100, 1000, ...
                candidate *= 2;
                if (candidate >= sign*limit) {
                    break;
                }
                // 20, 200, 2000, ...
                candidate *= 2;
                if (candidate >= sign*limit) {
                    break;
                }
            }
            limit = candidate*sign;
        }
    }

    class Path {
     public:
        Path()
            : m_path(),
              m_attributes()
            { }

        Path& move(int x, int y)
            { add(Format("M%d,%d", x, y)); return *this; }
        Path& draw(int x, int y)
            { add(Format("L%d,%d", x, y)); return *this; }
        Path& addAttribute(String_t key, String_t value)
            { m_attributes += Format(" %s=\"%s\"", key, value); return *this; }
        String_t render()
            {
                String_t result = Format("<path d=\"%s\"%s />\n", m_path, m_attributes);
                m_path.clear();
                m_attributes.clear();
                return result;
            }

     private:
        String_t m_path;
        String_t m_attributes;

        void add(const String_t& s)
            {
                if (!m_path.empty()) {
                    m_path += ' ';
                }
                m_path += s;
            }
    };

}

server::monitor::TimeSeries::TimeSeries()
    : m_items()
{ }

server::monitor::TimeSeries::~TimeSeries()
{ }

void
server::monitor::TimeSeries::add(afl::sys::Time time, bool valid, int32_t value)
{
    m_items.push_back(Item(time, valid, value));
}

size_t
server::monitor::TimeSeries::size() const
{
    return m_items.size();
}

bool
server::monitor::TimeSeries::get(size_t index, afl::sys::Time& timeOut, bool& validOut, int32_t& valueOut) const
{
    if (index < m_items.size()) {
        timeOut  = m_items[index].time;
        validOut = m_items[index].valid;
        valueOut = m_items[index].value;
        return true;
    } else {
        return false;
    }
}

bool
server::monitor::TimeSeries::get(size_t index, afl::sys::Time& timeOut, int32_t& valueOut) const
{
    afl::sys::Time time;
    bool flag;
    int32_t value;
    if (get(index, time, flag, value) && flag) {
        timeOut = time;
        valueOut = value;
        return true;
    } else {
        return false;
    }
}

void
server::monitor::TimeSeries::compact(size_t start, size_t count, size_t factor)
{
    // Limit start
    if (start >= m_items.size()) {
        return;
    }

    // Prepare input
    size_t in = start;
    size_t limit = start + std::min(count, m_items.size() - start);

    // Prepare output
    size_t out = start;

    // Read elements
    while (in < limit) {
        // Read first element
        const afl::sys::Time startTime = m_items[in].time;
        int32_t numValid = 0;
        int32_t numTimes = 1;
        int64_t sumValues = 0;
        afl::sys::Duration sumDurations;
        if (m_items[in].valid) {
            ++numValid;
            sumValues += m_items[in].value;
        }
        ++in;

        // Read following elements
        for (size_t i = 1; i < factor; ++i) {
            if (in < limit) {
                sumDurations += m_items[in].time - startTime;
                if (m_items[in].valid) {
                    ++numValid;
                    sumValues += m_items[in].value;
                }
                ++numTimes;
                ++in;
            } else {
                break;
            }
        }

        // Produce output
        m_items[out].time = startTime + afl::sys::Duration::fromMilliseconds(sumDurations.getMilliseconds() / numTimes);
        if (numValid > 0) {
            m_items[out].valid = true;
            m_items[out].value = int32_t(sumValues / numValid);
        } else {
            m_items[out].valid = false;
            m_items[out].value = 0;
        }
        ++out;
    }

    // Remove unused values
    if (in != out) {
        m_items.erase(m_items.begin() + out, m_items.begin() + in);
    }
}

String_t
server::monitor::TimeSeries::render(int width, int height) const
{
    String_t result;

    // Dimensions
    const int AXIS_WIDTH = 50;
    const int AXIS_LABEL_X = 45;
    const int AXIS_HEIGHT = 10;
    const int TOP_Y = 0;
    const int TIME_HEIGHT = 100;
    const int BOTTOM_Y = height - TIME_HEIGHT;

    const int CHART_LEFT = AXIS_WIDTH;
    const int CHART_TOP = 0;
    const int CHART_RIGHT = width;
    const int CHART_BOTTOM = BOTTOM_Y;

    const int LABEL_SPACING = 30;
    const int LABEL_Y = CHART_BOTTOM + 5;

    // Determine axes
    int32_t min, max;
    getMinMax(min, max);
    if (min != 0) {
        adjustLimit(min);
    }
    adjustLimit(max);

    // Render axes
    result += Format("<text x=\"%d\" y=\"%d\" text-anchor=\"end\" class=\"axes\">%d</text>\n", AXIS_LABEL_X, TOP_Y + AXIS_HEIGHT, max);
    result += Format("<text x=\"%d\" y=\"%d\" text-anchor=\"end\" class=\"axes\">%d</text>\n", AXIS_LABEL_X, BOTTOM_Y, min);
    result += Path().addAttribute("class", "axes")
        .move(CHART_LEFT, CHART_TOP)
        .draw(CHART_LEFT, CHART_BOTTOM)
        .draw(CHART_RIGHT, CHART_BOTTOM)
        .render();

    // Quick exit on empty graph
    if (m_items.empty()) {
        return result;
    }

    // Determine width
    const size_t n = m_items.size();
    const size_t scaleX = std::max(size_t(10), n);

    // Render time labels
    int numTimeLabels = std::min(width / LABEL_SPACING, int(m_items.size()));
    for (int i = 0; i < numTimeLabels; ++i) {
        size_t index = m_items.size()-1 - (i * int(m_items.size()) / numTimeLabels);
        if (index < m_items.size()) {
            result += Format("<text x=\"%d\" y=\"%d\" text-anchor=\"end\" transform=\"rotate(-90 %0$d,%d)\" class=\"axes\">%s</text>\n",
                             CHART_LEFT + ((CHART_RIGHT - CHART_LEFT) * int(index) / int(scaleX)),
                             LABEL_Y,
                             getAgeName((m_items.back().time - m_items[index].time).getMilliseconds()));
        }
    }

    // Render chart
    size_t end = n;
    size_t section = 0;
    while (end > 0) {
        size_t start = findLimit(end);
        size_t pathLength = 0;
        Path path;
        if (end < n) {
            // Connect plots!
            ++end;
        }
        for (size_t i = start; i < end; ++i) {
            if (m_items[i].valid) {
                int x = CHART_LEFT + ((CHART_RIGHT - CHART_LEFT) * int(i) / int(scaleX));
                int y = CHART_BOTTOM - ((CHART_BOTTOM - CHART_TOP) * (m_items[i].value - min) / (max - min));
                if (pathLength == 0) {
                    path.move(x, y);
                } else {
                    path.draw(x, y);
                    if (pathLength >= 100) {
                        result += path.addAttribute("class", Format("plot plot%d", section)).render();
                        path.move(x, y);
                        pathLength = 0;
                    }
                }
                ++pathLength;
            }
        }
        if (pathLength > 1) {
            result += path.addAttribute("class", Format("plot plot%d", section)).render();
        }
        ++section;
        end = start;
    }
    return result;
}


void
server::monitor::TimeSeries::getMinMax(int32_t& min, int32_t& max) const
{
    int32_t min_ = 0;
    int32_t max_ = 1;
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (m_items[i].valid) {
            min_ = std::min(min_, m_items[i].value);
            max_ = std::max(max_, m_items[i].value);
        }
    }
    min = min_;
    max = max_;
}

size_t
server::monitor::TimeSeries::findLimit(size_t top) const
{
    if (top >= 2) {
        // Compute acceptable range for top element
        int64_t delta = (m_items[top-1].time - m_items[top-2].time).getMilliseconds();
        int64_t maxDelta = (delta+1) + (delta/3);
        int64_t minDelta = 2*delta/3;

        // Find lower limit
        size_t limit = top-2;
        while (limit > 0) {
            int64_t newDelta = (m_items[limit].time - m_items[limit-1].time).getMilliseconds();
            if (newDelta < minDelta || newDelta > maxDelta) {
                break;
            }
            --limit;
        }

        return limit;
    } else {
        return 0;
    }
}
