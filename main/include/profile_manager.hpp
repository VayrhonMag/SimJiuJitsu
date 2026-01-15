#ifndef PROFILE_MANAGER_HPP
#define PROFILE_MANAGER_HPP

#include <string>
#include <iostream>
#include "fighter_profile.hpp"

class ProfileManager
{
public:
    ProfileManager()
    {
        std::cout << "[INFO] ProfileManager inicializado" << std::endl;
    }

    const FighterProfile &getProfile(const std::string &fighter_id)
    {
        std::cout << "[DEBUG] getProfile llamado para: '" << fighter_id << "'" << std::endl;

        // 🔴 PERFIL PARA FIGHTER A
        if (fighter_id == "fighterA")
        {
            static FighterProfile fighterA_profile("fighterA", "Luchador A", "Agresivo", 80, 40, 65, 70, 75);
            static bool a_initialized = false;

            if (!a_initialized)
            {
                fighterA_profile.preferred_technique_ids = {"T01", "T02", "T08", "T12", "T13", "T22"};
                a_initialized = true;
                std::cout << "[INFO] Perfil A creado: " << fighterA_profile.name
                          << " (" << fighterA_profile.style << ")" << std::endl;
            }
            return fighterA_profile;
        }

        //     if (!a_initialized) {
        //         fighterA_profile.preferred_technique_ids = {"T17", "T18", "T19", "T20", "T09", "T26"};
        //         a_initialized = true;
        //         std::cout << "[INFO] Perfil A creado: " << fighterA_profile.name
        //                   << " (" << fighterA_profile.style << ")" << std::endl;
        //     }
        //     return fighterB_profile;
        // }

        // 🔴 PERFIL PARA FIGHTER B
        if (fighter_id == "fighterB")
        {
            static FighterProfile fighterB_profile("fighterB", "Luchador B", "Tecnico",45, 85, 75, 85, 60);
            static bool b_initialized = false;

            if (!b_initialized)
            {
                fighterB_profile.preferred_technique_ids = {"T17", "T18", "T19", "T20", "T09", "T26"};
                b_initialized = true;
                std::cout << "[INFO] Perfil B creado: " << fighterB_profile.name
                          << " (" << fighterB_profile.style << ")" << std::endl;
            }
            return fighterB_profile;
        }
        // if (fighter_id == "fighterB") {
        //     static FighterProfile fighterB_profile("fighterB", "Luchador B", "Defensivo", 45, 85, 75, 85, 60);
        //     static bool b_initialized = false;
            
        //     if (!b_initialized) {
        //         fighterB_profile.preferred_technique_ids = {"T17", "T18", "T19", "T20", "T09", "T26"};
        //         b_initialized = true;
        //         std::cout << "[INFO] Perfil B creado: " << fighterB_profile.name 
        //                   << " (" << fighterB_profile.style << ")" << std::endl;
        //     }
        //     return fighterB_profile;
        // }

        // 🔴 PERFIL POR DEFECTO (si el ID no es reconocido)
        std::cout << "[WARN] ID no reconocido: '" << fighter_id
                  << "', usando perfil por defecto" << std::endl;

        static FighterProfile default_profile("default", "Luchador Genérico", "Neutral",
                                              50, 50, 50, 50, 50);
        static bool default_initialized = false;

        if (!default_initialized)
        {
            default_profile.preferred_technique_ids = {"T01", "T17"}; // Una técnica básica de cada tipo
            default_initialized = true;
        }

        return default_profile;
    }
};

#endif