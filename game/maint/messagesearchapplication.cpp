/**
  *  \file game/maint/messagesearchapplication.cpp
  *  \brief Class game::maint::MessageSearchApplication
  *
  *  FIXME: this is a very rough port that needs some love.
  *  We surely can carve some re-usable components out of this.
  */

#include "game/maint/messagesearchapplication.hpp"
#include "afl/base/inlinememory.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/archive/zipreader.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/limitedstream.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/playerlist.hpp"
#include "game/v3/inboxfile.hpp"
#include "game/v3/outboxreader.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/turnfile.hpp"
#include "util/charsetfactory.hpp"
#include "version.hpp"

using afl::string::Format;
namespace gt = game::v3::structures;

namespace {

    /************************** File Identification **************************/

    const uint8_t VPADBSIG[] = {'V','P','A',' ','D','a','t','a','b','a','s','e','\015','\012','\006'};

    typedef afl::io::Stream::FileSize_t FileSize_t;

    enum FileType {
        UnknownFile,
        TurnFile,
        ResultFile,
        OutboxFile,
        Outbox35File,
        InboxFile,
        VpaDatabaseFile,
        ZipArchive
    };

    bool isTurn(afl::base::ConstBytes_t data, FileSize_t total)
    {
        // Check header
        gt::TurnHeader hdr;
        if (!data.fullRead(afl::base::fromObject(hdr))) {
            return false;
        }

        // Validate ranges.
        // 100000 is an estimate how many commands a turn file can possibly contain.
        // MGREP 1.3d uses 5000, which is consistent with Host, but a turn can legally contain more commands.
        /* FIXME: this does not recognize Taccom. Should it? */
        if (hdr.playerId <= 0 || hdr.playerId > gt::NUM_PLAYERS) {
            return false;
        }
        if (hdr.numCommands < 0 || hdr.numCommands > 100000) {
            return false;
        }
        if (int32_t(afl::checksums::ByteSum().add(hdr.timestamp, 0)) != hdr.timeChecksum) {
            return false;
        }
        if (total < sizeof(hdr) + 4*hdr.numCommands) {
            return false;
        }
        return true;
    }

    bool isResult(afl::base::ConstBytes_t data, FileSize_t total)
    {
        // Check pointers in standard range
        for (int i = 0; i < 8; ++i) {
            gt::UInt32_t v;
            if (!data.fullRead(afl::base::fromObject(v))) {
                return false;
            }
            if (v < 20 || v > total) {
                return false;
            }
        }
        return true;
    }

    bool isOutboxFile(afl::base::ConstBytes_t data, FileSize_t total)
    {
        // Check count, and that the header fits in the file completely.
        gt::UInt16_t numMessagesRaw;
        if (!data.fullRead(afl::base::fromObject(numMessagesRaw))) {
            return false;
        }

        // Compute size to check. uint32_t to avoid overflow.
        uint32_t numMessages = numMessagesRaw;
        FileSize_t minSize = numMessagesRaw * sizeof(gt::OutgoingMessageHeader) + 2;
        if (total < minSize) {
            return false;
        }

        // Check all available headers. Length and position must be reasonable,
        // sender numbers must agree, receiver must be in range.
        gt::OutgoingMessageHeader hdr;
        int sender = 0;
        while (numMessages > 0 && data.fullRead(afl::base::fromObject(hdr))) {
            if (hdr.address < 0) {
                return false;
            }
            uint32_t unsignedAddress = hdr.address;
            if (unsignedAddress < minSize || unsignedAddress > total) {
                return false;
            }
            if (hdr.length < 0 || hdr.length > gt::MAX_MESSAGE_SIZE) {
                return false;
            }
            if (hdr.to <= 0 || hdr.to > gt::NUM_OWNERS) {
                return false;
            }
            if (sender == 0) {
                // first message
                if (hdr.from <= 0 || hdr.from > gt::NUM_PLAYERS) {
                    return false;
                }
                sender = hdr.from;
            } else {
                // subsequent message
                if (hdr.from != sender) {
                    return false;
                }
            }
            --numMessages;
        }
        return true;
    }

    bool isOutbox35Flag(uint8_t ch)
    {
        return (ch == '0' || ch == '1');
    }

