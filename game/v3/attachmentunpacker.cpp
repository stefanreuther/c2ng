/**
  *  \file game/v3/attachmentunpacker.cpp
  *  \brief Class game::v3::AttachmentUnpacker
  */

#include <stdexcept>
#include "game/v3/attachmentunpacker.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/udata/reader.hpp"
#include "util/fileparser.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"

namespace gt = game::v3::structures;

using afl::charset::Utf8Charset;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;
using afl::sys::LogListener;
using game::v3::AttachmentUnpacker;

namespace {
    const char* LOG_NAME = "game.v3.unpack";

    struct Map {
        const char* tpl;
        AttachmentUnpacker::Kind kind;
    };

    /* File name mapping:

       Everything not listed here is NormalFile (e.g. xtrfcode.txt, hullspec.dat).

       A CriticalFile is not unpacked by default.
       We need to guess whether a file is used legitimately.
       An attempt to overwrite a file that comes with a result normally is definitely fishy.
       Updating spec files can be fishy, but is also legitimate.
       If in doubt, we assume that host is somehow trusted.
       (They could as well send the file updates within a .zip.)


       Placeholders:

       "*" matches anything up to the next "." ("*foo" will not work),
       "#" matches any sequence of numbers.

       We do not match numeric placeholders to the current player number;
       although player3.rst never contains a ship4.dat, we still reject that. */
    const Map KIND_MAP[] = {
        { "*.bat",            AttachmentUnpacker::CriticalFile },         // also blacklisted by VPA
        { "*.com",            AttachmentUnpacker::CriticalFile },         // also blacklisted by VPA
        { "*.exe",            AttachmentUnpacker::CriticalFile },         // also blacklisted by VPA
        { "*.ovr",            AttachmentUnpacker::CriticalFile },         // also blacklisted by VPA
        { "*.q",              AttachmentUnpacker::CriticalFile },
        { "*.qc",             AttachmentUnpacker::CriticalFile },
        { "*.src",            AttachmentUnpacker::ConfigurationFile },
        { "*.sys",            AttachmentUnpacker::CriticalFile },         // also blacklisted by VPA
        { "bdata#.dat",       AttachmentUnpacker::CriticalFile },
        { "bdata#.dis",       AttachmentUnpacker::CriticalFile },
        { "cmd#.txt",         AttachmentUnpacker::CriticalFile },
        { "config#.cc",       AttachmentUnpacker::CriticalFile },
        { "contrl#.dat",      AttachmentUnpacker::CriticalFile },
        { "control.dat",      AttachmentUnpacker::CriticalFile },
        { "cp#.cc",           AttachmentUnpacker::CriticalFile },
        { "fizz.bin",         AttachmentUnpacker::CriticalFile },
        { "gen#.dat",         AttachmentUnpacker::CriticalFile },
        { "hconfig.hst",      AttachmentUnpacker::ConfigurationFile },
        { "kore#.dat",        AttachmentUnpacker::CriticalFile },
        { "mdata#.dat",       AttachmentUnpacker::CriticalFile },
        { "mess#.dat",        AttachmentUnpacker::CriticalFile },
        { "mess35#.dat",      AttachmentUnpacker::CriticalFile },
        { "pcc2.ini",         AttachmentUnpacker::CriticalFile },
        { "pdata#.dat",       AttachmentUnpacker::CriticalFile },
        { "pdata#.dis",       AttachmentUnpacker::CriticalFile },
        { "player#.rst",      AttachmentUnpacker::CriticalFile },
        { "player#.trn",      AttachmentUnpacker::CriticalFile },
        { "race.nm",          AttachmentUnpacker::RaceNameFile },
        { "ship#.dat",        AttachmentUnpacker::CriticalFile },
        { "ship#.dis",        AttachmentUnpacker::CriticalFile },
        { "shiplist.txt",     AttachmentUnpacker::ConfigurationFile },
        { "shipxy#.dat",      AttachmentUnpacker::CriticalFile },
        { "skore#.dat",       AttachmentUnpacker::CriticalFile },
        { "target#.dat",      AttachmentUnpacker::CriticalFile },
        { "target#.ext",      AttachmentUnpacker::CriticalFile },
        { "util#.dat",        AttachmentUnpacker::CriticalFile },
        { "vcr#.dat",         AttachmentUnpacker::CriticalFile },
    };

