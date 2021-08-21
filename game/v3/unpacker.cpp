/**
  *  \file game/v3/unpacker.cpp
  *  \brief Class game::v3::Unpacker
  *
  *  FIXME: this has some code duplication to stuff elsewhere, such as target decoding, index file, etc.
  *  It's probably not entirely worth to factor that out.
  *
  *  FIXME: can we template the hell out of this to remove the ship/planet/base code duplication?
  */

#include "game/v3/unpacker.hpp"
#include "afl/base/inlinememory.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/string/format.hpp"
#include "game/v3/registry.hpp"
#include "game/v3/resultfile.hpp"
#include "util/translation.hpp"
#include "game/v3/writer.hpp"
#include "game/v3/utils.hpp"
#include "afl/charset/codepage.hpp"

namespace gt = game::v3::structures;
using afl::base::GrowableMemory;
using afl::base::Memory;
using afl::base::Ref;
using afl::string::Format;
using afl::except::FileFormatException;
using afl::io::Stream;
using afl::io::FileSystem;
using afl::checksums::ByteSum;
using game::v3::structures::Int16_t;
using game::v3::structures::Int32_t;

namespace {
    const char* LOG_NAME = "game.v3.unpack";

    uint32_t getChecksum(Memory<const uint8_t> mem)
    {
        return ByteSum().add(mem, 0);
    }

    gt::Ship* findShip(Memory<gt::Ship> ships, int id)
    {
        while (gt::Ship* p = ships.eat()) {
            if (p->shipId == id) {
                return p;
            }
        }
        return 0;
    }

    gt::Planet* findPlanet(Memory<gt::Planet> planets, int id)
    {
        while (gt::Planet* p = planets.eat()) {
            if (p->planetId == id) {
                return p;
            }
        }
        return 0;
    }

    gt::Base* findBase(Memory<gt::Base> bases, int id)
    {
        while (gt::Base* p = bases.eat()) {
            if (p->baseId == id) {
                return p;
            }
        }
        return 0;
    }

    /** Copy ship record and fix errors. Copies in into out. Fixes that
        can be transmitted to the host will be fixed only in out, fixes
        that cannot be transmitted to the host will be fixed in both. */
    void copyAndFix(gt::Ship& out, gt::Ship& in, bool fixErrors)
    {
        // ex GUnpacker::copyAndFix
        // Fix errors that apply to both halves
        if (fixErrors) {
            // Dominate(?) creates ships with zero launchers of type X. Some clients
            // including ancient PCC versions erroneously treat these as torpers.
            if (in.numLaunchers == 0) {
                in.launcherType = 0;
            }
        }

        // Copy
        out = in;

        // Fix errors that apply to only one half. Maketurn will send those to the
        // host, fixing the problem in the host files as well.
        if (fixErrors) {
            // THost generates ships with negative warp when they have >100% damage.
            if (out.warpFactor < 0 || out.warpFactor > 10) {
                out.warpFactor = 0;
            }
            // Some programs generate nulls in string names. Convert to canonical format.
            out.name = afl::string::toBytes(afl::string::fromBytes(out.name));
            out.friendlyCode = afl::string::toBytes(afl::string::fromBytes(out.friendlyCode));
        }
    }

    /** Copy planet record and fix errors. Copies in into out. Fixes that
        can be transmitted to the host will be fixed only in dat, fixes
        that cannot be transmitted to the host will be fixed in both. */
    void copyAndFix(gt::Planet& out, gt::Planet& in, bool fixErrors)
    {
        // ex GUnpacker::copyAndFix
        // Copy
        out = in;

        // Fix errors
        if (fixErrors) {
            // Strings with embedded nulls
            out.friendlyCode = afl::string::toBytes(afl::string::fromBytes(out.friendlyCode));
        }
    }

    /** Copy starbase record and fix errors. Copies dat into dis. Fixes that
        can be transmitted to the host will be fixed only in dat, fixes
        that cannot be transmitted to the host will be fixed in both. */
    void copyAndFix(gt::Base& out, gt::Base& in, bool /*fixErrors*/)
    {
        // ex GUnpacker::copyAndFix
        out = in;
    }

    template<typename T>
    void copyBuffer(GrowableMemory<T>& out, Memory<T> in, bool fixErrors)
    {
        out.reserve(in.size());
        while (T* inElem = in.eat()) {
            T outElem;
            copyAndFix(outElem, *inElem, fixErrors);
            out.append(outElem);
        }
    }
}


/*
 *  Class game::v3::Unpacker
 */


