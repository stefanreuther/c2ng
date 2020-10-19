/**
  *  \file game/exception.hpp
  *  \brief Class game::Exception
  */
#ifndef C2NG_GAME_EXCEPTION_HPP
#define C2NG_GAME_EXCEPTION_HPP

#include <exception>
#include "afl/string/string.hpp"

namespace game {

    /** Game exception.
        These exceptions have a user side and a script side,
        allowing the user side to see a long, translated error message while keeping the script side constant.

        Change from PCC2: PCC2 used this class as an error return. We probably can do without that in c2ng. */
    class Exception : public std::exception {
     public:
        /** Constructor.
            \param scriptError Script-side error.
                               English, untranslated, with no period at end.
                               Ideally, one of the eXxxx constants.
            \param userError   User-side error. Translated, with no period at end. */
        Exception(String_t scriptError, String_t userError);

        /** Constructor.
            This uses the same text for user and script side;
            use only if the user-side error will not be used.
            \param error Text to use for both script and user side. */
        Exception(String_t error);

        /** Destructor. */
        ~Exception() throw();

        /** Get script-side error text.
            \return text */
        const String_t& getScriptError() const;

        /** Get user-side error text.
            \return text */
        const String_t& getUserError() const;

        /** Get user-side error text.
            \return text */
        virtual const char* what() const throw();

        static const char eFacility[], eNotOwner[], eRange[],
            ePerm[], eNoBase[], eNoResource[], ePos[], ePartial[],
            eUser[], eFleet[], eGraph[], eDone[], eNotPlaying[];

     private:
        String_t m_scriptError;
        String_t m_userError;
    };

}

#endif
