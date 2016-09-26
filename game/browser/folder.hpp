/**
  *  \file game/browser/folder.hpp
  */
#ifndef C2NG_GAME_BROWSER_FOLDER_HPP
#define C2NG_GAME_BROWSER_FOLDER_HPP

#include "afl/base/deletable.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "afl/base/ptr.hpp"
#include "game/root.hpp"
#include "util/rich/text.hpp"

namespace game { namespace browser {

    class Folder : public afl::base::Deletable {
     public:
        enum Kind {
            kRoot,
            kFolder,
            kAccount,
            kLocal,
            kGame,
            kFavorite,
            kFavoriteList
        };

        virtual void loadContent(afl::container::PtrVector<Folder>& result) = 0;

        virtual afl::base::Ptr<Root> loadGameRoot() = 0;

        virtual String_t getName() const = 0;

        virtual util::rich::Text getDescription() const = 0;

        virtual bool isSame(const Folder& other) const = 0;

        virtual bool canEnter() const = 0;

        virtual Kind getKind() const = 0;
    };

} }

#endif
