/**
  *  \file client/dialogs/buildqueuedialog.cpp
  *  \brief Build Queue Dialog
  */

#include "client/dialogs/buildqueuedialog.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/buildqueuesummary.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/buildqueueproxy.hpp"
#include "gfx/complex.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "util/unicodechars.hpp"
#include "util/vector.hpp"

namespace {
    using client::ScreenHistory;
    using game::proxy::BuildQueueProxy;
    using ui::widgets::Button;

    /*
     *  BuildQueueList - a list box displaying the build queue
     */

    const int ICON_HEMS = 3;       // half ems
    const int ACTION_EMS = 25;
    const int FCODE_EMS = 5;
    const int QPOS_EMS = 5;
    const int POINTS_EMS = 10;
    const int GAP_PX = 5;
    const int PAD_PX = 2;

    class BuildQueueList : public ui::widgets::AbstractListbox {
     public:
        typedef BuildQueueProxy::Infos_t Infos_t;

        enum Column {
            QueuePositionColumn,
            BuildPointsColumn
        };
        typedef afl::bits::SmallSet<Column> Columns_t;


        BuildQueueList(ui::Root& root, afl::string::Translator& tx, Columns_t cols);

        void setContent(const Infos_t& data);
        void scrollToPlanet(game::Id_t planetId);

        const Infos_t& getContent() const;
        bool hasChanges() const;
        game::Id_t getCurrentPlanetId() const;

        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        int getItemHeight() const;

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        Infos_t m_data;
        Columns_t m_columns;
    };

    class BuildQueueBar : public ui::SimpleWidget {
     public:
        typedef BuildQueueProxy::Infos_t Infos_t;
        typedef BuildQueueProxy::GlobalInfo GlobalInfo_t;

        BuildQueueBar(ui::Root& root);

        void setContent(const Infos_t& content, const GlobalInfo_t& global);
        void setPlanetId(game::Id_t planetId);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        static const int IdShift = 1;
        static const int PlannedFlag = 1;

        ui::Root& m_root;
        int m_maxBases;
        int m_planetId;

        /* 0 = foreign order, otherwise: id << IdShift, optionally + PlannedFlag */
        util::Vector<int,int> m_content;

        uint8_t getColorBySlot(int slot) const;
        int getCoordinateBySlot(const gfx::Rectangle& a, int slot) const;
    };

    class BuildQueueKeyHandler : public ui::InvisibleWidget {
     public:
        BuildQueueKeyHandler(BuildQueueProxy& proxy, BuildQueueList& list);
        bool handleKey(util::Key_t key, int prefix);

     private:
        BuildQueueProxy& m_proxy;
        BuildQueueList& m_list;
    };

    class BuildQueueDialog {
     public:
        typedef BuildQueueProxy::Infos_t Infos_t;

        BuildQueueDialog(ui::Root& root, afl::string::Translator& tx, BuildQueueProxy& proxy, BuildQueueList::Columns_t cols, util::RequestSender<game::Session> gameSender)
            : m_root(root),
              m_list(root, tx, cols),
              m_bar(root),
              m_loop(root),
              m_translator(tx),
              m_proxy(proxy),
              m_gameSender(gameSender),
              m_reference(),
              m_wantBar(cols.contains(BuildQueueList::QueuePositionColumn))
            {
                proxy.sig_update.add(this, &BuildQueueDialog::setListContent);
                m_list.sig_change.add(this, &BuildQueueDialog::updatePlanetId);
            }

        void setListContent(const Infos_t& data)
            {
                m_list.setContent(data);
            }

        void setContent(const Infos_t& data, const BuildQueueProxy::GlobalInfo& global)
            {
                m_list.setContent(data);
                m_bar.setContent(data, global);
            }

        void scrollToPlanet(game::Id_t planetId)
            {
                m_list.scrollToPlanet(planetId);
                updatePlanetId();
            }

