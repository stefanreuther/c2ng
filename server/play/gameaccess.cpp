/**
  *  \file server/play/gameaccess.cpp
  *  \brief Class server::play::GameAccess
  */

#include <stdexcept>
#include "server/play/gameaccess.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "server/errors.hpp"
#include "server/play/basichullfunctionpacker.hpp"
#include "server/play/beampacker.hpp"
#include "server/play/commandhandler.hpp"
#include "server/play/configurationpacker.hpp"
#include "server/play/enginepacker.hpp"
#include "server/play/flakconfigurationpacker.hpp"
#include "server/play/friendlycodepacker.hpp"
#include "server/play/hullpacker.hpp"
#include "server/play/imperialstatspacker.hpp"
#include "server/play/ionstormpacker.hpp"
#include "server/play/maincommandhandler.hpp"
#include "server/play/mainpacker.hpp"
#include "server/play/messagepacker.hpp"
#include "server/play/minefieldpacker.hpp"
#include "server/play/outmessagecommandhandler.hpp"
#include "server/play/outmessageindexpacker.hpp"
#include "server/play/outmessagepacker.hpp"
#include "server/play/packerlist.hpp"
#include "server/play/planetcommandhandler.hpp"
#include "server/play/planetfriendlycodepacker.hpp"
#include "server/play/planetpacker.hpp"
#include "server/play/planetxypacker.hpp"
#include "server/play/playerpacker.hpp"
#include "server/play/racenamepacker.hpp"
#include "server/play/shipcommandhandler.hpp"
#include "server/play/shipfriendlycodepacker.hpp"
#include "server/play/shipmissionpacker.hpp"
#include "server/play/shippacker.hpp"
#include "server/play/shipxypacker.hpp"
#include "server/play/torpedopacker.hpp"
#include "server/play/truehullpacker.hpp"
#include "server/play/ufopacker.hpp"
#include "server/play/vcrpacker.hpp"
#include "util/stringparser.hpp"

using game::actions::mustHaveRoot;
using game::actions::mustHaveShipList;

server::play::GameAccess::GameAccess(game::Session& session, util::MessageCollector& console)
    : m_session(session),
      m_console(console),
      m_lastMessage(0)
{ }

void
server::play::GameAccess::save()
{
    // Save status tracking
    bool result = false;

    // Create action
    std::auto_ptr<afl::base::Closure<void()> > action = m_session.save(game::TurnLoader::SaveOptions_t(), game::makeResultTask(result));

    // Invoke action if any (should always exist)
    if (action.get() != 0) {
        action->call();
    }

    // Discard action.
    // If this action pended some background task, this will kill it.
    // For now, we know that there cannot be such a background task,
    // because we don't have a RequestDispatcher that could be used to revive it.
    action.reset();

    // Error reporting
    if (!result) {
        throw std::runtime_error("Unable to save");
    }
}

String_t
server::play::GameAccess::getStatus()
{
    String_t result;
    afl::sys::LogListener::Message msg;
    while (m_console.readNewerMessage(m_lastMessage, &msg, m_lastMessage)) {
        result += afl::string::Format("%s [%s] %s\n",
                                      msg.m_time.toString(afl::sys::Time::LocalTime, afl::sys::Time::TimeFormat),
                                      msg.m_channel,
                                      msg.m_message);
    }
    return result;
}

server::Value_t*
server::play::GameAccess::get(String_t objName)
{
    util::StringParser p(objName);
    if (p.parseString("obj/")) {
        return getObject(p);
    } else if (p.parseString("query/")) {
        return getQuery(p);
    } else {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }
}

server::Value_t*
server::play::GameAccess::post(String_t objName, const Value_t* value)
{
    // ex doPostObject
    // Determine command handler
    util::StringParser p(objName);
    std::auto_ptr<CommandHandler> hdl(createCommandHandler(p, m_session));
    if (hdl.get() == 0 || !p.parseEnd()) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    // Posted value must be an array of commands.
    // @change This now generates 400 Invalid value, not 412 Type error.
    const afl::data::VectorValue* a = dynamic_cast<const afl::data::VectorValue*>(value);
    if (a == 0) {
        throw std::runtime_error(INVALID_VALUE);
    }
    const afl::data::Vector& vec = *a->getValue();

    // Process individual commands.
    PackerList result;
    for (size_t i = 0, n = vec.size(); i < n; ++i) {
        // Command must be an array.
        const afl::data::VectorValue* cmdValue = dynamic_cast<const afl::data::VectorValue*>(vec[i]);
        if (cmdValue == 0) {
            throw std::runtime_error(INVALID_VALUE);
        }
        const afl::data::Vector& cmdVector = *cmdValue->getValue();
        interpreter::Arguments args(cmdVector, 0, cmdVector.size());

        // Fetch command verb
        String_t verb;
        if (!interpreter::checkStringArg(verb, args.getNext())) {
            throw std::runtime_error(INVALID_VALUE);
        }

        // Do it
        // FIXME: PCC2 wraps this in a try/catch and generates '450 Failed'
        hdl->processCommand(verb, args, result);
    }

    // Generate output
    return result.buildValue();
}

