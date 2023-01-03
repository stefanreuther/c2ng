/**
  *  \file game/v3/check/checker.cpp
  */

#include <cmath>
#include "game/v3/check/checker.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/exception.hpp"
#include "game/v3/trn/turnprocessor.hpp"
#include "game/v3/turnfile.hpp"
#include "util/math.hpp"
#include "util/string.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::Stream;
using afl::string::Format;

namespace gs = game::v3::structures;

namespace {
    const int MAXINT = 32767;
}

game::v3::check::Checker::Checker(afl::io::Directory& gamedir,
                                  afl::io::Directory& rootdir,
                                  int player,
                                  afl::io::TextWriter& log,
                                  afl::io::TextWriter& output,
                                  afl::io::TextWriter& error)
    : gamedir(gamedir),
      rootdir(rootdir),
      m_player(player),
      log(log),
      output(output),
      error(error),
      m_config(),
      had_ck_error(false),
      had_divi(false),
      html_fmt(hRaw),
      ships(),
      planets(),
      torps(),
      beams(),
      hulls(),
      engines(),
      truehull(),
      had_any_error(false),
      had_error(false),
      ctx()
{ }

game::v3::check::Checker::~Checker()
{
    for (int i = 1; i <= NUM_PLANETS; ++i) {
        delete planets[i-1].pdat;
        delete planets[i-1].pdis;
        delete planets[i-1].bdat;
        delete planets[i-1].bdis;
    }
    for (int i = 1; i <= NUM_SHIPS; ++i) {
        delete ships[i-1].dat;
        delete ships[i-1].dis;
    }
}

void
game::v3::check::Checker::run()
{
    // ex check.pas:main
    if (m_config.isHtmlMode()) {
        log.writeText("<pre>");
    }
    try {
        had_error = false;
        if (m_config.isResultMode()) {
            uint8_t rst_timestamp[18];
            logStr("Loading Result File:");
            loadResult(rst_timestamp);
            logStr("Loading Turn File:");
            loadTurn(rst_timestamp);
        } else {
            DirStuff stuff;
            logStr("Loading Game:");
            loadGen(stuff);
            loadShips(stuff);
            loadPlanets(stuff);
            if (m_config.isChecksumsMode()) {
                logStr("Validating Checksums:");
                loadChecksums(stuff);
            }
        }
        loadSpecs();
        if (had_error) {
            // @change This branch not in check.pas 0.5
            logStr("Loading failed.");
            had_any_error = true;
        } else {
            logStr("Checking:");
            had_error = false;
            had_any_error = false;
            rangeCheckSpecs();
            if (had_error) {
                logItem("Spec file check", "failed");
                logStr("c2check cannot handle these specification files.");
                logStr("If you see this message on a correct ship list,");
                logStr("please contact the author.");
                die("Check cannot continue.");
            }
            logCheck("Spec file check");
            rangeCheckShips();
            logCheck("Ship range check");
            rangeCheckPlanets();
            logCheck("Planet range check");
            flowCheckOrbits();
            logCheck("Orbit flow check");
            flowCheckFreeSpace();
            logCheck("Space flow check");
            logStr("");
            if (had_any_error) {
                logStrB("Turn is invalid.");
            } else if (had_ck_error) {
                logStrB("Turn has checksum errors.");
            } else {
                logStrB("Turn is OK.");
            }
        }
    }
    catch (afl::except::FileFormatException& e) {
        logAbort(Format("SYNTAX: %s: %s", e.getFileName(), e.what()));
        had_any_error = true;
    }
    catch (afl::except::FileProblemException& e) {
        logAbort(Format("FATAL: %s: %s", e.getFileName(), e.what()));
        had_any_error = true;
    }
    catch (std::exception& e) {
        logAbort(Format("FATAL: %s", e.what()));
        had_any_error = true;
    }
    if (m_config.isHtmlMode()) {
        log.writeLine("</pre>");
    }
    log.flush();
    output.flush();
    error.flush();
}

/**************************** Log File Output ****************************/

void
game::v3::check::Checker::logAbort(const String_t& s)
{
    error.writeLine(s);
    log.writeLine(s);
    if (m_config.isHtmlMode()) {
        log.writeLine("<b>Check aborted.</b>");
    } else {
        log.writeLine("Check aborted.");
    }
}

void
game::v3::check::Checker::die(const String_t& s)
{
    // ex check.pas:Die
    logAbort(s);
    throw game::Exception("Check aborted.");
}

void
game::v3::check::Checker::syntax(const String_t& s)
{
    // ex check.pas:Syntax
    die("SYNTAX: " + s);
}

void
game::v3::check::Checker::startSec(const String_t& color)
{
    // ex check.pas:StartSec
    log.writeText("<div style=\"padding:1px; margin:1px; border:solid black 1px; background-color:#" + color + "\">");
    html_fmt = hInSec;
}

String_t
game::v3::check::Checker::escape(const String_t& s)
{
    // ex check.pas:Escape
    return util::encodeHtml(s, false);
}

void
game::v3::check::Checker::logStr(const String_t& s)
{
    // ex check.pas:LogStr
    if (m_config.isHtmlMode() && (html_fmt == hStartSec)) {
        if (s.size() < 2 || (s[1] < 'A' || s[1] > 'Z')) {
            html_fmt = hRaw;
        } else {
            switch (s[0]) {
             case 'R': startSec("ffcccc"); break;
             case 'W': startSec("ffffcc"); break;
             case 'I': startSec("ccffcc"); break;
             case 'B': startSec("ccddff"); break;
             case 'C': startSec("ccffcc"); break;
             default:  html_fmt = hRaw;    break;
            }
        }
    }
    output.writeLine(s);
    if (m_config.isHtmlMode()) {
        log.writeLine(escape(s));
    } else {
        log.writeLine(s);
    }
    had_divi = false;
}

void
game::v3::check::Checker::logStrB(const String_t& s)
{
    // ex check.pas:LogStrB
    if (m_config.isHtmlMode()) {
        if (html_fmt == hStartSec) {
            html_fmt = hRaw;
        }
        log.writeLine("<b>" + escape(s) + "</b>");
        output.writeLine(s);
        had_divi = false;
    } else {
        logStr(s);
    }
}

void
game::v3::check::Checker::logDivi()
{
    // ex check.pas:LogDivi
    const char* divi = "----------------------------------------------------------------------";
    if (m_config.isHtmlMode()) {
        if (!had_divi) {
            output.writeLine(divi);
            had_divi = true;
        }
        if (html_fmt == hInSec) {
            log.writeText("</div>");
        }
        html_fmt = hStartSec;
    } else {
        if (!had_divi) {
            output.writeLine(divi);
            log.writeLine(divi);
            had_divi = true;
        }
    }
}

void
game::v3::check::Checker::logItem(const String_t& pre, const String_t& suf)
{
    // ex check.pas:LogItem
    logStr(Format("  %-18s : %s", pre, suf));
}

void
game::v3::check::Checker::logBlock(const String_t& s)
{
    // ex check.pas:LogBlock
    logDivi();
    logStr(s);
    logDivi();
}

void
game::v3::check::Checker::logCheck(const String_t& title)
{
    // ex check.pas:LogCheck
    if (had_error) {
        logItem(title, "failed");
        had_any_error = true;
        had_error = false;
    } else {
        logItem(title, "succeeded");
    }
}

/******************************* Checksums *******************************/

uint32_t
game::v3::check::Checker::checksum(afl::base::ConstBytes_t bytes)
{
    // ex check.pas:Checksum
    return afl::checksums::ByteSum().add(bytes, 0);
}

bool
game::v3::check::Checker::memEq(afl::base::ConstBytes_t a, afl::base::ConstBytes_t b)
{
    // ex check.pas:MemEq
    return a.equalContent(b);
}

void
game::v3::check::Checker::checkChecksum(const String_t& title, uint32_t soll, uint32_t ist)
{
    // ex check.pas:CheckChecksum
    if (m_config.isChecksumsMode() && soll != ist) {
        logDivi();
        logStr(Format("CHECKSUM: %s checksum mismatch:", title));
        logStr(Format("    Stored value is %d", ist));
        logStr(Format("    Should be %d as computed from data", soll));
        logDivi();
        had_ck_error = true;
    }
}

void
game::v3::check::Checker::checkSigs(const String_t& name, afl::io::Stream& dat, afl::io::Stream& dis, const DirStuff& stuff)
{
    // ex check.pas:CheckSigs
    if (m_config.isChecksumsMode()) {
        uint8_t sdat[SIGNATURE_SIZE], sdis[SIGNATURE_SIZE];
        size_t gdat = dat.read(sdat);
        size_t gdis = dis.read(sdis);
        if (gdat == 0 && gdis == 0) {
            // short message in case both are missing - happens with xk
            logBlock(Format("CHECKSUM: %s%d.dat/.dis do not have a signature block.", name, m_player));
        } else {
            // long message
            if (gdat != SIGNATURE_SIZE) {
                logBlock(Format("CHECKSUM: %s%d.dat signature is only %d bytes, expecting 10.", name, m_player, gdat));
            } else if (!memEq(sdat, stuff.gen.getSignature2())) {
                logBlock(Format("CHECKSUM: %s%d.dat signature is invalid.", name, m_player));
            }

            if (gdis != SIGNATURE_SIZE) {
                logBlock(Format("CHECKSUM: %s%d.dis signature is only %d bytes, expecting 10.", name, m_player, gdis));
            } else if (!memEq(sdis, stuff.gen.getSignature1())) {
                logBlock(Format("CHECKSUM: %s%d.dis signature is invalid.", name, m_player));
            }
        }
    }
}

