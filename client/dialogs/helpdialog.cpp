/**
  *  \file client/dialogs/helpdialog.cpp
  *
  *  PCC2 Comment:
  *
  *  The PCC2 Help Viewer displays XML rendered into a RichDocument
  *  (as opposed to PCC 1.x, which displays preformatted text with
  *  custom control codes from a .res file).
  *
  *  This module provides user interface and control.
  */

#include "client/dialogs/helpdialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/io/xml/node.hpp"
#include "afl/io/xml/nodereader.hpp"
#include "client/downlink.hpp"
#include "client/proxy/helpproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/document.hpp"
#include "ui/rich/documentparser.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/window.hpp"
#include "util/translation.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/scrollbar.hpp"

using afl::io::xml::Nodes_t;

namespace {
    const char*const TOC_PAGE = "toc";

    class HelpDialog {
     public:
        HelpDialog(ui::Root& root, client::proxy::HelpProxy& proxy);

        void setPage(String_t pageName, Nodes_t& pageContent);
        void run();

        void onBack();
        void onContent();
        void onGoTo();
        void onHelp();
        void onLinkClick(String_t pageName);

     private:
        ui::Root& m_root;
        ui::EventLoop m_loop;
        ui::rich::DocumentView m_docView;
        ui::widgets::Button m_btnBack;
        ui::widgets::Button m_btnContent;
        String_t m_pageName;
        Nodes_t m_pageContent;
        client::proxy::HelpProxy& m_proxy;

        /* History: this is a ring buffer. historyCount contains the number
           of valid elements, historyHead points to the next usable (that is,
           unless the buffer is full, an unused) element. We limit history by
           simply not increasing historyCount above MAX. */
        struct History {
            String_t page;
            int top;
            ui::rich::Document::LinkId_t link;
        };
        enum { MAX = 32 };
        History m_history[MAX];
        size_t m_historyCount;
        size_t m_historyHead;

        void renderContent();
        void setButtonState();
        void pushHistory();
        void loadPage(String_t pageName);
    };
}

HelpDialog::HelpDialog(ui::Root& root, client::proxy::HelpProxy& proxy)
    : m_root(root),
      m_loop(root),
      m_docView(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(40, 20),
                ui::rich::DocumentView::fl_Help,
                root.provider()),
      m_btnBack(_("Back"), util::Key_Backspace, m_root),
      m_btnContent(_("T - Content"), 't', m_root),
      m_pageName(),
      m_pageContent(),
      m_proxy(proxy),
      m_history(),
      m_historyCount(0),
      m_historyHead(0)
{
    m_btnBack.sig_fire.add(this, &HelpDialog::onBack);
    m_btnContent.sig_fire.add(this, &HelpDialog::onContent);
}

void
HelpDialog::setPage(String_t pageName, Nodes_t& pageContent)
{
    m_pageName = pageName;
    m_pageContent.swap(pageContent);
}

