/**
  *  \file server/monitor/status.cpp
  *  \brief Class server::monitor::Status
  */

#include "server/monitor/status.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/mutexguard.hpp"
#include "server/monitor/timeserieswriter.hpp"
#include "server/monitor/timeseriesloader.hpp"

using server::monitor::Observer;
using afl::string::Format;

namespace {
    const char*const LOG_NAME = "monitor.status";

    const char* getStateName(Observer::Status st)
    {
        switch (st) {
         case Observer::Unknown: return "Unknown";
         case Observer::Running: return "Running";
         case Observer::Broken:  return "Broken";
         case Observer::Down:    return "Down";
         case Observer::Value:   return "Value";
        }
        return "?";
    }
}

server::monitor::Status::Status()
    : m_log(),
      m_mutex(),
      m_observers(),
      m_timeSeries(),
      m_status(),
      m_statusTime(),
      m_maxTimePoints(2000)
{ }

server::monitor::Status::~Status()
{ }

void
server::monitor::Status::addNewObserver(Observer* p)
{
    if (p != 0) {
        afl::sys::MutexGuard g(m_mutex);
        m_observers.pushBackNew(p);
    }
}

bool
server::monitor::Status::handleConfiguration(const String_t& key, const String_t& value)
{
    bool result = false;
    for (size_t i = 0, n = m_observers.size(); i < n; ++i) {
        if (m_observers[i]->handleConfiguration(key, value)) {
            result = true;
        }
    }
    if (key == "MONITOR.HISTORY") {
        /* @q Monitor.History:Int (Config)
           History depth.
           Load-average and latency probes will keep a history of this many values.
           When the history exceeds the limit, older values will be removed by averaging them.
           @since PCC2 2.40.3 */
        int n;
        if (afl::string::strToInteger(value, n) && n > 0) {
            m_maxTimePoints = size_t(n);
            result = true;
        } else {
            throw afl::except::CommandLineException(afl::string::Format("Invalid number for '%s'", key));
        }
    }
    return result;
}

void
server::monitor::Status::update()
{
    // Determine new status. Be careful to not hold a mutex while doing so.
    std::vector<Observer::Result> newStatus;
    for (size_t i = 0, n = getNumObservers(); i < n; ++i) {
        Observer::Result newItem;
        if (Observer* p = getObserverByIndex(i)) {
            try {
                newItem = p->check();
            }
            catch (std::exception& e) {
                log().write(afl::sys::LogListener::Warn, LOG_NAME, p->getName(), e);
                newItem = Observer::Result(Observer::Broken, 0);
            }
        }
        newStatus.push_back(newItem);
    }

    // Update status atomically
    afl::sys::MutexGuard g(m_mutex);
    while (m_timeSeries.size() < newStatus.size()) {
        m_timeSeries.pushBackNew(new TimeSeries());
    }
    m_statusTime = afl::sys::Time::getCurrentTime();
    for (size_t i = 0, n = newStatus.size(); i < n; ++i) {
        if (i >= m_status.size() || newStatus[i].status != m_status[i].status) {
            log().write(afl::sys::LogListener::Info, LOG_NAME,
                        Format("%s: %s (%d)",
                               i < m_observers.size() ? m_observers[i]->getName() : "?",
                               getStateName(newStatus[i].status),
                               newStatus[i].value));
        }
        m_timeSeries[i]->add(m_statusTime,
                             (newStatus[i].status == Observer::Value || newStatus[i].status == Observer::Running),
                             newStatus[i].value);
        if (m_timeSeries[i]->size() > m_maxTimePoints) {
            m_timeSeries[i]->compact(0, m_maxTimePoints/2, 2);
        }
    }
    m_status.swap(newStatus);
}

