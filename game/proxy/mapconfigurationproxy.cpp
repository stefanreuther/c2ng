/**
  *  \file game/proxy/mapconfigurationproxy.cpp
  *  \brief Class game::proxy::MapConfigurationProxy
  */

#include "game/proxy/mapconfigurationproxy.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

using game::config::MarkerOption;
using game::config::MarkerOptionDescriptor;
using game::map::Configuration;
using game::map::RenderOptions;

game::proxy::MapConfigurationProxy::MapConfigurationProxy(util::RequestSender<Session> gameSender)
    : ConfigurationProxy(gameSender)
{ }

void
game::proxy::MapConfigurationProxy::getMapConfiguration(WaitIndicator& ind, game::map::Configuration& config)
{
    class Task : public util::Request<Session> {
     public:
        Task(Configuration& config)
            : m_config(config)
            { }
        virtual void handle(Session& session)
            {
                if (Game* g = session.getGame().get()) {
                    m_config = g->mapConfiguration();
                }
            }
     private:
        Configuration& m_config;
    };
    Task t(config);
    ind.call(gameSender(), t);
}

void
game::proxy::MapConfigurationProxy::setMapConfiguration(const game::map::Configuration& config)
{
    class Task : public util::Request<Session> {
     public:
        Task(const Configuration& config)
            : m_config(config)
            { }
        virtual void handle(Session& session)
            {
                if (Game* g = session.getGame().get()) {
                    // Set configuration
                    Configuration& mapConfig = g->mapConfiguration();
                    mapConfig = m_config;

                    // Mark universe as changed to trigger redraw
                    g->currentTurn().universe().markChanged();

                    // Update user configuration
                    if (Root* r = session.getRoot().get()) {
                        mapConfig.saveToConfiguration(r->userConfiguration(), r->hostConfiguration());
                    }
                }
            }
     private:
        Configuration m_config;
    };
    gameSender().postNewRequest(new Task(config));
}

game::map::RenderOptions
game::proxy::MapConfigurationProxy::getRenderOptions(WaitIndicator& ind, game::map::RenderOptions::Area a)
{
    class Task : public util::Request<Session> {
     public:
        Task(RenderOptions& config, RenderOptions::Area area)
            : m_config(config), m_area(area)
            { }
        virtual void handle(Session& session)
            {
                if (Root* r = session.getRoot().get()) {
                    m_config = RenderOptions::fromConfiguration(r->userConfiguration(), m_area);
                }
            }
     private:
        RenderOptions& m_config;
        RenderOptions::Area m_area;
    };
    RenderOptions config;
    Task t(config, a);
    ind.call(gameSender(), t);
    return config;
}

void
game::proxy::MapConfigurationProxy::setRenderOptions(game::map::RenderOptions::Area area, const game::map::RenderOptions& opts)
{
    class Task : public util::Request<Session> {
     public:
        Task(RenderOptions::Area area, const RenderOptions& config)
            : m_area(area), m_config(config)
            { }
        virtual void handle(Session& session)
            {
                if (Root* r = session.getRoot().get()) {
                    m_config.storeToConfiguration(r->userConfiguration(), m_area);
                }
            }
     private:
        RenderOptions::Area m_area;
        RenderOptions m_config;
    };
    gameSender().postNewRequest(new Task(area, opts));
}

void
game::proxy::MapConfigurationProxy::getMarkerConfiguration(WaitIndicator& ind, std::vector<game::config::MarkerOption::Data>& config)
{
    class Task : public util::Request<Session> {
     public:
        Task(std::vector<MarkerOption::Data>& config)
            : m_config(config)
            { }
        virtual void handle(Session& session)
            {
                if (Root* r = session.getRoot().get()) {
                    int index = 0;
                    while (const MarkerOptionDescriptor* opt = game::config::UserConfiguration::getCannedMarker(index)) {
                        m_config.push_back(r->userConfiguration()[*opt]());
                        ++index;
                    }
                }
            }
     private:
        std::vector<MarkerOption::Data>& m_config;
    };
    config.clear();
    Task t(config);
    ind.call(gameSender(), t);
}

void
game::proxy::MapConfigurationProxy::setMarkerConfiguration(size_t index, const game::config::MarkerOption::Data& config)
{
    if (const MarkerOptionDescriptor* desc = game::config::UserConfiguration::getCannedMarker(int(index))) {
        setOption(*desc, config);
    }
}
