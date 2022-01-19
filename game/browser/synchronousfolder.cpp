/**
  *  \file game/browser/synchronousfolder.cpp
  *  \brief Class game::browser::SynchronousFolder
  */

#include "game/browser/synchronousfolder.hpp"

std::auto_ptr<game::browser::Task_t>
game::browser::SynchronousFolder::loadContent(std::auto_ptr<LoadContentTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(SynchronousFolder& parent, std::auto_ptr<LoadContentTask_t> then)
            : m_parent(parent), m_then(then)
            { }
        virtual void call()
            {
                afl::container::PtrVector<Folder> result;
                m_parent.loadContent(result);
                m_then->call(result);
            }
     private:
        SynchronousFolder& m_parent;
        std::auto_ptr<LoadContentTask_t> m_then;
    };
    return std::auto_ptr<Task_t>(new Task(*this, then));
}
