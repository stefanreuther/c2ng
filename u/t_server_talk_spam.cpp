/**
  *  \file u/t_server_talk_spam.cpp
  *  \brief Test for server::talk::Spam
  */

#include "server/talk/spam.hpp"

#include "t_server_talk.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/sys/log.hpp"
#include "server/talk/root.hpp"

// An actual spam message received by PlanetsCentral.
const char TestServerTalkSpam::SPAM_MESSAGE[] =
    "forumL:featured articles about del bosque\r\n"
    "\r\n"
    "\\\"Towards Clarification of a Concept: Mapping the Nature and Typologies of Afro Pessimism\\\". International Communicatio"
    "n Association (ICA) Annual Conference, London, 17 21 June 2013. (June 24 2013) This conference is a unique collaboratio"
    "n between the IAMCR Media Production Analysis Working Group with the ICA Journalism Studies Section and ECREA Media Ind"
    "ustries and Cultural Production Working Group, and is organised and hosted by theInstitute of Communications Studies (I"
    "CS) at Leeds University.\r\n"
    "\r\n"
    "Striking sidekick The first popular popup program for DOS PCs, introduced by Borland in 1984. Sidekick included a calcu"
    "lator, notepad, calendar, phone dialer and ASCII table and popularized the concept of a terminate and stay resident (TS"
    "R) utility. as the deadly duo fire themselves up to gun downCarlos Alberto Carlos Alberto is a Portuguese given name (E"
    "nglish language Charles Albert, Italian language Carlo Alberto).\r\n"
    "\r\n"
    "Maybe those early Eric Hasslired cards had more effect than many of us realise, because since then this team has been t"
    "oo nice by half, too ready to let the [url=http://elitejerseycheap.com]Nfl jerseys china[/url] opposition play, too war"
    "y to make that sameopposition know that whatever happens they are in for acontest that will leave them bruised and batt"
    "ered the next day.\r\n"
    "\r\n"
    "One potential hurdle [url=http://www.nfljerseyselite.com]Cheap nfl jerseys[/url] for 3 D technology, experts say, is th"
    "at the viewing experience may be too unique. Indeed, 3 D TV could be too immersive and occupy too much time, according "
    "to Eric Clemons, an operations and information management professor at Wharton. He notes that consumers typically watch"
    " television while doing other things eating, talking or working on a laptop and it would be hard to do those things whi"
    "le wearing 3 D goggles and engaged in a cutting edge entertainment experience.\r\n"
    "\r\n"
    "Next, one problem that athletes and runners complain about is sweaty feet. Rubber shoes are usually made of materials t"
    "hat do not allow air to circulate inside. Fortunately, Adidas was able [url=http://www.elitejerseywholesale.com]Authent"
    "ic nfl jerseys[/url] to create CLIMACOOL, a new technology that makes use of breathable materials. When you use Adidas "
    "shoes, you will feel more ventilation inside the shoes. This is important in making rubber shoes comfortable and cool. "
    "If the materials used in your rubber shoes promote proper ventilation and circulation of air, you will less likely expe"
    "rience smelly feet.\r\n"
    "\r\n"
    "Fatigue resulting from workaholism if husband and wife work long hours, there is less interest in sex. The demands of w"
    "ork and home rob people of their energy. Imagine this: husband and wife work 10 hour days in the office to earn good sa"
    "laries. They come home and work some more by doing housework and supervising the kids' homework;\r\n"
    "\r\n"
    "Ultimately, it all comes down to value and FIFA 06: Road to the World Cup just doesn offer enough stuff to make it wort"
    "h buying. At $30, this game would be highly recommended. But at a full $60, it just isn worth it. The gameplay is solid"
    " and it looks gorgeous, but you only get a handful of stripped down modes and a tiny fraction of the teams and players "
    "available in the current gen versions of FIFA 06. Rent it if you are dying to see David Beckham in all of his next gen "
    "glory, [url=http://footballjerseyswholesalesale.com]Wholesale nhl jerseys[/url] but don buy it.\r\n";

/** Simple test. */
void
TestServerTalkSpam::testIt()
{
    // Infrastructure
    afl::sys::Log log;
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::User u(root, "1003");
    server::talk::InlineRecognizer recog;

    // User
    u.profile().stringField("createacceptlanguage").set("zh_ZH");
    u.profile().intField("createtime").set(60*9999);                 // seconds, not minutes in this field!

    // Initial post.
    // Triggers on language (20), time (15), size (10), links (20) check and just reaches necessary spammity.
    TS_ASSERT(server::talk::checkSpam("whatever", SPAM_MESSAGE, 10000, u, recog, log));
    TS_ASSERT_EQUALS(u.profile().intField("spam").get(), 1);

    // Second post.
    // User is now marked as spammer and everything they post is spam.
    TS_ASSERT(server::talk::checkSpam("whatever", "innocent", 10000, u, recog, log));

    // Mark user as immune. It is no longer spam.
    u.profile().intField("spam").set(2);
    TS_ASSERT(!server::talk::checkSpam("whatever", SPAM_MESSAGE, 10000, u, recog, log));
    TS_ASSERT(!server::talk::checkSpam("whatever", "innocent", 10000, u, recog, log));
}
