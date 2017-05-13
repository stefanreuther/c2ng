/**
  *  \file server/talk/inlinerecognizer.hpp
  *  \brief Class server::talk::InlineRecognizer
  */
#ifndef C2NG_SERVER_TALK_INLINERECOGNIZER_HPP
#define C2NG_SERVER_TALK_INLINERECOGNIZER_HPP

#include "afl/string/string.hpp"
#include "afl/base/types.hpp"
#include "afl/bits/smallset.hpp"

namespace server { namespace talk {

    /** Inline object recognizer.
        This recognizes links and smileys that are embedded in text without special markup.

        This is a class to allow storage of configuration.
        As of 20170122, this still uses the constant configuration as in the original version. */
    class InlineRecognizer {
     public:
        /** Defines a smiley. */
        struct SmileyDefinition {
            const char* name;                     /**< Canonical name of smiley ("wink"). */
            const char* symbol;                   /**< Symbol of smiley (";-)"). */
            const char* symbol2;                  /**< Alternative symbol. */
            const char* image;                    /**< Name of image file ("res/smileys/wink.png"). */
            int16_t width, height;                /**< Size of image file to use in forum. */
        };

        /** Kind of a recognized element. */
        enum Kind {
            Smiley,                               /**< Smiley. Text is smiley name. */
            Link                                  /**< Link. Text is link. */
        };
        typedef afl::bits::SmallSet<Kind> Kinds_t;

        /** Information about recognized element. */
        struct Info {
            Kind kind;                            /**< Type of element. */
            String_t::size_type start;            /**< Starting position of element in string passed by user. */
            String_t::size_type length;           /**< Length of element in string passed by user. */
            String_t text;                        /**< Text of recognized element (smiley name, URL, ...). */
        };

        /** Constructor.
            Makes blank object. */
        InlineRecognizer();

        /** Destructor. */
        ~InlineRecognizer();

        /** Get smiley definition.
            \param name Name of smiley (e.g. from Info::text).
            \return Definition of smiley if known; null if unknown. */
        const SmileyDefinition* getSmileyDefinitionByName(const String_t& name) const;

        /** Find an inline element.
            \param text     [in] Text to scan
            \param start_at [in] Start scanning at this position
            \param what     [in] What to find
            \param info     [out] Result
            \retval true Found an element, info has been updated.
            \retval false Not found, info has indeterminate content

            The returned element will have
            - info.start >= start_at
            - info.length > 0
            To find the first element, call find() with start_at=0.
            To find the next element, call find() with start_at=info.start+info.length.
            Do not simply delete the start/length range and call again with start_at=0;
            this may return false matches by not noting a non-word-boundary. */
        bool find(const String_t& text, String_t::size_type start_at, Kinds_t what, Info& info) const;

     private:
        String_t m_firsts;
    };

} }

#endif
