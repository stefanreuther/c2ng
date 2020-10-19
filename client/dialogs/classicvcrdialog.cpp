/**
  *  \file client/dialogs/classicvcrdialog.cpp
  *
  *  FIXME: it may make sense to allow using this without a game, for a PlayVCR-type program.
  *  Right now, we need the game for accessing...
  *  - VCRs [alternative: give in handle to that]
  *  - viewpoint player and team settings [alternative: use neutral colors]
  */

#include "client/dialogs/classicvcrdialog.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/classic/utils.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/translation.hpp"
#include "util/unicodechars.hpp"

namespace {
    /* @q CCUI$CurrentVCR (Internal Variable)
       Zero-based index of current VCR.
       @since PCC2 2.40.4 */
    const char*const INDEX_VAR_NAME = "CCUI$CURRENTVCR";

    using client::widgets::ClassicVcrInfo;
    using afl::string::Format;

    void prepareWarrior(ClassicVcrInfo::Data& out, size_t side, const game::vcr::classic::Battle& in, const game::vcr::Object& obj, game::Session& session)
    {
        // ex WVcrSelector::drawWarrior, WVcrSelector::drawContent
        // Environment
        game::Game* g = session.getGame().get();
        game::Root* r = session.getRoot().get();
        game::spec::ShipList* sl = session.getShipList().get();
        if (g == 0 || r == 0 || sl == 0) {
            // Low-fi version
            out.info[side][0] = obj.getName();
            return;
        }

        int viewpointPlayer = g->getViewpointPlayer();

        // FIXME: this i18n approach is far from perfect
        // We have the following combinations:
        //    {A <race>|Our} {<Level>|(nothing)} {planet|<type>|starship}
        // Giving a total of 2x2x3 = 12 sentences.
        size_t line = 0;

        // Object title
        String_t adj = (obj.getOwner() == viewpointPlayer
                        ? _("our")
                        : String_t(Format(_("a %s").c_str(), r->playerList().getPlayerName(obj.getOwner(), game::Player::AdjectiveName))));

        // Experience
        String_t type;
        if (in.getType() == game::vcr::classic::PHost4 && r->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels]() > 0) {
            type += r->hostConfiguration().getExperienceLevelName(obj.getExperienceLevel(), session.translator());
            type += ' ';
        }

        // Type
        if (obj.isPlanet()) {
            type += _("planet");
        } else if (const game::spec::Hull* pHull = sl->hulls().get(obj.getGuessedHull(sl->hulls()))) {
            type += pHull->getName(sl->componentNamer());
        } else {
            type += _("starship");
        }

        out.info[side][line] = Format("%s (Id #%d, %s %s)", obj.getName(), obj.getId(), adj, type);
        out.color[side][line] = g->teamSettings().getPlayerColor(obj.getOwner());
        ++line;

        // Shield, Damage, Crew
        int shield = std::max(0, obj.getShield());
        out.info[side][line] = Format(_("%d%% shield (%d kt), %d %% damaged").c_str(), shield, obj.getMass(), obj.getDamage());
        if (!obj.isPlanet()) {
            out.info[side][line] += Format(_(", %d %1{crewman%|crewmen%}").c_str(), r->userConfiguration().formatNumber(obj.getCrew()));
        }
        ++line;

        // Beams
        if (obj.getNumBeams() > 0) {
            if (const game::spec::Beam* b = sl->beams().get(obj.getBeamType())) {
                out.info[side][line] = Format("%d " UTF_TIMES " %s", obj.getNumBeams(), b->getName(sl->componentNamer()));
                ++line;
            }
        }

