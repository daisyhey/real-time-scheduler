//
//  real_time_queueing.cpp
//  real-time-scheduler
//
//  Copyright (c) 2013-2014 Xiaohan Kang. All rights reserved.
//

#include "./real_time_queueing.h"
#include <iostream>  // NOLINT
#include <algorithm>
#include <fstream>  // NOLINT
#include <string>
#include <numeric>
#include "./prettyprint.hpp"

Counters deficit_arrival(const Traffic &traffic, const Ratios &qos,
                         std::mt19937 &rng);
bool cmp_delay_bound(const Packet &a, const Packet &b);
bool cmp_deadline(const Packet &a, const Packet &b);

QueueingSystem::QueueingSystem(const BooleanMatrix &m,
                               Policy s, Ratios q, int b,
                               int d, const std::string &f, int n, double th) {
    maximal_schedule_matrix_ = m;
    scheduler_ = s;
    qos_ = q;
    bandwidth_ = b;
    system_clock_ = 0;
    max_delay_bound_ = d;
    output_filename_ = f;
    network_size_ = static_cast<int>(maximal_schedule_matrix_[0].size());
    per_link_deficit_.insert(per_link_deficit_.begin(), network_size(), 0);
    PacketSet empty_packet_set;
    per_link_queue_.insert(per_link_queue_.begin(), network_size(),
                           empty_packet_set);
    if ( (scheduler() == SDBF) || (scheduler() == SDBF_NAIVE) ) {
        intra_link_tie_breaker_ = DELAY_BOUND;
    } else {  // Default intra-link tie-breaker.
        intra_link_tie_breaker_ = DEADLINE;
    }
    lower_deficit_sum_ = 0;
    upper_deficit_sum_ = 0;
    num_iterations_ = n;
    per_link_cumulative_throughput_ = Counters(network_size(), 0);
    per_link_cumulative_arrival_ = Counters(network_size(), 0);
    threshold_ratio_ = th;
}

void QueueingSystem::arrive(const Traffic &traffic, std::mt19937 &rng) {
    Counters deficit_increase = deficit_arrival(traffic, qos(), rng);
    for (int i = 0; i < network_size(); ++i) {
        per_link_queue_[i].insert(per_link_queue_[i].begin(),
                                  traffic[i].begin(), traffic[i].end());
        per_link_deficit_[i] += deficit_increase[i];
        per_link_cumulative_arrival_[i] += traffic[i].size();
    }
}

Counters QueueingSystem::queue_lengths() {
    Counters queues;
    for (int i = 0; i < network_size(); ++i) {
        queues.push_back(static_cast<int>(per_link_queue_[i].size()));
    }
    return queues;
}

void QueueingSystem::depart(std::mt19937 &rng, LinkScheduleMap &schedule_map) {  // NOLINT
    for (int i = 0; i < network_size(); ++i) {  // Make heaps.
        if (intra_link_tie_breaker() == DELAY_BOUND) {
            make_heap(per_link_queue_[i].begin(), per_link_queue_[i].end(),
                      cmp_delay_bound);
        } else if (intra_link_tie_breaker() == DEADLINE) {
            make_heap(per_link_queue_[i].begin(), per_link_queue_[i].end(),
                      cmp_deadline);
        }  // For RANDOM no tie-breaker is specified, so no heap is made.
    }
    Counters per_link_deficit_updated = per_link_deficit_;
    for (int sub_time_slot = 0; sub_time_slot < bandwidth(); ++sub_time_slot) {
        BooleanVector scheduled_links(network_size(), false);
        if (scheduler() == LDF) {
            scheduled_links = ldf(per_link_queue_, per_link_deficit_,
                                  maximal_schedule_matrix_, rng);
        } else if (scheduler() == EDF) {
            scheduled_links = edf(per_link_queue_, per_link_deficit_,
                                  maximal_schedule_matrix_, system_clock(),
                                  max_delay_bound(), rng);
        } else if (scheduler() == SDBF) {
            scheduled_links = sdbf(per_link_queue_, per_link_deficit_,
                                   maximal_schedule_matrix_, max_delay_bound(),
                                   rng);
        } else if (scheduler() == EDF_NAIVE) {
            scheduled_links = edf_naive(per_link_queue_,
                                        maximal_schedule_matrix_,
                                        system_clock(), rng);
        } else if (scheduler() == SDBF_NAIVE) {
            scheduled_links = sdbf_naive(per_link_queue_,
                                         maximal_schedule_matrix_, rng);
        } else if (scheduler() == MAXIMAL) {
            scheduled_links = maximal(per_link_queue_,
                                      maximal_schedule_matrix_, rng);
        } else if (scheduler() == LDF_THRESHOLD) {
            scheduled_links = ldf_threshold(per_link_queue_,
                per_link_deficit_, maximal_schedule_matrix_, system_clock(),
                max_delay_bound(), threshold_ratio_, rng);
        } else if (scheduler() == LDF_VISION) {
            scheduled_links = ldf_vision(per_link_queue_, per_link_deficit_,
                                         maximal_schedule_matrix_, rng,
                                         schedule_map);
        } else if (scheduler() == MAX_DEFICIT) {
            scheduled_links = max_deficit(per_link_queue_, per_link_deficit_,
                                          maximal_schedule_matrix_, rng);
        } else {
            std::cerr << "Error: scheduler type not recognized!" << std::endl;
            exit(1);
            break;
        }
        for (int i = 0; i < network_size(); ++i) {
            if (scheduled_links[i]) {
                if (intra_link_tie_breaker() == DELAY_BOUND) {
                    pop_heap(per_link_queue_[i].begin(),
                             per_link_queue_[i].end(), cmp_delay_bound);
                } else {  // Default intra-link tie-breaker is DEADLINE.
                    pop_heap(per_link_queue_[i].begin(),
                             per_link_queue_[i].end(), cmp_deadline);
                }
                per_link_queue_[i].pop_back();
                if (per_link_deficit_updated[i] > 0) {
                    per_link_deficit_updated[i] -= 1;  // Updated deficit
                                                       // decreases.
                }
                ++per_link_cumulative_throughput_[i];  // Throughput counter.
            }
        }
    }
    per_link_deficit_ = per_link_deficit_updated;
}

