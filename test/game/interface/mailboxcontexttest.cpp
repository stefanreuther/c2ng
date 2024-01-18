/**
  *  \file test/game/interface/mailboxcontexttest.cpp
  *  \brief Test for game::interface::MailboxContext
  */

#include "game/interface/mailboxcontext.hpp"

#include <stdexcept>
#include "afl/charset/utf8charset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/specificationloader.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/stringverifier.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/test/contextverifier.hpp"

using afl::base::Ref;
using afl::io::InternalDirectory;
using afl::io::FileSystem;

/** Test basics. */
AFL_TEST("game.interface.MailboxContext:basics", a)
{
    // Create
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);
    std::auto_ptr<game::interface::MailboxContext> ctx(game::interface::MailboxContext::create(session));
    a.checkNonNull("01. ctx", ctx.get());

    // Verify
    interpreter::test::ContextVerifier verif(*ctx, a);
    verif.verifyTypes();
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("11. next", ctx->next(), false);
    a.checkNull("12. getObject", ctx->getObject());

    std::auto_ptr<game::interface::MailboxContext> copy(ctx->clone());
    a.checkNonNull("21. clone", copy.get());
    a.checkEqual("22. mailbox", &ctx->mailbox(), &copy->mailbox());
}

/** Test Add command. */
AFL_TEST("game.interface.MailboxContext:Add", a)
{
    // Create
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    std::auto_ptr<game::interface::MailboxContext> ctx(game::interface::MailboxContext::create(session));
    a.checkNonNull("01. ctx", ctx.get());

    // Retrieve adder
    interpreter::test::ContextVerifier verif(*ctx, a);
    std::auto_ptr<afl::data::Value> add(verif.getValue("ADD"));

    interpreter::CallableValue* cvAdd = dynamic_cast<interpreter::CallableValue*>(add.get());
    a.checkNonNull("11. cvAdd", cvAdd);
    a.check("12. isProcedureCall", cvAdd->isProcedureCall());

    // Invoke adder
    interpreter::Process proc(session.world(), "testAdd", 99);
    afl::data::Segment args;
    args.pushBackString("msg");
    args.pushBackInteger(42);
    args.pushBackNew(new game::interface::ReferenceContext(game::Reference(game::Reference::Planet, 77), session));
    cvAdd->call(proc, args, false);

    // Verify result
    game::PlayerList players;
    a.checkEqual("21. getNumMessages", ctx->mailbox().getNumMessages(), 1U);
    a.checkEqual("22. getMessageText", ctx->mailbox().getMessageText(0, tx, players), "msg");
    a.checkEqual("23. turnNumber",     ctx->mailbox().getMessageMetadata(0, tx, players).turnNumber, 42);
    a.checkEqual("24. primaryLink",    ctx->mailbox().getMessageMetadata(0, tx, players).primaryLink, game::Reference(game::Reference::Planet, 77));
}

