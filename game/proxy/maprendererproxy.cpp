/**
  *  \file game/proxy/maprendererproxy.cpp
  *  \brief Class game::proxy::MapRendererProxy
  */

#include <memory>
#include "game/proxy/maprendererproxy.hpp"
#include "afl/base/ptr.hpp"
#include "game/game.hpp"
#include "game/interface/labelextra.hpp"
#include "game/map/renderer.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

using afl::base::Ptr;
using game::interface::LabelExtra;
using game::map::RenderOptions;
using game::map::Viewport;
using game::map::Renderer;
using game::map::RenderList;
using game::map::Point;

class game::proxy::MapRendererProxy::Trampoline {
 public:
    Trampoline(game::Session& session, const util::RequestSender<MapRendererProxy>& reply);

    void attachTurn();
    void onViewpointTurnChange();
    void onPreferencesChange();
    void onViewportUpdate();

    void setConfiguration(RenderOptions::Area area);
    void setRange(Point min, Point max);
    void toggleOptions(RenderOptions::Options_t opts);
    void setDrawingTagFilter(util::Atom_t tag);
    void clearDrawingTagFilter();
    void setShipTrailId(Id_t id);

 private:
    void loadOptions();

    util::RequestSender<MapRendererProxy> m_reply;
    Session& m_session;
    Ptr<Game> m_game;
    Ptr<Turn> m_turn;
    Ptr<Root> m_root;
    Ptr<game::spec::ShipList> m_shipList;
    std::auto_ptr<Viewport> m_viewport;
    std::auto_ptr<Renderer> m_renderer;
    RenderOptions::Area m_area;
    afl::base::SignalConnection conn_viewpointTurnChange;
    afl::base::SignalConnection conn_prefChange;
};

game::proxy::MapRendererProxy::Trampoline::Trampoline(game::Session& session, const util::RequestSender<MapRendererProxy>& reply)
    : m_reply(reply),
      m_session(session),
      m_game(),
      m_turn(),
      m_root(),
      m_shipList(),
      m_viewport(),
      m_renderer(),
      m_area(RenderOptions::Normal),
      conn_viewpointTurnChange(),
      conn_prefChange()
{
    // Get pointer to game to keep it alive
    m_game = session.getGame();
    m_root = session.getRoot();
    m_shipList = session.getShipList();
    if (m_game.get() != 0) {
        conn_viewpointTurnChange = m_game->sig_viewpointTurnChange.add(this, &Trampoline::onViewpointTurnChange);
        m_turn = m_game->getViewpointTurn();
        attachTurn();
    }
    if (m_root.get() != 0) {
        conn_prefChange = m_root->userConfiguration().sig_change.add(this, &Trampoline::onPreferencesChange);
    }
}

void
game::proxy::MapRendererProxy::Trampoline::attachTurn()
{
    if (m_turn.get() != 0 && m_game.get() != 0 && m_root.get() != 0 && m_shipList.get() != 0) {
        // Save previous viewpoint
        std::auto_ptr<Viewport> oldViewport(m_viewport);

        // Create objects
        m_viewport.reset(new Viewport(m_turn->universe(), m_turn->getTurnNumber(), m_game->teamSettings(),
                                      LabelExtra::get(m_session), m_game->shipScores(), *m_shipList, m_game->mapConfiguration(), m_root->hostConfiguration(), m_root->hostVersion()));
        m_renderer.reset(new Renderer(*m_viewport));
        loadOptions();

        // Attach signals
        if (oldViewport.get()) {
            m_viewport->setRange(oldViewport->getMin(), oldViewport->getMax());
        }
        m_viewport->sig_update.add(this, &Trampoline::onViewportUpdate);

        // Initial update
        onViewportUpdate();
    }
}

void
game::proxy::MapRendererProxy::Trampoline::onViewpointTurnChange()
{
    if (m_game.get() != 0) {
        m_turn = m_game->getViewpointTurn();
        attachTurn();
    }
}

void
game::proxy::MapRendererProxy::Trampoline::onPreferencesChange()
{
    loadOptions();
}

