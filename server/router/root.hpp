/**
  *  \file server/router/root.hpp
  */
#ifndef C2NG_SERVER_ROUTER_ROOT_HPP
#define C2NG_SERVER_ROUTER_ROOT_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/sys/log.hpp"
#include "server/common/idgenerator.hpp"
#include "server/interface/filebase.hpp"
#include "server/router/configuration.hpp"
#include "util/process/factory.hpp"

namespace server { namespace router {

    class Session;

    class Root {
     public:
        typedef afl::container::PtrVector<Session> Sessions_t;

        Root(util::process::Factory& factory,
             server::common::IdGenerator& gen,
             const Configuration& config,
             server::interface::FileBase* pFileBase);

        ~Root();

        afl::sys::Log& log();

        Session& createSession(afl::base::Memory<const String_t> args);

        void restartSession(Session& s);

        const Configuration& config() const;

        const Sessions_t& sessions() const;

        Session* getSessionById(String_t id) const;

        void removeExpiredSessions();

        void stopAllSessions();

     private:
        afl::sys::Log m_log;

        util::process::Factory& m_factory;

        server::common::IdGenerator& m_generator;

        server::interface::FileBase* m_pFileBase;
        
        Configuration m_config;

        Sessions_t m_sessions;
    };

} }

#endif