bool
game::v3::check::Checker::checkTimestamp(uint8_t (&ts)[18])
{
    // ex check.pas:CheckTimestampFormat
    bool ok = true;
    static const char ts_template[18+1] = "00-00-000000:00:00";
    for (size_t i = 0; i < 18; ++i) {
        if (ts_template[i] == '0') {
            if (ts[i] < '0' || ts[i] > '9') {
                ok = false;
            }
        } else {
            if (ts[i] != ts_template[i]) {
                ok = false;
            }
        }
    }
    return ok;
}

/***************************** Loading Stuff *****************************/

// { open a game file, game directory only }
afl::base::Ref<afl::io::Stream>
game::v3::check::Checker::openGameFile(const String_t& name) const
{
    // ex check.pas:OpenGameFile
    return gamedir.openFile(name, afl::io::FileSystem::OpenRead);
}

// { open a spec file, game or root directory }
afl::base::Ref<afl::io::Stream>
game::v3::check::Checker::openSpecFile(const String_t& name) const
{
    // ex check.pas:OpenSpecFile
    afl::base::Ptr<afl::io::Stream> p = gamedir.openFileNT(name, afl::io::FileSystem::OpenRead);
    if (p.get() != 0) {
        return *p;
    }
    return rootdir.openFile(name, afl::io::FileSystem::OpenRead);
}

void
game::v3::check::Checker::loadXYPlan()
{
    // ex check.pas:LoadXYPlan
    // FIXME: ExploreMap?
    Ref<Stream> dat = openSpecFile("xyplan.dat");
    for (int i = 0; i < NUM_PLANETS; ++i) {
        gs::PlanetXY xyr;
        dat->fullRead(afl::base::fromObject(xyr));
        planets[i].x = xyr.x;
        planets[i].y = xyr.y;
    }
}

int
game::v3::check::Checker::planetAt(int x, int y) const
{
    // ex check.pas:PlanetAt
    for (int i = 1; i <= NUM_PLANETS; ++i) {
        if (planets[i-1].x == x && planets[i-1].y == y) {
            return i;
        }
    }
    return 0;
}

void
game::v3::check::Checker::loadGen(DirStuff& stuff)
{
    // ex check.pas:LoadGen
    // load GEN file
    String_t ndat = Format("gen%d.dat", m_player);
    Ref<Stream> dat = openGameFile(ndat);
    gs::Gen gen;
    dat->fullRead(afl::base::fromObject(gen));
    stuff.gen = gen;

    if (gen.playerId != m_player) {
        logBlock(Format("INVALID: %s belongs to player %d, not %d", ndat, int(gen.playerId), m_player));
        had_error = true;
    }
    if (gen.newPasswordFlag != 0 && gen.newPasswordFlag != 13) {
        logBlock(Format("INVALID: password flag has invalid value %d'", int(gen.newPasswordFlag)));
        had_error = true;
    }
    if (gen.turnNumber <= 0) {
        logBlock(Format("INVALID: turn number has invalid value %d", int(gen.turnNumber)));
        had_error = true;
    }
    if (!checkTimestamp(gen.timestamp)) {
        logBlock("INVALID: time stamp has an invalid format");
        had_error = true;
    }
    checkChecksum(ndat + " timestamp", checksum(gen.timestamp), gen.timestampChecksum);
}

void
game::v3::check::Checker::loadShips(const DirStuff& stuff)
{
    // ex check.pas:LoadShips
    // FIXME:    FillChar(ships, Sizeof(ships), 0);
    gs::Int16_t cdat, cdis;
    gs::Ship rdat, rdis;

    String_t ndat = Format("ship%d.dat", m_player);
    String_t ndis = Format("ship%d.dis", m_player);
    Ref<Stream> dat = openGameFile(ndat);
    Ref<Stream> dis = openGameFile(ndis);
    dat->fullRead(cdat.m_bytes);
    dis->fullRead(cdis.m_bytes);
    const int ccdat = cdat, ccdis = cdis;
    if (ccdat != ccdis) {
        syntax(Format("%s and %s do not match (count).", ndat, ndis));
    }
    if (ccdat < 0 || ccdat > NUM_SHIPS) {
        syntax(Format("%s has too large counter and is probably invalid.", ndat));
    }
    for (int i = 1; i <= ccdat; ++i) {
        dat->fullRead(afl::base::fromObject(rdat));
        dis->fullRead(afl::base::fromObject(rdis));
        const int idat = rdat.shipId, idis = rdis.shipId;
        if (idat != idis) {
            syntax(Format("%s and %s do not match (ship Id).", ndat, ndis));
        }
        if (idat <= 0 || idat > NUM_SHIPS) {
            syntax(Format("%s contains invalid ship Id %d.", ndat, idat));
        }
        if (ships[idat-1].dat != 0) {
            syntax(Format("%s contains duplicate ship Id %d.", ndat, idat));
        }
        ships[idat-1].dat = new gs::Ship(rdat);
        ships[idat-1].dis = new gs::Ship(rdis);
        ships[idat-1].seen = false;
    }
    checkSigs("ship", *dat, *dis, stuff);
    logItem("Ships", Format("%d", ccdat));
}

void
game::v3::check::Checker::loadPlanets(const DirStuff& stuff)
{
    // ex check.pas:LoadPlanets
    // FIXME: FillChar(planets, Sizeof(planets), 0);
    loadXYPlan();

    // pdata
    {
        gs::Int16_t cdat, cdis;
        gs::Planet pdat, pdis;
        String_t ndat = Format("pdata%d.dat", m_player);
        String_t ndis = Format("pdata%d.dis", m_player);
        Ref<Stream> dat = openGameFile(ndat);
        Ref<Stream> dis = openGameFile(ndis);
        dat->fullRead(cdat.m_bytes);
        dis->fullRead(cdis.m_bytes);
        const int ccdat = cdat, ccdis = cdis;
        if (ccdat != ccdis) {
            syntax(Format("%s and %s do not match (count).", ndat, ndis));
        }
        if (ccdat < 0 || ccdat > NUM_PLANETS) {
            syntax(Format("%s has too large counter and is probably invalid.", ndat));
        }
        for (int i = 1; i <= ccdat; ++i) {
            dat->fullRead(afl::base::fromObject(pdat));
            dis->fullRead(afl::base::fromObject(pdis));
            const int idat = pdat.planetId, idis = pdis.planetId;
            if (idat != idis) {
                syntax(Format("%s and %s do not match (planet Id).", ndat, ndis));
            }
            if (idat <= 0 || idat > NUM_PLANETS) {
                syntax(Format("%s contains invalid planet Id %d.", ndat, idat));
            }
            if (planets[idat-1].pdat != 0) {
                syntax(Format("%s contains duplicate planet Id %d.", ndat, idat));
            }
            planets[idat-1].pdat = new gs::Planet(pdat);
            planets[idat-1].pdis = new gs::Planet(pdis);
        }
        logItem("Planets", Format("%d", ccdat));
        checkSigs("pdata", *dat, *dis, stuff);
    }

    // bdata
    {
        gs::Int16_t cdat, cdis;
        gs::Base bdat, bdis;
        String_t ndat = Format("bdata%d.dat", m_player);
        String_t ndis = Format("bdata%d.dis", m_player);
        Ref<Stream> dat = openGameFile(ndat);
        Ref<Stream> dis = openGameFile(ndis);
        dat->fullRead(cdat.m_bytes);
        dis->fullRead(cdis.m_bytes);
        const int ccdat = cdat, ccdis = cdis;
        if (ccdat != ccdis) {
            syntax(Format("%s and %s do not match (count).", ndat, ndis));
        }
        if (ccdis < 0 || ccdat > NUM_PLANETS) {
            syntax(Format("%s has too large counter and is probably invalid.", ndat));
        }
        for (int i = 1; i <= ccdat; ++i) {
            dat->fullRead(afl::base::fromObject(bdat));
            dis->fullRead(afl::base::fromObject(bdis));
            const int idat = bdat.baseId, idis = bdis.baseId;
            if (idat != idis) {
                syntax(Format("%s and %s do not match (base Id).", ndat, ndis));
            }
            if (idat <= 0 || idat > NUM_PLANETS) {
                syntax(Format("%s contains invalid planet Id %d.", ndat, idat));
            }
            if (planets[idat-1].pdat == 0) {
                syntax(Format("%s contains base at foreign planet Id %d.", ndat, idat));
            }
            if (planets[idat-1].bdat != 0) {
                syntax(Format("%s contains duplicate planet Id %s.", ndat, idat));
            }
            planets[idat-1].bdat = new gs::Base(bdat);
            planets[idat-1].bdis = new gs::Base(bdis);
        }
        logItem("Starbases", Format("%d", ccdat));
        checkSigs("bdata", *dat, *dis, stuff);
    }
}

