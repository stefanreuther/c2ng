/**
  *  \file game/interface/vmfile.cpp
  *  \brief VM File I/O
  *
  *  Format of VM file:
  *  <code>
  *      +0   4 BYTEs   Signature 'CCvm'
  *      +4  18 BYTEs   Timestamp
  *     +22     BYTE    Signature 26 (^Z)
  *     +23     BYTE    Format version (currently 100, PCC 1.x has 0 here)
  *     +24     WORD    Number of bytes following in header (currently 4)
  *     +26     WORD    Turn number
  *     +28     WORD    Player Id
  *  </code>
  *
  *  \todo Turn number validation is missing. PCC 1.x contains logic to
  *  - not load a VM file that has a higher turn number than the current
  *    turn
  *  - not save a VM file if the on-disk file has a higher turn number
  *    than the current turn
  *  This effectively means that scripts/auto tasks are ignored when
  *  reviewing a previous turn.
  */

#include "game/interface/vmfile.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/interface/loadcontext.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "game/v3/structures.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/vmio/filesavecontext.hpp"
#include "interpreter/vmio/objectloader.hpp"
#include "interpreter/vmio/worldloadcontext.hpp"
#include "interpreter/world.hpp"

using game::Game;
using game::Session;
using game::map::Universe;
using game::v3::structures::Timestamp_t;
using game::v3::structures::UInt16_t;
using game::v3::structures::UInt32_t;
using interpreter::BCORef_t;
using interpreter::BytecodeObject;
using interpreter::Opcode;
using interpreter::Process;
using interpreter::World;

namespace {

    /** Header of SCRIPTx.CC file. */
    struct Header {
        // -- Fixed header --
        UInt32_t    signature;        ///< Signature, 'CCvm' (MAGIC).
        Timestamp_t timestamp;        ///< Timestamp.
        uint8_t     eofMarker;        ///< MS-DOS EOF marker, 26 (Ctrl+Z, EOF_MARKER).
        uint8_t     formatVersion;    ///< File format version (VERSION).
        UInt16_t    headerSize;       ///< Number of bytes in following fields (HEADER_SIZE).

        // -- Variable header (future updates may extend this) --
        UInt16_t    turnNumber;
        UInt16_t    playerNumber;
    };
    static_assert(sizeof(Header) == 30, "sizeof Header");

    const uint32_t MAGIC      = 0x6D764343;   ///< Value for Header::signature.
    const uint8_t  EOF_MARKER = 26;           ///< Value for Header::eofMarker.
    const uint8_t  VERSION    = 100;          ///< Value for Header::formatVersion.

    const size_t PREFIX_SIZE = 26;            ///< Size of fixed header.
    const size_t HEADER_SIZE = sizeof(Header) - PREFIX_SIZE;  ///< Size of variable header.


    /** Determine whether we want to save the given process.
        We want to save: auto-tasks and the like.
        We do not want to save: UI processes (in particular, the one invoking Save command,
        and processes that deal with dialogs that cannot be saved).

        This distinction fails if an auto-task invokes the Save command;
        such a task is treated as UI process and NOT saved. */
    bool wantSaveProcess(const Process& p)
    {
        switch (p.getState()) {
         case Process::Suspended:
            // Normal suspended process.
            return true;

         case Process::Frozen:
            // Auto-task being edited. Do not lose it.
            return true;

         case Process::Runnable:
            // Scheduled for running. Typically, this is a UI process joined with another one.
            return false;

         case Process::Running:
            // This is the process that triggered the save. Typically, this is a UI process which we do not want to save.
            return false;

         case Process::Waiting:
         case Process::Ended:
         case Process::Terminated:
         case Process::Failed:
            // Final states. Don't save.
            return false;
        }
        return false;
    }

    /** Determine whether we want to keep a loaded process.
        We want to terminate auto-tasks refering to nonexistant objects,
        which means either they do not have an associated object,
        or that object is not playable in the given category. */
    bool wantKeepProcess(const Process& p, Session& session)
    {
        const Game* g = session.getGame().get();
        if (p.getState() != Process::Suspended || g == 0) {
            // Strange state, better keep it
            return true;
        } else {
            const game::map::Object* obj = dynamic_cast<game::map::Object*>(p.getInvokingObject());
            const Universe& univ = g->viewpointTurn().universe();
            switch (p.getProcessKind()) {
             case Process::pkShipTask:
                return obj != 0 && univ.playedShips().getObjectByIndex(obj->getId()) != 0;
             case Process::pkPlanetTask:
                return obj != 0 && univ.playedPlanets().getObjectByIndex(obj->getId()) != 0;
             case Process::pkBaseTask:
                return obj != 0 && univ.playedBases().getObjectByIndex(obj->getId()) != 0;
             case Process::pkDefault:
                break;
            }

            // Not an auto-task, keep it
            return true;
        }
    }
}