        // Torps/Fighters
        if (obj.getNumBays() > 0) {
            if (obj.getNumLaunchers() > 0) {
                if (const game::spec::TorpedoLauncher* tl = sl->launchers().get(obj.getTorpedoType())) {
                    out.info[side][line] = Format(_("%d %1{%s%|%ss%} and %d %1{fighter%|fighters%}").c_str(), obj.getNumTorpedoes(), tl->getName(sl->componentNamer()), obj.getNumFighters());
                } else {
                    out.info[side][line] = Format(_("%d torpedo%!1{es%} and %d %1{fighter%|fighters%}").c_str(), obj.getNumTorpedoes(), obj.getNumFighters());
                }
            } else {
                out.info[side][line] = Format(_("%d fighter bay%!1{s%} with %d fighter%!1{s%}").c_str(), obj.getNumBays(), obj.getNumFighters());
            }
            ++line;
        } else if (obj.getNumLaunchers() > 0) {
            if (const game::spec::TorpedoLauncher* tl = sl->launchers().get(obj.getTorpedoType())) {
                out.info[side][line] = Format(_("%d \xC3\x97 %1{%s launcher%|%s launchers%} with %d torp%!1{s%}").c_str(), obj.getNumLaunchers(), tl->getName(sl->componentNamer()),
                                              r->userConfiguration().formatNumber(obj.getNumTorpedoes()));
            } else {
                out.info[side][line] = Format(_("%d \xC3\x97 torpedo launcher%!1{s%} with %d torp%!1{s%}").c_str(), obj.getNumLaunchers(),
                                              r->userConfiguration().formatNumber(obj.getNumTorpedoes()));
            }
            ++line;
        } else {
            // No auxiliary weapons.
            // We can still give mor info.
            // When "NTP" is used, THost clears the "count" field, but keeps type and ammo count intact (thanks, Akseli, for that one!)
            // We can still give the number of bays if we know the hull.
            // PHost makes the ship appear with the correct weapon count, but no ammo, so this doesn't work here.
            if (const game::spec::Hull* pHull = sl->hulls().get(obj.getGuessedHull(sl->hulls()))) {
                if (pHull->getNumBays() > 0) {
                    out.info[side][line] = Format(_("(%d fighter bay%!1{s%} %snot used)").c_str(),
                                                  pHull->getNumBays(),
                                                  (obj.getNumFighters() > 0
                                                   ? String_t(Format(_("with %d fighter%!1{s%} ").c_str(), r->userConfiguration().formatNumber(obj.getNumFighters())))
                                                   : String_t()));
                    out.color[side][line] = ui::SkinColor::Faded;
                    ++line;
                } else if (pHull->getMaxLaunchers() > 0) {
                    const game::spec::TorpedoLauncher* tl = sl->launchers().get(obj.getTorpedoType());
                    out.info[side][line] = Format(_("(up to %s %snot used)").c_str(),
                                                  (tl != 0
                                                   ? String_t(Format(_("%d %s%!1{s%}").c_str(), pHull->getMaxLaunchers(), tl->getName(sl->componentNamer())))
                                                   : String_t(Format(_("%d torpedo launcher%!1{s%}").c_str(), pHull->getMaxLaunchers()))),
                                                  (obj.getNumTorpedoes() > 0
                                                   ? String_t(Format(_("with %d torp%!1{s%} ").c_str(), r->userConfiguration().formatNumber(obj.getNumTorpedoes())))
                                                   : String_t()));
                    out.color[side][line] = ui::SkinColor::Faded;
                    ++line;
                } else {
                    // No additional info possible
                }
            }
        }
    }

    String_t formatBuildPoints(const game::vcr::classic::Battle& entry, game::Session& session)
    {
        using game::vcr::classic::BattleResult_t;

        game::spec::ShipList* sl = session.getShipList().get();
        game::Root* r = session.getRoot().get();
        if (sl == 0 || r == 0) {
            return String_t();
        }

        game::vcr::Score pts;
        BattleResult_t br = entry.getResult();
        if (br == BattleResult_t(game::vcr::classic::LeftCaptured) || br == BattleResult_t(game::vcr::classic::LeftDestroyed)) {
            entry.computeScores(pts, game::vcr::classic::RightSide, r->hostConfiguration(), *sl);
        } else if (br == BattleResult_t(game::vcr::classic::RightCaptured) || br == BattleResult_t(game::vcr::classic::RightDestroyed)) {
            entry.computeScores(pts, game::vcr::classic::LeftSide, r->hostConfiguration(), *sl);
        } else {
            // ambiguous or unknown result
        }

        String_t text;

        // build points
        int minBP = pts.getBuildMillipointsMin() / 1000;
        int maxBP = pts.getBuildMillipointsMax() / 1000;
        if (minBP > 0) {
            if (maxBP == minBP) {
                text += Format("%d BP", r->userConfiguration().formatNumber(minBP));
            } else {
                text += Format("%d ... %d BP", r->userConfiguration().formatNumber(minBP), r->userConfiguration().formatNumber(maxBP));
            }
        } else {
            if (maxBP > 0) {
                text += Format("\xE2\x89\xA4%d BP", r->userConfiguration().formatNumber(maxBP));
            }
        }

        // experience
        if (pts.getExperience() > 0) {
            if (text.size()) {
                text += ", ";
            }
            text += Format("%d EP", r->userConfiguration().formatNumber(pts.getExperience()));
        }

        return text;
    }

    void prepareData(ClassicVcrInfo::Data& out, const game::vcr::classic::Battle& in, game::Session& session)
    {
        prepareWarrior(out, 0, in, in.left(), session);
        prepareWarrior(out, 1, in, in.right(), session);
        out.text[ClassicVcrInfo::Type] = in.getAlgorithmName(session.translator());

        game::Game* g = session.getGame().get();
        out.text[ClassicVcrInfo::Result] = in.formatResult(g ? g->getViewpointPlayer() : 0,
                                                           formatBuildPoints(in, session),
                                                           session.translator());

        game::map::Point pos;
        if (in.getPosition(pos)) {
            out.text[ClassicVcrInfo::Position] = Format("(%d,%d)", pos.getX(), pos.getY());
        } else {
            out.text[ClassicVcrInfo::Position] = "";
        }
    }
}


