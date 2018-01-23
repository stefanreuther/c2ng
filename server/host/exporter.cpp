/**
  *  \file server/host/exporter.cpp
  *  \brief Class server::host::Exporter
  */

#include "server/host/exporter.hpp"
#include "afl/base/countof.hpp"
#include "afl/io/archive/tarreader.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/inflatetransform.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/transformreaderstream.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/time.hpp"
#include "server/file/clientdirectoryhandler.hpp"
#include "server/file/filesystemhandler.hpp"
#include "server/file/utils.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/interface/filebaseclient.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::Directory;
using afl::io::DirectoryEntry;
using afl::string::Format;

namespace {
    const char*const LOG_NAME = "host.export";

    /** Export a database hash into c2host.ini.
        \param out output file, c2host.ini
        \param prefix Prefix to use for all values
        \param hash Hash to output */
    void exportHash(server::host::ConfigurationBuilder& out, const String_t& prefix, afl::net::redis::HashKey hash)
    {
        // ex planetscentral/host/exec.cc:exportHash
        afl::data::StringList_t data;
        hash.getAll(data);
        for (size_t i = 0; i+1 < data.size(); i += 2) {
            out.addValue(prefix + "_" + data[i], data[i+1]);
        }
    }

    /** Remove content of a directory.
        \param fs FileSystem instance
        \param fsDirName Directory name */
    void removeDirectoryContent(afl::io::FileSystem& fs, const String_t& fsDirName)
    {
        server::file::FileSystemHandler handler(fs, fsDirName);
        removeDirectoryContent(handler);
    }

    /** Split extension off a file name.
        \param fullName [in] Full file name (without directory etc.)
        \param ext      [in] Expected extension
        \param baseName [out] Basename
        \retval true Extension matched, baseName has been set
        \retval false Extension did not match */
    bool splitExtension(const String_t& fullName, const char* ext, String_t& baseName)
    {
        size_t n = std::strlen(ext);
        if (fullName.size() > n) {
            size_t splitPoint = fullName.size() - n;
            if (fullName.compare(splitPoint, n, ext, n) == 0) {
                baseName.assign(fullName, 0, splitPoint);
                return true;
            }
        }
        return false;
    }
}

/******************************** Exporter *******************************/


// Constructor.
server::host::Exporter::Exporter(afl::net::CommandHandler& source, afl::io::FileSystem& fs, afl::sys::LogListener& log)
    : m_source(source),
      m_fileSystem(fs),
      m_log(log)
{ }

// Export a game.
String_t
server::host::Exporter::exportGame(Game& game, Root& root, String_t fsDirName)
{
    // ex planetscentral/host/exec.cc:exportEnvironment, sort-of
    // @change This is totally different in c2ng than classic.
    // In classic, this only creates the c2host.ini file, and otherwise operates directly in the hostfile space.
    // c2ng uses copy-in/copy-out and therefore has to copy around files here.
    ConfigurationBuilder ini;

    uint32_t startTicks = afl::sys::Time::getTickCounter();

    removeDirectoryContent(m_fileSystem, fsDirName);
    Ref<Directory> target = m_fileSystem.openDirectory(fsDirName);

    // Export settings
    exportHash(ini, "game_settings", game.settings());
    ini.addValue("game_name", game.getName());

    // Host
    String_t host = game.getConfig("host");
    ini.addValue("game_host", host);
    exportTool(ini, *target, "host", "game_host", root.hostRoot().byName(host));

    // Master
    String_t master = game.getConfig("master");
    ini.addValue("game_master", master);
    exportTool(ini, *target, "master", "game_master", root.masterRoot().byName(master));

    // Ship list
    String_t sl = game.getConfig("shiplist");
    ini.addValue("game_sl", sl);
    exportTool(ini, *target, "shiplist", "game_sl", root.shipListRoot().byName(sl));

    // Tools
    afl::data::StringList_t tools;
    String_t toolList;
    game.toolsByKind().getAll(tools);
    for (size_t i = 0; i+1 < tools.size(); i += 2) {
        ini.addValue("game_tool_" + tools[i], tools[i+1]);
        exportTool(ini, *target, Format("tool%d", i), "game_tool_" + tools[i], root.toolRoot().byName(tools[i+1]));
        if (i != 0) {
            toolList += " ";
        }
        toolList += tools[i];
    }
    ini.addValue("game_tools", toolList);

    // Population
    String_t pop;
    for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
        if (i != 1) {
            pop += ",";
        }
        if (game.isSlotInGame(i)) {
            pop += "yes";
        } else {
            pop += "no";
        }
    }
    ini.addValue("game_slots", pop);

    // Game data
    const char* GAME_PATH = "game";
    String_t gamePath = game.getDirectory();
    Ref<DirectoryEntry> gameEntry = target->getDirectoryEntryByName(GAME_PATH);
    gameEntry->createAsDirectory();
    exportSubdirectory(gamePath + "/in", gameEntry->getPathName(), "in");
    exportSubdirectory(gamePath + "/out", gameEntry->getPathName(), "out");
    exportSubdirectory(gamePath + "/data", gameEntry->getPathName(), "data");

    // Existing game scripts will attempt to make backups. Make the directory so they don't fail.
    gameEntry->openDirectory()->getDirectoryEntryByName("backup")->createAsDirectory();

    // Main scripts
    exportSubdirectory("bin", fsDirName, "bin");
    exportSubdirectory("defaults", fsDirName, "defaults");

    // Save config
    storeConfigurationFile(ini, *gameEntry->openDirectory());

    // Log
    uint32_t elapsedTicks = afl::sys::Time::getTickCounter() - startTicks;
    m_log.write(afl::sys::LogListener::Info, LOG_NAME, Format("Export complete: host:%s -> %s, %d ms", gamePath, fsDirName, elapsedTicks));

    return GAME_PATH;
}

