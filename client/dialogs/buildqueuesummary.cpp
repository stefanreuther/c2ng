/**
  *  \file client/dialogs/buildqueuesummary.cpp
  *  \brief Build Queue Summary Dialog
  */

#include <algorithm>
#include "client/dialogs/buildqueuesummary.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/actions/changebuildqueue.hpp"
#include "game/proxy/selectionproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/richlistbox.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/string.hpp"

namespace {
    using game::proxy::BuildQueueProxy;
    using game::actions::ChangeBuildQueue;

    /*
     *  Matching items
     */

    // Match mode
    enum Match {
        MatchNone,
        MatchAll,
        MatchAction,
        MatchHull
    };
    typedef std::pair<Match, int> Match_t;

    /* Check for match of one build queue entry */
    bool matchInfo(const Match_t& m, const BuildQueueProxy::Info_t& in)
    {
        bool ok = false;
        switch (m.first) {
         case MatchNone:                                      break;
         case MatchAll:    ok = true;                         break;
         case MatchAction: ok = (m.second == int(in.action)); break;
         case MatchHull:   ok = (m.second == in.hullNr);      break;
        }
        return ok;
    }


    /*
     *  Hull summary
     */

    // Summary entry for one hull
    struct HullEntry {
        int numBuild, numPlan, numClone;
        const BuildQueueProxy::Info_t* pSpecimen;

        HullEntry()
            : numBuild(0), numPlan(0), numClone(0), pSpecimen(0)
            { }
    };
    typedef std::map<int, HullEntry> HullEntries_t;

    /* Account for one build queue entry */
    void addInfo(HullEntry& out, const BuildQueueProxy::Info_t& in)
    {
        switch (in.action) {
         case ChangeBuildQueue::BuildShip: ++out.numBuild; break;
         case ChangeBuildQueue::CloneShip: ++out.numClone; break;
         case ChangeBuildQueue::PlanShip:  ++out.numPlan;  break;
        }
        out.pSpecimen = &in;
    }


    /*
     *  Comparison
     */

    struct CompareHullEntriesByName {
        bool operator()(const HullEntry* a, const HullEntry* b) const
            {
                return a != 0 && a->pSpecimen != 0
                    && b != 0 && b->pSpecimen != 0
                    && (a->pSpecimen->hullName < b->pSpecimen->hullName);
            }
    };


    /*
     *  Build Queue Summary Listbox
     *
     *  We use a RichListbox implementation, but add some extra functions to set it up.
     */

    class BuildQueueSummaryListbox : public ui::widgets::RichListbox {
     public:
        BuildQueueSummaryListbox(gfx::ResourceProvider& provider, ui::ColorScheme& scheme);

        void addSummaryItem(Match_t m, String_t label, String_t info);
        void addSummaryHeading(String_t label);
        void addOrderItem(Match_t m, String_t label, int num);
        void addHullItem(const HullEntry& entry, afl::string::Translator& tx);

        Match_t getCurrentMatcher() const;

     private:
        std::vector<Match_t> m_match;
    };


    /*
     *  Dialog
     */

    class BuildQueueSummaryDialog {
     public:
        BuildQueueSummaryDialog(BuildQueueSummaryListbox& list,
                                const game::proxy::BuildQueueProxy::Infos_t& infos,
                                ui::Root& root,
                                util::RequestSender<game::Session> gameSender,
                                afl::string::Translator& tx)
            : m_list(list),
              m_infos(infos),
              m_root(root),
              m_gameSender(gameSender),
              m_translator(tx),
              m_markButton(tx("Mark..."), 'm', root)
            {
                m_markButton.sig_fire.add(this, &BuildQueueSummaryDialog::onMark);
            }

        void run()
            {
                // Window [VBox]
                //   FrameGroup > ScrollbarContainer > List
                //   HBox
                //      Button "Close"
                //      Button "Mark..."
                //      Spacer
                //      Button "Help"
                ui::EventLoop loop(m_root);
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(m_translator("Build Order Summary"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
                win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root))));

                ui::Widget& helpWidget = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:queuemanager"));

                ui::Group& buttonGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
                ui::widgets::Button& btnClose = del.addNew(new ui::widgets::Button(m_translator("Close"), util::Key_Escape, m_root));
                ui::widgets::Button& btnHelp  = del.addNew(new ui::widgets::Button(m_translator("Help"),  'h',              m_root));
                buttonGroup.add(btnClose);
                buttonGroup.add(m_markButton);
                buttonGroup.add(del.addNew(new ui::Spacer()));
                buttonGroup.add(btnHelp);
                win.add(buttonGroup);
                win.add(helpWidget);
                win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
                win.pack();

                // Limit size
                int rw = m_root.getExtent().getWidth();
                int rh = m_root.getExtent().getHeight();
                int w = std::min(rw * 9/10, win.getExtent().getWidth());
                int h = std::min(rh * 9/10, win.getExtent().getHeight());
                m_list.setPreferredHeight(rh * 7/10);
                win.setExtent(gfx::Rectangle(0, 0, w, h));

                btnClose.sig_fire.addNewClosure(loop.makeStop(0));
                btnHelp.dispatchKeyTo(helpWidget);

