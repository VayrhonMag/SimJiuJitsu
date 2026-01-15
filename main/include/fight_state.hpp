#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "technique.hpp"

struct FighterState {
    std::string my_name;
    std::string position;
    std::string opponent;
    bool can_think = false;
    bool is_attacker = false;
    int stamina = 100;
    int my_points = 0;
    int opponent_points = 0;
    double time_remaining = 300.0;
    bool is_winning = false;
    int exchanges = 0;
    double total_time = 0.0;  // 🔴 NUEVO: Tiempo total transcurrido
    
    std::vector<Technique> techniques;
    
    FighterState(const std::string& name) : my_name(name) {}
};

inline std::ostream &operator<<(std::ostream &os, const FighterState &s)
{
    os << "{name:" << s.my_name
       << ", position:" << s.position
       << ", opponent:" << s.opponent
       << ", can_think:" << s.can_think
       << ", is_attacker:" << s.is_attacker
       << ", stamina:" << s.stamina
       << ", my_points:" << s.my_points
       << ", opponent_points:" << s.opponent_points
       << ", time_remaining:" << s.time_remaining
       << ", is_winning:" << s.is_winning
       << ", exchanges:" << s.exchanges
       << ", total_time:" << s.total_time
       << ", techniques_count:" << s.techniques.size()
       << "}";
    return os;
}