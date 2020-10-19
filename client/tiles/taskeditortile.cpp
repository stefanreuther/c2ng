/**
  *  \file client/tiles/taskeditortile.cpp
  */

#include "client/tiles/taskeditortile.hpp"
#include "game/proxy/objectlistener.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/skincolorscheme.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/framegroup.hpp"

class client::tiles::TaskEditorTile::ListWidget : public ui::widgets::AbstractListbox {
 public:
    ListWidget(gfx::ResourceProvider& provider, ui::ColorScheme& scheme)
        : AbstractListbox(),
          m_status(),
          m_provider(provider),
          m_colorScheme(scheme),
          m_internalColorScheme(ui::GRAY_COLOR_SET, scheme)
        {
            // Internal color scheme is used to fill the unused bottom of the task
            setColorScheme(m_internalColorScheme);
            setFlag(NoPageKeys, true);
            setFlag(EqualSizes, true);
        }

    ~ListWidget()
        { }

    // AbstractListbox:
    virtual size_t getNumItems()
        {
            // +1 for blank item at end, +2 for divider line
            return m_status.commands.size() + 2;
        }
    virtual bool isItemAccessible(size_t n)
        { return n <= m_status.commands.size(); }
    virtual int getItemHeight(size_t /*n*/)
        { return m_provider.getFont(gfx::FontRequest())->getLineHeight(); }
    virtual int getHeaderHeight()
        { return 0; }
    virtual void drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
        { }
    virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
        {
            // ex WAutoTaskEditorList::drawPart
            gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
            ctx.useFont(*m_provider.getFont(gfx::FontRequest()));

            afl::base::Deleter del;
            ui::prepareColorListItem(ctx, area, state, m_colorScheme, del);

            // Draw content
            size_t numLines = m_status.commands.size();
            if (item < numLines) {
                // Cursor
                int cursorSize = getItemHeight(0);
                gfx::Rectangle cursorArea = area.splitX(cursorSize);
                if (item == m_status.pc) {
                    int y = cursorArea.getTopY() + cursorArea.getHeight()/2 - 1;
                    int steps = 5;
                    int xl = cursorArea.getLeftX() + 3;
                    ctx.setColor(ui::SkinColor::Red);
                    if (m_status.isInSubroutineCall) {
                        // Hollow triangle
                        for (int dy = 0; dy < steps; ++dy) {
                            int y1 = y + dy;
                            int y2 = y - dy;
                            drawPixel(ctx, gfx::Point(xl, y1));
                            drawPixel(ctx, gfx::Point(xl + 2*(steps-dy) - 1, y1));
                            drawPixel(ctx, gfx::Point(xl + 2*(steps-dy) - 2, y1));
                            if (y1 != y2) {
                                drawPixel(ctx, gfx::Point(xl, y2));
                                drawPixel(ctx, gfx::Point(xl + 2*(steps-dy) - 1, y2));
                                drawPixel(ctx, gfx::Point(xl + 2*(steps-dy) - 2, y2));
                            }
                        }
                    } else {
                        // Solid triangle
                        for (int dy = 0; dy < steps; ++dy) {
                            int y1 = y + dy;
                            int y2 = y - dy;
                            drawHLine(ctx, xl, y1, xl + 2*(steps-dy) - 1);
                            if (y1 != y2) {
                                drawHLine(ctx, xl, y2, xl + 2*(steps-dy) - 1);
                            }
                        }
                    }
                }

                // Program line
                const String_t& cmd = m_status.commands[item];
                if (!cmd.empty() && cmd[0] == '%') {
                    ctx.setColor(util::SkinColor::Faded);
                } else {
                    ctx.setColor(util::SkinColor::Static);
                }
                outTextF(ctx, area, cmd);
            }
            if (item == numLines + 1) {
                // Divider at end
                ctx.setColor(util::SkinColor::Faded);
                gfx::drawHLine(ctx, area.getLeftX(), area.getTopY(), area.getRightX() - 1);
            }
        }

    // Widget:
    virtual void handlePositionChange(gfx::Rectangle& oldPosition)
        { defaultHandlePositionChange(oldPosition); }
    virtual ui::layout::Info getLayoutInfo() const
        {
            gfx::Point cellSize = m_provider.getFont(gfx::FontRequest())->getCellSize();
            return ui::layout::Info(cellSize.scaledBy(20, 5),
                                    cellSize.scaledBy(30, 18),
                                    ui::layout::Info::GrowBoth);
        }