void
game::v3::check::Checker::loadChecksums(const DirStuff& stuff)
{
    // ex check.pas:LoadChecksums
    String_t n = "control.dat";
    Ptr<Stream> f = gamedir.openFileNT(n, afl::io::FileSystem::OpenRead);
    if (f.get() == 0) {
        n = Format("contrl%d.dat", m_player);
        f = gamedir.openFileNT(n, afl::io::FileSystem::OpenRead);
    }
    if (f.get() == 0) {
        die("FATAL: Unable to find a checksum (control) file.");
    }

    // Load the file
    gs::UInt32_t data[2500];
    afl::base::fromObject(data).fill(0);
    size_t count = f->read(afl::base::fromObject(data));
    if (count < 6000) {
        syntax(Format("%s is too short", n));
    }
    f = 0;

    count /= 4;          // conveniently convert to word count
    logItem("Checksum File", n);
    logItem("Entries", Format("%d", count));

    // Now check it
    uint32_t total_p = 0;
    uint32_t total_b = 0;
    uint32_t total_s = 0;
    int num_p = 0;
    int num_b = 0;
    int num_s = 0;
    for (int i = 1; i <= NUM_SHIPS; ++i) {
        if (ships[i-1].dat != 0) {
            uint32_t unitsum = checksum(afl::base::fromObject(*ships[i-1].dat));
            uint32_t filesum;
            if (i <= 500) {
                filesum = data[i-1];
            } else {
                if (int(count) < 1500+i) {
                    logDivi();
                    logStr(Format("CHECKSUM: Checksum for ship %d is not contained in file %s.", i, n));
                    logStr("    Ships above that Id are not checked.");
                    logDivi();
                    had_ck_error = true;
                    break;
                }
                filesum = data[i+1500-1];
            }
            checkChecksum(Format("Ship %d", i), unitsum, filesum);
            total_s += unitsum;
            total_s += checksum(afl::base::fromObject(*ships[i-1].dis));
            ++num_s;
        }
    }
    for (int i = 1; i <= NUM_PLANETS; ++i) {
        if (planets[i-1].pdat != 0) {
            uint32_t unitsum = checksum(afl::base::fromObject(*planets[i-1].pdat));
            uint32_t filesum = data[i+500-1];
            checkChecksum(Format("Planet %d", i), unitsum, filesum);
            total_p += unitsum;
            total_p += checksum(afl::base::fromObject(*planets[i-1].pdis));
            ++num_p;
        }
        if (planets[i-1].bdat != 0) {
            uint32_t unitsum = checksum(afl::base::fromObject(*planets[i-1].bdat));
            uint32_t filesum = data[i+1000-1];
            checkChecksum(Format("Starbase %d", i), unitsum, filesum);
            total_b += unitsum;
            total_b += checksum(afl::base::fromObject(*planets[i-1].bdis));
            ++num_b;
        }
    }

    // For the totals check, we assume what the file would be if it were syntactically correct with correct sig block.
    gs::Int16_t num;
    uint32_t sigsum = checksum(stuff.gen.getSignature1()) + checksum(stuff.gen.getSignature2());
    num = int16_t(num_s);
    checkChecksum("Ship totals", total_s + 2*checksum(num.m_bytes) + sigsum, stuff.gen.getSectionChecksum(gs::ShipSection));
    num = int16_t(num_p);
    checkChecksum("Planet totals", total_p + 2*checksum(num.m_bytes) + sigsum, stuff.gen.getSectionChecksum(gs::PlanetSection));
    num = int16_t(num_b);
    checkChecksum("Starbase totals", total_b + 2*checksum(num.m_bytes) + sigsum, stuff.gen.getSectionChecksum(gs::BaseSection));
}

void
game::v3::check::Checker::loadResult(uint8_t (&rst_timestamp)[18])
{
    // ex check.pas:LoadResult
    loadXYPlan();

    String_t nrst = Format("player%d.rst", m_player);
    Ref<Stream> rst = openGameFile(nrst);

    gs::ResultHeader header;
    rst->fullRead(afl::base::fromObject(header));
    Stream::FileSize_t rstsize = rst->getSize();

    // Coarse validation of result file
    static const uint8_t section_block_sizes[] = { 107, 34, 85, 156, 6, 0, 0, 100 };
    static const uint16_t section_limit[] = { NUM_SHIPS, NUM_SHIPS, NUM_PLANETS, NUM_PLANETS, MAXINT, MAXINT, MAXINT, MAXINT };
    for (int i = 1; i <= 8; ++i) {
        if (header.address[i-1] <= int(sizeof(header.address)) || Stream::FileSize_t(header.address[i-1]) > rstsize) {
            syntax(Format("Section %d pointer points outside file", i));
        }
        if (section_block_sizes[i-1] != 0) {
            gs::Int16_t j;
            rst->setPos(header.address[i-1]-1);
            rst->fullRead(j.m_bytes);
            if (j < 0 || j > section_limit[i-1]) {
                syntax(Format("Section %d counter out of range", i));
            }
            if (uint32_t(j) * section_block_sizes[i-1] + header.address[i-1] > rstsize) {
                syntax(Format("Section %d truncated", i));
            }
        }
    }

    // Load timestamp
    rst->setPos(header.address[7-1]-1);
    rst->fullRead(rst_timestamp);
    if (!checkTimestamp(rst_timestamp)) {
        syntax("Time stamp has an invalid format");
    }

    // Load ships
    gs::Int16_t cdat;
    rst->setPos(header.address[1-1]-1);
    rst->fullRead(cdat.m_bytes);
    int ccdat = cdat;
    for (int i = 1; i <= ccdat; ++i) {
        Ship_t sdat;
        rst->fullRead(afl::base::fromObject(sdat));
        int id = sdat.shipId;
        if (id <= 0 || id > NUM_SHIPS) {
            syntax(Format("%s contains invalid ship Id %d.", nrst, id));
        }
        if (ships[id-1].dat != 0) {
            syntax(Format("%s contains duplicate ship Id %d.", nrst, id));
        }
        ships[id-1].dat = new Ship_t(sdat);
        ships[id-1].dis = new Ship_t(sdat);
        ships[id-1].seen = false;
    }
    logItem("Ships", Format("%d", ccdat));

    // Load planets
    rst->setPos(header.address[3-1]-1);
    rst->fullRead(cdat.m_bytes);
    ccdat = cdat;
    for (int i = 1; i <= ccdat; ++i) {
        Planet_t pdat;
        rst->fullRead(afl::base::fromObject(pdat));
        int id = pdat.planetId;
        if (id <= 0 || id > NUM_PLANETS) {
            syntax(Format("%s contains invalid planet Id %d.", nrst, id));
        }
        if (planets[id-1].pdat != 0) {
            syntax(Format("%s contains duplicate planet Id %d.", nrst, id));
        }
        planets[id-1].pdat = new Planet_t(pdat);
        planets[id-1].pdis = new Planet_t(pdat);
    }
    logItem("Planets", Format("%d", ccdat));

    rst->setPos(header.address[4-1]-1);
    rst->fullRead(cdat.m_bytes);
    ccdat = cdat;
    for (int i = 1; i <= ccdat; ++i) {
        Base_t bdat;
        rst->fullRead(afl::base::fromObject(bdat));
        int id = bdat.baseId;
        if (id <= 0 || id > NUM_PLANETS) {
            syntax(Format("%s contains invalid planet Id %d.", nrst, id));
        }
        if (planets[id-1].pdat == 0) {
            syntax(Format("%s contains base at foreign planet %d.", nrst, id));
        }
        if (planets[id-1].bdat != 0) {
            syntax(Format("%s contains duplicate planet Id %d.", nrst, id));
        }
        planets[id-1].bdat = new Base_t(bdat);
        planets[id-1].bdis = new Base_t(bdat);
    }
    logItem("Starbases", Format("%d", ccdat));
}

