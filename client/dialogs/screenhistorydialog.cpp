/**
  *  \file client/dialogs/screenhistorydialog.cpp
  */

#include "client/dialogs/screenhistorydialog.hpp"
#include "afl/base/deleter.hpp"
#include "client/downlink.hpp"
#include "client/proxy/screenhistoryproxy.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/stringlistbox.hpp"

int32_t
client::dialogs::doScreenHistoryDialog(ui::Root& root,
                                       afl::string::Translator& tx,
                                       util::RequestSender<game::Session> gameSender,
                                       ScreenHistory& history,
                                       bool excludeCurrent)
{
    // ex client/history.cc:doScreenHistoryDialog, scrhist.pas:NBackOneScreen
    // Environment
    Downlink link(root, tx);
    client::proxy::ScreenHistoryProxy proxy(gameSender);

    // Validate references so we display only valid items
    afl::base::GrowableMemory<bool> mask;
    proxy.validateReferences(link, history.getAll(), mask);
    history.applyMask(mask);

    // Get list of references.
    // Most recent screen, if any, is last; exclude it if told so.
    afl::base::Memory<const ScreenHistory::Reference> refs(history.getAll());
    if (excludeCurrent && !refs.empty()) {
        refs.trim(refs.size() - 1);
    }

    // Get names
    afl::base::GrowableMemory<String_t> names;
    proxy.getReferenceNames(link, refs, names);

    // If we don't have anything to display, leave
    if (names.empty()) {
        return -1;
    }

    // Simple list box dialog
    // ex WScreenHistoryList
    afl::base::Deleter del;
    ui::widgets::StringListbox list(root.provider(), root.colorScheme());
    int32_t key = 0;
    for (size_t i = names.size(); i > 0; --i) {
        list.addItem(key++, *names.at(i-1));
    }
    bool ok = list.doStandardDialog(tx("Screen History"), String_t(), 0, root, tx);

    int32_t result;
    if (ok && list.getCurrentKey(result)) {
        return result + excludeCurrent;
    } else {
        return -1;
    }
}
