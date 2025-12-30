/**
  *  \file ui/reshack/painter.hpp
  *  \brief Class ui::reshack::Painter
  */
#ifndef C2NG_UI_RESHACK_PAINTER_HPP
#define C2NG_UI_RESHACK_PAINTER_HPP

#include "afl/base/growablememory.hpp"
#include "afl/base/signal.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "ui/colorscheme.hpp"
#include "ui/reshack/palette.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace ui { namespace reshack {

    class Tool;

    /** Bitmap editor widget.
        This is the basis for pixmap and font editing. */
    class Painter : public SimpleWidget, public Root::PaletteHandler {
     public:
        /** Constructor.
            @param pixmap     Picture to edit
            @param colorMode  Color mode
            @param root       UI root */
        Painter(afl::base::Ptr<gfx::PalettizedPixmap> pixmap, Palette::ColorMode colorMode, Root& root);

        /** Destructor. */
        ~Painter();

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        // PaletteHandler:
        virtual void loadPalette(Root::PaletteLoader& ldr);
        virtual void unloadPalette();

        /** Add new auxiliary line.
            If this auxiliary line already exists, this is a no-op.
            @param n         Coordinate
            @param onXAxis   true for a vertical line (n is an X coordinate); false for a horizontal line (n is an Y coordinate) */
        void addAuxLine(int n, bool onXAxis);

        /** Remove auxiliary line.
            If this auxiliary line does not exist, this is a no-op.
            @param n         Coordinate
            @param onXAxis   true for a vertical line (n is an X coordinate); false for a horizontal line (n is an Y coordinate) */
        void removeAuxLine(int n, bool onXAxis);

        /** Set tool.
            @param tool  Tool (remains owned by caller, must live long enough; can be null) */
        void setTool(Tool* tool);

        /** Get tool.
            @return Last tool set by setTool() */
        Tool* getTool() const;

        /** Set zoom level.
            @param scale Zoom level (effective zoom is 2**scale) */
        void setZoom(int scale);

        /** Get zoom level.
            @return Last zoom level set by setZoom() */
        int getZoom() const;

        /** Set color.
            @param bg     true to set background color; false to set foreground color
            @param color  Color */
        void setColor(bool bg, uint8_t color);

        /** Get color.
            @param bg     true to get background color; false to get foreground color
            @return color */
        uint8_t getColor(bool bg) const;

        /** Get color mode.
            @return color mode as set by constructor. */
        Palette::ColorMode getColorMode() const;

        /** Get pixmap.
            @return pixmap */
        afl::base::Ptr<gfx::PalettizedPixmap> getPixmap() const;

        /** Set pixmap.
            @param pix New pixmap */
        void setPixmap(afl::base::Ptr<gfx::PalettizedPixmap> pix);

        /** Get size.
            @return size. */
        gfx::Point getSize() const;

        /** Set size.
            Adjusts the pixmap size; fills with color 0.
            @param pt Size */
        void setSize(gfx::Point pt);

        /** Signal: color changed.
            Called by setColor(). */
        afl::base::Signal<void()> sig_colorChange;

     public:
        Root& m_root;
        afl::base::Ptr<gfx::PalettizedPixmap> m_pixmap;
        afl::base::Ptr<gfx::Canvas> m_canvas;
        afl::base::GrowableMemory<uint8_t> m_backup;
        Tool* m_tool;
        gfx::BaseContext m_context;

        std::vector<int> m_auxLines[2];

        gfx::Point m_lastPosition;
        bool m_mouseIsDown;
        int m_mouseColorSlot;

        int m_scale;
        uint8_t m_colors[3];
        const Palette::ColorMode m_colorMode;

        void moveOrigin(int dx, int dy);
        gfx::Point worldToScreen(gfx::Point pt);
        gfx::Point screenToWorld(gfx::Point pt);
    };

} }

#endif