void
game::v3::check::Checker::loadTurn(const uint8_t (&rst_timestamp)[18])
{
    // Load turn file
    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;        // FIXME
    String_t ntrn = Format("player%d.trn", m_player);
    Ref<Stream> trn = openGameFile(ntrn);
    TurnFile tf(cs, tx, *trn, true);

    // Validate header info
    if (tf.getPlayer() != m_player) {
        syntax(Format("%s belongs to player %d, not %d", ntrn, tf.getPlayer(), m_player));
    }
    if (tf.getTimestamp() != Timestamp(rst_timestamp)) {
        syntax(Format("%s does not belong to same turn as result file", ntrn));
    }

    // Process turn file
    class MyTurnProcessor : public game::v3::trn::TurnProcessor {
     public:
        // FIXME: check.pas logs command index for all errors
        MyTurnProcessor(Checker& parent, const String_t& ntrn)
            : m_parent(parent),
              m_turnName(ntrn),
              m_didWarn(false)
            { }
        virtual void handleInvalidCommand(int code)
            {
                m_parent.logDivi();
                m_parent.logStr(Format("WARNING: unknown command with code %d.", code));
                if (!m_didWarn) {
                    m_parent.logStr("    This is not a standard VGAP turn command, and c2check does not");
                    m_parent.logStr("    know what it means. This should not happen normally. Your host");
                    m_parent.logStr("    might reject the turn file.");
                    m_didWarn = true;
                }
                m_parent.logDivi();
            }
        virtual void validateShip(int id)
            {
                // ex check.pas:LoadTurn:CheckShip
                if (id <= 0 || id > NUM_SHIPS) {
                    m_parent.syntax(Format("%s contains invalid ship Id %d", m_turnName, id));
                }
                if (m_parent.ships[id-1].dat == 0) {
                    m_parent.syntax(Format("%s refers to ship %d which is not ours", m_turnName, id));
                }
            }
        virtual void validatePlanet(int id)
            {
                // ex check.pas:LoadTurn:CheckPlanet
                if (id <= 0 || id > NUM_PLANETS) {
                    m_parent.syntax(Format("%s contains invalid planet Id %d", m_turnName, id));
                }
                if (m_parent.planets[id-1].pdat == 0) {
                    m_parent.syntax(Format("%s refers to planet %d which is not ours", m_turnName, id));
                }
            }
        virtual void validateBase(int id)
            {
                // ex check.pas:LoadTurn:CheckBase
                if (id <= 0 || id > NUM_PLANETS) {
                    m_parent.syntax(Format("%s contains invalid base Id %d", m_turnName, id));
                }
                if (m_parent.planets[id-1].bdat == 0) {
                    m_parent.syntax(Format("%s refers to base %d which is not ours", m_turnName, id));
                }
            }
        virtual void getShipData(int id, Ship_t& out, afl::charset::Charset& /*charset*/)
            {
                if (id > 0 && id <= NUM_SHIPS && m_parent.ships[id-1].dat != 0) {
                    out = *m_parent.ships[id-1].dat;
                }
            }
        virtual void getPlanetData(int id, Planet_t& out, afl::charset::Charset& /*charset*/)
            {
                if (id > 0 && id <= NUM_PLANETS && m_parent.planets[id-1].pdat != 0) {
                    out = *m_parent.planets[id-1].pdat;
                }
            }
        virtual void getBaseData(int id, Base_t& out, afl::charset::Charset& /*charset*/)
            {
                if (id > 0 && id <= NUM_PLANETS && m_parent.planets[id-1].bdat != 0) {
                    out = *m_parent.planets[id-1].bdat;
                }
            }
        virtual void storeShipData(int id, const Ship_t& in, afl::charset::Charset& /*charset*/)
            {
                if (id > 0 && id <= NUM_SHIPS && m_parent.ships[id-1].dat != 0) {
                    *m_parent.ships[id-1].dat = in;
                }
            }
        virtual void storePlanetData(int id, const Planet_t& in, afl::charset::Charset& /*charset*/)
            {
                if (id > 0 && id <= NUM_PLANETS && m_parent.planets[id-1].pdat != 0) {
                    *m_parent.planets[id-1].pdat = in;
                }
            }
        virtual void storeBaseData(int id, const Base_t& in, afl::charset::Charset& /*charset*/)
            {
                if (id > 0 && id <= NUM_PLANETS && m_parent.planets[id-1].bdat != 0) {
                    *m_parent.planets[id-1].bdat = in;
                }
            }
        virtual void addMessage(int /*to*/, String_t /*text*/)
            { }
        virtual void addNewPassword(const NewPassword_t& /*pass*/)
            { }
        virtual void addAllianceCommand(String_t /*text*/)
            { }
     private:
        Checker& m_parent;
        String_t m_turnName;
        bool m_didWarn;
    };
    MyTurnProcessor(*this, ntrn).handleTurnFile(tf, cs);
    logItem("Commands", Format("%d", tf.getNumCommands()));
}

void
game::v3::check::Checker::loadSpecs()
{
    // ex check.pas:LoadSpecs
    openSpecFile("hullspec.dat")->fullRead(afl::base::fromObject(hulls));
    openSpecFile("torpspec.dat")->fullRead(afl::base::fromObject(torps));
    openSpecFile("beamspec.dat")->fullRead(afl::base::fromObject(beams));
    openSpecFile("truehull.dat")->fullRead(afl::base::fromObject(truehull));
    openSpecFile("engspec.dat")->fullRead(afl::base::fromObject(engines));
}

bool
game::v3::check::Checker::isActive(int pl) const
{
    // ex check.pas:IsActive
    return m_player == pl;
}

/********************************* Checks ********************************/

void
game::v3::check::Checker::rangeCheckSingleValue(const String_t& s, int32_t val, int32_t min, int32_t max)
{
    // ex check.pas:RangeCheckSingleValue
    if (val >= min && val <= max) {
        return;
    }
    if (val == -1 && m_config.isHandleMinus1Special()) {
        return;
    }
    logDivi();
    logStr(Format("RANGE: %s: %s out of allowed range.", ctx, s));
    logStr(Format("    Value is %d", val));
    logStr(Format("    Allowed range is %d .. %d", min, max));
    logDivi();
    had_error = true;
}

void
game::v3::check::Checker::rangeCheckCost(const game::v3::structures::Cost& cost)
{
    // ex check.pas:RangeCheckSpecs:RangeCheckCost
    rangeCheckSingleValue("MC cost",  cost.money,      0, MAXINT);
    rangeCheckSingleValue("Tri cost", cost.tritanium,  0, MAXINT);
    rangeCheckSingleValue("Dur cost", cost.duranium,   0, MAXINT);
    rangeCheckSingleValue("Mol cost", cost.molybdenum, 0, MAXINT);
}

void
game::v3::check::Checker::rangeCheckSpecs()
{
    // ex check.pas:RangeCheckSpecs
    for (int i = 1; i <= NUM_ENGINE_TYPES; ++i) {
        ctx = Format("Engine %d", i);
        rangeCheckCost(engines[i-1].cost);
        rangeCheckSingleValue("Tech level", engines[i-1].techLevel, 1, 10);
    }

    for (int i = 1; i <= NUM_BEAM_TYPES; ++i) {
        ctx = Format("Beam %d", i);
        rangeCheckCost(beams[i-1].cost);
        rangeCheckSingleValue("Tech level", beams[i-1].techLevel, 1, 10);
    }

    for (int i = 1; i <= NUM_TORPEDO_TYPES; ++i) {
        ctx = Format("Torpedo %d", i);
        rangeCheckCost(torps[i-1].launcherCost);
        rangeCheckSingleValue("Torp MC cost", torps[i-1].torpedoCost, 0, MAXINT);
        rangeCheckSingleValue("Tech level", torps[i-1].techLevel, 1, 10);
    }

    for (int i = 1; i <= NUM_HULL_TYPES; ++i) {
        ctx = Format("Hull %d", i);
        rangeCheckSingleValue("MC cost", hulls[i-1].money, 0, MAXINT);
        rangeCheckSingleValue("Tri cost", hulls[i-1].tritanium, 0, MAXINT);
        rangeCheckSingleValue("Dur cost", hulls[i-1].duranium, 0, MAXINT);
        rangeCheckSingleValue("Mol cost", hulls[i-1].molybdenum, 0, MAXINT);
        rangeCheckSingleValue("Fuel tank", hulls[i-1].maxFuel, 0, MAXINT);
        rangeCheckSingleValue("Engines", hulls[i-1].numEngines, 1, MAXINT);
        rangeCheckSingleValue("Tech level", hulls[i-1].techLevel, 1, 10);
        rangeCheckSingleValue("Cargo room", hulls[i-1].maxCargo, 0, MAXINT);
        rangeCheckSingleValue("Fighter bay count", hulls[i-1].numBays, 0, MAXINT);
        rangeCheckSingleValue("Torp launcher count", hulls[i-1].maxLaunchers, 0, MAXINT);
        rangeCheckSingleValue("Beam count", hulls[i-1].maxBeams, 0, MAXINT);
    }

    for (int i = 1; i <= 11; ++i) {
        ctx = Format("Truehull player %d", i);
        for (int j = 1; j <= 20; ++j) {
            rangeCheckSingleValue(Format("Slot %d", j), truehull[i-1][j-1], 0, NUM_HULL_TYPES);
        }
    }
}

void
game::v3::check::Checker::checkEditable(const String_t& s, int32_t dat, int32_t dis, int32_t min, int32_t max)
{
    // ex check.pas:CheckEditable
    static bool explained = false;    // FIXME
    if (dat != dis || m_config.isPickyMode()) {
        if (dat < min || dat > max) {
            logDivi();
            logStr(Format("RANGE: %s: %s out of allowed range.", ctx, s));
            logStr(Format("    Value is %d", dat));
            if (min == max) {
                logStr(Format("    Allowed value is %d", min));
            } else {
                logStr(Format("    Allowed range is %d .. %d", min, max));
            }
            logStr(Format("    Original value was %d", dis));
            if (dat == dis) {
                logStr("    The original value is already out of range.");
                if (!explained) {
                    logStr("    This means the host made a mistake by putting this value here.");
                    logStr("    It can still confuse some programs (including host itself).");
                    explained = true;
                }
            }
            logDivi();
            had_error = true;
        }
    }
}

