/**
  *  \file client/tiles/basescreenheadertile.cpp
  */

#include "client/tiles/basescreenheadertile.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/root.hpp"
#include "game/types.hpp"
#include "ui/res/resid.hpp"

using ui::widgets::FrameGroup;
using client::widgets::getFrameTypeFromTaskStatus;

client::tiles::BaseScreenHeaderTile::BaseScreenHeaderTile(ui::Root& root, gfx::KeyEventConsumer& kmw, bool forTask)
    : ControlScreenHeader(root, kmw),
      m_receiver(root.engine().dispatcher(), *this),
      m_forTask(forTask)
{
    // ex WBaseScreenHeaderTile::WBaseScreenHeaderTile
    if (forTask) {
        enableButton(btnCScr, ui::NoFrame);
    } else {
        enableButton(btnAuto, ui::NoFrame);
        enableButton(btnAdd,  ui::NoFrame);
    }
}

void
client::tiles::BaseScreenHeaderTile::attach(game::proxy::ObjectObserver& oop)
{
    class Job : public util::Request<ControlScreenHeader> {
     public:
        Job(game::Session& session, game::map::Object* obj, bool forTask)
            : m_name(obj != 0 ? obj->getName(game::PlainName, session.translator(), session.interface()) : String_t()),
              m_subtitle(),
              m_marked(obj != 0 && obj->isMarked()),
              m_forTask(forTask),
              m_hasMessages(false),
              m_taskStatus(session.getTaskStatus(obj, interpreter::Process::pkBaseTask, forTask))
            {
                game::map::Planet* p = dynamic_cast<game::map::Planet*>(obj);
                game::Game* g = session.getGame().get();
                game::Root* r = session.getRoot().get();
                afl::string::Translator& tx = session.translator();
                if (p != 0 && g != 0 && r != 0) {
                    // ex WBaseScreenHeaderTile::getSubtitle
                    int level = 0;
                    bool levelKnown = p->unitScores().getScoreById(game::ScoreId_ExpLevel, g->planetScores()).get(level);
                    String_t fmt = (levelKnown
                                    ? tx.translateString("(Id #%d, %s)")
                                    : tx.translateString("(Id #%d)"));
                    m_subtitle = afl::string::Format(fmt.c_str(),
                                                     p->getId(),
                                                     r->hostConfiguration().getExperienceLevelName(level, session.translator()));

                    // ex WBaseScreenHeaderTile::getPictureId
                    m_image = ui::res::makeResourceId(ui::res::BASE, p->getMaxBaseTechLevel().orElse(0), p->getId());
                    m_hasMessages = (!m_forTask && !p->messages().empty());
                }
            }
        void handle(ControlScreenHeader& t)
            {
                t.setText(txtHeading, m_name);
                t.setText(txtSubtitle, m_subtitle);
                t.setHasMessages(m_hasMessages);
                t.enableButton(btnImage, m_marked ? ui::YellowFrame : ui::NoFrame);
                if (m_forTask) {
                    t.enableButton(btnCScr, getFrameTypeFromTaskStatus(m_taskStatus));
                } else {
                    t.enableButton(btnAuto, getFrameTypeFromTaskStatus(m_taskStatus));
                }
                t.setImage(m_image);
            }
     private:
        String_t m_name;
        String_t m_subtitle;
        String_t m_image;
        bool m_marked;
        bool m_forTask;
        bool m_hasMessages;
        game::Session::TaskStatus m_taskStatus;
    };
    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<ControlScreenHeader> reply, bool forTask)
            : m_reply(reply), m_forTask(forTask)
            { }
        virtual void handle(game::Session& s, game::map::Object* obj)
            { m_reply.postNewRequest(new Job(s, obj, m_forTask)); }
     private:
        util::RequestSender<ControlScreenHeader> m_reply;
        bool m_forTask;
    };

    oop.addNewListener(new Listener(m_receiver.getSender(), m_forTask));
}
