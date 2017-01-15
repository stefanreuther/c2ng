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

// // /** Load context value from stream.
// //     \param sv Tag node read from header
// //     \param aux Auxiliary data can be read here */
interpreter::Context*
game::interface::LoadContext::loadContext(const interpreter::TagNode& tag, afl::io::Stream& /*aux*/)
{
    // ex int/contextio.h:loadContext
    using interpreter::TagNode;
    switch (tag.tag) {
     case TagNode::Tag_Ship:
        // Ship. Check range.
        return ShipContext::create(tag.value, m_session);

     case TagNode::Tag_Planet:
        // Planet. Check range.
        return PlanetContext::create(tag.value, m_session);

     case TagNode::Tag_Minefield:
        // We currently have no constraints on minefield Ids
        return MinefieldContext::create(tag.value, m_session, true);

     case TagNode::Tag_Ion:
        // Ion storm. Check range.
        return IonStormContext::create(tag.value, m_session);

     case TagNode::Tag_Hull:
        // Hull. Check range.
        return HullContext::create(tag.value, m_session);

     case TagNode::Tag_Engine:
        // Engine. Check range.
        return EngineContext::create(tag.value, m_session);

     case TagNode::Tag_Beam:
        // Beam. Check range.
        return BeamContext::create(tag.value, m_session);

     case TagNode::Tag_Torpedo:
        // Torpedo. Check range.
        return TorpedoContext::create(false, tag.value, m_session);

     case TagNode::Tag_Launcher:
        // Torpedo launcher. Check range.
        return TorpedoContext::create(true, tag.value, m_session);

     case TagNode::Tag_Global:
        // Global context; unique.
        return new GlobalContext(m_session);

     case TagNode::Tag_Iterator:
        // Iterator.
        return makeIteratorValue(m_session.getGame(), tag.value, false);

     case TagNode::Tag_Player:
        // Player. Check range.
        return PlayerContext::create(tag.value, m_session);

     default:
        return 0;
    }
}

interpreter::Context*
game::interface::LoadContext::loadMutex(const String_t& /*name*/, const String_t& /*note*/, interpreter::Process* /*owner*/)
{
    return 0;
}

interpreter::Process*
game::interface::LoadContext::createProcess()
{
    return 0;
}
