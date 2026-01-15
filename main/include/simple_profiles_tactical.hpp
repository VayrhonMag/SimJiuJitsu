
#ifndef SIMPLE_PROFILES_TACTICAL_HPP
#define SIMPLE_PROFILES_TACTICAL_HPP

#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <cctype>
#include "technique.hpp"
#include "fighter_profile.hpp"

inline void initRandomOnce()
{
    static bool initialized = false;
    if (!initialized)
    {
        srand(static_cast<unsigned int>(time(nullptr)));
        initialized = true;
    }
}

// Determinar si una posición es ventajosa/ofensiva
inline bool isAdvantagePosition(const std::string &my_position)
{
    return (my_position == "Montada_Superior" ||
            my_position == "Control_Lateral_Superior" ||
            my_position == "S-Mount_Superior" ||
            my_position == "Back_Mount_Superior" ||
            my_position == "North_South_Superior" ||
            my_position.find("Mount") != std::string::npos);
}

// Determinar si una posición es de desventaja/defensiva
inline bool isDisadvantagePosition(const std::string &my_position)
{
    return (my_position == "Montada_Inferior" ||
            my_position == "Control_Lateral_Inferior" ||
            my_position == "S-Mount_Inferior" ||
            my_position == "Back_Mount_Inferior" ||
            my_position == "North_South_Inferior" ||
            my_position.find("Sumision") != std::string::npos);
}

// Determinar si es posición neutral (guardia)
inline bool isNeutralPosition(const std::string &my_position)
{
    return (my_position.find("Guardia") != std::string::npos ||
            my_position == "De_Pie" ||
            my_position.find("Media_Guardia") != std::string::npos ||
            my_position.find("Half_Guard") != std::string::npos);
}

// Detectar bloqueo posicional
inline bool isPositionalDeadlock(const std::string& position, 
                                 const std::vector<std::string>& recent_positions,
                                 int consecutive_count = 3) {
    if (recent_positions.size() < consecutive_count) return false;
    
    int count = 0;
    for (int i = recent_positions.size() - 1; 
         i >= 0 && i >= (int)recent_positions.size() - consecutive_count; 
         i--) {
        if (recent_positions[i] == position) count++;
    }
    
    return count >= consecutive_count;
}

// Declaración anticipada
inline const Technique *selectTacticalWithProfile(
    const std::vector<const Technique *> &techniques,
    const std::string &my_position,
    const FighterProfile* profile,
    const std::string &style_backup,
    bool is_winning,
    int stamina,
    const std::vector<std::string>& recent_techs = {},
    const std::vector<std::string>& recent_positions = {});

// 🔴 VERSIÓN SIMPLE (para compatibilidad)
inline const Technique *selectTactical(
    const std::vector<const Technique *> &techniques,
    const std::string &my_position,
    const std::string &style,
    bool is_winning,
    int stamina,
    const std::vector<std::string>& recent_techs = {},
    const std::vector<std::string>& recent_positions = {})
{
    return selectTacticalWithProfile(techniques, my_position, nullptr, style, 
                                     is_winning, stamina, recent_techs, recent_positions);
}

