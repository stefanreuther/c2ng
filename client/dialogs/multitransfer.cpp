/**
  *  \file client/dialogs/multitransfer.cpp
  *  \brief Multi-Ship Cargo Transfer
  *
  *  In PCC2, this was a comparatively straightforward port of PCC 1.x's transfer.pas::DistributeCargo.
  *  In c2ng, the split of responsibility is slightly different; most of the logic is now in the UI-independant CargoTransfer class.
  *
  *  Multi-unit cargo transfer can transfer one type of cargo between multiple units.
  *  To support that, there are the following specialties:
  *  - hold space. Users unload to hold space, load from hold space.
  *    In PCC 1.x, hold space ist just an integer; here, it is a HoldSpace object that normally takes part in cargo transfer.
  *    MultiTransferSetup places hold space always at index 0 in the CargoTransfer action.
  *  - temporary space.
  *    Users can add up to five of these. They are, too, implemented as HoldSpace objects.
  *  - tagged object (extension).
  *    Essentially, acts as an extension to hold space; stuff taken when hold is empty is taken from here.
  *
  *  Transfer can be confirmed only when all HoldSpace (i.e. hold and temporary) are empty; this is verified by CargoTransfer.
  *  In addition, we allow confirmation if hold space is not empty, but can be unloaded to the tagged object.
  */

#include "client/dialogs/multitransfer.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/cargotransferproxy.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "gfx/complex.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/draw.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/numberformatter.hpp"
#include "util/string.hpp"
#include "util/unicodechars.hpp"
#include "util/updater.hpp"
#include "util/vector.hpp"

using afl::string::Format;
using game::Element;
using game::actions::CargoTransfer;
using game::actions::MultiTransferSetup;
using game::proxy::CargoTransferProxy;
using ui::dialogs::MessageBox;
using util::NumberFormatter;
using util::SkinColor;
using util::Updater;

namespace {
    /*
     *  Constants
     */

    // Dimensions of MultiTransferList
    const int FreeWidth_ems = 6;
    const int HaveWidth_ems = 6;
    const int UnitWidth_ems = 15;
    const int RightMargin_px = 5;

    // Maximum number of temporary objects users can create
    const size_t MAX_TEMPORARIES = 5;


    /*
     *  Classes
     */

    /* List box containing all participating units */
    class MultiTransferList : public ui::widgets::AbstractListbox {
     public:
        MultiTransferList(ui::Root& root, afl::string::Translator& tx, NumberFormatter fmt);

        // AbstractListbox:
        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget:
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

        // MultiTransferList:
        void addItem(String_t name, int32_t have, int32_t room, bool isTemporary);
        void setItem(size_t index, int32_t have, int32_t room);
        void setItemTag(size_t index, bool tag);
        bool hasAnyUsedTemporaries() const;
        bool hasRoomForHold(int32_t holdAmount, size_t extension) const;

     private:
        // Links
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        NumberFormatter m_formatter;

        // Internal data structure
        struct Item {
            String_t name;
            int32_t have;
            int32_t room;
            bool tag;
            bool isTemporary;
            Item(String_t name, int32_t have, int32_t room, bool tag, bool isTemporary)
                : name(name), have(have), room(room), tag(tag), isTemporary(isTemporary)
                { }
        };
        std::vector<Item> m_items;

        afl::base::Ref<gfx::Font> getFont() const
            { return m_root.provider().getFont(gfx::FontRequest()); }
    };

    /* Dialog main entry point */
    class MultiTransferDialog : public gfx::KeyEventConsumer {
     public:
        MultiTransferDialog(ui::Root& root, afl::string::Translator& tx, NumberFormatter fmt, Element::Type type, util::RequestSender<game::Session> gameSender, game::proxy::WaitIndicator& ind, CargoTransferProxy& proxy);

        void run(String_t title);
        void loadMoreParticipants();
        void setInitialPosition(size_t pos);
        void setExtension(size_t ext);
        bool handleKey(util::Key_t key, int prefix);
        void move(int32_t n);
        void distribute(CargoTransfer::DistributeMode m);
        void doContextMenu(gfx::Point anchor, bool context);

        bool isValid() const;
        void updateValidity();

        // Event handlers
        void onUpdateParticipant(size_t index, const CargoTransferProxy::Cargo& cargo);
        void onMenu(gfx::Point anchor);
        void onMenuButton();
        void onOK();
        void onListScroll();

