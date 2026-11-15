#include "game.hpp"
namespace mission {
    void Notice(GameState& g,const std::string& s,bool error){
        g.notice=s;g.noticeError=error;if(error&&uiAudio)uiAudio->Cue(g,0);
        g.noticeTime=4;
    }
    void Go(GameState& g,int stage,const AudioFeedback& a){
        if(g.stage==8&&stage!=8&&a.ready)StopSound(a.launch);
        if(stage==8){g.timeline.push_back("Launch: countdown and ascent started.");} if(stage==9){g.timeline.push_back("Orbit insertion complete: operations started.");} g.stage=stage; if(stage==10 && !g.reportSaved)SaveReport(g);
        g.transition=g.reducedMotion?0:0.25f;
        g.noticeTime=0;
        if(g.stage==8)a.Cue(g,1);else if(g.stage==10)a.Cue(g,2);else a.Play(g);
    }
    void StartFlight(GameState& g){
        g.flight=Flight{};
        g.flight.energy=Resources(g).battery;
        g.flight.events[0]=GetRandomValue(0,1)==0?0:2; g.flight.events[1]=g.flight.events[0]==0?2:0; g.timeline.clear(); g.reportSaved=false; g.reportText.clear(); g.reportScroll=0;
        g.paused=false;g.speed=1;
    }
    void Next(GameState& g,const AudioFeedback& a){
        std::string error;
        if(g.stage==1&&!g.moon)error="Select the Moon to accept the mission.";
        if(g.stage==4)error=BuildIssue(g);
        if(g.stage==5&&(g.rocket<0||Resources(g).mass>rockets[g.rocket].capacity))error="Choose a rocket that carries the whole spacecraft.";
        if(g.stage==6&&g.orbit<0)error="Select an orbit.";
        if(g.stage==7)error=Readiness(g);
        if(!error.empty()){
            Notice(g,error);
            return;
        }
        if(g.stage==7)StartFlight(g);
        if(g.stage<8)Go(g,g.stage+1,a);
    }
    void EventChoice(GameState& g,int choice){
        auto& f=g.flight;
        int e=f.event; g.timeline.push_back(std::string(TextFormat("Hour %.1f: issue %d, response %d applied.",f.time*kMissionHoursPerSecond,e+1,choice+1)));
        if(e==0){
            if(choice==0)f.camera=false;
            else if(choice==1)f.mode=2;
            else if(g.team[0])f.solarFault=0;
            else f.risk+=12;
        }
        if(e==1){
            if(choice==0){
                f.radioFault=g.team[2]?0:8;
                f.health-=g.team[2]?0:3;
            }
            else if(choice==1&&Has(g,9))f.radioFault=0;
            else f.transmit=false;
        }
        if(e==2){
            if(choice==0){
                f.camera=false;
                f.heat=0;
            }
            else if(choice==1){
                f.mode=1;
                f.heat=4;
            }
            else{
                f.heat=0;
                f.health-=15;
            }
        }
        if(e==3){
            if(choice==0)f.camera=false;
            else if(choice==1){
                float removed=f.stored*(g.team[1]?0.15f:0.3f);
                f.stored-=removed;
                f.health+=g.team[1]?2:0;
            }
            else f.transmit=true;
        }
        if(e==4){
            if(choice==0){
                f.pointing=5;
                f.health-=4;
            }
            else if(choice==1)f.camera=false;
            else if(g.team[3]||g.team[0])f.pointing=0;
        }
        f.health=Clamp(f.health,0,100);
        f.event=-1;
        Notice(g,"Command applied. Check your science and transmit controls.",false);
    }
    void Simulate(GameState& g,float dt,const AudioFeedback& a){
        auto& f=g.flight;
        if(g.stage!=9||g.paused||g.overlay||f.event>=0)return;
        auto t=Resources(g);
        dt=std::min(dt,kOperationSeconds-f.time); f.time+=dt;
        float phase=std::fmod(f.time,24.0f)/24.0f;
        bool sunlight=phase<0.70f,link=phase<(g.orbit==0?0.58f:0.76f);
        f.radioFault=fmaxf(0,f.radioFault-dt);
        f.solarFault=fmaxf(0,f.solarFault-dt);
        f.heat=fmaxf(0,f.heat-dt);
        f.pointing=fmaxf(0,f.pointing-dt);
        bool science=f.camera&&f.mode!=2&&f.heat<=0&&f.pointing<=0&&f.energy>0.1f&&(sunlight||Has(g,2));
        bool radio=f.transmit&&link&&f.radioFault<=0&&f.energy>0.1f;
        float duty=f.mode==0?1.0f:0.65f;
        float load=4;
        for(int id:g.installed) if(id>=0){
            const auto& part=parts[id];
            if(part.science>0){
                if(science)load+=part.consumption*duty;
            }
            else if(part.radio>0){
                if(radio)load+=part.consumption;
            }
            else load+=part.consumption;
        }
        // One real second equals 0.08 mission hours; watts times hours gives Wh.
        float hours=dt*kMissionHoursPerSecond;
        float generation=sunlight?(f.solarFault>0?t.generation*0.25f:t.generation):0;
        f.energy=Clamp(f.energy+(generation-load)*hours,0,t.battery);
        if(f.energy<=0){
            f.health-=dt*3;
            science=false;
            radio=false;
        }
        if(science){
            float data=fminf(t.storage-f.stored,t.science*duty*dt*(g.orbit==0?1:1.2f));
            f.stored+=data;
            f.generated+=data;
            f.observations+=data/3;
        }
        if(radio){
            float sent=fminf(f.stored,t.radio*dt*(g.team[2]?1.25f:1));
            f.stored-=sent;
            f.returned+=sent;
        }
        if(f.risk>0){
            f.health-=dt*0.5f;
            f.risk-=dt;
        }
        f.health=Clamp(f.health,0,100);
        if(f.nextEvent<2&&f.time>=35+f.nextEvent*45){
            f.event=f.events[f.nextEvent++];
            if(f.event==0)f.solarFault=18;
            if(f.event==1)f.radioFault=22;
            if(f.event==2)f.heat=16;
            if(f.event==4)f.pointing=18;
            a.Cue(g,0);
        }
        if(f.time>=kOperationSeconds||f.health<=0)Go(g,10,a);
    }
    int Score(const GameState& g){
        const auto& f=g.flight;
        auto t=Resources(g);
        int requirements=(f.observations>=10?1:0)+(f.returned>=21?1:0)+(f.health>0?1:0)+(t.cost<=kBudget?1:0);
        return int(40*Clamp(f.returned*(g.team[1]?1.15f:1)/50,0,1)+25*requirements/4.0f+20*f.health/100+15*(kBudget-t.cost)/kBudget);
    }

}
