/**
  *  \file client/widgets/planetmineralinfo.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_PLANETMINERALINFO_HPP
#define C2NG_CLIENT_WIDGETS_PLANETMINERALINFO_HPP

#include "ui/simplewidget.hpp"
#include "game/map/planetinfo.hpp"
#include "afl/string/translator.hpp"
#include "ui/root.hpp"

namespace client { namespace widgets {

    class PlanetMineralInfo : public ui::SimpleWidget {
     public:
        enum Mode {
            Blank,
            First,
            Second
        };

        typedef game::map::PlanetMineralInfo Info_t;

        PlanetMineralInfo(ui::Root& root, afl::string::Translator& tx);
        ~PlanetMineralInfo();

        void setContent(String_t name,
                        const Info_t& info,
                        Mode mode);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        String_t m_name;
        Info_t m_info;
        Mode m_mode;

        enum NoteType {
            ShowNothing,
            ShowAge,
            ShowMining
        };

        void drawNothing(gfx::Canvas& can);
        void drawExcuse(gfx::Canvas& can);
        void drawBars(gfx::Canvas& can, NoteType type);
    };

} }

#endif
