/**
  *  \file client/dialogs/attachmentselection.cpp
  *  \brief Attachment Selection Dialog
  */

#include "client/dialogs/attachmentselection.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/attachmentproxy.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/res/resid.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"


namespace {
    /*
     *  Layout Parameters
     */

    const int CB_SIZE = 24;

    afl::base::Ref<gfx::Font> getFirstFont(ui::Root& root)
    {
        return root.provider().getFont(gfx::FontRequest());
    }

    afl::base::Ref<gfx::Font> getSecondFont(ui::Root& root)
    {
        return root.provider().getFont("-");
    }

    int getWidth(ui::Root& root)
    {
        return getFirstFont(root)->getEmWidth() * 20;
    }

    afl::base::Ptr<gfx::Canvas> getImage(ui::Root& root, bool selected)
    {
        return root.provider().getImage(selected ? RESOURCE_ID("ui.cb1") : RESOURCE_ID("ui.cb0"));
    }


    /*
     *  List of Attachments
     */

    class AttachmentList : public ui::widgets::AbstractListbox {
     public:
        AttachmentList(game::proxy::AttachmentProxy::Infos_t& infos, ui::Root& root, afl::string::Translator& tx);

        // AbstractListbox:
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
        game::proxy::AttachmentProxy::Infos_t& m_infos;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        afl::base::SignalConnection conn_imageChange;

        void toggleSelection();
        void onItemClickAt(size_t item, gfx::Point pt);
        void onImageChange();
    };


    /*
     *  Dialog
     */

    class AttachmentDialog {
     public:
        AttachmentDialog(game::proxy::AttachmentProxy::Infos_t& infos, ui::Root& root, afl::string::Translator& tx);
        bool run(util::RequestSender<game::Session>& gameSender);

     private:
        AttachmentList m_list;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
    };
}

/*
 *  AttachmentList
 */

AttachmentList::AttachmentList(game::proxy::AttachmentProxy::Infos_t& infos, ui::Root& root, afl::string::Translator& tx)
    : AbstractListbox(),
      m_infos(infos),
      m_root(root),
      m_translator(tx),
      conn_imageChange(root.provider().sig_imageChange.add(this, &AttachmentList::onImageChange))
{
    // Preload
    getImage(m_root, false);
    getImage(m_root, true);

    // Internal event dispatch
    sig_itemClickAt.add(this, &AttachmentList::onItemClickAt);
    sig_itemDoubleClick.add(this, &AttachmentList::toggleSelection);
}

size_t
AttachmentList::getNumItems() const
{
    return m_infos.size();
}

bool
AttachmentList::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
AttachmentList::getItemHeight(size_t /*n*/) const
{
    return std::max(getFirstFont(m_root)->getLineHeight() + getSecondFont(m_root)->getLineHeight(), CB_SIZE);
}

int
AttachmentList::getHeaderHeight() const
{
    return 0;
}

int
AttachmentList::getFooterHeight() const
{
    return 0;
}

void
AttachmentList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
AttachmentList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
AttachmentList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex AttachmentList::drawPart(GfxCanvas& can, int from, int to)
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());

    afl::base::Deleter del;
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    if (item < m_infos.size()) {
        const game::proxy::AttachmentProxy::Info* p = &m_infos[item];

        // Checkbox
        gfx::Rectangle cbArea = area.splitX(CB_SIZE);
        afl::base::Ptr<gfx::Canvas> cb = getImage(m_root, p->selected);
        if (cb.get() != 0) {
            gfx::Rectangle cbSize(gfx::Point(0, 0), cb->getSize());
            cbSize.centerWithin(cbArea);
            blitSized(ctx, cbSize, *cb);
        }

        // Name & Information
        int availableHeight = area.getHeight();
        int neededHeight = getItemHeight(item);
        if (availableHeight > neededHeight) {
            area.consumeY((availableHeight - neededHeight) / 2);
        }

        ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        ctx.setColor(p->critical ? util::SkinColor::Red : util::SkinColor::Static);
        ctx.useFont(*getFirstFont(m_root));
        outTextF(ctx, area.splitY(ctx.getFont()->getLineHeight()), p->fileName);
        ctx.useFont(*getSecondFont(m_root));
        outTextF(ctx, area, afl::string::Format(m_translator("(%s, %d byte%!1{s%})"), p->kindName, p->size));
    }
}

void
AttachmentList::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
AttachmentList::getLayoutInfo() const
{
    gfx::Point size(getWidth(m_root), getItemHeight(0));
    return ui::layout::Info(size, ui::layout::Info::GrowBoth);
}

bool
AttachmentList::handleKey(util::Key_t key, int prefix)
{
    if (hasState(FocusedState) && !hasState(DisabledState)) {
        // ex AttachmentKeyHandler::handleEvent
        if (key == ' ') {
            requestActive();
            toggleSelection();
            return true;
        }
    }
    return defaultHandleKey(key, prefix);
}

void
AttachmentList::toggleSelection()
{
    size_t pos = getCurrentItem();
    if (pos < m_infos.size()) {
        m_infos[pos].selected = !m_infos[pos].selected;
        updateCurrentItem();
    }
}

void
AttachmentList::onItemClickAt(size_t /*item*/, gfx::Point pt)
{
    // ex AttachmentList::onItemClick
    if (pt.getX() < CB_SIZE) {
        toggleSelection();
    }
}

void
AttachmentList::onImageChange()
{
    requestRedraw();
}


/*
 *  AttachmentDialog
 */

AttachmentDialog::AttachmentDialog(game::proxy::AttachmentProxy::Infos_t& infos, ui::Root& root, afl::string::Translator& tx)
    : m_list(infos, root, tx),
      m_root(root),
      m_translator(tx)
{ }

bool
AttachmentDialog::run(util::RequestSender<game::Session>& gameSender)
{
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Attachments"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    const int width = getWidth(m_root);
    ui::rich::DocumentView& doc = del.addNew(new ui::rich::DocumentView(gfx::Point(width, 1), 0, m_root.provider()));
    doc.getDocument().setPageWidth(width);
    doc.getDocument().add(m_translator("You have received some additional files with your result. Choose which files you want to accept:"));
    doc.getDocument().finish();
    doc.adjustToDocumentSize();
    doc.setState(ui::Widget::DisabledState, true);
    win.add(doc);

    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root))));

    ui::EventLoop loop(m_root);
    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, gameSender, "pcc2:resultattachments"));
    btn.addStop(loop);
    btn.addHelp(help);
    win.add(btn);
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    return loop.run() != 0;
}


/*
 *  Main Entry Point
 */

bool
client::dialogs::chooseAttachments(game::proxy::AttachmentProxy::Infos_t& infos, util::RequestSender<game::Session> gameSender, ui::Root& root, afl::string::Translator& tx)
{
    // ex chooseAttachments(GAttachmentUnpacker& unp)
    if (!infos.empty()) {
        return AttachmentDialog(infos, root, tx).run(gameSender);
    } else {
        return false;
    }
}
