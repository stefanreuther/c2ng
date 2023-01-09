/**
  *  \file game/proxy/maintenanceproxy.cpp
  *  \brief Class game::proxy::MaintenanceProxy
  */

#include "game/proxy/maintenanceproxy.hpp"
#include "afl/base/closure.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"
#include "game/maint/directorywrapper.hpp"
#include "game/maint/sweeper.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/v3/directoryscanner.hpp"
#include "game/v3/maketurn.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/turnfile.hpp"
#include "game/v3/unpacker.hpp"

using afl::base::Ref;
using afl::string::Format;
using game::config::UserConfiguration;
using game::maint::DirectoryWrapper;
using game::maint::Sweeper;
using game::v3::DirectoryScanner;
using game::v3::Maketurn;
using game::v3::Unpacker;

namespace {
    const char*const LOG_NAME = "game.proxy";
}


/*
 *  Trampoline
 */

class game::proxy::MaintenanceProxy::Trampoline : private afl::sys::LogListener, private afl::io::TextWriter {
 public:
    Trampoline(const util::RequestSender<MaintenanceProxy>& reply, MaintenanceAdaptor& ad)
        : m_reply(reply),
          m_adaptor(ad),
          m_turnFiles()
        { }

    // Entry points
    void prepareMaketurn(MaketurnStatus& result);
    void startMaketurn(PlayerSet_t players);
    void prepareUnpack(UnpackStatus& result);
    void startUnpack(PlayerSet_t players, bool uncompileTurns);
    void prepareSweep(SweepStatus& result);
    void startSweep(PlayerSet_t players, bool eraseDatabase);

    // Utilities
    void emitActionComplete();

    // LogListener:
    virtual void handleMessage(const Message& msg);

    // TextWriter:
    virtual void doWriteText(afl::string::ConstStringMemory_t data);
    virtual void doWriteNewline();
    virtual void doFlush();

 private:
    util::RequestSender<MaintenanceProxy> m_reply;
    MaintenanceAdaptor& m_adaptor;

    PlayerSet_t m_turnFiles;
};

void
game::proxy::MaintenanceProxy::Trampoline::prepareMaketurn(MaketurnStatus& result)
{
    DirectoryScanner ds(m_adaptor.translator(), *this);
    ds.scan(m_adaptor.targetDirectory(), m_adaptor.charset(), false);

    result.availablePlayers = ds.getPlayersWhere(DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveUnpacked);
    result.valid            = !result.availablePlayers.empty();
}

void
game::proxy::MaintenanceProxy::Trampoline::startMaketurn(PlayerSet_t players)
{
    afl::string::Translator& tx = m_adaptor.translator();
    try {
        // Maketurn instance
        Maketurn m(m_adaptor.targetDirectory(), m_adaptor.playerList(), m_adaptor.charset(), m_adaptor.translator());

        // Create all turns
        for (int i = 1; i <= MAX_PLAYERS; ++i) {
            if (players.contains(i)) {
                m.makeTurn(i, *this);
            }
        }

        // Write them out
        m.saveAll(*this, m_adaptor.fileSystem(), m_adaptor.userConfiguration());
        write(Info, LOG_NAME, Format(tx("Created %d turn file%!1{s%}."), m.getNumFiles()));
    }
    catch (std::exception& e) {
        write(Error, LOG_NAME, tx("Error"), e);
    }

    // Signal completion
    emitActionComplete();
}

void
game::proxy::MaintenanceProxy::Trampoline::prepareUnpack(UnpackStatus& result)
{
    DirectoryScanner ds(m_adaptor.translator(), *this);

    // First scan with resultOnly=false to find unpacked data
    ds.scan(m_adaptor.targetDirectory(), m_adaptor.charset(), false);
    PlayerSet_t selectedPlayers = ds.getPlayersWhere(DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveUnpacked);

    // Scan again with resultOnly=true to find result files
    ds.scan(m_adaptor.targetDirectory(), m_adaptor.charset(), true);

    // Save set of turn files for startUnpack()
    m_turnFiles = ds.getPlayersWhere(DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveTurn);

    // Produce output
    const PlayerList& playerList = m_adaptor.playerList();
    result.allPlayers       = playerList.getAllPlayers();
    result.availablePlayers = ds.getPlayersWhere(DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult + DirectoryScanner::HaveNewResult + DirectoryScanner::HaveOtherResult);
    result.selectedPlayers  = selectedPlayers;
    result.playerNames      = playerList.getPlayerNames(Player::ShortName, m_adaptor.translator());
    result.turnFilePlayers  = m_turnFiles;
    result.valid            = true;
}