/** Test LoadUtilData command. */
AFL_TEST("game.interface.MailboxContext:LoadUtilData", a)
{
    class SpecLoader : public game::SpecificationLoader {
     public:
        SpecLoader(Ref<afl::io::Directory> dir)
            : m_dir(dir)
            { }
        virtual std::auto_ptr<game::Task_t> loadShipList(game::spec::ShipList& /*list*/, game::Root& /*root*/, std::auto_ptr<game::StatusTask_t> /*then*/)
            { throw std::runtime_error("loadShipList unexpected"); }
        virtual Ref<afl::io::Stream> openSpecificationFile(const String_t& fileName)
            { return m_dir->openFile(fileName, FileSystem::OpenRead); }
     private:
        Ref<afl::io::Directory> m_dir;
    };

    // Prepare directories
    // - subset of a proper util.dat file
    static const uint8_t UTIL[] = {
        0x0d, 0x00, 0x59, 0x00, 0x30, 0x33, 0x2d, 0x30, 0x31, 0x2d, 0x32, 0x30, 0x31, 0x38, 0x32, 0x30,
        0x3a, 0x30, 0x30, 0x3a, 0x30, 0x32, 0x1e, 0x00, 0x06, 0x00, 0x04, 0x01, 0x23, 0xcd, 0x28, 0x9d,
        0x22, 0xc6, 0x2a, 0x0e, 0x66, 0x1c, 0xf0, 0x1d, 0x8d, 0x2a, 0xde, 0x4a, 0xb7, 0x62, 0x36, 0x6a,
        0x18, 0x97, 0xa2, 0xb2, 0x6e, 0x3f, 0x0e, 0xae, 0xd3, 0xab, 0xdf, 0x91, 0x4e, 0x6f, 0x72, 0x74,
        0x68, 0x20, 0x53, 0x74, 0x61, 0x72, 0x20, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68,

        0x37, 0x00, 0x08, 0x00, 0x25, 0x00, 0x08, 0x00, 0x02, 0x00, 0x77, 0x01,
    };
    // - subset of a proper utildata.ini file
    const char*const SPEC =
        "13,Control Record\n"
        "        h = (-h0000)\n"
        "        t = Turn %18w for player %20w\n"
        "        t =\n"
        "        t = Host Time: %0S10 at %10S08\n"
        "        t = Version:   PHost %22b.%23b%88?S01\n"
        "        t = Game Name: %56S32\n"
        "55,Production Report\n"
        "        h = (-s%W)\n"
        "        t = Ship Id:   %w\n"
        "        t = Produced: %6w %2(kt Fuel,kt Tritanium,kt Duranium,kt Molybdenum,Colonist clans,kt Supplies,mc,Torpedoes/Fighters,Experience)\n"
        "        t = %(No resources used,Ship cargo used,Planetary resources used,Ship and planet resources used)\n";

    Ref<InternalDirectory> gameDir = InternalDirectory::create("gameDir");
    gameDir->openFile("util3.dat", FileSystem::Create)->fullWrite(UTIL);

    Ref<InternalDirectory> specDir = InternalDirectory::create("specDir");
    specDir->openFile("utildata.ini", FileSystem::Create)->fullWrite(afl::string::toBytes(SPEC));

    // Create
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    session.getGame()->setViewpointPlayer(3);
    session.setRoot(new game::Root(gameDir,
                                   *new SpecLoader(specDir),
                                   game::HostVersion(),
                                   std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Registered, 10)),
                                   std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                                   std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                   game::Root::Actions_t()));

    std::auto_ptr<game::interface::MailboxContext> ctx(game::interface::MailboxContext::create(session));
    a.checkNonNull("01. ctx", ctx.get());

    // Retrieve loader
    interpreter::test::ContextVerifier verif(*ctx, a);
    std::auto_ptr<afl::data::Value> load(verif.getValue("LOADUTILDATA"));

    interpreter::CallableValue* cvLoad = dynamic_cast<interpreter::CallableValue*>(load.get());
    a.checkNonNull("11. cvLoad", cvLoad);
    a.check("12. isProcedureCall", cvLoad->isProcedureCall());

    // Invoke loader
    interpreter::Process proc(session.world(), "testLoadUtilData", 99);
    afl::data::Segment args;
    cvLoad->call(proc, args, false);

    // Verify result
    game::PlayerList players;
    a.checkEqual("21. getNumMessages", ctx->mailbox().getNumMessages(), 2U);
    a.checkEqual("22. message 0", ctx->mailbox().getMessageText(0, tx, players),
                 "(-h0000)<<< Control Record >>>\n\n"
                 "Record type 13, 89 bytes\n\n"
                 "Turn 30 for player 6\n\n"
                 "Host Time: 03-01-2018 at 20:00:02\n"
                 "Version:   PHost 4.1h\n"
                 "Game Name: North Star 4\n");
    a.checkEqual("23. message 1", ctx->mailbox().getMessageText(1, tx, players),
                 "(-s0037)<<< Production Report >>>\n\n"
                 "Record type 55, 8 bytes\n\n"
                 "Ship Id:   37\n"
                 "Produced: 375 Experience\n"
                 "Planetary resources used\n");
    a.checkEqual("24. turnNumber",  ctx->mailbox().getMessageMetadata(0, tx, players).turnNumber, 30);
    a.checkEqual("25. turnNumber",  ctx->mailbox().getMessageMetadata(1, tx, players).turnNumber, 30);
    a.checkEqual("26. primaryLink", ctx->mailbox().getMessageMetadata(0, tx, players).primaryLink, game::Reference());
    a.checkEqual("27. primaryLink", ctx->mailbox().getMessageMetadata(1, tx, players).primaryLink, game::Reference(game::Reference::Ship, 37));
}

