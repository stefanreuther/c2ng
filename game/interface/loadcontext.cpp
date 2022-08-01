/**
  *  \file game/interface/loadcontext.cpp
  */

#include "game/interface/loadcontext.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "game/map/universe.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/interface/beamcontext.hpp"
#include "game/interface/enginecontext.hpp"
#include "game/interface/globalcontext.hpp"
#include "game/interface/hullcontext.hpp"
#include "game/interface/ionstormcontext.hpp"
#include "game/interface/minefieldcontext.hpp"
#include "game/interface/playercontext.hpp"
#include "game/interface/torpedocontext.hpp"
#include "game/interface/iteratorcontext.hpp"
#include "interpreter/mutexlist.hpp"
#include "interpreter/mutexcontext.hpp"

game::interface::LoadContext::LoadContext(Session& session)
    : m_session(session)
{ }

game::interface::LoadContext::~LoadContext()
{ }

afl::data::Value*
game::interface::LoadContext::loadBCO(uint32_t /*id*/)
{
    return 0;
}

afl::data::Value*
game::interface::LoadContext::loadArray(uint32_t /*id*/)
{
    return 0;
}

afl::data::Value*
game::interface::LoadContext::loadHash(uint32_t /*id*/)
{
    return 0;
}

afl::data::Value*
game::interface::LoadContext::loadStructureValue(uint32_t /*id*/)
{
    return 0;
}

afl::data::Value*
game::interface::LoadContext::loadStructureType(uint32_t /*id*/)
{
    return 0;
}

interpreter::Context*
game::interface::LoadContext::loadContext(const interpreter::TagNode& tag, afl::io::Stream& /*aux*/)
{
    // ex int/contextio.h:loadContext
    using interpreter::TagNode;
    switch (tag.tag) {
     case TagNode::Tag_Ship:
        return ShipContext::create(tag.value, m_session);

     case TagNode::Tag_Planet:
        return PlanetContext::create(tag.value, m_session);

     case TagNode::Tag_Minefield:
        return MinefieldContext::create(tag.value, m_session, true);

     case TagNode::Tag_Ion:
        return IonStormContext::create(tag.value, m_session);

     case TagNode::Tag_Hull:
        return HullContext::create(tag.value, m_session);

     case TagNode::Tag_Engine:
        return EngineContext::create(tag.value, m_session);

     case TagNode::Tag_Beam:
        return BeamContext::create(tag.value, m_session);

     case TagNode::Tag_Torpedo:
        return TorpedoContext::create(false, tag.value, m_session);

     case TagNode::Tag_Launcher:
        return TorpedoContext::create(true, tag.value, m_session);

     case TagNode::Tag_Global:
        return new GlobalContext(m_session);

     case TagNode::Tag_Iterator:
        return makeIteratorValue(m_session, tag.value);

     case TagNode::Tag_Player:
        return PlayerContext::create(tag.value, m_session);

     default:
        return 0;
    }
}

interpreter::Context*
game::interface::LoadContext::loadMutex(const String_t& /*name*/, const String_t& /*note*/)
{
    return 0;
}

interpreter::Process*
game::interface::LoadContext::createProcess()
{
    return 0;
}

void
game::interface::LoadContext::finishProcess(interpreter::Process& /*proc*/)
{ }