void
HelpDialog::run()
{
    // ex WHelpDialog::init
    // VBox
    //   HBox
    //     UIRichDocument
    //     UIScrollbar
    //   HBox
    //     UIButton...
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(_("Help"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    /* We use instance5 for g1 to have a little room between the document and the scrollbar.
       Normally, we don't have room between scrollee and scrollbar, but here it serves as
       a margin. */
    ui::Group& g1(del.addNew(new ui::Group(ui::layout::HBox::instance5)));
    ui::Group& g2(del.addNew(new ui::Group(ui::layout::HBox::instance5)));

    g1.add(m_docView);
    g1.add(del.addNew(new ui::widgets::Scrollbar(m_docView, m_root)));
    m_docView.sig_linkClick.add(this, &HelpDialog::onLinkClick);

    ui::widgets::Button& btnClose = del.addNew(new ui::widgets::Button(_("Close"), util::Key_Escape, m_root));
    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));

    g2.add(m_btnBack);
    g2.add(m_btnContent);
    g2.add(del.addNew(new ui::Spacer()));
    g2.add(btnClose);

    ui::widgets::KeyDispatcher& keys = del.addNew(new ui::widgets::KeyDispatcher());
    keys.add('b',                               this, &HelpDialog::onBack);                                   // Key_Backspace in button
    keys.add(util::KeyMod_Shift + util::Key_F1, this, &HelpDialog::onContent);  // 't' in button
    keys.add('h',                               this, &HelpDialog::onHelp);
    keys.add(util::KeyMod_Alt + 'h',            this, &HelpDialog::onHelp);
    keys.add(util::Key_F1,                      this, &HelpDialog::onHelp);
    keys.add('g',                               this, &HelpDialog::onGoTo);
                
    win.add(g1);
    win.add(g2);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(keys);
    win.pack();

    renderContent();

    setButtonState();

    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

void
HelpDialog::onBack()
{
    // ex WHelpDialog::onBack
    if (m_historyCount > 0) {
        // Pop entry
        --m_historyCount;
        m_historyHead = (m_historyHead > 0 ? m_historyHead-1 : MAX-1);
        const History& h = m_history[m_historyHead];

        // Load that page
        loadPage(h.page);
        m_docView.setTopY(h.top);
        if (m_docView.getDocument().isLinkVisible(h.link, gfx::Rectangle(0, m_docView.getPageTop(), m_docView.getExtent().getWidth(), m_docView.getExtent().getHeight()))) {
            m_docView.setSelectedLink(h.link);
        }
        setButtonState();
    }
}

void
HelpDialog::onContent()
{
    onLinkClick(TOC_PAGE);
}

void
HelpDialog::onGoTo()
{
    // ex WHelpDialog::onGoTo
    ui::widgets::InputLine input(4096, 20, m_root);
    input.setText(m_pageName);
    if (input.doStandardDialog(_("Go to"), _("Enter page name:"))) {
        if (m_pageName != input.getText()) {
            onLinkClick(input.getText());
        }
    }
}

void
HelpDialog::onHelp()
{
    onLinkClick("help");
}

void
HelpDialog::onLinkClick(String_t pageName)
{
    // ex WHelpDialog::onLinkClick
    pushHistory();
    loadPage(pageName);
    setButtonState();
}

void
HelpDialog::renderContent()
{
    // Build a reader
    afl::io::xml::NodeReader rdr;
    for (size_t i = 0, n = m_pageContent.size(); i < n; ++i) {
        rdr.addNode(m_pageContent[i]);
    }

    // Render
    ui::rich::Document& doc = m_docView.getDocument();
    doc.clear();
    m_docView.setTopY(0);
    ui::rich::DocumentParser parser(doc, rdr);
    parser.parseDocument();
    doc.finish();
    m_docView.handleDocumentUpdate();

    // FIXME: deal with hadLoadingImages()
}

void
HelpDialog::setButtonState()
{
    // ex WHelpDialog::setBackState
    m_btnContent.setState(ui::Widget::DisabledState, m_pageName == TOC_PAGE);
    m_btnBack.setState(ui::Widget::DisabledState, m_historyCount == 0);
}

/** Push current location on the history. */
void
HelpDialog::pushHistory()
{
    // ex WHelpDialog::pushHistory
    // Anything to save?
    if (m_pageName.empty()) {
        return;
    }

    // Fill in entry
    History& h = m_history[m_historyHead];
    h.page = m_pageName;
    h.top  = m_docView.getPageTop();
    h.link = m_docView.getSelectedLink();

    // Advance
    m_historyHead = (m_historyHead+1) % MAX;
    if (m_historyCount < MAX) {
        ++m_historyCount;
    }
}

void
HelpDialog::loadPage(String_t pageName)
{
    client::Downlink link(m_root);
    afl::io::xml::Nodes_t pageContent;
    m_proxy.loadHelpPage(link, pageContent, pageName);
    setPage(pageName, pageContent);
    renderContent();
}


/*
 *  Public Entry Point
 */

void
client::dialogs::doHelpDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, String_t pageName)
{
    // ex doHelp
    client::proxy::HelpProxy proxy(gameSender);
    Downlink link(root);

    afl::io::xml::Nodes_t pageContent;
    proxy.loadHelpPage(link, pageContent, pageName);

    HelpDialog dlg(root, proxy);
    dlg.setPage(pageName, pageContent);
    dlg.run();
}
