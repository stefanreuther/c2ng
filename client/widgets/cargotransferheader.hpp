/**
  *  \file client/widgets/cargotransferheader.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_CARGOTRANSFERHEADER_HPP
#define C2NG_CLIENT_WIDGETS_CARGOTRANSFERHEADER_HPP

#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace client { namespace widgets {

    class CargoTransferHeader : public ui::SimpleWidget {
     public:
        CargoTransferHeader(ui::Root& root, afl::string::Translator& tx, String_t leftName, String_t rightName);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        String_t m_leftName;
        String_t m_rightName;

        void drawHeader(gfx::Canvas& can, gfx::Rectangle area, const String_t& name);
    };

} }

#endif
