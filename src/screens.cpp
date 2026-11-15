#include "game.hpp"
namespace mission {
    void Screen(GameState& g,const Input& in,const AudioFeedback& a,float time){
        auto t=Resources(g);
        auto& f=g.flight;
        if(g.stage==0){
            DrawCircleGradient({1200,1150},580,{42,135,187,255},{12,40,77,255});
            DrawEllipse(1150,750,120,35,{65,170,145,255}); Ship({1220,450},1.2f,g.craft,time);
            auto title=Typography().title;
            for(int row=0;row<2;++row){const char* word=row==0?"DEAD CODERS":"SPACE MISSION";
                float width=MeasureTextEx(title,word,70,2).x;
                DrawTextEx(title,word,{800-width/2,185.0f+row*90},70,2,row==0?kText:kAccent);}
            Text("Build. Launch. Explore.",570,385,28,kGold);
            if(Button({180,465,330,65},"PLAY",in,true,true))Next(g,a);
            if(Button({180,550,330,65},"SETTINGS",in))g.overlay=2;
            if(Button({180,635,330,65},"INFO",in))g.overlay=1;
            if(Button({180,720,155,50},"ARCHIVE",in)){g.reportText.clear();g.reportSaved=true;g.reportScroll=0;Go(g,10,a);}
            if(Button({355,720,155,50},"EXIT",in))g.exitRequested=true;
            return;
        }
        if(g.stage==1){
            Text("CHOOSE YOUR NEXT FRONTIER",70,180,22,kMuted);
            for(int i=0;i<3;++i){
                float x=70+i*495.0f;Rectangle card{x,230,470,480};bool hover=Hovered(card,in);
                Panel(card,i==0?Color{31,27,23,255}:kPanel,i==0&&g.moon?kAccent:hover?kMuted:kBorder);
                Text(TextFormat("0%d / %s",i+1,i==0?"AVAILABLE":"LOCKED"),x+26,252,20,i==0?kAccent:kMuted);
                Vector2 p{x+235,425};float radius=i==0?112:96;
                DrawCircleGradient(p,radius+30,Fade(i==0?kAccent:kMuted,0.12f),BLANK);
                Moon(p,radius,i==0?Color{200,197,184,255}:i==1?Color{174,91,63,255}:Color{178,145,109,255});
                if(i==2)for(int j=-3;j<=3;++j){float dy=j*23.0f;float width=std::sqrt(radius*radius-dy*dy);DrawLineEx({p.x-width,p.y+dy},{p.x+width,p.y+dy},9,Fade({113,78,55,255},0.4f));}
                if(i>0){DrawRectangleRounded({p.x-14,p.y-8,28,24},0.2f,6,kBackground);DrawCircleLines(int(p.x),int(p.y-12),12,kBackground);}
                Text(i==0?"THE MOON":i==1?"MARS":"JUPITER",x+26,556,34,i==0?kText:kMuted);
                Text(i==0?"Surface mapping / Easy mission":"Future expedition",x+26,602,22,kMuted);
                if(Button({x+25,647,420,46},i==0?(g.moon?"MISSION SELECTED":"SELECT LUNAR MISSION"):"UNAVAILABLE",in,i==0,g.moon&&i==0)){g.moon=true;a.Play(g);}
                if(hover&&i>0)Text("This destination is not yet playable.",x+26,316,20,kDanger);
            }
            Text("BUDGET  "+Money(kBudget)+"   /   10 observations   /   Return 21 data units",80,750,25,kGold);
            Text("Orbit the Moon, study the surface and bring the science home.",80,788,22,kMuted);
            return;
        }
        if(g.stage==2){
            Ship({
                510,450}
            ,2.5f,g.craft,time);
            for(int i=0;i<3;++i){
                auto c=crafts[i];
                if(Button({
                    1040,float(230+i*145),460,65}
                ,(std::string(c.name)+(g.craft==i?" / SELECTED":"")).c_str(),in)){
                    GameState trial=g;
                    trial.craft=i;
                    bool fits=true;
                    for(int j=c.slots;j<11;++j)if(g.installed[j]>=0)fits=false;
                    auto r=Resources(trial);
                    if(!fits||r.used>c.slots||r.mass-c.dryMass>c.mass||r.cost>kBudget)Notice(g,"Remove modules or reduce spending before changing chassis.");
                    else{
                        g.craft=i;
                        a.Play(g);
                    }
                }
                Text(TextFormat("%d kg payload / %d slots / $%d",c.mass,c.slots,c.cost),1040,310+i*145,22,kMuted);
            }
            Text("Robotic chassis: computer and pointing control included.",90,710,24);
            Text("Larger capacity costs more. Solar generation comes from installed panels.",90,750,22,kMuted);
            return;
        }
        if(g.stage==3){
            for(int i=0;i<4;++i){
                float x=95+i*375;
                float y=330+std::sin(time+i)*5;
                DrawCircleV({
                    x+115,y}
                ,65,kPanel);
                DrawCircleV({
                    x+115,y-5}
                ,42,{
                    224,179,150,255}
                );
                DrawCircleV({
                    x+100,y-10}
                ,5,kBackground);
                DrawCircleV({
                    x+129,y-10}
                ,5,kBackground);
                DrawLine(int(x+100),int(y+19),int(x+130),int(y+19),kBackground);
                DrawCircleLines(int(x+115),int(y),55,i<4&&g.team[i]?kAccent:kGold);
                WrappedText(names[i],x,430,260,27,kText);
                WrappedText(abilities[i],x,515,260,22,kMuted);
                if(Button({
                    x,640,260,60}
                ,i==3?"Not needed":g.team[i]?"Release / refund":TextFormat("Hire / $%d",salaries[i]),in,i<3)){
                    if(g.team[i])g.team[i]=false;
                    else if(std::count(g.team.begin(),g.team.end(),true)>=3)Notice(g,"Ground team limit: three specialists.");
                    else if(t.cost+salaries[i]>kBudget)Notice(g,"Not enough money.");
                    else g.team[i]=true;
                    a.Play(g);
                }
            }
            Text("Your team works on Earth. Hiring is optional; each specialist unlocks useful recovery choices.",90,750,23,kAccent);
            return;
        }
        if(g.stage==4){
            if(in.enabled && CheckCollisionPointRec(in.mouse,{55,235,520,505}))g.payloadScroll=Clamp(g.payloadScroll-GetMouseWheelMove(),0,5);
            Scrollbar({555,245,14,465},g.payloadScroll,5,5,in);g.page=int(std::round(g.payloadScroll));
            Text("EQUIPMENT / scroll",70,190,25,kGold);
            for(int j=0;j<5;++j){int id=g.page+j;float y=245+j*100;int count=int(std::count(g.installed.begin(),g.installed.end(),id));
                Text(parts[id].name,70,y,23,count?kAccent:kText);
                Text(TextFormat("$%d / %d kg / installed %d",parts[id].cost,parts[id].mass,count),70,y+32,19,kMuted);
                if(Button({420,y,50,55},"+",in)){auto slot=std::find(g.installed.begin(),g.installed.begin()+crafts[g.craft].slots,-1);
                    if(slot==g.installed.begin()+crafts[g.craft].slots)Notice(g,"No free slots. Remove a module first.");
                    else if(t.cost+parts[id].cost>kBudget||t.mass-crafts[g.craft].dryMass+parts[id].mass>crafts[g.craft].mass)Notice(g,"Too expensive or too heavy for this ship.");
                    else{*slot=id;a.Play(g,true);Notice(g,"Equipment installed. Resources updated.",false);}}
                if(Button({480,y,50,55},"-",in,count>0)){*std::find(g.installed.begin(),g.installed.end(),id)=-1;a.Play(g);}
                if(InfoButton({380,y+12,26,26},in))Notice(g,parts[id].tip,false); Tooltip(in,{70,y,340,70},parts[id].tip);
            }
            if(Button({70,760,170,45},"UP",in,g.page>0))g.payloadScroll=std::max(0.0f,g.payloadScroll-1);
            if(Button({260,760,170,45},"DOWN",in,g.page<5))g.payloadScroll=std::min(5.0f,g.payloadScroll+1);
            Ship({800,460},1.45f,g.craft,0);
            Text(crafts[g.craft].name,710,650,32,kAccent);
            Panel({1080,205,450,580});Text("READINESS",1110,240,30,kGold);
            t=Resources(g);
            const char* labels[]={"Science instrument","Antenna","Battery","Enough solar power"};
            bool checks[]={t.science>0,t.radio>0,t.battery>0,t.generation>=t.load};
            for(int i=0;i<4;++i)Text(std::string(checks[i]?"OK  ":"NEED  ")+labels[i],1110,300+i*45,22,checks[i]?kAccent:kDanger);
            Text(TextFormat("Payload %d / %d kg",t.mass-crafts[g.craft].dryMass,crafts[g.craft].mass),1110,520,22);
            Text(TextFormat("Slots %d / %d",t.used,crafts[g.craft].slots),1110,560,22);
            Text(TextFormat("Power %.0f / %.0f W",t.generation,t.load),1110,600,22);
            Text(TextFormat("Battery %.0f Wh",t.battery),1110,640,22);
            Text(TextFormat("Storage %.0f data",t.storage),1110,680,22);
            return;
        }
        if(g.stage==5){
            for(int i=0;i<3;++i){
                float x=310+i*480;
                RocketArt({
                    x,430}
                ,i,g.rocket==i?45+std::sin(time*20)*10:0);
                if(Button({
                    x-190,610,380,60}
                ,rockets[i].name,in,true,g.rocket==i)){
                    int cost=t.cost-(g.rocket>=0?rockets[g.rocket].cost:0)+rockets[i].cost;
                    if(t.mass>rockets[i].capacity)Notice(g,"Not ready: total spacecraft mass exceeds lunar capacity.");
                    else if(cost>kBudget)Notice(g,"Not enough money for this rocket.");
                    else{
                        g.rocket=i;
                        a.Play(g);
                    }
                }
                Text(TextFormat("$%d / %d kg to Moon",rockets[i].cost,rockets[i].capacity),x-170,690,24,kMuted);
            }
            Text(TextFormat("Actual launch mass: %d kg (chassis + all installed equipment)",t.mass),90,760,24,kAccent);
            return;
        }
        if(g.stage==6){
            Moon({
                520,450}
            ,135);
            for(int i=0;i<2;++i){
                float r=220+i*100;
                DrawCircleLines(520,450,r,g.orbit==i?kAccent:kBorder);
                Vector2 p{
                    520+std::cos(time*0.4f)*r,450+std::sin(time*0.4f)*r};
                if(g.orbit==i)Ship(p,0.4f,g.craft,0);
                if(in.enabled&&in.clicked&&std::fabs(Vector2Distance(in.mouse,{
                    520,450}
                )-r)<18){
                    in.clicked=false;
                    g.orbit=i;
                    a.Play(g);
                }
                if(Button({
                    1020,float(300+i*180),470,65}
                ,i==0?"LOW LUNAR ORBIT":"HIGH LUNAR ORBIT",in,true,g.orbit==i)){
                    g.orbit=i;
                    a.Play(g);
                }
                WrappedText(i==0?"Finer surface detail. Shorter Earth contact windows.":"Wider coverage. More contact time; coarser surface detail.",1020,385+i*180,460,24,kMuted);
            }
            Text("Simplified trajectory illustration; not a physical orbital simulation.",90,790,22,kMuted);
            return;
        }
        if(g.stage==7){
            constexpr const char* months[]={"JANUARY","FEBRUARY","MARCH","APRIL","MAY","JUNE","JULY","AUGUST","SEPTEMBER","OCTOBER","NOVEMBER","DECEMBER"};
            Panel({60,190,940,605});
            if(Button({80,210,95,45},"<",in)){if(--g.calendarMonth<1){g.calendarMonth=12;--g.calendarYear;}g.calendarYear=std::max(1900,g.calendarYear);}
            Text(std::string(months[g.calendarMonth-1])+" "+std::to_string(g.calendarYear),245,215,32,kAccent);
            if(Button({875,210,95,45},">",in)){if(++g.calendarMonth>12){g.calendarMonth=1;++g.calendarYear;}g.calendarYear=std::min(2200,g.calendarYear);}
            constexpr const char* days[]={"MON","TUE","WED","THU","FRI","SAT","SUN"};
            for(int i=0;i<7;++i)Text(days[i],103+i*128,280,20,kMuted);
            int first=FirstWeekday(g.calendarYear,g.calendarMonth),count=DaysInMonth(g.calendarYear,g.calendarMonth);
            for(int day=1;day<=count;++day){
                int cell=first+day-1;Rectangle box{80.0f+(cell%7)*128,317.0f+(cell/7)*73,118,65};
                auto weather=WeatherFor(g.calendarYear,g.calendarMonth,day);
                bool selected=g.date==day-1&&g.launchYear==g.calendarYear&&g.launchMonth==g.calendarMonth;
                bool hover=Hovered(box,in);
                Panel(box,selected?Color{65,42,25,255}:Color{34,32,30,255},selected?kAccent:hover?kMuted:kBorder);
                Text(std::to_string(day),box.x+10,box.y+5,24,selected?kAccent:kText);
                WeatherIcon({box.x+89,box.y+25},weather,0.65f);
                Text(!weather.Valid()?"BLOCKED":weather.delay?"CAUTION":"CLEAR",box.x+8,box.y+43,15,!weather.Valid()?kDanger:weather.delay?kGold:kSuccess);
                if(hover){SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);if(in.clicked){in.clicked=false;g.date=day-1;g.launchYear=g.calendarYear;g.launchMonth=g.calendarMonth;a.Play(g);}}
            }
            Text("REAL CALENDAR / SIMULATED WEATHER",90,770,19,kMuted);
            Panel({1030,190,510,605});
            Text(g.date<0?"SELECT A DATE":DateLabel(g),1060,220,32,kGold);
            if(g.date>=0){auto weather=SelectedWeather(g);WeatherIcon({1475,250},weather,1.3f);
                Text(TextFormat("Wind  %d km/h",weather.windKph),1060,290,25);
                Text(weather.lightning?"Lightning: HIGH RISK":"Lightning: low risk",1060,332,23,weather.lightning?kDanger:kMuted);
                Text(weather.transfer?"Transfer window: OPEN":"Transfer window: CLOSED",1060,374,23,weather.transfer?kSuccess:kDanger);
                WrappedText(weather.Summary(),1060,430,445,22,kMuted);
            }
            auto issue=Readiness(g);
            DrawLine(1060,525,1510,525,kBorder);
            Text(issue.empty()?"READY FOR LAUNCH":"LAUNCH BLOCKED",1060,550,28,issue.empty()?kSuccess:kDanger);
            WrappedText(issue.empty()?"All engineering checks passed. Confirm to begin countdown.":issue,1060,595,445,23,issue.empty()?kText:kDanger);
            Text("Selected date stays saved when browsing months.",1060,733,17,kMuted);
            return;
        }
        if(g.stage==8){
            const char* title=f.launch<5?TextFormat("T - %d",5-int(f.launch)):f.launch<8?"IGNITION / LIFTOFF":f.launch<11?"STAGE SEPARATION":f.launch<15?"LUNAR TRANSFER":"ORBIT INSERTION COMPLETE";
            Text(title,120,220,42,kAccent);
            if(f.launch<11){
                float y=600-fmaxf(0,f.launch-5)*65;
                RocketArt({
                    700,y}
                ,g.rocket,f.launch>5?90+std::sin(time*40)*25:0);
                if(f.launch>9)DrawRectangle(750,int(y+130+(f.launch-9)*80),35,65,kMuted);
                DrawRectangle(0,770,1600,130,kPanel);
                for(int i=0;i<20&&f.launch>5;++i)DrawCircleV({
                    700+std::sin(i*7.0f+time)*80,720+float(i%4)*15}
                ,10+float(i%5)*4,Fade(kMuted,0.2f));
            }
            else{
                Moon({
                    1150,420}
                ,160);
                Ship({
                    350+(f.launch-11)*120,450}
                ,1,g.craft,time);
            }
            if(Button({
                1250,720,250,65}
            ,"SKIP TO ORBIT",in)){
                f.energy=fmaxf(0,t.battery-SelectedWeather(g).delay);
                Go(g,9,a);
            }
            return;
        }
        if(g.stage==9){
            float phase=std::fmod(f.time,24.0f)/24;
            bool link=phase<(g.orbit==0?0.58f:0.76f)&&f.radioFault<=0;
            DrawCircleV({780,2400},1830,{89,84,77,255});
            for(int i=0;i<16;++i){float x=std::fmod(i*180.0f-f.time*30+5000,1900.0f)-150;DrawEllipse(int(x),int(660+(i%3)*65),40,12,{58,56,53,255});}
            Ship({795,440},1.9f,g.craft,0);
            if(f.camera&&f.mode!=2&&f.heat<=0&&f.pointing<=0&&f.energy>0)DrawLineEx({795,460},{900,700},3,Fade(kAccent,0.5f));
            Panel({60,190,460,113});
            Text("SIMULATION REMAINING",85,205,20,kMuted);Text(Clock(kOperationSeconds-f.time),85,238,42,kOperationSeconds-f.time<20?kDanger:kAccent);
            Panel({545,190,490,113});Text("ACCELERATED MISSION CLOCK",570,205,20,kMuted);Text(Clock(f.time*kMissionHoursPerSecond*3600),570,238,42,kText);
            Panel({1060,190,480,113});Text("SCIENCE RETURNED / HEALTH",1085,205,20,kMuted);Text(TextFormat("%.1f / 21     %.0f%%",f.returned,f.health),1085,248,30,f.health<35?kDanger:kText);
            Panel({60,330,380,460});
            Text("FLIGHT STATUS",85,355,25,kGold);
            Text(link?"EARTH LINK: AVAILABLE":"EARTH LINK: BLOCKED",85,407,20,link?kSuccess:kDanger);
            Text(phase<0.7f?"SUNLIGHT / CHARGING":"ECLIPSE / BATTERY",85,448,20,phase<0.7f?kMuted:kGold);
            Text(TextFormat("Observations %.1f / 10",f.observations),85,490,23,kText);
            Text("Elapsed: "+Clock(f.time),85,533,22,kMuted);
            Meter("Battery / Wh",f.energy,t.battery,85,585,f.energy<t.battery*0.2f?kDanger:kAccent);
            Text(g.paused?"PAUSED":"Time factor: x"+std::to_string(g.speed),85,710,25,kGold);
            Panel({1110,330,430,460});
            Text("MISSION CONTROL",1135,355,25,kGold);
            if(Button({1135,405,380,55},f.camera?"SCIENCE ON":"SCIENCE OFF",in,true,f.camera))f.camera=!f.camera;
            if(Button({1135,478,380,55},f.transmit?"TRANSMIT ON":"TRANSMIT OFF",in,true,f.transmit))f.transmit=!f.transmit;
            if(Button({1135,551,380,55},f.mode==0?"MODE: SCIENCE":f.mode==1?"MODE: BALANCED":"MODE: SAFE",in))f.mode=(f.mode+1)%3;
            Meter("Storage / data",f.stored,t.storage,1135,638,f.stored>t.storage*0.85f?kDanger:kGold);
            if(Button({1135,735,180,40},g.paused?"RESUME":"PAUSE",in))g.paused=!g.paused;
            if(Button({1330,735,185,40},"END MISSION",in))Go(g,10,a);
            if(Button({640,732,310,55},TextFormat("TIME SPEED x%d",g.speed),in))g.speed=g.speed==1?3:g.speed==3?5:1;
            Text("Simplified model / 1 simulation second = 4.8 mission minutes",490,798,18,kMuted);
            return;
        }
        if(g.stage==10){ DrawReport(g,in,a); return; }
    }
    void Overlay(GameState& g,const Input& input,const AudioFeedback& a){
        DrawRectangle(0,0,1600,900,Fade(BLACK,0.8f));
        Panel({
            240,180,1120,560}
        ,kPanel,kGold);
        if(g.flight.event>=0&&g.stage==9){
            auto& f=g.flight;
            constexpr const char* titles[]={
                "SOLAR POWER SHORTAGE","COMMUNICATIONS MALFUNCTION","INSTRUMENT OVERHEATING","STORAGE PRESSURE","ATTITUDE CONTROL FAULT"};
            constexpr const char* tips[]={
                "Solar output is reduced. Batteries supply backup energy.","Your radio is offline. Stored data remains onboard.","Science is paused while the instrument cools.","Storage is limited. Decide which observations to keep.","Pointing is unreliable. Science needs stable orientation."};
            Text(titles[f.event],280,225,38,kDanger);
            WrappedText(tips[f.event],280,300,1000,26,kText);
            const char* options[3];
            bool enabled=true;
            switch(f.event){
                case 0:options[0]="Stop science";
                options[1]="Enter safe mode";
                options[2]=g.team[0]?"Engineer: restore panels":"Continue / damage risk";
                break;
                case 1:options[0]=g.team[2]?"Specialist: restore radio":"Remote recovery / 8 seconds";
                options[1]="Use backup computer radio";
                options[2]="Stop transmitting and wait";
                break;
                case 2:options[0]="Pause and cool instrument";
                options[1]="Reduce duty / cool briefly";
                options[2]="Continue / lose 15 health";
                break;
                case 3:options[0]="Stop collecting";
                options[1]=g.team[1]?"Scientist: retain best data":"Discard lower priority data";
                options[2]="Enable transmission";
                break;
                default:options[0]="Recalibrate / minor damage";
                options[1]="Suspend observations";
                options[2]="Specialist: restore pointing";
                break;
            }
            int event=f.event;
            for(int i=0;i<3;++i){
                enabled=!(event==1&&i==1&&!Has(g,9))&&!(event==4&&i==2&&!g.team[0]&&!g.team[3]);
                if(Button({
                    280,float(390+i*85),1040,65}
                ,options[i],input,enabled)){
                    EventChoice(g,i);
                    a.Play(g);
                    break;
                }
            }
            Text("Gray choices require the matching specialist or installed backup.",280,675,21,kMuted);
            return;
        }
        if(g.overlay==1){
            Text("DESIGN   >   LAUNCH   >   SURVIVE",280,230,40,kAccent);
            WrappedText("1. Select a spacecraft, hire a ground team and attach equipment.",280,340,1000,27,kText);
            WrappedText("2. Choose a rocket, orbit and safe simulated launch date.",280,420,1000,27,kText);
            WrappedText("3. Balance science, radio and battery. Return 21 data units and collect 10 observations in two minutes.",280,500,1000,27,kText);
        }
        if(g.overlay==2){
            Text("SETTINGS",280,230,40,kAccent);
            if(Button({
                280,340,480,65}
            ,g.soundEnabled?"SOUND: ON":"SOUND: OFF",input)){
                g.soundEnabled=!g.soundEnabled;
                a.Play(g);
            }
            if(Button({
                800,340,480,65}
            ,g.reducedMotion?"REDUCED MOTION: ON":"REDUCED MOTION: OFF",input))g.reducedMotion=!g.reducedMotion;
            Text("F11: fullscreen / Esc: pause or return / Settings last this session",280,470,24,kMuted);
        }
        if(g.overlay==3){
            Text("MISSION PAUSED",280,230,42,kAccent);
            if(Button({
                280,355,460,65}
            ,"RESUME",input))g.overlay=0;
            if(Button({
                800,355,460,65}
            ,"SETTINGS",input))g.overlay=2;
            if(Button({
                280,465,460,65}
            ,"MAIN MENU",input)) {
                g.overlay=0;
                Go(g,0,a);
            }
            if(Button({
                800,465,460,65}
            ,"EXIT",input))g.exitRequested=true;
        }
        if(g.overlay==4){
            Text("NASA INSIGHT / REAL PRINCIPLES",280,230,36,kGold);
            WrappedText("LRO uses instruments to investigate lunar terrain. Missions balance science objectives with spacecraft resources.",280,330,1000,27,kText);
            Text("Source: NASA Science / Lunar Reconnaissance Orbiter",280,445,24,kAccent);
            Text("science.nasa.gov/mission/lro/",280,490,23,kMuted);
            WrappedText("This game's hardware, prices, trajectories, weather and scores are fictional educational models, not verified NASA specifications.",280,545,1000,23,kMuted);
        }
        if(Button({
            1070,650,250,60}
        ,"CLOSE",input,true,true))g.overlay=0;
    }

}
