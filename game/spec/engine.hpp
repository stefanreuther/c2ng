/**
  *  \file game/spec/engine.hpp
  *  \brief Class game::spec::Engine
  */
#ifndef C2NG_GAME_SPEC_ENGINE_HPP
#define C2NG_GAME_SPEC_ENGINE_HPP

#include "game/spec/component.hpp"
#include "afl/base/optional.hpp"

namespace game { namespace spec {

    /** An engine.
        This class only holds data which it does not interpret or limit.

        In addition to the standard specification values, Engine can store and optionally compute the maximum efficient warp factor.
        This is defined as the highest warp factor where this engine runs at 120% fuel usage or less.  */
    class Engine : public Component {
     public:
        /** Maxium warp factor supported by engines. */
        static const int MAX_WARP = 9;

        /** Constructor.
            \param id engine Id */
        explicit Engine(int id);

        /** Destructor. */
        virtual ~Engine();

        /** Get fuel factor.
            The fuel factor is used in fuel usage computations.
            \param warp [in] Warp factor [0, MAX_WARP]
            \return Fuel factor if known; nothing if warp factor was out of range */
        afl::base::Optional<int32_t> getFuelFactor(int warp) const;

        /** Set fuel factor.
            \param warp Warp factor [0, MAX_WARP]. Out-of-range values are ignored.
            \param fuelFactor Fuel factor */
        void setFuelFactor(int warp, int32_t fuelFactor);

        /** Get maximum efficient warp.
            \return maximum efficient warp */
        int getMaxEfficientWarp() const;

        /** Set maximum efficient warp.
            \param warp Warp factor [0, MAX_WARP]

            Setting warp=0 enables automatic computation (default).
            Note that setMaxEfficientWarp(getMaxEfficientWarp()) is not a null operation;
            it replaces the automatic/computed value with a fixed value that does no longer update after setFuelFactor() calls. */
        void setMaxEfficientWarp(int warp);

     private:
        int m_maxEfficientWarp;
        int32_t m_fuelFactors[MAX_WARP];
    };

} }

// Get fuel factor.
inline afl::base::Optional<int32_t>
game::spec::Engine::getFuelFactor(int warp) const
{
    if (warp <= 0) {
        return 0;
    } else if (warp <= MAX_WARP) {
        return m_fuelFactors[warp-1];
    } else {
        return afl::base::Nothing;
    }
}

#endif