    virtual bool handleKey(util::Key_t key, int prefix)
        { return defaultHandleKey(key, prefix); }

    // ListWidget:
    void setContent(const game::proxy::TaskEditorProxy::Status& status)
        {
            // ex WAutoTaskEditorList::update
            bool majorChange = (m_status.valid != status.valid
                                || m_status.commands != status.commands);
            bool minorChange = (m_status.pc != status.pc
                                || m_status.isInSubroutineCall != status.isInSubroutineCall);

            m_status = status;

            // FIXME: deal with cursors
            // if (new_task) {
            //     /* Entirely new task */
            //     setTop(0);
            if (getCurrentItem() != status.cursor) {
                setCurrentItem(status.cursor);
            }
            // }
            if (majorChange) {
                handleModelChange();
            } else if (minorChange) {
                requestRedraw();
            } else {
                // No change
            }
        }

 private:
    game::proxy::TaskEditorProxy::Status m_status;
    gfx::ResourceProvider& m_provider;
    ui::ColorScheme& m_colorScheme;
    ui::SkinColorScheme m_internalColorScheme;
};





client::tiles::TaskEditorTile::TaskEditorTile(ui::Root& root,
                                              client::si::UserSide& userSide,
                                              interpreter::Process::ProcessKind kind)
    : Widget(),
      m_deleter(),
      m_proxy(userSide.gameSender(), root.engine().dispatcher()),
      m_receiver(root.engine().dispatcher(), *this),
      m_kind(kind),
      m_listWidget(0),
      m_childWidget(0)
{
    // ex WAutoTaskEditorTile::WAutoTaskEditorTile
    m_listWidget = &m_deleter.addNew(new ListWidget(root.provider(), root.colorScheme()));
    m_childWidget = &ui::widgets::FrameGroup::wrapWidget(m_deleter, root.colorScheme(), ui::LoweredFrame, *m_listWidget);
    addChild(*m_childWidget, 0);
    m_proxy.sig_change.add(this, &TaskEditorTile::onChange);
    m_listWidget->requestFocus();
    m_listWidget->sig_change.add(this, &TaskEditorTile::onListSelectionChange);
}

client::tiles::TaskEditorTile::~TaskEditorTile()
{
}

void
client::tiles::TaskEditorTile::draw(gfx::Canvas& can)
{
    defaultDrawChildren(can);
}

void
client::tiles::TaskEditorTile::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::tiles::TaskEditorTile::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::tiles::TaskEditorTile::handleChildAdded(Widget& /*child*/)
{ }

void
client::tiles::TaskEditorTile::handleChildRemove(Widget& /*child*/)
{ }

void
client::tiles::TaskEditorTile::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    m_childWidget->setExtent(getExtent());
}

void
client::tiles::TaskEditorTile::handleChildPositionChange(Widget& child, gfx::Rectangle& /*oldPosition*/)
{
    child.requestRedraw();
}

ui::layout::Info
client::tiles::TaskEditorTile::getLayoutInfo() const
{
    return m_childWidget->getLayoutInfo();
}

bool
client::tiles::TaskEditorTile::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::tiles::TaskEditorTile::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::tiles::TaskEditorTile::setId(game::Id_t id)
{
    // ex WAutoTaskObjectSelection::loadTask, WAutoTaskObjectSelection::onCurrentChanged (sort-of)
    m_proxy.selectTask(id, m_kind, true);
}

void
client::tiles::TaskEditorTile::attach(game::proxy::ObjectObserver& oop)
{
    class Job : public util::Request<TaskEditorTile> {
     public:
        Job(game::map::Object* obj)
            : m_id(obj != 0 ? obj->getId() : 0)
            { }
        void handle(TaskEditorTile& t)
            { t.setId(m_id); }
     private:
        game::Id_t m_id;
    };
    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<TaskEditorTile> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& /*s*/, game::map::Object* obj)
            { m_reply.postNewRequest(new Job(obj)); }
     private:
        util::RequestSender<TaskEditorTile> m_reply;
    };

    oop.addNewListener(new Listener(m_receiver.getSender()));
}

void
client::tiles::TaskEditorTile::onChange(const game::proxy::TaskEditorProxy::Status& status)
{
    m_listWidget->setContent(status);
}

void
client::tiles::TaskEditorTile::onListSelectionChange()
{
    m_proxy.setCursor(m_listWidget->getCurrentItem());
}
