/**
  *  \file test/util/doc/htmlrenderertest.cpp
  *  \brief Test for util::doc::HtmlRenderer
  */

#include "util/doc/htmlrenderer.hpp"

#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/test/testrunner.hpp"
#include "util/doc/renderoptions.hpp"
#include "util/unicodechars.hpp"

using util::doc::RenderOptions;
using afl::io::xml::Nodes_t;
using afl::io::xml::TagNode;
using afl::io::xml::TextNode;

/** Simple rendering using a heading. */
AFL_TEST("util.doc.HtmlRenderer:simple", a)
{
    // <h1 id=a>content</h1>
    Nodes_t nodes;
    TagNode* h1 = new TagNode("h1");
    nodes.pushBackNew(h1);
    h1->setAttribute("id", "a");
    h1->addNewChild(new TextNode("content"));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<h2 id=\"a\">content</h2>");
}

/** Rendering a link. */
AFL_TEST("util.doc.HtmlRenderer:link", a)
{
    // <p>link: <a href="other">click</a></p>
    Nodes_t nodes;
    TagNode* p = new TagNode("p");
    nodes.pushBackNew(p);
    p->addNewChild(new TextNode("link: "));

    TagNode* link = new TagNode("a");
    p->addNewChild(link);
    link->setAttribute("href", "other");
    link->addNewChild(new TextNode("click"));

    RenderOptions opts;
    opts.setDocumentRoot("/doc/");
    opts.setDocumentId("id");
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p>link: <a href=\"/doc/id/other\">click</a></p>");
}

/** Rendering a site link. */
AFL_TEST("util.doc.HtmlRenderer:link:site", a)
{
    // <p>link: <a href="site:file.cgi">click</a></p>
    Nodes_t nodes;
    TagNode* p = new TagNode("p");
    nodes.pushBackNew(p);
    p->addNewChild(new TextNode("link: "));

    TagNode* link = new TagNode("a");
    p->addNewChild(link);
    link->setAttribute("href", "site:file.cgi");
    link->addNewChild(new TextNode("click"));

    RenderOptions opts;
    opts.setSiteRoot("/site/");
    opts.setDocumentRoot("/doc/");
    opts.setDocumentId("id");
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p>link: <a href=\"/site/file.cgi\" class=\"site-link\">click</a></p>");
}

/** Rendering an external link. */
AFL_TEST("util.doc.HtmlRenderer:link:external", a)
{
    // <p>link: <a href="http://rcworld.de">click</a></p>
    Nodes_t nodes;
    TagNode* p = new TagNode("p");
    nodes.pushBackNew(p);
    p->addNewChild(new TextNode("link: "));

    TagNode* link = new TagNode("a");
    p->addNewChild(link);
    link->setAttribute("href", "http://rcworld.de");
    link->addNewChild(new TextNode("click"));

    RenderOptions opts;
    opts.setSiteRoot("/site/");
    opts.setDocumentRoot("/doc/");
    opts.setDocumentId("id");
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p>link: <a href=\"http://rcworld.de\" class=\"external-link\">click</a></p>");
}

/** Rendering a classified link. */
AFL_TEST("util.doc.HtmlRenderer:link:class", a)
{
    // <p>link: <a class="userlink" href="site:userinfo.cgi/a">click</a></p>
    Nodes_t nodes;
    TagNode* p = new TagNode("p");
    nodes.pushBackNew(p);
    p->addNewChild(new TextNode("link: "));

    TagNode* link = new TagNode("a");
    p->addNewChild(link);
    link->setAttribute("class", "userlink");
    link->setAttribute("href", "site:userinfo.cgi/a");
    link->addNewChild(new TextNode("click"));

    RenderOptions opts;
    opts.setSiteRoot("/site/");
    opts.setDocumentRoot("/doc/");
    opts.setDocumentId("id");
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p>link: <a href=\"/site/userinfo.cgi/a\" class=\"userlink\">click</a></p>");
}

/** Rendering a key list (custom tag). */
AFL_TEST("util.doc.HtmlRenderer:keylist", a)
{
    // <kl>
    //  <ki key="Alt-X">exit</ki>
    // <kl>
    Nodes_t nodes;
    TagNode* kl = new TagNode("kl");
    nodes.pushBackNew(kl);

    TagNode* ki = new TagNode("ki");
    kl->addNewChild(ki);
    ki->setAttribute("key", "Alt-X");
    ki->addNewChild(new TextNode("exit"));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<ul><li><kbd>Alt</kbd>" UTF_HYPHEN "<kbd>X</kbd>: exit</li></ul>");
}