     private:
        // Widgets
        MultiTransferList m_list;
        ui::widgets::StaticText m_holdInfo;
        ui::widgets::StaticText m_unitInfo1;
        ui::widgets::StaticText m_unitInfo2;
        ui::widgets::Button m_menuButton;
        ui::widgets::StandardDialogButtons m_dialogButtons;

        // Links and Objects
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        NumberFormatter m_formatter;
        Element::Type m_type;
        util::RequestSender<game::Session> m_gameSender;
        game::proxy::WaitIndicator& m_link;
        CargoTransferProxy& m_proxy;
        ui::EventLoop m_loop;

        // State
        size_t m_numParticipants;     // Number of participants whose data we've loaded
        size_t m_numTemporaries;      // Number of temporaries created by user
        size_t m_extension;           // Extension (=CargoTransferProxy slot Id)
        int32_t m_holdAmount;         // Amount in hold space (all other amounts are in the MultiTransferList)
        String_t m_typeUnit;          // Unit for our type
        util::Vector<String_t, size_t> m_info1;
        util::Vector<String_t, size_t> m_info2;

        afl::base::SignalConnection conn_updateParticipant;
    };
}

/*
 *  MultiTransferList
 */

MultiTransferList::MultiTransferList(ui::Root& root, afl::string::Translator& tx, NumberFormatter fmt)
    : m_root(root), m_translator(tx), m_formatter(fmt), m_items()
{ }

// AbstractListbox:
size_t
MultiTransferList::getNumItems() const
{
    return m_items.size();
}

bool
MultiTransferList::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
MultiTransferList::getItemHeight(size_t /*n*/) const
{
    return getFont()->getLineHeight();
}

int
MultiTransferList::getHeaderHeight() const
{
    return getFont()->getLineHeight();
}

int
MultiTransferList::getFooterHeight() const
{
    return 0;
}

void
MultiTransferList::drawHeader(gfx::Canvas& can, gfx::Rectangle area)
{
    // ex WMultiTransferHeader::drawContent
    afl::base::Ref<gfx::Font> font(getFont());
    const int em = font->getEmWidth();

    gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);
    ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
    ctx.setColor(SkinColor::Static);

    drawHLine(ctx, area.getLeftX(), area.getBottomY()-1, area.getRightX()-1);

    area.consumeRightX(RightMargin_px);
    outTextF(ctx, area.splitRightX(FreeWidth_ems * em), m_translator("Free"));
    outTextF(ctx, area.splitRightX(HaveWidth_ems * em), m_translator("Have"));

    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    outTextF(ctx, area, " " + m_translator("Unit"));
}

void
MultiTransferList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
MultiTransferList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WMultiTransferList::drawPart, CCargoDistroList.DrawPart
    afl::base::Ref<gfx::Font> font(getFont());
    const int em = font->getEmWidth();

    afl::base::Deleter del;
    gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
    ctx.useFont(*font);

    if (item < m_items.size()) {
        const Item& it = m_items[item];

        // Free
        area.consumeRightX(RightMargin_px);
        ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
        ctx.setColor(SkinColor::Faded);
        outTextF(ctx, area.splitRightX(FreeWidth_ems * em), it.room > 20000 ? m_translator("(unl)") : m_formatter.formatNumber(it.room));

        // Have
        ctx.setColor(SkinColor::Static);
        outTextF(ctx, area.splitRightX(HaveWidth_ems * em), m_formatter.formatNumber(it.have));

        // Tag mark/Name
        String_t text = it.tag ? UTF_BULLET : " ";
        text += it.name;
        ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        outTextF(ctx, area, text);
    }
}

// Widget:
void
MultiTransferList::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
MultiTransferList::getLayoutInfo() const
{
    gfx::Point cellSize = getFont()->getCellSize();

    int numLines = 1 + std::max(5, std::min(20, int(m_items.size())));
    int height = numLines * cellSize.getY();

    int width = (FreeWidth_ems + HaveWidth_ems + UnitWidth_ems) * cellSize.getX() + RightMargin_px;

    gfx::Point size(width, height);
    return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
}

bool
MultiTransferList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

// MultiTransferList:
void
MultiTransferList::addItem(String_t name, int32_t have, int32_t room, bool isTemporary)
{
    m_items.push_back(Item(name, have, room, false, isTemporary));
    handleModelChange();
}

