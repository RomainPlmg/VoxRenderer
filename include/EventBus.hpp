#pragma once

#include <functional>
#include <memory>
#include <typeindex>

#include "Logger.hpp"

class EventBus {
public:
    template<typename T>
    void subscribe(std::function<void(const T &)> callback);

    template<typename T>
    void publish(const T &event);

    template<typename T>
    void publishSync(const T &event);

    void pollEvents();

private:
    struct QueuedEvent {
        std::type_index typeIdx;
        std::shared_ptr<void> event;
    };

    std::unordered_map<std::type_index, std::vector<std::function<void(const void *)>>> m_callbacks;
    std::vector<QueuedEvent> m_queue;
};

template<typename T>
void EventBus::subscribe(std::function<void(const T &)> callback) {
    m_callbacks[typeid(T)].push_back([callback](const void *event) { callback(*static_cast<const T *>(event)); });
}

template<typename T>
void EventBus::publish(const T &event) {
    m_queue.push_back({typeid(T), std::make_shared<T>(event)});
}

template<typename T>
void EventBus::publishSync(const T &event) {
    if (!m_callbacks.contains(typeid(T))) {
        LOG_WARN("Event {} emitted but no subscribers, skip...", typeid(T).name());
        return;
    }
    for (auto &callback: m_callbacks[typeid(T)]) {
        callback(static_cast<const void *>(&event));
    }
}
