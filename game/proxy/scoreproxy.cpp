/**
  *  \file game/proxy/scoreproxy.cpp
  *  \brief Class game::proxy::ScoreProxy
  */

#include <cassert>
#include "game/proxy/scoreproxy.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/score/chartbuilder.hpp"
#include "game/score/tablebuilder.hpp"

/*
 *  Trampoline
 */

class game::proxy::ScoreProxy::Trampoline {
 public:
    Trampoline(util::RequestSender<ScoreProxy> reply, Session& session);
    ~Trampoline();

    void getChartVariants(Variants_t& out);
    void getTableVariants(Variants_t& out);
    void getTurns(util::StringList& out);
    void getOverviewInformation(Info& out);

    template<typename T>
    void setOption(void (game::score::ChartBuilder::*fcn)(T), T value);

    template<typename T>
    void setOption(void (game::score::TableBuilder::*fcn)(T), T value);

    void setTableTurnDifferenceIndexes(size_t first, size_t second);

    void sendChartUpdate(game::score::ChartBuilder& b);
    void sendTableUpdate(game::score::TableBuilder& b);

 private:
    util::RequestSender<ScoreProxy> m_reply;
    Session& m_session;
    std::auto_ptr<game::score::ChartBuilder> m_pChartBuilder;
    std::auto_ptr<game::score::TableBuilder> m_pTableBuilder;
};

class game::proxy::ScoreProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<ScoreProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(m_reply, session); }
 private:
    util::RequestSender<ScoreProxy> m_reply;
};


inline
game::proxy::ScoreProxy::Trampoline::Trampoline(util::RequestSender<ScoreProxy> reply, Session& session)
    : m_reply(reply), m_session(session)
{
    const Game* g = session.getGame().get();
    const Root* r = session.getRoot().get();
    if (g != 0 && r != 0) {
        m_pChartBuilder.reset(new game::score::ChartBuilder(g->scores(), r->playerList(), g->teamSettings(), r->hostVersion(), r->hostConfiguration(), session.translator()));
        m_pTableBuilder.reset(new game::score::TableBuilder(g->scores(), r->playerList(), g->teamSettings(), r->hostVersion(), r->hostConfiguration(), session.translator()));
    }
}

game::proxy::ScoreProxy::Trampoline::~Trampoline()
{ }

inline void
game::proxy::ScoreProxy::Trampoline::getChartVariants(Variants_t& out)
{
    if (m_pChartBuilder.get() != 0) {
        out = m_pChartBuilder->getVariants();
    }
}

inline void
game::proxy::ScoreProxy::Trampoline::getTableVariants(Variants_t& out)
{
    if (m_pTableBuilder.get() != 0) {
        out = m_pTableBuilder->getVariants();
    }
}

inline void
game::proxy::ScoreProxy::Trampoline::getTurns(util::StringList& out)
{
    const Game* g = m_session.getGame().get();
    if (g != 0) {
        const game::score::TurnScoreList& scores = g->scores();
        for (size_t i = 0, n = scores.getNumTurns(); i < n; ++i) {
            const game::score::TurnScore* t = scores.getTurnByIndex(i);
            assert(t != 0);
            out.add(t->getTurnNumber(), t->getTimestamp().getTimestampAsString());
        }
    }
}

inline void
game::proxy::ScoreProxy::Trampoline::getOverviewInformation(Info& out)
{
    const Game* g = m_session.getGame().get();
    const Root* r = m_session.getRoot().get();
    out.numTurns        = g != 0 ?  g->scores().getNumTurns() : 0;
    out.hasTeams        = g != 0 && g->teamSettings().hasAnyTeams();
    out.viewpointPlayer = g != 0 ? g->teamSettings().getViewpointPlayer() : 0;
    out.players         = r != 0 ?  r->playerList().getAllPlayers() : PlayerSet_t();
}

template<typename T>
inline void
game::proxy::ScoreProxy::Trampoline::setOption(void (game::score::ChartBuilder::*fcn)(T), T value)
{
    if (m_pChartBuilder.get() != 0) {
        ((*m_pChartBuilder).*fcn)(value);
        sendChartUpdate(*m_pChartBuilder);
    }
}

