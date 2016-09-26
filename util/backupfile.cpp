/**
  *  \file util/backupfile.cpp
  *  \brief Class util::BackupFile
  */

#include "util/backupfile.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/charset/utf8reader.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"

namespace {
    // FIXME: move this into a common place?
    /** Try to create a path. Creates a complete path that can contain multiple non-existant
        directory levels. This does not fail when the path cannot be created; in that case,
        subsequent operations using the path will fail.
        \param dirName Name of path to create */
    void tryCreatePath(afl::io::FileSystem& fs, const String_t dirName)
    {
        const String_t parentName = fs.getDirectoryName(dirName);
        const String_t childName  = fs.getFileName(dirName);

        // If parentName is the same as dirName, this means that dirName does not have a parent.
        // In this case, we don't do anything.
        if (parentName != dirName) {
            // Try enumerating the parent's content. If that fails, try to create it.
            // (openDir alone does not check whether the directory actually exists.)
            try {
                afl::base::Ptr<afl::io::Directory> parent = fs.openDirectory(parentName);
                parent->getDirectoryEntries();
            }
            catch (afl::except::FileProblemException&) {
                tryCreatePath(fs, parentName);
            }

            // Parent should now exist. Try creating child in it unless it already exists.
            try {
                afl::base::Ptr<afl::io::Directory> parent = fs.openDirectory(parentName);
                afl::base::Ptr<afl::io::DirectoryEntry> entry = parent->getDirectoryEntryByName(childName);
                if (entry->getFileType() != afl::io::DirectoryEntry::tDirectory) {
                    entry->createAsDirectory();
                }
            }
            catch (afl::except::FileProblemException&) { }
        }
    }

}

// Create a blank template.
util::BackupFile::BackupFile()
    : m_gameDirectory(),
      m_playerNumber(0),
      m_turnNumber(0)
{
    // ex GBackupFileTemplate::GBackupFileTemplate()
    // \change: PCC2 initialized player,turn to 1
}

util::BackupFile::~BackupFile()
{ }

// Configuration

// Set directory name for '%d' variable.
void
util::BackupFile::setGameDirectoryName(String_t dir)
{
    // ex GBackupFileTemplate::setGameDirectory
    m_gameDirectory = dir;
}

// Set player number for '%p' variable.
void
util::BackupFile::setPlayerNumber(int nr)
{
    // ex GBackupFileTemplate::setPlayerNumber
    m_playerNumber = nr;
}

// Set turn number for '%t' variable.
void
util::BackupFile::setTurnNumber(int nr)
{
    // ex GBackupFileTemplate::setTurnNumber
    m_turnNumber = nr;
}

// Templates & Backups

// Expand a template.
String_t
util::BackupFile::expandFileName(afl::io::FileSystem& fs, String_t tpl)
{
    // ex GBackupFileTemplate::expandFileName
    String_t result;
    afl::charset::Utf8Reader rdr(afl::string::toBytes(tpl), 0);
    afl::charset::Utf8 u8(0);
    while (rdr.hasMore()) {
         afl::charset::Unichar_t ch = rdr.eat();
         if (ch == '%' && rdr.hasMore()) {
             ch = rdr.eat();
             switch (ch) {
              case 'd': case 'D':
                 // Directory
                 if (result.empty()) {
                     // This is the first thing
                     result = m_gameDirectory;
                 } else {
                     // Not the first thing; append just the basename of game_dir
                     result += fs.getFileName(fs.getAbsolutePathName(m_gameDirectory));
                 }

                 // Make sure that the name ends in a path separator
                 if (!result.empty() && !fs.isPathSeparator(result[result.size()-1])) {
                     result = fs.makePathName(result, "");
                 }

                 // Make sure that the template does not produce another path separator
                 if (const uint8_t* next = rdr.getRemainder().at(0)) {
                     if (fs.isPathSeparator(char(*next))) {
                         rdr.eat();
                     }
                 }
                 break;
              case 'p': case 'P':
                 // Player number
                 result += afl::string::Format("%d", m_playerNumber);
                 break;
              case 't': case 'T':
                 // Turn number
                 result += afl::string::Format("%03d", m_turnNumber);
                 break;
              case '%':
              default:
                 u8.append(result, ch);
                 break;
             }
         } else {
             u8.append(result, ch);
         }
    }
    return result;
}

// Copy a file, using a template.
void
util::BackupFile::copyFile(afl::io::FileSystem& fs, String_t tpl, afl::io::Stream& src)
{
    // ex GBackupFileTemplate::copyFile
    // Anything to do?
    if (!tpl.empty()) {
        // Create directory for file
        const String_t name = expandFileName(fs, tpl);
        tryCreatePath(fs, fs.getDirectoryName(name));

        // Do it
        afl::base::Ptr<afl::io::Stream> file = fs.openFile(name, fs.Create);
        file->copyFrom(src);
    }
}

// Erase a file, using a template.
void
util::BackupFile::eraseFile(afl::io::FileSystem& fs, String_t tpl)
{
    // ex GBackupFileTemplate::eraseFile
    // Anything to do?
    if (!tpl.empty()) {
        // Erase the file
        const String_t name   = expandFileName(fs, tpl);
        const String_t parent = fs.getDirectoryName(name);
        const String_t child  = fs.getFileName(name);
        try {
            afl::base::Ptr<afl::io::Directory> dir = fs.openDirectory(parent);
            dir->eraseNT(child);
        }
        catch (afl::except::FileProblemException&) { }
    }
}

// Check existance of a file, using a template.
bool
util::BackupFile::hasFile(afl::io::FileSystem& fs, String_t tpl)
{
    if (tpl.empty()) {
        return false;
    } else {
        try {
            fs.openFile(expandFileName(fs, tpl), fs.OpenRead);
            return true;
        }
        catch (...) {
            return false;
        }
    }
}

afl::base::Ptr<afl::io::Stream>
util::BackupFile::openFile(afl::io::FileSystem& fs, String_t tpl)
{
    if (tpl.empty()) {
        throw afl::except::FileProblemException("<BackupFile>", _("No backup file configured"));
    } else {
        return fs.openFile(expandFileName(fs, tpl), fs.OpenRead);
    }
}