AFL_TEST("game.interface.MailboxContext:LoadFile", a)
{
    // Arbitrary file descriptor to use
    const int FD = 5;

    // Test file (valid output test case)
    const char*const FILE =
        "=== Turn 10 ===\n"
        "   2 message(s)\n"
        "--- Message 2 ---\n"
        "second header\n"
        "second body\n"
        "\n"
        "--- Message 3 ---\n"
        "(-r3000)<<< Data Transmission >>>\n"
        "<<< VPA Data Transmission >>>\n\n"
        "OBJECT: Mine field 61\n"
        "DATA: 2094989326\n"
        "ocaalekakbhadaaaijmcaaaaaaaa\n";

    // Create
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);

    // Provide test file
    session.world().fileTable().openFile(FD, *new afl::io::ConstMemoryStream(afl::string::toBytes(FILE)));

    // Test
    std::auto_ptr<game::interface::MailboxContext> ctx(game::interface::MailboxContext::create(session));
    a.checkNonNull("01. ctx", ctx.get());

    // Retrieve loader
    interpreter::test::ContextVerifier verif(*ctx, a);
    std::auto_ptr<afl::data::Value> load(verif.getValue("LOADFILE"));

    interpreter::CallableValue* cvLoad = dynamic_cast<interpreter::CallableValue*>(load.get());
    a.checkNonNull("11. cvLoad", cvLoad);
    a.check("12. isProcedureCall", cvLoad->isProcedureCall());

    // Invoke loader
    interpreter::Process proc(session.world(), "testLoadFile", 99);
    afl::data::Segment args;
    args.pushBackInteger(FD);
    cvLoad->call(proc, args, false);

    // Verify result
    game::PlayerList players;
    a.checkEqual("21. getNumMessages", ctx->mailbox().getNumMessages(), 2U);
    a.checkEqual("22. getMessageText", ctx->mailbox().getMessageText(0, tx, players),
                 "second header\n"
                 "second body\n");
    a.checkEqual("23. getMessageText", ctx->mailbox().getMessageText(1, tx, players),
                 "(-r3000)<<< Data Transmission >>>\n"
                 "<<< VPA Data Transmission >>>\n\n"
                 "OBJECT: Mine field 61\n"
                 "DATA: 2094989326\n"
                 "ocaalekakbhadaaaijmcaaaaaaaa\n");
    a.checkEqual("24. turnNumber",  ctx->mailbox().getMessageMetadata(0, tx, players).turnNumber, 10);
    a.checkEqual("25. turnNumber",  ctx->mailbox().getMessageMetadata(1, tx, players).turnNumber, 10);
    a.checkEqual("26. primaryLink", ctx->mailbox().getMessageMetadata(0, tx, players).primaryLink, game::Reference());
    a.checkEqual("27. primaryLink", ctx->mailbox().getMessageMetadata(1, tx, players).primaryLink, game::Reference(game::Reference::Minefield, 61));
}

/** Test public interface. */
AFL_TEST("game.interface.MailboxContext:IFMailbox", a)
{
    // Environment
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);

    std::auto_ptr<afl::data::Value> p;

    // Normal case
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        p.reset(game::interface::IFMailbox(session, args));
        a.checkNonNull("01. success", dynamic_cast<game::interface::MailboxContext*>(p.get()));
    }

    // Error case
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("11. arity error"), p.reset(game::interface::IFMailbox(session, args)), interpreter::Error);
    }
}
