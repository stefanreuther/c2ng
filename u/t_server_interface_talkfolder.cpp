/**
  *  \file u/t_server_interface_talkfolder.cpp
  *  \brief Test for server::interface::TalkFolder
  */

#include "server/interface/talkfolder.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceTalkFolder::testIt()
{
    class Tester : public server::interface::TalkFolder {
     public:
        virtual void getFolders(afl::data::IntegerList_t& /*result*/)
            { }
        virtual Info getInfo(int32_t /*ufid*/)
            { return Info(); }
        virtual void getInfo(afl::base::Memory<const int32_t> /*ufids*/, afl::container::PtrVector<Info>& /*results*/)
            { }
        virtual int32_t create(String_t /*name*/, afl::base::Memory<const String_t> /*args*/)
            { return 0; }
        virtual bool remove(int32_t /*ufid*/)
            { return false; }
        virtual void configure(int32_t /*ufid*/, afl::base::Memory<const String_t> /*args*/)
            { }
        virtual afl::data::Value* getPMs(int32_t /*ufid*/, const ListParameters& /*params*/, const FilterParameters& /*filter*/)
            { return 0; }
    };
    Tester t;
}

