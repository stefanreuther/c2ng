/**
  *  \file server/play/imperialstatspacker.hpp
  *  \brief Class server::play::ImperialStatsPacker
  */
#ifndef C2NG_SERVER_PLAY_IMPERIALSTATSPACKER_HPP
#define C2NG_SERVER_PLAY_IMPERIALSTATSPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "query/istatX.Y".
        - X is a page.
        - Y contains the options (typically, a bitset) */
    class ImperialStatsPacker : public Packer {
     public:
        /** Constructor.
            @param session Session
            @param page    Page
            @param opts    Options */
        ImperialStatsPacker(game::Session& session, int page, int opts);

        // Packer:
        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Session& m_session;
        const int m_page;
        const int m_options;
    };

} }

#endif