void
game::proxy::MaintenanceProxy::Trampoline::startUnpack(PlayerSet_t players, bool uncompileTurns)
{
    afl::string::Translator& tx = m_adaptor.translator();
    try {
        // Unpacker
        afl::io::Directory& dir = m_adaptor.targetDirectory();
        Unpacker theUnpacker(tx, m_adaptor.playerList());
        theUnpacker.log().addListener(*this);

        // Configure it
        const UserConfiguration& config = m_adaptor.userConfiguration();
        theUnpacker.setFormat(config[UserConfiguration::Unpack_Format]() == UserConfiguration::UnpackFormat_DOS ? Unpacker::DosFormat : Unpacker::WindowsFormat);
        theUnpacker.setCreateTargetExt(config[UserConfiguration::Unpack_TargetExt]() != 0);
        theUnpacker.setFixErrors(config[UserConfiguration::Unpack_FixErrors]() != 0);
        // Not configured: setIgnore35Part(), setForceIgnoreErrors(), setVerbose()

        int num = 0;
        for (int i = 1; i <= MAX_PLAYERS; ++i) {
            // We assume that files exist and have matching content; this has been checked by DirectoryScanner.
            // If files go missing between prepare() and start(), that'll be caught by the general handler.
            if (players.contains(i)) {
                Ref<afl::io::Stream> rst = dir.openFile(Format("player%d.rst", i), afl::io::FileSystem::OpenRead);
                game::v3::ResultFile rstFile(*rst, tx);
                write(Info, LOG_NAME, Format(tx("Unpacking player %d: %s"), i, m_adaptor.playerList().getPlayerName(i, Player::ShortName, tx)));
                theUnpacker.prepare(rstFile, i);

                if (uncompileTurns && m_turnFiles.contains(i)) {
                    Ref<afl::io::Stream> trn = dir.openFile(Format("player%d.trn", i), afl::io::FileSystem::OpenRead);
                    game::v3::TurnFile trnFile(theUnpacker.charset(), tx, *trn);
                    write(Info, LOG_NAME, Format(tx("Applying %d turn file command%!1{s%}..."), trnFile.getNumCommands()));
                    theUnpacker.turnProcessor().handleTurnFile(trnFile, theUnpacker.charset());
                }
                theUnpacker.finish(dir, rstFile);
                ++num;

                // FIXME: detacher.loadDirectory(*gameDir, i, log(), tx);
            }
        }
        write(Info, LOG_NAME, Format(tx("Unpacked %d result file%!1{s%}."), num));
    }
    catch (std::exception& e) {
        write(Error, LOG_NAME, tx("Error"), e);
    }

    // Signal completion
    emitActionComplete();
}

void
game::proxy::MaintenanceProxy::Trampoline::prepareSweep(SweepStatus& result)
{
    DirectoryScanner ds(m_adaptor.translator(), *this);
    ds.scan(m_adaptor.targetDirectory(), m_adaptor.charset(), false);

    const PlayerList& playerList = m_adaptor.playerList();

    result.allPlayers      = playerList.getAllPlayers();
    result.selectedPlayers = ds.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveConflict));
    result.playerNames     = playerList.getPlayerNames(Player::ShortName, m_adaptor.translator());
    result.valid           = true;
}

void
game::proxy::MaintenanceProxy::Trampoline::startSweep(PlayerSet_t players, bool eraseDatabase)
{
    afl::string::Translator& tx = m_adaptor.translator();
    try {
        // DirectoryWrapper to create some logging
        Ref<DirectoryWrapper> dir(DirectoryWrapper::create(m_adaptor.targetDirectory(), *this, m_adaptor.translator()));
        dir->setEraseMode(DirectoryWrapper::LogErase);

        // Do it
        Sweeper s;
        s.setEraseDatabase(eraseDatabase);
        s.setPlayers(players);
        s.execute(*dir);

        // Finish
        write(Info, LOG_NAME, Format(tx("%d file%1{ has%|s have%} been deleted."), dir->getNumRemovedFiles()));
    }
    catch (std::exception& e) {
        write(Error, LOG_NAME, tx("Error"), e);
    }

    // Signal completion
    emitActionComplete();
}

