#ifndef TECHNIQUE_LOADER_HPP
#define TECHNIQUE_LOADER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include "technique.hpp"

inline std::vector<Technique> loadTechniqueCSV(const std::string &filename)
{
    std::vector<Technique> techniques;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "[ERROR] No se pudo abrir el archivo: " << filename << std::endl;
        return techniques;
    }

    std::string line;
    int line_count = 0;

    // Leer la cabecera
    std::getline(file, line);
    std::cout << "[DEBUG] Cabecera CSV: " << line << std::endl;
    std::cout << "[DEBUG] Separador: punto y coma (;)" << std::endl;

    while (std::getline(file, line))
    {
        line_count++;
        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> fields;

        // Parsear línea con punto y coma como separador
        while (std::getline(ss, field, ';'))
        {
            fields.push_back(field);
        }

        // Mostrar información de depuración para las primeras líneas
        if (line_count <= 5)
        {
            std::cout << "[DEBUG] Línea " << line_count << ": " << fields.size() << " campos" << std::endl;
        }

        if (fields.size() >= 10)
        { // Tu CSV tiene 10 campos
            try
            {
                Technique t;

                // Campo 1: ID
                t.id = fields[0];

                // Campo 2: Nombre
                t.name = fields[1];

                // Campo 3: Categoría
                t.category = fields[2];

                // Campo 4: Tipo
                t.type = fields[3];

                // Campo 5: Estado Inicial
                t.from_state = fields[4];

                // Campo 6: Estado Final Exitoso
                t.to_state_success = fields[5];

                // Campo 7: Estado Final Fallido
                t.to_state_fail = fields[6];

                // Campo 8: Tiempo (s) - convertir a int
                try
                {
                    t.time_cost = std::stoi(fields[7]);
                }
                catch (...)
                {
                    t.time_cost = 3; // Valor por defecto
                }

                // Campo 9: Costo de Energía - convertir a int
                try
                {
                    t.energy_cost = std::stoi(fields[8]);
                }
                catch (...)
                {
                    t.energy_cost = 5; // Valor por defecto
                }

                // Campo 10: Ganancia de Energía - convertir a int
                try
                {
                    t.energy_gain = std::stoi(fields[9]);
                }
                catch (...)
                {
                    t.energy_gain = 0; // Valor por defecto
                }

                // Campos de descripción (opcionales, no están en tu CSV)
                t.desc_init = "";
                t.desc_success = "";
                t.desc_fail = "";

                // Validación adicional para técnicas defensivas
                if (t.type == "Defensiva" && t.energy_gain == 0 && t.energy_cost > 0)
                {
                    // Las técnicas defensivas deberían tener ganancia de energía
                    t.energy_gain = t.energy_cost + 2; // +2 de ganancia por defecto
                }

                techniques.push_back(t);

                if (line_count <= 5)
                {
                    std::cout << "[DEBUG] Cargada técnica: " << t.name
                              << " (Tipo: " << t.type
                              << ", Tiempo: " << t.time_cost
                              << "s, Energía: " << t.energy_cost
                              << "/" << t.energy_gain << ")" << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "[ERROR] Línea " << line_count
                          << ": Error al parsear: " << e.what()
                          << " - Línea: " << line << std::endl;
            }
        }
        else
        {
            std::cerr << "[WARN] Línea " << line_count
                      << ": Solo " << fields.size()
                      << " campos (se esperaban al menos 10)" << std::endl;
            std::cerr << "[WARN] Línea: " << line << std::endl;
        }
    }

    std::cout << "[INFO] Cargadas " << techniques.size()
              << " técnicas desde " << filename << std::endl;

    // Mostrar estadísticas
    int ofensivas = 0, defensivas = 0, neutras = 0;
    int sumisiones = 0, derribos = 0, pases = 0, escapes = 0, transiciones = 0, raspados = 0;

    for (const auto &t : techniques)
    {
        if (t.type == "Ofensiva")
            ofensivas++;
        else if (t.type == "Defensiva")
            defensivas++;
        else if (t.type == "Neutra")
            neutras++;

        if (t.category.find("Sumision") != std::string::npos ||
            t.category.find("Sumisión") != std::string::npos)
            sumisiones++;
        else if (t.category.find("Derribo") != std::string::npos)
            derribos++;
        else if (t.category.find("Pase") != std::string::npos)
            pases++;
        else if (t.category.find("Escape") != std::string::npos)
            escapes++;
        else if (t.category.find("Transicion") != std::string::npos ||
                 t.category.find("Transición") != std::string::npos)
            transiciones++;
        else if (t.category.find("Raspado") != std::string::npos)
            raspados++;
    }

    std::cout << "[INFO] Estadísticas:" << std::endl;
    std::cout << "[INFO]   Ofensivas: " << ofensivas
              << ", Defensivas: " << defensivas
              << ", Neutras: " << neutras << std::endl;
    std::cout << "[INFO]   Sumisiones: " << sumisiones
              << ", Derribos: " << derribos
              << ", Pases: " << pases << std::endl;
    std::cout << "[INFO]   Escapes: " << escapes
              << ", Transiciones: " << transiciones
              << ", Raspados: " << raspados << std::endl;

    file.close();
    return techniques;
}

#endif // TECHNIQUE_LOADER_HPP