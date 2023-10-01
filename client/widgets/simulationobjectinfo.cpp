/**
  *  \file client/widgets/simulationobjectinfo.cpp
  */

#include "client/widgets/simulationobjectinfo.hpp"
#include "afl/string/format.hpp"
#include "client/widgets/collapsibledataview.hpp"
#include "game/sim/ability.hpp"
#include "game/sim/object.hpp"
#include "ui/group.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/res/resid.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/simpletable.hpp"
#include "util/rich/parser.hpp"
#include "util/string.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/statictext.hpp"

namespace {
    const uint8_t LABEL_COLOR = ui::Color_Black;
    const uint8_t GREEN_COLOR = ui::Color_GreenBlack;
    const uint8_t YELLOW_COLOR = ui::Color_DarkYellow;

    int getButtonSize(ui::Root& root)
    {
        return root.provider().getFont("")->getLineHeight();
    }

    struct ButtonInfo {
        int x;
        int y;
        ui::widgets::BaseButton* w;
        ButtonInfo(int x, int y, ui::widgets::BaseButton* w)
            : x(x), y(y), w(w)
            { }
    };
    typedef std::vector<ButtonInfo> ButtonInfos_t;

    void placeButtons(ui::Root& root, gfx::Point refPoint, const ButtonInfos_t& infos)
    {
        const int grid = getButtonSize(root);
        for (size_t i = 0; i < infos.size(); ++i) {
            infos[i].w->setExtent(gfx::Rectangle(refPoint + gfx::Point(grid, grid).scaledBy(-1-infos[i].x, infos[i].y), gfx::Point(grid-1, grid-1)));
        }
    }

    void setButtonState(ButtonInfos_t& infos, util::Key_t key, ui::Widget::State st, bool enable)
    {
        for (size_t i = 0; i < infos.size(); ++i) {
            if (infos[i].w->getKey() == key) {
                infos[i].w->setState(st, enable);
            }
        }
    }

    void setButtonFlag(ButtonInfos_t& infos, util::Key_t key, ui::ButtonFlag fl, bool enable)
    {
        for (size_t i = 0; i < infos.size(); ++i) {
            if (infos[i].w->getKey() == key) {
                infos[i].w->setFlag(fl, enable);
            }
        }
    }

    class ButtonMaker {
     public:
        ButtonMaker(ui::Root& root, ui::Widget& keyHandler, ui::Widget& parent, ButtonInfos_t& infos, afl::base::Deleter& del)
            : m_root(root), m_keyHandler(keyHandler), m_parent(parent), m_infos(infos), m_deleter(del)
            { }

        void add(int x, int y, const char* label, util::Key_t key);

     private:
        ui::Root& m_root;
        ui::Widget& m_keyHandler;
        ui::Widget& m_parent;
        ButtonInfos_t& m_infos;
        afl::base::Deleter& m_deleter;
    };
}

void
ButtonMaker::add(int x, int y, const char* label, util::Key_t key)
{
    ui::widgets::Button& btn = m_deleter.addNew(new ui::widgets::Button(label, key, m_root));
    btn.dispatchKeyTo(m_keyHandler);
    btn.setFont("-");
    m_parent.addChild(btn, 0);
    m_infos.push_back(ButtonInfo(x, y, &btn));
}


/*
 *  Header
 */

class client::widgets::SimulationObjectInfo::Header : public CollapsibleDataView, public Child {
 public:
    Header(ui::Root& root, ui::Widget& keyHandler, afl::string::Translator& tx, bool isPlanet)
        : CollapsibleDataView(root), Child(),
          m_translator(tx), m_isPlanet(isPlanet),
          m_imageButton("", 0, root, gfx::Point(105, 93)),
          m_imageFrame(ui::layout::VBox::instance5, root.colorScheme(), ui::LoweredFrame),
          m_firstTable(root, 2, 6),
          m_secondTable(root, 2, 3)
        {
            m_imageButton.setBackgroundColor(ui::Color_Black);
            init(keyHandler);
        }

