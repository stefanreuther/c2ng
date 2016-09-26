/**
  *  \file game/spec/engine.hpp
  */
#ifndef C2NG_GAME_SPEC_ENGINE_HPP
#define C2NG_GAME_SPEC_ENGINE_HPP

#include "game/spec/component.hpp"

namespace game { namespace spec {

    class Engine : public Component {
     public:
        static const int MAX_WARP = 9;

        explicit Engine(int id);
        virtual ~Engine();

        bool getFuelFactor(int warp, int32_t& fuelFactor) const;
        void setFuelFactor(int warp, int32_t fuelFactor);

        int getMaxEfficientWarp() const;
        void setMaxEfficientWarp(int warp);

     private:
        int m_maxEfficientWarp;
        int32_t m_fuelFactors[MAX_WARP];
    };

} }

#endif
