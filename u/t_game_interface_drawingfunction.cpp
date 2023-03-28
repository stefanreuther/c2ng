/**
  *  \file u/t_game_interface_drawingfunction.cpp
  *  \brief Test for game::interface::DrawingFunction
  */

#include "game/interface/drawingfunction.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/drawing.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

void
TestGameInterfaceDrawingFunction::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    session.getGame()->currentTurn().universe().drawings().addNew(new game::map::Drawing(game::map::Point(1100, 1200), game::map::Drawing::MarkerDrawing));

    // Simple properties
    game::interface::DrawingFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();
    TS_ASSERT_EQUALS(testee.getDimension(0), 0);

    // Cannot invoke or assign to
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);

    // Can iterate
    std::auto_ptr<game::interface::DrawingContext> ctx(testee.makeFirstContext());
    TS_ASSERT(ctx.get() != 0);
    interpreter::test::ContextVerifier(*ctx, "testIt: ctx").verifyInteger("LOC.Y", 1200);
}

