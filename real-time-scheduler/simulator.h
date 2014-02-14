//
//  simulator.h
//  real-time-scheduler
//
//  Copyright (c) 2013-2014 Xiaohan Kang. All rights reserved.
//

#ifndef REAL_TIME_SCHEDULER_SIMULATOR_H_
#define REAL_TIME_SCHEDULER_SIMULATOR_H_

#include <random>
#include <string>
#include "./common.h"
#include "./real_time_queueing.h"

class Simulator {
public:  // NOLINT (to suppress cpplint.py style check errors)
    Simulator();  // zero-initialization
    bool init(const std::string &config_filename,
              const std::string &network_filename, std::mt19937 &rng);
    void save_config();  // save network config for all queueing systems
    void arrive(std::mt19937 &rng);  // NOLINT
    void depart(std::mt19937 &rng);  // NOLINT
    void update_stability_counter();
    void save_deficits();
    void clock_tick();
    void progress_bar();
    int num_iterations() const {return num_iterations_;}
    void save_stability_ratios(const std::string &stability_filename);
private:  // NOLINT
    NetworkType network_type_;
    int network_size_;
    int interference_radius_;  // of the conflict graph
    BooleanMatrix maximal_schedule_matrix_;
    ArrivalDistribution arrival_dist_;
    IntegerVector min_packet_;  // only for uniform and Bernoulli arrival
    IntegerVector max_packet_;
    Ratios binom_param_;  // only for binomial arrival
    Ratios bern_param_;  // only for Bernoulli arrival
    IntegerVector min_delay_bound_;
    IntegerVector max_delay_bound_;
    Ratios base_qos_;
    int num_iterations_;
    BooleanVector policy_indicator_;
    IntegerVector bandwidth_;
    Ratios qos_ratio_;
    QueueingSystem3D queueing_system_;
        // per-policy, per-bandwidth, per-qos-ratio systems
    int system_clock_;
    int rng_seed_;
    bool output_throughput_;
};

#endif  // REAL_TIME_SCHEDULER_SIMULATOR_H_