game::v3::Unpacker::Unpacker(afl::string::Translator& tx, afl::io::Directory& specDir)
    : m_translator(tx),
      m_format(WindowsFormat),
      m_ignore35(false),
      m_createTargetExt(false),
      m_fixErrors(true),
      m_ignoreErrors(false),
      m_verbose(false),
      m_datShips(), m_disShips(),
      m_datPlanets(), m_disPlanets(),
      m_datBases(), m_disBases(),
      m_specificationDirectory(specDir),
      m_outbox(),
      m_allianceCommands(),
      m_gen(),
      m_control(),
      m_playerId(0),
      m_charset(afl::charset::g_codepageLatin1)
{
    // ex GUnpacker::GUnpacker
}

game::v3::Unpacker::~Unpacker()
{ }

// Set file format to produce.
void
game::v3::Unpacker::setFormat(DirectoryFormat fmt)
{
    // ex GUnpacker::setFormat
    m_format = fmt;
}

// Set version 3.5 part handling.
void
game::v3::Unpacker::setIgnore35Part(bool flag)
{
    // ex GUnpacker::setIgnore35Part
    m_ignore35 = flag;
}

// Set target.ext creation flag.
void
game::v3::Unpacker::setCreateTargetExt(bool flag)
{
    // ex GUnpacker::setCreateTargetExt
    m_createTargetExt = flag;
}

// Set error correction flag.
void
game::v3::Unpacker::setFixErrors(bool flag)
{
    // ex GUnpacker::setFixErrors
    m_fixErrors = flag;
}

// Set checksum ignore flag.
void
game::v3::Unpacker::setForceIgnoreErrors(bool flag)
{
    // ex GUnpacker::setForceIgnoreErrors
    m_ignoreErrors = flag;
}

// Set verbosity flag.
void
game::v3::Unpacker::setVerbose(bool flag)
{
    m_verbose = flag;
}

// Get configured format.
game::v3::Unpacker::DirectoryFormat
game::v3::Unpacker::getFormat() const
{
    // ex GUnpacker::getFormat
    return m_format;
}

// Prepare unpacking a file.
void
game::v3::Unpacker::prepare(ResultFile& file, int player)
{
    // ex GUnpacker::unpack (part)
    // Clear memory
    m_datShips.clear();
    m_disShips.clear();
    m_datPlanets.clear();
    m_disPlanets.clear();
    m_datBases.clear();
    m_disBases.clear();
    m_control.clear();
    m_outbox.clear();
    m_allianceCommands.clear();
    m_playerId = player;

    // Start by reading GEN file to figure out player
    file.seekToSection(ResultFile::GenSection);
    m_gen.loadFromResult(file.getFile());
    if (m_gen.getPlayerId() != player) {
        if (!m_ignoreErrors) {
            throw FileFormatException(file.getFile(), Format(m_translator("File is owned by player %d, should be %d"), m_gen.getPlayerId(), player));
        }
    }

    // Load ships, planets, bases
    loadShips(file);
    loadPlanets(file);
    loadBases(file);
}

// Access TurnProcessor.
game::v3::trn::TurnProcessor&
game::v3::Unpacker::turnProcessor()
{
    return *this;
}

// Finish unpacking a file.
void
game::v3::Unpacker::finish(afl::io::Directory& dir, ResultFile& file)
{
    // ex GUnpacker::unpack (part)
    // Load control file. We don't care what kind it is, since we'll be rewriting it anyway.
    m_control.load(dir, m_playerId, m_translator, m_log);

    // Unpack ships, planets, bases
    saveShips(dir);
    savePlanets(dir);
    saveBases(dir);

    // Unpack targets
    TargetBuffer_t targetBuffer;
    file.seekToSection(ResultFile::TargetSection);
    unpackTargets(dir, file, targetBuffer);

    // Unpack VCRs
    file.seekToSection(ResultFile::VcrSection);
    unpackVcrs(dir, file);

    // Unpack SHIPXY
    file.seekToSection(ResultFile::ShipXYSection);
    unpackShipXY(dir, file);

    // Unpack messages
    file.seekToSection(ResultFile::MessageSection);
    unpackMessages(dir, file);

    // Unpack Kore (minefields, racenames, storms, AVCs, Ufos)
    removeGameFile(dir, Format("kore%d.dat", m_playerId));
    if (!m_ignore35 && file.hasSection(ResultFile::KoreSection)) {
        file.seekToSection(ResultFile::KoreSection);
        unpackKore(dir, file, targetBuffer);
    }

    // Unpack Skore (more Ufos)
    removeGameFile(dir, Format("skore%d.dat", m_playerId));
    if (!m_ignore35 && file.hasSection(ResultFile::SkoreSection)) {
        file.seekToSection(ResultFile::SkoreSection);
        unpackSkore(dir, file);
    }

    // FIXME: we don't unpack the LEECH file yet. Should we?
    // FIXME: we don't update the FIZZ file yet. Should we? PCC 1.x and CCUNPACK don't.

    // Create blank files
    createBlankFiles(dir);

    // Save target.ext file
    saveTargetExt(dir, targetBuffer);

    // Save gen file
    saveGen(dir);

    // Update indexes
    updateIndex(dir);
    updateGameRegistry(dir, m_gen.getTimestamp());

    // Save control file
    switch (m_format) {
     case WindowsFormat:
        removeGameFile(dir, "control.dat");
        m_control.setFileOwner(m_playerId);
        break;
     case DosFormat:
        removeGameFile(dir, Format("contrl%d.dat", m_playerId));
        m_control.setFileOwner(0);
        break;
    }
    m_control.save(dir, m_translator, m_log);
}