    bool matchName(const String_t& name, const char* tpl)
    {
        util::StringParser p(name);
        while (char ch = *tpl++) {
            if (ch == '*') {
                String_t tmp;
                p.parseDelimGreedy(".", tmp);
            } else if (ch == '#') {
                String_t tmp;
                if (!p.parseWhile(afl::string::charIsDigit, tmp)) {
                    return false;
                }
            } else {
                if (!p.parseCharacter(ch)) {
                    return false;
                }
            }
        }
        return p.parseEnd();
    }

    /* Get default kind for a file, given its name. */
    AttachmentUnpacker::Kind getFileKind(const String_t& name)
    {
        for (size_t i = 0; i < countof(KIND_MAP); ++i) {
            if (matchName(name, KIND_MAP[i].tpl)) {
                return KIND_MAP[i].kind;
            }
        }
        return AttachmentUnpacker::NormalFile;
    }

    /* Check for invalid characters in a file name.
       We reject anything that looks like it contains a directory separator,
       and characters that are not file-system safe (i.e. non-ASCII).
       For now, we also reject space which shouldn't appear in VGAP files. */
    bool hasInvalidCharacter(const String_t& name)
    {
        for (size_t i = 0, n = name.size(); i < n; ++i) {
            uint8_t ch = static_cast<uint8_t>(name[i]);
            if (ch == '\\' || ch == '/' || ch == ':' || ch >= 0x80 || ch <= ' ') {
                return true;
            }
        }
        return false;
    }

    String_t canonicalizeFileName(String_t name)
    {
        return afl::string::strTrim(afl::string::strLCase(name));
    }

    /* Check whether file has given content. */
    bool hasSameContent(afl::io::Directory& dir, const String_t& name, afl::base::ConstBytes_t content)
    {
        try {
            afl::base::Ref<Stream> s = dir.openFile(name, FileSystem::OpenRead);
            while (1) {
                // Read a page
                uint8_t buffer[4096];
                afl::base::Bytes_t bytes(buffer);
                size_t read = s->read(bytes);
                bytes.trim(read);

                // If input ends, files are equal if expected content also ends
                if (read == 0) {
                    return content.empty();
                }

                // Compare data. If expected content ends before input, this will yield false due to mismatching length.
                if (!content.split(read).equalContent(bytes)) {
                    return false;
                }
            }
        }
        catch (...) {
            return false;
        }
    }

    /* Check for valid racename file.
       It seems the race.nm section in a result file is sometimes blank; avoid damaging user's files. */
    bool isValidRaceNames(afl::base::ConstBytes_t buffer)
    {
        while (const uint8_t* p = buffer.eat()) {
            if (*p > 32) {
                return true;
            }
        }
        return false;
    }
}

/*
 *  Representation of an Attachment
 */

struct game::v3::AttachmentUnpacker::Attachment {
    String_t name;
    Kind kind;
    bool enabled;
    afl::base::GrowableBytes_t content;
};


/*
 *  util.dat Reader for Attachment Extraction
 */

