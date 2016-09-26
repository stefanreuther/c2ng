/**
  *  \file game/spec/standardcomponentnameprovider.hpp
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

    class StandardComponentNameProvider : public ComponentNameProvider {
     public:
        StandardComponentNameProvider();
        ~StandardComponentNameProvider();

        virtual String_t getName(Type type, int index, const String_t& name) const;
        virtual String_t getShortName(Type type, int index, const String_t& name, const String_t& shortName) const;

        void clear();
        void load(afl::io::Directory& dir, afl::string::Translator& tx, afl::sys::LogListener& log);

     private:
        static const size_t NUM_TRANSLATIONS = 4;
        std::map<String_t, String_t> m_translations[NUM_TRANSLATIONS];

        class NameFileParser;
        friend class NameFileParser;
    };

} }

#endif
