/**
  *  \file server/file/root.cpp
  */

#include "server/file/root.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/string/format.hpp"

server::file::Root::Root(DirectoryItem& rootDirectory, afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory)
    : m_log(),
      m_rootDirectory(rootDirectory),
      m_maxFileSize(10L*1024*1024),
      m_defaultCharset(afl::charset::g_codepage437),
      m_defaultRaceNames(),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_translator(),
      m_scanner(*m_defaultSpecificationDirectory, m_translator, m_log)
{
    loadRaceNames();
}

server::file::Root::~Root()
{ }

// Access root directory.
server::file::DirectoryItem&
server::file::Root::rootDirectory()
{
    return m_rootDirectory;
}

// Access logger.
afl::sys::Log&
server::file::Root::log()
{
    return m_log;
}

afl::charset::Charset&
server::file::Root::defaultCharacterSet()
{
    return m_defaultCharset;
}

const server::common::RaceNames&
server::file::Root::defaultRaceNames() const
{
    return m_defaultRaceNames;
}

game::v3::DirectoryScanner&
server::file::Root::directoryScanner()
{
    return m_scanner;
}

afl::io::Stream::FileSize_t
server::file::Root::getMaxFileSize() const
{
    return m_maxFileSize;
}

void
server::file::Root::setMaxFileSize(afl::io::Stream::FileSize_t limit)
{
    m_maxFileSize = limit;
}

void
server::file::Root::loadRaceNames()
{
    try {
        m_defaultRaceNames.load(m_defaultSpecificationDirectory->openFile("race.nm", afl::io::FileSystem::OpenRead)->createVirtualMapping()->get(),
                                defaultCharacterSet());
    }
    catch (std::exception& e) {
        // Ignore error. User of m_defaultRaceNames compensates.
    }
}
