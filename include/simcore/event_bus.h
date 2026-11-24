#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace simcore {

namespace detail {

/// @brief One registered listener: its callback plus a liveness flag.
///
/// Subscription handles set @c alive to false to detach a listener; the bus
/// skips dead listeners while delivering and reaps them on the next dispatch().
struct Subscriber {
    /// @brief Callback invoked with a type-erased pointer to the published event.
    std::function<void(const void*)> handler{};

    /// @brief false once the owning Subscription detaches this listener.
    bool alive{true};
};

}  // namespace detail

/// @brief Handle that detaches one event-bus listener.
///
/// A subscription is move-only. Unlike a scope guard, destroying a Subscription
/// does @em not detach its listener; the listener keeps receiving events until
/// unsubscribe() is called. This lets callers register a permanent listener with
/// a single statement and ignore the returned handle.
///
/// @code
/// registry.events().subscribe<Hit>([](const Hit& hit) { use(hit); });
/// auto sub = registry.events().subscribe<Hit>([](const Hit& hit) { log(hit); });
/// sub.unsubscribe();  // the second listener stops receiving events
/// @endcode
class Subscription {
public:
    /// @brief Constructs a handle that owns no listener.
    Subscription() noexcept = default;

    /// @brief Copying is disabled; a handle owns one listener identity.
    Subscription(const Subscription&) = delete;

    /// @brief Copy assignment is disabled; a handle owns one listener identity.
    Subscription& operator=(const Subscription&) = delete;

    /// @brief Transfers the listener identity.
    /// @param other Subscription being moved from.
    Subscription(Subscription&& other) noexcept = default;

    /// @brief Transfers the listener identity.
    /// @param other Subscription being moved from.
    /// @return Reference to this subscription.
    Subscription& operator=(Subscription&& other) noexcept = default;

    /// @brief Destroys the handle without detaching the listener.
    ~Subscription() = default;

    /// @brief Detaches the listener; calling it again has no effect.
    void unsubscribe() noexcept {
        if (subscriber_ != nullptr) {
            subscriber_->alive = false;
            subscriber_.reset();
        }
    }

    /// @brief Reports whether the listener is still attached.
    /// @return true until unsubscribe() has been called.
    [[nodiscard]] bool active() const noexcept { return subscriber_ != nullptr; }

private:
    friend class EventBus;

    /// @brief Wraps a listener that was just registered on the bus.
    /// @param subscriber Listener shared with the bus.
    explicit Subscription(std::shared_ptr<detail::Subscriber> subscriber) noexcept
        : subscriber_(std::move(subscriber)) {}

    std::shared_ptr<detail::Subscriber> subscriber_{};
};

/// @brief Type-safe publish/subscribe bus with deferred delivery.
///
/// publish() only enqueues an event; listeners run later, when dispatch() is
/// called. A Stepper calls dispatch() once at the end of every step, so every
/// event published during an update pass is delivered together, in publish
/// order -- the same deferred model as despawn().
///
/// @note The bus is not thread-safe; use it from the same thread as its Registry.
///
/// @code
/// struct Hit { int damage; };
///
/// simcore::Registry registry;
/// registry.events().subscribe<Hit>([](const Hit& hit) { apply(hit.damage); });
/// registry.events().publish(Hit{7});
/// // apply(7) runs on the next Stepper::step() or on a manual dispatch().
/// @endcode
class EventBus {
public:
    /// @brief Constructs an empty bus.
    EventBus() = default;

    /// @brief Copying is disabled; a bus owns listeners and queued events.
    EventBus(const EventBus&) = delete;

    /// @brief Copy assignment is disabled; a bus owns listeners and queued events.
    EventBus& operator=(const EventBus&) = delete;

    /// @brief Moving is disabled; subscriptions point into the bus.
    EventBus(EventBus&&) = delete;

    /// @brief Move assignment is disabled; subscriptions point into the bus.
    EventBus& operator=(EventBus&&) = delete;

    /// @brief Discards every listener and queued event.
    ~EventBus() = default;

