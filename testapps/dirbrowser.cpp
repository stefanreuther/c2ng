/**
  *  \file test_apps/dirbrowser.cpp
  */

#include <iostream>
#include "afl/io/filesystem.hpp"
#include "util/directorybrowser.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"

int main(int, char** /*argv*/)
{
    using util::DirectoryBrowser;
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    DirectoryBrowser b(fs);
    String_t cmd;
    size_t n;
    while (std::cout << b.getCurrentDirectory()->getTitle() << "> ", getline(std::cin, cmd)) {
        if (cmd == "") {
            // ok
        } else if (cmd == "pwd") {
            const std::vector<DirectoryBrowser::DirectoryPtr_t>& path = b.path();
            for (size_t i = 0, n = path.size(); i < n; ++i) {
                String_t title = path[i]->getTitle();
                if (title.empty()) {
                    title = path[i]->getDirectoryName();
                }
                std::cout << afl::string::Format("%3d. %s", i, title) << "\n";
            }
        } else if (cmd == "ls") {
            const std::vector<DirectoryBrowser::DirectoryItem>& dirs = b.directories();
            for (size_t i = 0, n = dirs.size(); i < n; ++i) {
                std::cout << afl::string::Format("%3d. %s <DIR>", i, dirs[i].title) << "\n";
            }
            const std::vector<DirectoryBrowser::DirectoryEntryPtr_t>& files = b.files();
            for (size_t i = 0, n = files.size(); i < n; ++i) {
                std::cout << afl::string::Format("%3d. %s <FILE>", i, files[i]->getTitle()) << "\n";
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
            std::cout << "Invalid command.\n";
        }
    }
}
