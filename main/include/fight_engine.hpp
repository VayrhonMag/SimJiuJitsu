
#ifndef FIGHT_ENGINE_HPP
#define FIGHT_ENGINE_HPP

#include <iostream>
#include <cstdlib>
#include <random>
#include <cmath>

#include "cadmium/modeling/devs/atomic.hpp"
#include "fight_engine_state.hpp"
#include "messages.hpp"
#include "technique.hpp"
#include "role_helper.hpp"

inline double random_between(double min, double max)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

using namespace cadmium;

class fight_engine : public Atomic<FightEngineState>
{
public:
    Port<FightAction> in;
    Port<FightUpdate> outA;
    Port<FightUpdate> outB;

    fight_engine(const std::string &id)
        : Atomic<FightEngineState>(id, FightEngineState())
    {
        in = addInPort<FightAction>("in_action");
        outA = addOutPort<FightUpdate>("outA_update");
        outB = addOutPort<FightUpdate>("outB_update");

        // Inicializar random seed
        srand(time(nullptr));
    }

    // 🔴 Función para calcular puntos SOLO por técnicas ofensivas exitosas
    int calculatePoints(const Technique &tech, const std::string &attacker) const
    {
        // 🔴 MODIFICACIÓN: Raspados SÍ dan puntos aunque sean "Defensiva"
        if (tech.type != "Ofensiva" && tech.category != "Raspado") {
            return 0; // Solo ofensivas Y raspados dan puntos
        }
        
        int base_points = 0;
        
        if (tech.category == "Golpe") {
            base_points = 1;
        }
        else if (tech.category == "Llave") {
            base_points = 2;
        }
        else if (tech.category == "Derribo") {
            base_points = 2;
        }
        else if (tech.category == "Sumision") {
            base_points = 4;
        }
        else if (tech.category == "Raspado") {  // 🔴 NUEVO
            base_points = 2; // 2 puntos por raspado (como IBJJF)
        }
        else {
            base_points = 1;
        }
        
        return base_points;
    }

    // 🔴 Calcular probabilidad de éxito basada en energía y tipo de técnica
    int calculateSuccessChance(const FightAction &act, int attacker_energy, int opponent_energy, double total_time) const
    {
        int base_chance = 50; // 50% base

        // Modificar según energía del atacante
        if (attacker_energy > 80)
            base_chance += 10;
        else if (attacker_energy > 50)
            base_chance += 5;
        else if (attacker_energy < 30)
            base_chance -= 15;
        else if (attacker_energy < 10)
            base_chance -= 30;

        // Modificar según energía del oponente (defensa)
        if (opponent_energy > 80)
            base_chance -= 10;
        else if (opponent_energy < 30)
            base_chance += 10;

        // Modificar según tipo de técnica
        if (act.tech.type == "Defensiva")
        {
            base_chance += 15; // Las defensas son más confiables
        }
        else if (act.tech.type == "Ofensiva")
        {
            // 🔴 TÉCNICAS DE SUMISIÓN MUCHO MÁS DIFÍCILES, ESPECIALMENTE TEMPRANO
            if (act.tech.category == "Sumision")
            {
                base_chance -= 50; // 50% menos de probabilidad base

                // Aún más difícil al inicio del combate
                if (total_time < 60)
                { // Primer minuto
                    base_chance -= 20;
                    std::cout << "[DEBUG][ENGINE] Sumisión temprana (t < 60s) - probabilidad muy reducida" << std::endl;
                }
                else if (total_time < 120)
                { // Primeros 2 minutos
                    base_chance -= 10;
                }

                // Más difícil si el oponente tiene buena energía
                if (opponent_energy > 70)
                    base_chance -= 10;
                if (opponent_energy > 50)
                    base_chance -= 5;

                // Más fácil si el oponente está muy cansado
                if (opponent_energy < 20)
                    base_chance += 15;

                std::cout << "[DEBUG][ENGINE] Técnica de sumisión - probabilidad final: "
                          << base_chance << "%" << std::endl;
            }

            if (act.tech.energy_cost > 25)
                base_chance -= 10; // Técnicas pesadas
            if (act.tech.time_cost < 0.5)
                base_chance += 5; // Técnicas rápidas
        }

        // Limitar entre 5% y 90%
        if (base_chance < 5)
            base_chance = 5;
        if (base_chance > 90)
            base_chance = 90;

        return base_chance;
    }