    /// @brief Registers a listener for events of type T.
    /// @tparam T Event type; published by value.
    /// @tparam Listener Callable type invoked as `void(const T&)`.
    /// @param listener Callback run for each delivered T, in registration order.
    /// @return Handle that detaches the listener when unsubscribe() is called.
    template <class T, class Listener>
    Subscription subscribe(Listener listener) {
        static_assert(std::is_invocable_v<Listener&, const T&>,
                      "simcore::EventBus::subscribe<T>: the listener must be callable as "
                      "'void(const T&)'.");
        auto subscriber = std::make_shared<detail::Subscriber>();
        subscriber->handler =
            [callback = std::move(listener)](const void* payload) {
                callback(*static_cast<const T*>(payload));
            };
        auto& listeners = subscribers_[std::type_index(typeid(T))];
        listeners.push_back(std::move(subscriber));
        return Subscription(listeners.back());
    }

    /// @brief Enqueues an event for delivery on the next dispatch().
    /// @tparam T Event type; stored by value inside the bus.
    /// @param event Event to enqueue; moved into the queue.
    template <class T>
    void publish(T event) {
        auto payload = std::make_shared<T>(std::move(event));
        pending_.push_back(PendingEvent{std::move(payload),
                                        [](EventBus& bus, const void* data) {
                                            bus.deliver<T>(*static_cast<const T*>(data));
                                        }});
    }

    /// @brief Delivers every currently queued event, in publish order.
    ///
    /// Each event is handed to the listeners registered for its exact type at
    /// the moment of delivery. Listeners added or detached during a dispatch
    /// take effect on the next dispatch, and events published by a listener are
    /// queued for the next dispatch as well. A nested dispatch() call (from
    /// inside a listener) is ignored so the current pass finishes first.
    void dispatch() {
        if (dispatching_) {
            return;
        }
        dispatching_ = true;
        const std::size_t count = pending_.size();
        std::size_t index = 0;
        try {
            for (; index < count; ++index) {
                // Read the delivery routine and payload out of the queue before
                // invoking it: a listener may publish(), which can reallocate
                // pending_ and invalidate any reference into it.
                const PendingEvent& pending = pending_[index];
                void (*const deliver_fn)(EventBus&, const void*) = pending.deliver;
                const void* const payload = pending.payload.get();
                deliver_fn(*this, payload);
            }
        } catch (...) {
            pending_.erase(pending_.begin(), pending_.begin() + static_cast<std::ptrdiff_t>(index));
            dispatching_ = false;
            reap();
            throw;
        }
        pending_.erase(pending_.begin(), pending_.begin() + static_cast<std::ptrdiff_t>(count));
        dispatching_ = false;
        reap();
    }

private:
    /// @brief One queued event: its storage and its delivery routine.
    struct PendingEvent {
        /// @brief Type-erased, owned copy of the queued event.
        std::shared_ptr<void> payload{};

        /// @brief Delivers the payload to the listeners registered for its type.
        void (*deliver)(EventBus&, const void*) = nullptr;
    };

    /// @brief Invokes, in order, the live listeners registered for type T.
    /// @tparam T Concrete event type being delivered.
    /// @param event Event payload passed to every matching listener.
    template <class T>
    void deliver(const T& event) {
        const auto found = subscribers_.find(std::type_index(typeid(T)));
        if (found == subscribers_.end()) {
            return;
        }
        // Copy the listener list so that subscribing during delivery neither
        // invalidates the traversal nor retroactively receives this event.
        const std::vector<std::shared_ptr<detail::Subscriber>> listeners = found->second;
        for (const auto& listener : listeners) {
            if (listener->alive) {
                listener->handler(&event);
            }
        }
    }

    /// @brief Removes listeners whose Subscription has detached them.
    void reap() {
        for (auto& entry : subscribers_) {
            auto& listeners = entry.second;
            listeners.erase(
                std::remove_if(listeners.begin(), listeners.end(),
                               [](const std::shared_ptr<detail::Subscriber>& listener) {
                                   return !listener->alive;
                               }),
                listeners.end());
        }
    }

    std::unordered_map<std::type_index, std::vector<std::shared_ptr<detail::Subscriber>>>
        subscribers_{};
    std::vector<PendingEvent> pending_{};
    bool dispatching_{false};
};

}  // namespace simcore
