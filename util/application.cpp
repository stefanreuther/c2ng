/**
  *  \file util/application.cpp
  */

#include <cstdlib>
#include <stdexcept>
#include "util/application.hpp"
#include "afl/io/nulltextwriter.hpp"
#include "afl/string/format.hpp"
#include "afl/except/fileproblemexception.hpp"

namespace {
    afl::base::Ref<afl::io::TextWriter> getWriter(afl::sys::Environment& env, afl::sys::Environment::Channel ch)
    {
        try {
            return env.attachTextWriter(ch);
        }
        catch (...) {
            return *new afl::io::NullTextWriter();
        }
    }

    class Exit {
     public:
        Exit(int n)
            : m_exitCode(n)
            { }
        int get()
            { return m_exitCode; }
     private:
        int m_exitCode;
    };
}

util::Application::Application(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : m_environment(env),
      m_fileSystem(fs),
      m_translator(),
      m_logger(),
      m_errorOutput(getWriter(env, env.Error)),
      m_standardOutput(getWriter(env, env.Output))
{
    m_logger.attachWriter(false, m_standardOutput.asPtr());
    m_logger.attachWriter(true, m_errorOutput.asPtr());
}

int
util::Application::run()
{
    try {
        appMain();
        m_standardOutput->flush();
        m_errorOutput->flush();
        return 0;
    }
    catch (Exit& n) {
        return n.get();
    }
    catch (afl::except::FileProblemException& e) {
        reportError(e.getFileName() + ": " + e.what());
        return 1;
    }
    catch (std::exception& e) {
        reportError(e.what());
        return 1;
    }
    catch (...) {
        reportError("Uncaught exception");
        return 1;
    }
}

void
util::Application::exit(int n)
{
    m_standardOutput->flush();
    m_errorOutput->flush();
    throw Exit(n);
}

void
util::Application::errorExit(String_t str)
{
    reportError(str);
    exit(1);
}

afl::sys::Environment&
util::Application::environment()
{
    return m_environment;
}

afl::io::FileSystem&
util::Application::fileSystem()
{
    return m_fileSystem;
}

afl::string::Translator&
util::Application::translator()
{
    return m_translator;
}

afl::sys::LogListener&
util::Application::log()
{
    return m_logger;
}

util::ConsoleLogger&
util::Application::consoleLogger()
{
    return m_logger;
}

afl::io::TextWriter&
util::Application::errorOutput()
{
    return *m_errorOutput;
}

afl::io::TextWriter&
util::Application::standardOutput()
{
    return *m_standardOutput;
}

void
util::Application::reportError(String_t str)
{
    m_errorOutput->writeLine(afl::string::Format("%s: %s", m_environment.getInvocationName(), str));
    m_standardOutput->flush();
    m_errorOutput->flush();
}
