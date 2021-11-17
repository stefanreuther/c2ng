/**
  *  \file util/doc/renderoptions.hpp
  *  \brief Class util::doc::RenderOptions
  */
#ifndef C2NG_UTIL_DOC_RENDEROPTIONS_HPP
#define C2NG_UTIL_DOC_RENDEROPTIONS_HPP

#include "afl/string/string.hpp"

namespace util { namespace doc {

    /** Configuration for HTML rendering. */
    class RenderOptions {
     public:
        /** Set prefix for site links ("site:XXXX").
            Should end in a slash, "XXXX" will be appended directly.
            @param s Prefix */
        void setSiteRoot(const String_t& s);

        /** Get prefix for site links.
            @return prefix */
        const String_t& getSiteRoot() const;

        /** Set prefix for asset links ("asset:XXXX").
            Should end in a separator, "XXXX" will be appended directly.
            @param s Prefix */
        void setAssetRoot(const String_t& s);

        /** Get prefix for asset links.
            @return prefix */
        const String_t& getAssetRoot() const;

        /** Set prefix for documentation links ("/xxx").
            Should end in a separator, "xxx" will be appended.
            @param s Prefix */
        void setDocumentRoot(const String_t& s);

        /** Get prefix for documentation links.
            @return prefix */
        const String_t& getDocumentRoot() const;

        /** Set suffix for documentation links ("/xxx").
            Will be appended to links and should therefore begin with a separator.
            @param s Prefix */
        void setDocumentLinkSuffix(const String_t& s);

        /** Get suffix for documentation links.
            @return prefix */
        const String_t& getDocumentLinkSuffix() const;

        /** Set document Id to use for generating local links.
            @param s Document Id
            @see Index::getNodeIdByIndex */
        void setDocumentId(const String_t& s);

        /** Get document Id.
            @return Id */
        const String_t& getDocumentId() const;

        /** Transform a link using the configured parameters.
            @param s Link text
            @return Transformed link */
        String_t transformLink(String_t s) const;

     private:
        String_t m_siteRoot;
        String_t m_assetRoot;
        String_t m_docRoot;
        String_t m_docLinkSuffix;
        String_t m_docId;
    };

} }

#endif
