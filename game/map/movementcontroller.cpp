/**
  *  \file game/map/movementcontroller.cpp
  *  \brief Class game::map::MovementController
  */

#include <cmath>
#include "game/map/movementcontroller.hpp"
#include "game/map/configuration.hpp"
#include "util/math.hpp"

game::map::MovementController::MovementController()
    : m_targetPosition(),
      m_currentPosition(),
      m_currentValid(),
      m_speed(0),
      m_animationThreshold(11)
{ }

void
game::map::MovementController::setTargetPosition(Point pt)
{
    m_targetPosition = pt;
}

game::map::Point
game::map::MovementController::getCurrentPosition() const
{
    return m_currentPosition;
}

void
game::map::MovementController::setAnimationThreshold(int threshold)
{
    m_animationThreshold = threshold;
}

bool
game::map::MovementController::update(const Configuration& config, int numTicks)
{
    if (!m_currentValid) {
        // First call
        m_currentValid = true;
        m_currentPosition = m_targetPosition;
        return true;
    } else if (m_targetPosition != m_currentPosition) {
        // We have to move
        if (m_speed == 0
            && (config.getSquaredDistance(m_currentPosition, m_targetPosition) <= util::squareInteger(m_animationThreshold)))
        {
            // Do single small movement immediately
            m_currentPosition = m_targetPosition;
            m_speed = 0;
        } else {
            // Big movement only after tick event
            for (int i = 0; i < numTicks; ++i) {
                Point a = m_targetPosition;
                Point b = config.getSimpleNearestAlias(m_currentPosition, a);
                double dist = std::sqrt(double(config.getSquaredDistance(a, b)));

                if (dist <= m_speed) {
                    // We're very close, so go directly
                    m_currentPosition = m_targetPosition;
                    m_speed = 0;
                    break;
                } else {
                    // We're a little farther. Compute speed.
                    if (dist < m_speed*m_speed) {
                        if (m_speed > 1) {
                            --m_speed;
                        }
                    } else {
                        ++m_speed;
                    }

                    // Compute new location.
                    int distx = int((a.getX() - b.getX()) * m_speed / dist);
                    int disty = int((a.getY() - b.getY()) * m_speed / dist);

                    // Ensure we make progress.
                    if (distx == 0 && a.getX() != b.getX()) {
                        distx = (a.getX() < b.getX() ? -1 : +1);
                    }
                    if (disty == 0 && a.getY() != b.getY()) {
                        disty = (a.getY() < b.getY() ? -1 : +1);
                    }

                    // Move.
                    m_currentPosition = Point(b.getX() + distx, b.getY() + disty);
                }
            }
        }
        return true;
    } else {
        // Nothing to do
        return false;
    }
}
