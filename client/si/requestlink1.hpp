/**
  *  \file client/si/requestlink1.hpp
  *  \brief Class client::si::RequestLink1
  */
#ifndef C2NG_CLIENT_SI_REQUESTLINK1_HPP
#define C2NG_CLIENT_SI_REQUESTLINK1_HPP

#include "interpreter/process.hpp"

namespace client { namespace si {

    /** Link to requesting process, physical version.
        Identifies the process as a reference.
        For use as temporary variable within ScriptSide only. */
    class RequestLink1 {
     public:
        /** Constructor.
            \param process Process
            \param wantResult "wantResult" flag from interpreter::CallableValue::call() */
        RequestLink1(interpreter::Process& process, bool wantResult)
            : m_process(process),
              m_wantResult(wantResult)
            { }

        /** Get process.
            \return process */
        interpreter::Process& getProcess() const
            { return m_process; }

        /** Get "wantResult" flag.
            \return flag */
        bool isWantResult() const
            { return m_wantResult; }
     private:
        interpreter::Process& m_process;
        bool m_wantResult;
    };

} }

#endif