class game::v3::AttachmentUnpacker::Reader : public game::v3::udata::Reader {
 public:
    Reader(AttachmentUnpacker& parent, afl::sys::LogListener& log, afl::string::Translator& tx)
        : m_parent(parent), m_log(log), m_translator(tx), m_openAttachment(0)
        { }
    virtual bool handleRecord(uint16_t recordId, afl::base::ConstBytes_t data)
        {
            switch (recordId) {
             case 27:
                // Raw pconfig.src
                if (Attachment* a = m_parent.createAttachment("pconfig.src", m_log, m_translator)) {
                    a->content.resize(data.size());
                    a->content.copyFrom(data);
                }
                break;

             case 34:
                // General file
                if (data.size() >= sizeof(gt::Util34FTP)) {
                    // Fetch header
                    gt::Util34FTP header;
                    afl::base::fromObject(header).copyFrom(data.split(sizeof(gt::Util34FTP)));

                    // Build attachment
                    if (Attachment* a = m_parent.createAttachment(Utf8Charset().decode(header.fileName), m_log, m_translator)) {
                        a->content.resize(data.size());
                        a->content.copyFrom(data);
                    }
                }
                break;

             case 59:
                // Long file
                if (data.size() >= sizeof(gt::Util59FTP)) {
                    // Fetch header and file name
                    gt::Util59FTP header;
                    afl::base::fromObject(header).copyFrom(data.split(sizeof(header)));
                    if (header.fileNameLength > data.size()) {
                        m_log.write(LogListener::Debug, LOG_NAME, m_translator("Attachment record is truncated"));
                        break;
                    }

                    const String_t fileName = Utf8Charset().decode(data.split(header.fileNameLength));

                    // Sort it to its place
                    if ((header.flags & gt::FTP_NOTFIRST) != 0) {
                        // It's not the first one, so there needs to be a matching open attachment
                        if (m_openAttachment == 0 || m_openAttachment->name != canonicalizeFileName(fileName)) {
                            m_log.write(LogListener::Debug, LOG_NAME, Format(m_translator("Attachment \"%s\" is missing parts."), fileName));
                            break;
                        }
                    } else {
                        // It is the first one, so there shouldn't be an open one
                        closeAttachment();
                        if (Attachment* a = m_parent.createAttachment(fileName, m_log, m_translator)) {
                            a->content.clear();
                            m_openAttachment = a;
                        }
                    }

                    // Append
                    if (m_openAttachment != 0) {
                        m_openAttachment->content.append(data);
                    }

                    // Finish attachment
                    if ((header.flags & gt::FTP_NOTLAST) == 0) {
                        m_openAttachment = 0;
                    }
                }
                break;
            }
            return true;
        }
    virtual void handleError(afl::io::Stream& /*in*/)
        { }
    virtual void handleEnd()
        { closeAttachment(); }

 private:
    void closeAttachment()
        {
            if (m_openAttachment != 0) {
                m_log.write(LogListener::Debug, LOG_NAME, Format(m_translator("Attachment \"%s\" is missing parts."), m_openAttachment->name));
                m_openAttachment = 0;
            }
        }

    AttachmentUnpacker& m_parent;
    afl::sys::LogListener& m_log;
    afl::string::Translator& m_translator;
    Attachment* m_openAttachment;
};



/*
 *  AttachmentUnpacker
 */

game::v3::AttachmentUnpacker::AttachmentUnpacker()
    : m_acceptableKinds(Kinds_t::fromInteger(-1) - CriticalFile),
      m_timestamp(),
      m_attachments()
{ }

game::v3::AttachmentUnpacker::~AttachmentUnpacker()
{ }

void
game::v3::AttachmentUnpacker::setAcceptableKind(Kind k, bool enable)
{
    m_acceptableKinds.set(k, enable);
}

void
game::v3::AttachmentUnpacker::loadDirectory(afl::io::Directory& dir, int playerNumber, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    afl::base::Ptr<Stream> in = dir.openFileNT(Format("player%d.rst", playerNumber), FileSystem::OpenRead);
    if (in.get() != 0) {
        try {
            loadResultFile(*in, playerNumber, log, tx);
        }
        catch (std::exception& e) {
            log.write(LogListener::Warn, LOG_NAME, tx("Error reading file"), e);
        }
    }

    in = dir.openFileNT(Format("util%d.dat", playerNumber), FileSystem::OpenRead);
    if (in.get() != 0) {
        try {
            loadUtilData(*in, playerNumber, log, tx);
        }
        catch (std::exception& e) {
            log.write(LogListener::Warn, LOG_NAME, tx("Error reading file"), e);
        }
    }
}

