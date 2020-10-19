/**
  *  \file ui/res/generatedengineprovider.hpp
  *  \brief Class ui::res::GeneratedEngineProvider
  */
#ifndef C2NG_UI_RES_GENERATEDENGINEPROVIDER_HPP
#define C2NG_UI_RES_GENERATEDENGINEPROVIDER_HPP

#include "afl/base/ref.hpp"
#include "afl/string/translator.hpp"
#include "gfx/font.hpp"
#include "ui/res/provider.hpp"

namespace ui { namespace res {

    /** Resource Provider for Engine Images.
        Engines are represented by a fuel-usage chart.
        This renders these charts. */
    class GeneratedEngineProvider : public Provider {
     public:
        /** Constructor.
            \param font  Font to use for chart legends
            \param tx    Translator to use for chart legends */
        GeneratedEngineProvider(afl::base::Ref<gfx::Font> font, afl::string::Translator& tx);

        /** Destructor. */
        ~GeneratedEngineProvider();

        // Provider:
        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t name, Manager& mgr);

     private:
        afl::base::Ref<gfx::Font> m_font;
        afl::string::Translator& m_translator;
    };

    /** Render engine fuel-usage chart.
        \param fuelUsage Fuel usage for warp factors 1..9, normalized to 0..1000.
        \param size      Desired image size
        \param font      Font to use for chart legends
        \param tx        Translator to use for chart legends
        \return Newly-created image */
    afl::base::Ref<gfx::Canvas> renderEngineDiagram(const std::vector<int>& fuelUsage, gfx::Point size, gfx::Font& font, afl::string::Translator& tx);

} }

#endif