// Get turn number.
int
game::v3::Unpacker::getTurnNumber() const
{
    // ex GUnpacker::getTurnNumber
    return m_gen.getTurnNumber();
}

// Access logger.
afl::sys::Log&
game::v3::Unpacker::log()
{
    return m_log;
}

// Access character set.
afl::charset::Charset&
game::v3::Unpacker::charset()
{
    return m_charset;
}

/** Load ships.
    \param result Result file */
void
game::v3::Unpacker::loadShips(ResultFile& result)
{
    // ex GUnpacker::unpackShips (part)
    // Start
    result.seekToSection(ResultFile::ShipSection);
    Stream& s = result.getFile();

    // Count
    Int16_t rawCount;
    s.fullRead(rawCount.m_bytes);

    int count = rawCount;
    if (count < 0 || count > gt::NUM_SHIPS) {
        throw FileFormatException(s, m_translator("Invalid number of ships"));
    }

    // Content
    m_disShips.resize(count);
    s.fullRead(m_disShips.toBytes());

    // Verify checksum
    if (ByteSum().add(m_disShips.toBytes(), 0) != m_gen.getSectionChecksum(gt::ShipSection)) {
        if (!m_ignoreErrors) {
            throw FileFormatException(s, m_translator("Checksum mismatch in ship section"));
        }
    }

    // Create *.dat content
    copyBuffer<gt::Ship>(m_datShips, m_disShips, m_fixErrors);
}

/** Load planets.
    \param result Result file */
void
game::v3::Unpacker::loadPlanets(ResultFile& result)
{
    // ex GUnpacker::unpackPlanets (part)
    // Start
    result.seekToSection(ResultFile::PlanetSection);
    Stream& s = result.getFile();

    // Count
    Int16_t rawCount;
    s.fullRead(rawCount.m_bytes);

    int count = rawCount;
    if (count < 0 || count > gt::NUM_PLANETS) {
        throw FileFormatException(s, m_translator("Invalid number of planets"));
    }

    // Content
    m_disPlanets.resize(count);
    s.fullRead(m_disPlanets.toBytes());

    // Verify checksum
    if (ByteSum().add(m_disPlanets.toBytes(), 0) != m_gen.getSectionChecksum(gt::PlanetSection)) {
        if (!m_ignoreErrors) {
            throw FileFormatException(s, m_translator("Checksum mismatch in planet section"));
        }
    }

    // Create *.dat content
    copyBuffer<gt::Planet>(m_datPlanets, m_disPlanets, m_fixErrors);
}

/** Load starbases.
    \param result Result file */
void
game::v3::Unpacker::loadBases(ResultFile& result)
{
    // ex GUnpacker::unpackBases (part)
    // Start
    result.seekToSection(ResultFile::BaseSection);
    Stream& s = result.getFile();

    // Count
    Int16_t rawCount;
    s.fullRead(rawCount.m_bytes);

    int count = rawCount;
    if (count < 0 || count > gt::NUM_PLANETS) {
        throw FileFormatException(s, m_translator("Invalid number of bases"));
    }

    // Content
    m_disBases.resize(count);
    s.fullRead(m_disBases.toBytes());

    // Verify checksum
    if (ByteSum().add(m_disBases.toBytes(), 0) != m_gen.getSectionChecksum(gt::BaseSection)) {
        if (!m_ignoreErrors) {
            throw FileFormatException(s, m_translator("Checksum mismatch in base section"));
        }
    }

    // Create *.dat content
    copyBuffer<gt::Base>(m_datBases, m_disBases, m_fixErrors);
}

/** Save ships.
    \param dir target directory */