        void updatePlanetId()
            {
                if (m_wantBar) {
                    m_bar.setPlanetId(m_list.getCurrentPlanetId());
                }
            }

        void run()
            {
                afl::base::Deleter del;
                ui::Window win(m_translator("Manage Build Queue"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root))));
                if (m_wantBar) {
                    win.add(m_bar);
                }

                ui::Widget& keys = del.addNew(new BuildQueueKeyHandler(m_proxy, m_list));
                win.add(keys);

                ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
                Button& btnFaster = del.addNew(new Button("+", '+', m_root));
                btnFaster.dispatchKeyTo(keys);
                g.add(btnFaster);
                g.add(del.addNew(new ui::widgets::StaticText(m_translator("Build earlier"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));

                Button& btnSlower = del.addNew(new Button("-", '-', m_root));
                btnSlower.dispatchKeyTo(keys);
                g.add(btnSlower);
                g.add(del.addNew(new ui::widgets::StaticText(m_translator("Build later"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                g.add(del.addNew(new ui::Spacer()));

                Button& btnGoto = del.addNew(new Button(m_translator("Go to"), 'g', m_root));
                g.add(btnGoto);
                btnGoto.sig_fire.add(this, &BuildQueueDialog::onGoto);

                Button& btnSummary = del.addNew(new Button(m_translator("Summary..."), 's', m_root));
                g.add(btnSummary);
                btnSummary.sig_fire.add(this, &BuildQueueDialog::onSummary);

                win.add(g);

                ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:queuemanager"));
                ui::widgets::StandardDialogButtons& btns = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
                btns.addHelp(help);
                btns.addStop(m_loop);
                win.add(btns);
                win.add(help);
                win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                if (m_loop.run() != 0) {
                    m_proxy.commit();
                }
            }

        void onSummary()
            {
                client::dialogs::doBuildQueueSummaryDialog(m_list.getContent(), m_root, m_gameSender, m_translator);
            }

        ScreenHistory::Reference getReference() const
            { return m_reference; }

        void onGoto()
            {
                // Fail-safe
                game::Id_t id = m_list.getCurrentPlanetId();
                if (id == 0) {
                    return;
                }

                // Ask for confirmation
                enum { Yes, No, Cancel };
                int mode;
                if (m_list.hasChanges()) {
                    mode = ui::dialogs::MessageBox(m_translator("Apply changes?"), m_translator("Manage Build Queue"), m_root)
                        .addButton(Yes,    util::KeyString(m_translator("Yes")))
                        .addButton(No,     util::KeyString(m_translator("No")))
                        .addButton(Cancel, m_translator("Cancel"), util::Key_Escape)
                        .run();
                } else {
                    mode = Yes;
                }

                // Do it
                if (mode != Cancel) {
                    m_reference = ScreenHistory::Reference(ScreenHistory::Starbase, id, 0);
                    m_loop.stop(mode == Yes);
                }
            }

     private:
        ui::Root& m_root;
        BuildQueueList m_list;
        BuildQueueBar m_bar;
        ui::EventLoop m_loop;
        afl::string::Translator& m_translator;
        BuildQueueProxy& m_proxy;
        util::RequestSender<game::Session> m_gameSender;
        ScreenHistory::Reference m_reference;
        bool m_wantBar;
    };

}

/*
 *  BuildQueueList
 */

inline
BuildQueueList::BuildQueueList(ui::Root& root, afl::string::Translator& tx, Columns_t cols)
    : m_root(root),
      m_translator(tx),
      m_data(),
      m_columns(cols)
{ }

void
BuildQueueList::setContent(const Infos_t& data)
{
    // Remember current Id
    game::Id_t currentId = getCurrentPlanetId();

    // Update
    m_data = data;
    requestRedraw();
    handleModelChange();

    // Select current Id
    if (currentId != 0) {
        scrollToPlanet(currentId);
    }
}

void
BuildQueueList::scrollToPlanet(game::Id_t planetId)
{
    for (size_t i = 0, n = m_data.size(); i < n; ++i) {
        if (m_data[i].planetId == planetId) {
            setCurrentItem(i);
            break;
        }
    }
}

const BuildQueueList::Infos_t&
BuildQueueList::getContent() const
{
    return m_data;
}

bool
BuildQueueList::hasChanges() const
{
    for (size_t i = 0, n = m_data.size(); i < n; ++i) {
        if (m_data[i].isChange) {
            return true;
        }
    }
    return false;
}

game::Id_t
BuildQueueList::getCurrentPlanetId() const
{
    game::Id_t currentId = 0;
    size_t currentItem = getCurrentItem();
    if (currentItem < m_data.size()) {
        currentId = m_data[currentItem].planetId;
    }
    return currentId;
}

size_t
BuildQueueList::getNumItems() const
{
    return m_data.size();
}

bool
BuildQueueList::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
BuildQueueList::getItemHeight(size_t /*n*/) const
{
    return getItemHeight();
}

int
BuildQueueList::getHeaderHeight() const
{
    return m_root.provider().getFont(gfx::FontRequest())->getLineHeight();
}

int
BuildQueueList::getFooterHeight() const
{
    return 0;
}

void
BuildQueueList::drawHeader(gfx::Canvas& can, gfx::Rectangle area)
{
    afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont(gfx::FontRequest());
    const int em = normalFont->getEmWidth();

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.setColor(util::SkinColor::Static);
    ctx.useFont(*normalFont);

    drawHLine(ctx, area.getLeftX(), area.getBottomY()-1, area.getRightX()-1);

    area.consumeX(GAP_PX + ICON_HEMS*em/2);
    outTextF(ctx, area.splitX(ACTION_EMS * em + GAP_PX), m_translator("Build Order"));
    outTextF(ctx, area.splitX(FCODE_EMS * em + GAP_PX), m_translator("FCode"));
    if (m_columns.contains(QueuePositionColumn)) {
        outTextF(ctx, area.splitX(QPOS_EMS * em + GAP_PX), m_translator("Q-Pos"));
    }
    if (m_columns.contains(BuildPointsColumn)) {
        outTextF(ctx, area, m_translator("Build Points"));
    }
}

void
BuildQueueList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
BuildQueueList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // Prepare
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    // Draw
    afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont(gfx::FontRequest());
    afl::base::Ref<gfx::Font> boldFont = m_root.provider().getFont(gfx::FontRequest().addWeight(1));
    afl::base::Ref<gfx::Font> smallFont = m_root.provider().getFont("-");
    if (item < m_data.size()) {
        const BuildQueueProxy::Info_t& e = m_data[item];
        const int em = normalFont->getEmWidth();

        area.consumeY(PAD_PX);
        area.consumeRightX(GAP_PX);
        ctx.useFont(*normalFont);

        // Icon
        gfx::Rectangle iconArea = area.splitX(GAP_PX + ICON_HEMS*em/2);
        ctx.setTextAlign(gfx::CenterAlign, gfx::TopAlign);
        if (e.planned) {
            ctx.setColor(util::SkinColor::Faded);
            outTextF(ctx, iconArea, UTF_STOPWATCH);
        } else {
            ctx.setColor(util::SkinColor::Green);
            outTextF(ctx, iconArea, UTF_CHECK_MARK);
        }

        // Name
        util::SkinColor::Color defColor = e.planned ? util::SkinColor::Faded : util::SkinColor::Static;
        gfx::Rectangle nameArea = area.splitX(ACTION_EMS * em);
        ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        ctx.setColor(defColor);
        outTextF(ctx, nameArea.splitY(normalFont->getLineHeight()), e.actionName);
        ctx.useFont(*smallFont);
        ctx.setColor(util::SkinColor::Faded);
        outTextF(ctx, nameArea, afl::string::Format("(%s, #%d)", e.planetName, e.planetId));
        area.consumeX(GAP_PX);

        // Friendly code
        ctx.useFont(e.hasPriority ? *boldFont : *normalFont);
        ctx.setColor(e.conflict ? util::SkinColor::Red : defColor);
        outTextF(ctx, area.splitX(FCODE_EMS * em), e.friendlyCode);
        area.consumeX(GAP_PX);

        // Queue position
        if (m_columns.contains(QueuePositionColumn)) {
            gfx::Rectangle queueArea = area.splitX(QPOS_EMS * em);
            if (e.queuePosition != 0) {
                ctx.useFont(*normalFont);
                ctx.setColor(defColor);
                ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
                outTextF(ctx, queueArea, afl::string::Format("%d", e.queuePosition));
            }
            area.consumeX(GAP_PX);
        }

        // Points
        if (m_columns.contains(BuildPointsColumn)) {
            int space = POINTS_EMS * em;
            int half = space/2 - 3;
            gfx::Rectangle needArea = area.splitX(half);
            gfx::Rectangle haveArea = area.splitX(space - half);

            int32_t reqd, avail;
            if (e.pointsRequired.get(reqd)) {
                ctx.useFont(*normalFont);
                if (e.pointsAvailable.get(avail)) {
                    ctx.setColor(defColor);
                    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
                    outTextF(ctx, haveArea, afl::string::Format(" / %d", avail));

                    ctx.setColor(e.hasPriority ? reqd > avail ? util::SkinColor::Red : defColor : util::SkinColor::Faded);
                } else {
                    ctx.setColor(defColor);
                }
                ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
                outTextF(ctx, needArea, afl::string::Format("%d", reqd));
            }
        }
    }
}

void
BuildQueueList::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
BuildQueueList::getLayoutInfo() const
{
    int em = m_root.provider().getFont(gfx::FontRequest())->getEmWidth();
    int extraSize = 0;
    if (m_columns.contains(QueuePositionColumn)) {
        extraSize += em * QPOS_EMS + GAP_PX;
    }
    if (m_columns.contains(BuildPointsColumn)) {
        extraSize += em * POINTS_EMS + GAP_PX;
    }

    gfx::Point size(em * (ACTION_EMS + FCODE_EMS) + 4*GAP_PX + extraSize, getItemHeight() * 15);
    return ui::layout::Info(size, ui::layout::Info::GrowBoth);
}

bool
BuildQueueList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

int
BuildQueueList::getItemHeight() const
{
    return m_root.provider().getFont(gfx::FontRequest())->getLineHeight()
        + m_root.provider().getFont("-")->getLineHeight()
        + 2*PAD_PX;
}


/*
 *  BuildQueueBar
 */

BuildQueueBar::BuildQueueBar(ui::Root& root)
    : m_root(root),
      m_maxBases(),
      m_planetId(),
      m_content(1)
{ }

void
BuildQueueBar::setContent(const Infos_t& content, const GlobalInfo_t& global)
{
    int maxPos = 0;

    // Queued orders
    for (size_t i = 0, n = content.size(); i < n; ++i) {
        if (int thisPos = content[i].queuePosition) {
            maxPos = std::max(maxPos, thisPos);
            m_content.set(thisPos, content[i].planetId << IdShift);
        }
    }

    // Planned orders (and orders submitted this turn)
    for (size_t i = 0, n = content.size(); i < n; ++i) {
        if (content[i].queuePosition == 0) {
            ++maxPos;
            m_content.set(maxPos, (content[i].planetId << IdShift) | PlannedFlag);
        }
    }

    // Total size
    m_maxBases = std::max(maxPos, global.totalBases);
}

void
BuildQueueBar::setPlanetId(game::Id_t planetId)
{
    if (m_planetId != planetId) {
        m_planetId = planetId;
        requestRedraw();
    }
}

void
BuildQueueBar::draw(gfx::Canvas& can)
{
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    gfx::Rectangle area = getExtent();

    // Outer frame
    ui::drawFrameDown(ctx, area);
    area.grow(-1, -1);

    // Content
    if (m_maxBases > 0) {
        int pos = 1;
        while (pos <= m_maxBases) {
            int firstPos = pos;
            uint8_t color = getColorBySlot(pos);
            ++pos;
            while (pos <= m_maxBases && getColorBySlot(pos) == color) {
                ++pos;
            }
            int x1 = getCoordinateBySlot(area, firstPos);
            int x2 = getCoordinateBySlot(area, pos);
            drawSolidBar(ctx, gfx::Rectangle(area.getLeftX() + x1, area.getTopY(), x2-x1, area.getHeight()), color);
        }
    } else {
        drawSolidBar(ctx, area, ui::Color_Gray);
    }
}

void
BuildQueueBar::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
BuildQueueBar::handlePositionChange()
{
    requestRedraw();
}

ui::layout::Info
BuildQueueBar::getLayoutInfo() const
{
    return ui::layout::Info(gfx::Point(100, 14), ui::layout::Info::GrowHorizontal);
}

bool
BuildQueueBar::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
BuildQueueBar::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}

uint8_t
BuildQueueBar::getColorBySlot(int slot) const
{
    int value = m_content.get(slot);
    if (value != 0) {
        if ((value >> IdShift) == m_planetId) {
            return ui::Color_Yellow;
        } else if ((value & PlannedFlag) != 0) {
            return ui::Color_GreenBlack;
        } else {
            return ui::Color_Green;
        }
    } else {
        return ui::Color_Grayscale+3;
    }
}

int
BuildQueueBar::getCoordinateBySlot(const gfx::Rectangle& a, int slot) const
{
    return a.getWidth() * (slot-1) / m_maxBases;
}


/*
 *  BuildQueueKeyHandler - special keys for the dialog
 */

inline
BuildQueueKeyHandler::BuildQueueKeyHandler(BuildQueueProxy& proxy, BuildQueueList& list)
    : m_proxy(proxy),
      m_list(list)
{ }

bool
BuildQueueKeyHandler::handleKey(util::Key_t key, int prefix)
{
    if (key == '-' || key == util::Key_Down + util::KeyMod_Shift) {
        m_proxy.decreasePriority(m_list.getCurrentItem());
        return true;
    } else if (key == '+' || key == util::Key_Up + util::KeyMod_Shift) {
        m_proxy.increasePriority(m_list.getCurrentItem());
        return true;
    } else if (key >= '0' && key <= '9') {
        m_proxy.setPriority(m_list.getCurrentItem(), key - '0');
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}


/*
 *  Main entry point
 */

client::ScreenHistory::Reference
client::dialogs::doBuildQueueDialog(game::Id_t baseId,
                                    ui::Root& root,
                                    util::RequestSender<game::Session> gameSender,
                                    afl::string::Translator& tx)
{
    // Set up proxy
    BuildQueueProxy proxy(gameSender, root.engine().dispatcher());
    BuildQueueProxy::Infos_t infos;
    BuildQueueProxy::GlobalInfo global;
    Downlink link(root, tx);
    proxy.getStatus(link, infos, global);
    if (infos.empty()) {
        ui::dialogs::MessageBox(tx("You have no active ship build orders."),
                                tx("Manage Build Queue"),
                                root).doOkDialog(tx);
        return ScreenHistory::Reference();
    }

    // Column configuration
    BuildQueueList::Columns_t cols;
    for (size_t i = 0, n = infos.size(); i < n; ++i) {
        if (infos[i].queuePosition != 0) {
            cols += BuildQueueList::QueuePositionColumn;
        }
        if (infos[i].pointsRequired.isValid()) {
            cols += BuildQueueList::BuildPointsColumn;
        }
    }

    // Set up dialog
    BuildQueueDialog dlg(root, tx, proxy, cols, gameSender);
    dlg.setContent(infos, global);
    dlg.scrollToPlanet(baseId);
    dlg.run();

    return dlg.getReference();
}