void
game::v3::AttachmentUnpacker::loadResultFile(afl::io::Stream& in, int playerNumber, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    ResultFile rst(in, tx);

    // Check timestamp
    rst.seekToSection(ResultFile::GenSection);
    gt::ResultGen gen;
    in.fullRead(afl::base::fromObject(gen));
    if (!checkTimestamp(gen.timestamp)) {
        return;
    }

    // Check for race.nm file in KORE section
    Stream::FileSize_t offset;
    if (rst.getSectionOffset(ResultFile::KoreSection, offset)) {
        in.setPos(offset
                  + 500 * sizeof(gt::KoreMine)
                  + 50 * sizeof(gt::KoreStorm)
                  + 50 * sizeof(gt::KoreExplosion));

        uint8_t raceNameBuffer[sizeof(gt::RaceNames)];
        in.fullRead(raceNameBuffer);

        if (isValidRaceNames(raceNameBuffer)) {
            if (Attachment* a = createAttachment("race.nm", log, tx)) {
                a->content.resize(sizeof(raceNameBuffer));
                a->content.copyFrom(raceNameBuffer);
            }
        }
    }

    // Check for LEECH section (probably nobody uses this today)
    if (rst.hasSection(ResultFile::LeechSection)) {
        rst.seekToSection(ResultFile::LeechSection);

        afl::bits::Value<afl::bits::UInt32LE> packedSize;
        in.fullRead(packedSize.m_bytes);

        uint32_t size = packedSize;
        if (size_t(size) == size) {
            if (Attachment* a = createAttachment(Format("leech%d.dat", playerNumber), log, tx)) {
                a->content.resize(size);
                in.fullRead(a->content);
            }
        }
    }
}

void
game::v3::AttachmentUnpacker::loadUtilData(afl::io::Stream& in, int /*playerNumber*/, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    Timestamp ts;
    if (Reader::check(in, &ts)) {
        if (checkTimestamp(ts)) {
            Reader(*this, log, tx).read(in);
        }
    }
}

void
game::v3::AttachmentUnpacker::dropUnchangedFiles(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    size_t o = 0;
    for (size_t i = 0; i < m_attachments.size(); ++i) {
        Attachment* p = m_attachments[i];
        if (!hasSameContent(dir, p->name, p->content)) {
            m_attachments.swapElements(i, o);
            ++o;
        } else {
            log.write(LogListener::Trace, LOG_NAME, Format(tx("File \"%s\" is unchanged."), p->name));
        }
    }
    m_attachments.resize(o);
}

void
game::v3::AttachmentUnpacker::dropUnselectedAttachments()
{
    size_t o = 0;
    for (size_t i = 0; i < m_attachments.size(); ++i) {
        Attachment* p = m_attachments[i];
        if (p->enabled) {
            m_attachments.swapElements(i, o);
            ++o;
        }
    }
    m_attachments.resize(o);
}

bool
game::v3::AttachmentUnpacker::saveFiles(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    bool ok = true;
    for (size_t i = 0; i < m_attachments.size(); ++i) {
        Attachment* p = m_attachments[i];
        if (p->enabled) {
            try {
                // Erase, so we're not surprised if it's a symlink
                dir.eraseNT(p->name);

                // Write
                dir.openFile(p->name, FileSystem::Create)
                    ->fullWrite(p->content);

                log.write(LogListener::Info, LOG_NAME, Format(tx("Received attachment \"%s\"."), p->name));
            }
            catch (std::exception& e) {
                log.write(LogListener::Error, LOG_NAME, tx("Unable to write attachment"), e);
                ok = false;
            }
        } else {
            log.write(LogListener::Debug, LOG_NAME, Format(tx("Attachment \"%s\" ignored."), p->name));
        }
    }
    return ok;
}

size_t
game::v3::AttachmentUnpacker::getNumAttachments() const
{
    return m_attachments.size();
}

game::v3::AttachmentUnpacker::Attachment*
game::v3::AttachmentUnpacker::getAttachmentByIndex(size_t i) const
{
    if (i < m_attachments.size()) {
        return m_attachments[i];
    } else {
        return 0;
    }
}

