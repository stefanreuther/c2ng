/**
  *  \file u/t_server_interface_hosttool.cpp
  *  \brief Test for server::interface::HostTool
  */

#include "server/interface/hosttool.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostTool::testInterface()
{
    class Tester : public server::interface::HostTool {
     public:
        virtual void add(String_t /*id*/, String_t /*fileName*/, String_t /*program*/, String_t /*kind*/)
            { }
        virtual void set(String_t /*id*/, String_t /*key*/, String_t /*value*/)
            { }
        virtual String_t get(String_t /*id*/, String_t /*key*/)
            { return String_t(); }
        virtual bool remove(String_t /*id*/)
            { return false; }
        virtual void getAll(std::vector<Info>& /*result*/)
            { }
        virtual void copy(String_t /*sourceId*/, String_t /*destinationId*/)
            { }
        virtual void setDefault(String_t /*id*/)
            { }
        virtual int32_t getDifficulty(String_t /*id*/)
            { return 0; }
        virtual void clearDifficulty(String_t /*id*/)
            { }
        virtual int32_t setDifficulty(String_t /*id*/, afl::base::Optional<int32_t> /*value*/, bool /*use*/)
            { return 0; }
    };
    Tester t;
}

