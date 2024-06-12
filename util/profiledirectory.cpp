/**
  *  \file util/profiledirectory.cpp
  *  \brief Class util::ProfileDirectory
  */

#include "util/profiledirectory.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "util/io.hpp"

util::ProfileDirectory::ProfileDirectory(afl::sys::Environment& env, afl::io::FileSystem& fileSystem)
    : m_name(env.getSettingsDirectoryName("PCC2")),
      m_fileSystem(fileSystem)
{ }

afl::base::Ptr<afl::io::Stream>
util::ProfileDirectory::openFileNT(String_t name)
{
    try {
        afl::base::Ref<afl::io::Directory> parent = m_fileSystem.openDirectory(m_name);
        return parent->openFile(name, afl::io::FileSystem::OpenRead).asPtr();
    }
    catch (afl::except::FileProblemException&) {
        return 0;
    }
}

afl::base::Ref<afl::io::Stream>
util::ProfileDirectory::createFile(String_t name)
{
    return open()->openFile(name, afl::io::FileSystem::Create);
}

afl::base::Ref<afl::io::Directory>
util::ProfileDirectory::open()
{
    createDirectoryTree(m_fileSystem, m_name);
    return m_fileSystem.openDirectory(m_name);
}