game::v3::AttachmentUnpacker::Attachment*
game::v3::AttachmentUnpacker::getAttachmentByName(const String_t& name) const
{
    for (size_t i = 0, n = m_attachments.size(); i < n; ++i) {
        if (m_attachments[i]->name == name) {
            return m_attachments[i];
        }
    }
    return 0;
}

game::v3::AttachmentUnpacker::Kind
game::v3::AttachmentUnpacker::getAttachmentKind(Attachment* p) const
{
    return p != 0
        ? p->kind
        : NormalFile;
}

void
game::v3::AttachmentUnpacker::selectAttachment(Attachment* p, bool enable)
{
    if (p != 0) {
        p->enabled = enable;
    }
}

void
game::v3::AttachmentUnpacker::selectAttachmentsByKind(Kind k, bool enable)
{
    for (size_t i = 0, n = m_attachments.size(); i < n; ++i) {
        if (m_attachments[i]->kind == k) {
            m_attachments[i]->enabled = enable;
        }
    }
}

void
game::v3::AttachmentUnpacker::selectAllAttachments(bool enable)
{
    for (size_t i = 0, n = m_attachments.size(); i < n; ++i) {
        m_attachments[i]->enabled = enable;
    }
}

bool
game::v3::AttachmentUnpacker::isAttachmentSelected(Attachment* p) const
{
    return p != 0 && p->enabled;
}

String_t
game::v3::AttachmentUnpacker::getAttachmentName(Attachment* p) const
{
    return p != 0
        ? p->name
        : String_t();
}

size_t
game::v3::AttachmentUnpacker::getAttachmentSize(Attachment* p) const
{
    return p != 0
        ? p->content.size()
        : 0;
}

game::v3::AttachmentUnpacker::Kinds_t
game::v3::AttachmentUnpacker::getAllAttachmentKinds() const
{
    Kinds_t result;
    for (size_t i = 0, n = m_attachments.size(); i < n; ++i) {
        result += m_attachments[i]->kind;
    }
    return result;
}

const game::Timestamp&
game::v3::AttachmentUnpacker::getTimestamp() const
{
    return m_timestamp;
}

String_t
game::v3::AttachmentUnpacker::toString(Kind k, afl::string::Translator& tx)
{
    String_t result;
    switch (k) {
     case NormalFile:        result = tx("File");               break;
     case ConfigurationFile: result = tx("Configuration File"); break;
     case RaceNameFile:      result = tx("Race Names");         break;
     case CriticalFile:      result = tx("Dangerous File");     break;
    }
    return result;
}

game::v3::AttachmentUnpacker::Attachment*
game::v3::AttachmentUnpacker::createAttachment(String_t name, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // Canonicalize name
    name = canonicalizeFileName(name);

    // Refuse invalid names
    if (name.empty() || name[0] == '.' || hasInvalidCharacter(name)) {
        log.write(LogListener::Debug, LOG_NAME, Format(tx("Attachment \"%s\" ignored because of invalid file name."), util::sanitizeString(name)));
        return 0;
    }

    // Look for existing attachment (or insert position)
    size_t i = 0;
    while (i < m_attachments.size() && util::strCollate(name, m_attachments[i]->name) > 0) {
        ++i;
    }

    // Existing attachment?
    if (i < m_attachments.size() && name == m_attachments[i]->name) {
        return m_attachments[i];
    }

    // Create new attachment
    Attachment* na = m_attachments.insertNew(m_attachments.begin() + i, new Attachment());
    na->name = name;
    na->kind = getFileKind(name);
    na->enabled = m_acceptableKinds.contains(na->kind);
    return na;
}

bool
game::v3::AttachmentUnpacker::checkTimestamp(const Timestamp& ts)
{
    if (!ts.isValid() || (m_timestamp.isValid() && ts.isEarlierThan(m_timestamp))) {
        return false;
    }

    if (ts != m_timestamp) {
        m_attachments.clear();
        m_timestamp = ts;
    }
    return true;
}
