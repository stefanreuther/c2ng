/**
  *  \file game/proxy/configurationproxy.cpp
  *  \brief Class game::proxy::ConfigurationProxy
  */

#include "game/proxy/configurationproxy.hpp"
#include "game/root.hpp"

game::proxy::ConfigurationProxy::ConfigurationProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

util::NumberFormatter
game::proxy::ConfigurationProxy::getNumberFormatter(WaitIndicator& link)
{
    class Query : public util::Request<Session> {
     public:
        Query(util::NumberFormatter& result)
            : m_result(result)
            { }

        virtual void handle(Session& session)
            {
                if (Root* pRoot = session.getRoot().get()) {
                    m_result = pRoot->userConfiguration().getNumberFormatter();
                }
            }
     private:
        util::NumberFormatter& m_result;
    };

    // Initialize with default
    util::NumberFormatter result(true, false);

    Query q(result);
    link.call(m_gameSender, q);

    return result;
}

int32_t
game::proxy::ConfigurationProxy::getOption(WaitIndicator& link, const game::config::IntegerOptionDescriptor& desc)
{
    int32_t result = 0;
    getOptionTemplate(link, desc, result);
    return result;
}

String_t
game::proxy::ConfigurationProxy::getOption(WaitIndicator& link, const game::config::StringOptionDescriptor& desc)
{
    String_t result;
    getOptionTemplate(link, desc, result);
    return result;
}

game::config::MarkerOption::Data
game::proxy::ConfigurationProxy::getOption(WaitIndicator& link, const game::config::MarkerOptionDescriptor& desc)
{
    game::config::MarkerOption::Data result;
    getOptionTemplate(link, desc, result);
    return result;
}

void
game::proxy::ConfigurationProxy::setOption(const game::config::IntegerOptionDescriptor& desc, int32_t value)
{
    setOptionTemplate(desc, value);
}

void
game::proxy::ConfigurationProxy::setOption(const game::config::StringOptionDescriptor& desc, String_t value)
{
    setOptionTemplate(desc, value);
}

void
game::proxy::ConfigurationProxy::setOption(const game::config::MarkerOptionDescriptor& desc, game::config::MarkerOption::Data value)
{
    setOptionTemplate(desc, value);
}

template<typename Desc, typename Value>
inline void
game::proxy::ConfigurationProxy::getOptionTemplate(WaitIndicator& link, const Desc& desc, Value& result)
{
    class Query : public util::Request<Session> {
     public:
        Query(const Desc& desc, Value& result)
            : m_desc(desc), m_result(result)
            { }

        virtual void handle(Session& session)
            {
                if (Root* pRoot = session.getRoot().get()) {
                    m_result = pRoot->userConfiguration()[m_desc]();
                }
            }
     private:
        const Desc& m_desc;
        Value& m_result;
    };
    Query q(desc, result);
    link.call(m_gameSender, q);
}


template<typename Desc, typename Value>
inline void
game::proxy::ConfigurationProxy::setOptionTemplate(const Desc& desc, const Value& value)
{
    class Query : public util::Request<Session> {
     public:
        Query(const Desc& desc, const Value& value)
            : m_desc(desc), m_value(value)
            { }

        virtual void handle(Session& session)
            {
                if (Root* pRoot = session.getRoot().get()) {
                    // Set value
                    typename Desc::OptionType_t& opt = pRoot->userConfiguration()[m_desc];
                    opt.set(m_value);

                    // Set storage location
                    // This is required for options not covered by UserConfiguration::setDefaultValues(),
                    // and when no configuration file was loaded.
                    opt.markUpdated(game::config::ConfigurationOption::User);
                }
            }
     private:
        const Desc& m_desc;
        Value m_value;
    };
    m_gameSender.postNewRequest(new Query(desc, value));
}
