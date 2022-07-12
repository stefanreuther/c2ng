/**
  *  \file ui/res/resourcefileprovider.hpp
  */
#ifndef C2NG_UI_RES_RESOURCEFILEPROVIDER_HPP
#define C2NG_UI_RES_RESOURCEFILEPROVIDER_HPP

#include "afl/string/translator.hpp"
#include "ui/res/provider.hpp"
#include "ui/res/resourcefile.hpp"

namespace ui { namespace res {

    class ResourceFileProvider : public Provider {
     public:
        ResourceFileProvider(afl::base::Ref<afl::io::Stream> file, afl::string::Translator& tx);
        ~ResourceFileProvider();

        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t name, Manager& mgr);

     private:
        ResourceFile m_file;

        afl::base::Ptr<gfx::Canvas> loadImageById(uint16_t id, Manager& mgr);
    };

} }

#endif