void
game::v3::Unpacker::saveShips(afl::io::Directory& dir)
{
    // Write files
    Ref<Stream> dat = dir.openFile(Format("ship%d.dat", m_playerId), FileSystem::Create);
    Ref<Stream> dis = dir.openFile(Format("ship%d.dis", m_playerId), FileSystem::Create);

    Int16_t rawCount;
    rawCount = static_cast<int16_t>(m_datShips.size());
    dat->fullWrite(rawCount.m_bytes);
    dis->fullWrite(rawCount.m_bytes);
    dat->fullWrite(m_datShips.toBytes());
    dis->fullWrite(m_disShips.toBytes());
    dat->fullWrite(m_gen.getSignature2());
    dis->fullWrite(m_gen.getSignature1());

    // Compute checksums
    uint32_t gameChecksum = 2*getChecksum(rawCount.m_bytes) + getChecksum(m_gen.getSignature2()) + getChecksum(m_gen.getSignature1());
    for (size_t i = 0, n = m_datShips.size(); i < n; ++i) {
        gt::Ship* datShip = m_datShips.at(i);
        gt::Ship* disShip = m_disShips.at(i);
        if (datShip != 0 && disShip != 0) {
            uint32_t datChecksum = getChecksum(afl::base::fromObject(*datShip));
            uint32_t disChecksum = getChecksum(afl::base::fromObject(*disShip));
            m_control.set(gt::ShipSection, datShip->shipId, datChecksum);
            gameChecksum += datChecksum;
            gameChecksum += disChecksum;
        }
    }
    m_gen.setSectionChecksum(gt::ShipSection, gameChecksum);

    m_log.write(afl::sys::Log::Info, LOG_NAME, Format(m_translator("Unpacked %d ship%!1{s%}."), m_datShips.size()));
}

/** Save planets.
    \param dir target directory */
void
game::v3::Unpacker::savePlanets(afl::io::Directory& dir)
{
    // Write files
    Ref<Stream> dat = dir.openFile(Format("pdata%d.dat", m_playerId), FileSystem::Create);
    Ref<Stream> dis = dir.openFile(Format("pdata%d.dis", m_playerId), FileSystem::Create);

    Int16_t rawCount;
    rawCount = static_cast<int16_t>(m_datPlanets.size());
    dat->fullWrite(rawCount.m_bytes);
    dis->fullWrite(rawCount.m_bytes);
    dat->fullWrite(m_datPlanets.toBytes());
    dis->fullWrite(m_disPlanets.toBytes());
    dat->fullWrite(m_gen.getSignature2());
    dis->fullWrite(m_gen.getSignature1());

    // Compute checksums
    uint32_t gameChecksum = 2*getChecksum(rawCount.m_bytes) + getChecksum(m_gen.getSignature2()) + getChecksum(m_gen.getSignature1());
    for (size_t i = 0, n = m_datPlanets.size(); i < n; ++i) {
        gt::Planet* datPlanet = m_datPlanets.at(i);
        gt::Planet* disPlanet = m_disPlanets.at(i);
        if (datPlanet != 0 && disPlanet != 0) {
            uint32_t datChecksum = getChecksum(afl::base::fromObject(*datPlanet));
            uint32_t disChecksum = getChecksum(afl::base::fromObject(*disPlanet));
            m_control.set(gt::PlanetSection, datPlanet->planetId, datChecksum);
            gameChecksum += datChecksum;
            gameChecksum += disChecksum;
        }
    }
    m_gen.setSectionChecksum(gt::PlanetSection, gameChecksum);

    m_log.write(afl::sys::Log::Info, LOG_NAME, Format(m_translator("Unpacked %d planet%!1{s%}."), m_datPlanets.size()));
}

/** Save starbases.
    \param dir target directory */
void
game::v3::Unpacker::saveBases(afl::io::Directory& dir)
{
    // Write files
    Ref<Stream> dat = dir.openFile(Format("bdata%d.dat", m_playerId), FileSystem::Create);
    Ref<Stream> dis = dir.openFile(Format("bdata%d.dis", m_playerId), FileSystem::Create);

    Int16_t rawCount;
    rawCount = static_cast<int16_t>(m_datBases.size());
    dat->fullWrite(rawCount.m_bytes);
    dis->fullWrite(rawCount.m_bytes);
    dat->fullWrite(m_datBases.toBytes());
    dis->fullWrite(m_disBases.toBytes());
    dat->fullWrite(m_gen.getSignature2());
    dis->fullWrite(m_gen.getSignature1());

    // Compute checksums
    uint32_t gameChecksum = 2*getChecksum(rawCount.m_bytes) + getChecksum(m_gen.getSignature2()) + getChecksum(m_gen.getSignature1());
    for (size_t i = 0, n = m_datBases.size(); i < n; ++i) {
        gt::Base* datBase = m_datBases.at(i);
        gt::Base* disBase = m_disBases.at(i);
        if (datBase != 0 && disBase != 0) {
            uint32_t datChecksum = getChecksum(afl::base::fromObject(*datBase));
            uint32_t disChecksum = getChecksum(afl::base::fromObject(*disBase));
            m_control.set(gt::BaseSection, datBase->baseId, datChecksum);
            gameChecksum += datChecksum;
            gameChecksum += disChecksum;
        }
    }
    m_gen.setSectionChecksum(gt::BaseSection, gameChecksum);

    m_log.write(afl::sys::Log::Info, LOG_NAME, Format(m_translator("Unpacked %d starbase%!1{s%}."), m_datBases.size()));
}

