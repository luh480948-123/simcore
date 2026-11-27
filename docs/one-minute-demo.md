# simcore one-minute demo - self-contained bundle

Verified on 2026-11-27. This one file contains the environment, the complete
source of every required file, build and run commands, expected output, common
errors, and the result of a reconstruction performed outside this repository.

It is the minimal consumer story behind `docs/speech.md`: `spawn`, `view`,
`despawn`, `run`, and a `Thermometer` type declared in the demo file. It is not
the full `simcore_demo`, and it does not exercise the priority or batch APIs.

## 1. Verified environment

| Item | Verified value |
| --- | --- |
| Operating system | Windows 11, NT 10.0.26300.0 |
| Shell | Windows PowerShell 5.1.26100.9818 |
| Compiler | MinGW-w64 GCC 15.2.0 (MSYS2 UCRT64), `C:\msys64\ucrt64\bin\g++.exe` |
| CMake | 3.30.5; the bundle requires CMake 3.20 or newer |
| CMake generator | `MinGW Makefiles` |
| Make program | `mingw32-make` from MSYS2 UCRT64 |
| Doxygen | Not required. Version 1.9.8 was present but is not used by this bundle. |
| GoogleTest | Not required. The bundle contains no tests. |
| C++ standard | C++20, set by the imported CMake target |

PATH is a list of directories, not a command. Add these two directories to PATH,
with `C:\msys64\ucrt64\bin` first, for the current terminal window only:

- `C:\msys64\ucrt64\bin` supplies `g++.exe`, `mingw32-make.exe`, and the runtime
  DLLs (`libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`) needed to
  run the produced `.exe`.
- The folder that contains `cmake.exe`. The examples below use the standard
  installer location `C:\Program Files\CMake\bin`. If your CMake is elsewhere,
  find its folder first (see "If `where cmake` finds nothing" below), then use
  that folder in place of `C:\Program Files\CMake\bin`.

Do **not** type a directory path by itself. `cmd.exe` will try to run the
directory as a program and fail with
`'C:\msys64\ucrt64\bin' is not recognized as an internal or external command`
(or `不是内部或外部命令` on a Chinese Windows).

### cmd.exe: prepend the directories

Paste this line exactly. It changes PATH only in the current `cmd.exe` window:

```bat
set PATH=C:\msys64\ucrt64\bin;C:\Program Files\CMake\bin;%PATH%
```

### PowerShell: prepend the directories

Paste this line exactly. It changes PATH only in the current PowerShell window:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;C:\Program Files\CMake\bin;" + $env:Path
```

Both forms affect only the current window. A new window starts with the old
PATH; run the line again there. To make the change permanent, edit the user or
system environment variables through the Windows settings instead.

### Verify the PATH entry

In `cmd.exe`:

```bat
where g++
where cmake
```

In PowerShell, use `where.exe`, because bare `where` is an alias for
`Where-Object`:

```powershell
where.exe g++
where.exe cmake
```

Expected result: one path per command and exit code 0, for example:

```text
C:\msys64\ucrt64\bin\g++.exe
C:\Program Files\CMake\bin\cmake.exe
```

Your CMake line may show a different folder; the verification machine used a
portable CMake directory. What matters is that each command prints a path and
returns exit code 0.

If `where cmake` finds nothing, search the usual install roots:

```bat
where /r "C:\Program Files" cmake.exe
where /r "%LOCALAPPDATA%" cmake.exe
```

In PowerShell:

```powershell
where.exe /r "C:\Program Files" cmake.exe
where.exe /r "$env:LOCALAPPDATA" cmake.exe
```

The recursive search can take a while. If it prints a path ending in
`\CMake\bin\cmake.exe`, prepend that folder in the `set PATH` or `$env:Path`
line above. CMake bundled with Visual Studio lives under
`C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin`.

Then check the versions:

```powershell
cmake --version
g++ --version
mingw32-make --version
```

Only the Windows/MinGW configuration above was executed. The code is standard
C++20 and CMake, so a Linux or macOS build with
`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` should also work, but that path
was not measured here.

## 2. Directory and file list

```text
one-minute-bundle/
|-- CMakeLists.txt
|-- include/
|   `-- simcore/
|       |-- simcore.h
|       |-- entity.h
|       |-- entity_handle.h
|       |-- event_bus.h
|       |-- registry.h
|       |-- view.h
|       `-- stepper.h
|-- src/
|   `-- stepper.cpp
`-- examples/
    `-- one_minute_demo.cpp
```

| File | Role | Required? |
| --- | --- | --- |
| `CMakeLists.txt` | Minimal build: static library `simcore`, alias `simcore::simcore`, demo target | Required |
| `include/simcore/simcore.h` | Aggregate public header included by the demo | Required |
| `include/simcore/entity.h` | `Entity` concept, entity IDs, type-erased holder | Required |
| `include/simcore/entity_handle.h` | Typed `EntityHandle<T>` | Required |
| `include/simcore/event_bus.h` | Event queue included by `registry.h`; the demo does not call it | Required for compilation |
| `include/simcore/registry.h` | `spawn`, `view`, `despawn`, `entity_count` | Required |
| `include/simcore/view.h` | `View<T>` range and iterator | Required |
| `include/simcore/stepper.h` | `Stepper` declaration and template members | Required |
| `src/stepper.cpp` | Non-template `Stepper` implementation | Required |
| `examples/one_minute_demo.cpp` | The one-minute demo itself | Demo-only; required to see the output |

No tests, GoogleTest, Doxygen files, priority demo, batch demo, or runner script
are needed. The build commands below replace the repository's PowerShell runner.

## 3. Complete source files

Every block below is complete and can be copied without editing. The HTML
comments are extraction markers and do not appear in the rendered document.
### 3.1 CMakeLists.txt

<!-- file: CMakeLists.txt -->
```cmake
cmake_minimum_required(VERSION 3.20)

project(simcore_one_minute LANGUAGES CXX)

add_library(simcore STATIC src/stepper.cpp)
add_library(simcore::simcore ALIAS simcore)

target_include_directories(
    simcore
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_features(simcore PUBLIC cxx_std_20)
set_target_properties(simcore PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simcore PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(simcore_one_minute_demo examples/one_minute_demo.cpp)
target_link_libraries(simcore_one_minute_demo PRIVATE simcore::simcore)
set_target_properties(simcore_one_minute_demo PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simcore_one_minute_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()
```

