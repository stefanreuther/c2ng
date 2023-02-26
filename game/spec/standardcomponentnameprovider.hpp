/**
  *  \file game/spec/standardcomponentnameprovider.hpp
  *  \brief Class game::spec::StandardComponentNameProvider
  */
#ifndef C2NG_GAME_SPEC_STANDARDCOMPONENTNAMEPROVIDER_HPP
#define C2NG_GAME_SPEC_STANDARDCOMPONENTNAMEPROVIDER_HPP

#include <map>
#include "game/spec/componentnameprovider.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/log.hpp"
#include "afl/io/directory.hpp"

namespace game { namespace spec {

    /** Standard implementation of ComponentNameProvider.
        Stores a set of translations and satisfies requests from that.
        Call load() to initialize this object by loading the names.cc file. */
    class StandardComponentNameProvider : public ComponentNameProvider {
     public:
        /** Default constructor.
            Makes an empty object. */
        StandardComponentNameProvider();

        /** Destructor. */
        ~StandardComponentNameProvider();

        // ComponentNameProvider:
        virtual String_t getName(Type type, int index, const String_t& name) const;
        virtual String_t getShortName(Type type, int index, const String_t& name, const String_t& shortName) const;

        /** Clear.
            Discards all translations. */
        void clear();

        /** Load configuration file.

            @param dir Game directory
            @param tx  Translator (to obtain current language; and for log messages)
            @param log Logger */
        void load(afl::io::Directory& dir, afl::string::Translator& tx, afl::sys::LogListener& log);

     private:
        static const size_t NUM_TRANSLATIONS = 4;
        std::map<String_t, String_t> m_translations[NUM_TRANSLATIONS];

        class NameFileParser;
        friend class NameFileParser;
    };

} }

#endif
