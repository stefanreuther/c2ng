/**
  *  \file gfx/gen/spaceviewconfig.hpp
  *  \brief Class gfx::gen::SpaceViewConfig
  */
#ifndef C2NG_GFX_GEN_SPACEVIEWCONFIG_HPP
#define C2NG_GFX_GEN_SPACEVIEWCONFIG_HPP

#include "afl/base/ref.hpp"
#include "gfx/rgbapixmap.hpp"
#include "util/randomnumbergenerator.hpp"
#include "gfx/point.hpp"

namespace gfx { namespace gen {

    class SpaceViewConfig {
     public:
        SpaceViewConfig();

        void setSize(Point pt);

        void setNumSuns(int n);

        void setStarProbability(int n);

        afl::base::Ref<RGBAPixmap> render(util::RandomNumberGenerator& rng) const;

     private:
        int m_width;
        int m_height;
        int m_numSuns;
        int m_starProbability;
    };

} }

#endif