void
MultiTransferList::setItem(size_t index, int32_t have, int32_t room)
{
    if (index < m_items.size()) {
        if (Updater()
            .set(m_items[index].have, have)
            .set(m_items[index].room, room))
        {
            requestRedraw();
        }
    }
}

void
MultiTransferList::setItemTag(size_t index, bool tag)
{
    if (index < m_items.size()) {
        if (Updater().set(m_items[index].tag, tag)) {
            requestRedraw();
        }
    }
}

bool
MultiTransferList::hasAnyUsedTemporaries() const
{
    // ex WMultiTransferList::isValid (part)
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].isTemporary && m_items[i].have > 0) {
            return true;
        }
    }
    return false;
}

bool
MultiTransferList::hasRoomForHold(int32_t holdAmount, size_t extension) const
{
    return (extension < m_items.size()
            && !m_items[extension].isTemporary
            && m_items[extension].room >= holdAmount);
}

/*
 *  MultiTransferDialog
 */

MultiTransferDialog::MultiTransferDialog(ui::Root& root, afl::string::Translator& tx, NumberFormatter fmt, Element::Type type, util::RequestSender<game::Session> gameSender, game::proxy::WaitIndicator& ind, CargoTransferProxy& proxy)
    : m_list(root, tx, fmt),
      m_holdInfo(String_t(), SkinColor::Static, "+", root.provider()),
      m_unitInfo1(String_t(), SkinColor::Faded, gfx::FontRequest(), root.provider()),
      m_unitInfo2(String_t(), SkinColor::Faded, gfx::FontRequest(), root.provider()),
      m_menuButton("#", '#', root),
      m_dialogButtons(root, tx),
      m_root(root),
      m_translator(tx),
      m_formatter(fmt),
      m_type(type),
      m_gameSender(gameSender),
      m_link(ind),
      m_proxy(proxy),
      m_loop(root),
      m_numParticipants(0),
      m_numTemporaries(0),
      m_extension(0),
      m_holdAmount(0),
      m_typeUnit(),
      m_info1(),
      m_info2(),
      conn_updateParticipant(proxy.sig_change.add(this, &MultiTransferDialog::onUpdateParticipant))
{
    m_holdInfo.setIsFlexible(true);
    m_list.sig_menuRequest.add(this, &MultiTransferDialog::onMenu);
    m_menuButton.sig_fire.add(this, &MultiTransferDialog::onMenuButton);
}

void
MultiTransferDialog::run(String_t title)
{
    // ex WMultiTransferWindow::init
    // Window [VBox]
    //   List
    //   VBox [UnitInfo1,2]
    //   HBox
    //     HoldInfo
    //     "#", "u"
    //   StandardDialogButtons
    afl::base::Deleter del;

    ui::Window& win = del.addNew(new ui::Window(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root))));

    ui::Group& infoGroup = del.addNew(new ui::Group(ui::layout::VBox::instance0));
    infoGroup.add(m_unitInfo1);
    infoGroup.add(m_unitInfo2);
    win.add(infoGroup);

    ui::Group& holdGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnU = del.addNew(new ui::widgets::Button("U", 'u', m_root));
    holdGroup.add(m_holdInfo);
    holdGroup.add(btnU);
    holdGroup.add(m_menuButton);
    win.add(holdGroup);

    win.add(m_dialogButtons);

    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:multicargo"));

    ui::Widget& keyHandler = del.addNew(new ui::widgets::KeyForwarder(*this));
    win.add(keyHandler);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(help);
    win.pack();

    btnU.dispatchKeyTo(keyHandler);
    m_dialogButtons.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    m_dialogButtons.ok().sig_fire.add(this, &MultiTransferDialog::onOK);
    m_dialogButtons.addHelp(help);

    // Attach this event here so it doesn't observe all the initialisation
    m_list.sig_change.add(this, &MultiTransferDialog::onListScroll);
    onListScroll();

    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

void
MultiTransferDialog::loadMoreParticipants()
{
    CargoTransferProxy::General gen;
    m_proxy.getGeneralInformation(m_link, gen);
    m_typeUnit = gen.typeUnits.get(m_type);
    while (m_numParticipants < gen.numParticipants) {
        CargoTransferProxy::Participant part;
        m_proxy.getParticipantInformation(m_link, m_numParticipants, part);
        if (m_numParticipants == 0) {
            // This is the hold space
            onUpdateParticipant(0, part.cargo);
        } else {
            // Add to list
            m_list.addItem(part.name, part.cargo.amount.get(m_type), part.cargo.remaining.get(m_type), part.isTemporary);

            String_t info1 = part.name;
            util::addListItem(info1, ": ", part.info1);

            m_info1.set(m_numParticipants-1, info1);
            m_info2.set(m_numParticipants-1, part.info2);
        }
        ++m_numParticipants;
    }
}