/** Rendering an image, standard case. */
AFL_TEST("util.doc.HtmlRenderer:image", a)
{
    // <p><img src="asset:a" /></p>
    Nodes_t nodes;
    TagNode* p = new TagNode("p");
    nodes.pushBackNew(p);

    TagNode* img = new TagNode("img");
    p->addNewChild(img);
    img->setAttribute("src", "asset:a");
    img->setAttribute("alt", "text");

    RenderOptions opts;
    opts.setAssetRoot("/asset/");
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p><img src=\"/asset/a\" alt=\"text\"></p>");
}

/** Rendering an image, scaled (width and height given). */
AFL_TEST("util.doc.HtmlRenderer:image:scaled", a)
{
    // <p><img src="asset:a" width=30 height=20 /></p>
    Nodes_t nodes;
    TagNode* p = new TagNode("p");
    nodes.pushBackNew(p);

    TagNode* img = new TagNode("img");
    p->addNewChild(img);
    img->setAttribute("src", "asset:a");
    img->setAttribute("width", "30");
    img->setAttribute("height", "20");

    RenderOptions opts;
    opts.setAssetRoot("/asset/");
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p><img src=\"/asset/a\" width=\"30\" height=\"20\"></p>");
}

/** Rendering an image, cropped (width, height, top, left given). */
AFL_TEST("util.doc.HtmlRenderer:image:cropped", a)
{
    // <p><img src="asset:a" width=30 height=20 top=5 left=10 /></p>
    Nodes_t nodes;
    TagNode* p = new TagNode("p");
    nodes.pushBackNew(p);

    TagNode* img = new TagNode("img");
    p->addNewChild(img);
    img->setAttribute("src", "asset:a");
    img->setAttribute("width", "30");
    img->setAttribute("height", "20");
    img->setAttribute("top", "5");
    img->setAttribute("left", "10");

    RenderOptions opts;
    opts.setAssetRoot("/asset/");
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p><div style=\"width:30px;height:20px;background:url(/asset/a);background-position:-10px -5px\">&nbsp;</div></p>");
}

/** Test table rendering. */
AFL_TEST("util.doc.HtmlRenderer:table", a)
{
    // <table>
    //  <tr>
    //   <td>first</td>
    //   <th align=right>second</th>
    //   <tn>3</tn>
    //  </tr>
    // </table>
    Nodes_t nodes;
    TagNode* table = new TagNode("table");
    nodes.pushBackNew(table);

    TagNode* row = new TagNode("tr");
    table->addNewChild(row);

    TagNode* td = new TagNode("td");
    row->addNewChild(td);
    td->setAttribute("width", "3");
    td->addNewChild(new TextNode("first"));

    TagNode* th = new TagNode("th");
    row->addNewChild(th);
    th->addNewChild(new TextNode("second"));
    th->setAttribute("align", "right");

    TagNode* tn = new TagNode("tn");
    row->addNewChild(tn);
    tn->addNewChild(new TextNode("3"));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<table align=\"center\" class=\"normaltable\"><tr><td valign=\"top\" width=\"48\">first</td><th valign=\"top\" align=\"right\">second</th><td valign=\"top\" align=\"right\">3</td></tr></table>");
}

/** Test definition list. */
AFL_TEST("util.doc.HtmlRenderer:definition", a)
{
    // <dl>
    //  <di term=t1>ex1</di>
    //  <di term=t2>ex2</di>
    //  <di        >ex3</di>      <!-- irregular case -->
    //  <dt>ex4</dt>
    //  <dd>t5</dd>
    // </dl>
    Nodes_t nodes;
    TagNode* dl = new TagNode("dl");
    nodes.pushBackNew(dl);

    TagNode* e1 = new TagNode("di");
    dl->addNewChild(e1);
    e1->setAttribute("term", "t1");
    e1->addNewChild(new TextNode("ex1"));

    TagNode* e2 = new TagNode("di");
    dl->addNewChild(e2);
    e2->setAttribute("term", "t2");
    e2->addNewChild(new TextNode("ex2"));

    TagNode* e3 = new TagNode("di");
    dl->addNewChild(e3);
    e3->addNewChild(new TextNode("ex3"));

    TagNode* e4 = new TagNode("dt");
    dl->addNewChild(e4);
    e4->addNewChild(new TextNode("ex4"));

    TagNode* e5 = new TagNode("dd");
    dl->addNewChild(e5);
    e5->addNewChild(new TextNode("t5"));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<dl><dt>t1</dt><dd>ex1</dd><dt>t2</dt><dd>ex2</dd><dd>ex3</dd><dt>ex4</dt><dd>t5</dd></dl>");
}

