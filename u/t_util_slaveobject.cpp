/**
  *  \file u/t_util_slaveobject.cpp
  *  \brief Test for util::SlaveObject
  */

#include "util/slaveobject.hpp"

#include "t_util.hpp"

/** Interface test. */
void
TestUtilSlaveObject::testIt()
{
    class Tester : public util::SlaveObject<int> {
     public:
        virtual void init(int& /*master*/)
            { }
        virtual void done(int& /*master*/)
            { }
    };
    Tester t;
}

