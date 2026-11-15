#include "game.hpp"
namespace mission {
    Totals Resources(const GameState& g){
        Totals t;
        t.cost=crafts[g.craft].cost;
        t.mass=crafts[g.craft].dryMass;
        for(int i=0;i<4;++i) if(g.team[i])t.cost+=salaries[i];
        if(g.rocket>=0)t.cost+=rockets[g.rocket].cost;
        for(int p:g.installed)if(p>=0){
            const auto& c=parts[p];
            t.cost+=c.cost;
            t.mass+=c.mass;
            ++t.used;
            t.generation+=c.generation;
            t.load+=c.consumption;
            t.battery+=c.battery;
            t.storage+=c.storage;
            t.science+=c.science;
            t.radio+=c.radio;
        }
        return t;
    }
    bool Has(const GameState& g,int p){
        return std::find(g.installed.begin(),g.installed.end(),p)!=g.installed.end();
    }
    std::string BuildIssue(const GameState& g){
        auto t=Resources(g);
        if(t.cost>kBudget)return "Over budget: remove equipment or choose a cheaper rocket.";
        if(t.mass-crafts[g.craft].dryMass>crafts[g.craft].mass)return "Payload too heavy: remove equipment.";
        if(t.used>crafts[g.craft].slots)return "Too many modules for this chassis.";
        if(t.science<=0)return "Attach a camera or laser altimeter.";
        if(t.radio<=0)return "Attach a communications antenna.";
        if(t.battery<=0)return "Attach a battery pack for eclipse.";
        if(t.generation<t.load)return "Add solar generation or remove power-hungry modules.";
        return "";
    }
    std::string Readiness(const GameState& g){
        auto issue=BuildIssue(g);
        if(!issue.empty())return issue;
        if(g.rocket<0)return "Choose a rocket.";
        if(Resources(g).mass>rockets[g.rocket].capacity)return "Rocket cannot deliver total spacecraft mass.";
        if(g.orbit<0)return "Choose a lunar orbit.";
        if(g.date<0||!SelectedWeather(g).Valid())return "Choose a favorable launch date.";
        return "";
    }

}
