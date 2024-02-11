/**
  *  \file client/tiles/fleetscreenheadertile.cpp
  *  \brief Class client::tiles::FleetScreenHeaderTile
  */

#include "client/tiles/fleetscreenheadertile.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/map/fleet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/root.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "ui/res/resid.hpp"

client::tiles::FleetScreenHeaderTile::FleetScreenHeaderTile(ui::Root& root, gfx::KeyEventConsumer& kmw)
    : ControlScreenHeader(root, kmw),
      m_receiver(root.engine().dispatcher(), *this)
{
    // ex WFleetScreenHeaderTile::WFleetScreenHeaderTile
    enableButton(btnName, ui::NoFrame);
    enableButton(btnAdd,  ui::NoFrame);
    enableButton(btnJoin, ui::NoFrame);
}

void
client::tiles::FleetScreenHeaderTile::attach(game::proxy::ObjectObserver& oop)
{
    class Job : public util::Request<ControlScreenHeader> {
     public:
        Job(game::Session& session, game::map::Object* obj)
            : m_name(),
              m_subtitle(),
              m_image(),
              m_marked(obj != 0 && obj->isMarked())
            {
                afl::string::Translator& tx = session.translator();

                // ex WShipScreenHeaderTile::getSubtitle
                const game::map::Ship* sh = dynamic_cast<game::map::Ship*>(obj);
                game::Game* g = session.getGame().get();
                game::Turn* t = (g != 0 ? &g->viewpointTurn() : 0);
                const game::spec::ShipList* sl = session.getShipList().get();
                game::map::Ship* leader = (sh != 0 && t != 0 ? t->universe().ships().get(sh->getFleetNumber()) : 0);
                if (sh != 0 && sl != 0 && t != 0 && leader != 0) {
                    // WFleetScreenHeaderTile::getTitle -- title for fleet
                    m_name = afl::string::Format(session.translator()("Fleet %d"), leader->getId());
                    if (!leader->getFleetName().empty()) {
                        m_name += ": ";
                        m_name += leader->getFleetName();
                    }

                    // WFleetScreenHeaderTile::getSubtitle -- subtitle for fleet
                    // FIXME: is it enough to take this from the current ship or do we need to observe the fleet?
                    m_subtitle = afl::string::Format(session.translator()("(%d ship%!1{s%})"), game::map::Fleet(t->universe(), *leader).countFleetMembers());

                    // WFleetScreenHeaderTile::getPictureId -- picture for member
                    int hullNumber;
                    if (const game::spec::Hull* hull = sh->getHull().get(hullNumber) ? sl->hulls().get(hullNumber) : 0) {
                        m_image = ui::res::makeResourceId(ui::res::SHIP, hull->getInternalPictureNumber(), hull->getId());
                    } else {
                        m_image = RESOURCE_ID("nvc");
                    }
                }
            }
        void handle(ControlScreenHeader& t)
            {
                t.setText(txtHeading, m_name);
                t.setText(txtSubtitle, m_subtitle);
                t.enableButton(btnImage, m_marked ? ui::YellowFrame : ui::NoFrame);
                t.setImage(m_image);
            }
     private:
        String_t m_name;
        String_t m_subtitle;
        String_t m_image;
        bool m_marked;
    };
    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<ControlScreenHeader> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& s, game::map::Object* obj)
            { m_reply.postNewRequest(new Job(s, obj)); }
     private:
        util::RequestSender<ControlScreenHeader> m_reply;
    };

    oop.addNewListener(new Listener(m_receiver.getSender()));
}
