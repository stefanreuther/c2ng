/**
  *  \file u/t_util_requestsender.cpp
  *  \brief Test for util::RequestSender
  */

#include "util/requestsender.hpp"

#include "t_util.hpp"

namespace {
    struct ObjectType {
        int value;
    };

}

/** Interface test. */
void
TestUtilRequestSender::testConvert()
{
    // Converter closure to convert a RequestSender<ObjectType> into a RequestSender<int>
    class Converter : public afl::base::Closure<int& (ObjectType&)> {
     public:
        int& call(ObjectType& t)
            { return t.value; }
        Converter* clone() const
            { return new Converter(); }
    };

    // RequestSender<ObjectType> implementation
    class Impl : public util::RequestSender<ObjectType>::Impl {
     public:
        Impl(ObjectType& obj)
            : m_obj(obj)
            { }
        virtual void postNewRequest(util::Request<ObjectType>* req)
            {
                req->handle(m_obj);
                delete req;
            }
     private:
        ObjectType& m_obj;
    };

    // Request
    class Request : public util::Request<int> {
     public:
        void handle(int& i)
            { ++i; }
    };

    ObjectType obj;
    obj.value = 10;

    util::RequestSender<ObjectType> objSender(*new Impl(obj));
    util::RequestSender<int> intSender(objSender.convert(new Converter()));

    intSender.postNewRequest(new Request());

    TS_ASSERT_EQUALS(obj.value, 11);
}

