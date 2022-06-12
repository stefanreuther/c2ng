/**
  *  \file client/tiles/selectionheadertile.cpp
  */

#include "client/tiles/selectionheadertile.hpp"
#include "util/unicodechars.hpp"
#include "client/marker.hpp"
#include "game/proxy/objectlistener.hpp"

client::tiles::SelectionHeaderTile::SelectionHeaderTile(ui::Root& root, gfx::KeyEventConsumer& keys)
    : m_root(root),
      m_name(),
      m_marked(),
      m_receiver(root.engine().dispatcher(), *this),
      // These need to be Key_Up, Key_Down; this widget appears together with +/- buttons on the Minefield Info dialog
      m_prev(UTF_UP_ARROW,   util::Key_Up,   root),
      m_next(UTF_DOWN_ARROW, util::Key_Down, root)
{
    // ex WObjectSelectionHeaderTile::WObjectSelectionHeaderTile
    // ex WObjectSelectionHeaderTile::init
    m_next.dispatchKeyTo(keys);
    m_prev.dispatchKeyTo(keys);
    addChild(m_prev, 0);
    addChild(m_next, 0);
    m_prev.setFont(gfx::FontRequest().addSize(-1));
    m_next.setFont(gfx::FontRequest().addSize(-1));

    setState(DisabledState, true); // FIXME: disable so it doesn't get focus - should we have a FocusableState instead?
}

client::tiles::SelectionHeaderTile::~SelectionHeaderTile()
{ }

void
client::tiles::SelectionHeaderTile::draw(gfx::Canvas& can)
{
    // ex WObjectSelectionHeaderTile::drawData
    defaultDrawChildren(can);

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addWeight(1)));

    gfx::Rectangle r = getExtent();
    r.setWidth(r.getWidth() - 2*r.getHeight() - 1); // room for buttons
    if (m_marked) {
        ctx.setColor(util::SkinColor::Selection);
        client::drawSelection(ctx, r.getTopLeft() + gfx::Point(5, 7), 1, 2);
        r.consumeX(15);
    }
    ctx.setColor(util::SkinColor::Static);
    outTextF(ctx, r, m_name);
}

void
client::tiles::SelectionHeaderTile::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::tiles::SelectionHeaderTile::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::tiles::SelectionHeaderTile::handleChildAdded(Widget& /*child*/)
{ }

void
client::tiles::SelectionHeaderTile::handleChildRemove(Widget& /*child*/)
{ }

void
client::tiles::SelectionHeaderTile::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    const int h = getExtent().getHeight();
    gfx::Rectangle r(getExtent().getRightX(), getExtent().getTopY(), h, h);
    r.moveBy(gfx::Point(-h, 0));
    m_next.setExtent(r);
    r.moveBy(gfx::Point(-h-1, 0));
    m_prev.setExtent(r);
}

void
client::tiles::SelectionHeaderTile::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::tiles::SelectionHeaderTile::getLayoutInfo() const
{
    // ex WObjectSelectionHeaderTile::getLayoutInfo
    gfx::Point size = m_root.provider().getFont(gfx::FontRequest().addWeight(1))->getCellSize().scaledBy(20, 1);
    return ui::layout::Info(size, size, ui::layout::Info::GrowHorizontal);
}

bool
client::tiles::SelectionHeaderTile::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::tiles::SelectionHeaderTile::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::tiles::SelectionHeaderTile::setStatus(String_t name, bool marked)
{
    m_name = name;
    m_marked = marked;
    requestRedraw();
}

void
client::tiles::SelectionHeaderTile::attach(game::proxy::ObjectObserver& oop)
{
    class Job : public util::Request<SelectionHeaderTile> {
     public:
        Job(String_t name, bool marked)
            : m_name(name), m_marked(marked)
            { }
        void handle(SelectionHeaderTile& t)
            { t.setStatus(m_name, m_marked); }
     private:
        String_t m_name;
        bool m_marked;
    };
    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<SelectionHeaderTile> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& s, game::map::Object* obj)
            {
                String_t name;
                bool marked = false;
                if (obj) {
                    marked = obj->isMarked();
                    name = obj->getName(game::LongName,
                                        s.translator(),
                                        s.interface());
                }
                m_reply.postNewRequest(new Job(name, marked));
            }
     private:
        util::RequestSender<SelectionHeaderTile> m_reply;
    };

    oop.addNewListener(new Listener(m_receiver.getSender()));
}