    void externalTransition(FightEngineState &s, double e) const override
    {
        // Si recibimos una acción, procesarla
        if (!in->empty())
        {
            for (const auto &act : in->getBag())
            {
                std::cout << "Recibí acción de: "
                          << act.fighter_id
                          << " - " << act.tech.name
                          << " (Costo energía: " << act.tech.energy_cost 
                          << ", Tiempo: " << act.tech.time_cost << "s)" << std::endl;

                const Technique &t = act.tech;

                // 🔴 NOTA: El luchador YA gastó su energía al seleccionar la técnica
                // Solo mantenemos tracking en el engine para mostrar en logs
                
                // Obtener energías actuales del engine
                int attacker_energy = (act.fighter_id == "fighterA")
                                          ? s.energy_fighterA
                                          : s.energy_fighterB;
                int opponent_energy = (act.fighter_id == "fighterA")
                                          ? s.energy_fighterB
                                          : s.energy_fighterA;

                // Calcular probabilidad de éxito dinámica
                int success_chance = calculateSuccessChance(act, attacker_energy, opponent_energy, s.total_time);
                bool success = (rand() % 100) < success_chance;

                std::cout << "% Éxito: " << (success ? "SÍ" : "NO") << "" << std::endl;

                // 🔴 Actualizar tiempo total CON EL TIEMPO DE LA TÉCNICA
                s.total_time += t.time_cost;

                std::cout << "[DEBUG][ENGINE] Tiempo agregado: " << t.time_cost
                          << "s (Total acumulado: " << s.total_time << "s)" << std::endl;

                // 🔴 ACTUALIZAR ENERGÍAS EN EL ENGINE (solo para tracking y condiciones de KO)
                if (act.fighter_id == "fighterA") {
                    s.energy_fighterA -= t.energy_cost;
                    if (t.type == "Defensiva" && success) { // 🔴 Solo si es exitosa
                        s.energy_fighterA += t.energy_gain;
                        if (s.energy_fighterA > 100) s.energy_fighterA = 100;
                    }
                    if (s.energy_fighterA < 0) s.energy_fighterA = 0;
                } else {
                    s.energy_fighterB -= t.energy_cost;
                    if (t.type == "Defensiva" && success) { // 🔴 Solo si es exitosa
                        s.energy_fighterB += t.energy_gain;
                        if (s.energy_fighterB > 100) s.energy_fighterB = 100;
                    }
                    if (s.energy_fighterB < 0) s.energy_fighterB = 0;
                }

                if (success)
                {
                    s.global_state = t.to_state_success;
                    s.last_success = true;

                    // 🔴 Asignar puntos SOLO si es técnica ofensiva exitosa
                    if (t.type == "Ofensiva" || t.category == "Raspado")
                    {
                        int points_earned = calculatePoints(t, act.fighter_id);
                        if (act.fighter_id == "fighterA")
                        {
                            s.points_fighterA += points_earned;
                            std::cout << "[DEBUG][ENGINE] 🥊 PUNTOS +" << points_earned
                                      << " para fighterA (Total: " << s.points_fighterA << ")" << std::endl;
                        }
                        else
                        {
                            s.points_fighterB += points_earned;
                            std::cout << "[DEBUG][ENGINE] 🥊 PUNTOS +" << points_earned
                                      << " para fighterB (Total: " << s.points_fighterB << ")" << std::endl;
                        }
                    }
                    else
                    {
                        // Técnica defensiva o neutra exitosa - no da puntos
                        std::cout << "[DEBUG][ENGINE] Técnica " << t.type
                                  << " exitosa - sin puntos" << std::endl;
                    }

                    std::cout << "[DEBUG][ENGINE] ÉXITO -> " << s.global_state
                              << " (Energía A: " << s.energy_fighterA
                              << ", B: " << s.energy_fighterB << ")" << std::endl;
                }
                else
                {
                    s.global_state = t.to_state_fail;
                    s.last_success = false;
                    
                    // 🔴 IMPORTANTE: Si falla una técnica defensiva, no debería 
                    // haber recuperado energía (el luchador ya gastó energía)
                    std::cout << "[DEBUG][ENGINE] FALLO -> " << s.global_state
                              << " (Energía A: " << s.energy_fighterA
                              << ", B: " << s.energy_fighterB << ")" << std::endl;
                }

                s.exchanges++;
                s.last_actor = act.fighter_id;

                // 🔴 ACTUALIZAR TIEMPOS DE PENSAMIENTO BASADO EN ENERGÍA
                s.updateThinkTimes();

                // 🔴 DETERMINAR PRÓXIMO ATACANTE BASADO EN QUIÉN PIENSA MÁS RÁPIDO
                int my_energy = (act.fighter_id == "fighterA") ? s.energy_fighterA : s.energy_fighterB;
                int opp_energy = (act.fighter_id == "fighterA") ? s.energy_fighterB : s.energy_fighterA;
                double my_think_time = (act.fighter_id == "fighterA") ? s.think_time_A : s.think_time_B;
                double opp_think_time = (act.fighter_id == "fighterA") ? s.think_time_B : s.think_time_A;

                if (success)
                {
                    // Éxito: tengo ventaja
                    if (my_energy > opp_energy * 0.7 && my_think_time < opp_think_time * 1.2)
                    {
                        // Mantengo la iniciativa si tengo ventaja energética y mental
                        s.current_actor = act.fighter_id;
                        std::cout << "[DEBUG][ENGINE] " << act.fighter_id
                                  << " mantiene iniciativa (ventaja energética/mental)" << std::endl;
                    }
                    else
                    {
                        // Pierdo la iniciativa
                        s.current_actor = (act.fighter_id == "fighterA") ? "fighterB" : "fighterA";
                        std::cout << "[DEBUG][ENGINE] " << act.fighter_id
                                  << " pierde iniciativa (oponente recupera)" << std::endl;
                    }
                }
                else
                {
                    // Falla: probablemente pierdo iniciativa
                    if (rand() % 100 < 70)
                    { // 70% chance de perder iniciativa
                        s.current_actor = (act.fighter_id == "fighterA") ? "fighterB" : "fighterA";
                        std::cout << "[DEBUG][ENGINE] Falla -> " << act.fighter_id
                                  << " pierde iniciativa" << std::endl;
                    }
                    else
                    {
                        s.current_actor = act.fighter_id;
                        std::cout << "[DEBUG][ENGINE] Falla pero " << act.fighter_id
                                  << " mantiene iniciativa" << std::endl;
                    }
                }

                std::cout << "[DEBUG][ENGINE] ROLES: A=" << s.actor_A
                          << "(" << s.energy_fighterA << "%," << s.points_fighterA << "pts, pensar:" << s.think_time_A << "s)"
                          << ", O=" << s.actor_O
                          << "(" << s.energy_fighterB << "%," << s.points_fighterB << "pts, pensar:" << s.think_time_B << "s)" << std::endl;
                std::cout << "[DEBUG][ENGINE] Tiempo total: " << s.total_time << "/" << s.MAX_FIGHT_TIME << "s" << std::endl;

                // 🔴 Verificar condiciones de victoria
                checkVictoryConditions(s);

                if (s.finished)
                {
                    s.current_actor = "none";
                    return;
                }
            }
        }
        else
        {
            // 🔴 INICIAL: Mostrar configuración inicial
            std::cout << "[DEBUG][ENGINE] Inicio. ROLES: A=" << s.actor_A
                      << ", O=" << s.actor_O << std::endl;
            std::cout << "[DEBUG][ENGINE] Energía inicial: A=100%, B=100%" << std::endl;
            std::cout << "[DEBUG][ENGINE] Puntos iniciales: A=0, B=0" << std::endl;
            std::cout << "[DEBUG][ENGINE] Tiempo máximo: " << s.MAX_FIGHT_TIME << "s" << std::endl;
            std::cout << "[DEBUG][ENGINE] Primer atacante: " << s.current_actor
                      << " (piensa en " << ((s.current_actor == "fighterA") ? s.think_time_A : s.think_time_B) << "s)" << std::endl;
        }

        s.has_output = true;
    }

