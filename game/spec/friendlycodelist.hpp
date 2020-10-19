/**
  *  \file game/spec/friendlycodelist.hpp
  *  \brief Class game::spec::FriendlyCodeList
  */
#ifndef C2NG_GAME_SPEC_FRIENDLYCODELIST_HPP
#define C2NG_GAME_SPEC_FRIENDLYCODELIST_HPP

#include "afl/base/growablememory.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/hostversion.hpp"
#include "game/spec/friendlycode.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace spec {

    /** List of friendly codes.
        This manages a list of friendly codes and offers operations on it.
        Lists can be loaded from a file, or created as a subset of another list.

        As of 20200807, "extra" friendly codes are merged in the main list and can therefore be accessed normally.

        Functions to test friendly codes take a HostSelection object that determines how to deal with host specifics.
        Pass a game::HostVersion to use that host's particular rules.
        Pass <tt>Pessimistic</tt> to assume pessimistic rules,
        i.e. assume this friendly code might be special and should be avoided when picking random friendly codes. */
    class FriendlyCodeList {
     public:
        typedef afl::container::PtrVector<FriendlyCode> Container_t;
        typedef Container_t::const_iterator Iterator_t;


        struct Info {
            String_t code;
            String_t description;

            Info(const String_t& code, const String_t& description)
                : code(code), description(description)
                { }
        };
        typedef std::vector<Info> Infos_t;

        enum Pessimistic_t { Pessimistic };

        /** Host selection helper object.
            See FriendlyCodeList description. */
        class HostSelection {
         public:
            /** Construct from HostVersion.
                Query will honor that host's rule.
                \param host Host version */
            HostSelection(const HostVersion& host);

            /** Pessimistic assumption.
                In case of doubt, query will assume that a code is special.
                \param p Placeholder */
            HostSelection(Pessimistic_t p);

            /** @copydoc game::HostVersion::hasSpacePaddedFCodes */
            bool hasSpacePaddedFCodes() const;

            /** @copydoc game::HostVersion::hasNegativeFCodes */
            bool hasNegativeFCodes() const;

            /** @copydoc game::HostVersion::hasCaseInsensitiveUniversalMinefieldFCodes */
            bool hasCaseInsensitiveUniversalMinefieldFCodes() const;

         private:
            bool m_hasSpacePaddedFCodes;
            bool m_hasNegativeFCodes;
            bool m_hasCaseInsensitiveUniversalMinefieldFCodes;
        };

        /** Default constructor.
            Makes an empty list. */
        FriendlyCodeList();

        /** Make sublist of some other list.
            The new list will contain all friendly codes valid for object o.
            \param originalList Original list
            \param obj Object to generate list for
            \param scoreDefinitions Ship score definitions
            \param shipList Ship list
            \param config Host configuration

            Note that this copies only actual friendly codes (which have a FriendlyCode object);
            it does not copy the extra friendly codes (that are only reserved as special). */
        FriendlyCodeList(const FriendlyCodeList& originalList,
                         const game::map::Object& obj,
                         const UnitScoreDefinitionList& scoreDefinitions,
                         const game::spec::ShipList& shipList,
                         const game::config::HostConfiguration& config);

        /** Destructor. */
        ~FriendlyCodeList();


        /*
         *  Container interface
         */

        /** Get number of friendly codes.
            \return number of friendly codes */
        size_t size() const;

        /** Get iterator to first friendly code.
            \return iterator */
        Iterator_t begin() const;

        /** Get iterator to one-past-last friendly code.
            \return iterator */
        Iterator_t end() const;

        /** Access a friendly code by index.
            \param n Index [0,size())
            \return Friendly code if found; null if index out of range. */
        const FriendlyCode* at(size_t n) const;

        /** Get index, given a friendly code.
            \param fc [in] Friendly code to look for, case-sensitive
            \param index [out] Index
            \retval true Code found; index has been set such that at(index)->getCode() == fc.
            \retval false Code not found */
        bool getIndexByName(const String_t& fc, size_t& index) const;

        /** Look up friendly code by name.
            \param fc Friendly code to look for, case-sensitive
            \return iterator pointing to friendly code, or end() if none */
        Iterator_t getCodeByName(const String_t& fc) const;


        /*
         *  Manipulator interface
         */

        /** Add a friendly code.
            The friendly code is always added at the end.
            \param code Friendly code object */
        void addCode(const FriendlyCode& code);

        /** Sort list in-place.
            This produces a user-friendly sorting order:
            - alphanumeric codes go first, in caseblind lexical order
            - codes starting with non-alphanumeric characters go last */
        void sort();

        /** Clear.
            \post size() == 0 */
        void clear();

        /** Load friendly code list from a file.
            Codes are appended to the end.
            Syntax errors are logged.
            \param in Input file
            \param log Logger
            \param tx Translator */
        void load(afl::io::Stream& in, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Load extra friendly codes list.
            This will append the specified file to the current list, avoiding duplicates to existing entries.
            Therefore, you should call this after load().
            \param in Input file */
        void loadExtraCodes(afl::io::Stream& in);

        /** Pack friendly-code list into standalone info object.
            This will only pack friendly codes, not prefixes (FriendlyCode::PrefixCode).
            \param out [out] List
            \param players [in] Players (for formatting descriptions) */
        void pack(Infos_t& out, const PlayerList& players) const;


        /*
         *  Checkers
         */

        /** Check whether a friendly code is numeric.
            Handles all host-specific rules.
            \param fc Friendly code
            \param host Host version
            \return true if friendly code is considered numeric */
        static bool isNumeric(const String_t& fc, const HostSelection host);

        /** Check whether a friendly code is a special code.
            A friendly code is special if it is contained in this list, and not marked UnspecialCode.
            \param fc Friendly code
            \param ignoreCase true to ignore case
            \return true if this friendly code is special */
        bool isSpecial(const String_t& fc, bool ignoreCase) const;

        /** Check whether a friendly code is a universal minefield friendly code.
            \param fc Friendly code
            \param tolerant true to accept upper and lower case; false to use host's rules
            \param host Host version
            \return true if this friendly code is a universal minefield friendly code */
        static bool isUniversalMinefieldFCode(const String_t& fc, bool tolerant, const HostSelection host);

        /** Get friendly code's numeric value.
            \param fc Friendly code
            \param host Host version
            \return numeric value (1000 if code is not numeric) */
        static int getNumericValue(const String_t& fc, const HostSelection host);

        /** Check whether a friendly code is permitted as random friendly code.
            Random codes must be
            - not special (ignoring case. HOST considers things like "eE7" special)
            - not numeric
            - random enough

            "Not special" means:
            - not listed in special-fcode list (GFCode::isSpecial)
            - not listed in extra-fcode list
            - is not a universal minefield friendly code
            - does not start with "X"
            - does not contain "#" or "?" (those are special to our simulator)

            "Random enough" means it does not contain any duplicate character.

            \param fc Friendly code
            \param host Host version
            \return friendly code is a valid result for generateRandomCode() */
        bool isAllowedRandomCode(const String_t& fc, const HostSelection host) const;

        /** Generate a random friendly code.
            See isAllowedRandomCode() for conditions for random friendly codes.
            \param rng Random number generator
            \param host Host version
            \return newly-generated code */
        String_t generateRandomCode(util::RandomNumberGenerator& rng, const HostSelection host) const;

     private:
        /** Special friendly codes. */
        Container_t m_data;
    };

} }


inline
game::spec::FriendlyCodeList::HostSelection::HostSelection(const HostVersion& host)
    : m_hasSpacePaddedFCodes(host.hasSpacePaddedFCodes()),
      m_hasNegativeFCodes(host.hasNegativeFCodes()),
      m_hasCaseInsensitiveUniversalMinefieldFCodes(host.hasCaseInsensitiveUniversalMinefieldFCodes())
{ }

inline
game::spec::FriendlyCodeList::HostSelection::HostSelection(Pessimistic_t)
    : m_hasSpacePaddedFCodes(true),
      m_hasNegativeFCodes(true),
      m_hasCaseInsensitiveUniversalMinefieldFCodes(true)
{ }

inline bool
game::spec::FriendlyCodeList::HostSelection::hasSpacePaddedFCodes() const
{
    return m_hasSpacePaddedFCodes;
}

inline bool
game::spec::FriendlyCodeList::HostSelection::hasNegativeFCodes() const
{
    return m_hasNegativeFCodes;
}

inline bool
game::spec::FriendlyCodeList::HostSelection::hasCaseInsensitiveUniversalMinefieldFCodes() const
{
    return m_hasCaseInsensitiveUniversalMinefieldFCodes;
}


#endif
