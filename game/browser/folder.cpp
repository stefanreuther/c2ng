/**
  *  \file game/browser/folder.cpp
  *  \brief Base class game::browser::Folder
  */

#include "game/browser/folder.hpp"

// Default (dummy) implementation of loadGameRoot().
std::auto_ptr<game::Task_t>
game::browser::Folder::defaultLoadGameRoot(std::auto_ptr<LoadGameRootTask_t> t)
{
    class Task : public Task_t {
     public:
        Task(std::auto_ptr<LoadGameRootTask_t>& t)
            : m_task(t)
            { }
        virtual void call()
            { m_task->call(0); }
     private:
        std::auto_ptr<LoadGameRootTask_t> m_task;
    };
    return std::auto_ptr<Task_t>(new Task(t));
}
