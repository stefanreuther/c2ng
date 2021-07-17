/**
  *  \file game/vcr/flak/visualisationsettings.hpp
  *  \brief Class game::vcr::flak::VisualisationSettings
  */
#ifndef C2NG_GAME_VCR_FLAK_VISUALISATIONSETTINGS_HPP
#define C2NG_GAME_VCR_FLAK_VISUALISATIONSETTINGS_HPP

#include "afl/bits/smallset.hpp"
#include "game/vcr/flak/visualisationstate.hpp"

namespace game { namespace vcr { namespace flak {

    /** Visualisation settings.
        Contains settings that are not (directly) affected by playback forward/rewind:
        - camera azimuth (angle)
        - camera height (elevation)
        - camera distance
        - camera raise (additional height adjustment, as an ad-hoc mechanism to make room for overlays
        - auto-camera flag
        - followed fleet

        Camera can automatically follow playback, or manually controlled.
        Manual control turns off automatic following.

        Instead of signal callbacks, modifications return a set of changes to process by the caller as needed. */
    class VisualisationSettings {
     public:
        /** Kind of change.
            Instead of callbacks, methods return a set of changes to signal implicit changes. */
        enum Change {
            CameraChange,            ///< Camera mode changed (state of "auto camera" toggle).
            FollowChange,            ///< Followed fleet changed (state of "followed fleet" change).
            ParameterChange          ///< Parameter change (just re-render the same content).
        };
        typedef afl::bits::SmallSet<Change> Changes_t;

        /** Type for a fleet index. */
        typedef VisualisationState::Fleet_t Fleet_t;


        /** Constructor. */
        VisualisationSettings();

        /** Update the camera if enabled. Call once per battle tick.
            \param st State
            \return changes */
        Changes_t updateCamera(const VisualisationState& st);

        /** Follow a fleet.
            \param fleet Fleet number
            \param st    State
            \return changes */
        Changes_t followFleet(Fleet_t fleet, const VisualisationState& st);

        /** Follow a player. Finds a fleet owned by the player and follows that.
            \param player Player
            \param st     State
            \return changes */
        Changes_t followPlayer(int player, const VisualisationState& st);

        /** Get currently-followed fleet.
            \return fleet number (can be NO_ENEMY) */
        Fleet_t getFollowedFleet() const;

        /** Toggle automatic camera.
            \return changes */
        Changes_t toggleAutoCamera();

        /** Check for automatic camera.
            \return status */
        bool isAutoCamera() const;

        /** Zoom in (move closer).
            \return changes */
        Changes_t zoomIn();

        /** Zoom out (move away).
            \return changes */
        Changes_t zoomOut();

        /** Move camera.
            \param dh Height (elevantion) change in radians
            \param da Azimuth (height) change in radians
            \return changes */
        Changes_t move(float dh, float da);

        /** Set target for camera raise.
            Camera will move there at raise speed.
            \param t target
            \return changes */
        Changes_t setCameraRaiseTarget(int t);

        /** Set raise speed.
            \param n speed (units per tick)
            \return changes */
        void setCameraRaiseSpeed(int n);

        /** Get camera azimuth (angle).
            \return angle in radians */
        float getCameraAzimuth() const;

        /** Get camera height (elevation).
            \return angle in radians */
        float getCameraHeight() const;

        /** Get camera distance.
            \return distance */
        float getCameraDistance() const;

        /** Get camera raise.
            \return raise in units */
        int getCameraRaise() const;

     private:
        // Camera
        bool m_autoCamera;         // ex _camAuto
        Fleet_t m_followedFleet;   // ex _camFollowFleet
        float m_cameraAzimuth;     // ex _camAzimut
        float m_cameraHeight;      // ex _camHeight
        float m_cameraDistance;    // ex _camDistance      // int?
        int m_cameraRaise;         // ex _camRaise         // int?
        int m_cameraRaiseTarget;   // ex _camRaiseTarget   // int?
        int m_raiseSpeed;

        Changes_t forceUpdateCamera();
    };

} } }

#endif
