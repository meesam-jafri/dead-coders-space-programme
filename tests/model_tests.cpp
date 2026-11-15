#include "game.hpp"
#include <cassert>
#include <iostream>
using namespace mission;
int main(){
    AudioFeedback silent;
    GameState empty;
    assert(!BuildIssue(empty).empty());
    for(int chassis=0;chassis<2;++chassis){
        GameState g;g.craft=chassis;g.moon=true;g.rocket=0;g.orbit=1;g.date=1;
        g.installed[0]=0;g.installed[1]=4;g.installed[2]=5;g.installed[3]=6;
        auto t=Resources(g);
        assert(t.mass==crafts[chassis].dryMass+40);
        assert(t.load==17 && t.generation==42 && t.battery==24);
        assert(Readiness(g).empty());
        g.date=0;assert(!Readiness(g).empty());g.date=1;
        auto original=g.installed;Go(g,3,silent);Go(g,4,silent);assert(g.installed==original);
        StartFlight(g);g.reportSaved=true;g.stage=9;
        for(int i=0;i<2600&&g.stage==9;++i){
            if(g.flight.event>=0){EventChoice(g,0);g.flight.camera=true;}
            Simulate(g,0.05f,silent);
            assert(g.flight.energy>=0&&g.flight.energy<=t.battery);
            assert(g.flight.stored>=0&&g.flight.stored<=t.storage);
            assert(g.flight.returned<=g.flight.generated+0.01f);
        }
        assert(g.stage==10&&g.flight.nextEvent<=2);
        assert(g.flight.observations>=10&&g.flight.returned>=21&&g.flight.health>0);
        assert(g.installed==original);
        if(chassis==0){g.reportSaved=false;SaveReport(g);assert(g.reportSaved);assert(g.reportText.find("SUCCESS")!=std::string::npos);assert(g.reportText.find("Scout")!=std::string::npos);}
    }
    std::cout<<"PASS: readiness, totals, navigation persistence, two successful builds, resource bounds, two-event cap, report saving.\n";
}