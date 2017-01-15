/**
  *  \file gfx/nullresourceprovider.hpp
  *  \brief Class gfx::NullResourceProvider
  */
#ifndef C2NG_GFX_NULLRESOURCEPROVIDER_HPP
#define C2NG_GFX_NULLRESOURCEPROVIDER_HPP

#include "gfx/resourceprovider.hpp"

namespace gfx {

    /** Null resource provider.
        This class is used for testing.
        It answers all getImage() requests as "not found",
        and all getFont() requests using the default font (createDefaultFont()). */
    class NullResourceProvider : public ResourceProvider {
     public:
        NullResourceProvider();
        ~NullResourceProvider();

        // ResourceProvider:
        virtual afl::base::Ptr<Canvas> getImage(String_t name, bool* status = 0);
        virtual afl::base::Ref<Font> getFont(FontRequest req);

     private:
        afl::base::Ref<Font> m_font;
    };

}

#endif
