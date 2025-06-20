/**
  *  \file util/directorybrowserapplet.cpp
  *  \brief Class util::DirectoryBrowserApplet
  */

#include "util/directorybrowserapplet.hpp"

#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "util/directorybrowser.hpp"

using afl::base::Ref;
using afl::io::TextReader;
using afl::io::TextWriter;
using afl::string::Format;
using afl::sys::Environment;

int
util::DirectoryBrowserApplet::run(Application& app, afl::sys::Environment::CommandLine_t& /*cmdl*/)
{
    DirectoryBrowser b(app.fileSystem());
    Ref<TextReader> in = app.environment().attachTextReader(Environment::Input);
    TextWriter& out = app.standardOutput();
    String_t cmd;
    while (out.writeText(b.getCurrentDirectory()->getTitle() + "> "), out.flush(), in->readLine(cmd)) {
        size_t n;
        if (cmd == "") {
            // ok
        } else if (cmd == "pwd") {
            const std::vector<DirectoryBrowser::DirectoryPtr_t>& path = b.path();
            for (size_t i = 0, n = path.size(); i < n; ++i) {
                String_t title = path[i]->getTitle();
                if (title.empty()) {
                    title = path[i]->getDirectoryName();
                }
                out.writeLine(Format("%3d. %s", i, title));
            }
        } else if (cmd == "ls") {
            const std::vector<DirectoryBrowser::DirectoryItem>& dirs = b.directories();
            for (size_t i = 0, n = dirs.size(); i < n; ++i) {
                out.writeLine(Format("%3d. %s <DIR>", i, dirs[i].title));
            }
            const std::vector<DirectoryBrowser::DirectoryEntryPtr_t>& files = b.files();
            for (size_t i = 0, n = files.size(); i < n; ++i) {
                out.writeLine(Format("%3d. %s <FILE>", i, files[i]->getTitle()));
            }
        } else if (cmd.substr(0, 5) == "open ") {
            b.openDirectory(cmd.substr(5));
        } else if (cmd.substr(0, 3) == "cd " && afl::string::strToInteger(cmd.substr(3), n)) {
            b.openChild(n);
        } else if (cmd == "up") {
            b.openParent();
        } else if (cmd == "root") {
            b.openRoot();
        } else if (cmd == "load") {
            b.loadContent();
        } else if (cmd.substr(0, 4) == "add ") {
            b.addFileNamePattern(cmd.substr(4));
        } else if (cmd == "clear") {
            b.clearFileNamePatterns();
        } else if (cmd == "hide") {
            b.setAcceptHiddenEntries(false);
        } else if (cmd == "unhide") {
            b.setAcceptHiddenEntries(true);
        } else {
            // huh?
            out.writeLine("Invalid command.");
        }
    }

    return 0;
}