void
game::proxy::MaintenanceProxy::Trampoline::emitActionComplete()
{
    m_reply.postRequest(&MaintenanceProxy::emitActionComplete);
}

void
game::proxy::MaintenanceProxy::Trampoline::handleMessage(const Message& msg)
{
    m_reply.postRequest(&MaintenanceProxy::emitMessage, msg.m_message);
}

void
game::proxy::MaintenanceProxy::Trampoline::doWriteText(afl::string::ConstStringMemory_t data)
{
    m_reply.postRequest(&MaintenanceProxy::emitMessage, afl::string::fromMemory(data));
}

void
game::proxy::MaintenanceProxy::Trampoline::doWriteNewline()
{ }

void
game::proxy::MaintenanceProxy::Trampoline::doFlush()
{ }



/*
 *  TrampolineFromAdaptor
 */

class game::proxy::MaintenanceProxy::TrampolineFromAdaptor : public afl::base::Closure<Trampoline*(MaintenanceAdaptor&)> {
 public:
    TrampolineFromAdaptor(const util::RequestSender<MaintenanceProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(MaintenanceAdaptor& ad)
        { return new Trampoline(m_reply, ad); }
 private:
    util::RequestSender<MaintenanceProxy> m_reply;
};


/*
 *  MaintenanceProxy
 */

game::proxy::MaintenanceProxy::MaintenanceProxy(util::RequestSender<MaintenanceAdaptor> sender, util::RequestDispatcher& reply)
    : m_receiver(reply, *this),
      m_sender(sender.makeTemporary(new TrampolineFromAdaptor(m_receiver.getSender())))
{ }

game::proxy::MaintenanceProxy::~MaintenanceProxy()
{ }

game::proxy::MaintenanceProxy::MaketurnStatus
game::proxy::MaintenanceProxy::prepareMaketurn(WaitIndicator& ind)
{
    class Task : public util::Request<Trampoline> {
     public:
        virtual void handle(Trampoline& tpl)
            { tpl.prepareMaketurn(m_result); }
        const MaketurnStatus& getResult() const
            { return m_result; }
     private:
        MaketurnStatus m_result;
    };
    Task t;
    ind.call(m_sender, t);
    return t.getResult();
}

void
game::proxy::MaintenanceProxy::startMaketurn(PlayerSet_t players)
{
    m_sender.postRequest(&Trampoline::startMaketurn, players);
}

game::proxy::MaintenanceProxy::UnpackStatus
game::proxy::MaintenanceProxy::prepareUnpack(WaitIndicator& ind)
{
    class Task : public util::Request<Trampoline> {
     public:
        virtual void handle(Trampoline& tpl)
            { tpl.prepareUnpack(m_result); }
        const UnpackStatus& getResult() const
            { return m_result; }
     private:
        UnpackStatus m_result;
    };
    Task t;
    ind.call(m_sender, t);
    return t.getResult();
}

void
game::proxy::MaintenanceProxy::startUnpack(PlayerSet_t players, bool uncompileTurns)
{
    m_sender.postRequest(&Trampoline::startUnpack, players, uncompileTurns);
}

game::proxy::MaintenanceProxy::SweepStatus
game::proxy::MaintenanceProxy::prepareSweep(WaitIndicator& ind)
{
    class Task : public util::Request<Trampoline> {
     public:
        virtual void handle(Trampoline& tpl)
            { tpl.prepareSweep(m_result); }
        const SweepStatus& getResult() const
            { return m_result; }
     private:
        SweepStatus m_result;
    };
    Task t;
    ind.call(m_sender, t);
    return t.getResult();
}

void
game::proxy::MaintenanceProxy::startSweep(PlayerSet_t players, bool eraseDatabase)
{
    m_sender.postRequest(&Trampoline::startSweep, players, eraseDatabase);
}

void
game::proxy::MaintenanceProxy::emitActionComplete()
{
    sig_actionComplete.raise();
}

void
game::proxy::MaintenanceProxy::emitMessage(String_t msg)
{
    sig_message.raise(msg);
}
