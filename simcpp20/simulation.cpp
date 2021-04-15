// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "simulation.hpp"

#include <memory>

namespace simcpp20 {
simcpp20::event simulation::any_of(std::vector<simcpp20::event> evs) {
  for (auto &ev : evs) {
    if (ev.processed()) {
      return timeout(0);
    }
  }

  auto any_of_ev = event();

  for (auto &ev : evs) {
    ev.add_callback([any_of_ev](auto) mutable { any_of_ev.trigger(); });
  }

  return any_of_ev;
}

simcpp20::event simulation::all_of(std::vector<simcpp20::event> evs) {
  int n = evs.size();

  for (auto &ev : evs) {
    if (ev.processed()) {
      --n;
    }
  }

  if (n == 0) {
    return timeout(0);
  }

  auto all_of_ev = event();
  auto n_ptr = std::make_shared<int>(n);

  for (auto &ev : evs) {
    ev.add_callback([all_of_ev, n_ptr](auto) mutable {
      --*n_ptr;
      if (*n_ptr == 0) {
        all_of_ev.trigger();
      }
    });
  }

  return all_of_ev;
}

void simulation::step() {
  auto scheduled_ev = scheduled_evs.top();
  scheduled_evs.pop();
  now_ = scheduled_ev.time();
  scheduled_ev.ev().process();
}

void simulation::run() {
  while (!empty()) {
    step();
  }
}

void simulation::run_until(simtime target) {
  if (target < now()) {
    return;
  }

  while (!empty() && scheduled_evs.top().time() < target) {
    step();
  }

  now_ = target;
}

simtime simulation::now() {
  return now_;
}

bool simulation::empty() {
  return scheduled_evs.empty();
}

void simulation::schedule(simtime delay, simcpp20::event ev) {
  scheduled_evs.emplace(now() + delay, next_id, ev);
  next_id++;
}
} // namespace simcpp20
