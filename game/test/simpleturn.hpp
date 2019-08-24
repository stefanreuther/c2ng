/**
  *  \file game/test/simpleturn.hpp
  */
#ifndef C2NG_GAME_TEST_SIMPLETURN_HPP
#define C2NG_GAME_TEST_SIMPLETURN_HPP

#include "game/turn.hpp"
#include "game/test/interpreterinterface.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/hostversion.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"

namespace game { namespace test {

    /** A simple game turn.
        An aggregation of a bunch of objects along with quick and simple way to add ships and planets,
        to set up tests for interacting objects. */
    class SimpleTurn {
     public:
        /** Constructor.
            Makes an empty turn. */
        SimpleTurn();

        /*
         *  Embedded objects (modify as desired)
         */

        /** Access embedded Turn object. */
        Turn& turn()
            { return m_turn; }

        /** Access embedded Universe object.
            Same as turn().universe(). */
        game::map::Universe& universe()
            { return turn().universe(); }

        /** Access embedded InterpreterInterface object. */
        game::InterpreterInterface& interface()
            { return m_interface; }

        /** Access embedded HostConfiguration object. */
        game::config::HostConfiguration& config()
            { return m_config; }

        /** Access embedded ShipList object. */
        game::spec::ShipList& shipList()
            { return m_shipList; }

        /** Access embedded HostVersion object. */
        HostVersion& version()
            { return m_version; }

        /*
         *  Objects
         */

        /** Add a ship.
            The ship will be created at the configured position (setPosition), with the configured hull (setHull).
            The hull will be created if necessary.
            \param shipId       Ship Id
            \param owner        Ship owner
            \param playability  Desired playability
            \return Ship object */
        game::map::Ship& addShip(Id_t shipId, int owner, game::map::Object::Playability playability);

        /** Add a planet.
            The planet will be created at the configured position (setPosition).
            \param planetId     Planet Id
            \param owner        Planet owner
            \param playability  Desired playability
            \return Planet object */
        game::map::Planet& addPlanet(Id_t planetId, int owner, game::map::Object::Playability playability);

        /** Set hull for future ships.
            \param n Hull number */
        void setHull(int n)
            { m_hullNr = n; }

        /** Set position for future objects.
            \param pt Position */
        void setPosition(game::map::Point pt)
            { m_position = pt; }

     private:
        Turn m_turn;
        InterpreterInterface m_interface;
        game::config::HostConfiguration m_config;
        game::spec::ShipList m_shipList;
        HostVersion m_version;

        game::map::Point m_position;
        int m_hullNr;
    };

} }

#endif
