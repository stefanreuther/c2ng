/**
  *  \file server/mailout/template.hpp
  *  \brief Class server::mailout::Template
  */
#ifndef C2NG_SERVER_MAILOUT_TEMPLATE_HPP
#define C2NG_SERVER_MAILOUT_TEMPLATE_HPP

#include <list>
#include <map>
#include <memory>
#include "afl/io/textreader.hpp"
#include "afl/net/mimebuilder.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/string/string.hpp"

namespace server { namespace mailout {

    /** Mailout template engine.
        Contains logic to build a mail message from a template file, variables, and possible attachments.
        Messages without attachments will be regular single-part MIME messages,
        messages with attachments will be multi-part.

        Template syntax is a stripped-down version of the web interface template engine.
        - "$(foo)" expands to a variable (case sensitive)
        - "!if $(foo) / !else / !endif" delimits conditionals; a condition is true if consists not entirely of whitespace and '0'
        - "!set name value" to set a variable
        - "!- ..." for comments */
    class Template {
     public:
        /** Default constructor. */
        Template();

        /** Destructor. */
        ~Template();

        /** Add a variable value for expansion.
            \param name Variable name (case sensitive)
            \param value Value */
        void addVariable(String_t name, String_t value);

        /** Add a file as attachment.
            The file is given as an URL that is resolved when the message is sent.
            \param url File to attach. Must have the form "c2file://[user@]host:port/path/file". */
        void addFile(String_t url);

        /** Build message from configured parameters.
            \param in Template file.
            \param net Network stack to resolve attachment links with
            \param forUser User to use for access checks if attachment links don't contain one
            \param smtpAddress Receiver address (SMTP address) to use for "To" header
            \return newly-allocated MimeBuilder containing the message; not null */
        std::auto_ptr<afl::net::MimeBuilder> generate(afl::io::TextReader& in, afl::net::NetworkStack& net, String_t forUser, String_t smtpAddress);

     private:
        struct ConditionState;

        void processLine(afl::net::MimeBuilder& result, ConditionState& state, String_t text);
        void processCommand(ConditionState& state, String_t text);
        String_t expand(String_t text) const;

        std::map<String_t, String_t> m_variables;
        std::list<String_t> m_attachments;
    };

} }

#endif
