/**
  *  \file server/play/packerlist.cpp
  *  \brief Class server::play::PackerList
  */

#include <memory>
#include "server/play/packerlist.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"

server::play::PackerList::PackerList()
    : m_packers()
{ }

server::play::PackerList::~PackerList()
{ }

void
server::play::PackerList::addNew(Packer* p)
{
    if (p != 0) {
        // Save it
        std::auto_ptr<Packer> pp(p);

        // Do we already have it?
        String_t name = p->getName();
        bool found = false;
        for (size_t i = 0, n = m_packers.size(); i < n; ++i) {
            if (m_packers[i]->getName() == name) {
                found = true;
                break;
            }
        }

        // If not, add
        if (!found) {
            m_packers.pushBackNew(pp.release());
        }
    }
}

server::Value_t*
server::play::PackerList::buildValue() const
{
    afl::base::Ref<afl::data::Hash> hv = afl::data::Hash::create();
    for (size_t i = 0, n = m_packers.size(); i < n; ++i) {
        String_t key = m_packers[i]->getName();
        hv->setNew(key, m_packers[i]->buildValue());
    }
    return new afl::data::HashValue(hv);
}
