/**
  *  \file u/t_server_file_ca_objectid.cpp
  *  \brief Test for server::file::ca::ObjectId
  */

#include "server/file/ca/objectid.hpp"

#include "t_server_file_ca.hpp"
#include "afl/checksums/hash.hpp"
#include "afl/base/staticassert.hpp"

/** Simple test. */
void
TestServerFileCaObjectId::testIt()
{
    using server::file::ca::ObjectId;
    ObjectId a = {{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}};
    ObjectId b = {{21,22,23,24,25,26,27,28,29,10,11,12,13,14,15,16,17,18,19,20}};
    ObjectId c = {{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,21}};
    ObjectId d = {{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}};

    TS_ASSERT(a == a);
    TS_ASSERT(a != b);
    TS_ASSERT(a != c);
    TS_ASSERT(a == d);

    TS_ASSERT(b != a);
    TS_ASSERT(b == b);
    TS_ASSERT(b != c);
    TS_ASSERT(b != d);

    TS_ASSERT(a != ObjectId::nil);
}

/** Test interaction with hash. */
void
TestServerFileCaObjectId::testHash()
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
            TS_ASSERT_EQUALS(testee.m_bytes[i], 1);
        }
        for (size_t i = 5; i < 20; ++i) {
            TS_ASSERT_EQUALS(testee.m_bytes[i], 0);
        }
    }

    // Hash just right
    {
        TestHash h20(20);
        ObjectId testee = ObjectId::fromHash(h20);
        for (size_t i = 0; i < 20; ++i) {
            TS_ASSERT_EQUALS(testee.m_bytes[i], 1);
        }
    }

    // Hash too long
    {
        TestHash h40(40);
        ObjectId testee = ObjectId::fromHash(h40);
        for (size_t i = 0; i < 20; ++i) {
            TS_ASSERT_EQUALS(testee.m_bytes[i], 1);
        }
    }
}