    // CollapsibleDataView:
    virtual void setChildPositions()
        {
            const gfx::Point dataAnchor = getAnchorPoint(LeftAligned | DataAligned);
            const gfx::Point imageSize = m_imageFrame.getLayoutInfo().getPreferredSize();
            const gfx::Point firstSize = m_firstTable.getLayoutInfo().getPreferredSize();
            const gfx::Point secondSize = m_secondTable.getLayoutInfo().getPreferredSize();

            const int firstHeight = std::max(imageSize.getY() + 5, firstSize.getY());
            const int imageSpace = imageSize.getX() + 5;
            const int buttonSpace = 2*getButtonSize(root());

            // Image: place as-is
            m_imageFrame.setExtent(gfx::Rectangle(dataAnchor, imageSize));

            // First table
            m_firstTable.setExtent(gfx::Rectangle(dataAnchor.getX() + imageSpace,
                                                  dataAnchor.getY(),
                                                  getExtent().getWidth() - imageSpace - buttonSpace,
                                                  firstSize.getY()));

            // Second table
            m_secondTable.setExtent(gfx::Rectangle(dataAnchor.getX(),
                                                   dataAnchor.getY() + firstHeight,
                                                   getExtent().getWidth() - buttonSpace,
                                                   secondSize.getY()));

            const gfx::Point buttonAnchor = getAnchorPoint(DataAligned);
            placeButtons(root(), buttonAnchor, m_firstButtons);
            placeButtons(root(), buttonAnchor + gfx::Point(0, firstHeight), m_secondButtons);
        }

    virtual gfx::Point getPreferredChildSize() const
        {
            gfx::Point imageSize = m_imageButton.getLayoutInfo().getPreferredSize() + gfx::Point(5, 5);
            gfx::Point firstSize = m_firstTable.getLayoutInfo().getPreferredSize();
            gfx::Point secondSize = m_secondTable.getLayoutInfo().getPreferredSize();

            return imageSize.extendRight(firstSize).extendBelow(secondSize).extendRight(gfx::Point(0, 2*getButtonSize(root())));
        }

    // Child:
    virtual void setContent(const ObjectInfo_t& info)
        {
            // Reject if data is not for us to save some time
            if (info.isPlanet != m_isPlanet) {
                return;
            }

            m_firstTable.cell(1, 0).setText(m_isPlanet ? m_translator("Planet") : info.hullType.second);
            m_firstTable.cell(1, 1).setText(info.owner.second);
            m_firstTable.cell(1, 2).setText(info.name);
            m_firstTable.cell(1, 3).setText(afl::string::Format("%d", info.id));
            m_firstTable.cell(1, 4).setText(info.friendlyCode);
            if (!m_isPlanet) {
                if ((info.flags & game::sim::Object::fl_RatingOverride) != 0) {
                    m_firstTable.cell(1, 5).setText(afl::string::Format("%d / %d", info.flakRatingOverride, info.flakCompensationOverride))
                        .setColor(YELLOW_COLOR);
                } else {
                    m_firstTable.cell(1, 5).setText(afl::string::Format("%d / %d", info.defaultFlakRating, info.defaultFlakCompensation))
                        .setColor(GREEN_COLOR);
                }
            }

            // Colors
            String_t fcColors;
            if ((info.flags & game::sim::Object::fl_RandomFC) != 0) {
                uint32_t which = (info.flags & game::sim::Object::fl_RandomDigits);
                if (which == 0) {
                    which = game::sim::Object::fl_RandomDigits;
                }
                for (size_t i = 0; i < 3; ++i) {
                    if ((which & (game::sim::Object::fl_RandomFC1 << i)) != 0) {
                        fcColors += char(YELLOW_COLOR);
                    } else {
                        fcColors += char(GREEN_COLOR);
                    }
                }
            }
            m_firstTable.cell(1, 4).setColorString(fcColors);

            if (m_isPlanet) {
                m_secondTable.cell(1, 0).setText(afl::string::Format("%d", info.defense));
                m_secondTable.cell(1, 1).setText(game::sim::toString(info.abilities, m_translator));
                m_secondTable.cell(1, 1).setColor(info.hasAnyNonstandardAbility ? YELLOW_COLOR : GREEN_COLOR);
                m_secondTable.cell(1, 2).setText(info.experienceLevel.second);
                m_imageButton.setImage(ui::res::PLANET);
            } else {
                m_secondTable.cell(1, 0).setText(afl::string::Format("%d%%", info.damage));
                m_secondTable.cell(1, 1).setText(afl::string::Format("%d, %s", info.crew, info.experienceLevel.second));
                m_secondTable.cell(1, 2).setText(afl::string::Format("%d%%, %d kt", info.shield, info.mass));
                if (info.hullType.first == 0 || info.hullPicture == 0) {
                    m_imageButton.setImage(ui::res::SHIP);
                } else {
                    m_imageButton.setImage(ui::res::makeResourceId(ui::res::SHIP, info.hullPicture, info.hullType.first));
                }
            }

            // Status
            setButtonFlag(m_firstButtons, 'r', ui::HighlightedButton, (info.flags & game::sim::Object::fl_RandomFC) != 0);
            if (!m_isPlanet) {
                setButtonState(m_secondButtons, 'm', DisabledState, info.hullType.first != 0);
            }
        }