String_t
server::monitor::Status::render(afl::sys::Time& time) const
{
    // ex planetscentral/monitor.cc:addStatus
    afl::sys::MutexGuard g(m_mutex);
    String_t result;
    const Observer::Result defaultItem;
    for (size_t i = 0, n = m_observers.size(); i < n; ++i) {
        const Observer::Result* pItem = (i < m_status.size() ? &m_status[i] : &defaultItem);
        const char* tpl = "";
        switch (pItem->status) {
         case Observer::Unknown:
            tpl = ("      <div class=\"service unknown-service\" id=\"service%d\">\n"
                   "        <h2>%s</h2>\n"
                   "        <span class=\"status\">unknown</span>\n"
                   "      </div>\n");
            break;
         case Observer::Running:
            tpl = ("      <div class=\"service active-service\" id=\"service%d\">\n"
                   "        <h2>%s</h2>\n"
                   "        <span class=\"status\">active</span>\n"
                   "        <span class=\"latency\">%d&nbsp;ms</span>\n"
                   "      </div>\n");
            break;
         case Observer::Broken:
            tpl = ("      <div class=\"service broken-service\" id=\"service%d\">\n"
                   "        <h2>%s</h2>\n"
                   "        <span class=\"status\">broken</span>\n"
                   "      </div>\n");
            break;
         case Observer::Down:
            tpl = ("      <div class=\"service failed-service\" id=\"service%d\">\n"
                   "        <h2>%s</h2>\n"
                   "        <span class=\"status\">down</span>\n"
                   "      </div>\n");
            break;
         case Observer::Value:
            tpl = ("      <div class=\"service active-service\" id=\"service%d\">\n"
                   "        <h2>%s</h2>\n"
                   "        <span class=\"value\">%d&nbsp;%s</span>\n"
                   "      </div>\n");
        }
        // FIXME: HTML escaping? We don't have that in utilities yet. c2monitor-classic didn't do it
        result += Format(tpl, i, m_observers[i]->getName(), pItem->value, m_observers[i]->getUnit());
    }
    time = m_statusTime;
    return result;
}

String_t
server::monitor::Status::renderTimeSeries() const
{
    const int W = 600;
    const int H = 450;
    afl::sys::MutexGuard g(m_mutex);
    String_t result;
    for (size_t i = 0, n = m_observers.size(); i < n; ++i) {
        const TimeSeries* p = (i < m_timeSeries.size() ? m_timeSeries[i] : 0);
        if (p != 0) {
            result += Format("<div class=\"chart\" id=\"chart%d\">\n", i);
            result += Format("<h2>%s</h2>\n", m_observers[i]->getName());
            result += Format("<svg width=\"%dpx\" height=\"%dpx\" viewbox=\"0 0 %0$d %1$d\"><g>\n", W, H);
            result += p->render(W, H);
            result += "</g></svg></div>\n";
        }
    }
    return result;
}

void
server::monitor::Status::load(afl::io::Stream& file)
{
    afl::sys::MutexGuard g(m_mutex);
    TimeSeriesLoader r;
    while (m_timeSeries.size() < m_observers.size()) {
        m_timeSeries.pushBackNew(new TimeSeries());
    }
    for (size_t i = 0, n = m_observers.size(); i < n; ++i) {
        if (m_observers[i] != 0 && m_timeSeries[i] != 0) {
            r.add(m_observers[i]->getId(), *m_timeSeries[i]);
        }
    }
    r.load(file);
}

void
server::monitor::Status::save(afl::io::Stream& file) const
{
    afl::sys::MutexGuard g(m_mutex);
    TimeSeriesWriter w;
    for (size_t i = 0, n = m_observers.size(), m = m_timeSeries.size(); i < n && i < m; ++i) {
        if (m_observers[i] != 0 && m_timeSeries[i] != 0) {
            w.add(m_observers[i]->getId(), *m_timeSeries[i]);
        }
    }
    w.save(file);
}

afl::sys::Log&
server::monitor::Status::log()
{
    return m_log;
}

size_t
server::monitor::Status::getNumObservers()
{
    afl::sys::MutexGuard g(m_mutex);
    return m_observers.size();
}

server::monitor::Observer*
server::monitor::Status::getObserverByIndex(size_t n)
{
    afl::sys::MutexGuard g(m_mutex);
    return n < m_observers.size() ? m_observers[n] : 0;
}
