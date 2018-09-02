/**
  *  \file ui/widgets/richlistbox.hpp
  */
#ifndef C2NG_UI_WIDGETS_RICHLISTBOX_HPP
#define C2NG_UI_WIDGETS_RICHLISTBOX_HPP

#include <vector>
#include "ui/widgets/abstractlistbox.hpp"
#include "afl/base/ptr.hpp"
#include "gfx/canvas.hpp"
#include "ui/rich/document.hpp"
#include "afl/container/ptrvector.hpp"
#include "ui/colorscheme.hpp"
#include "afl/bits/smallset.hpp"

namespace ui { namespace widgets {

    class RichListbox : public AbstractListbox {
     public:
        enum RenderFlag {
            UseBackgroundColorScheme,
            DisableWrap,
            NoShade
        };
        typedef afl::bits::SmallSet<RenderFlag> RenderFlagSet_t;

        RichListbox(gfx::ResourceProvider& provider, ui::ColorScheme& scheme);
        ~RichListbox();

        void clear();
        void addItem(const util::rich::Text text, afl::base::Ptr<gfx::Canvas> image, bool accessible);
        void setItemAccessible(size_t n, bool accessible);

        void setPreferredWidth(int width);

        void setRenderFlag(RenderFlag flag, bool value);
        bool hasRenderFlag(RenderFlag flag) const;

        // AbstractListbox:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight();
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        virtual void handlePositionChange(gfx::Rectangle& /*oldPosition*/);

        // Widget:
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        struct Item {
            bool accessible;
            util::rich::Text text;
            ui::rich::Document doc;
            afl::base::Ptr<gfx::Canvas> image;

            Item(const util::rich::Text text, afl::base::Ptr<gfx::Canvas> image, bool accessible, gfx::ResourceProvider& provider);
        };

        gfx::ResourceProvider& m_provider;
        ui::ColorScheme& m_colorScheme;
        afl::container::PtrVector<Item> m_items;
        RenderFlagSet_t m_renderFlags;
        int m_preferredWidth;

        void render(size_t pos, size_t n);
    };

} }

#endif
