/**
  *  \file game/task.cpp
  *  \brief Tasks
  */

#include "game/task.hpp"

std::auto_ptr<game::Task_t>
game::makeConfirmationTask(bool flag, std::auto_ptr<StatusTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(bool flag, std::auto_ptr<StatusTask_t> then)
            : m_flag(flag), m_then(then)
            { }
        void call()
            { m_then->call(m_flag); }
     private:
        bool m_flag;
        std::auto_ptr<StatusTask_t> m_then;
    };
    return std::auto_ptr<Task_t>(new Task(flag, then));
}

std::auto_ptr<game::StatusTask_t>
game::makeResultTask(bool& result)
{
    class Task : public StatusTask_t {
     public:
        Task(bool& result)
            : m_result(result)
            { }
        void call(bool flag)
            { m_result = flag; }
     private:
        bool& m_result;
    };
    return std::auto_ptr<StatusTask_t>(new Task(result));
}

std::auto_ptr<game::StatusTask_t>
game::makeConditionalTask(std::auto_ptr<Task_t> then, std::auto_ptr<Task_t> otherwise)
{
    class Condition : public game::StatusTask_t {
     public:
        Condition(std::auto_ptr<game::Task_t> then, std::auto_ptr<game::Task_t> otherwise)
            : m_then(then), m_otherwise(otherwise)
            { }
        virtual void call(bool flag)
            {
                if (flag) {
                    m_then->call();
                } else {
                    m_otherwise->call();
                }
            }
     private:
        std::auto_ptr<game::Task_t> m_then;
        std::auto_ptr<game::Task_t> m_otherwise;
    };
    return std::auto_ptr<StatusTask_t>(new Condition(then, otherwise));
}