server::Value_t*
server::play::GameAccess::getObject(util::StringParser& p)
{
    // ex server/getobj.cc:performGetObject
    // Collect objects
    PackerList packers;
    bool fail = false;

    while (1) {
        // Parse one element
        std::auto_ptr<Packer> thisPacker(createPacker(p));
        if (thisPacker.get() == 0) {
            fail = true;
            break;
        }
        packers.addNew(thisPacker.release());

        // Next element?
        if (!p.parseCharacter(',')) {
            break;
        }
    }

    if (fail || !p.parseEnd()) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    // Generate output
    return packers.buildValue();
}

server::Value_t*
server::play::GameAccess::getQuery(util::StringParser& p)
{
    // Build the packer
    std::auto_ptr<Packer> thisPacker(createQueryPacker(p, m_session));
    if (thisPacker.get() == 0 || !p.parseEnd()) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    // Container for result. We only produce one result, but PackerList conveniently formats it.
    PackerList packers;
    packers.addNew(thisPacker.release());
    return packers.buildValue();
}

server::play::Packer*
server::play::GameAccess::createPacker(util::StringParser& p)
{
    // ex server/getobj.cc:createWriter
    game::Session& session = m_session;
    int n;
    if (p.parseString("shipxy")) {
        return new ShipXYPacker(session);
    } else if (p.parseString("planetxy")) {
        return new PlanetXYPacker(session);
    } else if (p.parseString("main")) {
        return new MainPacker(session);
    } else if (p.parseString("player")) {
        return new PlayerPacker(session);
    } else if (p.parseString("racename")) {
        return new RaceNamePacker(mustHaveRoot(session), 0, session.translator());
    } else if (p.parseString("torp")) {
        return new TorpedoPacker(mustHaveShipList(session), mustHaveRoot(session), 0);
    } else if (p.parseString("beam")) {
        return new BeamPacker(mustHaveShipList(session), mustHaveRoot(session), 0);
    } else if (p.parseString("engine")) {
        return new EnginePacker(mustHaveShipList(session), 0);
    } else if (p.parseString("zstorm")) {
        return new IonStormPacker(session);
    } else if (p.parseString("zmine")) {
        return new MinefieldPacker(session);
    } else if (p.parseString("zufo")) {
        return new UfoPacker(session);
    } else if (p.parseString("truehull")) {
        return new TruehullPacker(mustHaveShipList(session), mustHaveRoot(session), 0);
    } else if (p.parseString("zvcr")) {
        return new VcrPacker(session);
    } else if (p.parseString("zab")) {
        return new BasicHullFunctionPacker(mustHaveShipList(session));
    } else if (p.parseString("fcode")) {
        return new FriendlyCodePacker(mustHaveShipList(session), mustHaveRoot(session), session.translator());
    } else if (p.parseString("outidx")) {
        return new OutMessageIndexPacker(session);
    } else if (p.parseString("hull") && p.parseInt(n)) {
        return new HullPacker(mustHaveShipList(session), mustHaveRoot(session), n);
    } else if (p.parseString("ship") && p.parseInt(n)) {
        return new ShipPacker(session, n);
    } else if (p.parseString("planet") && p.parseInt(n)) {
        return new PlanetPacker(session, n);
    } else if (p.parseString("msg") && p.parseInt(n)) {
        return new MessagePacker(session, n);
    } else if (p.parseString("outmsg") && p.parseInt(n)) {
        return new OutMessagePacker(session, n);
    } else if (p.parseString("cfg") && p.parseInt(n)) {
        return new ConfigurationPacker(mustHaveRoot(session), n);
    } else if (p.parseString("flakconfig")) {
        return new FlakConfigurationPacker(mustHaveRoot(session));
    } else {
        return 0;
    }
}

server::play::Packer*
server::play::GameAccess::createQueryPacker(util::StringParser& p, game::Session& session)
{
    // ex createQueryWriter
    int n, m;
    if (p.parseString("shipfc") && p.parseInt(n)) {
        return new ShipFriendlyCodePacker(session, n);
    } else if (p.parseString("planetfc") && p.parseInt(n)) {
        return new PlanetFriendlyCodePacker(session, n);
    } else if (p.parseString("shipmsn") && p.parseInt(n)) {
        return new ShipMissionPacker(session, n);
    } else if (p.parseString("istat") && p.parseInt(n) && p.parseString(".") && p.parseInt(m)) {
        return new ImperialStatsPacker(session, n, m);
    } else {
        return 0;
    }
}

server::play::CommandHandler*
server::play::GameAccess::createCommandHandler(util::StringParser& p, game::Session& session)
{
    // ex createCommandHandler
    int n;
    if (p.parseString("obj/ship") && p.parseInt(n)) {
        return new ShipCommandHandler(session, n);
    } else if (p.parseString("obj/planet") && p.parseInt(n)) {
        return new PlanetCommandHandler(session, n);
    } else if (p.parseString("obj/main")) {
        return new MainCommandHandler(session);
    } else if (p.parseString("obj/outmsg") && p.parseInt(n)) {
        return new OutMessageCommandHandler(session, n);
    } else {
        return 0;
    }
}
