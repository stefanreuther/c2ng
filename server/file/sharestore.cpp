/**
  *  \file server/file/sharestore.cpp
  *  \brief Class server::file::ShareStore
  */

#include "server/file/sharestore.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/root.hpp"
#include "util/string.hpp"

using afl::base::Ref;
using afl::io::ConstMemoryStream;
using afl::io::FileMapping;
using afl::io::InternalStream;
using afl::io::TextFile;
using afl::string::Format;
using afl::sys::LogListener;

namespace {
    const char*const LOG_NAME = "file.share";

    /** Name of directory where we store shares.
        Note that this must be a valid file name as per the DirectoryItem interface,
        and therefore cannot be a hidden file such as ".share". */
    const char*const SHARE_DIR_NAME = "_share";


    /** Check for user Id that we do share-tracking for.
        It needs to be a reasonable file name.
        In particular, "*" must not be a shareable user Id. */
    bool isShareableUserId(const String_t& userId)
    {
        if (userId.empty()) {
            return false;
        } else {
            for (size_t i = 0; i < userId.size(); ++i) {
                if (!(afl::string::charIsAlphanumeric(userId[i])
                      || userId[i] == '_'
                      || userId[i] == '-'))
                {
                    return false;
                }
            }
            return true;
        }
    }
}

struct server::file::ShareStore::Info {
    String_t pathName;
    String_t owner;
    int32_t seqNr;
};

server::file::ShareStore::ShareStore()
    : m_shares(),
      m_sequenceNumber(),
      m_changed()
{ }

server::file::ShareStore::~ShareStore()
{ }

void
server::file::ShareStore::load(String_t userId, Root& root)
{
    try {
        // User name filter
        if (!isShareableUserId(userId)) {
            return;
        }

        // Find folder
        root.rootDirectory().readContent(root);
        DirectoryItem* shareFolder = root.rootDirectory().findDirectory(SHARE_DIR_NAME);
        if (shareFolder == 0) {
            return;
        }

        // Find user Id
        shareFolder->readContent(root);
        FileItem* file = shareFolder->findFile(userId);
        if (file == 0) {
            return;
        }

        // Load
        Ref<FileMapping> content = shareFolder->getFileContent(*file);
        ConstMemoryStream contentStream(content->get());
        TextFile contentFile(contentStream);
        String_t line;
        while (contentFile.readLine(line)) {
            handleLine(userId, root, line);
        }
    }
    catch (std::exception& e) {
        root.log().write(LogListener::Warn, LOG_NAME, Format("[user %s] error reading shares", userId), e);
    }
}

void
server::file::ShareStore::save(String_t userId, Root& root) const
{
    try {
        // User name filter
        if (!isShareableUserId(userId)) {
            return;
        }

        // Save
        InternalStream contentFile;
        saveContent(contentFile);

        // Find/create directory
        root.rootDirectory().readContent(root);
        DirectoryItem* shareFolder = root.rootDirectory().findDirectory(SHARE_DIR_NAME);
        if (shareFolder == 0) {
            shareFolder = root.rootDirectory().createDirectory(SHARE_DIR_NAME);
        }

        // Save
        shareFolder->createFile(userId, contentFile.getContent());
    }
    catch (std::exception& e) {
        root.log().write(LogListener::Warn, LOG_NAME, Format("[user %s] error saving shares", userId), e);
    }
}

void
server::file::ShareStore::saveIfModified(String_t userId, Root& root)
{
    if (m_changed) {
        save(userId, root);
        m_changed = false;
    }
}

int32_t
server::file::ShareStore::getSequenceNumber() const
{
    return m_sequenceNumber;
}

void
server::file::ShareStore::addShare(String_t pathName, String_t owner)
{
    // Changing the owner of a share is not a supported usecase.
    // For now, we keep the owner upon initial creation.
    size_t idx = findShareByPathName(pathName);
    if (idx >= m_shares.size()) {
        Info* p = m_shares.pushBackNew(new Info());
        p->pathName = pathName;
        p->owner = owner;
        p->seqNr = ++m_sequenceNumber;
        m_changed = true;
    }
}

void
server::file::ShareStore::removeShare(String_t pathName)
{
    size_t idx = findShareByPathName(pathName);
    if (idx < m_shares.size()) {
        m_shares.erase(m_shares.begin() + idx);
        m_changed = true;
    }
}

size_t
server::file::ShareStore::getNumShares() const
{
    return m_shares.size();
}

const server::file::ShareStore::Info*
server::file::ShareStore::getShareByIndex(size_t idx) const
{
    if (idx < m_shares.size()) {
        return m_shares[idx];
    } else {
        return 0;
    }
}

String_t
server::file::ShareStore::getSharePathName(const Info* p) const
{
    return p != 0 ? p->pathName : String_t();
}

String_t
server::file::ShareStore::getShareOwner(const Info* p) const
{
    return p != 0 ? p->owner : String_t();
}

int32_t
server::file::ShareStore::getShareSequenceNumber(const Info* p) const
{
    return p != 0 ? p->seqNr : 0;
}

size_t
server::file::ShareStore::findShareByPathName(const String_t& pathName) const
{
    size_t i = 0;
    while (i < m_shares.size() && m_shares[i]->pathName != pathName) {
        ++i;
    }
    return i;
}

void
server::file::ShareStore::handleLine(const String_t& userId, Root& root, const String_t& line)
{
    bool ok;
    if (const char* p = util::strStartsWith(line, "SEQ=")) {
        ok = afl::string::strToInteger(p, m_sequenceNumber);
    } else if (const char* p = util::strStartsWith(line, "ENTRY=")) {
        m_shares.pushBackNew(new Info())
            ->pathName = p;
        ok = true;
    } else if (const char* p = util::strStartsWith(line, "EOWNER=")) {
        ok = !m_shares.empty();
        if (ok) {
            m_shares.back()->owner = p;
        }
    } else if (const char* p = util::strStartsWith(line, "ESEQ=")) {
        ok = !m_shares.empty()
            && afl::string::strToInteger(p, m_shares.back()->seqNr);
    } else {
        // ignore
        ok = false;
    }
    if (!ok) {
        root.log().write(LogListener::Warn, LOG_NAME, Format("[user %s] invalid line: %s", userId, line));
    }
}

void
server::file::ShareStore::saveContent(afl::io::Stream& out) const
{
    TextFile tf(out);
    tf.setSystemNewline(false);
    tf.writeLine(Format("SEQ=%d", m_sequenceNumber));
    for (size_t i = 0, n = m_shares.size(); i < n; ++i) {
        const Info* p = m_shares[i];
        tf.writeLine(Format("ENTRY=%s", p->pathName));
        tf.writeLine(Format("EOWNER=%s", p->owner));
        tf.writeLine(Format("ESEQ=%d", p->seqNr));
    }
    tf.flush();
}
