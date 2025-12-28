/**
  *  \file game/maint/dump/typedetector.hpp
  *  \brief Class game::maint::dump::TypeDetector
  */
#ifndef C2NG_GAME_MAINT_DUMP_TYPEDETECTOR_HPP
#define C2NG_GAME_MAINT_DUMP_TYPEDETECTOR_HPP

#include <vector>
#include "afl/string/string.hpp"

namespace game { namespace maint { namespace dump {

    class Input;
    class Output;

    /** File type detection.
        We detect file types based on names.
        Each file type has a name and a parser function.

        Usage:
        - create
        - call setRequiredType() if asked by user
        - call setFileBaseName() with file's basename ("bdata3.dat")
        - call start()
        - call match() with all known file types
        - use getNumPossibleTypes(), getParser(), getType() to retrieve result */
    class TypeDetector {
     public:
        /** Typedef for a parser function. */
        typedef void Parser_t(Input&, Output&);

        /** Constructor. */
        TypeDetector();

        /** Set required type.
            After this call, getType() will return only @c name or nothing.
            @param name Required type */
        void setRequiredType(String_t name);

        /** Set file name.
            @param base Basename ("bdata3.dat") */
        void setFileBaseName(String_t base);

        /** Start type detection.
            Resets any potential previous state. */
        void start();

        /** Add a potential candidate type.
            Updates the internal state considering this type.
            @param type   Type name (non-null).
                          Must be unique among all calls.
                          If setRequiredType() was called for this type,
                          this call will define a result.
            @param base   Optional base name.
                          If non-null, and it matches the file name,
                          this call will define a result.
            @param parser Associated parser function */
        void match(const char* type, const char* base, Parser_t& parser);

        /** Get number of possible results.
            @return number (0=no match, 1=unique match, >1=ambiguous) */
        size_t getNumPossibleTypes() const
            { return m_possibleTypes.size(); }

        /** Get resulting parser.
            @return parser; can be null if getNumPossibleTypes() is 0; unspecified if ambiguous */
        Parser_t* getParser() const
            { return m_foundParser; }

        /** Get name of result type.
            @return name; empty if none found; unspecified if ambiguous */
        String_t getType() const
            { return m_possibleTypes.empty() ? String_t() : m_possibleTypes[0]; }

        /** Check whether setRequiredType() has been called.
            @return flag */
        bool hasRequiredType() const
            { return !m_typeOverride.empty(); }

        /** Get required type that has been set with setRequiredType().
            @return type name */
        String_t getRequiredType() const
            { return m_typeOverride; }

     private:
        std::vector<const char*> m_possibleTypes;
        Parser_t* m_foundParser;
        String_t m_typeOverride;
        String_t m_fileBasename;
    };

} } }

#endif
