/**
  *  \file client/dialogs/alliancedialog.cpp
  */

#include "client/dialogs/alliancedialog.hpp"
#include "client/downlink.hpp"
#include "client/widgets/alliancestatuslist.hpp"
#include "game/alliance/level.hpp"
#include "game/game.hpp"
#include "game/playerlist.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/statictext.hpp"
#include "util/translation.hpp"

using client::widgets::AllianceLevelGrid;
using client::widgets::AllianceStatusList;
using game::alliance::Level;
using game::alliance::Offer;
using ui::Group;
using ui::Spacer;
using ui::layout::HBox;
using ui::layout::VBox;
using ui::widgets::Button;
using ui::widgets::FocusIterator;
using ui::widgets::StaticText;

client::dialogs::AllianceDialog::AllianceDialog(ui::Root& root,
                                                util::RequestSender<game::Session> gameSender,
                                                afl::string::Translator& tx)
    : Window(tx.translateString("Edit Alliances"),
             root.provider(),
             root.colorScheme(),
             ui::BLUE_WINDOW,
             ui::layout::VBox::instance5),
      m_deleter(),
      m_loop(root),
      m_root(root),
      m_pList(),
      m_pGrid(),
      m_data()
{
    // ex WAllyWindow::WAllyWindow
    initDialog();
    initContent(gameSender);
}

void
client::dialogs::AllianceDialog::run(util::RequestSender<game::Session> gameSender)
{
    // ex doAllianceDialog (sort-of)

    // Do we actually allow alliances?
    if (m_data.alliances.getLevels().size() == 0) {
        ui::dialogs::MessageBox(_("Your host does not support alliances, or PCC2 does not know how to configure them."),
                                _("Edit Alliances"),
                                m_root).doOkDialog();
    } else {
        pack();

        m_pList->requestFocus();

        m_root.centerWidget(*this);
        m_root.add(*this);
        int result = m_loop.run();
        m_root.remove(*this);

        if (result != 0) {
            // User confirmed; write back. This will update command messages.
            writeBack(gameSender);
        }
    }
}

void
client::dialogs::AllianceDialog::writeBack(util::RequestSender<game::Session> gameSender)
{
    class Query : public util::Request<game::Session> {
     public:
        Query(const Data& data)
            : m_data(data)
            { }
        void handle(game::Session& session)
            {
                if (game::Game* pGame = session.getGame().get()) {
                    pGame->currentTurn().alliances().copyFrom(m_data.alliances);
                    // FIXME-> // Update teams if configured
                    // FIXME-> syncTeamsFromAlliances(liveAllies);
                }
            }
     private:
        const Data& m_data;
    };
    Downlink link(m_root);
    Query q(m_data);
    link.call(gameSender, q);
}

void
client::dialogs::AllianceDialog::initDialog()
{
    // Build the dialog
    // VBox
    //   HBox
    //     VBox
    //       Static "Alliances:"
    //       AllianceStatusList
    //       Spacer
    //     VBox
    //       Static "Status:"
    //       AllianceLevelGrid
    //       Spacer
    //   HBox
    //     "OK"
    //     "Cancel"
    //    ["Teams"]
    //     Spacer
    //     "Help"
    m_pList = &m_deleter.addNew(new AllianceStatusList(m_root));
    m_pList->sig_selectPlayer.add(this, &AllianceDialog::onSelectPlayer);
    m_pList->sig_toggleAlliance.add(this, &AllianceDialog::onToggleAlliance);

    m_pGrid = &m_deleter.addNew(new AllianceLevelGrid(m_root));
    m_pGrid->sig_toggleOffer.add(this, &AllianceDialog::onToggleOffer);

    Group& g1  = m_deleter.addNew(new Group(HBox::instance5));
    Group& g11 = m_deleter.addNew(new Group(VBox::instance5));
    g11.add(m_deleter.addNew(new StaticText(_("Alliances:"), util::SkinColor::Static, gfx::FontRequest().addSize(1), m_root.provider())));
    g11.add(ui::widgets::FrameGroup::wrapWidget(m_deleter, m_root.colorScheme(), ui::widgets::FrameGroup::LoweredFrame, *m_pList));
    g11.add(m_deleter.addNew(new Spacer()));
    g1.add(g11);

    Group& g12 = m_deleter.addNew(new Group(VBox::instance5));
    g12.add(m_deleter.addNew(new StaticText(_("Status:"), ui::SkinColor::Static, gfx::FontRequest().addSize(1), m_root.provider())));
    g12.add(*m_pGrid);
    g12.add(m_deleter.addNew(new Spacer()));
    g1.add(g12);
    add(g1);

    Group& g2 = m_deleter.addNew(new Group(HBox::instance5));

    Button& btnOK     = m_deleter.addNew(new Button(_("OK"),     util::Key_Return, m_root));
    Button& btnCancel = m_deleter.addNew(new Button(_("Cancel"), util::Key_Escape, m_root));
    btnOK.sig_fire.addNewClosure(m_loop.makeStop(1));
    btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));
    g2.add(btnOK);
    g2.add(btnCancel);
    g2.add(m_deleter.addNew(new Spacer()));
    // FIXME->g2.add(m_deleter.addNew(new UIButton(_("Help"), 'h')));
    add(g2);

    FocusIterator& fi = m_deleter.addNew(new FocusIterator(FocusIterator::Horizontal + FocusIterator::Tab));
    add(fi);
    fi.add(*m_pList);
    fi.add(*m_pGrid);
    // FIXME->add(m_deleter.addNew(new WHelpWidget("pcc2:allies")));
    // FIXME->add(m_deleter.addNew(new UIQuit(0)));
}

