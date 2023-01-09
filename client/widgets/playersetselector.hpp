/**
  *  \file client/widgets/playersetselector.hpp
  *  \brief Class client::widgets::PlayerSetSelector
  */
#ifndef C2NG_CLIENT_WIDGETS_PLAYERSETSELECTOR_HPP
#define C2NG_CLIENT_WIDGETS_PLAYERSETSELECTOR_HPP

#include "afl/string/translator.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "ui/root.hpp"
#include "ui/widgets/checkboxlistbox.hpp"

namespace client { namespace widgets {

    /** Player set selector.
        Displays a list of players with optional information, and lets the user choose a subset.

        Use for unpack, sweep, message receiver.

        Nonzero player numbers correspond to regular players.
        Slot 0 is always displayed at the bottom and corresponds to the host.

        @change In PCC1, PCC2, this was a list with checkmarks/dots in front of the names.
        We now base it on the CheckboxListbox so it looks a little more according to current standards. */
    class PlayerSetSelector : public ui::widgets::CheckboxListbox {
     public:
        /** Constructor.
            \param root    UI root
            \param names   Names to display.
            \param set     Set of players to display
            \param tx      Translator */
        PlayerSetSelector(ui::Root& root, const game::PlayerArray<String_t>& names, game::PlayerSet_t set, afl::string::Translator& tx);
        ~PlayerSetSelector();

        /** Set selected players.
            Those have a checked box in front of them.
            This set is initially empty.
            \param set player set */
        void setSelectedPlayers(game::PlayerSet_t set);

        /** Set selectable players.
            Those can be chosen in the list box.
            This set is initially the same as the set passed to the constructor.
            \param set player set */
        void setSelectablePlayers(game::PlayerSet_t set);

        /** Get selected players.
            \return set */
        game::PlayerSet_t getSelectedPlayers() const;

        /** Toggle a player's state.
            \param player Player number */
        void togglePlayer(int player);

        /** Toggle a player set's state.
            If this set is selected, it's unselected, and vice versa.
            \param set Set to toggle */
        void togglePlayers(game::PlayerSet_t set);

        /** Toggle all selectable players. */
        void toggleAll();

        /** Signal: set change.
            Emitted whenever getSelectedPlayers() changes. */
        afl::base::Signal<void()> sig_setChange;

     private:
        void init(const game::PlayerArray<String_t>& names, game::PlayerSet_t set, afl::string::Translator& tx);
        void updateImageName(Item* p);
        void updateAllImageNames();

        game::PlayerSet_t m_selectedPlayers;
        game::PlayerSet_t m_selectablePlayers;
    };

} }

#endif