/** Rendering a single key. */
AFL_TEST("util.doc.HtmlRenderer:key", a)
{
    // <p>press <kbd>Ctrl+C</kbd></p>
    Nodes_t nodes;
    TagNode* p = new TagNode("p");
    nodes.pushBackNew(p);
    p->addNewChild(new TextNode("press "));

    TagNode* link = new TagNode("kbd");
    p->addNewChild(link);
    link->addNewChild(new TextNode("Ctrl+C"));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p>press <kbd>Ctrl</kbd>+<kbd>C</kbd></p>");
}

/** Rendering normal text markup. */
AFL_TEST("util.doc.HtmlRenderer:markup", a)
{
    // <p><b>bold</b><u>underline</u><em>emphasize</em><tt>typewriter</tt></p>
    Nodes_t nodes;
    TagNode* p = new TagNode("p");
    nodes.pushBackNew(p);

    TagNode* b = new TagNode("b");
    p->addNewChild(b);
    b->addNewChild(new TextNode("bold"));

    TagNode* u = new TagNode("u");
    p->addNewChild(u);
    u->addNewChild(new TextNode("underline"));

    TagNode* em = new TagNode("em");
    p->addNewChild(em);
    em->addNewChild(new TextNode("emphasize"));

    TagNode* tt = new TagNode("tt");
    p->addNewChild(tt);
    tt->addNewChild(new TextNode("typewriter"));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p><b>bold</b><u>underline</u><em>emphasize</em><tt>typewriter</tt></p>");
}

/** Rendering more normal text markup. */
AFL_TEST("util.doc.HtmlRenderer:markup:2", a)
{
    // <p><b>bold</b><u>underline</u><em>emphasize</em><tt>typewriter</tt></p>
    Nodes_t nodes;
    TagNode* p = new TagNode("p");
    nodes.pushBackNew(p);

    TagNode* cfg = new TagNode("cfg");
    p->addNewChild(cfg);
    cfg->addNewChild(new TextNode("ConfOpt"));

    TagNode* font = new TagNode("font");
    p->addNewChild(font);
    font->setAttribute("color", "red");
    font->addNewChild(new TextNode("red it"));

    TagNode* small = new TagNode("small");
    p->addNewChild(small);
    small->addNewChild(new TextNode("little"));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p><tt>ConfOpt</tt><span class=\"color-red\">red it</span><small>little</small></p>");
}

/** Rendering preformatted, bare. */
AFL_TEST("util.doc.HtmlRenderer:pre:bare", a)
{
    Nodes_t nodes;
    TagNode* p = new TagNode("pre");
    nodes.pushBackNew(p);
    p->setAttribute("class", "bare");
    p->addNewChild(new TextNode("a\n<b"));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<pre>a\n&lt;b</pre>");
}

/** Rendering preformatted, default. */
AFL_TEST("util.doc.HtmlRenderer:pre:default", a)
{
    Nodes_t nodes;
    TagNode* p = new TagNode("pre");
    nodes.pushBackNew(p);
    p->addNewChild(new TextNode("a\n<b"));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<pre class=\"code\">a\n&lt;b</pre>");
}

/** Rendering a list. */
AFL_TEST("util.doc.HtmlRenderer:list", a)
{
    Nodes_t nodes;
    TagNode* p = new TagNode("ul");
    nodes.pushBackNew(p);

    TagNode* li = new TagNode("li");
    p->addNewChild(li);
    li->addNewChild(new TextNode("it..."));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<ul><li>it...</li></ul>");
}

/** Test box markup. */
AFL_TEST("util.doc.HtmlRenderer:infobox", a)
{
    Nodes_t nodes;
    TagNode* p = new TagNode("infobox");
    nodes.pushBackNew(p);
    p->addNewChild(new TextNode("a"));

    TagNode* q = new TagNode("infobox");
    nodes.pushBackNew(q);
    q->setAttribute("id", "i2");
    q->setAttribute("type", "warning");
    q->addNewChild(new TextNode("b"));

    RenderOptions opts;
    String_t result = renderHTML(nodes, opts);

    a.checkEqual("01. result", result, "<p class=\"infobox\">a</p><p id=\"i2\" class=\"infobox-warning\">b</p>");
}
