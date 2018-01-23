/**
  *  \file server/host/resultsender.hpp
  *  \brief Class server::host::ResultSender
  */
#ifndef C2NG_SERVER_HOST_RESULTSENDER_HPP
#define C2NG_SERVER_HOST_RESULTSENDER_HPP

namespace server { namespace host {

    class Root;
    class Game;

    /** Result sender.
        Contains the logic for producing game mails. */
    class ResultSender {
     public:
        /** Constructor.
            \param root Service root
            \param game Game */
        ResultSender(Root& root, Game& game);

        /** Send all result files.
            Call after a host run. */
        void sendAllResults();

     private:
        Root& m_root;
        Game& m_game;
    };

} }

#endif
