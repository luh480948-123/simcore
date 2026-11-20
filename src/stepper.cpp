#include <simcore/stepper.h>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace simcore {

Stepper::Stepper(Registry& registry) noexcept : registry_(&registry) {}

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

double Stepper::elapsed() const noexcept { return elapsed_; }

}  // namespace simcore