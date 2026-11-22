#pragma once

#include <cstddef>

#include <simcore/registry.h>

namespace simcore {

/// @brief Drives a Registry forward in simulated time.
///
/// The stepper owns no entities. It calls update(dt) on every live entity in
/// registration order. It supports batch stepping via run(), single steps via
/// step(), and real-time fixed-step accumulation via advance().
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
