/**
  *  \file test/game/interface/globalactionstest.cpp
  *  \brief Test for game::interface::GlobalActions
  */

#include "game/interface/globalactions.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/process.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/values.hpp"

using game::interface::GlobalActions;
using game::map::Universe;
using interpreter::BCORef_t;
using interpreter::Opcode;
using interpreter::Process;

namespace {
    /*
     *  A Function implementation that logs calls into an accumulator string
     */
    class Function : public interpreter::CallableValue {
     public:
        /** Constructor.
            @param [in]     name   Name to use for logging
            @param [in]     isProc Result for isProcedureCall()
            @param [in,out] acc    Accumulator
            @param [in]     result Result to return */
        Function(String_t name, bool isProc, String_t& acc, const afl::data::Value* result)
            : CallableValue(), m_name(name), m_isProcedure(isProc), m_acc(acc),
              m_result(afl::data::Value::cloneOf(result))
            { }

        virtual void call(Process& proc, afl::data::Segment& args, bool want_result)
            {
                m_acc += m_name;
                m_acc += "(";
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i != 0) {
                        m_acc += ",";
                    }
                    if (interpreter::StructureValue* sv = dynamic_cast<interpreter::StructureValue*>(args[i])) {
                        m_acc += "{";
                        interpreter::StructureValueData::Ref_t v = sv->getValue();
                        const afl::data::NameMap& names = v->type().names();
                        for (size_t i = 0; i < names.getNumNames(); ++i) {
                            if (i != 0) {
                                m_acc += ",";
                            }
                            m_acc += names.getNameByIndex(i);
                            m_acc += ":";
                            m_acc += interpreter::toString(v->data()[i], true);
                        }
                        m_acc += "}";
                    } else {
                        m_acc += interpreter::toString(args[i], true);
                    }
                }
                if (want_result) {
                    proc.pushNewValue(afl::data::Value::cloneOf(m_result.get()));
                }
                m_acc += ")";
            }

        virtual bool isProcedureCall() const
            { return m_isProcedure; }
        virtual size_t getDimension(size_t /*which*/) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { return 0; }
        virtual interpreter::CallableValue* clone() const
            { return new Function(m_name, m_isProcedure, m_acc, m_result.get()); }
        virtual String_t toString(bool /*readable*/) const
            { return afl::string::Format("<%s>", m_name); }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
        // Portability hack for older compilers:
        const Function& self() const
            { return *this; }
     private:
        String_t m_name;
        bool m_isProcedure;
        String_t& m_acc;
        std::auto_ptr<afl::data::Value> m_result;
    };

    /* Create a default test action. */
    size_t createTestAction(afl::test::Assert a, GlobalActions& ga, String_t& acc)
    {
        afl::data::IntegerValue one(1);
        size_t actionNr = ga.addAction(Function("prepare", false, acc, &one).self(),
                                       Function("exec", true, acc, 0).self(),
                                       Function("result", true, acc, 0).self());
        a.checkNonNull("createTestAction: getActionByIndex", ga.getActionByIndex(actionNr));
        return actionNr;
    }

    /* Test universe with some objects. */
    struct TestUniverse {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        TestUniverse()
            : tx(), fs(), session(tx, fs)
            {
                const game::map::Point P(1000,1000);
                const game::PlayerSet_t S(3);
                session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
                session.setShipList(new game::spec::ShipList());
                session.setGame(new game::Game());
                Universe& univ = session.getGame()->currentTurn().universe();
                univ.ships().create(10)->addShipXYData(P, 10, 100, S);
                univ.ships().create(20)->addShipXYData(P, 10, 100, S);
                univ.planets().create(15)->setPosition(P);
                univ.planets().create(23)->setPosition(P);
                univ.planets().create(47)->setPosition(P);
                session.postprocessTurn(session.getGame()->currentTurn(), S, S, game::map::Object::Playable);
            }
    };

    BCORef_t makeBCO()
    {
        return interpreter::BytecodeObject::create(false);
    }

    /* Create object list to iterate through */
    game::ref::List createList()
    {
        game::ref::List list;
        list.add(game::Reference(game::Reference::Ship, 20));
        list.add(game::Reference(game::Reference::Planet, 47));
        list.add(game::Reference(game::Reference::Minefield, 7));
        list.add(game::Reference(game::Reference::Ship, 10));
        return list;
    }

    /* Compile a 'With Lock(<lockName>) Do Stop' command */
    BCORef_t makeTakeLockBCO(String_t lockName)
    {
        BCORef_t result = makeBCO();

        // Lock name
        afl::data::StringValue sv(lockName);
        result->addPushLiteral(&sv);
        result->addInstruction(Opcode::maPush, Opcode::sNamedVariable, result->addName("LOCK"));
        result->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
        return result;
    }

    /* Compile an action using a lock:
         Sub action(obj,state)
           With Lock("p"&obj->Id) Do
             cv
           EndWith
         EndSub */
    BCORef_t makeTakeLockActionBCO(const interpreter::CallableValue& cv)
    {
        BCORef_t result = makeBCO();
        result->setIsProcedure(true);
        result->addArgument("OBJ", false);
        result->addArgument("STATE", false);

        afl::data::StringValue sv("p");
        result->addPushLiteral(&sv);
        result->addInstruction(Opcode::maPush, Opcode::sLocal, 0);
        result->addInstruction(Opcode::maMemref, Opcode::miIMLoad, result->addName("ID"));
        result->addInstruction(Opcode::maBinary, interpreter::biConcatEmpty, 0);
        result->addInstruction(Opcode::maStack, Opcode::miStackDup, 0);
        result->addInstruction(Opcode::maPush, Opcode::sNamedVariable, result->addName("LOCK"));
        result->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        result->addPushLiteral(&cv);
        result->addInstruction(Opcode::maIndirect, Opcode::miIMCall, 1);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
        return result;
    }
}