// 🔴 VERSIÓN CON PERFIL COMPLETO - MEJORADA
inline const Technique *selectTacticalWithProfile(
    const std::vector<const Technique *> &techniques,
    const std::string &my_position,
    const FighterProfile* profile,
    const std::string &style_backup,
    bool is_winning,
    int stamina,
    const std::vector<std::string>& recent_techs,
    const std::vector<std::string>& recent_positions)
{
    if (techniques.empty())
        return nullptr;
    initRandomOnce();

    // 🔴 Determinar estilo efectivo
    std::string effective_style = style_backup;
    
    if (profile != nullptr) {
        std::string style_lower = profile->style;
        std::transform(style_lower.begin(), style_lower.end(), style_lower.begin(), 
                       [](unsigned char c){ return std::tolower(c); });
        
        if (style_lower.find("agres") != std::string::npos) {
            effective_style = "aggressive";
        } else if (style_lower.find("tecnic") != std::string::npos) {
            effective_style = "technical";
        } else if (style_lower.find("defens") != std::string::npos) {
            effective_style = "defensive";
        } else {
            effective_style = "neutral";
        }
    }

    // Detectar bloqueo posicional
    bool positional_deadlock = isPositionalDeadlock(my_position, recent_positions, 3);

    // 🔴 Filtrar por técnicas frescas (NO recientemente usadas)
    std::vector<const Technique *> fresh_techniques;
    
    for (const auto *t : techniques) {
        bool was_recently_used = false;
        
        for (const auto& recent_id : recent_techs) {
            if (t->id == recent_id) {
                was_recently_used = true;
                break;
            }
        }
        
        if (!was_recently_used) {
            fresh_techniques.push_back(t);
        }
    }
    
    // 🔴 Si no hay técnicas frescas, usar todas (pero con penalización)
    const std::vector<const Technique *>* tech_list = &fresh_techniques;
    bool using_fresh = true;
    
    if (fresh_techniques.empty()) {
        tech_list = &techniques;
        using_fresh = false;
    }
    
    // 🔴 Filtrar por técnicas preferidas del perfil
    std::vector<const Technique *> preferred_techniques;
    if (profile != nullptr && !profile->preferred_technique_ids.empty()) {
        for (const auto *t : *tech_list) {
            for (const auto& pref_id : profile->preferred_technique_ids) {
                if (t->id == pref_id) {
                    preferred_techniques.push_back(t);
                    break;
                }
            }
        }
        
        if (!preferred_techniques.empty()) {
            tech_list = &preferred_techniques;
        }
    }
    
    // 🔴 Determinar situación táctica
    std::vector<const Technique *> tactical_choices;

    if (isAdvantagePosition(my_position))
    {
        // DOMINIO REAL - priorizar ofensivas
        for (const auto *t : *tech_list)
        {
            if (t->type == "Ofensiva")
            {
                tactical_choices.push_back(t);
            }
        }
        if (tactical_choices.empty())
            tactical_choices = *tech_list;
    }
    else if (isDisadvantagePosition(my_position))
    {
        // EN DESVENTAJA REAL - priorizar defensivas/escapes
        for (const auto *t : *tech_list)
        {
            if (t->type == "Defensiva" ||
                t->category.find("Escape") != std::string::npos ||
                t->category.find("Raspado") != std::string::npos)
            {
                tactical_choices.push_back(t);
            }
        }
        if (tactical_choices.empty())
            tactical_choices = *tech_list;
    }
    else
    {
        // POSICIÓN NEUTRAL - según estilo
        if (effective_style == "aggressive")
        {
            // Agresivo: prefiere pasar guardia/avanzar
            for (const auto *t : *tech_list)
            {
                if (t->category.find("Pase") != std::string::npos ||
                    t->type == "Ofensiva")
                {
                    tactical_choices.push_back(t);
                }
            }
            if (tactical_choices.empty())
                tactical_choices = *tech_list;
        }
        else if (effective_style == "technical")
        {
            // Técnico: prefiere técnicas eficientes
            for (const auto *t : *tech_list)
            {
                if (t->category.find("Pase") != std::string::npos ||
                    t->category.find("Transicion") != std::string::npos ||
                    t->energy_cost <= 5 ||
                    (t->energy_gain > 0 && t->energy_gain > t->energy_cost))
                {
                    tactical_choices.push_back(t);
                }
            }
            if (tactical_choices.empty())
                tactical_choices = *tech_list;
        }
        else if (effective_style == "defensive")
        {
            // Defensivo: prefiere guardia/control
            for (const auto *t : *tech_list)
            {
                if (t->category.find("Guardia") != std::string::npos ||
                    t->type == "Defensiva" ||
                    t->type == "Neutra")
                {
                    tactical_choices.push_back(t);
                }
            }
            if (tactical_choices.empty())
                tactical_choices = *tech_list;
        }
        else
        { // neutral
            tactical_choices = *tech_list;
        }
    }

    // 🔴 Calcular pesos
    std::vector<int> weights(tactical_choices.size(), 10);

    for (size_t i = 0; i < tactical_choices.size(); i++)
    {
        const auto *t = tactical_choices[i];
        int weight_adjustment = 0;

        // 🔴 Penalización fuerte por técnicas recientemente usadas
        if (!using_fresh) {
            weight_adjustment -= 50; // Muy fuerte penalización
        }
        
        // 🔴 Bonus por romper bloqueo posicional
        if (positional_deadlock) {
            if (t->to_state_success.find(my_position) == std::string::npos &&
                t->to_state_success != t->from_state) {
                weight_adjustment += 40;
            }
        }

        // 🔴 Bonus por técnicas preferidas del perfil
        if (profile != nullptr) {
            bool is_preferred = false;
            for (const auto& pref_id : profile->preferred_technique_ids) {
                if (t->id == pref_id) {
                    is_preferred = true;
                    break;
                }
            }
            
            if (is_preferred) {
                weight_adjustment += 30;
            }
            
            // Considerar atributos del perfil
            if (profile->tecnica > 70 && t->energy_cost > 8) {
                weight_adjustment += 10;
            }
            
            if (profile->defensa > 70 && t->type == "Defensiva") {
                weight_adjustment += 10;
            }
            
            if (profile->agresividad > 70 && t->type == "Ofensiva") {
                weight_adjustment += 10;
            }
        }

        // Bonus por estilo
        if (effective_style == "aggressive")
        {
            if (t->type == "Ofensiva")
                weight_adjustment += 20;
            if (t->category.find("Sumision") != std::string::npos)
            {
                if (isAdvantagePosition(my_position) && stamina > 50)
                {
                    weight_adjustment += 15;
                }
                else
                {
                    weight_adjustment -= 15;
                }
            }
        }
        else if (effective_style == "technical")
        {
            if (t->energy_cost <= 5 && t->energy_gain >= 3) {
                weight_adjustment += 15;
            }
            if (t->category.find("Pase") != std::string::npos) {
                weight_adjustment += 10;
            }
        }
        else if (effective_style == "defensive")
        {
            if (t->type == "Defensiva")
                weight_adjustment += 20;
            if (t->category.find("Raspado") != std::string::npos)
                weight_adjustment += 15;

            if (t->type == "Ofensiva" && isAdvantagePosition(my_position))
            {
                weight_adjustment += 10;
            }
        }
        else // neutral
        {
            if (t->type == "Ofensiva") weight_adjustment += 5;
            if (t->type == "Defensiva") weight_adjustment += 5;
        }

        // Bonus por situación de combate
        if (!is_winning && stamina > 60)
        {
            if (t->type == "Ofensiva") {
                weight_adjustment += 10;
            }
        }

        if (is_winning && stamina < 40)
        {
            if (t->type == "Defensiva" || t->type == "Neutra") {
                weight_adjustment += 10;
            }
        }
        
        // Bonus por eficiencia energética
        if (t->energy_cost > 0) {
            double efficiency = (double)t->energy_gain / t->energy_cost;
            if (efficiency > 1.5) {
                weight_adjustment += 10;
            }
        }
        
        weights[i] += weight_adjustment;
        
        // Mínimo peso de 1
        if (weights[i] < 1) weights[i] = 1;
    }

    // 🔴 Selección ponderada
    int total_weight = 0;
    for (int w : weights)
        total_weight += w;

    if (total_weight <= 0) {
        return tactical_choices[0];
    }

    int random_val = rand() % total_weight;
    int cumulative = 0;

    for (size_t i = 0; i < tactical_choices.size(); i++)
    {
        cumulative += weights[i];
        if (random_val < cumulative)
        {
            return tactical_choices[i];
        }
    }

    return tactical_choices[0];
}

#endif
