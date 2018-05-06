/**
  *  \file util/directorybrowser.cpp
  *  \brief Class util::DirectoryBrowser
  */

#include "util/directorybrowser.hpp"
#include "afl/string/string.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"

namespace {
    using afl::io::Directory;
    using afl::io::DirectoryEntry;
    using afl::base::Ref;
    using afl::base::Ptr;

    bool isDirLT(const util::DirectoryBrowser::DirectoryItem& a, const util::DirectoryBrowser::DirectoryItem& b)
    {
        // ex UIFileChooser::isDirLT
        return afl::string::strCaseCompare(a.dir->getTitle(), b.dir->getTitle()) < 0;
    }

    bool isEntryLT(const Ptr<DirectoryEntry>& a, const Ptr<DirectoryEntry>& b)
    {
        // ex UIFileChooser::isEntryLT
        return afl::string::strCaseCompare(a->getTitle(), b->getTitle()) < 0;
    }
}

util::DirectoryBrowser::DirectoryBrowser(afl::io::FileSystem& fs)
    : m_fileSystem(fs),
      m_path(),
      m_pathOrigin(),
      m_directories(),
      m_files(),
      m_selectedDirectory(),
      m_error(),
      m_patterns(),
      m_acceptHiddenEntries(false)
{ }

util::DirectoryBrowser::~DirectoryBrowser()
{ }

void
util::DirectoryBrowser::addFileNamePattern(const FileNamePattern& pat)
{
    m_patterns.push_back(pat);
}

void
util::DirectoryBrowser::clearFileNamePatterns()
{
    m_patterns.clear();
}

void
util::DirectoryBrowser::setAcceptHiddenEntries(bool enable)
{
    m_acceptHiddenEntries = enable;
}

void
util::DirectoryBrowser::openDirectory(String_t name)
{
    // Get absolute, canonical path (resolve ".." etc.)
    // FIXME: should this be relative to current directory of the browser?
    name = m_fileSystem.getAbsolutePathName(name);

    // Are we going up?
    size_t i = 0;
    while (i < m_path.size() && name != m_path.back()->getDirectoryName()) {
        ++i;
    }

    if (i < m_path.size()) {
        // Found
        if (i == m_path.size() - 1) {
            // We are staying at the directory we are at. Just reset cursor.
            m_pathOrigin.reset();
        } else {
            // We're going up to a parent
            m_pathOrigin = m_path[i+1];
        }
    } else {
        // Not found. Build a new path.
        std::vector<DirectoryPtr_t> vec;
        for (DirectoryPtr_t dir = m_fileSystem.openDirectory(name).asPtr(); dir.get() != 0; dir = dir->getParentDirectory()) {
            vec.push_back(dir);
        }

        // Note that path has been built in wrong order; reverse
        m_path.assign(vec.rbegin(), vec.rend());
        m_pathOrigin.reset();
    }
    loadContent();
}

void
util::DirectoryBrowser::openChild(size_t n)
{
    if (n < m_directories.size()) {
        if (m_path.empty()) {
            openDirectory(m_directories[n].dir->getDirectoryName());
        } else {
            m_path.push_back(m_directories[n].dir);
            m_pathOrigin.reset();
            loadContent();
        }
    }
}

void
util::DirectoryBrowser::openParent()
{
    if (!m_path.empty()) {
        m_pathOrigin = m_path.back();
        m_path.pop_back();
        loadContent();
    }
}

void
util::DirectoryBrowser::openRoot()
{
    if (!m_path.empty()) {
        m_pathOrigin = m_path[0];
        m_path.clear();
        loadContent();
    }
}

void
util::DirectoryBrowser::selectChild(size_t n)
{
    if (n < m_directories.size()) {
        m_selectedDirectory = n;
    }
}

