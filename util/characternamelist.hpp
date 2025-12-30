/**
  *  \file util/characternamelist.hpp
  *  \brief Class util::CharacterNameList
  */
#ifndef C2NG_UTIL_CHARACTERNAMELIST_HPP
#define C2NG_UTIL_CHARACTERNAMELIST_HPP

#include <map>
#include <vector>
#include "afl/charset/unicode.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"

namespace util {

    /** Character name list.
        Stores a mapping of unicode characters to names, and possible aliases.
        This supports the use cases
        - naming characters
        - generating characters
        in reshack. */
    class CharacterNameList {
     public:
        /** List of characters. */
        typedef std::vector<afl::charset::Unichar_t> CharacterList_t;

        /** Character generator interface.
            @see CharacterNameList::generateCharacter() */
        class Generator {
         public:
            /** Virtual destructor. */
            virtual ~Generator()
                { }
            /** Check character.
                @param set Set of characters to combine
                @return true if the target character could be generated from the given set */
            virtual bool check(afl::base::Memory<const afl::charset::Unichar_t> set) = 0;
        };

        /** Constructor. Make an empty list. */
        CharacterNameList();

        /** Destructor. */
        ~CharacterNameList();

        /** Add default character names. */
        void addDefault();

        /** Load name list file.
            This loads the unicode NamesList.txt file.
            For now, we accept sections of the form
            <pre>0023  NUMBER SIGN</pre>
            and
            <pre>001B  &lt;control&gt;
                    = ESCAPE</pre>.

            @param in File */
        void loadNames(afl::io::Stream& in);

        /** Load alias file.
            This loads the alias.txt file.
            The file consists of lines of the form "NAME = ALIAS",
            stating that character NAME looks like ALIAS.
            On both sides, "?" can be used as shortcut for "CAPITAL LETTER or SMALL LETTER".
            @param in File */
        void loadAliases(afl::io::Stream& in);

        /** Get name of a character.
            @param ch Character
            @return Character name; guaranteed to not be empty. */
        String_t getCharacterName(afl::charset::Unichar_t ch) const;

        /** Find character, given a name.
            Searches for exact match only.
            @param name Name to find
            @return Character; 0 if none found */
        afl::charset::Unichar_t findCharacterByName(const String_t& name) const;

        /** Enumerate all characters that match a given expression.
            This checks for several criteria at once, and sorts the most likely results at the beginning:
            - character numbers (hex)
            - single characters
            - substring matches, where a match at the beginning of a word
              is worth more than one inside a word, and a match immediately
              after LETTER or DIGIT is worth most (i.e. "OM" matches first
              all the "LETTER OMEGA"s, then the "COMMA".

            @param expr         Expression to search for
            @param maxCharCode  Maximum character code to return (i.e. this will not return characters whose code is above this even if they match)
            @return list */
        CharacterList_t searchCharactersByName(String_t expr, afl::charset::Unichar_t maxCharCode);

        /** Attempt to generate a character.
            This calls Generator::check() with a list of characters, possibly multiple times,
            to generate a character by combining these characters.
            check() can return true if it can actually handle the combination;
            in this case, generate() will return true.
            If no combination is found or check() rejects them all, it will return false.

            For now, this does a syntactic search using character names and aliases.
            This problem can also partially be solved by using the ":" lines from the name list
            (i.e. "00C2 LATIN CAPITAL LETTER A WITH CIRCUMFLEX", described as ": 0041 0302").
            It doesn't help for all cases, i.e. generating CYRILLIC A from LATIN A.

            @param ch Character to generate
            @param gen Generator
            @return success status */
        bool generateCharacter(afl::charset::Unichar_t ch, Generator& gen) const;

     private:
        typedef std::map<afl::charset::Unichar_t, String_t> NameMap_t;
        NameMap_t m_names;

        typedef std::vector<std::pair<String_t, String_t> > AliasVector_t;
        AliasVector_t m_aliases;
    };

}

#endif