    bool isOutbox35File(afl::base::ConstBytes_t data, FileSize_t total)
    {
        /* File consists of a word with a count, 18 garbage bytes, and 13
           flag bytes, where the flag bytes are actually part of a
           repeating structure. Check that count and flag bytes are valid.
           It makes no sense to check the repeating structure, as our
           preview will probably be too small anyway. */
        gt::Outbox35FileHeader fileHeader;
        gt::Outbox35MessageHeader messageHeader;
        if (!data.fullRead(afl::base::fromObject(fileHeader))
            || !data.fullRead(afl::base::fromObject(messageHeader))
            || total < 100)
        {
            return false;
        }
        if (fileHeader.numMessages < 0 || fileHeader.numMessages > 10000) {
            return false;
        }
        if (!isOutbox35Flag(messageHeader.validFlag)) {
            return false;
        }
        for (int i = 0; i < gt::NUM_OWNERS; ++i) {
            if (!isOutbox35Flag(messageHeader.receivers[i])) {
                return false;
            }
        }
        return true;
    }

    bool isInbox(afl::base::ConstBytes_t data, FileSize_t total)
    {
        // Check count, and that the header fits in the file completely.
        gt::UInt16_t numMessagesRaw;
        if (!data.fullRead(afl::base::fromObject(numMessagesRaw))) {
            return false;
        }

        // Compute size to check. uint32_t to avoid overflow.
        uint32_t numMessages = numMessagesRaw;
        FileSize_t minSize = numMessagesRaw * sizeof(gt::IncomingMessageHeader) + 2;
        if (total < minSize) {
            return false;
        }

        // Check all available headers. Length and position must be reasonable.
        gt::IncomingMessageHeader hdr;
        while (numMessages > 0 && data.fullRead(afl::base::fromObject(hdr))) {
            if (hdr.address < 0) {
                return false;
            }
            uint32_t unsignedAddress = hdr.address;
            if (unsignedAddress < minSize || unsignedAddress > total) {
                return false;
            }
            if (hdr.length < 0 || hdr.length > 10000) {
                return false;
            }
            --numMessages;
        }
        return true;
    }

    bool isVpaDatabase(afl::base::ConstBytes_t data, FileSize_t total)
    {
        if (total < 128 || data.size() < sizeof(VPADBSIG)) {
            return false;
        }
        return data.subrange(0, sizeof(VPADBSIG)).equalContent(VPADBSIG);
    }

    bool isZipFile(afl::base::ConstBytes_t data, FileSize_t total)
    {
        return total >= 100  /* 31 bytes local header, 47 bytes central header, 22 bytes end */
            && data.size() >= 4
            && *data.at(0) == 'P'
            && *data.at(1) == 'K'
            && *data.at(2) == 3
            && *data.at(3) == 4;
    }

    FileType identifyFile(afl::io::Stream& s)
    {
        // Read first bytes of the file
        afl::base::InlineMemory<uint8_t,512> tmp;
        tmp.trim(s.read(tmp));
        FileSize_t total = s.getSize();
        s.setPos(0);

        // Apply all checkers, most reliable first
        if (isZipFile(tmp, total)) {
            return ZipArchive;
        }
        if (isVpaDatabase(tmp, total)) {
            return VpaDatabaseFile;
        }
        if (isTurn(tmp, total)) {
            return TurnFile;
        }
        if (isResult(tmp, total)) {
            return ResultFile;
        }
        if (isOutboxFile(tmp, total)) {
            return OutboxFile;
        }
        if (isOutbox35File(tmp, total)) {
            return Outbox35File;
        }
        if (isInbox(tmp, total)) {
            return InboxFile;
        }
        return UnknownFile;
    }

    struct Message {
        String_t text;
        String_t header;
        String_t file;
        int turn;
        int index;

        String_t query;
        bool optCaseSense;

        afl::charset::Charset& cs;
        afl::io::TextWriter& out;
        afl::string::Translator& tx;

        Message(afl::charset::Charset& cs, afl::io::TextWriter& out, afl::string::Translator& tx)
            : turn(0), index(0), optCaseSense(false),
              cs(cs), out(out), tx(tx)
            { }
        void search();
    };

