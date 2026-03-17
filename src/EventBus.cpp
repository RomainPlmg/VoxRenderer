#include "EventBus.hpp"

void EventBus::pollEvents() {
    for (auto &[typeIdx, event]: m_queue) {
        for (auto &callback: m_callbacks[typeIdx]) {
            callback(event.get());
        }
    }
    m_queue.clear();
}
