/**
  *  \file client/map/location.cpp
  *  \brief Class client::map::Location
  */

#include "client/map/location.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"

const char*const LOG_NAME = "client.map.location";

using game::map::Point;

/*
 *  Internal State
 *
 *  This implements the 'State' pattern.
 *
 *  The state machine we implement:
 *  - on every position change, requests an object list (BuildState),
 *    making sure that only one such request is active at a time.
 *  - unless a jump is already active, permits executing a jump (JumpState);
 *    a jump and a potential build can complete in any order.
 *  - a lock operation can be requested at any time (LockState);
 *    if it cannot be executed right now, it will be deferred.
 */

class client::map::Location::State : public afl::base::Deletable {
 public:
    virtual void onPositionChange(Location& parent, bool change) = 0;
    virtual void onObjectList(Location& parent) = 0;
    virtual void moveRelative(Location& parent, Point pt) = 0;
    virtual bool startJump(Location& parent) = 0;
    virtual void lockObject(Location& parent, Flags_t flags) = 0;
    virtual bool hasFocusedObject() = 0;
    virtual const char* getName() const = 0;
};

/*
 *  Init: wait for reception of onPositionChange(), ignore everything else
 */
class client::map::Location::InitState : public State {
 public:
    virtual void onPositionChange(Location& parent, bool /*change*/)
        { parent.setBuildState(); }
    virtual void onObjectList(Location& /*parent*/)
        { }
    virtual void moveRelative(Location& /*parent*/, Point /*pt*/)
        { }
    virtual bool startJump(Location& /*parent*/)
        { return false; }
    virtual void lockObject(Location& /*parent*/, Flags_t /*flags*/)
        { }
    virtual bool hasFocusedObject()
        { return false; }
    virtual const char* getName() const
        { return "Init"; }
};

/*
 *  Build: wait for reception of onObjectList() that we requested on entry.
 *  If the position changes, go to BuildAgain because the object list we'll get will be out-of-date.
 */
class client::map::Location::BuildState : public State {
 public:
    virtual void onPositionChange(Location& parent, bool change)
        {
            if (change) {
                parent.setBuildAgainState();
            }
        }
    virtual void onObjectList(Location& parent)
        { parent.setIdleState(); }
    virtual void moveRelative(Location& parent, Point pt)
        { parent.setPosition(parent.m_cursorPosition + pt); }
    virtual bool startJump(Location& parent)
        { parent.setBuildJumpState(); return true; }
    virtual void lockObject(Location& parent, Flags_t flags)
        { parent.setBuildLockState(Point(), flags); }
    virtual bool hasFocusedObject()
        { return false; }
    virtual const char* getName() const
        { return "Build"; }
};

/*
 *  BuildAgain: we still are waiting for onObjectList() but know that we don't want the result.
 *  Wait for it, then request the "real" one.
 */
class client::map::Location::BuildAgainState : public State {
 public:
    virtual void onPositionChange(Location& /*parent*/, bool /*change*/)
        { }
    virtual void onObjectList(Location& parent)
        { parent.setBuildState(); }
    virtual void moveRelative(Location& parent, Point pt)
        { parent.setPosition(parent.m_cursorPosition + pt); }
    virtual bool startJump(Location& parent)
        { parent.setBuildJumpState(); return true; }
    virtual void lockObject(Location& parent, Flags_t flags)
        { parent.setBuildLockState(Point(), flags); }
    virtual bool hasFocusedObject()
        { return false; }
    virtual const char* getName() const
        { return "BuildAgain"; }
};

/*
 *  BuildJump: wait for reception of onObjectList() -or- completion of a jump.
 *  Proceed with correct following state (BuildAgain, Jump).
 */
class client::map::Location::BuildJumpState : public State {
 public:
    virtual void onPositionChange(Location& parent, bool /*change*/)
        {
            // Jump completed, but object list did not.
            parent.setBuildAgainState();
        }
    virtual void onObjectList(Location& parent)
        {
            // Object list completed, but jump did not.
            parent.setJumpState(m_postJumpMove);
        }
    virtual void moveRelative(Location& /*parent*/, Point pt)
        { m_postJumpMove += pt; }
    virtual bool startJump(Location& /*parent*/)
        { return false; }
    virtual void lockObject(Location& parent, Flags_t flags)
        { parent.setBuildJumpLockState(m_postJumpMove, flags); }
    virtual bool hasFocusedObject()
        { return false; }
    virtual const char* getName() const
        { return "BuildJump"; }
 private:
    Point m_postJumpMove;
};

