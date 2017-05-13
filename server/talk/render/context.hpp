/**
  *  \file server/talk/render/context.hpp
  */
#ifndef C2NG_SERVER_TALK_RENDER_CONTEXT_HPP
#define C2NG_SERVER_TALK_RENDER_CONTEXT_HPP

#include "afl/string/string.hpp"
#include "afl/base/types.hpp"

namespace server { namespace talk { namespace render {

    /** Renderer state, trusted part.
        Trusted attributes are:
        - message Id (message we're referring to, for "quote:" format).
          Caller makes sure that this posting is actually accessible to the user.
        - message Author (alternative to message Id to specify author for "quote:" format)
        - user Id (user in whose context permission checks are performed) */
    class Context {
     public:
        explicit Context(String_t user);

        void setMessageId(int32_t id);
        void setMessageAuthor(String_t author);

        const String_t& getUser() const;
        int32_t getMessageId() const;
        const String_t& getMessageAuthor() const;

     private:
        String_t m_user;
        int32_t m_messageId;
        String_t m_messageAuthor;
    };

} } }

#endif
