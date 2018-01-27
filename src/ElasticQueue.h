#pragma once
#include <queue>
#include <future>

namespace Connectors {

template <typename T>
class ElasticQueue {
    std::mutex m_mutex;
    std::queue<T> m_ready;
    std::queue<std::promise<T>> m_promised;

public:
    // I'm taking it all
    void push(T value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_promised.size()) {
            m_promised.front().set_value(std::move(value));
            m_promised.pop();
            return;
        }

        m_ready.push(std::move(value));
    }

    // don't worry, I'll never run out of and never keep you waiting
    std::future<T> pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_ready.size()) {
            std::promise<T> promise;
            promise.set_value(std::move(m_ready.front()));
            m_ready.pop();
            return promise.get_future();
        }

        std::promise<T> p;
        auto future = p.get_future();
        m_promised.push(std::move(p));
        return future;
    }
};






}
