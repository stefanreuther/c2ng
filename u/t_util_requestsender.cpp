/**
  *  \file u/t_util_requestsender.cpp
  *  \brief Test for util::RequestSender
  */

#include "util/requestsender.hpp"

#include "t_util.hpp"

namespace {
    struct Value {
        int value;

        void incr()
            { ++value; }
        void add(int a)
            { value += a; }
        void mac(int a, int b)
            { value += a*b; }
        void tri(int a, int b, int c)
            { value += a*(b+c); }
    };

    struct ObjectType {
        Value value;
    };

    struct Temporary {
        ObjectType* parent;
    };

    // Simple RequestSender implementation
    class SimpleImpl : public util::RequestSender<ObjectType>::Impl {
     public:
        SimpleImpl(ObjectType& obj)
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

    // RequestSender<ObjectType> implementation that swallows exceptions
    class CatchingImpl : public util::RequestSender<ObjectType>::Impl {
     public:
        CatchingImpl(ObjectType& obj)
            : m_obj(obj)
            { }
        virtual void postNewRequest(util::Request<ObjectType>* req)
            {
                std::auto_ptr<util::Request<ObjectType> > p(req);
                try {
                    req->handle(m_obj);
                }
                catch (...)
                { }
            }
     private:
        ObjectType& m_obj;
    };
}

/** Test convert().
    A: create RequestSender. Use a converter closure to convert to a member, and operate on that
    E: verify correct result. */
void
TestUtilRequestSender::testConvert()
{
    // Converter closure to convert a RequestSender<ObjectType> into a RequestSender<Value>
    class Converter : public afl::base::Closure<Value& (ObjectType&)> {
     public:
        Value& call(ObjectType& t)
            { return t.value; }
    };

    // Request
    class Request : public util::Request<Value> {
     public:
        void handle(Value& i)
            { ++i.value; }
    };

    ObjectType obj;
    obj.value.value = 10;

    util::RequestSender<ObjectType> objSender(*new SimpleImpl(obj));
    util::RequestSender<Value> valSender(objSender.convert(new Converter()));

    valSender.postNewRequest(new Request());

    TS_ASSERT_EQUALS(obj.value.value, 11);
}

/** Test makeTemporary().
    A: create RequestSender. Use a converter closure to create a temporary, and operate on that
    E: verify correct result. */
void
TestUtilRequestSender::testMakeTemporary()
{
    // Converter closure to convert a RequestSender<ObjectType> into a RequestSender<Temporary>
    class Converter : public afl::base::Closure<Temporary* (ObjectType&)> {
     public:
        Temporary* call(ObjectType& t)
            {
                Temporary* p = new Temporary();
                p->parent = &t;
                return p;
            }
    };

    // Request
    class Request : public util::Request<Temporary> {
     public:
        void handle(Temporary& p)
            { ++p.parent->value.value; }
    };

    ObjectType obj;
    obj.value.value = 10;

    util::RequestSender<ObjectType> objSender(*new SimpleImpl(obj));
    util::RequestSender<Temporary> tmpSender(objSender.makeTemporary(new Converter()));

    tmpSender.postNewRequest(new Request());

    TS_ASSERT_EQUALS(obj.value.value, 11);
}

/** Test postRequest().
    A: create RequestSender for objects with multiple member functions. Call postRequest for member functions.
    E: correct results produced. */
void
TestUtilRequestSender::testPostRequest()
{
    // Implementation for testing
    class Impl : public util::RequestSender<Value>::Impl {
     public:
        Impl(Value& obj)
            : m_obj(obj)
            { }
        virtual void postNewRequest(util::Request<Value>* req)
            {
                req->handle(m_obj);
                delete req;
            }
     private:
        Value& m_obj;
    };

    Value value;
    value.value = 10;

    util::RequestSender<Value> sender(*new Impl(value));
    sender.postRequest(&Value::incr);
    TS_ASSERT_EQUALS(value.value, 11);

    sender.postRequest(&Value::add, 20);
    TS_ASSERT_EQUALS(value.value, 31);

    sender.postRequest(&Value::mac, 7, 9);
    TS_ASSERT_EQUALS(value.value, 94);

    sender.postRequest(&Value::tri, 1, 2, 3);
    TS_ASSERT_EQUALS(value.value, 99);
}

/** Test failure in convert.
    A: create RequestSender. Use a converter closure that throws.
    E: verify operation not executed, no memory leak (use valgrind to check). */
void
TestUtilRequestSender::testConvertFail()
{
    // Converter closure to convert a RequestSender<ObjectType> into a RequestSender<Value>
    class Converter : public afl::base::Closure<Value& (ObjectType&)> {
     public:
        Value& call(ObjectType& /*t*/)
            { throw "boom!"; }
    };

    // Request
    class Request : public util::Request<Value> {
     public:
        void handle(Value& i)
            { ++i.value; }
    };

    ObjectType obj;
    obj.value.value = 10;

    util::RequestSender<ObjectType> objSender(*new CatchingImpl(obj));
    util::RequestSender<Value> valSender(objSender.convert(new Converter()));

    valSender.postNewRequest(new Request());

    TS_ASSERT_EQUALS(obj.value.value, 10);
}

/** Test failure in convert.
    A: create RequestSender. Use a converter closure that throws.
    E: verify operation not executed, no memory leak (use valgrind to check). */
void
TestUtilRequestSender::testMakeTemporaryFail()
{
    // Converter closure to convert a RequestSender<ObjectType> into a RequestSender<Temporary>
    class Converter : public afl::base::Closure<Temporary* (ObjectType&)> {
     public:
        Temporary* call(ObjectType& /*t*/)
            { throw "boom"; }
    };

    // Request
    class Request : public util::Request<Temporary> {
     public:
        void handle(Temporary& p)
            { ++p.parent->value.value; }
    };

    ObjectType obj;
    obj.value.value = 10;

    util::RequestSender<ObjectType> objSender(*new CatchingImpl(obj));
    util::RequestSender<Temporary> tmpSender(objSender.makeTemporary(new Converter()));

    tmpSender.postNewRequest(new Request());

    TS_ASSERT_EQUALS(obj.value.value, 10);
}

