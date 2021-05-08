/**
  *  \file game/interface/userinterfacepropertystack.hpp
  *  \brief Class game::interface::UserInterfacePropertyStack
  */
#ifndef C2NG_GAME_INTERFACE_USERINTERFACEPROPERTYSTACK_HPP
#define C2NG_GAME_INTERFACE_USERINTERFACEPROPERTYSTACK_HPP

#include <vector>
#include "game/interface/userinterfaceproperty.hpp"
#include "afl/data/value.hpp"

namespace game { namespace interface {

    class UserInterfacePropertyAccessor;

    /** User interface property stack.
        This manages a stack of UserInterfacePropertyAccessor objects and provides a simple interface to implement set/get operations.
        This is used in the implementation of GlobalContext to manage the changing user interface properties.

        A process' property stack is determined when the process is started.
        If the process is started on a ship, it will always run on that ship.
        If a process is started on a dialog, we cannot provide status information for that dialog from a context.
        This would mean that we have to keep this status information around for a long time, including possible serialisation.
        In addition, if the script would invoke a user-interface mode change, it would always keep seeing the dialog status.

        Therefore, user interface properties are modeled as part of the global context which every process sees.
        These properties can change during lifetime of a script.
        We need to manage an internal stack to keep track of nested screens/dialogs.

        A dialog or screen that wants to provide context will instantiate an UserInterfacePropertyAccessor descendant
        and register it with the session's UserInterfacePropertyStack.
        The easiest way to do that is as using util::RequestSender<game::Session>::makeTemporary.

        <b>Lifetime</b>

        UserInterfacePropertyStack does not manage lifetimes.
        All add()ed UserInterfacePropertyAccessor's must either outlive it, or be remove()d before they die. */
    class UserInterfacePropertyStack {
     public:
        /** Constructor.
            Makes an empty stack. */
        UserInterfacePropertyStack();

        /** Destructor. */
        ~UserInterfacePropertyStack();

        /** Add property accessor.
            The new property accessor will be the first one to be asked for a property value.
            It can decide to answer the request or defer it to the previous accessor.
            \param a Property accessor. See class description for lifetime. */
        void add(UserInterfacePropertyAccessor& a);

        /** Remove property accessor.
            This function is typically used to remove the least-recently added accessor,
            but can also deal with removing another one or one that isn't registered.
            \param a Property accessor. */
        void remove(UserInterfacePropertyAccessor& a);

        /** Get property.
            \param p Property to get
            \return newly-allocated value, may be 0. Caller takes ownership. */
        afl::data::Value* get(UserInterfaceProperty p) const;

        /** Set property.
            \param p Property to set
            \param value Value to set, may be 0. Owned by caller. */
        void set(UserInterfaceProperty p, const afl::data::Value* value);

     private:
        std::vector<UserInterfacePropertyAccessor*> m_stack;
    };

} }

#endif
