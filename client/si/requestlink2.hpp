/**
  *  \file client/si/requestlink2.hpp
  *  \brief Class client::si::RequestLink2
  */
#ifndef C2NG_CLIENT_SI_REQUESTLINK2_HPP
#define C2NG_CLIENT_SI_REQUESTLINK2_HPP

#include "afl/base/types.hpp"
#include "client/si/requestlink1.hpp"

namespace client { namespace si {

    /** Link to requesting process, logical version.
        Identifies the process as a process Id that must be looked up.
        Can be transferred between UserSide and ScriptSide.

        A RequestLink2 can be valid (points at a process) or invalid (does not point at a process).

        \todo Reconsider. Only UserSide needs optional behaviour, but mostly ScriptSide checks it.
        Maybe make a RequestLink3? */
    class RequestLink2 {
     public:
        /** Default constructor.
            Makes a null (invalid) RequestLink2. */
        RequestLink2()
            : m_pid(0),
              m_wantResult(false),
              m_isValid(false)
            { }

        /** Constructor.
            \param pid Process Id
            \param wantResult "wantResult" flag from interpreter::CallableValue::call() */
        RequestLink2(uint32_t pid, bool wantResult)
            : m_pid(pid),
              m_wantResult(wantResult),
              m_isValid(true)
            { }

        /** Construct from RequestLink1.
            \param r Original value */
        RequestLink2(const RequestLink1& r)
            : m_pid(r.getProcess().getProcessId()),
              m_wantResult(r.isWantResult()),
              m_isValid(true)
            { }

        /** Get process Id.
            \param result [out] process Id
            \retval true if process Id was obtained
            \retval false no process Id available */
        bool getProcessId(uint32_t& result) const
            {
                if (m_isValid) {
                    result = m_pid;
                    return true;
                } else {
                    return false;
                }
            }

        /** Get "wantResult" flag.
            \return flag */
        bool isWantResult() const
            { return m_wantResult; }

        /** Check validity.
            \retval true object points at a process
            \retval false object does not point at a process */
        bool isValid() const
            { return m_isValid; }
     private:
        uint32_t m_pid;
        bool m_wantResult;
        bool m_isValid;
    };

} }

#endif
