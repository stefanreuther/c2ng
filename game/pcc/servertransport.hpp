/**
  *  \file game/pcc/servertransport.hpp
  *  \brief Class game::pcc::ServerTransport
  */
#ifndef C2NG_GAME_PCC_SERVERTRANSPORT_HPP
#define C2NG_GAME_PCC_SERVERTRANSPORT_HPP

#include <map>
#include "game/browser/account.hpp"
#include "util/serverdirectory.hpp"

namespace game { namespace pcc {

    class BrowserHandler;

    /** Transport implementation for PlanetsCentral. */
    class ServerTransport : public util::ServerDirectory::Transport {
     public:
        /** Constructor.
            @param handler BrowserHandler instance
            @param acc     Account instance (mutable; might eventually invalidate tokens or update caches)
            @param name    Name
            @param hostGameNumber  Game number on host side (0 for none) */
        ServerTransport(BrowserHandler& handler, const afl::base::Ref<game::browser::Account>& acc, String_t name, int32_t hostGameNumber);

        /** Set target status of turn files in this directory.
            @param flag True to mark temporary; false to mark final. */
        void setTemporaryTurn(bool flag);

        /** Destructor. */
        virtual ~ServerTransport();

        /** Access underlying BrowserHandler.
            @return BrowserHandler */
        BrowserHandler& handler();

        /** Access underlying Account.
            @return Account */
        game::browser::Account& account();

        // Transfer:
        virtual void getFile(String_t name, afl::base::GrowableBytes_t& data);
        virtual void putFile(String_t name, afl::base::ConstBytes_t data);
        virtual void eraseFile(String_t name);
        virtual void getContent(std::vector<util::ServerDirectory::FileInfo>& result);
        virtual bool isValidFileName(String_t name) const;
        virtual bool isWritable() const;

     private:
        BrowserHandler& m_handler;
        const afl::base::Ref<game::browser::Account> m_account;
        const String_t m_name;
        std::map<String_t, String_t> m_urls;
        const int32_t m_hostGameNumber;

        bool m_temporaryTurn;
    };

} }

inline void
game::pcc::ServerTransport::setTemporaryTurn(bool flag)
{
    m_temporaryTurn = flag;
}

inline game::pcc::BrowserHandler&
game::pcc::ServerTransport::handler()
{
    return m_handler;
}

inline game::browser::Account&
game::pcc::ServerTransport::account()
{
    return *m_account;
}

#endif