/** Save "genX.dat" file.
    \param dir target directory */
void
game::v3::Unpacker::saveGen(afl::io::Directory& dir)
{
    // ex GUnpacker::saveGen
    gt::Gen data;
    m_gen.getData(data);
    dir.openFile(Format("gen%d.dat", m_playerId), FileSystem::Create)->fullWrite(afl::base::fromObject(data));
}

/** Save "targetX.ext" file.
    If additional targets have accumulated in our buffer, write them out.
    Otherwise, remove the file.
    \param dir target directory */
void
game::v3::Unpacker::saveTargetExt(afl::io::Directory& dir, const TargetBuffer_t& targetBuffer)
{
    if (!targetBuffer.empty()) {
        // We have some additional targets
        Ref<Stream> dat = dir.openFile(Format("target%d.ext", m_playerId), FileSystem::Create);

        // Write count
        Int16_t rawCount;
        rawCount = static_cast<int16_t>(targetBuffer.size());
        dat->fullWrite(rawCount.m_bytes);

        // Write data
        dat->fullWrite(targetBuffer.toBytes());

        // Signature
        dat->fullWrite(m_gen.getSignature2());

        if (m_verbose) {
            m_log.write(afl::sys::Log::Info, LOG_NAME, Format(m_translator("Unpacked %d visual contact%!1{s%} to \"target%d.ext\"."), targetBuffer.size(), m_playerId));
        }
    } else {
        // No additional targets, scrap the file
        removeGameFile(dir, Format("target%d.ext", m_playerId));
    }
}

/** Unpack target section.
    Creates a target.dat file.
    If requested and necessary, stores excess targets in targetBuffer for later processing by saveTargetExt().
    \param dir target directory
    \param result Result file
    \param targetBuffer Storage for excess targets */
void
game::v3::Unpacker::unpackTargets(afl::io::Directory& dir, ResultFile& result, TargetBuffer_t& targetBuffer)
{
    // ex GUnpacker::unpackTargets
    Stream& s = result.getFile();
    Ref<Stream> dat = dir.openFile(Format("target%d.dat", m_playerId), FileSystem::Create);

    // Count
    Int16_t rawCount;
    s.fullRead(rawCount.m_bytes);

    int16_t count = rawCount;
    if (count < 0 || count > gt::NUM_SHIPS) {
        throw FileFormatException(s, m_translator("Invalid number of targets"));
    }

    // If target.ext is requested, only extract 50 targets; the others go in target.ext.
    int16_t targetsInFile = count;
    if (m_createTargetExt && targetsInFile > 50) {
        targetsInFile = 50;
    }

    // Save new count
    rawCount = targetsInFile;
    dat->fullWrite(rawCount.m_bytes);

    // Copy to file
    dat->copyFrom(s, targetsInFile * sizeof(gt::ShipTarget));

    // Copy remainder to target.ext buffer
    int targetsInExt = count - targetsInFile;
    if (targetsInExt > 0) {
        s.fullRead(targetBuffer.appendN(gt::ShipTarget(), targetsInExt).toBytes());
    }

    // Add signature
    dat->fullWrite(m_gen.getSignature2());

    if (m_verbose && targetsInFile != count) {
        m_log.write(afl::sys::Log::Info, LOG_NAME, Format(m_translator("Unpacked %d visual contact%!1{s%} to \"target%d.dat\"."), targetsInFile, m_playerId));
    } else {
        m_log.write(afl::sys::Log::Info, LOG_NAME, Format(m_translator("Unpacked %d visual contact%!1{s%}."), count));
    }
}

/** Unpack VCR section.
    Creates a vcr.dat file.
    \param dir target directory
    \param result Result file */
void
game::v3::Unpacker::unpackVcrs(afl::io::Directory& dir, ResultFile& result)
{
    // ex GUnpacker::unpackVCRs
    Stream& s = result.getFile();

    Ref<Stream> dat = dir.openFile(Format("vcr%d.dat", m_playerId), FileSystem::Create);

    // Read count
    Int16_t rawCount;
    s.fullRead(rawCount.m_bytes);

    int16_t count = rawCount;
    if (count < 0) {
        throw FileFormatException(s, m_translator("VCR file is invalid"));
    }

    dat->fullWrite(rawCount.m_bytes);

    // Copy
    dat->copyFrom(s, count * sizeof(gt::Vcr));

    // Add signature
    dat->fullWrite(m_gen.getSignature2());

    m_log.write(afl::sys::Log::Info, LOG_NAME, Format(m_translator("Unpacked %d combat recording%!1{s%}."), count));
}

