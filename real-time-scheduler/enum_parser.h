//
//  enum_parser.h
//  real-time-scheduler
//
//  Copyright (c) 2013-2014 Xiaohan Kang. All rights reserved.
//

#ifndef REAL_TIME_SCHEDULER_ENUM_PARSER_H_
#define REAL_TIME_SCHEDULER_ENUM_PARSER_H_

#include <string>
#include <iostream>  // NOLINT
#include <map>
#include "./common.h"

template <typename ENUM>
class EnumParser {
public:  // NOLINT
    EnumParser() {}
    std::string enum_to_string(ENUM e);
    ENUM string_to_enum(const std::string &s);
private:  // NOLINT
    std::map<std::string, ENUM> string_to_enum_;
    std::map<ENUM, std::string> enum_to_string_;
};

template <typename ENUM>
std::string EnumParser<ENUM>::enum_to_string(ENUM e) {
    typename std::map<ENUM, std::string>::const_iterator it =
        enum_to_string_.find(e);
    if (it == enum_to_string_.end()) {
        std::cerr << "Error parsing enum!" << std::endl;
        exit(1);
    }
    return it->second;
}

template <typename ENUM>
ENUM EnumParser<ENUM>::string_to_enum(const std::string &s) {
    typename std::map<std::string, ENUM>::const_iterator it =
        string_to_enum_.find(s);
    if (it == string_to_enum_.end()) {
        std::cerr << "Error parsing enum!" << std::endl;
        exit(1);
    }
    return it->second;
}

template<>
EnumParser<Policy>::EnumParser() {
    enum_to_string_[LDF] = "ldf";
    enum_to_string_[EDF] = "edf";
    enum_to_string_[SDBF] = "sdbf";
    enum_to_string_[EDF_NAIVE] = "edf-naive";
    enum_to_string_[SDBF_NAIVE] = "sdbf-naive";
    enum_to_string_[MAXIMAL] = "maximal";
    enum_to_string_[LDF_THRESHOLD] = "ldf-threshold";
    enum_to_string_[LDF_VISION] = "ldf-vision";
    enum_to_string_[MAX_DEFICIT] = "max-deficit";
    string_to_enum_["ldf"] = LDF;
    string_to_enum_["edf"] = EDF;
    string_to_enum_["sdbf"] = SDBF;
    string_to_enum_["edf-naive"] = EDF_NAIVE;
    string_to_enum_["sdbf-naive"] = SDBF_NAIVE;
    string_to_enum_["maximal"] = MAXIMAL;
    string_to_enum_["ldf-threshold"] = LDF_THRESHOLD;
    string_to_enum_["ldf-vision"] = LDF_VISION;
    string_to_enum_["max-deficit"] = MAX_DEFICIT;
}

template<>
EnumParser<NetworkType>::EnumParser() {
    enum_to_string_[COLLOCATED] = "collocated";
    enum_to_string_[LINE] = "line";
    enum_to_string_[CYCLE] = "cycle";
    enum_to_string_[UNIT_DISK] = "unit-disk";
    string_to_enum_["collocated"] = COLLOCATED;
    string_to_enum_["line"] = LINE;
    string_to_enum_["cycle"] = CYCLE;
    string_to_enum_["unit-disk"] = UNIT_DISK;
}

template<>
EnumParser<ArrivalDistribution>::EnumParser() {
    enum_to_string_[UNIFORM_PACKET] = "uniform";
    enum_to_string_[BINOMIAL_PACKET] = "binomial";
    enum_to_string_[BERNOULLI_PACKET] = "bernoulli";
    enum_to_string_[BERNOULLI_FINE_PACKET] = "bernoulli-fine";
    string_to_enum_["uniform"] = UNIFORM_PACKET;
    string_to_enum_["binomial"] = BINOMIAL_PACKET;
    string_to_enum_["bernoulli"] = BERNOULLI_PACKET;
    string_to_enum_["bernoulli-fine"] = BERNOULLI_FINE_PACKET;
}

#endif  // REAL_TIME_SCHEDULER_ENUM_PARSER_H_