 private:
    afl::string::Translator& m_translator;
    const bool m_isPlanet;

    ui::widgets::ImageButton m_imageButton;
    ui::widgets::FrameGroup m_imageFrame;
    ui::widgets::SimpleTable m_firstTable;
    ui::widgets::SimpleTable m_secondTable;

    afl::base::Deleter m_deleter;
    ButtonInfos_t m_firstButtons;
    ButtonInfos_t m_secondButtons;

    void init(ui::Widget& keyHandler);
};

void
client::widgets::SimulationObjectInfo::Header::init(ui::Widget& keyHandler)
{
    afl::string::Translator& tx = m_translator;

    const int em = root().provider().getFont(gfx::FontRequest())->getEmWidth();

    // Configure view
    if (m_isPlanet) {
        setTitle(tx("Planet"));
    } else {
        setTitle(tx("Ship"));
    }
    setViewState(Complete);

    // Configure first table
    m_firstTable.column(0).setColor(LABEL_COLOR);
    m_firstTable.column(1).setColor(GREEN_COLOR);
    m_firstTable.cell(0, 0).setText(tx("Type:"));
    m_firstTable.cell(0, 1).setText(tx("Owner:"));
    m_firstTable.cell(0, 2).setText(tx("Name:"));
    m_firstTable.cell(0, 3).setText(tx("Id:"));
    m_firstTable.cell(0, 4).setText(tx("FCode:"));
    if (!m_isPlanet) {
        m_firstTable.cell(0, 5).setText(tx("Rating:"));
    }
    m_firstTable.setColumnWidth(1, 20*em);
    m_firstTable.setColumnPadding(0, 5);

    // Configure second table
    m_secondTable.column(0).setColor(LABEL_COLOR);
    m_secondTable.column(1).setColor(GREEN_COLOR);
    if (m_isPlanet) {
        m_secondTable.cell(0, 0).setText(tx("Defense:"));
        m_secondTable.cell(0, 1).setText(tx("Abilities:"));
        m_secondTable.cell(0, 2).setText(tx("Experience:"));
    } else {
        m_secondTable.cell(0, 0).setText(tx("Damage:"));
        m_secondTable.cell(0, 1).setText(tx("Crew:"));
        m_secondTable.cell(0, 2).setText(tx("Shield:"));
    }
    m_secondTable.setColumnWidth(1, 30*em);
    m_secondTable.setColumnPadding(0, 5);

    // Widget structure
    m_imageFrame.add(m_imageButton);
    addChild(m_imageFrame, 0);
    addChild(m_firstTable, 0);
    addChild(m_secondTable, 0);

    // Buttons
    ButtonMaker firstMaker(root(), keyHandler, *this, m_firstButtons, m_deleter);
    if (!m_isPlanet) {
        firstMaker.add(0, 0, "T", 't');
    }
    firstMaker.add(0, 1, "O", 'o');
    firstMaker.add(0, 2, "N", 'n');
    firstMaker.add(0, 3, "I", 'i');
    firstMaker.add(0, 4, "F", 'f');
    firstMaker.add(1, 4, "R", 'r');
    if (!m_isPlanet) {
        firstMaker.add(0, 5, "K", 'k');
    }

    ButtonMaker secondMaker(root(), keyHandler, *this, m_secondButtons, m_deleter);
    if (m_isPlanet) {
        secondMaker.add(0, 0, "P", 'p');
        secondMaker.add(1, 0, "D", 'd');
        secondMaker.add(0, 1, "Y", 'y');
        secondMaker.add(0, 2, "L", 'l');
    } else {
        secondMaker.add(0, 0, "D", 'd');
        secondMaker.add(0, 1, "C", 'c');
        secondMaker.add(1, 1, "L", 'l');
        secondMaker.add(0, 2, "S", 's');
        secondMaker.add(1, 2, "M", 'm');
    }
}