### 3.2 include/simcore/simcore.h

<!-- file: include/simcore/simcore.h -->
```cpp
#pragma once

/// @file
/// @brief Aggregate header exposing the complete simcore public API.
///
/// Consumers only need:
/// @code
/// #include <simcore/simcore.h>
/// @endcode

#include <simcore/entity.h>
#include <simcore/entity_handle.h>
#include <simcore/event_bus.h>
#include <simcore/registry.h>
#include <simcore/stepper.h>
#include <simcore/view.h>
```

### 3.3 include/simcore/entity.h

<!-- file: include/simcore/entity.h -->
```cpp
#pragma once

#include <concepts>
#include <cstdint>
#include <typeinfo>
#include <utility>

namespace simcore {

/// @brief Stable identifier of an entity inside a Registry.
using EntityId = std::uint64_t;

/// @brief Constrains a type to be usable as a simulation entity.
///
/// A type models Entity when it exposes a member function callable as
/// `void update(double dt)`.
///
/// @tparam T Candidate entity type.
///
/// @code
/// struct Particle {
///     void update(double dt) { position += velocity * dt; }
/// };
/// static_assert(simcore::Entity<Particle>);
/// @endcode
template <class T>
concept Entity = requires(T& entity, double dt) {
    { entity.update(dt) } -> std::same_as<void>;
};

/// @brief Type-erased base class of every stored entity.
///
/// Registry stores entities through this interface so that entities of
/// different concrete types can share one container.
class EntityBase {
public:
    /// @brief Constructs the type-erased base object.
    EntityBase() = default;

    /// @brief Copying is disabled; entities are owned by a Registry.
    EntityBase(const EntityBase&) = delete;

    /// @brief Copy assignment is disabled; entities are owned by a Registry.
    EntityBase& operator=(const EntityBase&) = delete;

    /// @brief Moving is disabled so stored entity addresses stay stable.
    EntityBase(EntityBase&&) = delete;

    /// @brief Move assignment is disabled so stored entity addresses stay stable.
    EntityBase& operator=(EntityBase&&) = delete;

    /// @brief Destroys the stored entity.
    virtual ~EntityBase() = default;

    /// @brief Advances the entity by a time step.
    /// @param dt Step length in seconds.
    virtual void update(double dt) = 0;

    /// @brief Reports the runtime type of the concrete entity.
    /// @return Type information of the stored entity type.
    [[nodiscard]] virtual const std::type_info& type() const noexcept = 0;
};

/// @brief Adapter that stores one concrete entity behind EntityBase.
/// @tparam T Concrete entity type; must satisfy simcore::Entity.
template <class T>
class EntityHolder final : public EntityBase {
public:
    /// @brief Constructs the stored entity in place.
    /// @tparam Args Argument types forwarded to T's constructor.
    /// @param args Arguments forwarded to T's constructor.
    template <class... Args>
    explicit EntityHolder(Args&&... args) : value_(std::forward<Args>(args)...) {}

    /// @brief Forwards the time step to the stored entity's update().
    /// @param dt Step length in seconds.
    void update(double dt) override { value_.update(dt); }

    /// @brief Reports the stored entity's runtime type.
    /// @return Type information of T.
    [[nodiscard]] const std::type_info& type() const noexcept override { return typeid(T); }

    /// @brief Accesses the stored entity.
    /// @return Reference to the stored entity.
    [[nodiscard]] T& value() noexcept { return value_; }

private:
    T value_;
};

}  // namespace simcore
```

### 3.4 include/simcore/entity_handle.h

<!-- file: include/simcore/entity_handle.h -->
```cpp
#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <typeinfo>
#include <utility>

#include <simcore/entity.h>

namespace simcore {

class Registry;

/// @brief Type-safe non-owning reference to an entity owned by a Registry.
///
/// A handle remembers the entity's runtime type T. It resolves to nullptr once
/// the entity has been destroyed, so a stale handle never dereferences freed
/// memory.
///
/// @tparam T Concrete entity type referenced by the handle.
///
/// @code
/// simcore::Registry registry;
/// auto particle = registry.spawn<Particle>(1.0, 2.0);
/// if (particle.valid()) {
///     particle->update(0.016);
/// }
/// @endcode
template <class T>
class EntityHandle {
public:
    /// @brief Constructs a handle that refers to no entity.
    EntityHandle() noexcept = default;

    /// @brief Checks whether the referenced entity can still be resolved.
    /// @return true when the entity exists and has type T. An entity despawned during an
    ///         update pass or view traversal stays resolvable until that pass ends.
    [[nodiscard]] bool valid() const noexcept { return get() != nullptr; }

    /// @brief Accesses the referenced entity.
    /// @return Pointer to the entity, or nullptr when the handle is stale.
    [[nodiscard]] T* get() const noexcept {
        const auto base = entity_.lock();
        if (!base || base->type() != typeid(T)) {
            return nullptr;
        }
        return &static_cast<EntityHolder<T>*>(base.get())->value();
    }

    /// @brief Member access to the referenced entity.
    /// @return Pointer to the entity, or nullptr when the handle is stale.
    [[nodiscard]] T* operator->() const noexcept { return get(); }

    /// @brief Dereferences the referenced entity.
    /// @return Reference to the entity. The handle must be valid.
    [[nodiscard]] T& operator*() const noexcept { return *get(); }

    /// @brief Reports the identifier assigned at spawn time.
    /// @return Entity identifier.
    [[nodiscard]] EntityId id() const noexcept { return id_; }

    /// @brief Compares handles by owning registry and entity identifier.
    /// @return true when both handles refer to the same entity identity.
    /// @note Identity and hashing remain stable after the entity is despawned.
    friend bool operator==(const EntityHandle& lhs, const EntityHandle& rhs) noexcept {
        return lhs.owner_ == rhs.owner_ && lhs.id_ == rhs.id_;
    }

private:
    friend class Registry;
    friend struct std::hash<EntityHandle<T>>;

    /// @brief Builds a handle from an entity reference and an identifier.
    /// @param entity Weak reference to the type-erased entity.
    /// @param owner Registry that owns the entity.
    /// @param id Identifier assigned at spawn time.
    EntityHandle(std::weak_ptr<EntityBase> entity, const Registry* owner, EntityId id) noexcept
        : entity_(std::move(entity)), owner_(owner), id_(id) {}

    std::weak_ptr<EntityBase> entity_{};
    const Registry* owner_{nullptr};
    EntityId id_{0};
};

}  // namespace simcore

namespace std {

/// @brief Enables entity handles to be used as keys in unordered containers.
template <class T>
struct hash<simcore::EntityHandle<T>> {
    std::size_t operator()(const simcore::EntityHandle<T>& handle) const noexcept {
        const std::size_t owner = hash<const simcore::Registry*>{}(handle.owner_);
        const std::size_t id = hash<simcore::EntityId>{}(handle.id_);
        return owner ^ (id + 0x9e3779b9U + (owner << 6U) + (owner >> 2U));
    }
};

}  // namespace std
```

