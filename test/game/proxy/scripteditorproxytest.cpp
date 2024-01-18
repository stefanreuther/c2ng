/**
  *  \file test/game/proxy/scripteditorproxytest.cpp
  *  \brief Test for game::proxy::ScriptEditorProxy
  */

#include "game/proxy/scripteditorproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include <algorithm>

using game::interface::PropertyList;

namespace {
    const game::Id_t SHIP_ID = 111;

    // ContextProvider that creates a ship context
    class ContextProvider : public game::interface::ContextProvider {
     public:
        virtual void createContext(game::Session& session, interpreter::ContextReceiver& recv)
            { recv.pushNewContext(game::interface::ShipContext::create(SHIP_ID, session)); }
    };

    void createShip(game::test::SessionThread& h)
    {
        h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        h.session().setShipList(new game::spec::ShipList());
        h.session().setGame(new game::Game());
        h.session().getGame()->currentTurn().universe().ships().create(111);
        h.session().world().shipPropertyNames().add("XYZZYSHIP");
    }

    const PropertyList::Info* find(const PropertyList& pl, String_t name)
    {
        for (size_t i = 0; i < pl.infos.size(); ++i) {
            if (pl.infos[i].name == name) {
                return &pl.infos[i];
            }
        }
        return 0;
    }
}

/** Test buildCompletionList(), with no ContextProvider.
    A: set up empty session. Call buildCompletionList().
    E: expected result produced, with global variable names */
AFL_TEST("game.proxy.ScriptEditorProxy:buildCompletionList", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;

    // Define some unique names
    h.session().world().setNewGlobalValue("XYZZYFAZ", 0);
    h.session().world().setNewGlobalValue("XYZZYFOO", 0);

    // Attempt completion
    game::proxy::ScriptEditorProxy testee(h.gameSender());
    game::interface::CompletionList result;
    testee.buildCompletionList(ind, result, "print XyZz", false, std::auto_ptr<game::interface::ContextProvider>());

    // Verify
    a.checkEqual("01. getStem", result.getStem(), "XyZz");
    a.checkEqual("02. getImmediateCompletion", result.getImmediateCompletion(), "Xyzzyf");

    a.check("11. completion", std::find(result.begin(), result.end(), "Xyzzyfaz") != result.end());
    a.check("12. completion", std::find(result.begin(), result.end(), "Xyzzyfoo") != result.end());
}

/** Test buildCompletionList(), with ContextProvider.
    A: set up session with an object. Call buildCompletionList() with a matching ContextProvider.
    E: expected result produced, with object property names */
AFL_TEST("game.proxy.ScriptEditorProxy:buildCompletionList:with-context", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;

    // Define an object with properties
    createShip(h);

    // Attempt completion
    game::proxy::ScriptEditorProxy testee(h.gameSender());
    game::interface::CompletionList result;
    testee.buildCompletionList(ind, result, "print XyZz", false, std::auto_ptr<game::interface::ContextProvider>(new ContextProvider()));

    // Verify
    a.checkEqual("01. getStem", result.getStem(), "XyZz");
    a.checkEqual("02. getImmediateCompletion", result.getImmediateCompletion(), "Xyzzyship");
}

/** Test buildPropertyList(), with ContextProvider.
    A: set up session with an object. Call buildPropertyList() with a matching ContextProvider.
    E: expected result produced, with object property names and values */
AFL_TEST("game.proxy.ScriptEditorProxy:buildPropertyList", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;

    // Define an object with properties
    createShip(h);

    // Retrieve list
    game::proxy::ScriptEditorProxy testee(h.gameSender());
    game::interface::PropertyList result;
    testee.buildPropertyList(ind, result, std::auto_ptr<game::interface::ContextProvider>(new ContextProvider()));

    // Verify
    const PropertyList::Info* pi = find(result, "Xyzzyship");
    a.checkNonNull("01. result", pi);
    a.checkEqual("02. value", pi->value, "Empty");
}