/*
 *  ShipWeapons
 */

class client::widgets::SimulationObjectInfo::ShipWeapons : public CollapsibleDataView, public Child {
 public:
    // ex WSimShipWeaponTile::drawContent (part)
    ShipWeapons(ui::Root& root, ui::Widget& keyHandler, afl::string::Translator& tx)
        : CollapsibleDataView(root), Child(),
          m_translator(tx),
          m_table(root, 2, 3)
        {
            init(keyHandler);
        }

    // CollapsibleDataView:
    virtual void setChildPositions()
        {
            const gfx::Point dataAnchor = getAnchorPoint(LeftAligned | DataAligned);
            const gfx::Point tableSize = m_table.getLayoutInfo().getPreferredSize();
            m_table.setExtent(gfx::Rectangle(dataAnchor.getX(), dataAnchor.getY(), getExtent().getWidth() - getButtonSize(root()), tableSize.getY()));
            placeButtons(root(), getAnchorPoint(DataAligned), m_buttons);
        }

    virtual gfx::Point getPreferredChildSize() const
        {
            gfx::Point tableSize = m_table.getLayoutInfo().getPreferredSize();
            tableSize.addX(getButtonSize(root()));
            return tableSize;
        }

    // Child:
    virtual void setContent(const ObjectInfo_t& info)
        {
            // Reject if data is not for us to save some time
            if (info.isPlanet) {
                return;
            }

            afl::string::Translator& tx = m_translator;
            if (info.numBeams != 0) {
                m_table.cell(1, 0).setText(afl::string::Format(tx("%d \xC3\x97 %s"), info.numBeams, info.beamType.second));
            } else {
                m_table.cell(1, 0).setText(tx("none"));
            }
            if (info.numLaunchers != 0) {
                m_table.cell(1, 1).setText(afl::string::Format(tx("%d %s launcher%!1{s%}"), info.numLaunchers, info.torpedoType.second));
                m_table.cell(1, 2).setText(afl::string::Format(tx("%d torpedo%!1{es%}"), info.ammo));
            } else if (info.numBays != 0) {
                m_table.cell(1, 1).setText(afl::string::Format(tx("%d fighter bay%!1{s%}"), info.numBays));
                m_table.cell(1, 2).setText(afl::string::Format(tx("%d fighter%!1{s%}"), info.ammo));
            } else {
                m_table.cell(1, 1).setText(tx("none"));
                m_table.cell(1, 2).setText(String_t());
            }

            setButtonState(m_buttons, '1', DisabledState, !info.allowPrimaryWeapons);
            setButtonState(m_buttons, '2', DisabledState, !info.allowSecondaryWeapons);
        }

 private:
    afl::string::Translator& m_translator;
    ui::widgets::SimpleTable m_table;
    afl::base::Deleter m_deleter;
    ButtonInfos_t m_buttons;

    void init(ui::Widget& keyHandler);
};

void
client::widgets::SimulationObjectInfo::ShipWeapons::init(ui::Widget& keyHandler)
{
    afl::string::Translator& tx = m_translator;

    const int em = root().provider().getFont(gfx::FontRequest())->getEmWidth();

    // Configure view
    setTitle(tx("Weapons"));
    setViewState(Complete);

    // Configure table
    m_table.column(0).setColor(LABEL_COLOR);
    m_table.column(1).setColor(GREEN_COLOR);
    m_table.cell(0, 0).setText(tx("Primary:"));
    m_table.cell(0, 1).setText(tx("Secondary:"));
    m_table.setColumnWidth(1, 20*em);
    m_table.setColumnPadding(0, 5);

    addChild(m_table, 0);

    // Buttons
    ButtonMaker maker(root(), keyHandler, *this, m_buttons, m_deleter);
    maker.add(0, 0, "1", '1');
    maker.add(0, 1, "2", '2');
}

/*
 *  ShipDetails
 */

class client::widgets::SimulationObjectInfo::ShipDetails : public CollapsibleDataView, public Child {
 public:
    // ex WSimShipWeaponTile::drawContent (part)
    ShipDetails(ui::Root& root, ui::Widget& keyHandler, afl::string::Translator& tx)
        : CollapsibleDataView(root), Child(),
          m_translator(tx),
          m_table(root, 2, 4)
        {
            init(keyHandler);
        }

