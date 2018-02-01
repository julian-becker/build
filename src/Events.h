#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

#include <memory>
#include <functional>
#include <future>
#include <chrono>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <utility>
#include <queue>
#include <type_traits>
#include <typeindex>
#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <utility> /// for std::pair
#include <stack>

namespace Events {

    class Canceler {
    public:
        struct Concept {
            virtual ~Concept() {}
            virtual void cancel() {}
        };

        Canceler(std::unique_ptr<Concept> impl)
            : m_impl{std::move(impl)}
        {}

        Canceler()
            : m_impl{std::make_unique<Concept>()}
        {}

        ~Canceler();

        void cancel();

    private:
        std::unique_ptr<Concept> m_impl;
    };

    template <typename T>
    class Listener {
        std::function<void(T)> m_impl;

    public:
        template <typename Callback>
        Listener(Callback&& fn)
            : m_impl{std::forward<Callback>(fn)}
        {}

        template <typename U>
        void notify(U&& value) {
            m_impl(std::forward<U>(value));
        }
    };

    template <typename T>
    class Event {
    public:
        struct Concept {
            virtual ~Concept() {}
            virtual Canceler subscribe(Listener<T>) = 0;
        };

        Event(std::unique_ptr<Concept> impl)
            : m_impl{std::move(impl)}
        {}

        ~Event() {
            m_impl.reset();
        }

        Canceler subscribe(Listener<T> listener) {
            return m_impl->subscribe(std::move(listener));
        }

    private:
        std::unique_ptr<Concept> m_impl;
    };


    template <typename T>
    Event<T> makeFireOnceEvent(T value) {
        struct EventImpl : Event<T>::Concept {
            T m_value;

            EventImpl(T val)
                : m_value{std::move(val)}
            {}

            Canceler subscribe(Listener<T> l) override {
                l.notify(value);
            }
        };

        return { std::make_unique<EventImpl>(std::move(value)) };
    }


    template <typename Rep, typename Period, typename T>
    Event<T> makeTimerEvent(std::chrono::duration<Rep,Period> period, T value) {
        struct CancelerImpl : Canceler::Concept {
            std::promise<void> m_promise;
            std::atomic<bool> m_done{false};
            std::thread m_thread;

            CancelerImpl(std::promise<void> prom, std::thread t)
                : m_promise{std::move(prom)}
                , m_thread{std::move(t)}
            {}

            ~CancelerImpl() {
                cancel();
            }

            void cancel() override {
                if(m_done)
                    return;

                if(m_thread.joinable()) {
                    m_promise.set_value();
                    m_thread.join();
                    m_done.store(true);
                }
            }
        };

        struct EventImpl : Event<T>::Concept {
            T m_value;
            std::chrono::duration<Rep,Period> m_period;

            EventImpl(std::chrono::duration<Rep,Period> period, T value)
                : m_value{std::move(value)}
                , m_period{std::move(period)}
            {}

            Canceler subscribe(Listener<T> listener) override {
                // attention: inefficient!! Rework this
                std::promise<void> p;
                std::thread thread([
                    value=m_value,
                    listener=std::move(listener),
                    done=p.get_future(),
                    period=m_period
                ]() mutable {
                    while(std::future_status::timeout == done.wait_for(period)) {
                        listener.notify(value);
                    }
                });

                return { std::make_unique<CancelerImpl>(std::move(p), std::move(thread)) };
            }
        };

        return { std::make_unique<EventImpl>(std::move(period), std::move(value)) };
    }

}

#endif /* end of include guard: EVENTS_H_INCLUDED */
