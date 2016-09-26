/**
  *  \file game/browser/usercallbackproxy.cpp
  *  \brief Class game::browser::UserCallbackProxy
  */

#include "game/browser/usercallbackproxy.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/semaphore.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace {
    const char LOG_NAME[] = "game.browser.callback";
}

// Constructor.
game::browser::UserCallbackProxy::UserCallbackProxy(afl::string::Translator& tx, afl::sys::LogListener& log)
    : m_translator(tx),
      m_log(log),
      m_sender()
{ }

bool
game::browser::UserCallbackProxy::askInput(String_t title, const std::vector<Element>& question, afl::data::Segment& values)
{
    afl::sys::Semaphore sem(0);
    bool result = false;
    class Caller : public util::Request<UserCallback> {
     public:
        // Executed in session thread
        Caller(afl::sys::Semaphore& sem,
               bool& result,
               String_t title,
               const std::vector<Element>& question,
               afl::data::Segment& values,
               afl::string::Translator& tx,
               afl::sys::LogListener& log)
            : m_sem(sem), m_result(result), m_title(title), m_question(question), m_values(values),
              m_translator(tx), m_log(log),
              m_done(false)
            { }

        // Executed in UI thread
        ~Caller()
            {
                if (!m_done) {
                    m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Background dialog request \"%s\" rejected").c_str(), m_title));
                    m_sem.post();
                }
            }

        // Executed in UI thread
        void handle(UserCallback& cb)
            {
                m_result = cb.askInput(m_title, m_question, m_values);
                m_sem.post();
                m_done = true;
            }
     private:
        afl::sys::Semaphore& m_sem;
        bool& m_result;
        String_t m_title;
        const std::vector<Element>& m_question;
        afl::data::Segment& m_values;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
        bool m_done;
    };
    m_sender.postNewRequest(new Caller(sem, result, title, question, values, m_translator, m_log));
    sem.wait();
    return result;
}

void
game::browser::UserCallbackProxy::setInstance(util::RequestSender<UserCallback> p)
{
    m_sender = p;
}