                m_root.centerWidget(win);
                m_root.add(win);
                loop.run();
            }

        void onMark()
            {
                // Menu
                enum { Mark, MarkOnly, Unmark };

                ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
                list.addItem(Mark,     m_translator("Mark these starbases"));
                list.addItem(MarkOnly, m_translator("Mark only these starbases"));
                list.addItem(Unmark,   m_translator("Unmark these starbases"));

                ui::EventLoop loop(m_root);
                if (!ui::widgets::MenuFrame(ui::layout::VBox::instance0, m_root, loop).doMenu(list, m_markButton.getExtent().getBottomLeft())) {
                    return;
                }

                // Build list
                game::ref::List bases;
                const Match_t m = m_list.getCurrentMatcher();
                for (size_t i = 0, n = m_infos.size(); i < n; ++i) {
                    if (matchInfo(m, m_infos[i])) {
                        bases.add(game::Reference(game::Reference::Starbase, m_infos[i].planetId));
                    }
                }

                // Create a short-lived SelectionProxy; we don't need any callbacks that would necessitate a long-lived one.
                game::proxy::SelectionProxy proxy(m_gameSender, m_root.engine().dispatcher());

                // Commands
                int32_t key = 0;
                list.getCurrentKey(key);
                switch (key) {
                 case Mark:
                    proxy.markList(game::map::Selections::CurrentLayer, bases, true);
                    break;
                 case MarkOnly:
                    proxy.clearLayer(game::map::Selections::CurrentLayer);
                    proxy.markList(game::map::Selections::CurrentLayer, bases, true);
                    break;
                 case Unmark:
                    proxy.markList(game::map::Selections::CurrentLayer, bases, false);
                    break;
                }
            }

     private:
        BuildQueueSummaryListbox& m_list;
        const game::proxy::BuildQueueProxy::Infos_t& m_infos;
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;

        ui::widgets::Button m_markButton;
    };
}


/*
 *  BuildQueueSummaryListbox implementation
 */

BuildQueueSummaryListbox::BuildQueueSummaryListbox(gfx::ResourceProvider& provider, ui::ColorScheme& scheme)
    : RichListbox(provider, scheme)
{
    setRenderFlag(UseBackgroundColorScheme, true);
    setRenderFlag(DisableWrap, true);
    setRenderFlag(NoShade, true);
}

void
BuildQueueSummaryListbox::addSummaryItem(Match_t m, String_t label, String_t info)
{
    addItem(util::rich::Text("  " + label + " ") + util::rich::Text(info).withColor(util::SkinColor::Faded), 0, true);
    m_match.push_back(m);
}

void
BuildQueueSummaryListbox::addSummaryHeading(String_t label)
{
    addItem(util::rich::Text(label).withStyle(util::rich::StyleAttribute::Bold), 0, false);
    m_match.push_back(Match_t(MatchNone, 0));
}

void
BuildQueueSummaryListbox::addOrderItem(Match_t m, String_t label, int num)
{
    if (num != 0) {
        addSummaryItem(m, label, afl::string::Format("(%d)", num));
    }
}

void
BuildQueueSummaryListbox::addHullItem(const HullEntry& entry, afl::string::Translator& tx)
{
    if (entry.pSpecimen != 0) { // should never fail
        String_t extra;
        if (entry.numBuild != 0) {
            util::addListItem(extra, ", ", afl::string::Format(tx("%d\xC3\x97 build"), entry.numBuild));
        }
        if (entry.numClone != 0) {
            util::addListItem(extra, ", ", afl::string::Format(tx("%d\xC3\x97 clone"), entry.numClone));
        }
        if (entry.numPlan != 0) {
            util::addListItem(extra, ", ", afl::string::Format(tx("%d\xC3\x97 plan"), entry.numPlan));
        }
        addSummaryItem(Match_t(MatchHull, entry.pSpecimen->hullNr), entry.pSpecimen->hullName, "(" + extra + ")");
    }
}

Match_t
BuildQueueSummaryListbox::getCurrentMatcher() const
{
    size_t n = getCurrentItem();
    return (n < m_match.size()
            ? m_match[n]
            : Match_t());
}


/*
 *  Entry Point
 */

void
client::dialogs::doBuildQueueSummaryDialog(const game::proxy::BuildQueueProxy::Infos_t& infos,
                                           ui::Root& root,
                                           util::RequestSender<game::Session> gameSender,
                                           afl::string::Translator& tx)
{
    // Count everything
    HullEntries_t byHull;
    HullEntry summary;
    for (size_t i = 0; i < infos.size(); ++i) {
        addInfo(byHull[infos[i].hullNr], infos[i]);
        addInfo(summary, infos[i]);
    }

    // Sort by hull name
    std::vector<const HullEntry*> entries;
    for (HullEntries_t::const_iterator it = byHull.begin(); it != byHull.end(); ++it) {
        entries.push_back(&it->second);
    }
    std::sort(entries.begin(), entries.end(), CompareHullEntriesByName());

    // Build list
    BuildQueueSummaryListbox box(root.provider(), root.colorScheme());
    box.addSummaryHeading(tx("Orders"));
    box.addOrderItem(Match_t(MatchAll, 0), tx("All"), summary.numPlan + summary.numClone + summary.numBuild);
    box.addOrderItem(Match_t(MatchAction, ChangeBuildQueue::BuildShip), tx("Build"), summary.numBuild);
    box.addOrderItem(Match_t(MatchAction, ChangeBuildQueue::CloneShip), tx("Clone"), summary.numClone);
    box.addOrderItem(Match_t(MatchAction, ChangeBuildQueue::PlanShip),  tx("Plan"),  summary.numPlan);
    box.addSummaryHeading(tx("Ship Types"));
    for (size_t i = 0, n = entries.size(); i < n; ++i) {
        box.addHullItem(*entries[i], tx);
    }

    // Dialog
    BuildQueueSummaryDialog(box, infos, root, gameSender, tx).run();
}