    // CollapsibleDataView:
    virtual void setChildPositions()
        {
            const gfx::Point dataAnchor = getAnchorPoint(LeftAligned | DataAligned);
            const gfx::Point tableSize = m_table.getLayoutInfo().getPreferredSize();
            m_table.setExtent(gfx::Rectangle(dataAnchor.getX(), dataAnchor.getY(), getExtent().getWidth() - 2*getButtonSize(root()), tableSize.getY()));
            placeButtons(root(), getAnchorPoint(DataAligned), m_buttons);
        }

    virtual gfx::Point getPreferredChildSize() const
        {
            gfx::Point tableSize = m_table.getLayoutInfo().getPreferredSize();
            tableSize.addX(2*getButtonSize(root()));
            return tableSize;
        }

    // Child:
    virtual void setContent(const ObjectInfo_t& info)
        {
            // Reject if data is not for us to save some time
            if (info.isPlanet) {
                return;
            }

            afl::string::Translator& tx = m_translator;
            m_table.cell(1, 0).setText(info.engineType.second);

            String_t aggText = info.aggressiveness.second;
            if ((info.flags & game::sim::Object::fl_Cloaked) != 0) {
                util::addListItem(aggText, ", ", tx("cloaked"));
            }
            m_table.cell(1, 1).setText(aggText);

            m_table.cell(1, 2).setText(game::sim::toString(info.abilities, m_translator));
            m_table.cell(1, 2).setColor(info.hasAnyNonstandardAbility ? YELLOW_COLOR : GREEN_COLOR);

            m_table.cell(1, 3).setText(info.interceptId.second);
        }

 private:
    afl::string::Translator& m_translator;
    ui::widgets::SimpleTable m_table;
    afl::base::Deleter m_deleter;
    ButtonInfos_t m_buttons;

    void init(ui::Widget& keyHandler);
};

void
client::widgets::SimulationObjectInfo::ShipDetails::init(ui::Widget& keyHandler)
{
    afl::string::Translator& tx = m_translator;

    const int em = root().provider().getFont(gfx::FontRequest())->getEmWidth();

    // Configure view
    setTitle(tx("Details"));
    setViewState(Complete);

    // Configure table
    m_table.column(0).setColor(LABEL_COLOR);
    m_table.column(1).setColor(GREEN_COLOR);
    m_table.cell(0, 0).setText(tx("Engine:"));
    m_table.cell(0, 1).setText(tx("Aggressive:"));
    m_table.cell(0, 2).setText(tx("Abilities:"));
    m_table.cell(0, 3).setText(tx("Intercept-Att.:"));
    m_table.setColumnWidth(1, 20*em);
    m_table.setColumnPadding(0, 5);

    addChild(m_table, 0);

    // Buttons
    ButtonMaker maker(root(), keyHandler, *this, m_buttons, m_deleter);
    maker.add(0, 0, "E", 'e');
    maker.add(0, 1, "V", 'v');
    maker.add(1, 1, "A", 'a');
    maker.add(0, 2, "Y", 'y');
    maker.add(0, 3, "X", 'x');
}

/*
 *  BaseInfo
 */

class client::widgets::SimulationObjectInfo::BaseInfo : public CollapsibleDataView, public Child {
 public:
    // ex WSimPlanetBaseTile
    BaseInfo(ui::Root& root, ui::Widget& keyHandler, afl::string::Translator& tx)
        : CollapsibleDataView(root), Child(),
          m_translator(tx),
          m_table(root, 2, 5)
        {
            init(keyHandler);
        }

    // CollapsibleDataView:
    virtual void setChildPositions()
        {
            const gfx::Point dataAnchor = getAnchorPoint(LeftAligned | DataAligned);
            const gfx::Point tableSize = m_table.getLayoutInfo().getPreferredSize();
            m_table.setExtent(gfx::Rectangle(dataAnchor.getX(), dataAnchor.getY(), getExtent().getWidth() - 2*getButtonSize(root()), tableSize.getY()));
            placeButtons(root(), getAnchorPoint(DataAligned), m_buttons);
        }

    virtual gfx::Point getPreferredChildSize() const
        {
            gfx::Point tableSize = m_table.getLayoutInfo().getPreferredSize();
            tableSize.addX(2*getButtonSize(root()));
            return tableSize;
        }

