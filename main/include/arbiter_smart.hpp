
#ifndef ARBITER_SMART_HPP
#define ARBITER_SMART_HPP

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <vector>
#include <limits>
#include "cadmium/modeling/devs/atomic.hpp"
#include "messages.hpp"

using namespace cadmium;

struct ArbiterSmartState {
    bool ready = false;
    std::vector<FightAction> actions;
    int energy_fighterA = 100;
    int energy_fighterB = 100;
    
    ArbiterSmartState() {
        srand(static_cast<unsigned int>(time(nullptr)));
    }
    
    // Método para actualizar energía
    void updateEnergy(const FightAction& action) {
        if(action.fighter_id == "fighterA") {
            energy_fighterA -= action.tech.energy_cost;
            if(action.tech.type == "Defensiva") {
                energy_fighterA += action.tech.energy_gain;
                if(energy_fighterA > 100) energy_fighterA = 100;
            }
            if(energy_fighterA < 0) energy_fighterA = 0;
        } else {
            energy_fighterB -= action.tech.energy_cost;
            if(action.tech.type == "Defensiva") {
                energy_fighterB += action.tech.energy_gain;
                if(energy_fighterB > 100) energy_fighterB = 100;
            }
            if(energy_fighterB < 0) energy_fighterB = 0;
        }
    }
};

// Función de impresión para ArbiterSmartState (FUERA de la clase)
inline std::ostream& operator<<(std::ostream& os, const ArbiterSmartState& s) {
    os << "[ArbiterSmartState ready=" << s.ready
       << ", actions_count=" << s.actions.size()
       << ", energy_A=" << s.energy_fighterA
       << ", energy_B=" << s.energy_fighterB
       << "]";
    return os;
}

class arbiter_smart : public Atomic<ArbiterSmartState> {
public:
    Port<FightAction> inA;
    Port<FightAction> inB;
    Port<FightAction> outAction;
    
    arbiter_smart(const std::string& id) : Atomic<ArbiterSmartState>(id, ArbiterSmartState()) {
        inA = addInPort<FightAction>("inA");
        inB = addInPort<FightAction>("inB");
        outAction = addOutPort<FightAction>("outAction");
    }
    
