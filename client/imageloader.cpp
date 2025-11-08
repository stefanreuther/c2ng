/**
  *  \file client/imageloader.cpp
  *  \brief Class client::ImageLoader
  */

#include "client/imageloader.hpp"
#include "afl/base/signalconnection.hpp"
#include "client/widgets/busyindicator.hpp"

// Constructor.
client::ImageLoader::ImageLoader(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_loop(root),
      m_unloadedImages()
{ }

// Load an image.
void
client::ImageLoader::loadImage(const String_t& name)
{
    bool flag;
    m_root.provider().getImage(name, &flag);
    if (!flag) {
        m_unloadedImages.push_back(name);
    }
}

// Wait for pending images.
bool
client::ImageLoader::wait()
{
    if (!m_unloadedImages.empty()) {
        afl::base::SignalConnection conn(m_root.provider().sig_imageChange.add(this, &ImageLoader::onImageChange));
        client::widgets::BusyIndicator indicator(m_root, m_translator("Loading..."));
        indicator.setExtent(gfx::Rectangle(gfx::Point(), indicator.getLayoutInfo().getPreferredSize()));
        indicator.sig_quit.add(this, &ImageLoader::onQuit);
        m_root.moveWidgetToEdge(indicator, gfx::CenterAlign, gfx::BottomAlign, 10);
        m_root.add(indicator);
        int result = m_loop.run();
        indicator.replayEvents();
        return result == 0;
    } else {
        return true;
    }
}

void
client::ImageLoader::onImageChange()
{
    std::vector<String_t>::iterator it = m_unloadedImages.begin();
    while (it != m_unloadedImages.end()) {
        bool flag;
        m_root.provider().getImage(*it, &flag);
        if (flag) {
            it = m_unloadedImages.erase(it);
        } else {
            ++it;
        }
    }
    if (m_unloadedImages.empty()) {
        m_loop.stop(0);
    }
}

void
client::ImageLoader::onQuit()
{
    m_loop.stop(1);
}
