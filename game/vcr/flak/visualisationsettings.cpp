/**
  *  \file game/vcr/flak/visualisationsettings.cpp
  *  \brief Class game::vcr::flak::VisualisationSettings
  */

#include <cmath>
#include "game/vcr/flak/visualisationsettings.hpp"
#include "util/math.hpp"

namespace {
    const float MIN_DISTANCE = 2000;
    const float MAX_DISTANCE = 500000;
    const float MAX_HEIGHT = float(util::PI * 0.95);

    float limitAngle(float a) {
        const float FULL_CIRCLE = float(util::PI * 2);
        if (a < 0) {
            a += FULL_CIRCLE;
        }
        if (a > FULL_CIRCLE) {
            a -= FULL_CIRCLE;
        }
        return a;
    }

    float updateAngle(float current, float target, float speed) {
        float delta = current - target;
        if (delta < -util::PI) {
            delta += float(2*util::PI);
        }
        if (delta > util::PI) {
            delta -= float(2*util::PI);
        }
        if (std::abs(delta) < speed) {
            return target;
        } else if (delta < 0) {
            return current + speed;
        } else {
            return current - speed;
        }
    }
}

game::vcr::flak::VisualisationSettings::VisualisationSettings()
    : m_autoCamera(true),
      m_followedFleet(Visualizer::NO_ENEMY),
      m_cameraAzimuth(0),
      m_cameraHeight(float(util::PI/3)),
      m_cameraDistance(50000),
      m_cameraRaise(0),
      m_cameraRaiseTarget(0),
      m_raiseSpeed(100)
{ }

game::vcr::flak::VisualisationSettings::Changes_t
game::vcr::flak::VisualisationSettings::updateCamera(const VisualisationState& st)
{
    // ex fvUpdateCamera, flak.pas:CamFollow
    Changes_t result;
    if (m_autoCamera) {
        float target_azi = float(20*util::PI/180);
        float target_he  = float(30*util::PI/180);

        // If we're following a fleet, but it died; pick a new one. Prefer same player.
        const VisualisationState::Fleet* f = st.fleets().at(m_followedFleet);
        if (f != 0 && !f->isAlive) {
            Fleet_t b = Visualizer::NO_ENEMY;
            int player = f->player;
            for (Fleet_t a = 0; a < st.fleets().size(); ++a) {
                const VisualisationState::Fleet& p = *st.fleets().at(a);
                if (p.isAlive && (b == Visualizer::NO_ENEMY || (p.player == player && st.fleets().at(b)->player != player))) {
                    b = a;
                }
            }
            if (b != Visualizer::NO_ENEMY) {
                m_followedFleet = b;
                result += FollowChange;
            }
        }

        // Determine target camera position
        f = st.fleets().at(m_followedFleet);
        if (f != 0 && f->isAlive) {
            int32_t dx = f->x, dy = f->y;
            if (const VisualisationState::Ship* ene = st.ships().at(f->enemy)) {
                dx -= ene->pos.x;
                dy -= ene->pos.y;
            }
            if (dx == 0 && dy == 0) {
                target_azi = m_cameraAzimuth;
            } else {
                target_azi = float(std::atan2(double(dx), double(dy)) + 40*util::PI/180);
                if (target_azi > 2*util::PI) {
                    target_azi -= float(2*util::PI);
                }
            }
        }

        // Smooth update
        m_cameraAzimuth = updateAngle(m_cameraAzimuth, target_azi, 1.0/512);
        m_cameraHeight  = updateAngle(m_cameraHeight,  target_he,  1.0/512);

        // Now, the distance
        float size = st.getArenaSize() * 1.25f;
        if (m_cameraDistance > size) {
            m_cameraDistance -= std::min(100.0f, (m_cameraDistance - size)/2);
        }
        if (m_cameraDistance < size) {
            m_cameraDistance += std::min(100.0f, (size - m_cameraDistance)/2);
        }

        // Finally, raise (this is used to sort-of get it out of the way of detail panels)
        int delta = m_cameraRaise - m_cameraRaiseTarget;
        if (std::abs(delta) <= m_raiseSpeed) {
            m_cameraRaise = m_cameraRaiseTarget;
        } else if (delta > 0) {
            m_cameraRaise -= m_raiseSpeed;
        } else {
            m_cameraRaise += m_raiseSpeed;
        }
        result += ParameterChange;
    }
    return result;
}