// Import a game.
void
server::host::Exporter::importGame(Game& game, Root& root, String_t fsDirName)
{
    uint32_t startTicks = afl::sys::Time::getTickCounter();
    Ref<Directory> target = m_fileSystem.openDirectory(fsDirName);

    // Import
    const char* GAME_PATH = "game";
    String_t gamePath = game.getDirectory();
    Ref<DirectoryEntry> gameEntry = target->getDirectoryEntryByName(GAME_PATH);
    importSubdirectory(gamePath + "/in", gameEntry->getPathName(), "in");
    importSubdirectory(gamePath + "/out", gameEntry->getPathName(), "out");
    importSubdirectory(gamePath + "/data", gameEntry->getPathName(), "data");

    // FIXME: deal with logfiles? (runhost.log, runmaster.log)
    importBackups(gamePath + "/backup", gameEntry->getPathName(), "backup", root.config().unpackBackups);

    // Log
    uint32_t elapsedTicks = afl::sys::Time::getTickCounter() - startTicks;
    m_log.write(afl::sys::LogListener::Info, LOG_NAME, Format("Import complete: host:%s <- %s, %d ms", gamePath, fsDirName, elapsedTicks));
}

// Export a tool.
void
server::host::Exporter::exportTool(ConfigurationBuilder& ini, afl::io::Directory& parent, const String_t& dirName, const String_t& prefix, afl::net::redis::HashKey hash)
{
    // Create target directory
    Ref<DirectoryEntry> dirEntry = parent.getDirectoryEntryByName(dirName);
    dirEntry->createAsDirectory();

    // Get source directory
    String_t sourceName = hash.stringField("path").get();

    // Sync
    if (!sourceName.empty()) {
        m_log.write(afl::sys::LogListener::Trace, LOG_NAME, Format("Exporting host:%s -> %s (tool)", sourceName, dirEntry->getPathName()));
        server::file::FileSystemHandler targetHandler(m_fileSystem, dirEntry->getPathName());
        server::file::ClientDirectoryHandler sourceHandler(m_source, sourceName);
        copyDirectory(targetHandler, sourceHandler, true);
    }

    // Copy config
    String_t program = hash.stringField("program").get();
    ini.addValue(prefix + "_path", dirName);
    ini.addValue(prefix + "_program", program);

    const char*const OTHER_KEYS[] = {
        "description",
        "docurl",
        "extradescription",
        "files",
        "difficulty",
        "kind",
        "mainurl",
        "useDifficulty",
    };
    for (size_t i = 0; i < countof(OTHER_KEYS); ++i) {
        String_t value = hash.stringField(OTHER_KEYS[i]).get();
        if (!value.empty()) {
            ini.addValue(prefix + "_" + OTHER_KEYS[i], value);
        }
    }

    // Mark program executable, if possible
    if (!program.empty()) {
        Ref<DirectoryEntry> progEntry = dirEntry->openDirectory()->getDirectoryEntryByName(program);
        try {
            progEntry->setFlag(DirectoryEntry::Executable, true);
        }
        catch (...)
        { }
    }
}

// Export a subdirectory.
void
server::host::Exporter::exportSubdirectory(const String_t& source, const String_t& targetBase, const String_t& targetSub)
{
    // Create target
    Ref<DirectoryEntry> dirEntry = m_fileSystem.openDirectory(targetBase)->getDirectoryEntryByName(targetSub);
    dirEntry->createAsDirectory();

    // Copy
    m_log.write(afl::sys::LogListener::Trace, LOG_NAME, Format("Exporting host:%s -> %s (game)", source, dirEntry->getPathName()));
    server::file::FileSystemHandler targetHandler(m_fileSystem, dirEntry->getPathName());
    server::file::ClientDirectoryHandler sourceHandler(m_source, source);
    copyDirectory(targetHandler, sourceHandler, true);
}