    void externalTransition(ArbiterSmartState& s, double) const override {
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
    
    void internalTransition(ArbiterSmartState& s) const override {
        s.actions.clear();
        s.ready = false;
    }
    
    void output(const ArbiterSmartState& s) const override {
        if(!s.ready || s.actions.empty()) return;
        
        std::cout << "\n[ARBITER_SMART] Decisión inteligente:" << std::endl;
        std::cout << "-------------------------" << std::endl;
        
        // 🔴 Primero filtrar por energía disponible
        std::vector<FightAction> affordable_actions;
        std::vector<FightAction> unaffordable_actions;
        
        for(const auto& action : s.actions) {
            int current_energy = (action.fighter_id == "fighterA") 
                                ? s.energy_fighterA 
                                : s.energy_fighterB;
            
            if(current_energy >= action.tech.energy_cost) {
                affordable_actions.push_back(action);
            } else {
                unaffordable_actions.push_back(action);
                std::cout << "[ARBITER_SMART] " << action.fighter_id 
                          << " no tiene suficiente energía para " 
                          << action.tech.name 
                          << " (necesita " << action.tech.energy_cost 
                          << ", tiene " << current_energy << ")" << std::endl;
            }
        }
        
        // 🔴 Si no hay acciones asequibles, buscar la de menor costo
        if(affordable_actions.empty() && !unaffordable_actions.empty()) {
            std::cout << "[ARBITER_SMART] Ninguna acción asequible, buscando la de menor costo..." << std::endl;
            
            // Ordenar por costo de energía
            std::sort(unaffordable_actions.begin(), unaffordable_actions.end(),
                [](const FightAction& a, const FightAction& b) {
                    return a.tech.energy_cost < b.tech.energy_cost;
                });
            
            // Tomar la de menor costo
            affordable_actions.push_back(unaffordable_actions[0]);
            std::cout << "[ARBITER_SMART] Usando " << unaffordable_actions[0].tech.name 
                      << " (costo mínimo: " << unaffordable_actions[0].tech.energy_cost << ")" << std::endl;
        }
        
        if(affordable_actions.empty()) {
            std::cout << "[ARBITER_SMART] No hay acciones posibles. Tiempo pasa sin acción." << std::endl;
            return; // No enviar ninguna acción, el tiempo pasará
        }
        
        // 🔴 Decisión basada en múltiples factores
        FightAction selected;
        if(affordable_actions.size() == 1) {
            // Solo una opción
            selected = affordable_actions[0];
        } else {
            // Elegir la mejor técnica basada en:
            // 1. Eficiencia energética (mayor ganancia/costo)
            // 2. Tiempo de ejecución
            // 3. Tipo de técnica (defensivas tienen prioridad cuando la energía es baja)
            
            // Primero, calcular puntaje para cada acción
            std::vector<std::pair<FightAction, double>> scored_actions;
            
            for(const auto& action : affordable_actions) {
                double score = 0.0;
                int current_energy = (action.fighter_id == "fighterA") 
                                    ? s.energy_fighterA 
                                    : s.energy_fighterB;
                
                // Factor 1: Eficiencia energética
                double efficiency = 0.0;
                if(action.tech.energy_cost > 0) {
                    efficiency = static_cast<double>(action.tech.energy_gain) / action.tech.energy_cost;
                }
                
                // Factor 2: Velocidad (menor tiempo es mejor)
                double speed_factor = 1.0 / (action.tech.time_cost + 0.1);
                
                // Factor 3: Prioridad de defensa si energía baja
                double defense_bonus = 0.0;
                if(current_energy < 30 && action.tech.type == "Defensiva") {
                    defense_bonus = 2.0; // Bonus para técnicas defensivas cuando la energía es baja
                }
                
                // Factor 4: Penalización por bajo stamina
                double stamina_penalty = (100.0 - current_energy) / 100.0;
                
                // Factor 5: Bonus por energía disponible vs costo
                double energy_ratio = static_cast<double>(current_energy) / action.tech.energy_cost;
                if(energy_ratio > 2.0) score += 0.5;
                
                score = efficiency * 3.0 + speed_factor * 2.0 + defense_bonus - stamina_penalty;
                
                scored_actions.push_back({action, score});
            }
            
            // Seleccionar la acción con mayor puntaje
            auto best_action = *std::max_element(scored_actions.begin(), scored_actions.end(),
                [](const auto& a, const auto& b) {
                    return a.second < b.second;
                });
            
            selected = best_action.first;
        }
        
        // 🔴 Verificar si la acción se puede ejecutar (energía suficiente)
        int current_energy = (selected.fighter_id == "fighterA") 
                            ? s.energy_fighterA 
                            : s.energy_fighterB;
        
        if(current_energy < selected.tech.energy_cost) {
            std::cout << "[ARBITER_SMART] ⚠️ Acción " << selected.tech.name 
                      << " NO se ejecuta por falta de energía" << std::endl;
            std::cout << "[ARBITER_SMART] Enviando acción de descanso/respiro" << std::endl;
            
            // 🔴 Crear acción de "descanso" o "respiro" automática
            Technique rest_tech;
            rest_tech.id = "REST";
            rest_tech.name = "Descanso";
            rest_tech.type = "Defensiva";
            rest_tech.category = "Recuperación";
            rest_tech.time_cost = 5; // 5 segundos de descanso
            rest_tech.energy_cost = 0;
            rest_tech.energy_gain = 5; // Recupera 5 de energía
            
            // Mantener posiciones actuales
            rest_tech.from_state = "Pos(A:De_Pie, O:De_Pie)"; // Placeholder
            rest_tech.to_state_success = "Pos(A:De_Pie, O:De_Pie)";
            rest_tech.to_state_fail = "Pos(A:De_Pie, O:De_Pie)";
            
            FightAction rest_action(selected.fighter_id, rest_tech);
            outAction->addMessage(rest_action);
            
            std::cout << "🏃 " << selected.fighter_id << " descansa (Energía +5)" << std::endl;
        } else {
            // Ejecutar acción normalmente
            const_cast<ArbiterSmartState&>(s).updateEnergy(selected);
            outAction->addMessage(selected);
            
            std::cout << "🏆 SELECCIONADA: " << selected.fighter_id 
                      << " - " << selected.tech.name 
                      << " (Tiempo: " << selected.tech.time_cost << "s)" << std::endl;
        }
        
        std::cout << "Energía actual - A: " << s.energy_fighterA 
                  << " B: " << s.energy_fighterB << std::endl;
        std::cout << "-------------------------\n" << std::endl;
    }
    
    double timeAdvance(const ArbiterSmartState& s) const override {
        return s.ready ? 0.0 : std::numeric_limits<double>::infinity();
    }
};

#endif
