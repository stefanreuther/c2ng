/**
  *  \file u/t_ui_widgets_treelistbox.cpp
  *  \brief Test for ui::widgets::TreeListbox
  */

#include "ui/widgets/treelistbox.hpp"

#include "t_ui_widgets.hpp"
#include "ui/root.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"

typedef ui::widgets::TreeListbox::Node Node_t;

void
TestUiWidgetsTreeListbox::testIt()
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Widget
    ui::widgets::TreeListbox t(root, 10, 100);
    Node_t* a   = t.addNode(1, 0, "a",   true);
    Node_t* a1  = t.addNode(2, 1, "a1",  false);
    Node_t* a2  = t.addNode(3, 1, "a2",  false);
    Node_t* a21 = t.addNode(4, 2, "a21", true);
    Node_t* a22 = t.addNode(5, 2, "a22", true);
    Node_t* a3  = t.addNode(6, 1, "a3",  true);
    Node_t* b   = t.addNode(7, 0, "b",   false);
    Node_t* b1  = t.addNode(8, 1, "b1",  true);

    // Verify inquiry
    TS_ASSERT_EQUALS(t.hasChildren(a),   true);
    TS_ASSERT_EQUALS(t.hasChildren(a1),  false);
    TS_ASSERT_EQUALS(t.hasChildren(a2),  true);
    TS_ASSERT_EQUALS(t.hasChildren(a21), false);
    TS_ASSERT_EQUALS(t.hasChildren(b1),  false);

    TS_ASSERT_EQUALS(t.findNodeById(5), a22);
    TS_ASSERT_EQUALS(t.findNodeById(55), (Node_t*) 0);

    TS_ASSERT_EQUALS(t.getIdFromNode(a2), 3);
    TS_ASSERT_EQUALS(t.getIdFromNode(b1), 8);

    // Verify rendering: content should be
    //    + a
    //        a1
    //      - a2
    //        a3
    //    - b
    TS_ASSERT_EQUALS(t.getNumItems(), 5U);
    TS_ASSERT_EQUALS(t.getNodeFromItem(0), a);
    TS_ASSERT_EQUALS(t.getNodeFromItem(1), a1);
    TS_ASSERT_EQUALS(t.getNodeFromItem(2), a2);
    TS_ASSERT_EQUALS(t.getNodeFromItem(3), a3);
    TS_ASSERT_EQUALS(t.getNodeFromItem(4), b);

    // Toggle a, verify rendering
    t.toggleNode(a);
    TS_ASSERT_EQUALS(t.getNumItems(), 2U);
    TS_ASSERT_EQUALS(t.getNodeFromItem(0), a);
    TS_ASSERT_EQUALS(t.getNodeFromItem(1), b);

    // Toggle a again (a2 remains closed)
    t.toggleNode(a);
    TS_ASSERT_EQUALS(t.getNumItems(), 5U);
}