void
MultiTransferDialog::setInitialPosition(size_t pos)
{
    if (pos != 0) {
        m_list.setCurrentItem(pos-1);
    }
}

void
MultiTransferDialog::setExtension(size_t ext)
{
    if (ext != m_extension) {
        if (m_extension != 0) {
            m_list.setItemTag(m_extension-1, false);
        }
        m_extension = ext;
        if (m_extension != 0) {
            m_list.setItemTag(m_extension-1, true);
        }
        updateValidity();
    }
}

bool
MultiTransferDialog::handleKey(util::Key_t key, int prefix)
{
    // ex WMultiTransferList::handleEvent, CCargoDistroWindow.Handle
    switch (key) {
     case ' ':
     case '.': {
        // Move tag mark here, or untag.
        size_t newExtension = m_list.getCurrentItem()+1;
        if (newExtension == m_extension) {
            setExtension(0);
        } else {
            setExtension(newExtension);
        }
        return true;
     }

     case util::Key_Insert:
     case 'a':
        // Create temporary
        if (m_numTemporaries < MAX_TEMPORARIES) {
            if (m_numTemporaries != 0
                || MessageBox(m_translator("PCC2 will now create a temporary cargo storage which you can use "
                                           "to shuffle cargo around. It must be empty before you can finish "
                                           "this cargo transfer.\n"
                                           "Proceed?"),
                              m_translator("Cargo Transfer"), m_root).doYesNoDialog(m_translator))
            {
                ++m_numTemporaries;
                m_proxy.addHoldSpace(Format(m_translator("Temporary Storage #%d"), m_numTemporaries));
                loadMoreParticipants();
                m_list.setCurrentItem(m_numParticipants-1);
            }
        }
        return true;

     case 'u':
        // Unload everything to hold
        m_proxy.moveAll(m_type, 0, m_extension, false);
        return true;

     case 'e':
        // Distribute equally
        distribute(CargoTransfer::DistributeEqually);
        return true;

     case 'f':
        // Distribute equal free space
        distribute(CargoTransfer::DistributeFreeSpace);
        return true;

     case 'p':
        // Distribute proportionally
        distribute(CargoTransfer::DistributeProportionally);
        return true;

     case util::Key_Left:
     case '+':
        move(prefix != 0 ? prefix : 10);
        return true;

     case util::Key_Left + util::KeyMod_Shift:
     case '+' + util::KeyMod_Shift:
        move(1);
        return true;

     case util::Key_Left + util::KeyMod_Ctrl:
     case '+' + util::KeyMod_Ctrl:
        move(100);
        return true;

     case util::Key_Left + util::KeyMod_Alt:
     case '+' + util::KeyMod_Alt:
        move(0x7FFFFFFF);
        return true;

     case util::Key_Right:
     case '-':
        move(prefix != 0 ? -prefix : -10);
        return true;

     case util::Key_Right + util::KeyMod_Shift:
     case '-' + util::KeyMod_Shift:
        move(-1);
        return true;

     case util::Key_Right + util::KeyMod_Ctrl:
     case '-' + util::KeyMod_Ctrl:
        move(-100);
        return true;

     case util::Key_Right + util::KeyMod_Alt:
     case '-' + util::KeyMod_Alt:
        move(-0x7FFFFFFF);
        return true;

     default:
        return false;
    }
}

void
MultiTransferDialog::move(int32_t n)
{
    // ex WMultiTransferList::move
    m_proxy.moveExt(m_type, n, 0, m_list.getCurrentItem()+1, m_extension, false);
}

void
MultiTransferDialog::distribute(CargoTransfer::DistributeMode m)
{
    // ex WMultiTransferList::distribute
    m_proxy.distribute(m_type, 0, m_extension, m);
}

