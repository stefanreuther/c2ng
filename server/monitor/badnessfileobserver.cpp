/**
  *  \file server/monitor/badnessfileobserver.cpp
  *  \brief Class server::monitor::BadnessFileObserver
  */

#include <stdio.h>
#include "server/monitor/badnessfileobserver.hpp"
#include "afl/string/parse.hpp"
#include "afl/io/stream.hpp"
#include "afl/sys/time.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"

namespace {
    /** Maximum age of the badness file until it is considered stale. */
    const int64_t MAX_FILE_AGE = 3600*1000;
}

// Constructor.
server::monitor::BadnessFileObserver::BadnessFileObserver(String_t name, String_t identifier, afl::io::FileSystem& fs)
    : m_name(name),
      m_identifier(identifier),
      m_fileSystem(fs),
      m_fileName("unconfigured")
{ }

// Destructor.
server::monitor::BadnessFileObserver::~BadnessFileObserver()
{ }

String_t
server::monitor::BadnessFileObserver::getName()
{
    return m_name;
}

String_t
server::monitor::BadnessFileObserver::getId()
{
    return m_identifier;
}

bool
server::monitor::BadnessFileObserver::handleConfiguration(const String_t& key, const String_t& value)
{
    if (key == m_identifier) {
        m_fileName = value;
        return true;
    } else {
        return false;
    }
}

server::monitor::Observer::Status
server::monitor::BadnessFileObserver::checkStatus()
{
    // ex planetscentral/monitor.cc:checkStatusFile
    try {
        afl::base::Ref<afl::io::Stream> f = m_fileSystem.openFile(m_fileName, afl::io::FileSystem::OpenRead);

        // Read content
        uint8_t buffer[20];
        afl::base::Bytes_t bufferDesc(buffer);
        bufferDesc.trim(f->read(bufferDesc));

        // Get file time. We don't have a "fstat" equivalent, so this must be done with file names.
        afl::sys::Time fileTime = m_fileSystem.openDirectory(m_fileSystem.getDirectoryName(m_fileName))
            ->getDirectoryEntryByName(m_fileSystem.getFileName(m_fileName))
            ->getModificationTime();

        if ((afl::sys::Time::getCurrentTime() - fileTime).getMilliseconds() > MAX_FILE_AGE) {
            // Status file has not been changed in a long time: service must be down
            return Down;
        }

        if (bufferDesc.empty()) {
            // File is empty.
            // This can happen during the small instant of time the service updates the file.
            // So assume for the benefit of the doubt that the system is OK.
            return Running;
        }

        // Parse
        uint32_t badness = 0;
        if (!afl::string::strToInteger(afl::string::fromBytes(bufferDesc), badness)) {
            // Syntax error
            return Broken;
        }

        if (badness > 1) {
            // More than one bad cycle: broken
            return Broken;
        } else {
            // Everything OK
            return Running;
        }
    }
    catch (std::exception&) {
        // File does not exist or I/O error, service must be down
        return Down;
    }
}