    class OutboxSearch : public game::v3::OutboxReader {
     public:
        OutboxSearch(Message& m)
            : m(m)
            {
                for (int i = 0; i <= game::v3::structures::NUM_PLAYERS; ++i) {
                    playerList.create(i);
                }
            }
        void addMessage(String_t text, game::PlayerSet_t receivers)
            {
                ++m.index;
                m.text   = text;
                m.header = Format("TO: %s\n", game::formatPlayerHostSet(receivers, playerList, m.tx));
                m.search();
            }
     private:
        Message& m;
        game::PlayerList playerList;   // FIXME
    };

    void Message::search()
    {
        String_t t = text;
        if (!optCaseSense) {
            t = afl::string::strUCase(t);
        }

        if (query.size() == 0 || t.find(query) != t.npos) {
            // Divider
            if (index == 0) {
                out.writeLine("--- Message ---");
            } else if (file.empty()) {
                out.writeLine(Format("--- Message %d ---", index));
            } else {
                out.writeLine(Format("--- Message %s (%s) ---", index, file));
            }

            // Header
            out.writeText(header);
            if (turn != 0) {
                out.writeLine(Format("TURN: %d", turn));
            }

            // Body
            out.writeLine(text);
        }
    }

    void searchInbox(Message& m, afl::io::Stream& s)
    {
        game::v3::InboxFile inbox(s, m.cs);
        for (size_t i = 0, n = inbox.getNumMessages(); i < n; ++i) {
            m.text = inbox.loadMessage(i);
            m.index = int(i+1);
            m.search();
        }
    }

    void searchOutbox(Message& m, afl::io::Stream& s)
    {
        OutboxSearch o(m);
        m.index = 0;
        o.loadOutbox(s, m.cs, m.tx);
    }

    void searchOutbox35(Message& m, afl::io::Stream& s)
    {
        OutboxSearch o(m);
        m.index = 0;
        o.loadOutbox35(s, m.cs, m.tx);
    }

    void searchResult(Message& m, afl::io::Stream& s)
    {
        game::v3::ResultFile rst(s, m.tx);

        // extract turn number
        FileSize_t pos;
        if (!rst.getSectionOffset(rst.GenSection, pos)) {
            return;
        }
        s.setPos(pos);

        gt::ResultGen rg;
        s.fullRead(afl::base::fromObject(rg));
        m.turn = rg.turnNumber;

        // messages
        if (!rst.getSectionOffset(rst.MessageSection, pos)) {
            return;
        }
        s.setPos(pos);
        searchInbox(m, s);
    }

    void searchTurn(Message& m, afl::io::Stream& s)
    {
        game::v3::TurnFile trn(m.cs, s);
        m.turn = trn.tryGetTurnNr();
        for (size_t i = 0, e = trn.getNumCommands(); i < e; ++i) {
            game::v3::TurnFile::CommandCode_t cc;
            int size;
            if (trn.getCommandCode(i, cc)
                && cc == game::v3::tcm_SendMessage
                && trn.getCommandId(i, size)
                && size >= 0)
            {
                // data contains [from, to, message...]
                afl::base::ConstBytes_t data = trn.getCommandData(i);
                gt::Int16_t to;
                to = 0;
                afl::base::fromObject(to).copyFrom(data.subrange(2, 2));
                m.text = game::v3::decodeMessage(data.subrange(4, size), m.cs, true);
                m.header = Format("FROM: Player %d\nTO: Player %d\n", trn.getPlayer(), int(to));
                ++m.index;
                m.search();
            }
        }
    }