/** Unpack shipxy section.
    Creates a shipxy.dat file.
    \param dir target directory
    \param result Result file */
void
game::v3::Unpacker::unpackShipXY(afl::io::Directory& dir, ResultFile& result)
{
    // ex GUnpacker::unpackShipXY
    Stream& s = result.getFile();
    int numEntries = result.getNumShipCoordinates();
    Ref<Stream> dat = dir.openFile(Format("shipxy%d.dat", m_playerId), FileSystem::Create);
    dat->copyFrom(s, numEntries * sizeof(gt::ShipXY));
    dat->fullWrite(m_gen.getSignature2());

    m_log.write(afl::sys::Log::Info, LOG_NAME, Format(m_translator("Unpacked ship location file with %d entries."), numEntries));
}

/** Unpack messages. Creates mdata.dat file.
    \param dir target directory
    \param result Result file */
void
game::v3::Unpacker::unpackMessages(afl::io::Directory& dir, ResultFile& result)
{
    // ex GUnpacker::unpackMessages
    Stream& s = result.getFile();

    // Read count
    Int16_t rawCount;
    s.fullRead(rawCount.m_bytes);
    int16_t count = rawCount;
    if (count < 0) {
        throw FileFormatException(s, m_translator("Message file is invalid"));
    }

    // Read index
    afl::base::GrowableMemory<gt::IncomingMessageHeader> index;
    index.resize(count);
    s.fullRead(index.toBytes());

    // Validate index
    for (int i = 0; i < count; ++i) {
        gt::IncomingMessageHeader* mh = index.at(i);
        if (mh->address <= 0 || mh->length <= 0) {
            throw FileFormatException(s, m_translator("Message file is invalid"));
        }
    }

    // Now generate the file, Start by writing dummy count + invalid directory.
    Ref<Stream> dat = dir.openFile(Format("mdata%d.dat", m_playerId), FileSystem::Create);
    Int16_t rawZero;
    rawZero = 0;
    dat->fullWrite(rawZero.m_bytes);
    dat->fullWrite(index.toBytes());

    // Copy each message
    afl::base::GrowableMemory<uint8_t> messageBuffer;
    for (int i = 0; i < count; ++i) {
        // Read
        gt::IncomingMessageHeader* mh = index.at(i);
        s.setPos(mh->address - 1);

        size_t size = mh->length;
        messageBuffer.resize(size);
        s.fullRead(messageBuffer);

        // Write
        mh->address = static_cast<int32_t>(dat->getPos() + 1);
        dat->fullWrite(messageBuffer);
    }

    // Write real header
    dat->setPos(0);
    dat->fullWrite(rawCount.m_bytes);
    dat->fullWrite(index.toBytes());

    m_log.write(afl::sys::Log::Info, LOG_NAME, Format(m_translator("Unpacked %d message%!1{s%}."), count));
}

/** Unpack KORE file.
    Creates kore.dat file and, optionally, a race.nm file.
    \param dir target directory
    \param result Result file
    \param targetBuffer Storage for excess targets */