void
client::dialogs::AllianceDialog::initContent(util::RequestSender<game::Session> gameSender)
{
    // Get alliances
    class Query : public util::Request<game::Session> {
     public:
        Query(Data& data)
            : m_data(data)
            { }
        void handle(game::Session& session)
            {
                if (game::Game* pGame = session.getGame().get()) {
                    // FIXME: how do we solve this? // liveAllies is not necessarily in sync with command messages; update it.
                    pGame->currentTurn().alliances().postprocess();

                    m_data.alliances = pGame->currentTurn().alliances();
                    m_data.self = pGame->getViewpointPlayer();
                }
                if (game::Root* pRoot = session.getRoot().get()) {
                    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
                        if (game::Player* pl = pRoot->playerList().get(i)) {
                            if (pl->isReal()) {
                                m_data.names.set(i, pl->getName(game::Player::ShortName));
                                m_data.players += i;
                            }
                        }
                    }
                }
            }

     private:
        Data& m_data;
    };
    Downlink link(m_root);
    Query q(m_data);
    link.call(gameSender, q);

    // Initialize player list
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        if (m_data.players.contains(i)) {
            m_pList->add(i, m_data.names.get(i), getPlayerFlags(i));
        }
    }
    m_pList->setCurrentItem(0);     // This selects the first valid player

    // Initialize level list
    for (size_t i = 0, n = m_data.alliances.getLevels().size(); i < n; ++i) {
        m_pGrid->add(i, m_data.alliances.getLevels()[i].getName());
    }

    // Load current levels
    onSelectPlayer(m_pList->getCurrentPlayer());
}

client::widgets::AllianceStatusList::ItemFlags_t
client::dialogs::AllianceDialog::getPlayerFlags(int player) const
{
    AllianceStatusList::ItemFlags_t result;
    if (player == m_data.self) {
        result += AllianceStatusList::Self;
    } else {
        if (m_data.alliances.isAny(player, Level::IsOffer, true)) {
            result += AllianceStatusList::WeOffer;
        }
        if (m_data.alliances.isAny(player, Level::IsOffer, false)) {
            result += AllianceStatusList::TheyOffer;
        }
        if (m_data.alliances.isAny(player, Level::IsEnemy, true)) {
            result += AllianceStatusList::Enemy;
        }
    }
    return result;
}

void
client::dialogs::AllianceDialog::onSelectPlayer(int player)
{
    for (size_t i = 0, n = m_data.alliances.getOffers().size(); i < n; ++i) {
        const game::alliance::Offer& o = m_data.alliances.getOffers()[i];
        m_pGrid->setOffer(i, o.theirOffer.get(player), o.newOffer.get(player));
    }
}

void
client::dialogs::AllianceDialog::onToggleAlliance(int player)
{
    // ex WAllyList::toggleCurrent
    // FIXME: cascading operation?
    m_data.alliances.setAll(player, Level::IsOffer, !m_data.alliances.isAny(player, Level::IsOffer, true));

    // FIXME: manually propagate changes
    onChange();
    onSelectPlayer(player);
}

void
client::dialogs::AllianceDialog::onChange()
{
    // ex WAllyLevelGrid::setPlayer [sort-of]
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        if (m_data.players.contains(i)) {
            m_pList->setFlags(i, getPlayerFlags(i));
        }
    }
}

void
client::dialogs::AllianceDialog::onToggleOffer(size_t index)
{
    // ex WAllyLevelGrid::toggleCurrent
    const Level* level = m_data.alliances.getLevel(index);
    const Offer* offer = m_data.alliances.getOffer(index);
    int player = m_pList->getCurrentPlayer();
    if (level && offer) {
        // Cycling order is (Unknown/No) -> (Yes) -> (Conditional)
        switch (offer->newOffer.get(player)) {
         case Offer::No:
         case Offer::Unknown:
            m_data.alliances.set(index, player, Offer::Yes);
            if (level->hasFlag(Level::NeedsOffer)) {
                m_data.alliances.setAll(player, Level::IsOffer, true);
            }
            break;

         case Offer::Yes:
            if (level->hasFlag(Level::AllowConditional)) {
                m_data.alliances.set(index, player, Offer::Conditional);
            } else {
                m_data.alliances.set(index, player, Offer::No);
            }
            break;

         case Offer::Conditional:
            m_data.alliances.set(index, player, Offer::No);
            break;
        }

        // FIXME: manually propagate changes
        onChange();
        onSelectPlayer(player);
    }
}