    // 🔴 Función para verificar todas las condiciones de victoria
    void checkVictoryConditions(FightEngineState &s) const
    {
        // 1. Verificar tiempo agotado
        if (s.total_time >= s.MAX_FIGHT_TIME)
        {
            s.finished = true;
            s.victory_type = FightEngineState::POINTS;

            if (s.points_fighterA > s.points_fighterB)
            {
                s.winner = "fighterA";
                std::cout << "[DEBUG][ENGINE] ⏰ TIEMPO AGOTADO! Ganador por puntos: fighterA"
                          << " (" << s.points_fighterA << " vs " << s.points_fighterB << ")" << std::endl;
            }
            else if (s.points_fighterB > s.points_fighterA)
            {
                s.winner = "fighterB";
                std::cout << "[DEBUG][ENGINE] ⏰ TIEMPO AGOTADO! Ganador por puntos: fighterB"
                          << " (" << s.points_fighterB << " vs " << s.points_fighterA << ")" << std::endl;
            }
            else
            {
                s.winner = "Empate";
                std::cout << "[DEBUG][ENGINE] ⏰ TIEMPO AGOTADO! EMPATE por puntos"
                          << " (" << s.points_fighterA << " vs " << s.points_fighterB << ")" << std::endl;
            }
            return;
        }

        // 2. Verificar sumisión
        if (s.global_state.find("Sumision") != std::string::npos ||
            s.global_state.find("Sumisión") != std::string::npos)
        {

            // 🔴 VERIFICAR QUE NO SEA MUY TEMPRANO
            if (s.total_time < 30)
            { // Muy temprano para sumisión real
                std::cout << "[DEBUG][ENGINE] Sumisión muy temprana (t=" << s.total_time
                          << "s) - probablemente escape" << std::endl;
                // Tratar como fallo de sumisión
                s.global_state = "Pos(A:De_Pie, O:De_Pie)"; // Reset a posición neutral
                s.last_success = false;
                return;
            }

            s.finished = true;
            s.victory_type = FightEngineState::SUBMISSION;

            // Determinar quién aplicó la sumisión
            if (s.last_success && s.last_actor == "fighterA")
            {
                s.winner = "fighterA";
                std::cout << "[DEBUG][ENGINE] 🏆 VICTORIA POR SUMISIÓN! Ganador: fighterA (t="
                          << s.total_time << "s)" << std::endl;
            }
            else if (s.last_success && s.last_actor == "fighterB")
            {
                s.winner = "fighterB";
                std::cout << "[DEBUG][ENGINE] 🏆 VICTORIA POR SUMISIÓN! Ganador: fighterB (t="
                          << s.total_time << "s)" << std::endl;
            }
            else
            {
                // Si la sumisión falló, no hay victoria aún
                s.finished = false;
                std::cout << "[DEBUG][ENGINE] Sumisión fallida - continúa el combate" << std::endl;
            }
            return;
        }

        // 3. Verificar KO (sin energía)
        if (s.energy_fighterA <= 0 && s.energy_fighterB <= 0)
        {
            s.finished = true;
            s.victory_type = FightEngineState::DOUBLE_KO;
            s.winner = "Empate";
            std::cout << "[DEBUG][ENGINE] 🥊 DOBLE KO! Ambos sin energía (t="
                      << s.total_time << "s)" << std::endl;
            return;
        }
        else if (s.energy_fighterA <= 0)
        {
            s.finished = true;
            s.victory_type = FightEngineState::KO;
            s.winner = "fighterB";
            std::cout << "[DEBUG][ENGINE] 🥊 KO! fighterA sin energía. Ganador: fighterB (t="
                      << s.total_time << "s)" << std::endl;
            return;
        }
        else if (s.energy_fighterB <= 0)
        {
            s.finished = true;
            s.victory_type = FightEngineState::KO;
            s.winner = "fighterA";
            std::cout << "[DEBUG][ENGINE] 🥊 KO! fighterB sin energía. Ganador: fighterA (t="
                      << s.total_time << "s)" << std::endl;
            return;
        }

        // 4. Verificar diferencia de puntos muy grande (victoria técnica)
        int point_difference = std::abs(s.points_fighterA - s.points_fighterB);
        if (point_difference >= 20 && s.total_time > s.MAX_FIGHT_TIME / 2)
        {
            s.finished = true;
            s.victory_type = FightEngineState::POINTS;

            if (s.points_fighterA > s.points_fighterB)
            {
                s.winner = "fighterA";
                std::cout << "[DEBUG][ENGINE] 🏆 VICTORIA TÉCNICA! fighterA lidera por "
                          << point_difference << " puntos (t=" << s.total_time << "s)" << std::endl;
            }
            else
            {
                s.winner = "fighterB";
                std::cout << "[DEBUG][ENGINE] 🏆 VICTORIA TÉCNICA! fighterB lidera por "
                          << point_difference << " puntos (t=" << s.total_time << "s)" << std::endl;
            }
            return;
        }
    }