void
game::v3::Unpacker::unpackKore(afl::io::Directory& dir, ResultFile& result, TargetBuffer_t& targetBuffer)
{
    // ex GUnpacker::unpackKore
    // RST:                              File:
    //                                   2      turn number
    //                                   7      junk
    //                                   10     sig2
    //                                   83     junk
    //   500x8  minefields               500x8  minefields
    //   50x12  ion storms               50x12  ion storms
    //   50x4   bangs                    50x4   bangs
    //   682    race names
    //   100x78 Ufos                     100x78 Ufos
    //   4      "1211" or "1120"         4      "1211" or "1120"
    //                                   16     junk
    //   if 1120:                        if 1120:
    //     4    count                      4    count
    //     nx34 AVCs                       nx34 AVCs
    //                                   10     sig2

    Stream& s = result.getFile();

    Ref<Stream> dat = dir.openFile(Format("kore%d.dat", m_playerId), FileSystem::Create);

    // Write the "junk" block
    gt::KoreHeader header;
    afl::base::fromObject(header).fill(0);
    header.turnNumber = static_cast<int16_t>(m_gen.getTurnNumber());
    afl::base::Bytes_t(header.signature2).copyFrom(m_gen.getSignature2());
    dat->fullWrite(afl::base::fromObject(header));

    // Copy minefields, ion storms, bangs
    dat->copyFrom(s, 500*8 + 50*12 + 50*4);

    // Skip race names. Those are now unpacked by AttachmentUnpacker.
    uint8_t rstRaceNameBuffer[sizeof(gt::RaceNames)];
    s.fullRead(rstRaceNameBuffer);

    // Ufos
    dat->copyFrom(s, 78*100);

    // Deal with AVCs. We have to do two things here: copy them to the
    // koreX.dat file, and duplicate the decrypted version to targetX.ext
    // if required. First, copy the marker and create the junk block.
    Int32_t rawMarker;
    s.fullRead(rawMarker.m_bytes);
    dat->fullWrite(rawMarker.m_bytes);

    uint8_t pad[16] = {};
    dat->fullWrite(pad);

    if (rawMarker == 0x30323131) {
        // AVCs present
        Int32_t rawCount;
        s.fullRead(rawCount.m_bytes);
        dat->fullWrite(rawCount.m_bytes);

        int32_t count = rawCount;
        if (count < 0 || count > gt::NUM_SHIPS) {
            throw FileFormatException(s, m_translator("Unbelievable number of visual contacts"));
        }

        for (int32_t i = 0; i < count; ++i) {
            gt::ShipTarget target;
            s.fullRead(afl::base::fromObject(target));
            dat->fullWrite(afl::base::fromObject(target));
            if (m_createTargetExt) {
                // Target.ext requested, so decode and save it
                encryptTarget(target);
                targetBuffer.append(target);
            }
        }
    }

    // Add signature
    dat->fullWrite(m_gen.getSignature2());

    if (m_verbose) {
        m_log.write(afl::sys::Log::Info, LOG_NAME, m_translator("Unpacked version 3.5 object file (kore)."));
    }
}

/** Unpack SKORE file.
    Creates a skore.dat file if needed.
    \param dir target directory
    \param result Result file */
void
game::v3::Unpacker::unpackSkore(afl::io::Directory& dir, ResultFile& result)
{
    // ex GUnpacker::unpackSkore
    // RST:                   File:
    //                        96   junk
    //                        5    "yAmsz" signature
    //   2    count           2    count
    //                        2    minor version ("01")
    //   nx78 Ufos            nx78 Ufos
    //                        if count <= 100: "no000"

    // We don't ever generate the "no000" case. In that case, we don't
    // generate a Skore file at all.

    Stream& s = result.getFile();

    Int16_t rawCount;
    s.fullRead(rawCount.m_bytes);

    int count = rawCount;
    if (count <= 100) {
        if (m_verbose) {
            m_log.write(afl::sys::Log::Info, LOG_NAME, m_translator("Extended Ufo database exists but is empty."));
        }
        return;
    }

    // File exists
    Ref<Stream> dat = dir.openFile(Format("skore%d.dat", m_playerId), FileSystem::Create);

    // Generate the "junk" block.
    gt::SkoreHeader header;
    afl::base::fromObject(header).fill(0);
    std::memcpy(header.signature, "yAmsz", 5);
    header.numUfos = rawCount;
    header.resultVersion = static_cast<int16_t>(result.getVersion());
    dat->fullWrite(afl::base::fromObject(header));

    // Copy Ufo data
    dat->copyFrom(s, sizeof(gt::Ufo) * (count - 100));

    m_log.write(afl::sys::Log::Info, LOG_NAME, Format(m_translator("Unpacked %d additional Ufo%!1{s%}."), count - 100));
}

/** Create blank files.
    Creates or erases files that must be blank at the start of the turn:
    - mess.dat/mess35.dat
    - cp.cc (used by ancient PCC)
    - cmd.txt (used by PCC, PCC2, VPA)
    \param dir target directory */
void
game::v3::Unpacker::createBlankFiles(afl::io::Directory& dir)
{
    const String_t dosOutboxName = Format("mess%d.dat", m_playerId);
    const String_t windowsOutboxName = Format("mess35%d.dat", m_playerId);

    // Create outbox file
    if (m_format == WindowsFormat) {
        // Windows
        Ref<Stream> file = dir.openFile(windowsOutboxName, FileSystem::Create);
        Writer(m_charset, m_translator, m_log).saveOutbox35(m_outbox, m_playerId, *file);
        removeGameFile(dir, dosOutboxName);
    } else {
        // DOS: we need a player list to write correct message headers
        PlayerList playerList;
        loadRaceNames(playerList, m_specificationDirectory, m_charset);

        Ref<Stream> file = dir.openFile(dosOutboxName, FileSystem::Create);
        Writer(m_charset, m_translator, m_log).saveOutbox(m_outbox, m_playerId, playerList, *file);
        removeGameFile(dir, windowsOutboxName);
    }

    // Old PCC additional command files
    removeGameFile(dir, Format("cp%d.cc", m_playerId));

    // New PCC additional command file
    String_t commandFileName = Format("cmd%d.txt", m_playerId);
    if (m_allianceCommands.empty()) {
        removeGameFile(dir, commandFileName);
    } else {
        const String_t commandFileContent = Format("# Additional commands\n"
                                                   "$time %s\n"
                                                   "$thost-allies %s\n",
                                                   m_gen.getTimestamp().getTimestampAsString(),
                                                   m_allianceCommands);
        dir.openFile(commandFileName, FileSystem::Create)
            ->fullWrite(afl::string::toBytes(commandFileContent));
    }
}