/*
 *  BuildJumpLockState: wait for reception of onObjectList() -or- completion of a jump.
 *  Proceed with correct following state (BuildLockState, JumpLockState) to schedule a lock.
 */
class client::map::Location::BuildJumpLockState : public State {
 public:
    BuildJumpLockState(Point pt, Flags_t flags)
        : m_postJumpMove(pt), m_flags(flags)
        { }
    virtual void onPositionChange(Location& parent, bool /*change*/)
        { parent.setBuildLockState(m_postJumpMove, m_flags); }
    virtual void onObjectList(Location& parent)
        { parent.setJumpLockState(m_postJumpMove, m_flags); }
    virtual void moveRelative(Location& /*parent*/, Point pt)
        { m_postJumpMove += pt; }
    virtual bool startJump(Location& /*parent*/)
        { return false; }
    virtual void lockObject(Location& /*parent*/, Flags_t flags)
        { m_flags = flags; }
    virtual bool hasFocusedObject()
        { return false; }
    virtual const char* getName() const
        { return "BuildJumpLock"; }
 private:
    Point m_postJumpMove;
    Flags_t m_flags;
};

/*
 *  BuildLockState: wait for reception of object list, then lock.
 */
class client::map::Location::BuildLockState : public State {
 public:
    BuildLockState(Point pt, Flags_t flags)
        : m_postJumpMove(pt), m_flags(flags)
        { }
    virtual void onPositionChange(Location& /*parent*/, bool /*change*/)
        { }
    virtual void onObjectList(Location& parent)
        { parent.setLockState(m_postJumpMove, m_flags); }
    virtual void moveRelative(Location& /*parent*/, Point pt)
        { m_postJumpMove += pt; }
    virtual bool startJump(Location& /*parent*/)
        { return false; }
    virtual void lockObject(Location& /*parent*/, Flags_t flags)
        { m_flags = flags; }
    virtual bool hasFocusedObject()
        { return false; }
    virtual const char* getName() const
        { return "BuildLock"; }
 private:
    Point m_postJumpMove;
    Flags_t m_flags;
};

/*
 *  Jump: we are performing a jump to a still-unknown location (e.g. paging).
 *  Wait for new location using onPositionChange(). In the meantime, gather relative movement,
 *  so that "page-then-move" works.
 */
class client::map::Location::JumpState : public State {
 public:
    JumpState(Point pt)
        : m_postJumpMove(pt)
        { }
    virtual void onPositionChange(Location& parent, bool /*change*/)
        {
            parent.m_cursorPosition += m_postJumpMove;
            parent.setBuildState();
        }
    virtual void onObjectList(Location& /*parent*/)
        { }
    virtual void moveRelative(Location& /*parent*/, Point pt)
        { m_postJumpMove += pt; }
    virtual bool startJump(Location& /*parent*/)
        { return false; }
    virtual void lockObject(Location& parent, Flags_t flags)
        { parent.setJumpLockState(m_postJumpMove, flags); }
    virtual bool hasFocusedObject()
        { return false; }
    virtual const char* getName() const
        { return "Jump"; }
 private:
    Point m_postJumpMove;
};

/*
 *  JumpLockState: we are jumping, but user already requested to lock.
 */
class client::map::Location::JumpLockState : public State {
 public:
    JumpLockState(Point pt, Flags_t flags)
        : m_postJumpMove(pt), m_flags(flags)
        { }
    virtual void onPositionChange(Location& parent, bool /*change*/)
        { parent.setLockState(m_postJumpMove, m_flags); }
    virtual void onObjectList(Location& /*parent*/)
        { }
    virtual void moveRelative(Location& /*parent*/, Point pt)
        { m_postJumpMove += pt; }
    virtual bool startJump(Location& /*parent*/)
        { return false; }
    virtual void lockObject(Location& /*parent*/, Flags_t flags)
        { m_flags = flags; }
    virtual bool hasFocusedObject()
        { return false; }
    virtual const char* getName() const
        { return "JumpLock"; }
 private:
    Point m_postJumpMove;
    Flags_t m_flags;
};

/*
 *  LockState: we are asking for a lock onto an object.
 *  Wait for new position.
 */
class client::map::Location::LockState : public State {
 public:
    LockState(Point pt)
        : m_postJumpMove(pt)
        { }
    virtual void onPositionChange(Location& parent, bool /*change*/)
        {
            // For now, ignore movement after lock, assuming that it is mouse jitter.
            // Ignoring this movement makes the "hold mouse button and move mouse" usecase look somewhat acceptable.
            // parent.m_cursorPosition += m_postJumpMove;
            parent.setBuildState();
        }
    virtual void onObjectList(Location& /*parent*/)
        { }
    virtual void moveRelative(Location& /*parent*/, Point pt)
        { m_postJumpMove += pt; }
    virtual bool startJump(Location& /*parent*/)
        { return false; }
    virtual void lockObject(Location& parent, Flags_t flags)
        { parent.setLockAgainState(m_postJumpMove, flags); }
    virtual bool hasFocusedObject()
        { return false; }
    virtual const char* getName() const
        { return "Lock"; }
 private:
    Point m_postJumpMove;
};

