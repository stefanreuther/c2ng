/**
  *  \file client/tiles/basescreenheadertile.cpp
  */

#include "client/tiles/basescreenheadertile.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "afl/string/format.hpp"
#include "ui/res/resid.hpp"
#include "client/objectlistener.hpp"
#include "game/types.hpp"

using ui::widgets::FrameGroup;

client::tiles::BaseScreenHeaderTile::BaseScreenHeaderTile(ui::Root& root, client::widgets::KeymapWidget& kmw)
    : ControlScreenHeader(root, kmw),
      m_receiver(root.engine().dispatcher(), *this)
{
    // ex WBaseScreenHeaderTile::WBaseScreenHeaderTile
    enableButton(btnAuto, FrameGroup::NoFrame);
    enableButton(btnAdd,  FrameGroup::NoFrame);

    // FIXME: alternative personalities
    //   Base Task: CScr
    //   Base:      Auto + Add
}

void
client::tiles::BaseScreenHeaderTile::attach(ObjectObserverProxy& oop)
{
    class Job : public util::Request<ControlScreenHeader> {
     public:
        Job(game::Session& session, game::map::Object* obj)
            : m_name(obj != 0 ? obj->getName(game::map::Object::PlainName, session.translator(), session.interface()) : String_t()),
              m_subtitle(),
              m_marked(obj != 0 && obj->isMarked())
            {
                game::map::Planet* p = dynamic_cast<game::map::Planet*>(obj);
                game::Game* g = session.getGame().get();
                game::Root* r = session.getRoot().get();
                afl::string::Translator& tx = session.translator();
                if (p != 0 && g != 0 && r != 0) {
                    // ex WBaseScreenHeaderTile::getSubtitle
                    game::UnitScoreList::Index_t index = 0;
                    int16_t level = 0, turn = 0;
                    bool levelKnown = (g->planetScores().lookup(game::ScoreId_ExpLevel, index) && p->unitScores().get(index, level, turn));
                    String_t fmt = (levelKnown
                                    ? tx.translateString("(Id #%d, %s)")
                                    : tx.translateString("(Id #%d)"));
                    m_subtitle = afl::string::Format(fmt.c_str(),
                                                     p->getId(),
                                                     r->hostConfiguration().getExperienceLevelName(level, session.translator()));

                    // ex WBaseScreenHeaderTile::getPictureId
                    int tech = std::max(std::max(p->getBaseTechLevel(game::EngineTech).orElse(0),
                                                 p->getBaseTechLevel(game::HullTech).orElse(0)),
                                        std::max(p->getBaseTechLevel(game::BeamTech).orElse(0),
                                                 p->getBaseTechLevel(game::TorpedoTech).orElse(0)));
                    m_image = ui::res::makeResourceId(ui::res::BASE, tech, p->getId());
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
    class Listener : public ObjectListener {
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
