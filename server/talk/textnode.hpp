/**
  *  \file server/talk/textnode.hpp
  */
#ifndef C2NG_SERVER_TALK_TEXTNODE_HPP
#define C2NG_SERVER_TALK_TEXTNODE_HPP

#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"

// Some @$%#! dumps these macros into our code.
#undef major
#undef minor

namespace server { namespace talk {

    /** Text node. This is our minimal "DOM" for manipulating forum text. */
    struct TextNode {
        enum MajorKind {
            maPlain,                    // Plain text, no children
            maInline,                   // Inline-formatting
            maInlineAttr,               // Inline-formatting with attribute
            maLink,                     // Link
            maParagraph,                // Paragraph. Contains text, inline format, links.
            maGroup,                    // Group. Contains paragraphs or groups.
            maSpecial                   // Specialties
        };

        enum InlineFormat {
            miInBold,                   // Bold
            miInItalic,                 // Italic
            miInStrikeThrough,          // Strikethrough
            miInUnderline,              // Underline
            miInMonospace               // Monospace
        };

        enum InlineAttrFormat {
            miIAColor,                  // Color (color in text, always in "#rrggbb" format)
            miIASize,                   // Size (size in text, always in "+nn" or "-nn" format)
            miIAFont                    // Font (font name in text)
        };

        enum LinkFormat {
            miLinkUrl,                  // URL (link target in text)
            miLinkEmail,                // Email (link target in text)
            miLinkThread,               // Thread link (thread Id in text)
            miLinkPost,                 // Post link (post Id in text)
            miLinkGame,                 // Game link (game Id in text)
            miLinkUser,                 // User link (user Id in text)
            miLinkForum                 // Forum link (forum Id in text)
        };

        enum ParagraphFormat {
            miParNormal,                // normal paragraph
            miParCode,                  // [code], language in text
            miParCentered,              // [center]
            miParBreak,                 // Cut mark for blog entries
            miParFragment               // fragment of a paragraph; for use by ADDRMRENDER
        };

        enum GroupFormat {
            miGroupRoot,                // outermost group. Content is paragraphs or groups.
            miGroupQuote,               // reference in text. Content is paragraphs or groups.
            miGroupList,                // type in text. Content is miGroupListItem.
            miGroupListItem             // list items. Content is paragraphs.
        };

        enum SpecialFormat {
            miSpecialBreak,             // Line break
            miSpecialImage,             // Image (link in text, alt-text in content)
            miSpecialSmiley             // Smiley (name in text)
        };

        TextNode(TextNode::MajorKind major, uint8_t minor)
            : major(major), minor(minor), children(), text()
            { }

        TextNode(TextNode::MajorKind major, uint8_t minor, String_t text)
            : major(major), minor(minor), children(), text(text)
            { }

        TextNode::MajorKind major : 8;  // major type, MajorKind
        uint8_t minor;              // minor type
        afl::container::PtrVector<TextNode> children;
        String_t text;

        bool isSimpleList() const;
        void stripQuotes();
        String_t getTextContent() const;
    };

} }

#endif
