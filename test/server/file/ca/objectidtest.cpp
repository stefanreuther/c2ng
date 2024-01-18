/**
  *  \file test/server/file/ca/objectidtest.cpp
  *  \brief Test for server::file::ca::ObjectId
  */

#include "server/file/ca/objectid.hpp"

#include "afl/base/staticassert.hpp"
#include "afl/checksums/hash.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("server.file.ca.ObjectId:basics", a)
{
    using server::file::ca::ObjectId;
    ObjectId aa = {{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}};
    ObjectId bb = {{21,22,23,24,25,26,27,28,29,10,11,12,13,14,15,16,17,18,19,20}};
    ObjectId cc = {{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,21}};
    ObjectId dd = {{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}};

    a.check("01", aa == aa);
    a.check("02", aa != bb);
    a.check("03", aa != cc);
    a.check("04", aa == dd);

    a.check("11", bb != aa);
    a.check("12", bb == bb);
    a.check("13", bb != cc);
    a.check("14", bb != dd);

    a.check("21", aa != ObjectId::nil);
}

/** Test interaction with hash. */
AFL_TEST("server.file.ca.ObjectId:hash", a)
{
    using server::file::ca::ObjectId;
    class TestHash : public afl::checksums::Hash {
     public:
        TestHash(size_t n)
            : m_n(n)
            { }
        virtual void clear()
            { }
        virtual void add(ConstBytes_t /*data*/)
            { }
        virtual size_t getHashSize() const
            { return m_n; }
        virtual size_t getBlockSize() const
            { return m_n; }
        virtual Bytes_t getHash(Bytes_t data) const
            {
                data.trim(m_n);
                data.fill(1);
                return data;
            }
     private:
        const size_t m_n;
    };

    // Verify this test
    static_assert(sizeof(ObjectId) == 20, "sizeof ObjectId");

    // Hash too short
    {
        TestHash h5(5);
        ObjectId testee = ObjectId::fromHash(h5);
        for (size_t i = 0; i < 5; ++i) {
            a.checkEqual("01", testee.m_bytes[i], 1);
        }
        for (size_t i = 5; i < 20; ++i) {
            a.checkEqual("02", testee.m_bytes[i], 0);
        }
    }

    // Hash just right
    {
        TestHash h20(20);
        ObjectId testee = ObjectId::fromHash(h20);
        for (size_t i = 0; i < 20; ++i) {
            a.checkEqual("11", testee.m_bytes[i], 1);
        }
    }

    // Hash too long
    {
        TestHash h40(40);
        ObjectId testee = ObjectId::fromHash(h40);
        for (size_t i = 0; i < 20; ++i) {
            a.checkEqual("21", testee.m_bytes[i], 1);
        }
    }
}
