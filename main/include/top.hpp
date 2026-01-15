#ifndef TOP_HPP
#define TOP_HPP

#include "cadmium/modeling/devs/coupled.hpp"
#include "fight_engine.hpp"
#include "fighterA.hpp"
#include "fighterB.hpp"
#include "arbiter_smart.hpp"

using namespace cadmium;

struct topSystem : public Coupled {
    topSystem(const std::string& id) : Coupled(id) {
        auto engine = addComponent<fight_engine>("fight_engine");
        auto fA     = addComponent<fighterA>("fighterA");
        auto fB     = addComponent<fighterB>("fighterB");
        auto arbiter= addComponent<arbiter_smart>("arbiter");

        // Fighters → Arbiter
        addCoupling(fA->out, arbiter->inA);
        addCoupling(fB->out, arbiter->inB);
        
        // Arbiter → Engine
        addCoupling(arbiter->outAction, engine->in);

        // Engine → Fighters
        addCoupling(engine->outA, fA->in);
        addCoupling(engine->outB, fB->in);
    }
};

#endif