void
game::v3::check::Checker::checkInvariant(const String_t& s, int32_t dat, int32_t dis, int32_t min, int32_t max)
{
    // ex check.pas:CheckInvariant
    static bool explained = false;    // FIXME
    if (dat != dis) {
        logDivi();
        logStr(Format("INVALID: %s: %s was modified.", ctx, s));
        logStr(Format("    Value is %d", dat));
        logStr(Format("    Original value was %d", dis));
        if (!explained) {
            logStr("    This is not permitted by the rules, and will not be transmitted");
            logStr("    to the host. Some of the following errors may be consequences of");
            logStr("    this one. CHECK will continue with the old value, because that's");
            logStr("    what the host will see.");
            explained = true;
        }
        logDivi();
        had_error = true;
    }
    rangeCheckSingleValue(s, dis, min, max);
}

void
game::v3::check::Checker::checkTransfer(const String_t& name, const game::v3::structures::ShipTransfer& dat, const game::v3::structures::ShipTransfer& dis)
{
    // ex check.pas:RangeCheckShips:CheckTransfer
    static bool explained = false;   // FIXME
    checkEditable (name + " Colonists",   dat.colonists, dis.colonists, 0, 10000);
    checkEditable (name + " Neutronium",  dat.ore[0], dis.ore[0], 0, 10000);
    checkEditable (name + " Tritanium",   dat.ore[1], dis.ore[1], 0, 10000);
    checkEditable (name + " Duranium",    dat.ore[2], dis.ore[2], 0, 10000);
    checkEditable (name + " Molybdenum",  dat.ore[3], dis.ore[3], 0, 10000);
    checkEditable (name + " Supplies",    dat.supplies, dis.supplies, 0, 10000);
    checkEditable (name + " Target",      dat.targetId, dis.targetId, 0, NUM_SHIPS);
    if (m_config.isPickyMode() || m_config.isResultMode()) {
        if (dat.targetId != 0 && dat.colonists == 0 && dat.ore[0] == 0
            && dat.ore[1] == 0 && dat.ore[2] == 0 && dat.ore[3] == 0 && dat.supplies == 0)
        {
            // FIXME: This one happens when using planets.exe and a 3rd-party Maketurn.
            // planets.exe creates an empty host-side transfer along with every client-side transfer.
            // PHost <4.0h/3.4j rejects that as an illegal transfer (host-side cannot be between own ships),
            // a filter was added on 2004-04-11.
            // Tim-Host does not seem to consider these host-side transfers bad.
            // Also, Tims Maketurn does not send these empty transfers at all.
            // This problem is found very often in turns on PlanetsCentral (>150x).
            logDivi();
            logStr(Format("INVALID: %s: %s order is empty but has target.", ctx, name));
            if (!explained) {
                logStr("    Such orders are sometimes created by PLANETS.EXE in local data files.");
                logStr("    They might trigger false cheat alerts in the host, and should");
                logStr("    therefore not be sent to the host.");
                explained = true;
            }
            had_error = true;
            logDivi();
        }
    }
}

void
game::v3::check::Checker::checkTransferTarget(const String_t& name, const game::v3::structures::ShipTransfer& dat, int i)
{
    // ex check.pas:RangeCheckShips:CheckTransferTarget
    if (dat.colonists != 0 || dat.ore[0] != 0 || dat.ore[1] != 0
        || dat.ore[2] != 0 || dat.ore[3] != 0 || dat.supplies != 0)
    {
        if (dat.targetId != i) {
            // FIXME: this is a yellow alert for PHost.
            logDivi();
            logStr(Format("RANGE: %s: %s order has invalid target.", ctx, name));
            logStr(Format("    Value is %d", int(dat.targetId)));
            logStr(Format("    Expected value is %d", i));
            had_error = true;
            logDivi();
        }
    }
}

void
game::v3::check::Checker::rangeCheckShips()
{
    // ex check.pas:RangeCheckShips
    for (int i = 1; i <= NUM_SHIPS; ++i) {
        const Ship_t* dat = ships[i-1].dat;
        const Ship_t* dis = ships[i-1].dis;
        if (!dat || !dis) {
            continue;
        }
        ctx = Format("Ship %d", i);

        int cargo, fuel, crew;
        int hullType = dis->hullType;
        if (hullType > 0 && hullType <= NUM_HULL_TYPES) {
            cargo = hulls[hullType-1].maxCargo;
            fuel  = hulls[hullType-1].maxFuel;
            crew  = hulls[hullType-1].maxCrew;
        } else {
            cargo = 10000;
            fuel = 10000;
            crew = MAXINT;
        }
        checkInvariant("Owner",       dat->owner, dis->owner, 1, 11);
        checkEditable ("Speed",       dat->warpFactor, dis->warpFactor, 0, 9);
        checkEditable ("Waypoint DX", dat->waypointDX, dis->waypointDX, -3000, 3000);
        checkEditable ("Waypoint DY", dat->waypointDY, dis->waypointDY, -3000, 3000);
        checkInvariant("X Position",  dat->x,     dis->x,     0, 10000);
        checkInvariant("Y Position",  dat->y,     dis->y,     0, 10000);
        checkInvariant("Engine type", dat->engineType, dis->engineType, 1, NUM_ENGINE_TYPES);
        checkInvariant("Hull type",   dat->hullType, hullType,  1, NUM_HULL_TYPES);
        checkInvariant("Beam type",   dat->beamType, dis->beamType, 0, NUM_BEAM_TYPES);
        checkInvariant("Beam count",  dat->numBeams, dis->numBeams, 0, MAXINT);
        checkInvariant("Bay count",   dat->numBays, dis->numBays, 0, MAXINT);
        checkInvariant("Torp type",   dat->torpedoType, dis->torpedoType, 0, NUM_TORPEDO_TYPES);
        checkInvariant("Torp launcher count", dat->numLaunchers, dis->numLaunchers, 0, MAXINT);
        checkEditable ("Ammo",        dat->ammo, dis->ammo, 0, cargo);
        checkEditable ("Mission",     dat->mission, dis->mission, 0, 10000);
        checkEditable ("Enemy",       dat->primaryEnemy, dis->primaryEnemy, 0, 11);
        checkEditable ("Mission Tow arg", dat->missionTowParameter, dis->missionTowParameter, 0, 10000);
        checkEditable ("Mission Intercept arg", dat->missionInterceptParameter, dis->missionInterceptParameter, 0, 10000);
        checkInvariant("Damage",      dat->damage, dis->damage, 0, 150);
        if (m_config.isPickyMode()) {
            checkInvariant("Crew",      dat->crew, dis->crew, 0, crew);
        } else {
            checkInvariant("Crew",      dat->crew, dis->crew, 0, MAXINT); // do not check maximum, HOST gives too much sometimes
        }
        checkEditable ("Colonists",   dat->colonists, dis->colonists, 0, cargo);
        checkEditable ("Neutronium",  dat->ore[0], dis->ore[0], 0, fuel);
        checkEditable ("Tritanium",   dat->ore[1], dis->ore[1], 0, cargo);
        checkEditable ("Duranium",    dat->ore[2], dis->ore[2], 0, cargo);
        checkEditable ("Molybdenum",  dat->ore[3], dis->ore[3], 0, cargo);
        checkEditable ("Supplies",    dat->supplies, dis->supplies, 0, cargo);
        checkEditable ("Money",       dat->money, dis->money, 0, 10000);

        checkTransfer ("Unload",  dat->unload, dis->unload);
        checkTransferTarget("Unload", dat->unload, planetAt(dat->x, dat->y));
        checkTransfer ("Transfer", dat->transfer, dis->transfer);
    }
}

void
game::v3::check::Checker::checkComponent(const String_t& what, int want, int have, int max)
{
    // ex check.pas:CheckComponent
    if (want > max || want > have) {
        logDivi();
        logStr(Format("RANGE: %s", ctx));
        logStr(Format("    Attempt to build ship with %d %d", want, what));
        if (want > max) {
            logStr(Format("    Maximum allowed by hull is %d", max));
        }
        if (want > have) {
            logStr(Format("    Available in storage are %d", have));
        }
        logDivi();
        had_error = true;
    }
}

