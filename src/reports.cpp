#include "game.hpp"
#include <ctime>
#include <iomanip>
namespace mission {
namespace fs=std::filesystem;
static fs::path ReportFolder(){return fs::path(GetApplicationDirectory())/"reports";}
// CSV supports quoted commas, escaped quotes and multiline fields.
static std::vector<std::vector<std::string>> ImageMetadata(){
    std::ifstream file("assets/moon/image_details.csv");std::vector<std::vector<std::string>> rows;std::vector<std::string> row;std::string field;bool quoted=false;char c;
    while(file.get(c)){if(c=='"'){if(quoted&&file.peek()=='"'){file.get(c);field+='"';}else quoted=!quoted;}
        else if(c==','&&!quoted){row.push_back(field);field.clear();}
        else if(c=='\n'&&!quoted){row.push_back(field);if(row.size()==3)rows.push_back(row);row.clear();field.clear();}
        else if(c!='\r')field+=c;}
    if(!field.empty()){row.push_back(field);if(row.size()==3)rows.push_back(row);}return rows;
}std::string ComposeReport(const GameState& g){
    auto t=Resources(g); auto& f=g.flight; std::ostringstream out;
    bool success=f.health>0&&f.observations>=10&&f.returned>=21;
        bool partial=!success&&f.health>0&&f.returned>0;
    int score=std::clamp(Score(g),0,success?100:partial?69:39);
    const char* grade=score>=95?"S":score>=85?"A":score>=75?"B":score>=60?"C":score>=40?"D":"F";
    out << "LUNAR MISSION / " << (success?"SUCCESS":partial?"PARTIAL SUCCESS":"FAILURE") << "\n\nMISSION VERDICT\n";
    out<<"GRADE "<<grade<<" / "<<score<<" out of 100\n";
    out<<(success?"Both science targets met and the spacecraft survived.":partial?"Some science reached Earth, but one or more targets were missed.":"The mission lost its spacecraft or returned no scientific data.")<<"\n";
    out<<"Grade balances returned science, objectives, survival and budget efficiency. Failed missions are capped below 40; partial missions below 70.\n\n";
    out << "MISSION CONFIGURATION\nShip: "<<crafts[g.craft].name<<"\nGround team: ";
    for(int i=0;i<4;++i)if(g.team[i])out<<names[i]<<"; ";
    out<<"\nPayloads:\n";for(int id=0;id<10;++id){int count=int(std::count(g.installed.begin(),g.installed.end(),id));if(count)out<<"  "<<parts[id].name<<" x"<<count<<"\n";}
    if(g.rocket>=0)out<<"Rocket: "<<rockets[g.rocket].name<<"\n";
    out<<"Orbit: "<<(g.orbit==0?"Low lunar":"High lunar")<<"\n";
    if(g.date>=0)out<<"Launch: "<<DateLabel(g)<<" / "<<SelectedWeather(g).Summary()<<"\n";
    out<<"Slots: "<<t.used<<" / "<<crafts[g.craft].slots<<". Payload capacity: "<<crafts[g.craft].mass<<" kg. Battery: "<<t.battery<<" Wh. Storage: "<<t.storage<<" units.\n"; out<<"\nPERFORMANCE\nCost: "<<Money(t.cost)<<" / Budget $1,200. Mass: "<<t.mass<<" kg.\n";
    out<<"Generation: "<<t.generation<<" W. Installed load: "<<t.load<<" W.\n";
    out<<"Mission duration: "<<f.time*kMissionHoursPerSecond<<" hours. Health: "<<f.health<<"%.\n";
    out<<"Observations: "<<f.observations<<" / 10. Returned data: "<<f.returned<<" / 21.\n";
    out<<"Generated: "<<f.generated<<". Onboard: "<<f.stored<<". Battery: "<<f.energy<<" Wh.\n";
    out<<"Educational score: "<<score<<" / 100.\n\nSCIENTIFIC RESULTS\n";
    if(Has(g,0)||Has(g,1))out<<"Camera installed: surface imaging was part of the science plan.\n";
    if(Has(g,2))out<<"Laser altimeter installed: elevation measurements were part of the plan.\n";
    out<<"Only collected and returned data contributes to mission performance.\n\nTIMELINE\n";
    for(const auto& event:g.timeline)out<<event<<"\n";
    out<<"\nDECISIONS & CONSEQUENCES\n";
    for(const auto& event:g.timeline)if(event.find("Decision:")!=std::string::npos)out<<event<<"\n";
    if(f.event>=0)out<<"An emergency remained unanswered at mission end. No successful recovery is credited.\n";
    out<<"The selected instruments generated "<<f.generated<<" data units; "<<f.returned<<" reached Earth and "<<f.stored<<" remained onboard.\n";
    out<<"Final reserve: "<<f.energy<<" Wh. Remaining health: "<<f.health<<"%. These measured outcomes include all operating choices.\n";
    out<<"\nFINAL EVALUATION\nBattery energy is stored in Wh; solar power is generated in W.\nA blocked Earth link requires onboard storage until contact returns.\n";
    out<<(f.returned<21?"Allow more transmission time before ending the mission.\n":"Your radio returned the required science. Try a lower-cost design.\n");
    out<<(f.observations<10?"Keep science enabled longer when power allows.\n":"Observation target met. Compare other instrument combinations.\n");
    out<<"\nAll numerical values are fictional game balance.\nImages below are supplied illustrations, not captures from this flight.\n";
    if((Has(g,0)||Has(g,1))&&f.observations>0&&f.imagingReturned>0){
        out<<"\nLUNAR REFERENCE IMAGERY\nSupplied NASA reference imagery illustrates lunar features; these are not captures from this fictional flight.\n";
        int limit=std::min(Has(g,1)?4:2,std::max(1,int(f.imagingReturned/10)));
        for(const auto& row:ImageMetadata()){
            if(limit<=0)break;
            for(int i=1;i<=6;++i)if(row[0]==std::to_string(i)+".jpg"&&FileExists(("assets/moon/"+row[0]).c_str())){
                out<<"@IMAGE "<<i-1<<"\n"<<row[1]<<"\n"<<row[2]<<"\n\n";--limit;
            }
        }
    }
    return out.str();
}
void SaveReport(GameState& g){
    g.reportText=ComposeReport(g);g.reportScroll=0;
    try{
        fs::create_directories(ReportFolder());
        auto id=std::chrono::system_clock::now().time_since_epoch().count();
        std::time_t now=std::time(nullptr);std::ostringstream stamp;stamp<<std::put_time(std::gmtime(&now),"%Y-%m-%d %H:%M:%S UTC");
        g.reportText.insert(g.reportText.find("\n")+1,"Report ID: "+std::to_string(id)+" / "+stamp.str()+"\n"); fs::path final=ReportFolder()/(std::to_string(id)+".txt"),temp=final;temp+=".tmp";
        std::ofstream file(temp);file<<g.reportText;file.close();
        if(!file){Notice(g,"Report is visible, but could not be saved.");return;}
        fs::rename(temp,final);g.reportSaved=true;
    }catch(const std::exception&){Notice(g,"Report is visible, but saving failed. Check folder permissions.");}
}
// Some supplied display fonts omit punctuation/digits. Keep report prose in
// Tigreal and fall back per glyph, rather than printing missing-character boxes.
static Font ReportGlyph(char& ch){
    Font report=Typography().report;
    auto has=[&](char c){for(int i=0;i<report.glyphCount;++i)if(report.glyphs[i].value==c)return true;return false;};
    if(has(ch))return report;
    if(ch>='a'&&ch<='z'&&has(char(ch-'a'+'A'))){ch=char(ch-'a'+'A');return report;}
    return Typography().body;
}
static float ReportWidth(const std::string& text){float x=0;for(char ch:text){Font font=ReportGlyph(ch);char glyph[]={ch,0};x+=MeasureTextEx(font,glyph,23,0).x+1;}return x;}
static void ReportLine(const std::string& text,float x,float y){for(char ch:text){Font font=ReportGlyph(ch);char glyph[]={ch,0};DrawTextEx(font,glyph,{x,y},23,0,{43,43,49,255});x+=MeasureTextEx(font,glyph,23,0).x+1;}}
void DrawReport(GameState& g,const Input& in,const AudioFeedback& a){
    if(g.reportText.empty()){
        Text("MISSION ARCHIVE",100,210,38,kGold);
        std::vector<fs::path> paths;std::error_code error;
        if(fs::exists(ReportFolder(),error))for(const auto& item:fs::directory_iterator(ReportFolder(),error))if(item.path().extension()==".txt")paths.push_back(item.path());
        std::sort(paths.rbegin(),paths.rend());
        if(in.enabled)g.reportScroll=Clamp(g.reportScroll-GetMouseWheelMove(),0,float(paths.size()>6?paths.size()-6:0));
        for(int i=int(g.reportScroll);i<int(paths.size())&&i<int(g.reportScroll)+6;++i){
            if(Button({100,290.0f+(i-int(g.reportScroll))*75,1100,60},paths[i].filename().string().c_str(),in)){
                std::ifstream file(paths[i]);std::ostringstream content;content<<file.rdbuf();g.reportText=content.str();g.reportScroll=0;break;}}
        if(paths.empty())Text("No saved missions yet. Complete a flight first.",100,320,26);
    }else{
        const Color paper{241,231,210,255}, ink{41,36,31,255};
        Rectangle sheet{180,180,1250,610};
        DrawRectangleRec({188,188,1250,610},Fade(BLACK,0.4f));DrawRectangleRec(sheet,paper);
        for(int i=0;i<230;++i){DrawPixel(190+(i*137)%1230,285+(i*79)%490,Color{225,213,192,255});} DrawRectangleRoundedLinesEx({1396,188,15,59},0.8f,8,2,kMuted);
        DrawRectangle(180,180,1250,94,kBackground);DrawRectangle(180,274,1250,5,kAccent);
        DrawRectangleRoundedLinesEx({1400,185,13,65},0.8f,8,2,kMuted); Text("DC / SCIENCE DIVISION",215,195,18,kAccent);
        Text("MISSION DEBRIEF",215,224,33,kText);
        bool success=g.reportText.substr(0,g.reportText.find('\n')).find("SUCCESS")!=std::string::npos;
        bool partial=g.reportText.find("PARTIAL SUCCESS")<g.reportText.find("\n"); Color stamp=success&&!partial?kSuccess:kDanger;
        DrawRectangleLinesEx({1130,200,260,55},3,stamp);
        Text(partial?"PARTIAL SUCCESS":success?"MISSION SUCCESS":"MISSION FAILURE",1150,216,23,stamp);
        struct Row{std::string text;bool heading;float y;};
        static std::string cached;static std::vector<Row> rows;static std::vector<std::pair<std::string,float>> sections;static float bodyHeight=0;
        if(cached!=g.reportText){
            cached=g.reportText;rows.clear();sections.clear();float y=0;std::istringstream paragraphs(g.reportText);std::string paragraph;bool first=true;
            while(std::getline(paragraphs,paragraph)){
                if(first){first=false;continue;}
                if(paragraph.empty()){y+=12;continue;}
                if(paragraph.rfind("@IMAGE ",0)==0){rows.push_back({paragraph,false,y});y+=290;continue;} bool heading=paragraph=="MISSION VERDICT"||paragraph=="DECISIONS & CONSEQUENCES"||paragraph=="SCIENTIFIC RESULTS"||paragraph=="FINAL EVALUATION"||paragraph=="LUNAR REFERENCE IMAGERY"||paragraph=="MISSION CONFIGURATION"||paragraph=="PERFORMANCE"||paragraph=="RESEARCH"||paragraph=="TIMELINE"||paragraph=="LESSONS / RECOMMENDATIONS";
                if(heading){y+=18;sections.push_back({paragraph,y});rows.push_back({paragraph,true,y});y+=56;continue;}
                std::istringstream words(paragraph);std::string word,line;
                while(words>>word){std::string next=line.empty()?word:line+" "+word;
                    if(ReportWidth(next)>1100&&!line.empty()){rows.push_back({line,false,y});y+=34;line=word;}else line=next;}
                rows.push_back({line,false,y});y+=34;
            }
            bodyHeight=y+30;
        }
        constexpr float visible=485;
        float maximum=fmaxf(0,bodyHeight-visible);
        if(in.enabled&&CheckCollisionPointRec(in.mouse,{180,280,1300,510}))g.reportScroll=Clamp(g.reportScroll-GetMouseWheelMove()*70,0,maximum);
        Scrollbar({1452,285,16,505},g.reportScroll,maximum,visible,in);
        for(int i=0;i<int(sections.size());++i){
            if(Button({55,285.0f+i*69,100,51},TextFormat("%02d",i+1),in))g.reportScroll=Clamp(sections[i].second,0,maximum);
        }
        BeginScissorMode(180,280,1250,510);
        for(const auto& row:rows){float y=295+row.y-g.reportScroll;if(y<-20||y>790)continue; if(row.text.rfind("@IMAGE ",0)==0){int id=std::atoi(row.text.c_str()+7);if(id>=0&&id<6&&Skin().photos[id].id){auto tex=Skin().photos[id];float scale=std::min(1100.0f/tex.width,260.0f/tex.height);DrawTexturePro(tex,{0,0,float(tex.width),float(tex.height)},{230,y,tex.width*scale,tex.height*scale},{0,0},0,WHITE);DrawRectangleLinesEx({230,y,tex.width*scale,tex.height*scale},1,kBorder);}continue;}
            if(row.heading){DrawRectangle(215,int(y),1180,37,{223,208,180,255});DrawRectangle(215,int(y),5,37,kAccent);DrawTextEx(Typography().font,row.text.c_str(),{233,y+7},23,1,ink);}
            else if(row.text.rfind("GRADE ",0)==0){DrawTextEx(Typography().font,row.text.c_str(),{230,y},27,1,ink);} else {ReportLine(row.text,230,y);}
        }
        EndScissorMode();
        Text(TextFormat("PAGE %d / %d",1+int(g.reportScroll/visible),std::max(1,int(std::ceil(bodyHeight/visible)))),1230,801,18,kMuted); Text("Dossier / scroll or drag the rail / reference images are not mission captures",205,801,18,kMuted);
    }
    if(Button({60,835,210,50},"MAIN MENU",in))Go(g,0,a);
    if(Button({1050,835,230,50},"REDESIGN",in)){g.reportSaved=false;Go(g,4,a);}
    if(Button({1310,835,230,50},"ARCHIVE",in)){g.reportText.clear();g.reportScroll=0;}
}
}