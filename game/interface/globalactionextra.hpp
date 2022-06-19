/**
  *  \file game/interface/globalactionextra.hpp
  *  \brief Class game::interface::GlobalActionExtra
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALACTIONEXTRA_HPP
#define C2NG_GAME_INTERFACE_GLOBALACTIONEXTRA_HPP

#include "game/extra.hpp"
#include "game/interface/globalactions.hpp"
#include "game/session.hpp"
#include "util/treelist.hpp"

namespace game { namespace interface {

    /** Global Action extra.
        Adding this to a Session using GlobalActionExtra::create() will enable that session to do global actions.
        It will provide script functions to manage/define global actions.
        (As of 20220619, this is the AddGlobalAction command.)

        Global actions are managed in a GlobalActions instance.

        Global action names are stored in a util::TreeList.
        Each node's key is 1 plus the actionId from GlobalActions. */
    class GlobalActionExtra : public Extra {
     private:
        /** Constructor.
            @param session Session
            @see create() */
        explicit GlobalActionExtra(Session& session);

     public:
        /** Destructor. */
        ~GlobalActionExtra();

        /** Create GlobalActionExtra for a Session.
            If the Session already has one, returns that, otherwise, creates one.
            This will provide storage for GlobalActions in the session, and register appropriate script commands.
            @param session Session
            @return LabelExtra */
        static GlobalActionExtra& create(Session& session);

        /** Get GlobalActionExtra for a Session.
            @param session Session
            @return GlobalActionExtra if the session has one, otherwise, null. */
        static GlobalActionExtra* get(Session& session);

        /** Access global actions.
            @return GlobalActions instance */
        GlobalActions& actions();
        const GlobalActions& actions() const;

        /** Access global action names.
            @return TreeList instance */
        util::TreeList& actionNames();
        const util::TreeList& actionNames() const;

     private:
        GlobalActions m_actions;
        util::TreeList m_actionNames;
    };

} }

#endif