void
game::v3::check::Checker::rangeCheckPlanets()
{
    // ex check.pas:RangeCheckPlanets
    for (int i = 1; i <= NUM_PLANETS; ++i) {
        const Planet_t* dat = planets[i-1].pdat;
        const Planet_t* dis = planets[i-1].pdis;
        if (dat == 0 || dis == 0) {
            continue;
        }
        ctx = Format("Planet %d", i);
        checkInvariant("Owner", dat->owner, dis->owner, 0, 11);
        if (isActive(dis->owner)) {
            // editable planet
            checkEditable ("Mines",         dat->numMines,        dis->numMines,          0, 10000);
            checkEditable ("Factories",     dat->numFactories,    dis->numFactories,      0, 10000);
            checkEditable ("Defense",       dat->numDefensePosts, dis->numDefensePosts,   0, 10000);
            checkEditable ("Mined N",       dat->minedOre[0],     dis->minedOre[0],       0, 1000000000);
            checkEditable ("Mined T",       dat->minedOre[1],     dis->minedOre[1],       0, 1000000000);
            checkEditable ("Mined D",       dat->minedOre[2],     dis->minedOre[2],       0, 1000000000);
            checkEditable ("Mined M",       dat->minedOre[3],     dis->minedOre[3],       0, 1000000000);
            checkEditable ("Colonists",     dat->colonists,       dis->colonists,         0, 10000000);
            checkEditable ("Supplies",      dat->supplies,        dis->supplies,          0, 1000000000);
            checkEditable ("Money",         dat->money,           dis->money,             0, 1000000000);
            checkEditable ("Colonist Tax",  dat->colonistTax,     dis->colonistTax,       0, 100);
            checkEditable ("Native Tax",    dat->nativeTax,       dis->nativeTax,         0, 100);
            if (planets[i-1].bdat != 0) {
                checkEditable ("Base Build Order", dat->buildBaseFlag, dis->buildBaseFlag, 0, 0);
                ctx = Format("Starbase %d", i);
                const Base_t* bdat = planets[i-1].bdat;
                const Base_t* bdis = planets[i-1].bdis;
                const int baseOwner = bdis->owner;
                checkInvariant("Base Owner", bdat->owner, baseOwner, 1, 11);
                if (baseOwner != dis->owner) {
                    logDivi();
                    logStr(Format("WARNING: Starbase %d is not owned by the same player as the planet.", i));
                    logStr("    For the check, we will ignore this anomaly.");
                    logDivi();
                }
                checkEditable("Base Defense",      bdat->numBaseDefensePosts,     bdis->numBaseDefensePosts,     0, 200);
                checkEditable("Engine Tech",       bdat->techLevels[EngineTech],  bdis->techLevels[EngineTech],  1, 10);
                checkEditable("Hull Tech",         bdat->techLevels[HullTech],    bdis->techLevels[HullTech],    1, 10);
                checkEditable("Beam Tech",         bdat->techLevels[BeamTech],    bdis->techLevels[BeamTech],    1, 10);
                checkEditable("Torp Tech",         bdat->techLevels[TorpedoTech], bdis->techLevels[TorpedoTech], 1, 10);
                checkEditable("Fighters",          bdat->numFighters,             bdis->numFighters,             0, 60);
                checkEditable("Shipyard Action",   bdat->shipyardAction,          bdis->shipyardAction,          0, 2);
                checkEditable("Shipyard Ship",     bdat->shipyardId,              bdis->shipyardId,              0, 999);
                checkEditable("Base Mission",      bdat->mission,                 bdis->mission,                 0, 6);
                for (int x = 1; x <= NUM_ENGINE_TYPES; ++x) {
                    checkEditable(Format("Engine storage #%d", x), bdat->engineStorage[x-1], bdis->engineStorage[x-1], 0, MAXINT);
                }
                for (int x = 1; x <= NUM_BEAM_TYPES; ++x) {
                    checkEditable(Format("Beam storage #%d", x), bdat->beamStorage[x-1], bdis->beamStorage[x-1], 0, MAXINT);
                }
                for (int x = 1; x <= NUM_TORPEDO_TYPES; ++x) {
                    checkEditable(Format("Launcher storage #%d", x), bdat->launcherStorage[x-1], bdis->launcherStorage[x-1], 0, MAXINT);
                }
                for (int x = 1; x <= NUM_TORPEDO_TYPES; ++x) {
                    checkEditable(Format("Torpedo storage #%d", x), bdat->torpedoStorage[x-1], bdis->torpedoStorage[x-1], 0, MAXINT);
                }
                for (int x = 1; x <= 20; ++x) {
                    if (baseOwner <= 0 || baseOwner > 11 || truehull[baseOwner-1][x-1] == 0) {
                        checkEditable(Format("Unused hull storage #%d", x), bdat->hullStorage[x-1], bdis->hullStorage[x-1], 0, 0);
                    } else {
                        checkEditable(Format("Hull storage #%d", x),        bdat->hullStorage[x-1], bdis->hullStorage[x-1], 0, MAXINT);
                    }
                }
                checkEditable("Build order: Hull",       bdat->shipBuildOrder.hullIndex,    bdis->shipBuildOrder.hullIndex,    0, 20);
                checkEditable("Build order: Engine",     bdat->shipBuildOrder.engineType,   bdis->shipBuildOrder.engineType,   0, NUM_ENGINE_TYPES);
                checkEditable("Build order: Beam type",  bdat->shipBuildOrder.beamType,     bdis->shipBuildOrder.beamType,     0, NUM_BEAM_TYPES);
                checkEditable("Build order: Torp type",  bdat->shipBuildOrder.torpedoType,  bdis->shipBuildOrder.torpedoType,  0, NUM_TORPEDO_TYPES);
                if (bdat->shipBuildOrder.zero != 0) {
                    logDivi();
                    logStr(Format("WARNING: The last word of starbase %d's ship build order is not zero.", i));
                    logStr("    This may cause bad things to happen in HOST!");
                    logDivi();
                }
                if (bdat->shipBuildOrder.hullIndex > 0 && bdat->shipBuildOrder.hullIndex <= 20) {
                    int x = (baseOwner > 0 && baseOwner <= 11
                             ? truehull[baseOwner-1][bdat->shipBuildOrder.hullIndex-1]
                             : 0);
                    if (x <= 0 || x > NUM_HULL_TYPES) {
                        logDivi();
                        logStr(Format("RANGE: %s", ctx));
                        logStr("    Build order refers to a non-existant hull type.");
                        logDivi();
                        had_error = true;
                    } else {
                        if (bdat->hullStorage[bdat->shipBuildOrder.hullIndex-1] <= 0) {
                            logDivi();
                            logStr(Format("RANGE: %s", ctx));
                            logStr(Format("    Build order refers to hull slot %s, but", int(bdat->shipBuildOrder.hullIndex-1)));
                            logStr("    that hull is not available in storage.");
                            logDivi();
                            had_error = true;
                        }
                        if (bdat->shipBuildOrder.engineType <= 9) {
                            if (bdat->shipBuildOrder.engineType <= 0) {
                                logDivi();
                                logStr(Format("RANGE: %s", ctx));
                                logStr("    Attempt to build ship without engine.");
                            } else {
                                checkComponent("engines", hulls[x-1].numEngines, bdat->engineStorage[bdat->shipBuildOrder.engineType-1], hulls[x-1].numEngines);
                            }
                        }
                        if (bdat->shipBuildOrder.beamType <= NUM_BEAM_TYPES && bdat->shipBuildOrder.beamType > 0) {
                            checkComponent("beams", bdat->shipBuildOrder.numBeams, bdat->beamStorage[bdat->shipBuildOrder.beamType-1], hulls[x-1].maxBeams);
                        }
                        if (bdat->shipBuildOrder.torpedoType <= NUM_TORPEDO_TYPES && bdat->shipBuildOrder.torpedoType > 0) {
                            checkComponent("torpedo launchers", bdat->shipBuildOrder.numLaunchers, bdat->launcherStorage[bdat->shipBuildOrder.torpedoType-1], hulls[x-1].maxLaunchers);
                        }
                    }
                }
                ctx = Format("Planet %d", i);
            }
        } else {
            // everything is invariant
            // FIXME: FCode
            checkEditable ("Base Build Order",   dat->buildBaseFlag,   dis->buildBaseFlag,   0, 0);
            checkInvariant("Mines",              dat->numMines,        dis->numMines,        0, 10000);
            checkInvariant("Factories",          dat->numFactories,    dis->numFactories,    0, 10000);
            checkInvariant("Defense",            dat->numDefensePosts, dis->numDefensePosts, 0, 10000);
            checkInvariant("Mined N",            dat->minedOre[0],     dis->minedOre[0],     0, 1000000000);
            checkInvariant("Mined T",            dat->minedOre[1],     dis->minedOre[1],     0, 1000000000);
            checkInvariant("Mined D",            dat->minedOre[2],     dis->minedOre[2],     0, 1000000000);
            checkInvariant("Mined M",            dat->minedOre[3],     dis->minedOre[3],     0, 1000000000);
            checkInvariant("Colonists",          dat->colonists,       dis->colonists,       0, 10000000);
            checkInvariant("Supplies",           dat->supplies,        dis->supplies,        0, 1000000000);
            checkInvariant("Money",              dat->money,           dis->money,           0, 1000000000);
            checkInvariant("Colonist Tax",       dat->colonistTax,     dis->colonistTax,     0, 100);
            checkInvariant("Native Tax",         dat->nativeTax,       dis->nativeTax,       0, 100);
            if (planets[i-1].bdat != 0) {
                logDivi();
                logStr(Format("WARNING: Planet %d has a starbase, although it is not played.", i));
                logStr("    The starbase will be ignored by the check.");
                logDivi();
            }
        }
        checkInvariant("Ground N",           dat->groundOre[0],      dis->groundOre[0],         0, 1000000000);
        checkInvariant("Ground T",           dat->groundOre[1],      dis->groundOre[1],         0, 1000000000);
        checkInvariant("Ground D",           dat->groundOre[2],      dis->groundOre[2],         0, 1000000000);
        checkInvariant("Ground M",           dat->groundOre[3],      dis->groundOre[3],         0, 1000000000);
        checkInvariant("Density N",          dat->oreDensity[0],     dis->oreDensity[0],        0, 100);
        checkInvariant("Density T",          dat->oreDensity[1],     dis->oreDensity[1],        0, 100);
        checkInvariant("Density D",          dat->oreDensity[2],     dis->oreDensity[2],        0, 100);
        checkInvariant("Density M",          dat->oreDensity[3],     dis->oreDensity[3],        0, 100);
        checkInvariant("Colonist Happiness", dat->colonistHappiness, dis->colonistHappiness, -300, 100);
        checkInvariant("Native Happiness",   dat->nativeHappiness,   dis->nativeHappiness,   -300, 100);
        checkInvariant("Native Government",  dat->nativeGovernment,  dis->nativeGovernment,     0, 9);
        checkInvariant("Natives",            dat->natives,           dis->natives,              0, 10000000);
        checkInvariant("Native Race",        dat->nativeRace,        dis->nativeRace,           0, 9);
        checkInvariant("Temperature",        dat->temperatureCode,   dis->temperatureCode,      0, 100);
    }
}