client::dialogs::ClassicVcrDialog::ClassicVcrDialog(ui::Root& root, util::RequestSender<game::Session> gameSender)
    : m_root(root),
      m_gameSender(gameSender),
      m_info(root),
      m_dialogReceiver(root.engine().dispatcher(), *this),
      m_currentIndex(0),
      m_numBattles(0),
      m_isActiveQuery(false)
{ }

client::dialogs::ClassicVcrDialog::~ClassicVcrDialog()
{ }

int
client::dialogs::ClassicVcrDialog::run()
{
    // Query number of battles
    initNumBattles();
    if (m_numBattles == 0) {
        return 0;
    }

    // Build dialog
    ui::Window window("!VCR", m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
    window.add(m_info);

    ui::widgets::Button btnUp(UTF_UP_ARROW, util::Key_Up, m_root);
    ui::widgets::Button btnDown(UTF_DOWN_ARROW, util::Key_Down, m_root);
    ui::widgets::Button btnPlay("!Play", util::Key_Return, m_root);
    ui::Spacer spc;
    ui::widgets::Button btnCancel("!Back", util::Key_Escape, m_root);

    ui::Group g(ui::layout::HBox::instance5);
    g.add(btnUp);
    g.add(btnDown);
    g.add(btnPlay);
    g.add(spc);
    g.add(btnCancel);
    window.add(g);

    ui::EventLoop loop(m_root);
    btnUp.sig_fire.add(this, &ClassicVcrDialog::onPrevious);
    btnDown.sig_fire.add(this, &ClassicVcrDialog::onNext);
    btnCancel.sig_fire.addNewClosure(loop.makeStop(0));
    btnPlay.sig_fire.add(this, &ClassicVcrDialog::onPlay);

    postLoad();

    window.pack();
    m_root.centerWidget(window);
    m_root.add(window);
    loop.run();

    saveCurrentIndex();

    return 0;
}

void
client::dialogs::ClassicVcrDialog::initNumBattles()
{
    // Query
    class Query : public util::Request<game::Session> {
     public:
        Query()
            : m_numBattles(0),
              m_currentIndex(0)
            { }
        virtual void handle(game::Session& s)
            {
                if (game::vcr::classic::Database* p = game::vcr::classic::getDatabase(s)) {
                    m_numBattles = p->getNumBattles();
                }
                try {
                    int32_t i;
                    if (interpreter::checkIntegerArg(i, s.world().getGlobalValue(INDEX_VAR_NAME))) {
                        m_currentIndex = static_cast<size_t>(i);
                    }
                }
                catch (...)
                { }
            }
        size_t getNumBattles() const
            { return m_numBattles; }
        size_t getCurrentIndex() const
            { return m_currentIndex; }
     private:
        size_t m_numBattles;
        size_t m_currentIndex;
    };

    // Call query
    Query q;
    Downlink link(m_root);
    link.call(m_gameSender, q);

    // Process result
    m_numBattles = q.getNumBattles();
    m_currentIndex = q.getCurrentIndex();
    if (m_currentIndex >= m_numBattles) {
        m_currentIndex = 0;
    }
}

void
client::dialogs::ClassicVcrDialog::saveCurrentIndex()
{
    // Query
    class Query : public util::Request<game::Session> {
     public:
        Query(size_t currentIndex)
            : m_currentIndex(currentIndex)
            { }
        virtual void handle(game::Session& s)
            { s.world().setNewGlobalValue(INDEX_VAR_NAME, interpreter::makeIntegerValue(int32_t(m_currentIndex))); }
     private:
        size_t m_currentIndex;
    };

    // Call query
    Query q(m_currentIndex);
    Downlink link(m_root);
    link.call(m_gameSender, q);
}

void
client::dialogs::ClassicVcrDialog::onPrevious()
{
    // ex WVcrSelector::next
    if (m_currentIndex > 0) {
        setCurrentIndex(m_currentIndex-1);
    }
}

void
client::dialogs::ClassicVcrDialog::onNext()
{
    // ex WVcrSelector::prev
    if (m_numBattles - m_currentIndex > 1) {
        setCurrentIndex(m_currentIndex+1);
    }
}

void
client::dialogs::ClassicVcrDialog::onPlay()
{
    sig_play.raise(m_currentIndex);
}

void
client::dialogs::ClassicVcrDialog::setCurrentIndex(size_t index)
{
    m_currentIndex = index;
    if (!m_isActiveQuery) {
        // No active query, ask game side
        postLoad();
    } else {
        // Query is active. Show temporary data.
        setTempData();
    }
}

void
client::dialogs::ClassicVcrDialog::postLoad()
{
    class Response : public util::Request<ClassicVcrDialog> {
     public:
        Response(size_t index, const ClassicVcrInfo::Data& data)
            : m_index(index),
              m_data(data)
            { }
        virtual void handle(ClassicVcrDialog& dlg)
            { dlg.setData(m_index, m_data); }
     private:
        size_t m_index;
        ClassicVcrInfo::Data m_data;
    };

    class Request : public util::Request<game::Session> {
     public:
        Request(util::RequestSender<ClassicVcrDialog> reply, size_t index)
            : m_reply(reply), m_index(index)
            { }
        virtual void handle(game::Session& s)
            {
                if (game::vcr::classic::Database* db = game::vcr::classic::getDatabase(s)) {
                    if (game::vcr::classic::Battle* battle = db->getBattle(m_index)) {
                        // Compute battle result
                        if (game::spec::ShipList* sl = s.getShipList().get()) {
                            if (game::Root* r = s.getRoot().get()) {
                                battle->getOutcome(r->hostConfiguration(), *sl, 0);
                                battle->getOutcome(r->hostConfiguration(), *sl, 1);
                            }
                        }

                        // Produce output
                        ClassicVcrInfo::Data data;
                        prepareData(data, *battle, s);
                        m_reply.postNewRequest(new Response(m_index, data));
                    }
                }
            }
     private:
        util::RequestSender<ClassicVcrDialog> m_reply;
        size_t m_index;
    };
    m_isActiveQuery = true;
    m_gameSender.postNewRequest(new Request(m_dialogReceiver.getSender(), m_currentIndex));
}

void
client::dialogs::ClassicVcrDialog::setData(size_t index, ClassicVcrInfo::Data data)
{
    if (index == m_currentIndex) {
        // Looking at correct data: finish request
        m_isActiveQuery = false;
        postprocessData(data);
        m_info.setData(data);
    } else {
        // This request no longer matches what we want. Post new request and show intermediate data.
        postLoad();
        setTempData();
    }
}

void
client::dialogs::ClassicVcrDialog::postprocessData(client::widgets::ClassicVcrInfo::Data& data)
{
    data.text[ClassicVcrInfo::Heading] = Format(_("Battle %d of %d").c_str(), m_currentIndex+1, m_numBattles);
}

void
client::dialogs::ClassicVcrDialog::setTempData()
{
    ClassicVcrInfo::Data tempData;
    postprocessData(tempData);
    m_info.setData(tempData);
}