/** Test normal operation. */
AFL_TEST("game.interface.GlobalActions:normal", a)
{
    // Define one global action
    GlobalActions testee;
    String_t acc;
    size_t actionNr = createTestAction(a, testee, acc);

    // Define a universe with some units
    TestUniverse h;

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileGlobalAction(testee.getActionByIndex(actionNr),
                                              h.session.world(),
                                              GlobalActions::Flags_t()),
                   false);

    // Run it
    // (For the test, don't use process groups etc., we don't need that synchronisation for now.)
    proc.run(0);

    // Verify result
    a.checkEqual("01. getState", proc.getState(), Process::Ended);
    a.checkEqual("02. acc", acc,
                 "prepare()"
                 "exec(Ship(10),1)exec(Ship(20),1)"
                 "exec(Planet(15),1)exec(Planet(23),1)exec(Planet(47),1)"
                 "result(1,{NUMSUCCESS:5,NUMERRORS:0,ERROR:Z(0)})");
}

/** Test handling of the ExcludeShips flag. */
AFL_TEST("game.interface.GlobalActions:compileGlobalAction:ExcludeShips", a)
{
    // Define one global action
    GlobalActions testee;
    String_t acc;
    size_t actionNr = createTestAction(a, testee, acc);

    // Define a universe with some units
    TestUniverse h;

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileGlobalAction(testee.getActionByIndex(actionNr),
                                              h.session.world(),
                                              GlobalActions::Flags_t() + GlobalActions::ExcludeShips),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("01. getState", proc.getState(), Process::Ended);
    a.checkEqual("02. acc", acc,
                 "prepare()"
                 "exec(Planet(15),1)exec(Planet(23),1)exec(Planet(47),1)"
                 "result(1,{NUMSUCCESS:3,NUMERRORS:0,ERROR:Z(0)})");
}

/** Test handling of the ExcludePlanets flag. */
AFL_TEST("game.interface.GlobalActions:compileGlobalAction:ExcludePlanets", a)
{
    // Define one global action
    GlobalActions testee;
    String_t acc;
    size_t actionNr = createTestAction(a, testee, acc);

    // Define a universe with some units
    TestUniverse h;

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileGlobalAction(testee.getActionByIndex(actionNr),
                                              h.session.world(),
                                              GlobalActions::Flags_t() + GlobalActions::ExcludePlanets),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("01. getState", proc.getState(), Process::Ended);
    a.checkEqual("02. acc", acc,
                 "prepare()"
                 "exec(Ship(10),1)exec(Ship(20),1)"
                 "result(1,{NUMSUCCESS:2,NUMERRORS:0,ERROR:Z(0)})");
}

/** Test handling of the ExcludeUnmarkedObjects flag. */
AFL_TEST("game.interface.GlobalActions:compileGlobalAction:ExcludeUnmarkedObjects", a)
{
    // Define one global action
    GlobalActions testee;
    String_t acc;
    size_t actionNr = createTestAction(a, testee, acc);

    // Define a universe with some units
    TestUniverse h;
    Universe& univ = h.session.getGame()->currentTurn().universe();
    univ.ships().get(20)->setIsMarked(true);
    univ.planets().get(23)->setIsMarked(true);

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileGlobalAction(testee.getActionByIndex(actionNr),
                                              h.session.world(),
                                              GlobalActions::Flags_t() + GlobalActions::ExcludeUnmarkedObjects),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("01. getState", proc.getState(), Process::Ended);
    a.checkEqual("02. acc", acc,
                 "prepare()"
                 "exec(Ship(20),1)"
                 "exec(Planet(23),1)"
                 "result(1,{NUMSUCCESS:2,NUMERRORS:0,ERROR:Z(0)})");
}