    // Child:
    virtual void setContent(const ObjectInfo_t& info)
        {
            // Reject if data is not for us to save some time
            if (!info.isPlanet) {
                return;
            }

            afl::string::Translator& tx = m_translator;
            if (!info.hasBase) {
                m_table.cell(1, 0).setText(tx("(no starbase)"));
                m_table.column(1).subrange(1, 5).setText(String_t());
            } else {
                m_table.cell(1, 0).setText(afl::string::Format("%d", info.baseBeamTech));
                m_table.cell(1, 1).setText(afl::string::Format("%d", info.numBaseFighters));
                m_table.cell(1, 2).setText(afl::string::Format("%d", info.baseDefense));
                m_table.cell(1, 3).setText(afl::string::Format("%d", info.baseTorpedoTech));
                m_table.cell(1, 4).setText(afl::string::Format("%d", info.effBaseTorpedoes));
            }

            setButtonState(m_buttons, 'g', DisabledState, !info.hasBase);
            setButtonState(m_buttons, 's', DisabledState, !info.hasBase);
            setButtonState(m_buttons, 't', DisabledState, !info.hasBase);
            setButtonState(m_buttons, 'a', DisabledState, !info.hasBase);
        }

 private:
    afl::string::Translator& m_translator;
    ui::widgets::SimpleTable m_table;
    afl::base::Deleter m_deleter;
    ButtonInfos_t m_buttons;

    void init(ui::Widget& keyHandler);
};

void
client::widgets::SimulationObjectInfo::BaseInfo::init(ui::Widget& keyHandler)
{
    // ex WSimPlanetBaseTile::init
    afl::string::Translator& tx = m_translator;

    const int em = root().provider().getFont(gfx::FontRequest())->getEmWidth();

    // Configure view
    setTitle(tx("Starbase"));
    setViewState(Complete);

    // Configure table
    m_table.column(0).setColor(LABEL_COLOR);
    m_table.column(1).setColor(GREEN_COLOR);
    m_table.cell(0, 0).setText(tx("Beam Tech:"));
    m_table.cell(0, 1).setText(tx("Fighters:"));
    m_table.cell(0, 2).setText(tx("Starbase Defense:"));
    m_table.cell(0, 3).setText(tx("Torpedo Tech:"));
    m_table.cell(0, 4).setText(tx("Torpedoes:"));
    m_table.setColumnWidth(1, 20*em);
    m_table.setColumnPadding(0, 5);

    addChild(m_table, 0);

    // Buttons
    ButtonMaker maker(root(), keyHandler, *this, m_buttons, m_deleter);
    maker.add(0, 0, "B", 'b');
    maker.add(0, 1, "G", 'g');
    maker.add(0, 2, "S", 's');
    maker.add(0, 3, "T", 't');
    maker.add(0, 4, "A", 'a');
}

/*
 *  Footer
 */

class client::widgets::SimulationObjectInfo::Footer : public ui::Group, public Child {
 public:
    Footer(ui::Root& root, ui::Widget& keyHandler, afl::string::Translator& tx, bool isPlanet)
        : Group(ui::layout::HBox::instance5),
          m_updateButton("U", 'u', root),
          m_writeButton("W", 'w', root),
          m_gotoButton(isPlanet ? "F2" : "F1", isPlanet ? util::Key_F2 : util::Key_F1, root),
          m_text(tx("Update/write back"), util::SkinColor::Static, "+", root.provider())
        {
            init(keyHandler);
        }

    // Child:
    virtual void setContent(const ObjectInfo_t& info)
        {
            using game::sim::GameInterface;

            m_updateButton.setState(DisabledState, info.relation < GameInterface::ReadOnly);
            m_writeButton.setState(DisabledState, info.relation < GameInterface::Playable);
            m_gotoButton.setState(DisabledState, info.relation < GameInterface::Playable);
        }

 private:
    ui::widgets::Button m_updateButton;
    ui::widgets::Button m_writeButton;
    ui::widgets::Button m_gotoButton;
    ui::widgets::StaticText m_text;

    void init(ui::Widget& keyHandler);
};

