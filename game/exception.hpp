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

        Change from PCC2:
        - PCC2 used this class as an error return. We probably can do without that in c2ng.
        - We don't expect these messages to hit the user in normal operation.
          Therefore, the split into a user and game side has been removed.
          (Using them when a function is called in wrong context is ok.) */
    class Exception : public std::exception {
     public:
        /** Constructor.
            \param error Error message, English, untranslated, with no period at end.
                         Probably one of the eXxxx constants. */
        explicit Exception(String_t error);

        /** Destructor. */
        ~Exception() throw();

        /** Get user-side error text.
            \return text */
        virtual const char* what() const throw();

        static const char eFacility[], eNotOwner[], eRange[],
            ePerm[], eNoBase[], eNoResource[], ePos[], ePartial[],
            eUser[], eFleet[], eNotFleet[], eGraph[], eDone[], eNotPlaying[];

     private:
        String_t m_scriptError;
    };

}

#endif
