/**
  *  \file util/process/nullfactory.cpp
  */

#include "util/process/nullfactory.hpp"
#include "util/process/subprocess.hpp"
#include "afl/string/messages.hpp"

namespace {
    class NullProcess : public util::process::Subprocess {
     public:
        virtual bool isActive() const
            { return false; }
        virtual uint32_t getProcessId() const
            { return 0; }
        virtual bool start(const String_t& /*path*/, afl::base::Memory<const String_t> /*args*/)
            { return false; }
        virtual bool stop()
            { return true; }
        virtual bool writeLine(const String_t& /*line*/)
            { return false; }
        virtual bool readLine(String_t& /*result*/)
            { return false; }
        virtual String_t getStatus() const
            { return afl::string::Messages::unsupportedFeature(); }
    };
}

util::process::Subprocess*
util::process::NullFactory::createNewProcess()
{
    return new NullProcess();
}
