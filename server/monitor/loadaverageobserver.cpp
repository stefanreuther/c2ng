/**
  *  \file server/monitor/loadaverageobserver.cpp
  */

#include "server/monitor/loadaverageobserver.hpp"
#include "afl/io/stream.hpp"
#include "afl/base/inlinememory.hpp"

server::monitor::LoadAverageObserver::LoadAverageObserver(afl::io::FileSystem& fs, const String_t& fileName)
    : Observer(),
      m_fileSystem(fs),
      m_fileName(fileName)
{ }

String_t
server::monitor::LoadAverageObserver::getName()
{
    return "CPU Load";
}

String_t
server::monitor::LoadAverageObserver::getId()
{
    return "CPULOAD";
}

String_t
server::monitor::LoadAverageObserver::getUnit()
{
    return "%";
}

bool
server::monitor::LoadAverageObserver::handleConfiguration(const String_t& /*key*/, const String_t& /*value*/)
{
    return false;
}

server::monitor::Observer::Result
server::monitor::LoadAverageObserver::check()
{
    // Read load average file.
    // This may throw if the file name is wrong, in which case the caller will supply a default (Broken) result and a log message.
    afl::base::Ref<afl::io::Stream> file = m_fileSystem.openFile(m_fileName, afl::io::FileSystem::OpenRead);
    afl::base::InlineMemory<uint8_t, 4096> buffer;
    buffer.trim(file->read(buffer));

    // Parse "5 minute" average:
    // - skip first space
    buffer.split(buffer.find(' '));
    buffer.split(buffer.findNot(' '));

    // - parse number
    int32_t result = 0;
    const uint8_t* p;
    while ((p = buffer.at(0)) != 0 && *p >= '0' && *p <= '9') {
        result = 10*result + (*p - '0');
        buffer.split(1);
    }
    result *= 100;
    if ((p = buffer.at(0)) != 0 && *p == '.') {
        buffer.split(1);
        int32_t scale = 10;
        while ((p = buffer.at(0)) != 0 && *p >= '0' && *p <= '9') {
            result += scale * (*p - '0');
            buffer.split(1);
            scale /= 10;
        }
    }

    // Produce result
    if (!buffer.empty()) {
        return Result(Value, result);
    } else {
        return Result();
    }
}
