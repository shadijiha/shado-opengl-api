#pragma once

#ifndef APPLICATION_EVENT_H
#define APPLICATION_EVENT_H

#include <filesystem>

#include "Event.h"
#include <sstream>

namespace Shado {
    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_Width(width), m_Height(height) {
        }

        inline unsigned int getWidth() const { return m_Width; }
        inline unsigned int getHeight() const { return m_Height; }

        std::string toString() const override {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        unsigned int m_Width, m_Height;
    };

    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class WindowDropEvent : public Event {
    public:
        WindowDropEvent(const std::vector<std::filesystem::path>& paths)
            : m_Paths(paths) {
        }

        WindowDropEvent(std::vector<std::filesystem::path>&& paths)
            : m_Paths(std::move(paths)) {
        }

        const std::vector<std::filesystem::path>& GetPaths() const { return m_Paths; }
        EVENT_CLASS_TYPE(WindowDrop)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        std::vector<std::filesystem::path> m_Paths;
    };

    class AppTickEvent : public Event {
    public:
        AppTickEvent() = default;

        EVENT_CLASS_TYPE(AppTick)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class AppUpdateEvent : public Event {
    public:
        AppUpdateEvent() = default;

        EVENT_CLASS_TYPE(AppUpdate)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class AppRenderEvent : public Event {
    public:
        AppRenderEvent() = default;

        EVENT_CLASS_TYPE(AppRender)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };
}


#endif // !APPLICATION_EVENT_H
