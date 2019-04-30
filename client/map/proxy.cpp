/**
  *  \file client/map/proxy.cpp
  */

#include <memory>
#include "client/map/proxy.hpp"
#include "afl/base/ptr.hpp"
#include "game/map/renderer.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"

class client::map::Proxy::Trampoline : public util::SlaveObject<game::Session> {
 public:
    Trampoline(util::RequestSender<Proxy> reply)
        : m_reply(reply),
          m_game(),
          m_turn(),
          m_viewport(),
          m_renderer(),
          conn_viewpointTurnChange()
        { }
    ~Trampoline()
        { }

    virtual void init(game::Session& session)
        {
            // Clear possible previous value
            m_renderer.reset();
            m_viewport.reset();

            // Get pointer to game to keep it alive
            m_game = session.getGame();
            if (m_game.get() != 0) {
                conn_viewpointTurnChange = m_game->sig_viewpointTurnChange.add(this, &Trampoline::onViewpointTurnChange);
                m_turn = m_game->getViewpointTurn();
                attachTurn();
            }
        }

    virtual void done(game::Session& /*session*/)
        {
            conn_viewpointTurnChange.disconnect();
            m_renderer.reset();
            m_viewport.reset();
            m_game = 0;
            m_turn = 0;
        }

    void attachTurn()
        {
            if (m_turn.get() != 0 && m_game.get() != 0) {
                // Save previous viewpoint
                std::auto_ptr<game::map::Viewport> oldViewport(m_viewport);

                // Create objects
                m_viewport.reset(new game::map::Viewport(m_turn->universe(), m_game->teamSettings()));
                m_renderer.reset(new game::map::Renderer(*m_viewport));

                // Attach signals
                m_viewport->sig_update.add(this, &Trampoline::onViewportUpdate);
                if (oldViewport.get()) {
                    m_viewport->setOptions(oldViewport->getOptions());
                    m_viewport->setRange(oldViewport->getMin(), oldViewport->getMax());
                }

                // Initial update
                onViewportUpdate();
            }
        }

    void onViewpointTurnChange()
        {
            if (m_game.get() != 0) {
                m_turn = m_game->getViewpointTurn();
                attachTurn();
            }
        }

    void onViewportUpdate()
        {
            if (m_renderer.get() != 0 && m_viewport.get() != 0) {
                afl::base::Ptr<game::map::RenderList> list = new game::map::RenderList();
                m_renderer->render(*list);

                class Reply : public util::Request<Proxy> {
                 public:
                    Reply(afl::base::Ptr<game::map::RenderList> list)
                        : m_list(list)
                        { }
                    virtual void handle(Proxy& p)
                        { p.sig_update.raise(m_list); }
                 private:
                    afl::base::Ptr<game::map::RenderList> m_list;
                };
                m_reply.postNewRequest(new Reply(list));
            }
        }

    void setRange(game::map::Point min, game::map::Point max)
        {
            if (m_viewport.get() != 0) {
                m_viewport->setRange(min, max);
            }
        }

    void setOption(game::map::Viewport::Option option, bool flag)
        {
            if (m_viewport.get() != 0) {
                m_viewport->setOption(option, flag);
            }
        }

 private:
    util::RequestSender<Proxy> m_reply;
    afl::base::Ptr<game::Game> m_game;
    afl::base::Ptr<game::Turn> m_turn;
    std::auto_ptr<game::map::Viewport> m_viewport;
    std::auto_ptr<game::map::Renderer> m_renderer;
    afl::base::SignalConnection conn_viewpointTurnChange;
};


client::map::Proxy::Proxy(util::RequestSender<game::Session> gameSender,
                          util::RequestDispatcher& dispatcher)
    : m_receiver(dispatcher, *this),
      m_trampoline(gameSender, new Trampoline(m_receiver.getSender()))
{ }

client::map::Proxy::~Proxy()
{ }

void
client::map::Proxy::setRange(game::map::Point min, game::map::Point max)
{
    class Request : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Request(game::map::Point min, game::map::Point max)
            : m_min(min), m_max(max)
            { }
        virtual void handle(game::Session& /*session*/, Trampoline& tpl)
            { tpl.setRange(m_min, m_max); }
     private:
        game::map::Point m_min;
        game::map::Point m_max;
    };
    m_trampoline.postNewRequest(new Request(min, max));
}

void
client::map::Proxy::setOption(game::map::Viewport::Option option, bool flag)
{
    class Request : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Request(game::map::Viewport::Option option, bool flag)
            : m_option(option), m_flag(flag)
            { }
        virtual void handle(game::Session& /*session*/, Trampoline& tpl)
            { tpl.setOption(m_option, m_flag); }
     private:
        game::map::Viewport::Option m_option;
        bool m_flag;
    };
    m_trampoline.postNewRequest(new Request(option, flag));
}
