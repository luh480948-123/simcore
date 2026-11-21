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