// Store configuration file.
void
server::host::Exporter::storeConfigurationFile(const ConfigurationBuilder& ini, afl::io::Directory& parent)
{
    parent.openFile("c2host.ini", afl::io::FileSystem::Create)->fullWrite(ini.getContent());
}

// Import a subdirectory.
void
server::host::Exporter::importSubdirectory(const String_t& source, const String_t& targetBase, const String_t& targetSub)
{
    String_t targetName = m_fileSystem.makePathName(targetBase, targetSub);
    m_log.write(afl::sys::LogListener::Trace, LOG_NAME, Format("Importing host:%s <- %s (game)", source, targetName));

    server::file::FileSystemHandler targetHandler(m_fileSystem, targetName);
    server::file::ClientDirectoryHandler sourceHandler(m_source, source);

    // This synchronizes the target back into the source.
    synchronizeDirectories(sourceHandler, targetHandler);
}

// Import backups.
void
server::host::Exporter::importBackups(const String_t& source, const String_t& targetBase, const String_t& targetSub, bool unpackBackups)
{
    String_t targetName = m_fileSystem.makePathName(targetBase, targetSub);
    if (unpackBackups) {
        // Unpack.
        Ref<afl::base::Enumerator<Ptr<DirectoryEntry> > > targetContent(m_fileSystem.openDirectory(targetName)->getDirectoryEntries());
        Ptr<DirectoryEntry> targetEle;
        while (targetContent->getNextElement(targetEle) && targetEle.get() != 0) {
            String_t elementBaseName;
            if (targetEle->getFileType() == DirectoryEntry::tFile) {
                if (splitExtension(targetEle->getTitle(), ".tar.gz", elementBaseName)
                    || splitExtension(targetEle->getTitle(), ".tgz", elementBaseName))
                {
                    importTarball(source, elementBaseName, targetEle->openFile(afl::io::FileSystem::OpenRead));
                }
            }
        }
    } else {
        // Don't unpack; just copy (same logic as importSubdirectory, but using copyDirectory instead of synchronizeDirectories).
        m_log.write(afl::sys::LogListener::Trace, LOG_NAME, Format("Importing host:%s <- %s (backup)", source, targetName));
        server::file::FileSystemHandler targetHandler(m_fileSystem, targetName);
        server::file::ClientDirectoryHandler sourceHandler(m_source, source);
        copyDirectory(sourceHandler, targetHandler, false);
    }
}

// Import tarball.
void
server::host::Exporter::importTarball(const String_t& source, const String_t& tarballBase, afl::base::Ref<afl::io::Stream> tarball)
{
    // Set up for accessing the source [filer]
    server::interface::FileBaseClient sourceClient(m_source);
    String_t fullSource = Format("%s/%s", source, tarballBase);

    m_log.write(afl::sys::LogListener::Trace, LOG_NAME, Format("Importing host:%s <- %s (backup file)", fullSource, tarball->getName()));

    // Set up for reading a tarball
    afl::io::InflateTransform tx(afl::io::InflateTransform::Gzip);
    Ref<afl::io::Stream> reader(*new afl::io::TransformReaderStream(*tarball, tx));
    Ref<Directory> dir(afl::io::archive::TarReader::open(reader, 0));

    // Create directory in source [filer] and make sure it is empty
    sourceClient.createDirectoryTree(fullSource);
    server::file::ClientDirectoryHandler sourceHandler(m_source, fullSource);
    removeDirectoryContent(sourceHandler);

    // Copy content, one-by-one, but in lock-step order.
    // This is required to allow reading a .tar.gz (which does not support random access) on-the-fly.
    Ref<afl::base::Enumerator<Ptr<DirectoryEntry> > > tarballContent(dir->getDirectoryEntries());
    Ptr<DirectoryEntry> tarballEle;
    while (tarballContent->getNextElement(tarballEle) && tarballEle.get() != 0) {
        if (tarballEle->getFileType() == DirectoryEntry::tFile) {
            // Use a virtual file mapping. This gives maximum possiblities to avoid copying.
            Ref<afl::io::FileMapping> content = tarballEle->openFile(afl::io::FileSystem::OpenRead)->createVirtualMapping();
            sourceClient.putFile(Format("%s/%s", fullSource, tarballEle->getTitle()),
                                 afl::string::fromBytes(content->get()));
        }
    }
}
