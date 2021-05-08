/**
  *  \file client/dialogs/newdrawing.cpp
  */

#include "client/dialogs/newdrawing.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/newdrawingtag.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/drawingproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

namespace {
    class NewDrawingDialog {
     public:
        NewDrawingDialog(client::dialogs::NewDrawingInfo& result,
                         ui::Root& root,
                         util::RequestSender<game::Session> gameSender,
                         afl::string::Translator& tx)
            : m_result(result),
              m_root(root),
              m_gameSender(gameSender),
              m_translator(tx),
              m_loop(root),
              m_tag("", util::SkinColor::Static, gfx::FontRequest(), root.provider())
            {
                m_tag.setIsFlexible(true);
                renderTag();
            }

        bool run()
            {
                // ex WNewDrawingDialog::buildDialog
                afl::base::Deleter del;
                // VBox
                //   UIFrameGroup/VBox
                //     "What do you want to draw?"
                //     Grid
                //       P | Polygon
                //       R | Rectangle
                //       C | Circle
                //       M | Marker
                //   UIFrameGroup/HBox
                //     T
                //     "Tag: ..."
                //   HBox
                //     ESC, H

                ui::Window& win = del.addNew(new ui::Window(m_translator("Starchart Drawing"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

                ui::widgets::FrameGroup& g1 = del.addNew(new ui::widgets::FrameGroup(ui::layout::VBox::instance5, m_root.colorScheme(), ui::LoweredFrame));
                g1.setPadding(5);

                ui::Group& g12 = del.addNew(new ui::Group(del.addNew(new ui::layout::Grid(3))));
                ui::Group& g3 = del.addNew(new ui::Group(ui::layout::HBox::instance5));

                ui::widgets::FrameGroup& g2 = del.addNew(new ui::widgets::FrameGroup(ui::layout::HBox::instance5, m_root.colorScheme(), ui::LoweredFrame));
                g2.setPadding(5);

                // FIXME: use OptionGrid?
                addTypeButton(del, g12, "P", 'p', m_translator("Polygon: set of lines, starting here"), game::map::Drawing::LineDrawing);
                addTypeButton(del, g12, "R", 'r', m_translator("Rectangle"),                            game::map::Drawing::RectangleDrawing);
                addTypeButton(del, g12, "C", 'c', m_translator("Circle centered here"),                 game::map::Drawing::CircleDrawing);
                addTypeButton(del, g12, "M", 'm', m_translator("Marker"),                               game::map::Drawing::MarkerDrawing);

                ui::widgets::Button& btnTag = del.addNew(new ui::widgets::Button("T", 't', m_root));
                btnTag.sig_fire.add(this, &NewDrawingDialog::onTag);
                g2.add(btnTag);
                g2.add(m_tag);

                ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:draw"));
                ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(m_translator("Cancel"), util::Key_Escape, m_root));
                ui::widgets::Button& btnHelp   = del.addNew(new ui::widgets::Button(m_translator("Help"), 'h', m_root));
                g3.add(btnHelp);
                g3.add(del.addNew(new ui::Spacer()));
                g3.add(btnCancel);

                g1.add(del.addNew(new ui::widgets::StaticText(m_translator("What do you want to draw into the starcharts?"), util::SkinColor::Static, "+", m_root.provider())));
                g1.add(g12);
                win.add(g1);
                win.add(g2);
                win.add(g3);
                win.add(help);
                win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
                win.pack();

                btnHelp.dispatchKeyTo(help);
                btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));

                m_root.centerWidget(win);
                m_root.add(win);
                return m_loop.run() != 0;
            }

        void onTag()
            {
                // Fetch tag list. Use our own DrawingProxy for simplicity.
                util::StringList tagList;
                client::Downlink link(m_root, m_translator);
                game::proxy::DrawingProxy(m_gameSender, m_root.engine().dispatcher())
                    .getTagList(link, tagList);
                tagList.sortAlphabetically();

                // Dialog
                client::dialogs::NewDrawingTag dlg(tagList, m_root, m_gameSender);
                dlg.setTagName(m_result.tagName);
                if (dlg.run(m_translator("Starchart Drawing"), m_translator, 0)) {
                    m_result.tagName = dlg.getTagName();
                    renderTag();
                }
            }

        void renderTag()
            {
                m_tag.setText(afl::string::Format(m_translator("Tag: %s"), m_result.tagName.empty() ? String_t("none") : m_result.tagName));
            }

        void onConfirm(game::map::Drawing::Type type)
            {
                m_result.type = type;
                m_loop.stop(1);
            }

        void addTypeButton(afl::base::Deleter& del, ui::Group& container, const char* keyLabel, util::Key_t key, String_t label, game::map::Drawing::Type type)
            {
                class Handler : public afl::base::Closure<void(int)> {
                 public:
                    Handler(NewDrawingDialog& me, game::map::Drawing::Type type)
                        : m_me(me), m_type(type)
                        { }
                    void call(int)
                        { m_me.onConfirm(m_type); }
                 private:
                    NewDrawingDialog& m_me;
                    game::map::Drawing::Type m_type;
                };

                ui::widgets::Button& btn = del.addNew(new ui::widgets::Button(keyLabel, key, m_root));
                container.add(btn);
                container.add(del.addNew(new ui::widgets::StaticText(label, util::SkinColor::Static, "+", m_root.provider())));
                container.add(del.addNew(new ui::Spacer()));
                btn.sig_fire.addNewClosure(new Handler(*this, type));
            }

     private:
        client::dialogs::NewDrawingInfo& m_result;
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;

        ui::EventLoop m_loop;
        ui::widgets::StaticText m_tag;
    };
}

bool
client::dialogs::chooseNewDrawingParameters(NewDrawingInfo& result,
                                            ui::Root& root,
                                            util::RequestSender<game::Session> gameSender,
                                            afl::string::Translator& tx)
{
    return NewDrawingDialog(result, root, gameSender, tx).run();
}
