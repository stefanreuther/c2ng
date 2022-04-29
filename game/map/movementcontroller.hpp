/**
  *  \file game/map/movementcontroller.hpp
  *  \brief Class game::map::MovementController
  */
#ifndef C2NG_GAME_MAP_MOVEMENTCONTROLLER_HPP
#define C2NG_GAME_MAP_MOVEMENTCONTROLLER_HPP

#include "game/map/point.hpp"

namespace game { namespace map {

    class Configuration;

    /** Starchart smooth movement controller.
        Provides logic for smooth movement on a starchart.

        Theory of operation:
        - User input controls a "target position" (e.g. mouse movement, keyboard input, object lock)
        - Starchart display follows the target position by periodically computing a new "current position"

        To use,
        - call setTargetPosition() for every change
        - periodically, call update() to generate a new current position
        - call getCurrentPosition() to obtain new current position */
    class MovementController {
     public:
        /** Constructor. */
        MovementController();

        /** Set target position.
            Controller will move towards that position.
            @param pt Position */
        void setTargetPosition(Point pt);

        /** Get current position.
            @return position */
        Point getCurrentPosition() const;

        /** Set animation threshold.
            Movement equal or larger than this value is animated, movement shorter than this is exeuted directly.
            @param threshold New threshold */
        void setAnimationThreshold(int threshold);

        /** Perform update.
            Moves the current position towards the current position.
            @param config    Starchart configuration
            @param numTicks  Number of ticks, must be 1 or more.
                             Pass the number of animation ticks passed (normally 1, but may be more if animation is lagging).
                             update(a+b) produces the same result as update(a) followed by update(b).
            @retval true   current position has changed. Call function again to obtain next position.
            @retval false  current position has reached target. Future calls will not change anything until setTargetPosition() is called. */
        bool update(const Configuration& config, int numTicks);

     private:
        Point m_targetPosition;
        Point m_currentPosition;
        bool m_currentValid;
        int m_speed;
        int m_animationThreshold;
    };

} }

#endif