    void searchVPA(Message& m, afl::io::Stream& s, afl::string::Translator& tx)
    {
        // Verify signature
        uint8_t sig[sizeof(VPADBSIG)];
        s.fullRead(sig);
        if (std::memcmp(sig, VPADBSIG, sizeof(VPADBSIG)) != 0) {
            throw afl::except::FileFormatException(s, tx("File is missing required signature"));
        }

        gt::VpaTurn turnHeader;
        while (s.read(afl::base::fromObject(turnHeader)) == sizeof(turnHeader)) {
            if (turnHeader.signature != gt::VPA_TURN_MAGIC) {
                throw afl::except::FileFormatException(s, tx("Invalid file block"));
            }
            m.turn = turnHeader.turnNumber;
            uint32_t turnSize = turnHeader.size;
            while (turnSize >= sizeof(gt::VpaChunk)) {
                // Read chunk header and advance positions
                gt::VpaChunk chunkHeader;
                s.fullRead(afl::base::fromObject(chunkHeader));
                uint32_t chunkSize = chunkHeader.size;
                turnSize -= uint32_t(sizeof(chunkHeader));
                turnSize -= std::min(turnSize, chunkSize);
                FileSize_t endPos = s.getPos() + chunkSize;

                // Check chunk
                if (chunkHeader.type == gt::VPA_IMSG_CHUNK_MAGIC) {
                    // Build virtual inbox file and read that;
                    // include the "count" word from the header
                    // FIXME: LimitedStream wants a Ref<>.
                    // This happens to work because the stream actually was born as a Ref<>,
                    // but it would be better if it were given to us as a Ref<>.
                    afl::io::LimitedStream sub(s, s.getPos()-2, chunkSize+2);
                    sub.setPos(0);
                    searchInbox(m, sub);
                } else if (chunkHeader.type == gt::VPA_OMSG_CHUNK_MAGIC) {
                    // Again, build virtual outbox file and read that.
                    // FIXME: LimitedStream wants a Ref<>.
                    afl::io::LimitedStream sub(s, s.getPos()-2, chunkSize+2);
                    sub.setPos(0);
                    searchOutbox(m, sub);
                } else {
                    // skip
                }

                // Update file pointer
                s.setPos(endPos);
            }
            if (turnSize != 0) {
                throw afl::except::FileFormatException(s, tx("Invalid file block"));
            }
        }
    }

}


struct game::maint::MessageSearchApplication::Job {
    String_t query;
    bool optCaseSense;
    bool optAllowZip;
    bool optWarnUnknown;
    FileType optFileType;
    std::auto_ptr<afl::charset::Charset> charset;

    Job()
        : query(),
          optCaseSense(false),
          optAllowZip(false),
          optWarnUnknown(true),
          optFileType(UnknownFile),
          charset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1))
        { }
    Job(const Job& other)
        : query(other.query),
          optCaseSense(other.optCaseSense),
          optAllowZip(other.optAllowZip),
          optWarnUnknown(other.optWarnUnknown),
          optFileType(other.optFileType),
          charset(other.charset->clone())
        { }
};


