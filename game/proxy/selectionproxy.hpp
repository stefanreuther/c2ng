/**
  *  \file game/proxy/selectionproxy.hpp
  *  \brief Class game::proxy::SelectionProxy
  */
#ifndef C2NG_GAME_PROXY_SELECTIONPROXY_HPP
#define C2NG_GAME_PROXY_SELECTIONPROXY_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/slaverequestsender.hpp"

namespace game { namespace proxy {

    /** Bidirectional proxy for selection.
        This accesses a Session > Game > Selections object.

        Bidirectional, synchronous:
        - retrieve initial state (init)
        - execute user-provided expression (executeExpression)

        Bidirectional, asynchronous:
        - execute hard-coded expression (clearLayer, clearAllLayers, etc.)
        - select layer (setCurrentLayer)
        - update from game (sig_selectionChange) */
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
        void setCurrentLayer(size_t newLayer);

        /** Execute user-provided expression, synchronously.
            \param       ind          WaitIndicator for UI synchronisation
            \param [in]  expression   User-provided expression
            \param [in]  targetLayer  Result of expression is assigned here
            \param [out] error        On error, error message produced here
            \retval true  Expression parsed correctly; update will be reported using sig_selectionChange
            \retval false Expression failed to parse; error message placed in \c error
            \see game::map::Selections::executeCompiledExpression */
        bool executeExpression(WaitIndicator& ind, const String_t& expression, size_t targetLayer, String_t& error);

        /** Clear layer, asynchronously.
            The change is reported using sig_selectionChange.
            \param targetLayer Layer to clear */
        void clearLayer(size_t targetLayer);

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

        /** Signal: selection change.
            Called whenever the current status changes.
            \param info Information about selections
            \see init()
            \see game::map::Selections::sig_selectionChange */
        afl::base::Signal<void(const Info& info)> sig_selectionChange;

     private:
        class Trampoline;
        util::RequestReceiver<SelectionProxy> m_reply;
        util::SlaveRequestSender<Session, Trampoline> m_request;

        void executeCompiledExpression(String_t compiledExpression, size_t targetLayer);
        void executeCompiledExpressionAll(String_t compiledExpression);
    };

} }

#endif
