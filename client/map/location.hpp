/**
  *  \file client/map/location.hpp
  */
#ifndef C2NG_CLIENT_MAP_LOCATION_HPP
#define C2NG_CLIENT_MAP_LOCATION_HPP

#include <memory>
#include "afl/base/signal.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/map/configuration.hpp"
#include "game/map/point.hpp"
#include "game/ref/userlist.hpp"
#include "game/reference.hpp"

namespace client { namespace map {

    /** Information about current starchart location.
        This implements state tracking for the current starchart location and coordinates game thread communication.
        It provides signals to inform possibly many subscribers about situation changes.
        It requires a listener to perform required callbacks.

        State variables:
        - current position (always valid once initialisation completed)
        - list of objects at current position (valid once list obtained)
        - currently-focused object at current position (valid once list obtained)

        Sequences:
        - Upon start, call setConfiguration(), setPosition(), setFocusedObject() with the initial values.
          Until that is done, this object is quasi-dead.
        - After requested by Listener::updateObjectList(), call setObjectList().
          That aside you can call setObjectList() any time you want to update.
        - You can call moveRelative() at any time.
        - To perform an absolute jump ("next planet", "lock to planet"), ask for permission first using startJump().
          If permission is granted, call setPosition().

        Position changes are reflected by sig_positionChange callbacks.
        Map display should exclusively honor sig_positionChange (resp. getPosition())
        which are internally validated by class Location. */
    class Location {
     public:
        /** Listener. */
        class Listener {
         public:
            virtual ~Listener()
                { }

            /** Request update of object list.
                Caller must eventually (or immediately) call setObjectList(). */
            virtual void updateObjectList() = 0;
        };

        /** Constructor.
            \param listener Listener
            \param log Logger */
        Location(Listener& listener, afl::sys::LogListener& log);

        /** Destructor. */
        ~Location();

        /** Get current location.
            Valid after the first setPosition() call.
            \return location */
        game::map::Point getPosition() const;

        /** Get number of objects on current object list.
            \return Number of objects. 0 if object list currently not valid. */
        size_t getNumObjects() const;

        /** Get index of current object on current object list.
            \return Index [0, getNumObjects()) */
        size_t getCurrentObjectIndex() const;

        /** Get object by index.
            Equivalent to game::ref::UserList::get().
            \param i Index [0, getNumObjects())
            \return Pointer to item. 0 if index out of range or list currently not valid */
        const game::ref::UserList::Item* getObjectByIndex(size_t i) const;

        /** Set configuration.
            This is used to verify movements.
            \param config Configuration */
        void setConfiguration(const game::map::Configuration& config);

        /** Set position.
            Call after startup, after startJump(), or after other user-initiated movements.
            Prefer moveRelative() for user-initiated movement if possible.
            \param pt Target position */
        void setPosition(game::map::Point pt);

        /** Set object list.
            Call after requested by listener, or when it changes.
            \param list Liste */
        void setObjectList(const game::ref::UserList& list);

        /** Set focused object.
            Can be called at any time.
            Note that movement will invalidate the focused object, so call this after setPosition()/moveRelative().
            If the given object is not on the current position (=in the object list), the call will be ignored.
            \param ref Object */
        void setFocusedObject(game::Reference ref);

        /** Cycle through focused objects.
            Ignored if we don't currently have a focused object.
            \param forward true to cycle forward, false to cycle backward
            \param markedOnly true to accept only marked units */
        void cycleFocusedObject(bool forward, bool markedOnly);

        /** Get focused object.
            Can be called at any time.
            May return unvalidated objects.
            \return object (same as in last setFocusedObject()) */
        game::Reference getFocusedObject() const;

        /** Move relative.
            Relative movement can be queued if it cannot be executed immediately.
            \param dx X movement (west-east)
            \param dy Y movement (south-north) */
        void moveRelative(int dx, int dy);

        /** Ask permission to jump.
            \retval true Jump permitted; eventually call setPosition
            \retval false Jump not possible because another jump is still active */
        bool startJump();

        /** Access map configuration.
            \return handle to configuration */
        const game::map::Configuration& configuration() const;

        /** Signal: change of position. */
        afl::base::Signal<void(game::map::Point)> sig_positionChange;

        /** Signal: change of focused object. */
        afl::base::Signal<void(game::Reference)> sig_objectChange;

     private:
        class State;
        class InitState;
        class BuildState;
        class BuildAgainState;
        class BuildJumpState;
        class JumpState;
        class IdleState;

        Listener& m_listener;                   // Listener
        afl::sys::LogListener& m_log;           // Logger
        std::auto_ptr<State> m_pState;          // Current state. Never null.

        game::map::Point m_cursorPosition;      // Cursor position. Valid after first setPosition (=in all states but InitState)
        game::Reference m_focusedObject;        // Focused object. Part of object list or unset in IdleState, otherwise arbitrary
        game::ref::UserList m_objectList;       // Object list. Valid in IdleState.
        game::map::Configuration m_config;      // Configuration. Not part of state machine; assumed to be present when needed.

        void verifyFocusedObject();

        void setStateNew(State* pNewState);

        void setBuildState();
        void setBuildAgainState();
        void setBuildJumpState();
        void setJumpState(game::map::Point pt);
        void setIdleState();
    };

} }

#endif
