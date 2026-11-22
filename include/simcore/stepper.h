#pragma once

#include <simcore/registry.h>

namespace simcore {

/// @brief Drives a Registry forward in simulated time.
///
/// The stepper owns no entities. It calls update(dt) in ascending priority
/// order, with registration order breaking ties. Ordinary spawn() uses zero.
/// After each step it dispatches the registry's queued events (see
/// Registry::events()), once the update pass has finished.
///
/// @note A stepper is not thread-safe; drive a registry from one thread only.
///
/// @code
/// simcore::Registry registry;
/// simcore::Stepper stepper(registry);
/// stepper.run(1.0, 0.016);
/// // stepper.elapsed() == 1.0
/// @endcode
class Stepper {
public:
    /// @brief Binds the stepper to a registry.
    /// @param registry Registry whose entities receive update calls.
    explicit Stepper(Registry& registry) noexcept;

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

    /// @brief Reports the total simulated time advanced so far.
    /// @return Accumulated simulated time in seconds.
    [[nodiscard]] double elapsed() const noexcept;

private:
    Registry* registry_;
    double elapsed_{0.0};
};

}  // namespace simcore