void QueueingSystem::clock_tick() {
    ++system_clock_;
    for (int i = 0; i < network_size(); ++i) {
        for (int j = static_cast<int>(per_link_queue_[i].size())-1; j >= 0;
             --j) {
            if (per_link_queue_[i][j].deadline() < system_clock()) {
                per_link_queue_[i].erase(per_link_queue_[i].begin()+j);
            }
        }
    }
}

void QueueingSystem::output_deficits(const std::string &filename) {
    std::ofstream out(filename, std::ofstream::app);
    if (!out) {
        std::cerr << "Error: Could not open file " << filename << "!"
            << std::endl;
        exit(1);
    }
    for (auto i : per_link_deficit_) {
        out << i << " ";
    }
    out << std::endl;
    out.close();
}

int QueueingSystem::quarter_point() {
    return static_cast<int>(static_cast<double>(num_iterations_)/4);
}

int QueueingSystem::half_point() {
    return static_cast<int>(static_cast<double>(num_iterations_)/2);
}

void QueueingSystem::update_stability_counter() {
    if (system_clock() >= quarter_point()) {
        int current_deficit_sum = 0;
        for (int i = 0; i < network_size(); ++i) {
            current_deficit_sum += per_link_deficit_[i];
        }
        if (system_clock() >= half_point()) {
            upper_deficit_sum_ += current_deficit_sum;
        } else {
            lower_deficit_sum_ += current_deficit_sum;
        }
    }
}

double QueueingSystem::stability_ratio() {
    if ( (lower_deficit_sum() == 0) || (upper_deficit_sum() == 0) ) {
        return 0.0;
    } else {
        return static_cast<double>(upper_deficit_sum())
            /(num_iterations_-half_point())/lower_deficit_sum()
            *(half_point()-quarter_point());
    }
}

int64_t QueueingSystem::sum_cumulative_arrival() {
    int64_t sum = 0;
    for (int i = 0; i < network_size(); ++i) {
        sum += per_link_cumulative_arrival_[i];
    }
    return sum;
}

int64_t QueueingSystem::sum_cumulative_throughput() {
    int64_t sum = 0;
    for (int i = 0; i < network_size(); ++i) {
        sum += per_link_cumulative_throughput_[i];
    }
    return sum;
}

Counters deficit_arrival(const Traffic &traffic, const Ratios &qos,
                         std::mt19937 &rng) {
    int network_size = static_cast<int>(traffic.size());
    Counters deficit_increase(network_size, 0);
    for (int i = 0; i < network_size; ++i) {
        std::binomial_distribution<> d(static_cast<int>(traffic[i].size()),
                                       qos[i]);
            // TODO(Veggente): share the distributions to reduce object
            // generation time.
        deficit_increase[i] = d(rng);
    }
    return deficit_increase;
}

bool cmp_delay_bound(const Packet &a, const Packet &b) {
    return (a.delay_bound() > b.delay_bound());
        // a is considered less than b when a has a larger delay bound.
}

bool cmp_deadline(const Packet &a, const Packet &b) {
    return (a.deadline() > b.deadline());
        // a is considered less than b when a has a larger deadline.
}
