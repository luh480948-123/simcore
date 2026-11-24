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
