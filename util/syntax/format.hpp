/**
  *  \file util/syntax/format.hpp
  *  \brief Enum util::syntax::Format
  */
#ifndef C2NG_UTIL_SYNTAX_FORMAT_HPP
#define C2NG_UTIL_SYNTAX_FORMAT_HPP

namespace util { namespace syntax {

    /** Format category for syntax highlighting. */
    enum Format {
        DefaultFormat,          ///< Nothing in particular. Use default text format.
        KeywordFormat,          ///< A keyword (C++: 'else').
        NameFormat,             ///< A name ('i' in 'int i', 'Dim i'; 'NumMinefields' in 'NumMinefields=1').
        StringFormat,           ///< A string.
        CommentFormat,          ///< A comment.
        Comment2Format,         ///< A special comment, e.g. "### foo" in ini files.
        SectionFormat,          ///< A section divider, e.g. "%PHOST".
        QuoteFormat,            ///< Quoted text, e.g. "> foo" in a message.
        ErrorFormat             ///< Unparseable.
    };

} }

#endif