### 3.5 include/simcore/event_bus.h

<!-- file: include/simcore/event_bus.h -->
```cpp
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
```

### 3.6 include/simcore/registry.h

<!-- file: include/simcore/registry.h -->
```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#include <simcore/entity.h>
#include <simcore/entity_handle.h>
#include <simcore/event_bus.h>

namespace simcore {

template <class T>
class View;
template <class T>
class ViewIterator;
class Stepper;

/// @brief Owns a heterogeneous set of entities and their lifetimes.
///
/// Entities are stored behind EntityBase through type erasure, so any number of
/// unrelated types can coexist in one registry. Destruction requested with
/// despawn() is deferred until the current update pass or view traversal has
/// finished, which keeps in-flight iteration valid.
///
/// A registry also owns an EventBus (see events()) for typed publish/subscribe
/// communication between entities; a Stepper delivers queued events at the end
/// of each step.
///
/// @note A registry is not thread-safe; all access must happen on one thread.
///
/// @code
/// simcore::Registry registry;
/// auto particle = registry.spawn<Particle>(1.0, 2.0);
/// for (auto& p : registry.view<Particle>()) {
///     p.update(0.016);
/// }
/// registry.despawn(particle);
/// @endcode
class Registry {
public:
    /// @brief Constructs an empty registry.
    Registry() = default;

    /// @brief Copying is disabled; a registry owns its entities.
    Registry(const Registry&) = delete;

    /// @brief Copy assignment is disabled; a registry owns its entities.
    Registry& operator=(const Registry&) = delete;

    /// @brief Moving is disabled because handles keep a pointer to the owner.
    Registry(Registry&&) = delete;

    /// @brief Move assignment is disabled because handles keep a pointer to the owner.
    Registry& operator=(Registry&&) = delete;

    /// @brief Destroys all stored entities.
    ~Registry() = default;

    /// @brief Constructs an entity of type T and registers it.
    /// @tparam T Concrete entity type; must satisfy simcore::Entity.
    /// @tparam Args Constructor argument types.
    /// @param args Arguments forwarded to T's constructor.
    /// @return Typed handle to the new entity.
    ///
    /// @code
    /// struct Particle {
    ///     Particle(double x, double y) : x_(x), y_(y) {}
    ///     void update(double dt) { x_ += dt; }
    ///     double x_;
    ///     double y_;
    /// };
    /// auto handle = registry.spawn<Particle>(0.0, 0.0);
    /// @endcode
    template <class T, class... Args>
        requires Entity<T>
    EntityHandle<T> spawn(Args&&... args) {
        static_assert(std::is_constructible_v<T, Args...>,
                      "simcore::Registry::spawn<T>: T is not constructible from the arguments "
                      "forwarded to spawn().");
        if constexpr (std::is_constructible_v<T, Args...>) {
            auto holder = std::make_shared<EntityHolder<T>>(std::forward<Args>(args)...);
            const std::weak_ptr<EntityBase> reference = holder;
            const EntityId id = insert(std::move(holder));
            return EntityHandle<T>(reference, this, id);
        } else {
            return EntityHandle<T>{};
        }
    }

    /// @brief Rejects types that do not model simcore::Entity.
    /// @tparam T Candidate entity type.
    /// @tparam Args Ignored constructor argument types.
    /// @param args Ignored constructor arguments.
    /// @return Never returns; the static assertion fails during compilation.
    template <class T, class... Args>
        requires(!Entity<T>)
    EntityHandle<T> spawn([[maybe_unused]] Args&&... args) {
        static_assert(Entity<T>,
                      "simcore::Registry::spawn<T>: T does not satisfy the simcore::Entity "
                      "concept. An entity type must provide a member function callable as "
                      "'void update(double dt)'.");
        return EntityHandle<T>{};
    }

    /// @brief Constructs several entities of the same type with identical constructor arguments.
    ///
    /// The constructor arguments are copied for each entity. Use individual spawn() calls when
    /// each entity needs distinct or move-only construction data.
    ///
    /// @tparam T Concrete entity type; must satisfy simcore::Entity.
    /// @tparam Args Constructor argument types.
    /// @param count Number of entities to construct.
    /// @param args Constructor arguments copied into every entity.
    /// @return Typed handles for the newly created entities, in creation order.
    template <class T, class... Args>
        requires Entity<T>
    std::vector<EntityHandle<T>> spawn_many(std::size_t count, const Args&... args) {
        static_assert(std::is_constructible_v<T, const Args&...>,
                      "simcore::Registry::spawn_many<T>: T is not constructible from the "
                      "arguments forwarded to spawn_many().");
        std::vector<EntityHandle<T>> handles;
        handles.reserve(count);
        slots_.reserve(slots_.size() + count);
        order_.reserve(order_.size() + count);
        for (std::size_t index = 0; index < count; ++index) {
            handles.push_back(spawn<T>(args...));
        }
        return handles;
    }

    /// @brief Rejects batch creation of a type that does not model simcore::Entity.
    template <class T, class... Args>
        requires(!Entity<T>)
    std::vector<EntityHandle<T>> spawn_many([[maybe_unused]] std::size_t count,
                                             [[maybe_unused]] const Args&... args) {
        static_assert(Entity<T>,
                      "simcore::Registry::spawn_many<T>: T does not satisfy the "
                      "simcore::Entity concept. An entity type must provide a member function "
                      "callable as 'void update(double dt)'.");
        return {};
    }

    /// @brief Constructs an entity with an explicit update priority.
    ///
    /// Lower numbers run first; equal priorities run in registration order.
    /// Ordinary spawn() calls use priority zero. The priority affects Stepper
    /// updates; views continue to iterate in storage order.
    ///
    /// @tparam T Concrete entity type; must satisfy simcore::Entity.
    /// @tparam Args Constructor argument types.
    /// @param priority Update priority, including negative values.
    /// @param args Arguments forwarded to T's constructor.
    /// @return Typed handle to the new entity.
    template <class T, class... Args>
    EntityHandle<T> spawn_with_priority(int priority, Args&&... args) {
        auto handle = spawn<T>(std::forward<Args>(args)...);
        slots_[index_of(handle.id_)].priority = priority;
        return handle;
    }

    /// @brief Requests destruction of an entity.
    ///
    /// The entity stops being visible to view() and entity_count() immediately,
    /// while its storage is released after the current update pass or view
    /// traversal ends. Calling despawn() again on the same handle, or on a stale
    /// handle, has no effect (the operation is idempotent).
    ///
    /// @tparam T Concrete entity type referenced by the handle.
    /// @param handle Handle returned by spawn(); may be stale.
    template <class T>
    void despawn(EntityHandle<T> handle) {
        if (handle.owner_ != this) {
            return;
        }
        const auto entity = handle.entity_.lock();
        if (!entity || entity->type() != typeid(T)) {
            return;
        }
        const std::size_t index = index_of(handle.id_);
        if (index >= slots_.size()) {
            return;
        }
        Slot& slot = slots_[index];
        if (!slot.alive || slot.generation != generation_of(handle.id_) || slot.entity != entity) {
            return;
        }
        slot.alive = false;
        --entity_count_;
        pending_.push_back(index);
        flush_if_idle();
    }

    /// @brief Requests destruction of every live entity whose exact type is T.
    ///
    /// Like despawn(), removal is immediately visible to view() and entity_count(), while
    /// storage is released after the active update pass or view traversal ends.
    ///
    /// @tparam T Concrete entity type to remove.
    /// @return Number of entities marked for removal.
    template <class T>
        requires Entity<T>
    std::size_t despawn_all() {
        std::size_t removed = 0;
        for (std::size_t index = 0; index < slots_.size(); ++index) {
            Slot& slot = slots_[index];
            if (!slot_matches(index, typeid(T))) {
                continue;
            }
            slot.alive = false;
            --entity_count_;
            pending_.push_back(index);
            ++removed;
        }
        flush_if_idle();
        return removed;
    }

    /// @brief Requests destruction of every live entity.
    ///
    /// This uses the same deferred-destruction policy as despawn(), so it is safe during an
    /// active update pass or view traversal. It does not alter EventBus subscriptions or events.
    void clear() {
        for (std::size_t index = 0; index < slots_.size(); ++index) {
            Slot& slot = slots_[index];
            if (!slot.alive) {
                continue;
            }
            slot.alive = false;
            --entity_count_;
            pending_.push_back(index);
        }
        flush_if_idle();
    }

    /// @brief Preallocates storage for at least the requested number of entity slots.
    ///
    /// This changes capacity only; it does not create entities or change entity_count().
    /// @param entity_capacity Minimum total slot capacity to reserve.
    void reserve(std::size_t entity_capacity) {
        slots_.reserve(entity_capacity);
        order_.reserve(entity_capacity);
        free_slots_.reserve(entity_capacity);
        pending_.reserve(entity_capacity);
    }

    /// @brief Creates a view over all live entities of type T.
    ///
    /// Traversing the whole view is O(N) in the total number of entities. A view
    /// filters both by exact runtime type and by liveness, so entities despawned
    /// while the view is being traversed are skipped without invalidating it.
    ///
    /// @tparam T Concrete entity type to iterate.
    /// @return View that yields mutable references to T entities.
    ///
    /// @code
    /// for (auto& particle : registry.view<Particle>()) {
    ///     particle.update(0.016);
    /// }
    /// @endcode
    template <class T>
    [[nodiscard]] View<T> view() noexcept;

    /// @brief Counts the entities that are currently alive.
    /// @return Number of entities not yet despawned.
    [[nodiscard]] std::size_t entity_count() const noexcept { return entity_count_; }

    /// @brief Accesses the registry's event bus.
    ///
    /// The bus lets entities and user code publish typed events and subscribe to
    /// them. Queued events are delivered by Stepper::step() at the end of each
    /// step, or earlier by calling EventBus::dispatch() directly.
    ///
    /// @return Reference to the registry's event bus.
    ///
    /// @code
    /// registry.events().subscribe<Hit>([](const Hit& hit) { apply(hit); });
    /// registry.events().publish(Hit{7});
    /// @endcode
    [[nodiscard]] EventBus& events() noexcept { return events_; }

private:
    struct Slot {
        std::shared_ptr<EntityBase> entity{};
        std::uint32_t generation{1};
        bool alive{false};
        std::size_t order_pos{0};
        int priority{0};
        std::uint64_t registration_order{0};
    };

    friend class Stepper;
    template <class T>
    friend class View;
    template <class T>
    friend class ViewIterator;

    /// @brief Encodes a slot index and a generation counter into one identifier.
    /// @param index Slot index.
    /// @param generation Slot generation.
    /// @return Combined identifier.
    [[nodiscard]] static constexpr EntityId make_id(std::size_t index,
                                                    std::uint32_t generation) noexcept {
        return (static_cast<EntityId>(index) << 32U) | static_cast<EntityId>(generation);
    }

    /// @brief Extracts the slot index from an identifier.
    /// @param id Entity identifier.
    /// @return Slot index.
    [[nodiscard]] static constexpr std::size_t index_of(EntityId id) noexcept {
        return static_cast<std::size_t>(id >> 32U);
    }

    /// @brief Extracts the generation counter from an identifier.
    /// @param id Entity identifier.
    /// @return Slot generation.
    [[nodiscard]] static constexpr std::uint32_t generation_of(EntityId id) noexcept {
        return static_cast<std::uint32_t>(id);
    }

    /// @brief Stores an entity in a free slot or appends a new slot.
    ///
    /// The entity becomes the newest registration. Equal priorities use that
    /// registration order even when it recycled the slot of an older entity.
    /// @param entity Entity to store.
    /// @return Identifier assigned to the stored entity.
    EntityId insert(std::shared_ptr<EntityBase> entity) {
        std::size_t index = 0;
        if (free_slots_.empty()) {
            index = slots_.size();
            slots_.push_back(Slot{std::move(entity), 1U, true, 0U});
        } else {
            index = free_slots_.back();
            free_slots_.pop_back();
            Slot& recycled = slots_[index];
            recycled.entity = std::move(entity);
            recycled.alive = true;
        }
        Slot& slot = slots_[index];
        slot.priority = 0;
        slot.registration_order = next_registration_order_++;
        slot.order_pos = order_.size();
        order_.push_back(index);
        ++entity_count_;
        return make_id(index, slot.generation);
    }

    /// @brief Releases storage of entities that were despawned.
    void flush_deferred() noexcept {
        for (const std::size_t index : pending_) {
            Slot& slot = slots_[index];
            slot.entity.reset();
            ++slot.generation;
            if (slot.generation == 0U) {
                slot.generation = 1U;
            }
            // Drop the slot from the registration order in O(1) by moving the
            // last entry into the freed position.
            const std::size_t position = slot.order_pos;
            const std::size_t moved = order_.back();
            order_[position] = moved;
            slots_[moved].order_pos = position;
            order_.pop_back();
            free_slots_.push_back(index);
        }
        pending_.clear();
    }

    /// @brief Releases deferred entities when no traversal is in progress.
    void flush_if_idle() noexcept {
        if (iteration_depth_ == 0U && !pending_.empty()) {
            flush_deferred();
        }
    }

    /// @brief Marks the start of a view traversal or update pass.
    void begin_iteration() noexcept { ++iteration_depth_; }

    /// @brief Marks the end of a view traversal or update pass.
    void end_iteration() noexcept {
        --iteration_depth_;
        flush_if_idle();
    }

    /// @brief Reports how many slots the registry has allocated.
    /// @return Number of slots, including dead and recycled ones.
    [[nodiscard]] std::size_t slot_count() const noexcept { return slots_.size(); }

    /// @brief Checks whether a slot currently holds a live entity of the given type.
    /// @param index Slot index.
    /// @param type Runtime type to match.
    /// @return true when the slot is alive and stores an entity of the given type.
    [[nodiscard]] bool slot_matches(std::size_t index, const std::type_info& type) const noexcept {
        const Slot& slot = slots_[index];
        return slot.alive && slot.entity != nullptr && slot.entity->type() == type;
    }

    /// @brief Accesses the type-erased entity stored in a slot.
    /// @param index Slot index.
    /// @return Pointer to the stored entity.
    [[nodiscard]] EntityBase* entity_base_at(std::size_t index) const noexcept {
        return slots_[index].entity.get();
    }

    std::vector<Slot> slots_{};
    std::vector<std::size_t> order_{};
    std::vector<std::size_t> free_slots_{};
    std::vector<std::size_t> pending_{};
    EventBus events_{};
    std::size_t entity_count_{0};
    std::size_t iteration_depth_{0};
    std::uint64_t next_registration_order_{0};
};

}  // namespace simcore

// view.h defines View, ViewIterator and Registry::view(); it includes this header, so the
// include guard makes the cycle safe and keeps registry.h self-contained for its consumers.
#include <simcore/view.h>
```