template<typename T>
inline void
game::proxy::ScoreProxy::Trampoline::setOption(void (game::score::TableBuilder::*fcn)(T), T value)
{
    if (m_pTableBuilder.get() != 0) {
        ((*m_pTableBuilder).*fcn)(value);
        sendTableUpdate(*m_pTableBuilder);
    }
}

inline void
game::proxy::ScoreProxy::Trampoline::setTableTurnDifferenceIndexes(size_t first, size_t second)
{
    if (m_pTableBuilder.get() != 0) {
        m_pTableBuilder->setTurnDifferenceIndexes(first, second);
        sendTableUpdate(*m_pTableBuilder);
    }
}

void
game::proxy::ScoreProxy::Trampoline::sendChartUpdate(game::score::ChartBuilder& b)
{
    class Task : public util::Request<ScoreProxy> {
     public:
        Task(game::score::ChartBuilder& b)
            : m_chart(b.build())
            { }
        virtual void handle(ScoreProxy& proxy)
            { proxy.sig_chartUpdate.raise(m_chart); }
     private:
        std::auto_ptr<util::DataTable> m_chart;
    };
    m_reply.postNewRequest(new Task(b));
}

void
game::proxy::ScoreProxy::Trampoline::sendTableUpdate(game::score::TableBuilder& b)
{
    class Task : public util::Request<ScoreProxy> {
     public:
        Task(game::score::TableBuilder& b)
            : m_chart(b.build())
            { }
        virtual void handle(ScoreProxy& proxy)
            { proxy.sig_tableUpdate.raise(m_chart); }
     private:
        std::auto_ptr<util::DataTable> m_chart;
    };
    m_reply.postNewRequest(new Task(b));
}

/*
 *  ScoreProxy
 */

game::proxy::ScoreProxy::ScoreProxy(util::RequestDispatcher& reply,
                                    util::RequestSender<Session> gameSender)
    : m_reply(reply, *this),
      m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

game::proxy::ScoreProxy::~ScoreProxy()
{ }

void
game::proxy::ScoreProxy::getChartVariants(WaitIndicator& ind, Variants_t& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Variants_t& out)
            : m_out(out)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getChartVariants(m_out); }
     private:
        Variants_t& m_out;
    };
    Task t(out);
    ind.call(m_trampoline, t);
}

void
game::proxy::ScoreProxy::getTableVariants(WaitIndicator& ind, Variants_t& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Variants_t& out)
            : m_out(out)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getTableVariants(m_out); }
     private:
        Variants_t& m_out;
    };
    Task t(out);
    ind.call(m_trampoline, t);
}

void
game::proxy::ScoreProxy::getTurns(WaitIndicator& ind, util::StringList& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(util::StringList& out)
            : m_out(out)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getTurns(m_out); }
     private:
        util::StringList& m_out;
    };
    Task t(out);
    ind.call(m_trampoline, t);
}

void
game::proxy::ScoreProxy::getOverviewInformation(WaitIndicator& ind, Info& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Info& out)
            : m_out(out)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getOverviewInformation(m_out); }
     private:
        Info& m_out;
    };
    Task t(out);
    ind.call(m_trampoline, t);
}

void
game::proxy::ScoreProxy::setChartIndex(size_t index)
{
    setOption(&game::score::ChartBuilder::setVariantIndex, index);
}

void
game::proxy::ScoreProxy::setTableTurnIndex(size_t index)
{
    setOption(&game::score::TableBuilder::setTurnIndex, index);
}

void
game::proxy::ScoreProxy::setTableTurnDifferenceIndexes(size_t first, size_t second)
{
    m_trampoline.postRequest(&Trampoline::setTableTurnDifferenceIndexes, first, second);
}

void
game::proxy::ScoreProxy::setByTeam(bool flag)
{
    setOption(&game::score::ChartBuilder::setByTeam, flag);
    setOption(&game::score::TableBuilder::setByTeam, flag);
}

void
game::proxy::ScoreProxy::setCumulativeMode(bool flag)
{
    setOption(&game::score::ChartBuilder::setCumulativeMode, flag);
}

template<typename Object, typename Parameter>
void
game::proxy::ScoreProxy::setOption(void (Object::*fcn)(Parameter), Parameter value)
{
    m_trampoline.postRequest(&Trampoline::setOption, fcn, value);
}
