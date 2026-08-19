/**
  *  \file server/console/fileexporter.cpp
  *  \brief Interface server::console::FileExporter
  */

#include "server/console/fileexporter.hpp"

#include "afl/checksums/sha1.hpp"
#include "afl/string/posixfilenames.hpp"
#include "server/console/parser.hpp"
#include "util/io.hpp"
#include <set>

namespace {
    void writeFile(afl::io::FileSystem& fs, const String_t& fileName, const String_t& fileContent)
    {
        // Create directory
        util::createDirectoryTree(fs, afl::string::PosixFileNames().getDirectoryName(fileName));

        // Store file
        fs.openFile(fileName, afl::io::FileSystem::Create)
            ->fullWrite(afl::string::toBytes(fileContent));
    }
}


/*
 *  InlineExporter
 */

class server::console::FileExporter::InlineExporter : public FileExporter {
 public:
    virtual String_t exportFile(const String_t& fileName, const String_t& fileContent);
};

String_t
server::console::FileExporter::InlineExporter::exportFile(const String_t& /*fileName*/, const String_t& fileContent)
{
    return Parser::quoteConsoleString(fileContent);
}


/*
 *  NormalExporter
 */

class server::console::FileExporter::NormalExporter : public FileExporter {
 public:
    NormalExporter(afl::io::FileSystem& fs, String_t serviceName);
    virtual String_t exportFile(const String_t& fileName, const String_t& fileContent);

 private:
    const String_t m_serviceName;
    afl::io::FileSystem& m_fileSystem;
};

server::console::FileExporter::NormalExporter::NormalExporter(afl::io::FileSystem& fs, String_t serviceName)
    : m_serviceName(serviceName),
      m_fileSystem(fs)
{ }

String_t
server::console::FileExporter::NormalExporter::exportFile(const String_t& fileName, const String_t& fileContent)
{
    String_t pathName = m_serviceName + "/" + fileName;
    writeFile(m_fileSystem, pathName, fileContent);
    return "<" + Parser::quoteConsoleString(pathName);
}

/*
 *  DedupExporter
 */

class server::console::FileExporter::DedupExporter : public FileExporter {
 public:
    DedupExporter(afl::io::FileSystem& fs, String_t serviceName);
    virtual String_t exportFile(const String_t& fileName, const String_t& fileContent);

 private:
    const String_t m_serviceName;
    afl::io::FileSystem& m_fileSystem;
    std::set<String_t> m_files;
};

server::console::FileExporter::DedupExporter::DedupExporter(afl::io::FileSystem& fs, String_t serviceName)
    : m_serviceName(serviceName),
      m_fileSystem(fs),
      m_files()
{ }

String_t
server::console::FileExporter::DedupExporter::exportFile(const String_t& /*fileName*/, const String_t& fileContent)
{
    afl::checksums::SHA1 hash;
    hash.add(afl::string::toBytes(fileContent));
    const String_t hashName = hash.getHashAsHexString();
    const String_t pathName = m_serviceName + "/" + hashName;
    if (m_files.insert(hashName).second) {
        writeFile(m_fileSystem, pathName, fileContent);
    }
    return "<" + Parser::quoteConsoleString(pathName);
}


/*
 *  Main Entry Point
 */

server::console::FileExporter*
server::console::FileExporter::create(Mode mode, afl::io::FileSystem& fs, String_t serviceName)
{
    switch (mode) {
     case NoFiles:
        return 0;
     case InlineFiles:
        return new InlineExporter();
     case NormalFiles:
        return new NormalExporter(fs, serviceName);
     case DedupFiles:
        return new DedupExporter(fs, serviceName);
    }
    return 0;
}