### 3.7 include/simcore/view.h

<!-- file: include/simcore/view.h -->
```cpp
#pragma once

#include <cstddef>
#include <iterator>
#include <typeinfo>

#include <simcore/registry.h>

namespace simcore {

/// @brief Forward iterator over the live entities of one concrete type.
///
/// The iterator walks the registry's slot storage by index, so slot reallocation
/// caused by spawn() during traversal does not invalidate it. Entities that are
/// despawned while iterating are skipped.
///
/// @tparam T Concrete entity type produced by the iterator.
template <class T>
class ViewIterator {
public:
    /// @brief Iterator category (forward iterator).
    using iterator_category = std::forward_iterator_tag;

    /// @brief Type produced when dereferencing the iterator.
    using value_type = T;

    /// @brief Type of an iterator difference.
    using difference_type = std::ptrdiff_t;

    /// @brief Pointer type returned by operator->().
    using pointer = T*;

    /// @brief Reference type returned by operator*().
    using reference = T&;

    /// @brief Constructs an end-like iterator that belongs to no registry.
    ViewIterator() noexcept = default;

    /// @brief Accesses the current entity.
    /// @return Mutable reference to the current entity.
    [[nodiscard]] reference operator*() const noexcept {
        return static_cast<EntityHolder<T>*>(registry_->entity_base_at(index_))->value();
    }

    /// @brief Accesses the current entity.
    /// @return Pointer to the current entity.
    [[nodiscard]] pointer operator->() const noexcept {
        return &static_cast<EntityHolder<T>*>(registry_->entity_base_at(index_))->value();
    }

    /// @brief Advances to the next matching entity.
    /// @return Reference to this iterator.
    ViewIterator& operator++() noexcept {
        ++index_;
        skip_to_match();
        return *this;
    }

    /// @brief Advances to the next matching entity.
    /// @return Copy of the iterator before advancing.
    ViewIterator operator++(int) noexcept {
        ViewIterator copy = *this;
        ++index_;
        skip_to_match();
        return copy;
    }

    /// @brief Compares two iterators.
    /// @param lhs Left-hand iterator.
    /// @param rhs Right-hand iterator.
    /// @return true when both iterators refer to the same position in the same registry.
    [[nodiscard]] friend bool operator==(const ViewIterator& lhs,
                                         const ViewIterator& rhs) noexcept {
        return lhs.registry_ == rhs.registry_ && lhs.index_ == rhs.index_;
    }

private:
    friend class View<T>;
    friend class Registry;

    /// @brief Builds an iterator positioned at a slot index.
    /// @param registry Registry being traversed.
    /// @param index Starting slot index.
    ViewIterator(Registry* registry, std::size_t index) noexcept
        : registry_(registry), index_(index) {
        skip_to_match();
    }

    /// @brief Advances until a live entity of type T or the end is reached.
    void skip_to_match() noexcept {
        if (registry_ == nullptr) {
            return;
        }
        const std::size_t count = registry_->slot_count();
        while (index_ < count && !registry_->slot_matches(index_, typeid(T))) {
            ++index_;
        }
    }

    Registry* registry_{nullptr};
    std::size_t index_{0};
};

/// @brief Range over the live entities of one concrete type.
///
/// A view does not own entities; it only filters the registry. Holding a view
/// keeps deferred destruction pending, which guarantees that despawn() during
/// traversal never invalidates the traversal itself.
///
/// @tparam T Concrete entity type to iterate.
///
/// @code
/// for (auto& particle : registry.view<Particle>()) {
///     particle.update(0.016);
/// }
/// @endcode
template <class T>
class View {
public:
    /// @brief Iterator type produced by begin() and end().
    using Iterator = ViewIterator<T>;

    /// @brief Copying is disabled; a view guards a live traversal.
    View(const View&) = delete;

    /// @brief Copy assignment is disabled; a view guards a live traversal.
    View& operator=(const View&) = delete;

    /// @brief Transfers the view to another object.
    /// @param other View being moved from.
    View(View&& other) noexcept : registry_(other.registry_) { other.registry_ = nullptr; }

    /// @brief Move assignment is disabled; a view guards a live traversal.
    View& operator=(View&&) = delete;

    /// @brief Ends the traversal and releases deferred entities if this was the last one.
    ~View() {
        if (registry_ != nullptr) {
            registry_->end_iteration();
        }
    }

    /// @brief Returns an iterator to the first live entity of type T.
    /// @return Iterator to the first match, or end() when there is none.
    [[nodiscard]] Iterator begin() const noexcept { return Iterator(registry_, 0); }

    /// @brief Returns the past-the-end iterator.
    /// @return Iterator positioned after the last slot.
    [[nodiscard]] Iterator end() const noexcept {
        return Iterator(registry_, registry_ != nullptr ? registry_->slot_count() : 0);
    }

private:
    friend class Registry;

    /// @brief Starts a traversal on a registry.
    /// @param registry Registry to traverse.
    explicit View(Registry& registry) noexcept : registry_(&registry) {
        registry_->begin_iteration();
    }

    Registry* registry_{nullptr};
};

/// @brief Creates a view over all live entities of type T.
/// @tparam T Concrete entity type to iterate.
/// @return View over the matching entities.
template <class T>
[[nodiscard]] View<T> Registry::view() noexcept {
    static_assert(Entity<T>,
                  "simcore::Registry::view<T>: T does not satisfy the simcore::Entity concept.");
    return View<T>(*this);
}

}  // namespace simcore
```

