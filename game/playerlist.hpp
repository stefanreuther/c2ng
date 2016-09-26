/**
  *  \file game/playerlist.hpp
  */
#ifndef C2NG_GAME_PLAYERLIST_HPP
#define C2NG_GAME_PLAYERLIST_HPP

#include "game/player.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/playerset.hpp"
#include "afl/base/signal.hpp"

namespace game {

    class PlayerList {
     public:
        PlayerList();

        ~PlayerList();

        Player* create(int id);

        Player* get(int id) const;

        void clear();

        int size() const;

        Player* getPlayerFromCharacter(char ch) const;

        char getCharacterFromPlayer(int id) const;

        String_t expandNames(String_t tpl) const;

        PlayerSet_t getAllPlayers() const;

        Player* getFirstPlayer() const;

        Player* getNextPlayer(Player* p) const;

        Player* getNextPlayer(int n) const;

        void notifyListeners();

        afl::base::Signal<void()> sig_change;

     private:
        afl::container::PtrVector<Player> m_players;

        void init();
        Player* findNextPlayer(int nr) const;
    };

}

#endif
