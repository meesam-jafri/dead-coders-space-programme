#include "game.hpp"
namespace mission {
    void Screen(GameState& g,const Input& in,const AudioFeedback& a,float time){
        auto t=Resources(g);
        auto& f=g.flight;
        if(g.stage==0){
            auto bg=Skin().menuBackground;
            if(bg.id){float scale=std::max(1600.0f/bg.width,900.0f/bg.height);DrawTexturePro(bg,{0,0,float(bg.width),float(bg.height)},{(1600-bg.width*scale)/2,(900-bg.height*scale)/2,bg.width*scale,bg.height*scale},{0,0},0,WHITE);}
            DrawRectangle(0,0,1600,900,Fade(kBackground,0.25f));
            auto earth=Skin().earth;
            if(earth.id)DrawTexturePro(earth,{0,0,float(earth.width),float(earth.height)},{990,470,730,730},{0,0},0,WHITE);
            auto title=Typography().title;
            for(int row=0;row<2;++row){const char* word=row==0?"DEAD CODERS":"SPACE MISSION";
                float width=MeasureTextEx(title,word,70,2).x;
                DrawTextEx(title,word,{800-width/2,185.0f+row*90},70,2,row==0?kText:kAccent);}
            Text("Build. Launch. Explore.",570,385,28,kGold);
            if(Button({180,455,400,72},"PLAY",in,true,true))Next(g,a);
            if(Button({180,635,400,65},"SETTINGS",in))g.overlay=2;
            if(Button({180,720,400,65},"INFO",in))g.overlay=1;
            if(Button({180,545,400,65},"PAST MISSIONS",in)){g.reportText.clear();g.reportSaved=true;g.reportScroll=0;Go(g,10,a);}

            return;
        }
        if(g.stage==1){
            Text("CHOOSE YOUR NEXT FRONTIER / HOVER TO EXPLORE",70,180,22,kMuted);
            static int focus=-1;static float expansion=0;
            auto cardBounds=[&](int i,float amount){return Rectangle{Lerp(70+i*495.0f,70,amount),230,Lerp(470,1460,amount),480};};
            float ease=expansion*expansion*(3-2*expansion);
            bool staying=focus>=0&&Hovered(cardBounds(focus,ease),in);
            if(focus<0){for(int i=0;i<3;++i)if(Hovered(cardBounds(i,0),in)){focus=i;staying=true;break;}}
            float step=GetFrameTime()/0.3f;
            expansion=in.reducedMotion?(staying?1.0f:0.0f):Clamp(expansion+(staying?step:-step),0,1);
            if(!staying&&expansion<=0)focus=-1;
            ease=expansion*expansion*(3-2*expansion);
            auto drawCard=[&](int i,bool expanded){
                Rectangle card=cardBounds(i,expanded?ease:0);
                if(focus>=0&&!expanded)card.x+=(i<focus?-1:1)*ease*300;
                float x=card.x;
                Panel(card,i==0&&g.moon?kSelected:kPanel,i==0&&g.moon?kAccent:kBorder);
                Text(TextFormat("0%d / %s",i+1,i==0?"AVAILABLE":"LOCKED"),x+26,252,20,i==0?kAccent:kMuted);
                Vector2 p{x+235,425};float radius=i==0?112:96;
                DrawCircleGradient(p,radius+30,Fade(i==0?Color{255,139,54,255}:kMoonColor,0.12f),BLANK);
                Moon(p,radius,i==0?Color{200,197,184,255}:i==1?Color{174,91,63,255}:Color{178,145,109,255});
                if(i==2)for(int j=-3;j<=3;++j){float dy=j*23.0f;float width=std::sqrt(radius*radius-dy*dy);DrawLineEx({p.x-width,p.y+dy},{p.x+width,p.y+dy},9,Fade({113,78,55,255},0.4f));}
                if(i>0){DrawRectangleRounded({p.x-14,p.y-8,28,24},0.2f,6,{12,12,14,255});DrawCircleLines(int(p.x),int(p.y-12),12,{12,12,14,255});}
                Text(i==0?"LUNAR RECON":i==1?"MARS EXPLORATION":"JUPITER EXPEDITION",x+26,556,29,kText);
                Text(i==0?"THE MOON / EASY":i==1?"MARS / MEDIUM":"JUPITER / HARD",x+26,602,22,kMuted);
                Input cardInput=in;cardInput.enabled=in.enabled&&(focus<0||expanded);
                if(Button({x+25,647,420,46},i==0?(g.moon?"MISSION SELECTED":"SELECT LUNAR MISSION"):"LOCKED / FUTURE MISSION",cardInput,i==0,!g.moon,g.moon&&i==0)){g.moon=true;in.clicked=false;a.Play(g);}
                if(expanded&&ease>0.62f){
                    BeginScissorMode(int(card.x),230,int(card.width),480);
                    float opacity=Clamp((ease-0.62f)/0.38f,0,1),tx=x+530;
                    Color normal=Fade(kText,opacity),accent=Fade(kAccent,opacity);
                    DrawLine(int(tx-35),285,int(tx-35),675,Fade(kBorder,opacity));
                    Text(i==0?"OUR CLOSEST NEIGHBOR":i==1?"THE RED PLANET":"THE GAS GIANT",tx,290,32,normal);
                    WrappedText(i==0?"Explore our closest celestial neighbor.":i==1?"Discover the secrets of the Red Planet.":"Venture into the giant planet's dangerous environment.",tx,345,830,26,normal);
                    Text(i==0?"OBJECTIVE":"CHALLENGE",tx,445,20,accent);
                    WrappedText(i==0?"Observe the lunar surface and return scientific data.":i==1?"Long-distance communication and limited resources.":"Radiation, distance, and power management.",tx,477,830,24,normal);
                    Text(i==0?"CHALLENGE":"DIFFICULTY",tx,545,20,accent);
                    Text(i==0?"Manage power, equipment, and communications.":i==1?"Medium / a longer journey":"Hard / an extreme environment",tx,577,23,normal);
                    Text(i==0?"REWARD / Complete your first space mission.":"COMING IN A FUTURE MISSION",tx,640,23,normal);
                    EndScissorMode();
                }
                if(focus>=0&&!expanded)DrawRectangleRec(card,Fade(kBackground,ease));
            };
            BeginScissorMode(60,225,1480,495);
            for(int i=0;i<3;++i)if(i!=focus&&ease<0.999f)drawCard(i,false);
            if(focus>=0)drawCard(focus,true);
            EndScissorMode();
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
                ,(std::string(c.name)+(g.craft==i?" / SELECTED":"")).c_str(),in,true,false,g.craft==i)){
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
                if(Skin().portraits[i].id){DrawCircleV({x+115,y},70,kPanel);auto portrait=Skin().portraits[i];DrawTexturePro(portrait,{0,0,float(portrait.width),float(portrait.height)},{x+40,y-75,150,150},{0,0},0,WHITE);DrawCircleLines(int(x+115),int(y),78,g.team[i]?kAccent:kBorder);}
                WrappedText(names[i],x,430,260,27,kText);
                WrappedText(abilities[i],x,515,260,22,kMuted);
                if(Button({
                    x,640,260,60}
                ,i==3?"Not needed":g.team[i]?"Release / refund":TextFormat("Hire / $%d",salaries[i]),in,i<3,false,g.team[i])){
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
            static int failedItem=-1;static float failure=0;
            failure=std::max(0.0f,failure-GetFrameTime());
            if(in.enabled && CheckCollisionPointRec(in.mouse,{55,235,520,505}))g.payloadScroll=Clamp(g.payloadScroll-GetMouseWheelMove(),0,5);
            Scrollbar({555,245,14,465},g.payloadScroll,5,5,in);g.page=int(std::round(g.payloadScroll));
            Text("EQUIPMENT / scroll",70,190,25,kGold);
            constexpr const char* education[]={
                "Science instruments are the mission payload. In this game, a camera collects surface data but needs power and storage.",
                "Choose instruments around your science goal. This fictional camera collects faster but uses more electricity and storage.",
                "An altimeter measures terrain height. Here it enables observations even during eclipse, using battery power.",
                "Solar panels turn sunlight into electricity. In shadow, the battery powers your ship. Add enough generation for the installed load.",
                "Solar panels turn sunlight into electricity. In shadow, the battery powers your ship. Add enough generation for the installed load.",
                "Batteries store energy for periods without solar power. Add reserve capacity to keep instruments running through shadow.",
                "Antennas support commands and data exchange with Earth. Here, science stays in storage until a contact window opens.",
                "Antennas support commands and data exchange with Earth. Here, a faster radio clears storage sooner but draws more power.",
                "Onboard storage holds observations until downlink. Here, full storage pauses collection; transmit data or add capacity.",
                "Redundancy provides a fallback if hardware fails. Here, a backup computer adds a radio recovery option at extra cost and mass."
            };
            for(int j=0;j<5;++j){int id=g.page+j;float y=245+j*100;int count=int(std::count(g.installed.begin(),g.installed.end(),id));
                float shake=(id==failedItem&&failure>0&&!g.reducedMotion)?std::sin(failure*80)*3*(failure/0.45f):0;
                if(count)DrawRectangleRounded({62+shake,y-5,480,76},0.08f,8,kSelected);
                if(id==failedItem&&failure>0)DrawRectangleRoundedLinesEx({62+shake,y-5,480,76},0.08f,8,2,Fade(kDanger,failure/0.45f));
                Text(parts[id].name,70+shake,y,23,kText);
                Text(TextFormat("$%d / %d kg",parts[id].cost,parts[id].mass),70+shake,y+32,19,kMuted);
                if(Button({420+shake,y,50,55},"+",in)){
                    auto current=Resources(g);
                    auto slot=std::find(g.installed.begin(),g.installed.begin()+crafts[g.craft].slots,-1);
                    std::string error;
                    if(slot==g.installed.begin()+crafts[g.craft].slots)error="NOT ENOUGH SLOTS / remove equipment first.";
                    else if(current.cost+parts[id].cost>kBudget)error="NOT ENOUGH BUDGET / remove equipment or choose a cheaper option.";
                    else if(current.mass-crafts[g.craft].dryMass+parts[id].mass>crafts[g.craft].mass)error="PAYLOAD TOO HEAVY / reduce mass before adding this item.";
                    if(!error.empty()){Notice(g,error);failedItem=id;failure=0.45f;}
                    else{*slot=id;a.Play(g,true);}
                }
                if(Button({480+shake,y,50,55},"-",in,count>0)){*std::find(g.installed.begin(),g.installed.end(),id)=-1;a.Play(g);}
                count=int(std::count(g.installed.begin(),g.installed.end(),id));
                if(count){DrawCircleV({467+shake,y+1},13,kAccent);Text(std::to_string(count),461+shake,y-10,19,kBackground);}
                InfoTip({380,y+12,26,26},in,parts[id].name,education[id]);
            }
            if(Button({70,760,170,45},"UP",in,g.page>0))g.payloadScroll=std::max(0.0f,g.payloadScroll-1);
            if(Button({260,760,170,45},"DOWN",in,g.page<5))g.payloadScroll=std::min(5.0f,g.payloadScroll+1);
            Ship({800,460},1.45f,g.craft,0);
            Text(crafts[g.craft].name,710,650,32,kAccent);
            Panel({1080,205,450,580});Text("MISSION READINESS",1105,235,27,kText);
            InfoTip({1485,235,25,25},in,"PREFLIGHT TESTING","Real CubeSats undergo environmental tests, including vibration. Here, all four equipment checks must pass before you can continue.");
            t=Resources(g);
            const char* labels[]={"Science instrument","Communication antenna","Battery","Sufficient solar power"};
            bool checks[]={t.science>0,t.radio>0,t.battery>0,t.generation>=t.load};
            static std::array<float,4> ticks{};
            for(int i=0;i<4;++i){
                ticks[i]=g.reducedMotion?float(checks[i]):Lerp(ticks[i],float(checks[i]),Clamp(GetFrameTime()*14,0,1));
                float y=292+i*43;
                DrawRectangleRounded({1108,y,24,24},0.15f,6,checks[i]?kSelected:kBackground);
                DrawRectangleRoundedLinesEx({1108,y,24,24},0.15f,6,1.5f,checks[i]?Color{91,210,202,255}:kDanger);
                if(ticks[i]>0.01f){
                    Vector2 start{1113,y+12},middle{1119,y+18},end{1128,y+6};
                    DrawLineEx(start,Vector2Lerp(start,middle,Clamp(ticks[i]*2,0,1)),2.5f,kAccent);
                    if(ticks[i]>0.5f)DrawLineEx(middle,Vector2Lerp(middle,end,(ticks[i]-0.5f)*2),2.5f,kAccent);
                }
                Text(labels[i],1144,y,21,checks[i]?Color{91,210,202,255}:kDanger);
            }
            DrawLine(1105,478,1505,478,kBorder);Text("SPACECRAFT STATS",1105,496,25,kText);
            const char* stats[]={"Payload mass","Slots used","Power used / generated","Battery capacity","Storage capacity"};
            std::string values[]={TextFormat("%d / %d kg",t.mass-crafts[g.craft].dryMass,crafts[g.craft].mass),TextFormat("%d / %d",t.used,crafts[g.craft].slots),TextFormat("%.0f / %.0f W",t.load,t.generation),TextFormat("%.0f Wh",t.battery),TextFormat("%.0f data",t.storage)};
            for(int i=0;i<5;++i){float y=546+i*40;Text(stats[i],1105,y,18,kMuted);Text(values[i],1505-TextWidth(values[i].c_str(),21),y,21,kText);}
            InfoTip({1485,495,25,25},in,"MASS AND CAPACITY","Launch systems limit spacecraft mass. Extra equipment can exceed your ship or rocket capacity; compare the totals before choosing a launcher.");
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
                ,rockets[i].name,in,true,false,g.rocket==i)){
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
                ,i==0?"LOW LUNAR ORBIT":"HIGH LUNAR ORBIT",in,true,false,g.orbit==i)){
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
                Panel(box,selected?kSelected:kPanel,selected?kAccent:hover?kMuted:kBorder);
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
            Ship({795,510},1.7f,g.craft,0);
            if(f.camera&&f.mode!=2&&f.heat<=0&&f.pointing<=0&&f.energy>0)DrawLineEx({795,530},{900,700},3,Fade(kAccent,0.5f));
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
            if(Button({1135,405,380,55},f.camera?"SCIENCE ON":"SCIENCE OFF",in,true,false,f.camera))f.camera=!f.camera;
            if(Button({1135,478,380,55},f.transmit?"TRANSMIT ON":"TRANSMIT OFF",in,true,false,f.transmit))f.transmit=!f.transmit;
            if(Button({1135,551,380,55},f.mode==0?"MODE: SCIENCE":f.mode==1?"MODE: BALANCED":"MODE: SAFE",in))f.mode=(f.mode+1)%3;
            Meter("Storage / data",f.stored,t.storage,1135,638,f.stored>t.storage*0.85f?kDanger:kGold);
            if(Button({1135,735,180,40},g.paused?"RESUME":"PAUSE",in,true,g.paused,!g.paused))g.paused=!g.paused;
            if(Button({1330,735,185,40},"END MISSION",in))Go(g,10,a);
            for(int i=0;i<3;++i){int speed=i==0?1:i==1?3:5; if(Button({640+i*108.0f,732,100,55},TextFormat("x%d",speed),in,true,false,g.speed==speed))g.speed=speed;}
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
            ,g.soundEnabled?"SOUND: ON":"SOUND: OFF",input,true,false,g.soundEnabled)){
                g.soundEnabled=!g.soundEnabled;
                a.Play(g);
            }
            if(Button({
                800,340,480,65}
            ,g.reducedMotion?"REDUCED MOTION: ON":"REDUCED MOTION: OFF",input,true,false,g.reducedMotion))g.reducedMotion=!g.reducedMotion;
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
