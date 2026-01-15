#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <string>
#include <ostream>
#include "technique.hpp"

// =======================
// FightAction
// =======================
struct FightAction
{
    std::string fighter_id; // "fighterA" o "fighterB"
    Technique tech;

    FightAction() = default;

    FightAction(const std::string &f, const Technique &t)
        : fighter_id(f), tech(t) {}
};

inline std::ostream &operator<<(std::ostream &os, const FightAction &a)
{
    os << "[FightAction fighter=" << a.fighter_id
       << ", tech=" << a.tech.name
       << ", energy_cost=" << a.tech.energy_cost
       << ", energy_gain=" << a.tech.energy_gain
       << "]";
    return os;
}

// =======================
// FightUpdate
// =======================
struct FightUpdate
{
    std::string actor;
    std::string my_pos;
    std::string opp_pos;
    std::string global_state;
    bool success = false;
    bool finished = false;
    int my_points = 0;     // 🔴 NUEVO: Puntos del luchador
    int opp_points = 0;    // 🔴 NUEVO: Puntos del oponente
    double time_remaining = 0.0; // 🔴 NUEVO: Tiempo restante

    FightUpdate() = default;

    FightUpdate(const std::string &act,
                const std::string &my,
                const std::string &opp,
                bool succ,
                bool fin)
        : actor(act),
          my_pos(my),
          opp_pos(opp),
          success(succ),
          finished(fin) {}
};

inline std::ostream &operator<<(std::ostream &os, const FightUpdate &u)
{
    os << "[FightUpdate actor=" << u.actor
       << ", my_pos=" << u.my_pos
       << ", opp_pos=" << u.opp_pos
       << ", success=" << u.success
       << ", finished=" << u.finished
       << ", my_points=" << u.my_points
       << ", opp_points=" << u.opp_points
       << ", time_remaining=" << u.time_remaining
       << "]";
    return os;
}
#endif