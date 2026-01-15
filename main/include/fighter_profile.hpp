#ifndef FIGHTER_PROFILE_HPP
#define FIGHTER_PROFILE_HPP

#include <string>
#include <vector>
#include <iostream>

struct FighterProfile {
    std::string id;          // "fighterA", "fighterB"
    std::string name;        // "Luchador A", "Luchador B"
    std::string style;       // "Agresivo", "Tecnico", "Defensivo", "Neutral"
    
    // Atributos (0-100)
    int agresividad;      // Tendencia a atacar
    int precision;        // Precisión en técnicas
    int defensa;          // Habilidad defensiva
    int tecnica;          // Habilidad técnica general
    int resistencia;      // Stamina y recuperación
    
    // Técnicas preferidas (IDs de técnicas)
    std::vector<std::string> preferred_technique_ids;
    
    FighterProfile() 
        : id(""), name(""), style("Neutral"), 
          agresividad(50), precision(50), defensa(50), tecnica(50), resistencia(50) {}
    
    FighterProfile(const std::string& i, const std::string& n, const std::string& s,
                   int agr, int pre, int def, int tec, int res)
        : id(i), name(n), style(s), 
          agresividad(agr), precision(pre), defensa(def), tecnica(tec), resistencia(res) {}
    
    // Método para mostrar el perfil
    void display() const {
        std::cout << "[PROFILE] " << name << " (" << id << ")" << std::endl;
        std::cout << "  Estilo: " << style << std::endl;
        std::cout << "  Atributos: AGR=" << agresividad 
                  << " PRE=" << precision 
                  << " DEF=" << defensa 
                  << " TEC=" << tecnica 
                  << " RES=" << resistencia << std::endl;
        std::cout << "  Técnicas preferidas: ";
        for (const auto& tech : preferred_technique_ids) {
            std::cout << tech << " ";
        }
        std::cout << std::endl;
    }
};

#endif