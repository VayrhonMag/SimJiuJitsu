#ifndef ARBITER_RANDOM_HPP
#define ARBITER_RANDOM_HPP

#include <iostream>
#include <cstdlib>
#include <ctime>
#include "cadmium/modeling/devs/atomic.hpp"
#include "messages.hpp"

using namespace cadmium;

struct ArbiterState {
    bool ready = false;
    std::vector<FightAction> actions;
    
    ArbiterState() {
        srand(time(nullptr));
    }
};

// 🔴 AGREGAR ESTA FUNCIÓN: Sobrecarga del operador << para ArbiterState
inline std::ostream& operator<<(std::ostream& os, const ArbiterState& s) {
    os << "[ArbiterState ready=" << s.ready
       << ", actions_count=" << s.actions.size()
       << "]";
    return os;
}

class arbiter_random : public Atomic<ArbiterState> {
public:
    Port<FightAction> inA;
    Port<FightAction> inB;
    Port<FightAction> outAction;
    
    arbiter_random(const std::string& id) : Atomic<ArbiterState>(id, ArbiterState()) {
        inA = addInPort<FightAction>("inA");
        inB = addInPort<FightAction>("inB");
        outAction = addOutPort<FightAction>("outAction");
    }
    
    void externalTransition(ArbiterState& s, double) const override {
        // Recolectar todas las acciones pendientes
        for(const auto& a : inA->getBag()) {
            s.actions.push_back(a);
        }
        for(const auto& a : inB->getBag()) {
            s.actions.push_back(a);
        }
        
        if(!s.actions.empty()) {
            s.ready = true;
        }
    }
    
    void internalTransition(ArbiterState& s) const override {
        s.actions.clear();
        s.ready = false;
    }
    
    void output(const ArbiterState& s) const override {
        if(!s.ready || s.actions.empty()) return;
        
        // 🔥 DECISIÓN 100% RANDOM
        int idx = rand() % s.actions.size(); // Si hay 1 o 2 acciones
        FightAction selected = s.actions[idx];
        
        outAction->addMessage(selected);
        
        std::cout << "\n[ARBITER_RANDOM] Decisión:" << std::endl;
        std::cout << "-------------------------" << std::endl;
        for(size_t i = 0; i < s.actions.size(); i++) {
            std::cout << "Opción " << i << ": " << s.actions[i].fighter_id 
                      << " - " << s.actions[i].tech.name << std::endl;
        }
        std::cout << "🎲 GANADOR RANDOM: " << selected.fighter_id << std::endl;
        std::cout << "-------------------------\n" << std::endl;
    }
    
    double timeAdvance(const ArbiterState& s) const override {
        return s.ready ? 0.0 : std::numeric_limits<double>::infinity();
    }
};

#endif