void
game::proxy::MapRendererProxy::Trampoline::onViewportUpdate()
{
    if (m_renderer.get() != 0 && m_viewport.get() != 0) {
        Ptr<RenderList> list = new RenderList();
        m_renderer->render(*list);

        class Reply : public util::Request<MapRendererProxy> {
         public:
            Reply(Ptr<RenderList> list)
                : m_list(list)
                { }
            virtual void handle(MapRendererProxy& p)
                { p.sig_update.raise(m_list); }
         private:
            Ptr<RenderList> m_list;
        };
        m_reply.postNewRequest(new Reply(list));
    }
}

void
game::proxy::MapRendererProxy::Trampoline::setConfiguration(RenderOptions::Area area)
{
    m_area = area;
    loadOptions();
}

void
game::proxy::MapRendererProxy::Trampoline::setRange(Point min, Point max)
{
    if (m_viewport.get() != 0) {
        m_viewport->setRange(min, max);
    }
}

void
game::proxy::MapRendererProxy::Trampoline::toggleOptions(RenderOptions::Options_t opts)
{
    if (m_root.get() != 0 && m_viewport.get() != 0) {
        RenderOptions parsedOptions = RenderOptions::fromConfiguration(m_root->userConfiguration(), m_area);
        parsedOptions.toggleOptions(opts);
        parsedOptions.storeToConfiguration(m_root->userConfiguration(), m_area);
        m_viewport->setOptions(parsedOptions.getViewportOptions()); /* triggers update */

        // We're not doing session.notifyListeners() here (should we?).
        // To be internally consistent, explicitly forward the changed options.
        m_reply.postRequest(&MapRendererProxy::emitConfiguration, parsedOptions);
    }
}

void
game::proxy::MapRendererProxy::Trampoline::setDrawingTagFilter(util::Atom_t tag)
{
    if (m_viewport.get() != 0) {
        m_viewport->setDrawingTagFilter(tag);
    }
}

void
game::proxy::MapRendererProxy::Trampoline::clearDrawingTagFilter()
{
    if (m_viewport.get() != 0) {
        m_viewport->clearDrawingTagFilter();
    }
}

void
game::proxy::MapRendererProxy::Trampoline::setShipTrailId(Id_t id)
{
    if (m_viewport.get() != 0) {
        m_viewport->setShipTrailId(id);
    }
}

void
game::proxy::MapRendererProxy::Trampoline::loadOptions()
{
    if (m_viewport.get() != 0 && m_root.get() != 0) {
        RenderOptions opts = RenderOptions::fromConfiguration(m_root->userConfiguration(), m_area);
        m_viewport->setOptions(opts.getViewportOptions());
        m_reply.postRequest(&MapRendererProxy::emitConfiguration, opts);
    }
}




/*
 *  TrampolineFromSession
 */

class game::proxy::MapRendererProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(game::Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<MapRendererProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(game::Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<MapRendererProxy> m_reply;
};

game::proxy::MapRendererProxy::MapRendererProxy(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& dispatcher)
    : m_receiver(dispatcher, *this),
      m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender())))
{ }

game::proxy::MapRendererProxy::~MapRendererProxy()
{ }

void
game::proxy::MapRendererProxy::setConfiguration(game::map::RenderOptions::Area area)
{
    m_trampoline.postRequest(&Trampoline::setConfiguration, area);
}

void
game::proxy::MapRendererProxy::setRange(game::map::Point min, game::map::Point max)
{
    m_trampoline.postRequest(&Trampoline::setRange, min, max);
}

void
game::proxy::MapRendererProxy::toggleOptions(game::map::RenderOptions::Options_t opts)
{
    m_trampoline.postRequest(&Trampoline::toggleOptions, opts);
}

void
game::proxy::MapRendererProxy::setDrawingTagFilter(util::Atom_t tag)
{
    m_trampoline.postRequest(&Trampoline::setDrawingTagFilter, tag);
}

void
game::proxy::MapRendererProxy::clearDrawingTagFilter()
{
    m_trampoline.postRequest(&Trampoline::clearDrawingTagFilter);
}

void
game::proxy::MapRendererProxy::setShipTrailId(Id_t id)
{
    m_trampoline.postRequest(&Trampoline::setShipTrailId, id);
}

void
game::proxy::MapRendererProxy::emitConfiguration(game::map::RenderOptions opts)
{
    sig_configuration.raise(opts);
}