    void internalTransition(FightEngineState &s) const override
    {
        s.has_output = false;
    }

    void output(const FightEngineState &s) const override
    {
        if (!s.has_output)
            return;

        // 🔴 Usar el mapeo dinámico de roles
        std::string estado_para_A = convertRoles(s.global_state, s.actor_A, s.actor_O);
        std::string estado_para_B = convertRoles(s.global_state, s.actor_A, s.actor_O);

        // Calcular puntos para cada luchador según su rol actual
        int points_A = (s.actor_A == "fighterA") ? s.points_fighterA : s.points_fighterB;
        int points_B = (s.actor_O == "fighterB") ? s.points_fighterB : s.points_fighterA;

        double time_remaining = s.MAX_FIGHT_TIME - s.total_time;
        if (time_remaining < 0)
            time_remaining = 0;

        // FighterA siempre recibe su perspectiva
        FightUpdate msgA(
            s.current_actor,
            estado_para_A,
            estado_para_A,
            s.last_success,
            s.finished);
        msgA.my_points = points_A;
        msgA.opp_points = points_B;
        msgA.time_remaining = time_remaining;

        // FighterB siempre recibe su perspectiva
        FightUpdate msgB(
            s.current_actor,
            estado_para_B,
            estado_para_B,
            s.last_success,
            s.finished);
        msgB.my_points = points_B;
        msgB.opp_points = points_A;
        msgB.time_remaining = time_remaining;

        outA->addMessage(msgA);
        outB->addMessage(msgB);

        std::cout << "Enviando update. Actor: " << s.current_actor
                  << " | Estado: " << s.global_state << std::endl;
        std::cout << "ROLES: A=" << s.actor_A << " O=" << s.actor_O << std::endl;
        std::cout << "PUNTOS: A=" << s.points_fighterA << " B=" << s.points_fighterB << std::endl;
        std::cout << "TIEMPO: " << s.total_time << "/" << s.MAX_FIGHT_TIME
                  << "s (Restante: " << time_remaining << "s)" << std::endl;
        std::cout << "T. PENSAR: A=" << s.think_time_A << "s B=" << s.think_time_B << "s" << std::endl;
                        std::cout << "Próximo atacante: " << s.current_actor
                          << " (piensa en " << ((s.current_actor == "fighterA") ? s.think_time_A : s.think_time_B) << "s)" << std::endl;
    }

    double timeAdvance(const FightEngineState &s) const override
    {
        return s.has_output ? 0.0 : std::numeric_limits<double>::infinity();
    }
};

#endif
