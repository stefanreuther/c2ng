/**
  *  \file ui/res/generatedplanetprovider.hpp
  */
#ifndef C2NG_UI_RES_GENERATEDPLANETPROVIDER_HPP
#define C2NG_UI_RES_GENERATEDPLANETPROVIDER_HPP

#include "ui/res/provider.hpp"

namespace ui { namespace res {

    class GeneratedPlanetProvider : public Provider {
     public:
        GeneratedPlanetProvider();
        ~GeneratedPlanetProvider();

        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t name, Manager& mgr);

     private:
        afl::base::Ptr<gfx::Canvas> renderPlanet(int temp, int id);
    };


} }

#endif
