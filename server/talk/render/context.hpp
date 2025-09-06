/**
  *  \file server/talk/render/context.hpp
  *  \brief Class server::talk::render::Context
  */
#ifndef C2NG_SERVER_TALK_RENDER_CONTEXT_HPP
#define C2NG_SERVER_TALK_RENDER_CONTEXT_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "server/talk/linkparser.hpp"
#include "server/talk/root.hpp"

namespace server { namespace talk { namespace render {

    /** Renderer state, trusted part.
        Trusted attributes are:
        - message Id (message we're referring to, for "quote:" format).
          Caller makes sure that this posting is actually accessible to the user.
        - message Author (alternative to message Id to specify author for "quote:" format)
        - user Id (user in whose context permission checks are performed)

        In addition, Context provides an implementation of LinkParser. */
    class Context : public LinkParser {
     public:
        /** Constructor.
            @param root Root (database access for LinkParser)
            @param user Authenticated user */
        Context(Root& root, String_t user);

        /** Set message Id.
            @param id Message Id */
        void setMessageId(int32_t id);

        /** Set message author.
            @param author Author user Id */
        void setMessageAuthor(String_t author);

        /** Get authenticated user.
            @return user Id */
        const String_t& getUser() const;

        /** Get message Id.
            @return message Id; 0 if not set */
        int32_t getMessageId() const;

        /** Get message author.
            @return user Id; empty if not set */
        const String_t& getMessageAuthor() const;

        // LinkParser:
        virtual afl::base::Optional<Result_t> parseGameLink(String_t text) const;
        virtual afl::base::Optional<Result_t> parseForumLink(String_t text) const;
        virtual afl::base::Optional<Result_t> parseTopicLink(String_t text) const;
        virtual afl::base::Optional<Result_t> parseMessageLink(String_t text) const;
        virtual afl::base::Optional<String_t> parseUserLink(String_t text) const;

     private:
        Root& m_root;
        String_t m_user;
        int32_t m_messageId;
        String_t m_messageAuthor;
    };

} } }

#endif
