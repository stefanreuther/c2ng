/**
  *  \file client/widgets/alliancelevelgrid.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_ALLIANCELEVELGRID_HPP
#define C2NG_CLIENT_WIDGETS_ALLIANCELEVELGRID_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/alliance/offer.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace client { namespace widgets {

    class AllianceLevelGrid : public ui::SimpleWidget {
     public:
        typedef game::alliance::Offer::Type OfferType_t;

        AllianceLevelGrid(ui::Root& root, afl::string::Translator& tx);

        void add(size_t ref, String_t name);
        void setOffer(size_t ref, OfferType_t theirOffer, OfferType_t ourOffer);

        void setPosition(size_t index);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        afl::base::Signal<void(size_t)> sig_toggleOffer;

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        
        struct Item {
            size_t ref;
            String_t name;
            OfferType_t theirOffer;
            OfferType_t ourOffer;
            Item(size_t ref, String_t name)
                : ref(ref), name(name), theirOffer(), ourOffer()
                { }
        };
        std::vector<Item> m_items;
        
        size_t m_position;
        bool m_mouseDown;

        void drawCheckbox(gfx::Context<util::SkinColor::Color>& ctx, int x, int y, OfferType_t offer, int gridSize, bool focused);
        void toggleCurrent();
    };

} }

#endif
