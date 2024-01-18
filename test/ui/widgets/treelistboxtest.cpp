/**
  *  \file test/ui/widgets/treelistboxtest.cpp
  *  \brief Test for ui::widgets::TreeListbox
  */

#include "ui/widgets/treelistbox.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"

typedef ui::widgets::TreeListbox::Node Node_t;

AFL_TEST("ui.widgets.TreeListbox:basics", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Widget
    ui::widgets::TreeListbox t(root, 10, 100);
    Node_t* na   = t.addNode(1, 0, "a",   true);
    Node_t* na1  = t.addNode(2, 1, "a1",  false);
    Node_t* na2  = t.addNode(3, 1, "a2",  false);
    Node_t* na21 = t.addNode(4, 2, "a21", true);
    Node_t* na22 = t.addNode(5, 2, "a22", true);
    Node_t* na3  = t.addNode(6, 1, "a3",  true);
    Node_t* nb   = t.addNode(7, 0, "b",   false);
    Node_t* nb1  = t.addNode(8, 1, "b1",  true);

    // Verify inquiry
    a.checkEqual("01. hasChildren", t.hasChildren(na),   true);
    a.checkEqual("02. hasChildren", t.hasChildren(na1),  false);
    a.checkEqual("03. hasChildren", t.hasChildren(na2),  true);
    a.checkEqual("04. hasChildren", t.hasChildren(na21), false);
    a.checkEqual("05. hasChildren", t.hasChildren(nb1),  false);

    a.checkEqual("11. findNodeById", t.findNodeById(5), na22);
    a.checkEqual("12. findNodeById", t.findNodeById(55), (Node_t*) 0);

    a.checkEqual("21. getIdFromNode", t.getIdFromNode(na2), 3);
    a.checkEqual("22. getIdFromNode", t.getIdFromNode(nb1), 8);

    // Verify rendering: content should be
    //    + a
    //        a1
    //      - a2
    //        a3
    //    - b
    a.checkEqual("31. getNumItems", t.getNumItems(), 5U);
    a.checkEqual("32. getNodeFromItem", t.getNodeFromItem(0), na);
    a.checkEqual("33. getNodeFromItem", t.getNodeFromItem(1), na1);
    a.checkEqual("34. getNodeFromItem", t.getNodeFromItem(2), na2);
    a.checkEqual("35. getNodeFromItem", t.getNodeFromItem(3), na3);
    a.checkEqual("36. getNodeFromItem", t.getNodeFromItem(4), nb);

    // Toggle a, verify rendering
    t.toggleNode(na);
    a.checkEqual("41. getNumItems", t.getNumItems(), 2U);
    a.checkEqual("42. getNodeFromItem", t.getNodeFromItem(0), na);
    a.checkEqual("43. getNodeFromItem", t.getNodeFromItem(1), nb);

    // Toggle a again (a2 remains closed)
    t.toggleNode(na);
    a.checkEqual("51. getNumItems", t.getNumItems(), 5U);
}

