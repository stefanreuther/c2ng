/**
  *  \file server/nntp/linehandler.hpp
  *  \brief Class server::nntp::LineHandler
  */
#ifndef C2NG_SERVER_NNTP_LINEHANDLER_HPP
#define C2NG_SERVER_NNTP_LINEHANDLER_HPP

#include "afl/net/line/linehandler.hpp"

namespace server { namespace nntp {

    class Root;
    class Session;

    /** LineHandler for NNTP.
        This serves one NNTP connection.
        The main state is in a Session; the protocol parsing is in class LineHandler. */
    class LineHandler : public afl::net::line::LineHandler {
     public:
        LineHandler(Root& root, Session& session);
        ~LineHandler();

        virtual bool handleOpening(afl::net::line::LineSink& response);
        virtual bool handleLine(const String_t& line, afl::net::line::LineSink& response);
        virtual void handleConnectionClose();

     private:
        Root& m_root;
        Session& m_session;
        const int32_t m_id;

        /* Input Parser Status */
        enum Status {
            ReadCommand,
            ReadPostData
        };
        Status m_status;


        bool handleCommand(String_t line, afl::net::line::LineSink& response);
        bool handlePostData(String_t line, afl::net::line::LineSink& response);

        bool checkAuth(afl::net::line::LineSink& response);
        bool fillGroupListCache(afl::net::line::LineSink& response);
        int32_t resolveSequenceNumber(int32_t seq, String_t& rfcMsgId, afl::net::line::LineSink& response);
        bool parseRange(const String_t& range, int32_t& min, int32_t& max, afl::net::line::LineSink& response);
        bool enterGroup(const String_t& groupName, afl::net::line::LineSink& response);

        /* Commands */
        void handleArticle(String_t args, bool header, bool body, afl::net::line::LineSink& response);
        void handleAuthinfo(String_t args, afl::net::line::LineSink& response);
        void handleGroup(String_t args, afl::net::line::LineSink& response);
        void handleList(String_t args, afl::net::line::LineSink& response);
        void handleListActive(afl::net::line::LineSink& response);
        void handleListNewsgroups(afl::net::line::LineSink& response);
        void handleListSubscriptions(afl::net::line::LineSink& response);
        void handleListOverviewFormat(afl::net::line::LineSink& response);
        void handleListGroup(String_t args, afl::net::line::LineSink& response);
        void handleHelp(afl::net::line::LineSink& response);
        void handleOver(String_t args, afl::net::line::LineSink& response);
    };

} }

#endif