### 3.8 include/simcore/stepper.h

<!-- file: include/simcore/stepper.h -->
```cpp
#pragma once

#include <cstddef>

#include <simcore/registry.h>

namespace simcore {

/// @brief Drives a Registry forward in simulated time.
///
/// The stepper owns no entities. It calls update(dt) on every live entity in
/// registration order. It supports batch stepping via run(), single steps via
/// step(), and real-time fixed-step accumulation via advance().
/// The stepper owns no entities. It calls update(dt) in ascending priority
/// order, with registration order breaking ties. Ordinary spawn() uses zero.
/// After each step it dispatches the registry's queued events (see
/// Registry::events()), once the update pass has finished.
///
/// @note A stepper is not thread-safe; drive a registry from one thread only.
///
/// @code
/// simcore::Registry registry;
/// simcore::Stepper stepper(registry, 0.016);
/// stepper.advance(0.040);  // executes 2 fixed steps, leaves 0.008 s
/// double alpha = stepper.interpolation_alpha();  // 0.5
/// @endcode
class Stepper {
public:
    /// @brief Binds the stepper to a registry without configuring a fixed step.
    /// @param registry Registry whose entities receive update calls.
    explicit Stepper(Registry& registry) noexcept;

    /// @brief Binds the stepper to a registry with a fixed step for advance().
    /// @param registry Registry whose entities receive update calls.
    /// @param fixed_step Fixed step length used by advance(), in seconds; must be positive.
    /// @throws std::invalid_argument if fixed_step is not positive.
    Stepper(Registry& registry, double fixed_step);

    /// @brief Advances the simulation by a duration in fixed steps.
    ///
    /// When duration is not an exact multiple of fixed_step, the final step
    /// receives the remaining time.
    ///
    /// @param duration Total simulated time to advance, in seconds.
    /// @param fixed_step Preferred step length, in seconds; must be positive.
    /// @throws std::invalid_argument if duration is negative or fixed_step is not positive.
    void run(double duration, double fixed_step);

    /// @brief Advances the simulation by exactly one step.
    ///
    /// The update order is captured at the start of the step. New entities first
    /// update on the next step. An entity despawned before its turn is skipped;
    /// destruction stays deferred until active updates and views finish.
    /// Building the order scans all allocated slots and sorts the live entities.
    /// @param dt Step length in seconds; must be non-negative.
    /// @throws std::invalid_argument if dt is negative.
    void step(double dt);

    /// @brief Accumulates frame time and executes whole fixed steps.
    ///
    /// Any remaining fraction of a step stays in the accumulator for subsequent
    /// advance() calls and can be queried via interpolation_alpha().
    ///
    /// @param frame_dt Elapsed real/frame time to accumulate, in seconds; must be non-negative.
    /// @return Number of fixed simulation steps executed during this call.
    /// @throws std::logic_error if fixed_step has not been configured.
    /// @throws std::invalid_argument if frame_dt is negative.
    std::size_t advance(double frame_dt);

    /// @brief Configures the fixed step length used by advance().
    /// @param fixed_step Fixed step length in seconds; must be positive.
    /// @throws std::invalid_argument if fixed_step is not positive.
    void set_fixed_step(double fixed_step);

    /// @brief Reports the configured fixed step length for advance().
    /// @return Fixed step length in seconds, or 0.0 if not configured.
    [[nodiscard]] double fixed_step() const noexcept;

    /// @brief Reports the unconsumed time currently stored in the accumulator.
    /// @return Remaining accumulated time in seconds.
    [[nodiscard]] double accumulator() const noexcept;

    /// @brief Reports the interpolation factor for rendering between steps.
    /// @return Ratio accumulator() / fixed_step(), or 0.0 if fixed_step is not configured.
    [[nodiscard]] double interpolation_alpha() const noexcept;

    /// @brief Reports the total simulated time advanced so far.
    /// @return Accumulated simulated time in seconds.
    [[nodiscard]] double elapsed() const noexcept;

private:
    Registry* registry_;
    double elapsed_{0.0};
    double fixed_step_{0.0};
    double accumulator_{0.0};
};

}  // namespace simcore
```

