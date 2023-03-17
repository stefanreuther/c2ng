/**
  *  \file game/ref/listobserver.hpp
  *  \brief Class game::ref::ListObserver
  */
#ifndef C2NG_GAME_REF_LISTOBSERVER_HPP
#define C2NG_GAME_REF_LISTOBSERVER_HPP

#include "afl/base/signal.hpp"
#include "game/ref/list.hpp"
#include "game/ref/userlist.hpp"
#include "game/session.hpp"

namespace game { namespace ref {

    class Configuration;
    class ConfigurationSelection;

    /** List change observer.
        Maintains a UserList and notifies the user of changes.

        The UserList consists of
        - the main list, a (possibly empty) List of game objects.
          Those are translated into UserList items using the current turn,
          sorted, and amended with dividers according to the sort configuration.
          Changes to the game data, or selection of a different turn, will update the list.
        - the extra list, a (possibly empty) UserList of extra items shown below the game objects.

        To use,
        - register for sig_listChange
        - call setList(), setExtra(), setSession() in any sequence */
    class ListObserver {
     public:
        /** Constructor.
            Makes an empty list that uses the REGULAR ConfigurationSelection. */
        ListObserver();

        /** Set main list.
            If this triggers a change to the result list, emits sig_listChange.
            @param list List; will be copied */
        void setList(const List& list);

        /** Set extra list.
            If this triggers a change to the result list, emits sig_listChange.
            @param list List; will be copied */
        void setExtra(const UserList& list);

        /** Set session.
            Only this call enables the transformation of the main list into a result list;
            before this call, setList() does not produce any output.
            @param session Session; must live as long as the ListObserver */
        void setSession(Session& session);

        /** Set configuration selection.
            Defines the ConfigurationSelection that is used to sort the main list.
            @param sel ConfigurationSelection */
        void setConfigurationSelection(const ConfigurationSelection& sel);

        /** Get effective sort configuration.
            @return configuration */
        Configuration getConfig() const;

        /** Set sort configuration.
            Updates the UserConfiguration in the current session, according to the ConfigurationSelection.
            @param config New configuration */
        void setConfig(const Configuration& config);

        /** Get current result list.
            @return list */
        const UserList& getList() const;

        /** Signal: list change.
            Called whenever the return value of getList() changes. */
        afl::base::Signal<void()> sig_listChange;

     private:
        List m_mainList;
        UserList m_extraList;
        UserList m_resultList;

        const ConfigurationSelection* m_pConfigurationSelection;
        Session* m_pSession;

        afl::base::SignalConnection conn_viewpointTurnChange;
        afl::base::SignalConnection conn_universeChange;

        void updateResultList();
        void onViewpointTurnChange();
        void onUniverseChange();
    };

} }

#endif