/** Update index.
    This updates the init.tmp file and marks our new player as present.
    \param dir target directory */
void
game::v3::Unpacker::updateIndex(afl::io::Directory& dir)
{
    // ex GUnpacker::updateIndex, ccload.pas:MakeInitTmp
    if (m_playerId > 0 && m_playerId <= gt::NUM_PLAYERS) {
        // Initialize
        uint8_t index[2 * gt::NUM_PLAYERS];
        afl::base::Bytes_t(index).fill(0);

        // Load old index, if any. Deliberately ignore read errors.
        try {
            dir.openFile("init.tmp", FileSystem::OpenRead)->read(index);
        }
        catch (...) { }

        // Mark new player and write new index
        index[2*(m_playerId-1)]   = 1;
        index[2*(m_playerId-1)+1] = 0;
        dir.openFile("init.tmp", FileSystem::Create)->fullWrite(index);
    }
}

/** Remove a file.
    \param dir target directory
    \param name File name */
void
game::v3::Unpacker::removeGameFile(afl::io::Directory& dir, const String_t& name)
{
    dir.eraseNT(name);
}

/** Report failure.
    Common code for TurnProcessor error handlers.
    \param tpl Format template
    \param arg Format parameter */
void
game::v3::Unpacker::fail(const char* tpl, int arg)
{
    throw FileFormatException(String_t("<turn>"), Format(m_translator(tpl), arg));
}

void
game::v3::Unpacker::handleInvalidCommand(int code)
{
    fail(N_("Turn file contains invalid command code %d"), code);
}

void
game::v3::Unpacker::validateShip(int id)
{
    if (!findShip(m_datShips, id)) {
        fail(N_("Turn file refers to non-existant ship %d"), id);
    }
}

void
game::v3::Unpacker::validatePlanet(int id)
{
    if (!findPlanet(m_datPlanets, id)) {
        fail(N_("Turn file refers to non-existant planet %d"), id);
    }
}

void
game::v3::Unpacker::validateBase(int id)
{
    if (!findBase(m_datBases, id)) {
        fail(N_("Turn file refers to non-existant starbase %d"), id);
    }
}

void
game::v3::Unpacker::getShipData(int id, Ship_t& out, afl::charset::Charset& /*charset*/)
{
    if (gt::Ship* p = findShip(m_datShips, id)) {
        out = *p;
    }
}

void
game::v3::Unpacker::getPlanetData(int id, Planet_t& out, afl::charset::Charset& /*charset*/)
{
    if (gt::Planet* p = findPlanet(m_datPlanets, id)) {
        out = *p;
    }
}

void
game::v3::Unpacker::getBaseData(int id, Base_t& out, afl::charset::Charset& /*charset*/)
{
    if (gt::Base* p = findBase(m_datBases, id)) {
        out = *p;
    }
}

void
game::v3::Unpacker::storeShipData(int id, const Ship_t& in, afl::charset::Charset& /*charset*/)
{
    if (gt::Ship* p = findShip(m_datShips, id)) {
        *p = in;
    }
}

void
game::v3::Unpacker::storePlanetData(int id, const Planet_t& in, afl::charset::Charset& /*charset*/)
{
    if (gt::Planet* p = findPlanet(m_datPlanets, id)) {
        *p = in;
    }
}

void
game::v3::Unpacker::storeBaseData(int id, const Base_t& in, afl::charset::Charset& /*charset*/)
{
    if (gt::Base* p = findBase(m_datBases, id)) {
        *p = in;
    }
}

void
game::v3::Unpacker::addMessage(int to, String_t text)
{
    if (to == 12) {  // FIXME
        to = 0;
    }
    m_outbox.addMessageFromFile(m_playerId, text, PlayerSet_t(to));
}

void
game::v3::Unpacker::addNewPassword(const NewPassword_t& pass)
{
    m_gen.setNewPasswordData(pass);
}

void
game::v3::Unpacker::addAllianceCommand(String_t text)
{
    m_allianceCommands += text;
}
