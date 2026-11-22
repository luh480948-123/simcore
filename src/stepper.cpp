#include <simcore/stepper.h>

#include <cstddef>
#include <stdexcept>

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
    registry_->begin_iteration();
    try {
        const std::size_t count = registry_->slots_.size();
        for (std::size_t index = 0; index < count; ++index) {
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
