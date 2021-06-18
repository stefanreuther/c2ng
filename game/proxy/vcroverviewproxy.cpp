/**
  *  \file game/proxy/vcroverviewproxy.cpp
  *  \brief Class game::proxy::VcrOverviewProxy
  */

#include "game/proxy/vcroverviewproxy.hpp"

using game::vcr::Overview;

/*
 *  Trampoline
 */

class game::proxy::VcrOverviewProxy::Trampoline {
 public:
    Trampoline(VcrDatabaseAdaptor& adaptor)
        : m_overview(adaptor.battles(),
                     adaptor.root().hostConfiguration(),
                     adaptor.shipList()),
          m_adaptor(adaptor)
        { }

    Overview& overview()
        { return m_overview; }

    const Root& root()
        { return m_adaptor.root(); }

    afl::string::Translator& translator()
        { return m_adaptor.translator(); }

 private:
    Overview m_overview;
    VcrDatabaseAdaptor& m_adaptor;
};


/*
 *  TrampolineFromAdaptor
 */

class game::proxy::VcrOverviewProxy::TrampolineFromAdaptor : public afl::base::Closure<Trampoline*(VcrDatabaseAdaptor&)> {
 public:
    virtual Trampoline* call(VcrDatabaseAdaptor& adaptor)
        { return new Trampoline(adaptor); }
};


/*
 *  VcrOverviewProxy
 */

game::proxy::VcrOverviewProxy::VcrOverviewProxy(util::RequestSender<VcrDatabaseAdaptor> sender)
    : m_request(sender.makeTemporary(new TrampolineFromAdaptor()))
{ }

game::proxy::VcrOverviewProxy::~VcrOverviewProxy()
{ }

void
game::proxy::VcrOverviewProxy::buildDiagram(WaitIndicator& ind, game::vcr::Overview::Diagram& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Overview::Diagram& out)
            : m_out(out)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.overview().buildDiagram(m_out, tpl.root().playerList(), tpl.translator()); }
     private:
        Overview::Diagram& m_out;
    };
    Task t(out);
    ind.call(m_request, t);
}

void
game::proxy::VcrOverviewProxy::buildScoreSummary(WaitIndicator& ind, game::vcr::Overview::ScoreSummary& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Overview::ScoreSummary& out)
            : m_out(out)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.overview().buildScoreSummary(m_out); }
     private:
        Overview::ScoreSummary& m_out;
    };
    Task t(out);
    ind.call(m_request, t);
}