void
client::widgets::SimulationObjectInfo::Footer::init(ui::Widget& keyHandler)
{
    m_updateButton.dispatchKeyTo(keyHandler);
    m_writeButton.dispatchKeyTo(keyHandler);
    m_gotoButton.dispatchKeyTo(keyHandler);

    m_text.setIsFlexible(true);

    add(m_updateButton);
    add(m_writeButton);
    add(m_text);
    add(m_gotoButton);
}



/*
 *  SimulationObjectInfo
 */

client::widgets::SimulationObjectInfo::SimulationObjectInfo(ui::Root& root, ui::Widget& keyHandler, afl::string::Translator& tx)
    : CardGroup(),
      m_root(root),
      m_translator(tx),
      m_children(),
      m_emptyPage(),
      m_planetPage(),
      m_shipPage(),
      m_introPage()
{
    init(keyHandler);
}

client::widgets::SimulationObjectInfo::~SimulationObjectInfo()
{ }

void
client::widgets::SimulationObjectInfo::setContent(const ObjectInfo_t& info)
{
    // Dispatch into children
    for (size_t i = 0, n = m_children.size(); i < n; ++i) {
        m_children[i]->setContent(info);
    }

    // Pick correct child
    ui::Widget* child = (info.isPlanet ? m_planetPage : m_shipPage);
    if (child) {
        setFocusedChild(child);
    }
}

void
client::widgets::SimulationObjectInfo::clearContent()
{
    if (m_emptyPage) {
        setFocusedChild(m_emptyPage);
    }
}

void
client::widgets::SimulationObjectInfo::showIntroPage()
{
    if (m_introPage) {
        setFocusedChild(m_introPage);
    }
}

inline void
client::widgets::SimulationObjectInfo::init(ui::Widget& keyHandler)
{
    // Empty page
    m_emptyPage = &m_deleter.addNew(new ui::Group(ui::layout::VBox::instance5));
    add(*m_emptyPage);

    // Planet page
    ui::Group& planetPage = m_deleter.addNew(new ui::Group(ui::layout::VBox::instance5));
    add(planetPage);
    Header& planetHeader = m_deleter.addNew(new Header(m_root, keyHandler, m_translator, true));
    planetPage.add(planetHeader);
    m_children.push_back(&planetHeader);

    BaseInfo& baseInfo = m_deleter.addNew(new BaseInfo(m_root, keyHandler, m_translator));
    planetPage.add(baseInfo);
    m_children.push_back(&baseInfo);

    planetPage.add(m_deleter.addNew(new ui::Spacer()));

    Footer& planetFooter = m_deleter.addNew(new Footer(m_root, keyHandler, m_translator, true));
    planetPage.add(planetFooter);
    m_children.push_back(&planetFooter);

    m_planetPage = &planetPage;

    // Ship page
    ui::Group& shipPage = m_deleter.addNew(new ui::Group(ui::layout::VBox::instance5));
    add(shipPage);
    Header& shipHeader = m_deleter.addNew(new Header(m_root, keyHandler, m_translator, false));
    shipPage.add(shipHeader);
    m_children.push_back(&shipHeader);

    ShipWeapons& shipWeapons = m_deleter.addNew(new ShipWeapons(m_root, keyHandler, m_translator));
    shipPage.add(shipWeapons);
    m_children.push_back(&shipWeapons);

    ShipDetails& shipDetails = m_deleter.addNew(new ShipDetails(m_root, keyHandler, m_translator));
    shipPage.add(shipDetails);
    m_children.push_back(&shipDetails);

    shipPage.add(m_deleter.addNew(new ui::Spacer()));

    Footer& shipFooter = m_deleter.addNew(new Footer(m_root, keyHandler, m_translator, false));
    shipPage.add(shipFooter);
    m_children.push_back(&shipFooter);

    m_shipPage = &shipPage;

    // Intro page
    ui::Group& introPage = m_deleter.addNew(new ui::Group(ui::layout::VBox::instance5));
    add(introPage);
    introPage.add(m_deleter.addNew(new ui::rich::StaticText(
                                       util::rich::Parser::parseXml(m_translator("<big>Welcome to the Battle Simulator!</big>\n\n"
                                                                                 "Press <kbd>Ins</kbd> to add a ship, <kbd>P</kbd> to add a planet.")),
                                       30 * m_root.provider().getFont("")->getEmWidth(),
                                       m_root.provider())));
    introPage.add(m_deleter.addNew(new ui::Spacer()));

    m_introPage = &introPage;
}