/*
 *  LockAgainState: we are locking on an object, but another lock request already came in.
 *  Wait until it completes, then submit it.
 */
class client::map::Location::LockAgainState : public State {
 public:
    LockAgainState(Point pt, Flags_t flags)
        : m_postJumpMove(pt), m_flags(flags)
        { }
    virtual void onPositionChange(Location& parent, bool /*change*/)
        { parent.setLockState(m_postJumpMove, m_flags); }
    virtual void onObjectList(Location& /*parent*/)
        { }
    virtual void moveRelative(Location& /*parent*/, Point pt)
        { m_postJumpMove += pt; }
    virtual bool startJump(Location& /*parent*/)
        { return false; }
    virtual void lockObject(Location& /*parent*/, Flags_t flags)
        { m_flags = flags; }
    virtual bool hasFocusedObject()
        { return false; }
    virtual const char* getName() const
        { return "LockAgain"; }
 private:
    Point m_postJumpMove;
    Flags_t m_flags;
};

/*
 *  Idle: regular state in which all information is current.
 *  This is the only state in which we report a focused object.
 */
class client::map::Location::IdleState : public State {
 public:
    virtual void onPositionChange(Location& parent, bool change)
        {
            if (change) {
                parent.setBuildState();
            }
        }
    virtual void onObjectList(Location& parent)
        { parent.verifyFocusedObject(); }
    virtual void moveRelative(Location& parent, Point pt)
        { parent.setPosition(parent.m_cursorPosition + pt); }
    virtual bool startJump(Location& parent)
        {
            parent.setJumpState(Point());
            return true;
        }
    virtual void lockObject(Location& parent, Flags_t flags)
        { parent.setLockState(Point(), flags); }
    virtual bool hasFocusedObject()
        { return true; }
    virtual const char* getName() const
        { return "Idle"; }
};


/******************************** Location *******************************/


client::map::Location::Location(Listener& listener, afl::sys::LogListener& log)
    : m_listener(listener),
      m_log(log),
      m_pState(new InitState()),
      m_cursorPosition(),
      m_focusedObject(),
      m_objectList(),
      m_config()
{ }

client::map::Location::~Location()
{ }

game::map::Point
client::map::Location::getPosition() const
{
    return m_cursorPosition;
}

size_t
client::map::Location::getNumObjects() const
{
    if (m_pState->hasFocusedObject()) {
        return m_objectList.size();
    } else {
        return 0;
    }
}

size_t
client::map::Location::getCurrentObjectIndex() const
{
    size_t n = 0;
    if (m_pState->hasFocusedObject() && m_objectList.find(m_focusedObject, n)) {
        return n;
    } else {
        return 0;
    }
}

const game::ref::UserList::Item*
client::map::Location::getObjectByIndex(size_t i) const
{
    if (m_pState->hasFocusedObject()) {
        return m_objectList.get(i);
    } else {
        return 0;
    }
}

void
client::map::Location::setConfiguration(const game::map::Configuration& config)
{
    m_config = config;
}

void
client::map::Location::setPosition(game::map::Point pt)
{
    // ex GChartLocation::setLocation, GChartLocation::updateLocation (sort-of)
    // Take over new position
    bool change = (pt != m_cursorPosition);
    if (change) {
        m_focusedObject = game::Reference();
        m_objectList.clear();
        m_cursorPosition = pt;
    }

    // State transition
    m_log.write(afl::sys::Log::Trace, LOG_NAME, afl::string::Format("Trigger: setPosition(%s,%s)", m_cursorPosition.toString(), change ? "true" : "false"));
    m_pState->onPositionChange(*this, change);

    // Report change
    if (change) {
        sig_positionChange.raise(m_cursorPosition);
    }
}

void
client::map::Location::setObjectList(const game::ref::UserList& list)
{
    m_log.write(afl::sys::Log::Trace, LOG_NAME, afl::string::Format("Trigger: setObjectList(%d entr%1{y%|ies%})", list.size()));
    m_objectList = list;
    m_pState->onObjectList(*this);
}