/** Test handling of the ExcludeNumericFriendlyCodes flag. */
AFL_TEST("game.interface.GlobalActions:compileGlobalAction:ExcludeNumericFriendlyCodes", a)
{
    // Define one global action
    GlobalActions testee;
    String_t acc;
    size_t actionNr = createTestAction(a, testee, acc);

    // Define a universe with some units
    TestUniverse h;
    Universe& univ = h.session.getGame()->currentTurn().universe();
    univ.ships().get(10)->setFriendlyCode(String_t("abc"));
    univ.ships().get(20)->setFriendlyCode(String_t("123"));
    univ.planets().get(15)->setFriendlyCode(String_t("xyz"));
    univ.planets().get(23)->setFriendlyCode(String_t("777"));
    univ.planets().get(47)->setFriendlyCode(String_t("xyz"));

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileGlobalAction(testee.getActionByIndex(actionNr),
                                              h.session.world(),
                                              GlobalActions::Flags_t() + GlobalActions::ExcludeNumericFriendlyCodes),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("01. getState", proc.getState(), Process::Ended);
    a.checkEqual("02. acc", acc,
                 "prepare()"
                 "exec(Ship(10),1)"
                 "exec(Planet(15),1)exec(Planet(47),1)"
                 "result(1,{NUMSUCCESS:3,NUMERRORS:0,ERROR:Z(0)})");
}

/** Test handling of the ExcludeSpecialFriendlyCodes flag. */
AFL_TEST("game.interface.GlobalActions:compileGlobalAction:ExcludeSpecialFriendlyCodes", a)
{
    // Define one global action
    GlobalActions testee;
    String_t acc;
    size_t actionNr = createTestAction(a, testee, acc);

    // Define a universe with some units
    TestUniverse h;
    Universe& univ = h.session.getGame()->currentTurn().universe();
    univ.ships().get(10)->setFriendlyCode(String_t("abc"));
    univ.ships().get(20)->setFriendlyCode(String_t("123"));
    univ.planets().get(15)->setFriendlyCode(String_t("xyz"));
    univ.planets().get(23)->setFriendlyCode(String_t("777"));
    univ.planets().get(47)->setFriendlyCode(String_t("abc"));

    // Define a special friendly code
    h.session.getShipList()->friendlyCodes().addCode(game::spec::FriendlyCode("abc", ",", h.session.translator()));

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileGlobalAction(testee.getActionByIndex(actionNr),
                                              h.session.world(),
                                              GlobalActions::Flags_t() + GlobalActions::ExcludeSpecialFriendlyCodes),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("01. getState", proc.getState(), Process::Ended);
    a.checkEqual("02. acc", acc,
                 "prepare()"
                 "exec(Ship(20),1)"
                 "exec(Planet(15),1)exec(Planet(23),1)"
                 "result(1,{NUMSUCCESS:3,NUMERRORS:0,ERROR:Z(0)})");
}

/** Test iteration using a list. */
AFL_TEST("game.interface.GlobalActions:compileListAction", a)
{
    // Define one global action
    GlobalActions testee;
    String_t acc;
    size_t actionNr = createTestAction(a, testee, acc);

    // Define a universe with some units
    TestUniverse h;

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileListAction(testee.getActionByIndex(actionNr),
                                            createList(),
                                            h.session.world(),
                                            GlobalActions::Flags_t()),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("01. getState", proc.getState(), Process::Ended);
    a.checkEqual("02. acc", acc,
                 "prepare()"
                 "exec(Ship(20),1)"
                 "exec(Planet(47),1)"
                 "exec(Ship(10),1)"
                 "result(1,{NUMSUCCESS:3,NUMERRORS:0,ERROR:Z(0)})");
}

/** Test iteration using a list with ExcludeShips flag. */
AFL_TEST("game.interface.GlobalActions:compileListAction:ExcludeShips", a)
{
    // Define one global action
    GlobalActions testee;
    String_t acc;
    size_t actionNr = createTestAction(a, testee, acc);

    // Define a universe with some units
    TestUniverse h;

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileListAction(testee.getActionByIndex(actionNr),
                                            createList(),
                                            h.session.world(),
                                            GlobalActions::Flags_t() + GlobalActions::ExcludeShips),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("01. getState", proc.getState(), Process::Ended);
    a.checkEqual("02. acc", acc,
                 "prepare()"
                 "exec(Planet(47),1)"
                 "result(1,{NUMSUCCESS:1,NUMERRORS:0,ERROR:Z(0)})");
}

