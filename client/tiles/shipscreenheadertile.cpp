/**
  *  \file client/tiles/shipscreenheadertile.cpp
  */

#include "client/tiles/shipscreenheadertile.hpp"
#include "game/map/ship.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "afl/string/format.hpp"
#include "ui/res/resid.hpp"

using ui::widgets::FrameGroup;
using client::widgets::getFrameTypeFromTaskStatus;

client::tiles::ShipScreenHeaderTile::ShipScreenHeaderTile(ui::Root& root, gfx::KeyEventConsumer& kmw, Kind k)
    : ControlScreenHeader(root, kmw),
      m_receiver(root.engine().dispatcher(), *this),
      m_kind(k)
{
    // ex WShipScreenHeaderTile::WShipScreenHeaderTile
    switch (k) {
     case ShipScreen:
        enableButton(btnAuto, ui::NoFrame);
        enableButton(btnName, ui::NoFrame);
        enableButton(btnAdd,  ui::NoFrame);
        enableButton(btnTab,  ui::NoFrame);
        break;

     case HistoryScreen:
        enableButton(btnAdd,  ui::NoFrame);
        enableButton(btnTab,  ui::NoFrame);
        break;

     case ShipTaskScreen:
        enableButton(btnCScr, ui::NoFrame);
        break;
    }
}

void
client::tiles::ShipScreenHeaderTile::attach(game::proxy::ObjectObserver& oop)
{
    class Job : public util::Request<ControlScreenHeader> {
     public:
        Job(game::Session& session, game::map::Object* obj, Kind kind)
            : m_name(obj != 0 ? obj->getName(game::PlainName, session.translator(), session.interface()) : String_t()),
              m_subtitle(),
              m_marked(obj != 0 && obj->isMarked()),
              m_hasMessages(false),
              m_kind(kind),
              m_taskStatus(kind == ShipScreen     ? session.getTaskStatus(obj, interpreter::Process::pkShipTask, false) :
                           kind == ShipTaskScreen ? session.getTaskStatus(obj, interpreter::Process::pkShipTask,  true) :
                           game::Session::NoTask)
            {
                afl::string::Translator& tx = session.translator();

                // ex WShipScreenHeaderTile::getSubtitle
                game::map::Ship* sh = dynamic_cast<game::map::Ship*>(obj);
                game::Game* g = session.getGame().get();
                game::Root* r = session.getRoot().get();
                game::spec::ShipList* sl = session.getShipList().get();
                if (sh != 0 && g != 0 && r != 0 && sl != 0) {
                    /* Format is "(Id #%d, [race] [level] hull)".
                       Since format() cannot easily suppress strings conditionally, we
                       implement the suppression here. The suppressed parameters still
                       appear in the format string, but the %! makes them not show up.
                       This has the added benefit of making the format strings for the
                       "(ID race hull)" and (ID level hull)" cases different which some
                       languages may need. An alternative would have been to use the
                       %nn$ syntax to move the argument pointer. */
                    game::UnitScoreList::Index_t index = 0;
                    int16_t level = 0, turn = 0;
                    bool levelKnown = (g->shipScores().lookup(game::ScoreId_ExpLevel, index) && sh->unitScores().get(index, level, turn));
                    int owner = sh->getRealOwner().orElse(0);
                    String_t fmt = (owner != g->getViewpointPlayer()
                                    ? (levelKnown
                                       ? tx("(Id #%d, %s %s %s)")
                                       : tx("(Id #%d, %s %!s%s)"))
                                    : (levelKnown
                                       ? tx("(Id #%d, %!s%s %s)")
                                       : tx("(Id #%d, %!s%!s%s)")));
                    int hullNumber;
                    game::spec::Hull* hull = sh->getHull().get(hullNumber) ? sl->hulls().get(hullNumber) : 0;
                    m_subtitle = afl::string::Format(fmt.c_str(),
                                                     sh->getId(),
                                                     r->playerList().getPlayerName(owner, game::Player::AdjectiveName),
                                                     r->hostConfiguration().getExperienceLevelName(level, session.translator()),
                                                     (hull ? hull->getName(sl->componentNamer()) : tx("ship")));

                    if (hull) {
                        m_image = ui::res::makeResourceId(ui::res::SHIP, hull->getInternalPictureNumber(), hull->getId());
                    } else {
                        // Unknown or out-of-range. In any case, it's not known, so it's a nonvisual contact.
                        m_image = RESOURCE_ID("nvc");
                    }
                    m_hasMessages = (m_kind == ShipScreen && !sh->messages().empty());
                }
            }
        void handle(ControlScreenHeader& t)
            {
                t.setText(txtHeading, m_name);
                t.setText(txtSubtitle, m_subtitle);
                t.setHasMessages(m_hasMessages);
                t.enableButton(btnImage, m_marked ? ui::YellowFrame : ui::NoFrame);
                t.setImage(m_image);
                if (m_kind == ShipScreen) {
                    t.enableButton(btnAuto, getFrameTypeFromTaskStatus(m_taskStatus));
                }
                if (m_kind == ShipTaskScreen) {
                    t.enableButton(btnCScr, getFrameTypeFromTaskStatus(m_taskStatus));
                }
            }
     private:
        String_t m_name;
        String_t m_subtitle;
        String_t m_image;
        bool m_marked;
        bool m_hasMessages;
        Kind m_kind;
        game::Session::TaskStatus m_taskStatus;
    };
    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<ControlScreenHeader> reply, Kind kind)
            : m_reply(reply), m_kind(kind)
            { }
        virtual void handle(game::Session& s, game::map::Object* obj)
            { m_reply.postNewRequest(new Job(s, obj, m_kind)); }
     private:
        util::RequestSender<ControlScreenHeader> m_reply;
        Kind m_kind;
    };

    oop.addNewListener(new Listener(m_receiver.getSender(), m_kind));
}
