/**
  *  \file util/translator.hpp
  *  \brief Class util::Translator
  */
#ifndef C2NG_UTIL_TRANSLATOR_HPP
#define C2NG_UTIL_TRANSLATOR_HPP

#include <map>
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/mutex.hpp"

namespace util {
    /** Implementation of Translator interface for PCC2 language files.

        Implements translation using a map that can be loaded from a file.

        To cut down number of translations, a sequence of punctuation characters
        at the end of a string is taken over verbatim and not looked up in the translation map.
        Punctuation characters are "\n", ":" and " ".

        If text cannot be translated, it is returned as-is.
        A possible keyword in braces is removed before,
        e.g. "{building}mine" is returned as "mine".
        Presence of the keyword allows different translations for languages that require it,
        for example for "{weapon}mine" and "{attribute}mine".

        Since translators are used multi-threaded, this class is interlocked. */
    class Translator : public afl::string::Translator {
     public:
        /** Constructor. */
        Translator();

        /** Destructor. */
        ~Translator();

        /** Clear.
            Discards all translations. */
        void clear();

        /** Add single translation.
            @param orig    Original text (input)
            @param result  Translated text (output) */
        void addTranslation(const String_t& orig, const String_t& result);

        /** Load from file.
            @param s   Stream
            @throw afl::except::FileProblemException on error */
        void loadFile(afl::io::Stream& s);

        /** Load default translation, given an environment.
            Loads the language corresponding to the user's settings.

            This function ignores all possible errors.
            If the language cannot be loaded, this translator will not translate.

            @param fs  File system (for file access)
            @param env Environment (for directory name and language) */
        void loadDefaultTranslation(afl::io::FileSystem& fs, afl::sys::Environment& env);

        /** Load translation, given a language.

            This function ignores all possible errors.
            If the language cannot be loaded, this translator will not translate.

            @param fs   File system (for file access)
            @param env  Environment (for directory name and language)
            @param code Code for language to load */
        void loadTranslation(afl::io::FileSystem& fs, afl::sys::Environment& env, afl::string::LanguageCode code);

        // Translator:
        virtual String_t translate(afl::string::ConstStringMemory_t in) const;

     private:
        mutable afl::sys::Mutex m_mutex;
        std::map<String_t, String_t> m_map;
    };
}

#endif
