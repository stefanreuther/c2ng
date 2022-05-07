/**
  *  \file game/proxy/configurationproxy.hpp
  *  \brief Class game::proxy::ConfigurationProxy
  */
#ifndef C2NG_GAME_PROXY_CONFIGURATIONPROXY_HPP
#define C2NG_GAME_PROXY_CONFIGURATIONPROXY_HPP

#include "game/config/integeroption.hpp"
#include "game/config/markeroption.hpp"
#include "game/config/stringoption.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/numberformatter.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Proxy to access configuration items.
        (As of 20200808, incomplete.) */
    class ConfigurationProxy {
     public:
        /** Constructor.
            \param gameSender Game sender */
        ConfigurationProxy(util::RequestSender<Session> gameSender);

        /** Get number formatter.
            Obtain a formatter to format numbers and population counts according to user's choice.
            \param link Synchronisation */
        util::NumberFormatter getNumberFormatter(WaitIndicator& link);

        /** Get integer option.
            \param link Synchronisation
            \param desc Option descriptor
            \return option value */
        int32_t getOption(WaitIndicator& link, const game::config::IntegerOptionDescriptor& desc);

        /** Get string option.
            \param link Synchronisation
            \param desc Option descriptor
            \return option value */
        String_t getOption(WaitIndicator& link, const game::config::StringOptionDescriptor& desc);

        /** Get marker option.
            \param link Synchronisation
            \param desc Option descriptor
            \return option value */
        game::config::MarkerOption::Data getOption(WaitIndicator& link, const game::config::MarkerOptionDescriptor& desc);

        /** Set integer option.
            \param desc Option descriptor
            \param value New value */
        void setOption(const game::config::IntegerOptionDescriptor& desc, int32_t value);

        /** Set string option.
            \param desc Option descriptor
            \param value New value */
        void setOption(const game::config::StringOptionDescriptor& desc, String_t value);

        /** Set marker option.
            \param desc Option descriptor
            \param value New value */
        void setOption(const game::config::MarkerOptionDescriptor& desc, game::config::MarkerOption::Data value);

     private:
        util::RequestSender<Session> m_gameSender;

        template<typename Desc, typename Value>
        void getOptionTemplate(WaitIndicator& link, const Desc& desc, Value& result);

        template<typename Desc, typename Value>
        void setOptionTemplate(const Desc& desc, const Value& value);
    };

} }

#endif