void
client::map::Location::setFocusedObject(game::Reference ref)
{
    m_focusedObject = ref;
    if (m_pState->hasFocusedObject()) {
        verifyFocusedObject();
    }
}

void
client::map::Location::cycleFocusedObject(bool forward, bool markedOnly)
{
    // ex GChartLocation::doScroll
    using game::ref::UserList;

    // Implementing this using the public methods means that those methods to the state check and we don't have to.
    size_t pos = getCurrentObjectIndex();
    size_t limit = getNumObjects();
    for (size_t i = 0; i < limit; ++i) {
        // Advance cursor
        if (forward) {
            ++pos;
            if (pos >= limit) {
                pos = 0;
            }
        } else {
            if (pos == 0) {
                pos = limit;
            }
            --pos;
        }

        // Check whether item is acceptable
        const UserList::Item* p = getObjectByIndex(pos);
        if (p != 0 && p->type == UserList::ReferenceItem && (p->marked || !markedOnly)) {
            m_focusedObject = p->reference;
            sig_objectChange.raise(m_focusedObject);
            break;
        }
    }
}

game::Reference
client::map::Location::getFocusedObject() const
{
    // ex GChartLocation::getCurrentObject, (GChartLocation::getCurrentShipId)
    return m_focusedObject;
}

void
client::map::Location::moveRelative(int dx, int dy)
{
    m_pState->moveRelative(*this, Point(dx, dy));
}

void
client::map::Location::lockObject(Flags_t flags)
{
    // FIXME: this means the locked object will flicker if users repeatedly press Enter; that does not happen in PCC2.
    // We could avoid that by pre-validating the object list whether it already matches our desired object.
    m_pState->lockObject(*this, flags);
}

bool
client::map::Location::startJump()
{
    return m_pState->startJump(*this);
}

const game::map::Configuration&
client::map::Location::configuration() const
{
    return m_config;
}

void
client::map::Location::verifyFocusedObject()
{
    // Cancel focused object if it is not in the list
    if (m_focusedObject.isSet()) {
        size_t pos;
        if (!m_objectList.find(m_focusedObject, pos)) {
            afl::string::NullTranslator tx;
            m_log.write(afl::sys::Log::Trace, LOG_NAME, afl::string::Format("Unit not on current place: %s", m_focusedObject.toString(tx)));
            m_focusedObject = game::Reference();
        }
    }

    // If we have a list, we should have a focused object; focus on first possible.
    if (!m_focusedObject.isSet()) {
        size_t pos = 0;
        size_t limit = m_objectList.size();
        while (pos < limit) {
            if (const game::ref::UserList::Item* pItem = m_objectList.get(pos)) {
                if (pItem->type == game::ref::UserList::ReferenceItem) {
                    m_focusedObject = pItem->reference;
                    break;
                }
            }
            ++pos;
        }
    }

    // Notify user
    sig_objectChange.raise(m_focusedObject);
}

void
client::map::Location::setStateNew(State* pNewState)
{
    std::auto_ptr<State> p(pNewState);
    m_log.write(afl::sys::Log::Trace, LOG_NAME, afl::string::Format("State change: %s -> %s", m_pState->getName(), p->getName()));
    m_pState = p;
}

void
client::map::Location::setBuildState()
{
    setStateNew(new BuildState());
    m_listener.requestObjectList(getPosition());
    sig_objectChange.raise(game::Reference());
}

void
client::map::Location::setBuildAgainState()
{
    setStateNew(new BuildAgainState());
}

void
client::map::Location::setBuildJumpState()
{
    setStateNew(new BuildJumpState());
}

void
client::map::Location::setBuildJumpLockState(game::map::Point pt, Flags_t flags)
{
    setStateNew(new BuildJumpLockState(pt, flags));
}

void
client::map::Location::setBuildLockState(game::map::Point pt, Flags_t flags)
{
    setStateNew(new BuildLockState(pt, flags));
}

void
client::map::Location::setJumpState(game::map::Point pt)
{
    setStateNew(new JumpState(pt));
    sig_objectChange.raise(game::Reference());
}

void
client::map::Location::setJumpLockState(game::map::Point pt, Flags_t flags)
{
    setStateNew(new JumpLockState(pt, flags));
}

void
client::map::Location::setLockState(game::map::Point pt, Flags_t flags)
{
    setStateNew(new LockState(pt));
    m_listener.requestLockObject(getPosition() + pt, flags);
}

void
client::map::Location::setLockAgainState(game::map::Point pt, Flags_t flags)
{
    setStateNew(new LockAgainState(pt, flags));
}

void
client::map::Location::setIdleState()
{
    setStateNew(new IdleState());
    verifyFocusedObject();
}
