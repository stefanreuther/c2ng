/**
  *  \file client/widgets/planetmineralinfo.hpp
  *  \brief Class client::widgets::PlanetMineralInfo
  */
#ifndef C2NG_CLIENT_WIDGETS_PLANETMINERALINFO_HPP
#define C2NG_CLIENT_WIDGETS_PLANETMINERALINFO_HPP

#include "afl/string/translator.hpp"
#include "game/map/planetinfo.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "util/numberformatter.hpp"

namespace client { namespace widgets {

    /** Mineral information.
        Displays information for one mineral: mined and core amount, density.

        Usage:
        - create
        - call setContent to update content */
    class PlanetMineralInfo : public ui::SimpleWidget {
     public:
        /** Display mode. */
        enum Mode {
            /** Draw nothing (empty field). */
            Blank,

            /** Special handling for first field.
                Display normal information.
                For Unknown information, explains unavailability.
                For Scanned information, explains the age. */
            First,

            /** Special handling for subsequent fields.
                Display normal information; no additional explanations for Unknown/Scanned. */
            Second
        };

        typedef game::map::PlanetMineralInfo Info_t;

        /** Constructor.
            @param root UI root
            @param fmt  Number formatter
            @param tx   Translator */
        PlanetMineralInfo(ui::Root& root, util::NumberFormatter fmt, afl::string::Translator& tx);

        /** Destructor. */
        ~PlanetMineralInfo();

        /** Set content.
            @param name Name of this mineral
            @param info Information
            @param mode Display mode */
        void setContent(String_t name, const Info_t& info, Mode mode);

        // Widget/SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        // Final state:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        const util::NumberFormatter m_formatter;

        // Variable state:
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
