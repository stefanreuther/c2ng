/**
  *  \file game/spec/friendlycodelist.hpp
  */
#ifndef C2NG_GAME_SPEC_FRIENDLYCODELIST_HPP
#define C2NG_GAME_SPEC_FRIENDLYCODELIST_HPP

#include "game/spec/friendlycode.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/base/growablememory.hpp"
#include "game/hostversion.hpp"
#include "util/randomnumbergenerator.hpp"
#include "afl/io/stream.hpp"
#include "afl/sys/loglistener.hpp"

namespace game { namespace spec {

    class FriendlyCodeList {
     public:
        typedef afl::container::PtrVector<FriendlyCode> Container_t;
        typedef Container_t::const_iterator Iterator_t;

        FriendlyCodeList();

        /** Make sublist of some other list. The new list will contain all
            friendly codes valid for object o.
            \param l   output list, should be empty
            \param o   object to generate list for */
        FriendlyCodeList(const FriendlyCodeList& originalList, const game::map::Object& obj, game::config::HostConfiguration& config);

        ~FriendlyCodeList();

        /*
         *  Container interface
         */

        size_t size() const;

        Iterator_t begin() const;

        Iterator_t end() const;

        const FriendlyCode* at(size_t n) const;

        bool getIndexByName(const String_t& fc, size_t& index) const;

        Iterator_t getCodeByName(const String_t& fc) const;


        /*
         *  Manipulator interface
         */

        void addCode(const FriendlyCode& code);
        void sort();
        void clear();
        void load(afl::io::Stream& in, afl::sys::LogListener& log);

        void clearExtraCodes();
        void loadExtraCodes(afl::io::Stream& in);


        /*
         *  Checkers
         */

        bool isNumeric(const String_t& fc, const HostVersion& host) const;
        bool isExtra(const String_t& fc) const;
        bool isSpecial(const String_t& fc, bool ignoreCase) const;
        bool isUniversalMinefieldFCode(const String_t& fc, bool tolerant, const HostVersion& host) const;
        int  getNumericValue(const String_t& fc, const HostVersion& host) const;

        bool isAllowedRandomCode(const String_t& fc, const HostVersion& host);
        String_t generateRandomCode(util::RandomNumberGenerator& rng, const HostVersion& host);


     private:
        Container_t m_data;

// /** Extra fcodes. \todo Since this is compared against UTF-8 encoded
//     fcodes, it implicitly assumes that xtrafcode.txt is stored in
//     UTF-8 encoding. That's not particularily nice, but on the other
//     hand not much of a problem as fcodes are expected to not contain
//     extended characters. */
        afl::base::GrowableMemory<uint8_t> m_extraData;
    };

} }

#endif
