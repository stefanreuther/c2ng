/**
  *  \file game/proxy/costsummaryadaptor.cpp
  *  \brief Class game::proxy::CostSummaryAdaptor
  */

#include "game/proxy/costsummaryadaptor.hpp"
#include "game/interface/costsummarycontext.hpp"

game::proxy::CostSummaryAdaptor::CostSummaryAdaptor(afl::io::FileSystem& fs, afl::string::Translator& tx, afl::base::Ptr<game::spec::CostSummary> costSummary)
    : m_fileSystem(fs),
      m_translator(tx),
      m_costSummary(costSummary)
{ }

game::proxy::CostSummaryAdaptor::~CostSummaryAdaptor()
{ }

void
game::proxy::CostSummaryAdaptor::initConfiguration(interpreter::exporter::Configuration& config)
{
    // ex client/billexport.cc:DEFAULT_FORMAT
    config.fieldList().addList("COUNT@5,NAME@40,T@6,D@6,M@6,CASH@6");
}

void
game::proxy::CostSummaryAdaptor::saveConfiguration(const interpreter::exporter::Configuration& /*config*/)
{
    // Changes are not persisted
}

interpreter::Context*
game::proxy::CostSummaryAdaptor::createContext()
{
    return game::interface::CostSummaryContext::create(m_costSummary);
}

afl::io::FileSystem&
game::proxy::CostSummaryAdaptor::fileSystem()
{
    return m_fileSystem;
}

afl::string::Translator&
game::proxy::CostSummaryAdaptor::translator()
{
    return m_translator;
}

afl::base::Closure<game::proxy::ExportAdaptor*(game::Session&)>*
game::proxy::makeCostSummaryAdaptor(const game::spec::CostSummary& costSummary)
{
    class AdaptorFromSession : public afl::base::Closure<game::proxy::ExportAdaptor*(game::Session&)> {
     public:
        AdaptorFromSession(afl::base::Ptr<game::spec::CostSummary> costSummary)
            : m_costSummary(costSummary)
            { }
        virtual ExportAdaptor* call(Session& session)
            { return new CostSummaryAdaptor(session.world().fileSystem(), session.translator(), m_costSummary); }
     private:
        afl::base::Ptr<game::spec::CostSummary> m_costSummary;
    };
    afl::base::Ptr<game::spec::CostSummary> cs = new game::spec::CostSummary(costSummary);
    return new AdaptorFromSession(cs);
}