/** Test iteration using a list with ExcludePlanets flag. */
AFL_TEST("game.interface.GlobalActions:compileListAction:ExcludePlanets", a)
{
    // Define one global action
    GlobalActions testee;
    String_t acc;
    size_t actionNr = createTestAction(a, testee, acc);

    // Define a universe with some units
    TestUniverse h;

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileListAction(testee.getActionByIndex(actionNr),
                                            createList(),
                                            h.session.world(),
                                            GlobalActions::Flags_t() + GlobalActions::ExcludePlanets),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("01. getState", proc.getState(), Process::Ended);
    a.checkEqual("02. acc", acc,
                     "prepare()"
                     "exec(Ship(20),1)"
                     "exec(Ship(10),1)"
                     "result(1,{NUMSUCCESS:2,NUMERRORS:0,ERROR:Z(0)})");
}

/** Test cancelation (prepare returns null). */
AFL_TEST("game.interface.GlobalActions:cancel", a)
{
    GlobalActions testee;
    String_t acc;
    size_t actionNr = testee.addAction(Function("prepare", false, acc, 0).self(),
                                       Function("exec", true, acc, 0).self(),
                                       Function("result", true, acc, 0).self());
    a.checkNonNull("01. getActionByIndex", testee.getActionByIndex(actionNr));

    // Define a universe with some units
    TestUniverse h;

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileGlobalAction(testee.getActionByIndex(actionNr),
                                              h.session.world(),
                                              GlobalActions::Flags_t()),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("11. getState", proc.getState(), Process::Ended);
    a.checkEqual("12. acc", acc, "prepare()");
}

/** Test lock handling: normal case. */
AFL_TEST("game.interface.GlobalActions:locked", a)
{
    // Make an action that takes a lock
    GlobalActions testee;
    String_t acc;
    afl::data::IntegerValue one(1);
    size_t actionNr = testee.addAction(Function("prepare", false, acc, &one).self(),
                                       interpreter::SubroutineValue(makeTakeLockActionBCO(Function("exec", true, acc, 0).self())),
                                       Function("result", true, acc, 0).self());
    a.checkNonNull("01. getActionByIndex", testee.getActionByIndex(actionNr));

    // Define a universe with some units and take a lock
    TestUniverse h;
    Process& taker = h.session.processList().create(h.session.world(), "t");
    taker.pushFrame(makeTakeLockBCO("p23"), false);
    taker.run(0);

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileGlobalAction(testee.getActionByIndex(actionNr),
                                              h.session.world(),
                                              GlobalActions::Flags_t() + GlobalActions::ExcludeShips),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("11. getState", proc.getState(), Process::Ended);
    a.checkEqual("12. acc", acc, "prepare()exec(\"p15\")exec(\"p47\")result(1,{NUMSUCCESS:2,NUMERRORS:1,ERROR:\"Already locked\"})");
}

/** Test lock handling: overriding locks. */
AFL_TEST("game.interface.GlobalActions:locked:OverrideLocks", a)
{
    // Make an action that takes a lock
    GlobalActions testee;
    String_t acc;
    afl::data::IntegerValue one(1);
    size_t actionNr = testee.addAction(Function("prepare", false, acc, &one).self(),
                                       interpreter::SubroutineValue(makeTakeLockActionBCO(Function("exec", true, acc, 0).self())),
                                       Function("result", true, acc, 0).self());
    a.checkNonNull("01. getActionByIndex", testee.getActionByIndex(actionNr));

    // Define a universe with some units and take a lock
    TestUniverse h;
    Process& taker = h.session.processList().create(h.session.world(), "t");
    taker.pushFrame(makeTakeLockBCO("p23"), false);
    taker.run(0);

    // Create process
    Process& proc = h.session.processList().create(h.session.world(), "p");
    proc.pushFrame(testee.compileGlobalAction(testee.getActionByIndex(actionNr),
                                              h.session.world(),
                                              GlobalActions::Flags_t() + GlobalActions::ExcludeShips + GlobalActions::OverrideLocks),
                   false);

    // Run it
    proc.run(0);

    // Verify result
    a.checkEqual("11. getState", proc.getState(), Process::Ended);
    a.checkEqual("12. acc", acc, "prepare()exec(\"p15\")exec(\"p23\")exec(\"p47\")result(1,{NUMSUCCESS:3,NUMERRORS:0,ERROR:Z(0)})");
}