// Load script VM file.
void
game::interface::loadVM(Session& session, int playerNr)
{
    // ex int/vmfile.cc:loadVM
    // Determine directory
    Root* pRoot = session.getRoot().get();
    if (pRoot == 0) {
        return;
    }
    afl::io::Directory& dir = pRoot->gameDirectory();

    // Open file
    afl::base::Ptr<afl::io::Stream> file = dir.openFileNT(afl::string::Format("script%d.cc", playerNr), afl::io::FileSystem::OpenRead);
    if (file.get() == 0) {
        return;
    }

    // Load and validate header
    Header hdr;
    file->fullRead(afl::base::fromObject(hdr));
    uint16_t headerSize = hdr.headerSize;
    if (hdr.signature != MAGIC
        || hdr.formatVersion != VERSION
        || headerSize < HEADER_SIZE
        || (playerNr != 0 && hdr.playerNumber != playerNr))
    {
        throw afl::except::FileFormatException(*file, session.translator()("Invalid file header"));
    }

    // Move to data position
    file->setPos(headerSize + PREFIX_SIZE);

    // Load content
    // - LoadContext allows loading game objects
    LoadContext ctx1(session);

    // - WorldLoadContext allows loading processes
    interpreter::vmio::WorldLoadContext ctx2(ctx1, session.processList(), session.world());

    // - do it!
    interpreter::vmio::ObjectLoader(pRoot->charset(), session.translator(), ctx2).load(*file);
}

// Remove unusuable auto tasks.
void
game::interface::terminateUnusableAutoTasks(Session& session)
{
    // Function to inject
    BCORef_t code = BytecodeObject::create(true);
    code->addInstruction(Opcode::maPush, Opcode::sNamedVariable, code->addName("CC$AUTOTERMINATE"));
    code->addInstruction(Opcode::maIndirect, Opcode::miIMCall, 0);

    // Do it
    const interpreter::ProcessList::Vector_t& list = session.processList().getProcessList();
    for (size_t i = 0, n = list.size(); i < n; ++i) {
        if (list[i] != 0 && !wantKeepProcess(*list[i], session)) {
            list[i]->pushFrame(code, false);
        }
    }
}

// Save script VM file.
void
game::interface::saveVM(Session& session, int playerNr)
{
    // ex int/vmfile.cc:saveVM
    // Determine directory
    const Root* pRoot = session.getRoot().get();
    const Game* pGame = session.getGame().get();
    if (pRoot == 0 || pGame == 0) {
        return;
    }
    afl::io::Directory& dir = pRoot->gameDirectory();

    // Prepare plan
    interpreter::vmio::FileSaveContext ctx(pRoot->charset());
    const interpreter::ProcessList::Vector_t& list = session.processList().getProcessList();
    for (size_t i = 0, n = list.size(); i < n; ++i) {
        if (list[i] != 0 && wantSaveProcess(*list[i])) {
            ctx.addProcess(*list[i]);
        }
    }

    // Do we have anything to do?
    String_t fileName = afl::string::Format("script%d.cc", playerNr);
    if (ctx.getNumPreparedObjects() != 0) {
        // We must save some data
        afl::base::Ref<afl::io::Stream> file = dir.openFile(fileName, afl::io::FileSystem::Create);

        // Create header
        Header h;
        h.signature = MAGIC;
        pGame->currentTurn().getTimestamp().storeRawData(h.timestamp);
        h.eofMarker = EOF_MARKER;
        h.formatVersion = VERSION;
        h.headerSize = static_cast<uint16_t>(HEADER_SIZE);
        h.turnNumber = static_cast<int16_t>(pGame->currentTurn().getTurnNumber());
        h.playerNumber = static_cast<int16_t>(playerNr);
        file->fullWrite(afl::base::fromObject(h));

        // Write content
        ctx.save(*file);
    } else {
        // No processes to save, so erase the file
        dir.eraseNT(fileName);
    }
}