/****************************** Flow Checks ******************************/

void
game::v3::check::Checker::addTransfer(ResourceSummary& rs, const game::v3::structures::ShipTransfer& t)
{
    // ex check.pas:AddShip:AddTransfer
    rs.n += t.ore[0];
    rs.t += t.ore[1];
    rs.d += t.ore[2];
    rs.m += t.ore[3];
    rs.clans += t.colonists;
    rs.sup += t.supplies;
}

void
game::v3::check::Checker::addShip(ResourceSummary& rs, const Ship_t& s)
{
    // ex check.pas:AddShip
    if (s.numBays != 0) {
        rs.fighters += s.ammo;
    } else if (s.torpedoType > 0 && s.torpedoType <= NUM_TORPEDO_TYPES) {
        rs.torps[s.torpedoType-1] += s.ammo;
    } else {
        // no ammo
    }

    rs.clans += s.colonists;
    rs.n += s.ore[0];
    rs.t += s.ore[1];
    rs.d += s.ore[2];
    rs.m += s.ore[3];
    rs.sup += s.supplies;
    rs.mc += s.money;
    addTransfer(rs, s.unload);
    addTransfer(rs, s.transfer);
}

void
game::v3::check::Checker::addPlanet(ResourceSummary& rs, const Planet_t& p)
{
    // ex check.pas:AddPlanet
    rs.n += p.minedOre[0];
    rs.t += p.minedOre[1];
    rs.d += p.minedOre[2];
    rs.m += p.minedOre[3];
    rs.sup += p.supplies;
    rs.mc += p.money;
    rs.clans += p.colonists;
}

void
game::v3::check::Checker::addBase(ResourceSummary& rs, const Base_t& b)
{
    // ex check.pas:AddBase
    rs.fighters += b.numFighters;
    for (int i = 1; i <= NUM_TORPEDO_TYPES; ++i) {
        rs.torps[i-1] += b.torpedoStorage[i-1];
    }
}

void
game::v3::check::Checker::tryBuy(ResourceSummary& corr, const String_t& what, int cur, int ori, int t, int d, int m, int mc, int sup, int need_tech, int have_tech)
{
    // ex check.pas:FlowCheckOrbits:TryBuy
    if (cur < ori) {
        logDivi();
        logStr(Format("RANGE: %s", ctx));
        logStr(Format("    %d %s have been sold. This is not permitted.", ori-cur, what));
        logDivi();
        had_error = true;
    }
    if (cur > ori && need_tech > have_tech) {
        logDivi();
        logStr(Format("RANGE: %s", ctx));
        logStr(Format("    %s has been built without sufficient tech.", what));
        logStr(Format("    Required tech: %d, available tech: %d", need_tech, have_tech));
        logDivi();
        had_error = true;
    }
    int32_t bought = cur - ori;
    corr.t -= t*bought;
    corr.d -= d*bought;
    corr.m -= m*bought;
    corr.mc -= mc*bought;
    corr.sup -= sup*bought;
}

void
game::v3::check::Checker::tryBuyTech(ResourceSummary& corr, const String_t& what, int cur, int ori)
{
    // ex check.pas:FlowCheckOrbits:TryBuyTech
    // FIXME: calculate these as usual
    static const int techValues[] = {0,100,300,600,1000,1500,2100,2800,3600,4500};
    if (cur < ori) {
        logDivi();
        logStr(Format("RANGE: %s", ctx));
        logStr(Format("    %s has been lowered. This is not permitted.", what));
        logDivi();
        had_error = true;
    }
    if (cur > 0 && ori > 0 && cur <= 10 && ori <= 10) {
        corr.mc -= techValues[cur-1] - techValues[ori-1];
    }
}

void
game::v3::check::Checker::checkBalance(bool& ok, const String_t& what, int32_t cur, int32_t old, int32_t corr)
{
    // ex check.pas:FlowCheckOrbits:Check
    if (corr != cur) {
        if (ok) {
            logDivi();
            logStr(Format("BALANCE: %s", ctx));
            logStr("    Resources do not match.");
            ok = false;
            had_error = true;
        }
        logStr(Format("    %-15s: start %d, now %d,", what, old, cur));
        logStr(Format("                     should be %d, difference %d", corr, cur - corr));
    }
}

void
game::v3::check::Checker::validateStructures(const String_t& what, int cur, int old, int32_t col, int cutoff)
{
    // ex check.pas:FlowCheckOrbits:ValidateStructures
    // No need to check cur<old; this is later checked in tryBuy().
    if (cur > old) {
        // FIXME: formula from library?
        int lim = (col > cutoff ? util::roundToInt(std::sqrt(col - cutoff)) + cutoff : col);
        if (cur > lim) {
            logDivi();
            logStr(Format("RANGE: %s", ctx));
            logStr(Format("    Too many %s have been built.", what));
            logStr(Format("    The limit is %d, but there are %d %s.", lim, cur, what));
            logDivi();
            had_error = true;
        }
    }
}

