/**
  *  \file server/host/gamerating.hpp
  *  \brief Game Rating
  */
#ifndef C2NG_SERVER_HOST_GAMERATING_HPP
#define C2NG_SERVER_HOST_GAMERATING_HPP

namespace server { namespace host {

    class Root;
    class Game;

    /** Compute difficulty rating of a game.
        If the game has already been hosted, computes the rating from the game directory.
        If the game is still preparing, attempts to give a good estimate using the game fragments.
        It also attempts to merge hardcoded difficulties.

        \param root Service root
        \param g Game

        \return Rating [0,100] */
    int computeGameRating(Root& root, Game& g);

} }

#endif