game::vcr::flak::VisualisationSettings::Changes_t
game::vcr::flak::VisualisationSettings::followFleet(Fleet_t fleet, const VisualisationState& st)
{
    // ex Client.FlakVis.followFleet
    Changes_t result;
    if (fleet != m_followedFleet) {
        m_followedFleet = fleet;
        result += FollowChange;
    }
    if (const VisualisationState::Fleet* f = st.fleets().at(fleet)) {
        if (f->isAlive && !m_autoCamera) {
            result += toggleAutoCamera();
        }
    }
    return result;
}

game::vcr::flak::VisualisationSettings::Changes_t
game::vcr::flak::VisualisationSettings::followPlayer(int player, const VisualisationState& st)
{
    // ex Client.FlakVis.followPlayer
    Changes_t result;

    afl::base::Memory<const VisualisationState::Fleet> fleets = st.fleets();
    for (Fleet_t i = 0; i < fleets.size(); ++i) {
        const VisualisationState::Fleet& f = *fleets.at(i);
        if (f.isAlive && f.player == player) {
            result += followFleet(i, st);
            break;
        }
    }
    return result;
}

game::vcr::flak::VisualisationSettings::Fleet_t
game::vcr::flak::VisualisationSettings::getFollowedFleet() const
{
    // ex Client.FlakVis.getFollowedFleet
    return m_followedFleet;
}

game::vcr::flak::VisualisationSettings::Changes_t
game::vcr::flak::VisualisationSettings::toggleAutoCamera()
{
    // ex Client.FlakVis.toggleCamera
    Changes_t result;
    m_autoCamera = !m_autoCamera;
    result += CameraChange;
    result += forceUpdateCamera();
    return result;
}

bool
game::vcr::flak::VisualisationSettings::isAutoCamera() const
{
    // ex Client.FlakVis.isAutoCamera
    return m_autoCamera;
}

game::vcr::flak::VisualisationSettings::Changes_t
game::vcr::flak::VisualisationSettings::zoomIn()
{
    // ex Client.FlakVis.zoomIn
    Changes_t result;

    // Disable auto cam upon manual move
    if (m_autoCamera) {
        result += toggleAutoCamera();
    }

    m_cameraDistance = std::max(MIN_DISTANCE, m_cameraDistance - 100);
    result += ParameterChange;
    return result;
}

game::vcr::flak::VisualisationSettings::Changes_t
game::vcr::flak::VisualisationSettings::zoomOut()
{
    // ex Client.FlakVis.zoomOut
    Changes_t result;

    // Disable auto cam upon manual move
    if (m_autoCamera) {
        result += toggleAutoCamera();
    }

    m_cameraDistance = std::min(MAX_DISTANCE, m_cameraDistance + 100);
    result += ParameterChange;
    return result;
}

game::vcr::flak::VisualisationSettings::Changes_t
game::vcr::flak::VisualisationSettings::move(float dh, float da)
{
    // ex Client.FlakVis.move
    Changes_t result;

    // Disable auto cam upon manual move
    if (m_autoCamera) {
        result += toggleAutoCamera();
    }

    // Limit height
    m_cameraHeight = std::min(MAX_HEIGHT, std::max(-MAX_HEIGHT, m_cameraHeight + dh));

    // Limit azimuth
    m_cameraAzimuth = limitAngle(m_cameraAzimuth + da);

    result += ParameterChange;
    return result;
}

game::vcr::flak::VisualisationSettings::Changes_t
game::vcr::flak::VisualisationSettings::setCameraRaiseTarget(int t)
{
    // ex Client.FlakVis.setRaiseTarget
    m_cameraRaiseTarget = t;
    return forceUpdateCamera();
}

void
game::vcr::flak::VisualisationSettings::setCameraRaiseSpeed(int n)
{
    m_raiseSpeed = n;
}

float
game::vcr::flak::VisualisationSettings::getCameraAzimuth() const
{
    return m_cameraAzimuth;
}

float
game::vcr::flak::VisualisationSettings::getCameraHeight() const
{
    return m_cameraHeight;
}

float
game::vcr::flak::VisualisationSettings::getCameraDistance() const
{
    return m_cameraDistance;
}

int
game::vcr::flak::VisualisationSettings::getCameraRaise() const
{
    return m_cameraRaise;
}

game::vcr::flak::VisualisationSettings::Changes_t
game::vcr::flak::VisualisationSettings::forceUpdateCamera()
{
    // ex fvForceUpdateCamera
    Changes_t result;
    if (!m_autoCamera) {
        if (m_cameraRaise != m_cameraRaiseTarget) {
            m_cameraRaise = m_cameraRaiseTarget;
            result += ParameterChange;
        }
    }
    return result;
}
