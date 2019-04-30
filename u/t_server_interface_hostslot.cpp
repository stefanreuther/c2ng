/**
  *  \file u/t_server_interface_hostslot.cpp
  *  \brief Test for server::interface::HostSlot
  */

#include "server/interface/hostslot.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostSlot::testInterface()
{
    class Tester : public server::interface::HostSlot {
     public:
        virtual void add(int32_t /*gameId*/, afl::base::Memory<const int32_t> /*slotNrs*/)
            { }
        virtual void remove(int32_t /*gameId*/, afl::base::Memory<const int32_t> /*slotNrs*/)
            { }
        virtual void getAll(int32_t /*gameId*/, afl::data::IntegerList_t& /*result*/)
            { }
    };
    Tester t;
}