### 3.9 src/stepper.cpp

<!-- file: src/stepper.cpp -->
```cpp
#include <simcore/stepper.h>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace simcore {

Stepper::Stepper(Registry& registry) noexcept : registry_(&registry) {}

Stepper::Stepper(Registry& registry, double fixed_step) : registry_(&registry) {
    set_fixed_step(fixed_step);
}

void Stepper::run(double duration, double fixed_step) {
    if (!(duration >= 0.0)) {
        throw std::invalid_argument("simcore::Stepper::run: duration must be non-negative");
    }
    if (!(fixed_step > 0.0)) {
        throw std::invalid_argument("simcore::Stepper::run: fixed_step must be positive");
    }
    // Absorbs floating-point drift so that an exact multiple of fixed_step does
    // not produce a spurious near-zero final step.
    const double tolerance = fixed_step * 1e-9;
    double remaining = duration;
    while (remaining > fixed_step + tolerance) {
        step(fixed_step);
        remaining -= fixed_step;
    }
    if (remaining > tolerance) {
        step(remaining);
    }
}

void Stepper::step(double dt) {
    if (!(dt >= 0.0)) {
        throw std::invalid_argument("simcore::Stepper::step: dt must be non-negative");
    }

    // Snapshot live slot indices before invoking user code. A spawn during an
    // update joins the next step, even when it reuses an earlier free slot.
    std::vector<std::size_t> update_order;
    update_order.reserve(registry_->entity_count_);
    for (const std::size_t index : registry_->order_) {
        if (registry_->slots_[index].alive) {
            update_order.push_back(index);
        }
    }
    std::sort(update_order.begin(), update_order.end(), [this](std::size_t lhs, std::size_t rhs) {
        const auto& left = registry_->slots_[lhs];
        const auto& right = registry_->slots_[rhs];
        return std::tie(left.priority, left.registration_order) <
               std::tie(right.priority, right.registration_order);
    });

    registry_->begin_iteration();
    try {
        for (const std::size_t index : update_order) {
            const Registry::Slot& slot = registry_->slots_[index];
            if (!slot.alive) {
                continue;
            }
            slot.entity->update(dt);
        }
    } catch (...) {
        registry_->end_iteration();
        throw;
    }
    registry_->end_iteration();
    registry_->events().dispatch();
    elapsed_ += dt;
}

std::size_t Stepper::advance(double frame_dt) {
    if (!(fixed_step_ > 0.0)) {
        throw std::logic_error(
            "simcore::Stepper::advance: fixed_step must be configured before calling advance()");
    }
    if (!(frame_dt >= 0.0)) {
        throw std::invalid_argument("simcore::Stepper::advance: frame_dt must be non-negative");
    }

    accumulator_ += frame_dt;
    // Matches run()'s tolerance so exact multiples like 0.3 / 0.1 execute 3 steps cleanly.
    const double tolerance = fixed_step_ * 1e-9;
    std::size_t steps = 0;
    while (accumulator_ + tolerance >= fixed_step_) {
        step(fixed_step_);
        accumulator_ -= fixed_step_;
        if (accumulator_ < 0.0) {
            accumulator_ = 0.0;
        }
        ++steps;
    }
    return steps;
}

void Stepper::set_fixed_step(double fixed_step) {
    if (!(fixed_step > 0.0)) {
        throw std::invalid_argument("simcore::Stepper::set_fixed_step: fixed_step must be positive");
    }
    fixed_step_ = fixed_step;
}

double Stepper::fixed_step() const noexcept { return fixed_step_; }

double Stepper::accumulator() const noexcept { return accumulator_; }

double Stepper::interpolation_alpha() const noexcept {
    if (!(fixed_step_ > 0.0)) {
        return 0.0;
    }
    return accumulator_ / fixed_step_;
}

double Stepper::elapsed() const noexcept { return elapsed_; }

}  // namespace simcore
```

