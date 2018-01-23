/**
  *  \file game/map/markings.hpp
  *  \brief Class game::map::Markings
  */
#ifndef C2NG_GAME_MAP_MARKINGS_HPP
#define C2NG_GAME_MAP_MARKINGS_HPP

#include "game/map/markingvector.hpp"
#include "afl/base/signal.hpp"

namespace game { namespace map {

    class Universe;

    /** Marked objects.
        This class stores "mark" bits for all objects in multiple layers.
        Therefore, it glues a couple of MarkingVector's together to a Universe.
        In addition, this class stores a currently-selected layer number.

        Each layer contains mark bits separately for object types.

        Objects have an embedded "mark" bit (Object::isMarked()).
        This class stores a shadow copy.
        Whenever an operation is attempted on the whole selection layer,
        it is synchronized both ways.

        Layers are numbered starting from 0, see getNumLayers(). */
    class Markings {
     public:
        /** Object type for a query. */
        enum Kind {
            Ship,               ///< Marked ships.
            Planet              ///< Marked planets.
        };

        /** Constructor.
            Makes a blank object where everything is unmarked. */
        Markings();

        /** Destructor. */
        ~Markings();

        /** Clear all markings.
            Resets this object to the empty post-constructor state.
            Does NOT update the universe. */
        void clear();

        /** Copy from a universe.
            Updates a selection layer from the universe.
            Note that this does not count as a change to markings, and thus does not trigger sig_selectionChange.
            \param u Universe to read from
            \param layer Target layer [0, getNumLayers()) */
        void copyFrom(Universe& u, size_t layer);

        /** Copy to universe.
            Updates the universe from a selection layer.
            \param u Universe to update
            \param layer Source layer [0, getNumLayers()) */
        void copyTo(Universe& u, size_t layer);

        /** Limit to existing objects.
            Unmarks all objects in the Markings/MarkingVector that do not exist in the universe.
            Permitted objects:
            - ships: history ships (even invisible ones, HistoryShipType)
            - planets: all planets on map (AnyPlanetType)

            \param u Universe to update
            \param layer Layer [0, getNumLayers()) */
        void limitToExistingObjects(Universe& u, size_t layer);

        /** Execute compiled expression.
            Replaces \c targetLayer's content with the result of the given expression.
            \param compiledExpression Compiled expression (see interpreter::SelectionExpression)
            \param targetLayer        Target layer (accessible as opCurrent in expression,  [0, getNumLayers()))
            \param u                  Universe */
        void executeCompiledExpression(const String_t& compiledExpression, size_t targetLayer, Universe& u);

        /** Get current layer number.
            \return layer [0, getNumLayers()) */
        size_t getCurrentLayer() const;

        /** Set current layer number.
            Stores the current markings from the in the original layer,
            and updates the universe with the new ones.
            \param newLayer New layer [0, getNumLayers())
            \param u        Universe */
        void setCurrentLayer(size_t newLayer, Universe& u);

        /** Get MarkingVector for one area.
            \param k     Object type
            \param layer Layer number [0, getNumLayers())
            \return MarkingVector; null if parameter out of range */
        MarkingVector* get(Kind k, size_t layer);

        /** Get all MarkingVector's for one area.
            \param k Object type */
        afl::base::Memory<MarkingVector> get(Kind k);

        /** Get number of layers.
            \return number */
        size_t getNumLayers() const;

        /** Signal: change.
            Called whenever anything changes to the selection-as-whole:
            - change of a whole layer
            - change of current selection layer */
        afl::base::Signal<void()> sig_selectionChange;

     private:
        static const size_t NUM_LAYERS = 8;

        MarkingVector m_ships[NUM_LAYERS];
        MarkingVector m_planets[NUM_LAYERS];

        size_t m_currentLayer;
    };

} }

#endif
