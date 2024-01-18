/**
  *  \file test/game/interface/drawingfunctiontest.cpp
  *  \brief Test for game::interface::DrawingFunction
  */

#include "game/interface/drawingfunction.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/drawing.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

AFL_TEST("game.interface.DrawingFunction", a)
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
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
    a.checkEqual("01. getDimension", testee.getDimension(0), 0);

    // Cannot invoke or assign to
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a("11. get()"), testee.get(args), interpreter::Error);
    AFL_CHECK_THROWS(a("12. set()"), testee.set(args, 0), interpreter::Error);

    // Can iterate
    std::auto_ptr<game::interface::DrawingContext> ctx(testee.makeFirstContext());
    a.checkNonNull("21. makeFirstContext", ctx.get());
    interpreter::test::ContextVerifier(*ctx, a("ctx")).verifyInteger("LOC.Y", 1200);
}
