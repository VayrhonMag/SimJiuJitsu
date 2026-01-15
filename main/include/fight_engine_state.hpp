#ifndef FIGHT_ENGINE_STATE_HPP
#define FIGHT_ENGINE_STATE_HPP

#include <string>
#include <ostream>
#include <cstdlib>  // Para rand()
#include <ctime>    // Para time()

struct FightEngineState
{
    bool finished = false;
    int exchanges = 0;
    std::string global_state = "Pos(A:De_Pie, O:De_Pie)";

    // 🔴 MAPEO DE ROLES: quién es A (Atacante) y O (Oponente) en este momento
    std::string actor_A = "fighterA"; // Quién representa el rol A
    std::string actor_O = "fighterB"; // Quién representa el rol O

    // 🔴 TIEMPO DE PENSAMIENTO DINÁMICO (basado en energía y experiencia)
    double think_time_A = 1.5; // Tiempo base para pensar (segundos)
    double think_time_B = 1.5;
    
    // Tiempo total transcurrido
    double total_time = 0.0;
    
    // Tiempo máximo del combate (5 minutos = 300 segundos)
    const double MAX_FIGHT_TIME = 300.0;
    
    bool has_output = true;
    bool last_success = false;
    std::string current_actor = ""; // Quién ataca AHORA
    std::string last_actor = "";    // Quién atacó la última vez
    bool thinking = false;
    
    // Energía de los luchadores
    int energy_fighterA = 100;
    int energy_fighterB = 100;
    
    // 🔴 NUEVO: Sistema de puntos (SOLO técnicas ofensivas)
    int points_fighterA = 0;
    int points_fighterB = 0;
    
    // 🔴 NUEVO: Información de la última técnica usada
    std::string last_technique_name = "";
    std::string last_technique_type = "";
    std::string last_technique_category = "";
    int last_technique_success_chance = 50;
    
    // 🔴 NUEVO: Variables para manejo de sumisiones
    double last_submission_time = 0.0;       // Tiempo de la última sumisión bloqueada
    std::string state_before_submission = ""; // Estado ANTES de intentar sumisión
    int submission_blocked_count = 0;        // Contador de sumisiones bloqueadas
    const double SUBMISSION_COOLDOWN = 60.0; // Tiempo de bloqueo después de sumisión fallida (60 segundos)
    
    // Tipos de victoria
    enum VictoryType {
        NONE,
        SUBMISSION,
        KO,
        POINTS,
        DOUBLE_KO
    } victory_type = NONE;
    
    std::string winner = "";

    FightEngineState()
    {
        // Inicializar random seed
        srand(static_cast<unsigned int>(time(nullptr)));
        
        // 🔴 Inicializar tiempos de pensamiento aleatorios
        think_time_A = 0.8 + (rand() % 700) / 1000.0; // 0.8-1.5 segundos
        think_time_B = 0.8 + (rand() % 700) / 1000.0;
        
        // Inicial: quien tenga menor tiempo de pensamiento empieza
        if (think_time_A <= think_time_B)
        {
            actor_A = "fighterA";
            actor_O = "fighterB";
            current_actor = "fighterA";
        }
        else
        {
            actor_A = "fighterB";
            actor_O = "fighterA";
            current_actor = "fighterB";
        }
        
        // 🔴 Inicializar estado antes de sumisión
        state_before_submission = global_state;
        
        std::cout << "[DEBUG][ENGINE] Primer atacante (más rápido): " << current_actor << std::endl;
        std::cout << "[DEBUG][ENGINE] Cooldown sumisión: " << SUBMISSION_COOLDOWN << "s" << std::endl;
    }
    
    // 🔴 Método para actualizar tiempos de pensamiento basado en energía
    void updateThinkTimes() {
        // Más energía = piensa más rápido
        think_time_A = 1.5 * (100.0 - energy_fighterA) / 100.0 + 0.5;
        think_time_B = 1.5 * (100.0 - energy_fighterB) / 100.0 + 0.5;
        
        // Límites: 0.5s mínimo, 2.0s máximo
        if (think_time_A < 0.5) think_time_A = 0.5;
        if (think_time_A > 2.0) think_time_A = 2.0;
        if (think_time_B < 0.5) think_time_B = 0.5;
        if (think_time_B > 2.0) think_time_B = 2.0;
    }
    
    // 🔴 Método para registrar una técnica usada
    void recordTechniqueUsed(const std::string& name, 
                            const std::string& type,
                            const std::string& category,
                            int success_chance) {
        last_technique_name = name;
        last_technique_type = type;
        last_technique_category = category;
        last_technique_success_chance = success_chance;
        last_actor = current_actor;
        
        // 🔴 GUARDAR ESTADO ANTERIOR (siempre que no sea sumisión)
        // Para sumisiones, guardamos el estado ANTES de intentarla
        if (category != "Sumision") {
            state_before_submission = global_state;
        }
    }
    
    // 🔴 Método ESPECIAL para preparar un intento de sumisión
    void prepareSubmissionAttempt() {
        // Guardar el estado actual ANTES de intentar la sumisión
        state_before_submission = global_state;
        std::cout << "[DEBUG][ENGINE] Guardando estado antes de sumisión: " 
                  << state_before_submission << std::endl;
    }
    
    // 🔴 Método para revertir a estado anterior después de sumisión fallida
    void revertAfterFailedSubmission() {
        if (!state_before_submission.empty()) {
            global_state = state_before_submission;
            std::cout << "[DEBUG][ENGINE] Revertiendo a estado anterior: " 
                      << state_before_submission << std::endl;
        } else {
            // Si no hay estado guardado, usar uno por defecto
            global_state = "Pos(A:De_Pie, O:De_Pie)";
            std::cout << "[DEBUG][ENGINE] Usando estado por defecto: " 
                      << global_state << std::endl;
        }
    }
    
    // 🔴 Método para verificar si una sumisión está bloqueada por cooldown
    bool isSubmissionBlocked() const {
        if (last_submission_time == 0.0) return false; // Nunca se ha intentado
        
        double time_since_last = total_time - last_submission_time;
        return time_since_last < SUBMISSION_COOLDOWN;
    }
    
    // 🔴 Método para obtener tiempo restante de cooldown
    double getSubmissionCooldownRemaining() const {
        if (last_submission_time == 0.0) return 0.0;
        
        double time_since_last = total_time - last_submission_time;
        if (time_since_last >= SUBMISSION_COOLDOWN) return 0.0;
        
        return SUBMISSION_COOLDOWN - time_since_last;
    }
};

inline std::ostream &operator<<(std::ostream &os, const FightEngineState &s)
{
    os << "{finished:" << s.finished
       << ", exchanges:" << s.exchanges
       << ", global_state:" << s.global_state
       << ", total_time:" << s.total_time
       << ", energy_A:" << s.energy_fighterA
       << ", energy_B:" << s.energy_fighterB
       << ", points_A:" << s.points_fighterA
       << ", points_B:" << s.points_fighterB
       << ", think_time_A:" << s.think_time_A
       << ", think_time_B:" << s.think_time_B
       << ", last_technique:" << s.last_technique_name
       << ", last_technique_success:" << s.last_technique_success_chance
       << ", victory_type:" << s.victory_type
       << ", winner:" << s.winner
       << "}";
    return os;
}

#endif