String_t
util::DirectoryBrowser::createDirectory(String_t name)
{
    // Verify
    if (name.empty() || m_fileSystem.getFileName(name) != name) {
        return _("Invalid directory name");
    }

    // Create
    DirectoryPtr_t ptr;
    try {
        DirectoryEntryPtr_t e = getCurrentDirectory()->getDirectoryEntryByName(name).asPtr();
        e->createAsDirectory();
        ptr = e->openDirectory().asPtr();
    }
    catch (std::exception& e) {
        return e.what();
    }

    // Add to content
    m_directories.push_back(DirectoryItem(ptr, name));
    if (!m_path.empty()) {
        std::sort(m_directories.begin(), m_directories.end(), isDirLT);
    }

    // Select it
    for (size_t i = 0, n = m_directories.size(); i < n; ++i) {
        if (m_directories[i].title == name) {
            m_selectedDirectory = i;
            break;
        }
    }

    // Success
    return String_t();
}

afl::base::Ref<afl::io::Directory>
util::DirectoryBrowser::getCurrentDirectory() const
{
    if (m_path.empty()) {
        return m_fileSystem.openRootDirectory();
    } else {
        return *m_path.back();
    }
}

void
util::DirectoryBrowser::loadContent()
{
    // ex UIFileChooser::scanDirectory (sort-of)
    using afl::io::DirectoryEntry;
    m_files.clear();
    m_directories.clear();
    m_error.clear();

    // Read directory content
    try {
        afl::base::Ref<afl::base::Enumerator<DirectoryEntryPtr_t> > e = getCurrentDirectory()->getDirectoryEntries();
        DirectoryEntryPtr_t p;
        while (e->getNextElement(p)) {
            if (p.get() != 0 && (m_acceptHiddenEntries || !p->getFlags().contains(DirectoryEntry::Hidden))) {
                switch (p->getFileType()) {
                 case DirectoryEntry::tDirectory:
                 case DirectoryEntry::tRoot:
                    m_directories.push_back(DirectoryItem(p->openDirectory().asPtr(), p->getTitle()));
                    break;

                 case DirectoryEntry::tFile:
                 case DirectoryEntry::tArchive:
                    if (acceptFile(p->getTitle())) {
                        m_files.push_back(p);
                    }
                    break;

                 case DirectoryEntry::tUnknown:
                 case DirectoryEntry::tDevice:
                 case DirectoryEntry::tOther:
                    break;
                }
            }
        }

        // Sort only if we're not the root.
        // For the root, assume that root knows the correct order.
        if (!m_path.empty()) {
            std::sort(m_directories.begin(), m_directories.end(), isDirLT);
            std::sort(m_files.begin(), m_files.end(), isEntryLT);
        }

        // Place cursor
        m_selectedDirectory.clear();
        if (m_pathOrigin.get() != 0) {
            String_t name = m_pathOrigin->getDirectoryName();
            for (size_t i = 0, n = m_directories.size(); i < n; ++i) {
                if (m_directories[i].dir->getDirectoryName() == name) {
                    m_selectedDirectory = i;
                    break;
                }
            }
        }
    }
    catch (std::exception& e) {
        // No need to separately catch FileProblemException:
        // The file name we're having problems with will be implicit from the browser position.
        m_error = e.what();
    }

    // Reset origin.
    // FIXME: pass it as parameter? Only game::browser needs it as member.
    m_pathOrigin.reset();
}

const std::vector<util::DirectoryBrowser::DirectoryPtr_t>&
util::DirectoryBrowser::path() const
{
    return m_path;
}

const std::vector<util::DirectoryBrowser::DirectoryItem>&
util::DirectoryBrowser::directories() const
{
    return m_directories;
}

const std::vector<util::DirectoryBrowser::DirectoryEntryPtr_t>&
util::DirectoryBrowser::files() const
{
    return m_files;
}

util::DirectoryBrowser::OptionalIndex_t
util::DirectoryBrowser::getSelectedChild() const
{
    return m_selectedDirectory;
}

String_t
util::DirectoryBrowser::getErrorText() const
{
    return m_error;
}

bool
util::DirectoryBrowser::acceptFile(const String_t& name) const
{
    for (size_t i = 0, n = m_patterns.size(); i < n; ++i) {
        if (m_patterns[i].match(name)) {
            return true;
        }
    }
    return false;
}
