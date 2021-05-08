/**
  *  \file game/proxy/selectionproxy.hpp
  *  \brief Class game::proxy::SelectionProxy
  */
#ifndef C2NG_GAME_PROXY_SELECTIONPROXY_HPP
#define C2NG_GAME_PROXY_SELECTIONPROXY_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "game/map/selections.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/ref/list.hpp"
#include "game/session.hpp"

namespace game { namespace proxy {

    /** Bidirectional proxy for selection.
        This accesses a Session > Game > Selections object.

        Bidirectional, synchronous:
        - retrieve initial state (init)
        - execute user-provided expression (executeExpression)

        Bidirectional, asynchronous:
        - execute hard-coded expression (clearLayer, clearAllLayers, etc.)
        - select layer (setCurrentLayer)
        - update from game (sig_selectionChange)
        - mark objects in range (markObjectsInRange, sig_numObjectsInRange) */
    class SelectionProxy {
     public:
        /** Information about a layer. */
        struct Layer {
            size_t numShips;            /**< Number of selected ships. */
            size_t numPlanets;          /**< Number of selected planets. */

            Layer(size_t numShips, size_t numPlanets)
                : numShips(numShips), numPlanets(numPlanets)
                { }
        };

        /** Information about current state. */
        struct Info {
            std::vector<Layer> layers;  /**< Information about all current layers. */
            size_t currentLayer;        /**< Current layer (index into layers). */

            Info()
                : layers(), currentLayer(0)
                { }
        };

        typedef game::map::Selections::LayerReference LayerReference_t;


        /** Constructor.
            \param gameSender Sender
            \param reply RequestDispatcher to send replies back */
        SelectionProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~SelectionProxy();

        /** Get state, synchronously.
            \param       ind      WaitIndicator for UI synchronisation
            \param [out] result   Result */
        void init(WaitIndicator& ind, Info& result);

        /** Set current layer, asynchronously.
            The change is reported using sig_selectionChange.
            \param newLayer New layer
            \see game::map::Selections::setCurrentLayer */
        void setCurrentLayer(LayerReference_t newLayer);

        /** Execute user-provided expression, synchronously.
            \param       ind          WaitIndicator for UI synchronisation
            \param [in]  expression   User-provided expression
            \param [in]  targetLayer  Result of expression is assigned here
            \param [out] error        On error, error message produced here
            \retval true  Expression parsed correctly; update will be reported using sig_selectionChange
            \retval false Expression failed to parse; error message placed in \c error
            \see game::map::Selections::executeCompiledExpression */
        bool executeExpression(WaitIndicator& ind, const String_t& expression, LayerReference_t targetLayer, String_t& error);

        /** Mark objects given as list, asynchronously.
            \param targetLayer Target layer
            \param list        List of objects to process
            \param mark        true to mark, false to unmark
            \see game::map::Selections::markList */
        void markList(LayerReference_t targetLayer, const game::ref::List& list, bool mark);

        /** Clear layer, asynchronously.
            The change is reported using sig_selectionChange.
            \param targetLayer Layer to clear */
        void clearLayer(LayerReference_t targetLayer);

        /** Invert layer, asynchronously.
            The change is reported using sig_selectionChange.
            \param targetLayer Layer to invert */
        void invertLayer(size_t targetLayer);

        /** Clear all layers, asynchronously.
            The change is reported using sig_selectionChange. */
        void clearAllLayers();

        /** Invert all layers, asynchronously.
            The change is reported using sig_selectionChange. */
        void invertAllLayers();

        /** Mark objects in range, asynchronously.
            This does NOT change the information reported by sig_selectionChange.
            \param a First coordinate of a rectangle (inclusive)
            \param b Second coordinate of a rectangle (inclusive)
            \param revertFirst If true, revert the selection first, as if per revertCurrentLayer().
            \see game::map::Universe::markObjectsInRange */
        void markObjectsInRange(game::map::Point a, game::map::Point b, bool revertFirst);

        /** Revert current layer, asynchronously.
            Undoes selection changes to the universe that have happened between creation of the SelectionProxy
            or the last bulk operation.
            This does NOT change the information reported by sig_selectionChange. */
        void revertCurrentLayer();

        /** Signal: selection change.
            Called whenever the current status changes.
            \param info Information about selections
            \see init()
            \see game::map::Selections::sig_selectionChange */
        afl::base::Signal<void(const Info& info)> sig_selectionChange;

        /** Signal: result of markObjectsInRange.
            Called in response to markObjectsInRange to report the number of marked units.
            \param n Number of marked units */
        afl::base::Signal<void(int)> sig_numObjectsInRange;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<SelectionProxy> m_reply;
        util::RequestSender<Trampoline> m_request;

        void executeCompiledExpression(String_t compiledExpression, LayerReference_t targetLayer);
        void executeCompiledExpressionAll(String_t compiledExpression);

        void reportObjectsInRange(int n);
    };

} }

#endif