void
game::maint::MessageSearchApplication::appMain()
{
    afl::string::Translator& tx = translator();

    // Arguments
    bool hadSearchString = false;
    bool hadFiles = false;
    Job job;

    // Parse and search
    afl::sys::StandardCommandLineParser parser(environment().getCommandLine());
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (option) {
            if (text == "h" || text == "help") {
                help();
            } else if (text == "C") {
                // Fetch character set name
                String_t charsetName;
                if (!parser.getParameter(charsetName)) {
                    errorExit(tx("option '-C' needs an argument (the character set)"));
                }
                job.charset.reset(util::CharsetFactory().createCharset(charsetName));
                if (job.charset.get() == 0) {
                    errorExit(tx("the specified character set is not known"));
                }
            } else if (text == "r") {
                job.optFileType = ResultFile;
            } else if (text == "t") {
                job.optFileType = TurnFile;
            } else if (text == "m") {
                job.optFileType = InboxFile;
            } else if (text == "d") {
                job.optFileType = OutboxFile;
            } else if (text == "w") {
                job.optFileType = Outbox35File;
            } else if (text == "a") {
                job.optFileType = VpaDatabaseFile;
            } else if (text == "A") {
                job.optFileType = UnknownFile;
            } else if (text == "z") {
                job.optAllowZip = true;
                job.optFileType = UnknownFile;
            } else if (text == "c") {
                job.optCaseSense = true;
            } else if (text == "I") {
                job.optWarnUnknown = false;
            } else if (text == "n") {
                // ignore: disable mbox format
            } else if (text == "f") {
                // ignore: enable mbox format
            } else {
                errorExit(Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else if (!hadSearchString) {
            job.query = text;
            hadSearchString = true;
        } else {
            searchFile(text, job);
            hadFiles = true;
        }
    }

    if (!hadSearchString) {
        errorExit(Format(tx("no search string specified. Use '%s -h' for help").c_str(), environment().getInvocationName()));
    }
    if (!hadFiles) {
        errorExit(Format(tx("no file name specified. Use '%s -h' for help").c_str(), environment().getInvocationName()));
    }
}


void
game::maint::MessageSearchApplication::searchZip(afl::io::Stream& file, String_t fname, const Job& job)
{
    // Construct sub-job as modified version of existing job
    Job subjob(job);
    subjob.optAllowZip = false;
    subjob.optWarnUnknown = false;

    // Iterate through zip file
    afl::base::Ref<afl::io::Directory> zip = afl::io::archive::ZipReader::open(file, afl::io::archive::ZipReader::KeepPaths);
    afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > zipEntries = zip->getDirectoryEntries();
    afl::base::Ptr<afl::io::DirectoryEntry> zipEntry;
    while (zipEntries->getNextElement(zipEntry)) {
        if (zipEntry.get() != 0 && zipEntry->getFileType() == afl::io::DirectoryEntry::tFile) {
            afl::base::Ref<afl::io::Stream> entryStream = zipEntry->openFile(afl::io::FileSystem::OpenRead);
            if (!entryStream->hasCapabilities(afl::io::Stream::CanSeek)) {
                // Entry cannot seek. Read into memory.
                afl::base::Ref<afl::io::InternalStream> contentStream = *new afl::io::InternalStream();
                contentStream->copyFrom(*entryStream);
                contentStream->setPos(0);
                contentStream->setWritePermission(false);
                entryStream.reset(*contentStream);
            }
            searchStream(*entryStream, Format("%s(%s)", fname, zipEntry->getTitle()), subjob);
        }
    }
}

void
game::maint::MessageSearchApplication::searchStream(afl::io::Stream& file, const String_t& fname, const Job& job)
{
    afl::string::Translator& tx = translator();

    FileType type = job.optFileType == UnknownFile ? identifyFile(file) : job.optFileType;

    Message m(*job.charset, standardOutput(), translator());
    m.file = fname;
    m.optCaseSense = job.optCaseSense;
    m.query = job.optCaseSense ? job.query : afl::string::strUCase(job.query);

    switch (type) {
     case TurnFile:
        searchTurn(m, file);
        break;
     case ResultFile:
        searchResult(m, file);
        break;
     case OutboxFile:
        searchOutbox(m, file);
        break;
     case Outbox35File:
        searchOutbox35(m, file);
        break;
     case InboxFile:
        searchInbox(m, file);
        break;
     case VpaDatabaseFile:
        searchVPA(m, file, tx);
        break;
     case ZipArchive:
        if (job.optAllowZip) {
            searchZip(file, fname, job);
        } else {
            errorOutput().writeLine(Format(tx("%s: compressed file").c_str(), fname));
        }
        break;
     case UnknownFile:
        if (job.optWarnUnknown) {
            errorOutput().writeLine(Format(tx("%s: unknown file format").c_str(), fname));
        }
        break;
    }
}

void
game::maint::MessageSearchApplication::searchFile(const String_t& fname, const Job& job)
{
    try {
        afl::base::Ref<afl::io::Stream> file = fileSystem().openFile(fname, afl::io::FileSystem::OpenRead);
        searchStream(*file, fname, job);
    }
    catch (afl::except::FileProblemException& ex) {
        errorOutput().writeLine(Format("%s: %s", ex.getFileName(), ex.what()));
    }
    catch (std::exception& ex) {
        errorOutput().writeLine(Format("%s: %s", fname, ex.what()));
    }
}


/** Exit with help message. */
void
game::maint::MessageSearchApplication::help()
{
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(Format(tx("PCC2 Message Search v%s - (c) 2011-2020 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-h]\n"
                            "  %$0s [-opts] \"search string\" [-type] files...\n\n"
                            "Options:\n"
                            "  -c           Case-sensitive\n"
                            "  -C CHARSET   Select character set\n"
                            "\n"
                            "Type options apply to all subsequent file names:\n"
                            "  -r           Result files\n"
                            "  -t           Turn files\n"
                            "  -m           Message inbox (MDATA)\n"
                            "  -d           Dosplan outbox (MESS)\n"
                            "  -w           Winplan outbox (MESS35)\n"
                            "  -a           VPA database\n"
                            "  -A           Auto-detect (default)\n"
                            "  -z           Search in ZIP files (implies -A)\n"
                            "  -I           Ignore unknown files (default: warn)\n"
                            "\n"
                            "Report bugs to <Streu@gmx.de>").c_str(),
                         environment().getInvocationName()));
    out.flush();
    exit(0);
}