void
MultiTransferDialog::doContextMenu(gfx::Point anchor, bool context)
{
    // ex WMultiTransferList::doDistributeMenu
    ui::widgets::StringListbox box(m_root.provider(), m_root.colorScheme());
    if (m_holdAmount != 0) {
        // Distributions only when hold is nonempty
        box.addItem('e', m_translator("E - Add Equal"));
        box.addItem('f', m_translator("F - Equal Free Space"));
        box.addItem('p', m_translator("P - Proportional"));
    }
    box.addItem('u', m_translator("U - Unload"));
    if (context) {
        // Context menu: offer tag/untag
        if (m_list.getCurrentItem() + 1 == m_extension) {
            box.addItem(' ', m_translator("Space - Un-tag"));
        } else {
            box.addItem(' ', m_translator("Space - Tag"));
        }
    }

    ui::EventLoop loop(m_root);
    if (ui::widgets::MenuFrame(ui::layout::HBox::instance0, m_root, loop).doMenu(box, anchor)) {
        int32_t key;
        if (box.getCurrentKey(key)) {
            handleKey(key, 0);
        }
    }
}

bool
MultiTransferDialog::isValid() const
{
    // ex WMultiTransferList::isValid, CCargoDistroWindow.IsValid
    return !m_list.hasAnyUsedTemporaries()
        && (m_holdAmount == 0
            || (m_extension > 0
                && m_list.hasRoomForHold(m_holdAmount, m_extension-1)));
}

void
MultiTransferDialog::updateValidity()
{
    m_dialogButtons.ok().setState(ui::Widget::DisabledState, !isValid());
}

void
MultiTransferDialog::onUpdateParticipant(size_t index, const CargoTransferProxy::Cargo& cargo)
{
    if (index == 0) {
        // ex WMultiTransferHoldInfo::drawContent, CCargoDistroWindow.DrawInterior
        // This is the hold space
        m_holdAmount = cargo.amount.get(m_type);
        m_holdInfo.setText(Format(m_translator("On hold: %d %s"), m_formatter.formatNumber(m_holdAmount), m_typeUnit));
    } else {
        // Update in list
        m_list.setItem(index-1, cargo.amount.get(m_type), cargo.remaining.get(m_type));
    }

    updateValidity();
}

void
MultiTransferDialog::onMenu(gfx::Point anchor)
{
    // ex WMultiTransferList::onMenu
    doContextMenu(anchor, true);
}

void
MultiTransferDialog::onMenuButton()
{
    doContextMenu(m_menuButton.getExtent().getBottomLeft(), false);
}

void
MultiTransferDialog::onOK()
{
    if (isValid()) {
        // ex WMultiTransferList::finish
        if (m_extension != 0) {
            m_proxy.move(m_type, m_holdAmount, 0, m_extension, false);
        }
        m_proxy.commit();
        m_loop.stop(0);
    }
}

void
MultiTransferDialog::onListScroll()
{
    size_t index = m_list.getCurrentItem();
    m_unitInfo1.setText(m_info1.get(index));
    m_unitInfo2.setText(m_info2.get(index));
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doMultiTransfer(game::actions::MultiTransferSetup setup,
                                 util::RequestSender<game::Session> gameSender,
                                 String_t elementName, ui::Root& root,
                                 afl::string::Translator& tx)
{
    // ex client/dialogs/multitransfer.cc:doMultiTransfer, transfer.pas:DistributeCargo
    Downlink link(root, tx);
    CargoTransferProxy proxy(gameSender, root.engine().dispatcher());
    MultiTransferSetup::Result r = proxy.init(link, setup);

    switch (r.status) {
     case MultiTransferSetup::Failure:    // This has normally been caught earlier
     case MultiTransferSetup::NoCargo:
        MessageBox(Format(tx("Nobody has %s at this place."), elementName),
                   tx("Cargo Transfer"),
                   root).doOkDialog(tx);
        break;

     case MultiTransferSetup::NoPeer:
        if (setup.isFleetOnly()) {
            MessageBox(Format(tx("There is no other fleet member at this place which could carry %s."), elementName),
                       tx("Cargo Transfer"),
                       root).doOkDialog(tx);
        } else {
            MessageBox(Format(tx("There is no other unit at this place which could carry %s."), elementName),
                       tx("Cargo Transfer"),
                       root).doOkDialog(tx);
        }
        break;

     case MultiTransferSetup::Success:
        MultiTransferDialog dlg(root, tx, game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(link), setup.getElementType(), gameSender, link, proxy);
        dlg.loadMoreParticipants();
        dlg.setInitialPosition(r.thisShipIndex);
        dlg.setExtension(r.extensionIndex);
        dlg.run(Format(tx("Transfer %s"), elementName));
        break;
    }
}
