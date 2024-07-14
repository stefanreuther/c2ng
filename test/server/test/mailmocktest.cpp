/**
  *  \file test/server/test/mailmocktest.cpp
  *  \brief Test for server::test::MailMock
  */

#include "server/test/mailmock.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/except/assertionfailedexception.hpp"

using server::test::MailMock;

/** Test normal operation using extract(). */
AFL_TEST("server.test.MailMock:success:extract", a)
{
    MailMock testee(afl::test::Assert("sub"));

    // Send
    testee.startMessage("tpl", String_t("uniq"));
    testee.addParameter("param", "value");
    testee.addAttachment("url");

    String_t recv[] = {"r1", "r2"};
    testee.send(recv);

    // Verify
    // - not empty
    a.check("00. empty", !testee.empty());

    // - message to r2
    MailMock::Message* msg = testee.extract("r2");
    a.checkNonNull("01. msg",  msg);
    a.checkEqual  ("02. tpl",  msg->templateName, "tpl");
    a.check       ("03. att", !msg->attachments.empty());
    a.checkEqual  ("04. att", *msg->attachments.begin(), "url");

    // - no further messages to r2
    msg = testee.extract("r2");
    a.checkNull   ("11. msg", msg);

    // - message to r1
    msg = testee.extract("r1");
    a.checkNonNull("21. msg", msg);
    a.check       ("23. att", !msg->attachments.empty());
    a.checkEqual  ("24. att", *msg->attachments.begin(), "url");

    // - no further messages to r1
    msg = testee.extract("r1");
    a.checkNull   ("31. msg", msg);

    // - empty
    a.check("41. empty", testee.empty());
}

/** Test normal operation using extractFirst(). */
AFL_TEST("server.test.MailMock:success:extractFirst", a)
{
    MailMock testee(afl::test::Assert("sub"));

    // Send
    testee.startMessage("tpl", String_t("uniq"));
    testee.addParameter("param", "value");
    testee.addAttachment("url");

    String_t recv[] = {"r1", "r2"};
    testee.send(recv);

    // Verify
    // - not empty
    a.check("00. empty", !testee.empty());

    std::auto_ptr<MailMock::Message> msg = testee.extractFirst();
    a.checkNonNull("01. msg",  msg.get());
    a.checkEqual  ("02. tpl",  msg->templateName, "tpl");
    a.check       ("03. att", !msg->attachments.empty());
    a.checkEqual  ("04. att", *msg->attachments.begin(), "url");
    a.checkEqual  ("05. recv", msg->receivers.size(), 2U);

    // - empty
    msg = testee.extractFirst();
    a.checkNull("11. msg", msg.get());
    a.check("12. empty", testee.empty());
}

/** Test normal operation using extractFirst(). */
AFL_TEST("server.test.MailMock:error:duplicate-parameter", a)
{
    MailMock testee(afl::test::Assert("sub"));
    testee.startMessage("tpl", String_t("uniq"));
    testee.addParameter("param", "value");
    AFL_CHECK_THROWS(a, testee.addParameter("param", "value2"), afl::except::AssertionFailedException);
}

/** Test normal operation using extractFirst(). */
AFL_TEST("server.test.MailMock:error:sequence:addParameter", a)
{
    MailMock testee(afl::test::Assert("sub"));
    AFL_CHECK_THROWS(a, testee.addParameter("p", "v"), afl::except::AssertionFailedException);
}

/** Test normal operation using extractFirst(). */
AFL_TEST("server.test.MailMock:error:sequence:send", a)
{
    MailMock testee(afl::test::Assert("sub"));
    String_t recv[] = {"r1", "r2"};
    AFL_CHECK_THROWS(a, testee.send(recv), afl::except::AssertionFailedException);
}

/** Test rejected operations (coverage). */
AFL_TEST("server.test.MailMock:error:unsupported", a)
{
    MailMock testee(afl::test::Assert("sub"));
    AFL_CHECK_SUCCEEDS(a("01. cancelMessage"), testee.cancelMessage("xy"));
    AFL_CHECK_THROWS(a("02. confirmAddress"), testee.confirmAddress("a@b", "key", String_t("info")), afl::except::AssertionFailedException);
    AFL_CHECK_THROWS(a("03. requestAddress"), testee.requestAddress("a@b"),                          afl::except::AssertionFailedException);
    AFL_CHECK_THROWS(a("04. runQueue"),       testee.runQueue(),                                     afl::except::AssertionFailedException);
    AFL_CHECK_THROWS(a("05. getUserStatus"),  testee.getUserStatus("u"),                             afl::except::AssertionFailedException);
}
