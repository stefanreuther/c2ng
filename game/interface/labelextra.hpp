/**
  *  \file game/interface/labelextra.hpp
  *  \brief Class game::interface::LabelExtra
  */
#ifndef C2NG_GAME_INTERFACE_LABELEXTRA_HPP
#define C2NG_GAME_INTERFACE_LABELEXTRA_HPP

#include "afl/base/optional.hpp"
#include "afl/base/signal.hpp"
#include "game/extra.hpp"
#include "game/interface/labelvector.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    /** Central plumbing component for Object Labels.

        This is a Session extra; to enable the Label functionality, just create it using create().
        It will hook all relevant events and automatically provide updated labels.
        It will automatically retrieve the configuration from UserConfiguration.

        In c2ng, labels are updated using regular processes, not temporary processes.
        Those processes run asynchronously and unknown to the UI.
        LabelExtra will start such a process after every change to the universe
        (game::map::Planet::isDirty(), game::map::Ship::isDirty()). */
    class LabelExtra : public Extra {
     private:
        /** Constructor.
            @param session Session
            @see create() */
        explicit LabelExtra(Session& session);

     public:
        /** Destructor. */
        ~LabelExtra();

        /** Create LabelExtra for a Session.
            If the Session already has one, returns that, otherwise, creates one.
            @param session Session
            @return LabelExtra */
        static LabelExtra& create(Session& session);

        /** Get LabelExtra for a Session.
            @param session Session
            @return LabelExtra if the session has one, otherwise, null. */
        static LabelExtra* get(Session& session);

        /** Access ship labels.
            @return ship labels */
        LabelVector& shipLabels();
        const LabelVector& shipLabels() const;

        /** Access planet labels.
            @return planet labels */
        LabelVector& planetLabels();
        const LabelVector& planetLabels() const;

        /** Set configuration.
            Updates the configuration and recomputes everything.
            Unlike directly updating the configuration,
            this function guarantees a sig_change callback,
            even if the configuration does not actually change.
            This allows precise error reporting.
            @param shipExpr     New ship expression; Nothing to leave unchanged.
            @param planetExpr   New planet expression; Nothing to leave unchanged. */
        void setConfiguration(afl::base::Optional<String_t> shipExpr,
                              afl::base::Optional<String_t> planetExpr);

        /** Change signal.
            This signal is emitted after every complete recomputation of the labels.
            (It is not emitted if further changes are still being processed.)

            @param flag true if recomputation caused any labells to change

            Hook this signal for starchart display; redraw if parameter is true. */
        afl::base::Signal<void(bool)> sig_change;

     private:
        class Finalizer;

        // Events
        void onConnectionChange();
        void onViewpointTurnChange();
        void onPreUpdate();
        void onUniverseChanged();
        void onConfigChange();
        void onUpdateComplete();

        // Actions
        void notifyCompletion();
        void checkObjects();
        void markObjects();
        bool runUpdater();

        // Data
        Session& m_session;                     ///< Session link.
        LabelVector m_shipLabels;               ///< Ship label container.
        LabelVector m_planetLabels;             ///< Planet label container.
        bool m_running;                         ///< true if update process is running, to avoid starting multiple in parallel.
        int m_paranoiaCounter;                  ///< Paranoia counter to limit update-retriggering-itself.

        // Signal connections
        afl::base::SignalConnection conn_connectionChange;
        afl::base::SignalConnection conn_viewpointTurnChange;
        afl::base::SignalConnection conn_preUpdate;
        afl::base::SignalConnection conn_universeChange;
        afl::base::SignalConnection conn_configChange;
    };

} }

#endif