void
game::v3::check::Checker::flowCheckOrbits()
{
    // ex check.pas:FlowCheckOrbits
    for (int pid = 1; pid <= NUM_PLANETS; ++pid) {
        if (planets[pid-1].pdat == 0 || planets[pid-1].pdis == 0) {
            continue;
        }
        const Planet_t* pdat = planets[pid-1].pdat;
        const Planet_t* pdis = planets[pid-1].pdis;
        const Base_t* bdat = planets[pid-1].bdat;
        const Base_t* bdis = planets[pid-1].bdis;
        if (!isActive(pdat->owner)) {
            continue;
        }

        String_t note;
        for (int sid = 1; sid <= NUM_SHIPS; ++sid) {
            if (ships[sid-1].dat != 0 && ships[sid-1].dis != 0
                && ships[sid-1].dis->x == planets[pid-1].x
                && ships[sid-1].dis->y == planets[pid-1].y
                && ships[sid-1].dis->owner == pdis->owner)
            {
                // FIXME: test owner?
                note = " and orbit";
                break;
            }
        }
        ctx = Format("Planet %d%s, player %d (%d,%d)") << pid << note << int(pdis->owner) << planets[pid-1].x << planets[pid-1].y;

        ResourceSummary dat = ResourceSummary();
        ResourceSummary dis = ResourceSummary();
        addPlanet(dat, *pdat);
        addPlanet(dis, *pdis);
        if (bdat != 0 && bdis != 0) {
            addBase(dat, *bdat);
            addBase(dis, *bdis);
        }
        int nships = 0;
        for (int sid = 1; sid <= NUM_SHIPS; ++sid) {
            if (ships[sid-1].dat != 0 && ships[sid-1].dis != 0
                && ships[sid-1].dis->x == planets[pid-1].x
                && ships[sid-1].dis->y == planets[pid-1].y
                && ships[sid-1].dis->owner == pdis->owner)
            {
                if (ships[sid-1].seen) {
                    logDivi();
                    logStr(Format("WARNING: Ship %d seen again during orbits check.", sid));
                    logStr("    This usually means that your planet X/Ys are not unique.");
                    logStr("    The ship will only be processed once.");
                    logDivi();
                } else {
                    ++nships;
                    ships[sid-1].seen = true;
                    addShip(dat, *ships[sid-1].dat);
                    addShip(dis, *ships[sid-1].dis);
                }
            }
        }

        // Validate structure ranges
        validateStructures("Mines", pdat->numMines, pdis->numMines, dis.clans, 200);
        validateStructures("Factories", pdat->numFactories, pdis->numFactories, dis.clans, 100);
        validateStructures("Defense Posts", pdat->numDefensePosts, pdis->numDefensePosts, dis.clans, 50);

        // Now attempt to sell bought items, to get into balance
        ResourceSummary corr = dis;
        tryBuy(corr, "Mines",         pdat->numMines,        pdis->numMines,        0, 0, 0,  4, 1, 0, 0);
        tryBuy(corr, "Factories",     pdat->numFactories,    pdis->numFactories,    0, 0, 0,  3, 1, 0, 0);
        tryBuy(corr, "Defense Posts", pdat->numDefensePosts, pdis->numDefensePosts, 0, 0, 0, 10, 1, 0, 0);
        tryBuy(corr, "Starbase",      pdat->buildBaseFlag,   pdis->buildBaseFlag, 402, 120, 340, 900, 0, 0, 0);
        if (bdat != 0 && bdis != 0) {
            tryBuy(corr, "Base Defense", bdat->numBaseDefensePosts, bdis->numBaseDefensePosts, 0, 1, 0, 10, 0, 0, 0);
            for (int i = 1; i <= NUM_ENGINE_TYPES; ++i) {
                tryBuy(corr, Format("Engine #%d", i), bdat->engineStorage[i-1], bdis->engineStorage[i-1],
                       engines[i-1].cost.tritanium, engines[i-1].cost.duranium, engines[i-1].cost.molybdenum, engines[i-1].cost.money, 0,
                       engines[i-1].techLevel, bdat->techLevels[EngineTech]);
            }
            for (int i = 1; i <= NUM_BEAM_TYPES; ++i) {
                tryBuy(corr, Format("Beam #%d", i), bdat->beamStorage[i-1], bdis->beamStorage[i-1],
                       beams[i-1].cost.tritanium, beams[i-1].cost.duranium, beams[i-1].cost.molybdenum, beams[i-1].cost.money, 0,
                       beams[i-1].techLevel, bdat->techLevels[BeamTech]);
            }
            for (int i = 1; i <= NUM_TORPEDO_TYPES; ++i) {
                tryBuy(corr, Format("Launcher #%d", i), bdat->launcherStorage[i-1], bdis->launcherStorage[i-1],
                       torps[i-1].launcherCost.tritanium, torps[i-1].launcherCost.duranium, torps[i-1].launcherCost.molybdenum, torps[i-1].launcherCost.money, 0,
                       torps[i-1].techLevel, bdat->techLevels[TorpedoTech]);
            }
            for (int i = 1; i <= NUM_TORPEDO_TYPES; ++i) {
                tryBuy(corr, Format("Torpedo #%d", i), dat.torps[i-1], dis.torps[i-1],
                       1, 1, 1, torps[i-1].torpedoCost, 0,
                       torps[i-1].techLevel, bdat->techLevels[TorpedoTech]);
            }
            if (bdis->owner > 0 && bdis->owner <= 11) {
                for (int i = 1; i <= 20; ++i) {
                    int h = truehull[bdis->owner-1][i-1];
                    if (h > 0 && h <= NUM_HULL_TYPES) {
                        tryBuy(corr, Format("Hull #%d", h), bdat->hullStorage[i-1], bdis->hullStorage[i-1],
                               hulls[h-1].tritanium, hulls[h-1].duranium, hulls[h-1].molybdenum, hulls[h-1].money, 0,
                               hulls[h-1].techLevel, bdat->techLevels[HullTech]);
                    }
                }
            }

            tryBuy(corr, "Fighters", dat.fighters, dis.fighters, 3, 0, 2, 100, 0, 0, 0);

            // Now sell tech levels
            tryBuyTech(corr, "Engine Tech",  bdat->techLevels[0], bdis->techLevels[0]);
            tryBuyTech(corr, "Hull Tech",    bdat->techLevels[1], bdis->techLevels[1]);
            tryBuyTech(corr, "Beam Tech",    bdat->techLevels[2], bdis->techLevels[2]);
            tryBuyTech(corr, "Torpedo Tech", bdat->techLevels[3], bdis->techLevels[3]);
        }

        // Attempt to fix MC imbalance
        if (corr.mc < dat.mc) {
            corr.sup -= dat.mc - corr.mc;
            corr.mc  += dat.mc - corr.mc;
        }

        // Now validate balance
        bool ok = true;
        checkBalance(ok, "Neutronium", dat.n, dis.n, corr.n);
        checkBalance(ok, "Tritanium",  dat.t, dis.t, corr.t);
        checkBalance(ok, "Duranium",   dat.d, dis.d, corr.d);
        checkBalance(ok, "Molybdenum", dat.m, dis.m, corr.m);
        checkBalance(ok, "Money",      dat.mc, dis.mc, corr.mc);
        checkBalance(ok, "Supplies",   dat.sup, dis.sup, corr.sup);
        checkBalance(ok, "Colonists",  dat.clans, dis.clans, corr.clans);

        if (bdat == 0 || bdis == 0) {
            for (int i = 1; i <= NUM_TORPEDO_TYPES; ++i) {
                checkBalance(ok, Format("Torpedoes #%d", i), dat.torps[i-1], dis.torps[i-1], corr.torps[i-1]);
            }
            checkBalance(ok, "Fighters", dat.fighters, dis.fighters, corr.fighters);
        }
        if (!ok) {
            if (nships == 0) {
                if (bdat != 0 && bdis != 0) {
                    logStr(Format("    This incident involves planet %d with base.", pid));
                } else {
                    logStr(Format("    This incident involves planet %d.", pid));
                }
            } else {
                if (bdat != 0 && bdis != 0) {
                    logStr(Format("    This incident involves %s ship%!1{s%} and planet %d with base.", nships, pid));
                } else {
                    logStr(Format("    This incident involves %s ship%!1{s%} and planet %d.", nships, pid));
                }
            }
            logDivi();
        }
    }
}

void
game::v3::check::Checker::checkBalanceSpace(bool& ok, const String_t& what, int32_t cur, int32_t old)
{
    // ex check.pas:FlowCheckFreeSpace:Check
    if (cur > old) {
        if (ok) {
            logDivi();
            logStr(Format("BALANCE: %s", ctx));
            logStr("    Resources appeared in free space:");
            ok = false;
            had_error = true;
        }
        logStr(Format("      %-15s: start %d, now %d, difference %d", what, old, cur, cur-old));
    }
}

void
game::v3::check::Checker::flowCheckFreeSpace()
{
    // ex check.pas:FlowCheckFreeSpace
    for (int sid1 = 1; sid1 <= NUM_SHIPS; ++sid1) {
        if (ships[sid1-1].dat == 0 || ships[sid1-1].dis == 0 || ships[sid1-1].seen) {
            continue;
        }
        ctx = Format("Ship %d and other ships of player %d at (%d,%d)",
                     sid1, int(ships[sid1-1].dis->owner),
                     int(ships[sid1-1].dis->x), int(ships[sid1-1].dis->y));
        ships[sid1-1].seen = true;

        bool ok = true;
        ResourceSummary dat = ResourceSummary();
        ResourceSummary dis = ResourceSummary();
        addShip(dat, *ships[sid1-1].dat);
        addShip(dis, *ships[sid1-1].dis);
        int nships = 1;
        for (int sid2 = sid1+1; sid2 <= NUM_SHIPS; ++sid2) {
            if (ships[sid2-1].dat != 0 && ships[sid2-1].dis != 0 && !ships[sid2-1].seen
                && ships[sid1-1].dis->x == ships[sid2-1].dis->x
                && ships[sid1-1].dis->y == ships[sid2-1].dis->y
                && ships[sid1-1].dis->owner == ships[sid2-1].dis->owner)
            {
                ++nships;
                ships[sid2-1].seen = true;
                addShip(dat, *ships[sid2-1].dat);
                addShip(dis, *ships[sid2-1].dis);
            }
        }
        checkBalanceSpace(ok, "Neutronium", dat.n, dis.n);
        checkBalanceSpace(ok, "Tritanium",  dat.t, dis.t);
        checkBalanceSpace(ok, "Duranium",   dat.d, dis.d);
        checkBalanceSpace(ok, "Molybdenum", dat.m, dis.m);
        checkBalanceSpace(ok, "Money",      dat.mc, dis.mc);
        checkBalanceSpace(ok, "Supplies",   dat.sup, dis.sup);
        checkBalanceSpace(ok, "Colonists",  dat.clans, dis.clans);
        for (int i = 1; i <= NUM_TORPEDO_TYPES; ++i) {
            checkBalanceSpace(ok, Format("Torpedoes #%d", i), dat.torps[i-1], dis.torps[i-1]);
        }
        checkBalanceSpace(ok, "Fighters",   dat.fighters, dis.fighters);
        if (!ok) {
            logStr(Format("    This incident involves %d ship%!1{s%}.", nships));
            logDivi();
        }
    }
}