### 3.10 examples/one_minute_demo.cpp

<!-- file: examples/one_minute_demo.cpp -->
```cpp
// simcore one-minute demo.
//
// The shortest end-to-end consumer: one file, four core calls, and one new
// entity type (Thermometer) declared here rather than in the library.
//
// Differences from the other examples:
// - simcore_demo is the full particle/box scenario with deferred cleanup.
// - simcore_priority_demo shows update ordering only.
// - simcore_batch_demo shows spawn_many/despawn_all/clear only.
// This file is the minimal story used by the one-minute talk.

#include <simcore/simcore.h>

#include <cstddef>
#include <cstdio>

namespace {

/// @brief A consumer-defined entity; simcore does not know this type.
struct Thermometer {
    double celsius{20.0};
    void update(double dt) { celsius += 0.5 * dt; }
};

/// @brief A short-lived pulse used to show view traversal and despawn.
struct Pulse {
    double value{1.0};
    double ttl{2.0};
    void update(double dt) {
        value += dt;
        ttl -= dt;
    }
};

}  // namespace

int main() {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);

    std::puts("simcore one-minute demo");
    std::puts("-----------------------");

    // Thermometer is declared in this file, not in include/simcore/.
    auto thermometer = registry.spawn<Thermometer>();
    registry.spawn<Pulse>();
    auto second = registry.spawn<Pulse>();
    registry.spawn<Pulse>();
    std::printf("spawned: %zu entities\n", registry.entity_count());

    // 1.0 s at 0.3 s: three full steps plus a 0.1 s remainder.
    stepper.run(1.0, 0.3);

    std::size_t pulses_seen = 0;
    for (const auto& pulse : registry.view<Pulse>()) {
        (void)pulse;
        ++pulses_seen;
    }
    std::printf("view<Pulse> before despawn: %zu\n", pulses_seen);

    // Despawning inside the traversal is safe; destruction is deferred.
    for (auto& pulse : registry.view<Pulse>()) {
        if (&pulse == second.get()) {
            registry.despawn(second);
            break;
        }
    }
    std::puts("despawned one pulse during view traversal");

    // 0.5 s at 0.1 s: five more steps, nine in total.
    stepper.run(0.5, 0.1);

    std::size_t alive = 0;
    for (const auto& pulse : registry.view<Pulse>()) {
        (void)pulse;
        ++alive;
    }

    std::printf("elapsed: %.2f s in 9 fixed steps (4 + 5)\n", stepper.elapsed());
    std::printf("view<Pulse> alive: %zu\n", alive);
    std::printf("view<Thermometer> celsius: %.2f\n", thermometer->celsius);
    std::puts("Thermometer is declared in this file, not in include/simcore/");

    return 0;
}
```

