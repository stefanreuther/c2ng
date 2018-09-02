/**
  *  \file client/tiles/shipscreenheadertile.cpp
  */

#include "client/tiles/shipscreenheadertile.hpp"
#include "util/translation.hpp"
#include "game/map/ship.hpp"
#include "client/proxy/objectlistener.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "afl/string/format.hpp"
#include "ui/res/resid.hpp"

using ui::widgets::FrameGroup;

client::tiles::ShipScreenHeaderTile::ShipScreenHeaderTile(ui::Root& root, client::widgets::KeymapWidget& kmw)
    : ControlScreenHeader(root, kmw),
      m_receiver(root.engine().dispatcher(), *this)
{
    // ex WShipScreenHeaderTile::WShipScreenHeaderTile
    enableButton(btnAuto, FrameGroup::NoFrame);
    enableButton(btnAdd,  FrameGroup::NoFrame);
    enableButton(btnTab,  FrameGroup::NoFrame);
    enableButton(btnName, FrameGroup::NoFrame);

    // FIXME: alternative personalities
    // enum Kind {
    //     ShipScreen,
    //     HistoryScreen,
    //     ShipTaskScreen
    // };
    // Ship Screen:    Name, Auto, Tab, Add
    // History Screen:             Tab, Add
    // Task Screen:          CScr
}

void
client::tiles::ShipScreenHeaderTile::attach(client::proxy::ObjectObserver& oop)
{
    class Job : public util::Request<ControlScreenHeader> {
     public:
        Job(game::Session& session, game::map::Object* obj)
            : m_name(obj != 0 ? obj->getName(game::map::Object::PlainName, session.translator(), session.interface()) : String_t()),
              m_subtitle(),
              m_marked(obj != 0 && obj->isMarked())
            {
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
                                       ? _("(Id #%d, %s %s %s)")
                                       : _("(Id #%d, %s %!s%s)"))
                                    : (levelKnown
                                       ? _("(Id #%d, %!s%s %s)")
                                       : _("(Id #%d, %!s%!s%s)")));
                    int hullNumber;
                    game::spec::Hull* hull = sh->getHull().get(hullNumber) ? sl->hulls().get(hullNumber) : 0;
                    m_subtitle = afl::string::Format(fmt.c_str(),
                                                     sh->getId(),
                                                     r->playerList().getPlayerName(owner, game::Player::AdjectiveName),
                                                     r->hostConfiguration().getExperienceLevelName(level, session.translator()),
                                                     (hull ? hull->getName(sl->componentNamer()) : _("ship")));

                    if (hull) {
                        m_image = ui::res::makeResourceId(ui::res::SHIP, hull->getInternalPictureNumber(), hull->getId());
                    } else {
                        // Unknown or out-of-range. In any case, it's not known, so it's a nonvisual contact.
                        m_image = RESOURCE_ID("nvc");
                    }
                }
            }
        void handle(ControlScreenHeader& t)
            {
                t.setText(txtHeading, m_name);
                t.setText(txtSubtitle, m_subtitle);
                t.enableButton(btnImage, m_marked ? FrameGroup::YellowFrame : FrameGroup::NoFrame);
                t.setImage(m_image);
            }
     private:
        String_t m_name;
        String_t m_subtitle;
        String_t m_image;
        bool m_marked;
    };
    class Listener : public client::proxy::ObjectListener {
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
