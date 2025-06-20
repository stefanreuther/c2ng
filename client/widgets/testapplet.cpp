/**
  *  \file client/widgets/testapplet.cpp
  *  \brief Class client::widgets::TestApplet
  */

#include "client/widgets/testapplet.hpp"
#include "client/widgets/alliancestatuslist.hpp"
#include "client/widgets/filelistbox.hpp"
#include "client/widgets/playerlist.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "game/map/object.hpp"
#include "game/ref/userlist.hpp"
#include "game/reference.hpp"

using game::map::Object;
using game::Reference;

// PlayerList widget
client::widgets::TestApplet*
client::widgets::TestApplet::makePlayerList(bool flow)
{
    class Class : public TestApplet {
     public:
        Class(bool flow)
            : m_flow(flow)
            { }
        void runTest(ui::Root& root, afl::string::Translator& /*tx*/)
            {

                int preferredWidth = m_flow ? 300 : 0;
                PlayerList::Layout lay = m_flow ? PlayerList::FlowLayout : PlayerList::VerticalLayout;

                PlayerList pl(root, lay, pl.ShowNames, pl.PlayerColors, preferredWidth, game::PlayerSet_t::allUpTo(12));
                pl.setName(1, "Feds");
                pl.setName(2, "Lizard");
                pl.setName(3, "Bird Men");
                pl.setName(4, "Klingon");
                pl.setName(5, "Privateer");
                pl.setName(6, "Cyborg");
                pl.setName(7, "Tholian");
                pl.setName(8, "Imperial");
                pl.setName(9, "Robot");
                pl.setName(10, "Rebel");
                pl.setName(11, "Colonial");
                pl.setName(12, "Alien");

                testWidget(root, pl);
            }

     private:
        bool m_flow;
    };
    return new Class(flow);
}

// AllianceStatusList widget
client::widgets::TestApplet*
client::widgets::TestApplet::makeAllianceStatusList()
{
    class Class : public TestApplet {
     public:
        void runTest(ui::Root& root, afl::string::Translator& tx)
            {
                AllianceStatusList asl(root, tx);
                asl.add(1, "Federation", AllianceStatusList::ItemFlags_t(AllianceStatusList::Self));
                asl.add(2, "Lizard",     AllianceStatusList::ItemFlags_t(AllianceStatusList::WeOffer));
                asl.add(3, "Bird",       AllianceStatusList::ItemFlags_t(AllianceStatusList::TheyOffer));
                asl.add(4, "Klingon",    AllianceStatusList::ItemFlags_t(AllianceStatusList::TheyOffer) + AllianceStatusList::Enemy);
                asl.add(5, "Orion",      AllianceStatusList::ItemFlags_t());

                testWidget(root, asl);
            }
    };
    return new Class();
}

// ReferenceList widget
client::widgets::TestApplet*
client::widgets::TestApplet::makeReferenceList()
{
    class Class : public TestApplet {
     public:
        void runTest(ui::Root& root, afl::string::Translator& /*tx*/)
            {
                game::ref::UserList ul;
                ul.add(ul.DividerItem,    "SMALL DEEP SPACE FREIGHTER", Reference(), false, Object::Playable, util::SkinColor::Static);
                ul.add(ul.SubdividerItem, "The Lizards",                Reference(), false, Object::Playable, util::SkinColor::Static);
                ul.add(ul.ReferenceItem,  "Listiger Lurch",             Reference(), false, Object::Playable, util::SkinColor::Green);
                ul.add(ul.ReferenceItem,  "Crocodile Dundee",           Reference(), true,  Object::Playable, util::SkinColor::Green);
                ul.add(ul.SubdividerItem, "The Bird Men",               Reference(), false, Object::Playable, util::SkinColor::Static);
                ul.add(ul.ReferenceItem,  "Starling",                   Reference(), false, Object::Playable, util::SkinColor::Red);
                ul.add(ul.ReferenceItem,  "Eagle",                      Reference(), false, Object::Playable, util::SkinColor::Red);
                ul.add(ul.OtherItem,      "Some Link",                  Reference(), false, Object::Playable, util::SkinColor::Static);

                ReferenceListbox list(root);
                list.setContent(ul);
                testWidget(root, list);
            }
    };
    return new Class();
}

// FileListbox widget
client::widgets::TestApplet*
client::widgets::TestApplet::makeFileList()
{
    class Class : public TestApplet {
     public:
        void runTest(ui::Root& root, afl::string::Translator& /*tx*/)
            {
                FileListbox box(2, 7, root);

                FileListbox::Items_t items;
                items.push_back(FileListbox::Item("up", 0, true, FileListbox::iUp));
                for (int i = 0; i < 10; ++i) {
                    items.push_back(FileListbox::Item("directory", 1, true, FileListbox::iFolder));
                }
                for (int i = 0; i < 20; ++i) {
                    items.push_back(FileListbox::Item("file", 1, false, FileListbox::iFile));
                }
                box.swapItems(items);

                testWidget(root, box);
            }
    };
    return new Class();
}
