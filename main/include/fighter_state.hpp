#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "technique.hpp"
#include "fighter_profile.hpp"

struct FighterState {
    std::string my_name;
    std::string position;
    std::string opponent;
    bool is_attacker = false;
    int stamina = 100;
    int my_points = 0;
    int opponent_points = 0;
    double time_remaining = 300.0;
    bool is_winning = false;
    int exchanges = 0;
    double total_time = 0.0;
    
    enum class Phase {
        IDLE,
        THINKING,
        READY_TO_OUTPUT
    } phase = Phase::IDLE;
    
    double thinking_time = 0.0;
    double fatigue = 0.0;
    double pressure = 0.0;
    
    FighterProfile profile;
    
    std::vector<std::string> recent_techniques;
    const int MAX_RECENT_TECHNIQUES = 3;
    std::vector<std::string> recent_positions;
    const int MAX_RECENT_POSITIONS = 5;
    
    std::vector<Technique> techniques;
    const Technique* selected_technique = nullptr;
    
    FighterState(const std::string& name) : my_name(name) {
        profile = FighterProfile(name, name, "Neutral", 50, 50, 50, 50, 50);
    }
    
    // 🔴 NUEVO: Aplicar costo de energía cuando selecciona técnica
    bool canAffordTechnique(const Technique& tech) const {
        return stamina >= tech.energy_cost;
    }
    
    // 🔴 NUEVO: Gastar energía de la técnica seleccionada
    void spendEnergyForTechnique(const Technique& tech) {
        stamina -= tech.energy_cost;
        
        // Si es técnica defensiva, recuperar energía
        if (tech.type == "Defensiva") {
            stamina += tech.energy_gain;
            if (stamina > 100) stamina = 100;
        }
        
        // Asegurar no negativo
        if (stamina < 0) stamina = 0;
        
        // Actualizar fatiga
        fatigue = 100 - stamina;
        
        std::cout << "[ENERGY][" << my_name << "] Energía gastada: " 
                  << tech.energy_cost << " (Restante: " << stamina << ")" << std::endl;
    }
    
    // 🔴 MODIFICADO: Ahora verifica energía ANTES de seleccionar
    void finishThinking(const Technique* tech) {
        if (tech && canAffordTechnique(*tech)) {
            spendEnergyForTechnique(*tech);
            selected_technique = tech;
            recordTechnique(tech->id);
            phase = Phase::READY_TO_OUTPUT;
        } else {
            std::cout << "[WARNING][" << my_name 
                      << "] No puede pagar técnica o técnica nula. Reset." << std::endl;
            selected_technique = nullptr;
            phase = Phase::IDLE;
        }
    }
    
    double calculateThinkTime() const {
        double base_time = 0.8;
        
        if (profile.style == "Tecnico") base_time = 0.6;
        else if (profile.style == "Agresivo") base_time = 1.0;
        else if (profile.style == "Defensivo") base_time = 0.8;
        
        double fatigue_factor = 1.0 + (fatigue / 100.0);
        double pressure_factor = 1.0 - (pressure / 200.0);
        if (pressure_factor < 0.5) pressure_factor = 0.5;
        
        double final_time = base_time * fatigue_factor * pressure_factor;
        
        if (final_time < 0.3) final_time = 0.3;
        if (final_time > 2.0) final_time = 2.0;
        
        return final_time;
    }
    
    void startThinking() {
        phase = Phase::THINKING;
        thinking_time = calculateThinkTime();
        std::cout << "[STATE][" << my_name << "]" << "Energía: " << stamina << "" << std::endl;
    }
    
    bool isThinkingDone() const {
        return (phase == Phase::THINKING) && (thinking_time <= 0);
    }
    
    void resetAfterAction() {
        phase = Phase::IDLE;
        thinking_time = 0.0;
        selected_technique = nullptr;
        is_attacker = false;
    }
    
    void applyFatigueOverTime(double elapsed) {
        if (pressure > 30 || !is_winning) {
            fatigue += 0.05 * elapsed;
            if (fatigue > 100) fatigue = 100;
            pressure = std::max(0.0, pressure - (0.2 * elapsed));
        }
        
        stamina = 100 - static_cast<int>(fatigue);
        if (stamina < 0) stamina = 0;
    }
    
    void applyActionEffects(const Technique& tech) {
        if (tech.type == "Ofensiva") {
            pressure += tech.energy_cost * 0.3;
            if (pressure > 100) pressure = 100;
            fatigue += tech.energy_cost * 0.1;
        }
    }
    
    void recordTechnique(const std::string& tech_id) {
        recent_techniques.push_back(tech_id);
        if (recent_techniques.size() > MAX_RECENT_TECHNIQUES) {
            recent_techniques.erase(recent_techniques.begin());
        }
    }
    
    void recordPosition(const std::string& pos) {
        recent_positions.push_back(pos);
        if (recent_positions.size() > MAX_RECENT_POSITIONS) {
            recent_positions.erase(recent_positions.begin());
        }
    }
    
    bool wasRecentlyUsed(const std::string& tech_id) const {
        return std::find(recent_techniques.begin(), recent_techniques.end(), tech_id) 
               != recent_techniques.end();
    }
};

inline std::ostream &operator<<(std::ostream &os, const FighterState &s)
{
    os << "{name:" << s.my_name
       << ", phase:" << (s.phase == FighterState::Phase::IDLE ? "IDLE" : 
                        s.phase == FighterState::Phase::THINKING ? "THINKING" : "READY_TO_OUTPUT")
       << ", position:" << s.position
       << ", is_attacker:" << s.is_attacker
       << ", stamina:" << s.stamina
       << ", fatigue:" << s.fatigue
       << ", pressure:" << s.pressure
       << "}";
    return os;
}