## 4. Build and run

From the directory that contains `CMakeLists.txt`:

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --target simcore_one_minute_demo --config Release
.\build\simcore_one_minute_demo.exe
```

The build should end with `[100%] Built target simcore_one_minute_demo` and
produce no warnings. The executable prints:

```text
simcore one-minute demo
-----------------------
spawned: 4 entities
view<Pulse> before despawn: 3
despawned one pulse during view traversal
elapsed: 1.50 s in 9 fixed steps (4 + 5)
view<Pulse> alive: 2
view<Thermometer> celsius: 20.75
Thermometer is declared in this file, not in include/simcore/
```

The nine steps are `run(1.0, 0.3)` (three full 0.3 s steps plus a 0.1 s
remainder) and `run(0.5, 0.1)` (five steps). The thermometer starts at 20.0 and
rises by 0.5 per simulated second, so 1.5 s gives 20.75.

The reconstruction was run on 2026-11-27 in
`C:\Users\zhh\AppData\Local\Temp\one_minute_bundle_rebuild`, outside the
repository.

Method:

1. A parser read only this markdown file and extracted the 10 marked code
   blocks into the directory tree from section 2.
2. The parser wrote 10 files; the staged repository copies were read afterward
   only to compare content. All 10 matched, with 0 mismatches.
3. The three commands in section 4 were executed in that directory with CMake
   3.30.5 and GCC 15.2.0.

Measured on that clean directory:

| Step | Time | Result |
| --- | --- | --- |
| Configure | 10.07 s | `Configuring done`, `Generating done` |
| Build | 10.12 s | `[100%] Built target simcore_one_minute_demo`, 0 warnings/errors |
| Run | 0.63 s | output below |
| Total | 20.82 s | exit 0 |

Observed output, identical to section 4:

```text
simcore one-minute demo
-----------------------
spawned: 4 entities
view<Pulse> before despawn: 3
despawned one pulse during view traversal
elapsed: 1.50 s in 9 fixed steps (4 + 5)
view<Pulse> alive: 2
view<Thermometer> celsius: 20.75
Thermometer is declared in this file, not in include/simcore/
```

This is the self-containment check: the build used only files reconstructed
from this document, not the repository working tree.

## 6. Common errors and fixes

| Symptom | Cause | Fix |
| --- | --- | --- |
| `cmake` is not recognized | CMake is not on PATH | Add the folder that contains `cmake.exe` to PATH as in section 1, then re-run `where cmake` |
| `'C:\msys64\ucrt64\bin' is not recognized as an internal or external command` (or `不是内部或外部命令`) | The directory path was typed as a command | Do not run the directory itself. Prepend it with `set PATH=...` in cmd or `$env:Path = ...` in PowerShell, as in section 1 |
| `The syntax of the command is incorrect` (or `命令语法不正确`) | An angle-bracket placeholder was pasted literally instead of being replaced | Replace the whole placeholder with the real folder path, without `<` or `>`; use the exact line in section 1 and edit only the path |
| `where` is not recognized, or it behaves unexpectedly in PowerShell | PowerShell aliases bare `where` to `Where-Object` | Use `where.exe g++` and `where.exe cmake` in PowerShell |
| `CMake Error: No CMAKE_CXX_COMPILER could be found` | `g++` is not on PATH, or the wrong generator is selected | Add `C:\msys64\ucrt64\bin` to PATH and use `-G "MinGW Makefiles"` |
| `mingw32-make` is not recognized | MSYS2 UCRT64 tools are not on PATH | Add `C:\msys64\ucrt64\bin` before other entries, or install the MinGW-w64 make package |
| The `.exe` exits with a missing `libstdc++-6.dll` / `libgcc_s_seh-1.dll` | The runtime directory is not on PATH | Put `C:\msys64\ucrt64\bin` on PATH before running the executable |
| `simcore/simcore.h: No such file or directory` | The demo was compiled without the imported target | Link `simcore::simcore`; do not compile the demo as a standalone file |
| `T does not satisfy the simcore::Entity concept` | A consumer type lacks `void update(double)` | Give the type a member function callable as `void update(double dt)` |
| `undefined reference to simcore::Stepper::...` | `src/stepper.cpp` was not added to the library | Keep `src/stepper.cpp` in the `add_library(simcore STATIC ...)` list |
| CMake reports a C++ standard error | The consumer target did not link `simcore::simcore` | Link the imported target; it carries `cxx_std_20` |

Doxygen is not part of this bundle. A Doxygen failure cannot block this demo.

## 7. What is intentionally omitted

- The full `simcore_demo`, `simcore_priority_demo`, and `simcore_batch_demo`.
- GoogleTest and all 36 tests.
- `docs/Doxyfile` and generated HTML.
- The repository's PowerShell runner; the three commands in section 4 replace it.
