/**
  *  \file game/test/shiplist.hpp
  *  \brief Ship List Initialisation Functions for Testing
  */
#ifndef C2NG_GAME_TEST_SHIPLIST_HPP
#define C2NG_GAME_TEST_SHIPLIST_HPP

#include "game/spec/shiplist.hpp"

namespace game { namespace test {

    /** Define all standard beams (for testing).
        \param [out] shipList 10 beams will be added here */
    void initStandardBeams(game::spec::ShipList& shipList);

    /** Define all torpedoes beams (for testing).
        \param [out] shipList 10 torpedoes will be added here */
    void initStandardTorpedoes(game::spec::ShipList& shipList);

    /** Define all PList beams (for testing).
        \param [out] shipList 10 beams will be added here */
    void initPListBeams(game::spec::ShipList& shipList);

    /** Define all PList torpedoes (for testing).
        \param [out] shipList 10 torpedoes will be added here */
    void initPListTorpedoes(game::spec::ShipList& shipList);

    /** Define all PList 3.2 beams (for testing).
        \param [out] shipList 10 beams will be added here */
    void initPList32Beams(game::spec::ShipList& shipList);

    /** Define all PList 3.2 torpedoes (for testing).
        \param [out] shipList 10 torpedoes will be added here */
    void initPList32Torpedoes(game::spec::ShipList& shipList);

    /*
     *  Hulls
     */

    const int OUTRIDER_HULL_ID = 1;
    const int GORBIE_HULL_ID = 70;
    const int ANNIHILATION_HULL_ID = 53;
    
    void addOutrider(game::spec::ShipList& list);
    void addGorbie(game::spec::ShipList& list);
    void addAnnihilation(game::spec::ShipList& list);

    /*
     *  Engines
     */

    const int NOVA_ENGINE_ID = 5;
    const int TRANSWARP_ENGINE_ID = 9;

    void addNovaDrive(game::spec::ShipList& list);
    void addTranswarp(game::spec::ShipList& list);

} }

#endif
