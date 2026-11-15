#pragma once
#include "raylib.h"
#include "raymath.h"
#include <array>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdio>
namespace mission {
    constexpr int kWidth=1600, kHeight=900, kBudget=1200;
    constexpr Color kBackground{12,12,14,255}, kPanel{26,26,29,255}, kBorder{67,62,58,255};
    constexpr Color kText{246,241,232,255}, kMuted{176,170,160,255};
    constexpr Color kAccent{255,139,54,255}, kGold{240,189,94,255};
    constexpr Color kDanger{244,65,65,255}, kSuccess{138,203,157,255};
    // All credits, masses, power ratings, simulation rates and scores are fictional educational values.
    constexpr const char* stages[]={
        "Mission: Lunar Recon","Select mission","Select spacecraft","Recruit your team","Spacecraft assembly","Select rocket","Select orbit","Launch date","Launch","Mission operations","Mission results"};
    struct Craft {
        const char* name;
        int cost, dryMass, mass, slots;
    };
    constexpr Craft crafts[]={
        {
            "Scout",160,35,90,7}
        ,{
            "Explorer",260,60,140,9}
        ,{
            "Carrier",380,100,190,11}
    };
    struct Component {
        const char* name;
        const char* tip;
        int cost,mass;
        float generation, consumption, battery, storage, science, radio;
    };
    constexpr Component parts[]={
        {
            "Lunar camera","Pictures the surface. Produces data that must be stored and returned.",60,8,0,8,0,0,1,0}
        ,
        {
            "Advanced camera","More observations per hour; higher power and storage demand.",125,15,0,15,0,0,1.8f,0}
        ,
        {
            "Laser altimeter","Uses laser pulses to measure elevation. Adds terrain observations.",90,12,0,10,0,0,1.2f,0}
        ,
        {
            "Small solar panel","Generates 22 W in sunlight. No generation during eclipse.",45,10,22,0,0,0,0,0}
        ,
        {
            "Large solar panel","More ongoing power, at greater cost and mass.",85,18,42,0,0,0,0,0}
        ,
        {
            "Battery pack","Stores 24 Wh. Backup energy does not fix a permanent shortage.",45,8,0,0,24,0,0,0}
        ,
        {
            "Basic antenna","Returns 3 data units per real second when Earth is visible.",50,6,0,5,0,0,0,3}
        ,
        {
            "High-gain antenna","Returns data faster, but consumes more power.",100,12,0,8,0,0,0,6}
        ,
        {
            "Storage upgrade","Adds 50 data units of onboard capacity.",40,5,0,1,0,50,0,0}
        ,
        {
            "Backup computer","Adds redundancy and a backup radio recovery option.",65,7,0,2,0,0,0,0}
    };
    constexpr const char* names[]={
        "Engineer","Scientist","Pilot","Doctor","Doctor"};
    constexpr const char* abilities[]={
        "Restores solar output remotely.","Improves returned science quality.","Faster radio recovery and transmission.","Restores pointing with less damage.","Not required: this mission has no onboard crew."};
    constexpr int salaries[]={
        65,70,65,60,0};
    struct Rocket {
        const char* name;
        int cost,capacity;
    };
    constexpr Rocket rockets[]={
        {
            "Budget Launcher",180,110}
        ,{
            "Standard Launcher",260,185}
        ,{
            "Heavy Launcher",350,300}
    };
    // Gregorian dates are real. Weather is deterministic fictional game data.
    struct LaunchWeather {
        int windKph=0; bool lightning=false, transfer=true; int delay=0;
        bool Valid() const { return windKph<25 && !lightning && transfer; }
        std::string Summary() const;
    };
    constexpr float kOperationSeconds=120.0f, kMissionHoursPerSecond=0.08f;
    struct Totals {
        int cost=0,mass=0,used=0;
        float generation=0,load=4,battery=0,storage=20,science=0,radio=0;
    };
    struct Flight {
        float time=0,energy=0,stored=0,returned=0,generated=0,observations=0,health=100,launch=0;
        float radioFault=0,heat=0,pointing=0,solarFault=0,risk=0;
        int mode=1,event=-1,nextEvent=0;
        bool camera=true,transmit=true;
        std::array<int,5> events{
            {
                0,1,2,3,4}
        };
    };
    struct GameState {
        int stage=0,craft=0,rocket=-1,orbit=-1,date=-1,pending=-1,page=0,overlay=0;
        int calendarYear=2026,calendarMonth=11,launchYear=2026,launchMonth=11; float payloadScroll=0; bool noticeError=true; bool moon=false,soundEnabled=true,reducedMotion=false,exitRequested=false,paused=false;
        std::array<int,11> installed;
        std::array<bool,4> team{};
        Flight flight;
        float transition=0,noticeTime=0,reportScroll=0; int speed=1; bool reportSaved=false; std::string reportText; std::vector<std::string> timeline;
        std::string notice;
        GameState(){
            installed.fill(-1);
        }
    };

    struct AudioFeedback {
        Sound error{}, launch{}, complete{}; Music ambience{}; bool hasAmbience=false; mutable double lastClick=-1; Sound select{};
        Sound navigate{};
        bool ready = false;
        // Short generated tones keep this foundation self-contained: no assets/files.
        static Sound MakeTone(float frequency) {
            constexpr int sampleRate = 22050;
            constexpr int sampleCount = sampleRate / 10;
            std::vector<short> samples(sampleCount);
            for (int i = 0; i < sampleCount; ++i) {
                const float t = static_cast<float>(i) / sampleRate;
                const float envelope = std::sin(PI * static_cast<float>(i) / sampleCount);
                samples[i] = static_cast<short>(std::sin(2.0f * PI * frequency * t) * envelope * 5500);
            }
            Wave wave{};
            wave.frameCount = sampleCount;
            wave.sampleRate = sampleRate;
            wave.sampleSize = 16;
            wave.channels = 1;
            wave.data = samples.data();
            return LoadSoundFromWave(wave);
            // raylib copies the samples.
        }
        void Init() {
            InitAudioDevice();
            ready = IsAudioDeviceReady();
            if (ready) {
                error=FileExists("assets/sound/error sound.mp3")?LoadSound("assets/sound/error sound.mp3"):MakeTone(180); launch=FileExists("assets/sound/rocket launch sound.mp3")?LoadSound("assets/sound/rocket launch sound.mp3"):MakeTone(100); complete=FileExists("assets/sound/mission complete sound.mp3")?LoadSound("assets/sound/mission complete sound.mp3"):MakeTone(900); SetSoundVolume(launch,0.25f);
                hasAmbience=FileExists("assets/sound/brown noise.mp3"); if(hasAmbience){ambience=LoadMusicStream("assets/sound/brown noise.mp3"); hasAmbience=ambience.ctxData!=nullptr; if(hasAmbience){ambience.looping=true;SetMusicVolume(ambience,0.035f);PlayMusicStream(ambience);}}
                select = FileExists("assets/sound/payloadadded.wav")?LoadSound("assets/sound/payloadadded.wav"):MakeTone(740.0f);
                navigate = FileExists("assets/sound/button click.mp3")?LoadSound("assets/sound/button click.mp3"):MakeTone(520.0f);
            }
        }
        void Play(const GameState& game, bool selection = false) const {
            if (ready && game.soundEnabled){double now=GetTime();if(selection||now-lastClick>0.08){PlaySound(selection ? select : navigate);lastClick=now;}}
        }
        void Cue(const GameState& game,int cue) const {if(ready&&game.soundEnabled)PlaySound(cue==0?error:cue==1?launch:complete);}
        void Update(bool enabled) {
            if(!ready)return;
            if(hasAmbience){UpdateMusicStream(ambience);SetMusicVolume(ambience,enabled?0.035f:0);}
            if(!enabled){for(Sound sound:{select,navigate,error,launch,complete})StopSound(sound);}
        }
        void Close() {
            if (ready) {
                if(hasAmbience)UnloadMusicStream(ambience); UnloadSound(error);UnloadSound(launch);UnloadSound(complete); UnloadSound(select);
                UnloadSound(navigate);
                CloseAudioDevice();
            }
        }
    };
    inline AudioFeedback* uiAudio=nullptr; inline const GameState* uiGame=nullptr;
    struct Input {
        Vector2 mouse;
        mutable bool clicked;
        // A press is consumed by its first control only.
        bool enabled;
        bool reducedMotion = false;
    };
    // One shared font keeps text rendering, wrapping and button alignment consistent.
    // Fonts are read from existing files only; no assets are downloaded or written.
    // Kenney UI Pack: Sci-fi (CC0). Shared skin for every screen.
    // Textures load once and use nine-slice borders so corners do not stretch.
    Texture2D LoadPhoto(const char* path);
    struct UiSkin {
        std::array<Texture2D,3> photos{}; Texture2D info{}, panel{}, normal{}, selected{}, barTrack{}, barFill{}, marker{};
        std::string root;
        void Load() {
            const char* candidates[]={"assets/kenney_ui-pack-space-expansion/", "../assets/kenney_ui-pack-space-expansion/"};
            for(const char* candidate:candidates) {
                if(DirectoryExists(candidate)) { root=candidate; break; }
            }
            if(root.empty()) root=std::string(GetApplicationDirectory())+"../assets/kenney_ui-pack-space-expansion/";
            auto load=[&](const char* name) {
                const std::string path=root+"PNG/"+name;
                Texture2D texture{};
                if(FileExists(path.c_str())) texture=LoadTexture(path.c_str());
                if(texture.id) SetTextureFilter(texture,TEXTURE_FILTER_BILINEAR);
                return texture;
            };
            for(int i=0;i<3;++i){std::string path="assets/moon/"+std::to_string(i+1)+".jpg";if(FileExists(path.c_str()))photos[i]=LoadPhoto(path.c_str());} if(FileExists("assets/icons/info.png")){Image icon=LoadImage("assets/icons/info.png"); if(icon.data){Color* colors=LoadImageColors(icon);float radius=icon.width*0.36f;for(int y=0;y<icon.height;++y)for(int x=0;x<icon.width;++x)if(Vector2Distance({float(x),float(y)},{icon.width/2.0f,icon.height/2.0f})>radius)colors[y*icon.width+x].a=0;Image masked{colors,icon.width,icon.height,1,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};info=LoadTextureFromImage(masked);UnloadImageColors(colors);UnloadImage(icon);}} panel=load("Grey/Default/button_square_header_large_rectangle_screws.png");
            normal=load("Grey/Default/button_square_header_small_rectangle.png");
            selected=load("Grey/Default/button_square_header_small_rectangle.png");
            barTrack=load("Grey/Default/bar_round_large.png");
            barFill=load("Grey/Default/bar_round_large.png");
            marker=load("Grey/Default/bar_round_large_square.png");
        }
        void Unload() {
            for(auto texture:photos){if(texture.id)UnloadTexture(texture);} for(Texture2D texture:{info,panel,normal,selected,barTrack,barFill,marker})
            if(texture.id) UnloadTexture(texture);
        }
    };
    inline UiSkin& Skin() { static UiSkin skin; return skin; }
    inline void SkinPatch(Texture2D texture,Rectangle bounds,int top=18,Color tint=WHITE) {
        if(!texture.id) { DrawRectangleRec(bounds,kPanel); DrawRectangleLinesEx(bounds,1,kBorder); return; }
        NPatchInfo patch{{0,0,float(texture.width),float(texture.height)},8,top,8,8,NPATCH_NINE_PATCH};
        DrawTextureNPatch(texture,patch,bounds,{0,0},0,tint);
    }
    struct InterfaceFont {
        Font font{}
        , body{}
        , telemetry{};
        Font title{}, report{}, button{}; bool owned = false;
        void Load() {
            font = GetFontDefault();
            const char* candidates[] = {
                "assets/font/PolarisBold.ttf", "../assets/font/PolarisBold.ttf",
                "C:/Windows/Fonts/bahnschrift.ttf", "C:/Windows/Fonts/consola.ttf"
            };
            for (const char* path : candidates) {
                if (!FileExists(path)) continue;
                Font loaded = LoadFontEx(path, 64, nullptr, 0);
                if (loaded.texture.id != 0 && loaded.texture.id != GetFontDefault().texture.id) {
                    font = loaded;
                    owned = true;
                    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
                    break;
                }
            }
        }
        void LoadBodyFonts() {
            body = GetFontDefault();
auto custom=[](const char* path){return FileExists(path)?LoadFontEx(path,72,nullptr,0):GetFontDefault();};
title=custom("assets/font/LIQUIDNIGHT-BF64bf9967d3b8f.ttf");
report=custom("assets/font/Tigreal Rough.ttf");
button=custom("assets/font/PolarisSemibold.ttf");
            telemetry = GetFontDefault();
            const char* bodyPaths[]={
                "assets/font/PolarisRegular.ttf","../assets/font/PolarisRegular.ttf","C:/Windows/Fonts/segoeui.ttf"};
            const char* numberPaths[]={
                "assets/font/PolarisRegular.ttf","../assets/font/PolarisRegular.ttf","C:/Windows/Fonts/consola.ttf"};
            for(const char* path:bodyPaths) if(FileExists(path)){
                body=LoadFontEx(path,48,nullptr,0);
                break;
            }
            for(const char* path:numberPaths) if(FileExists(path)){
                telemetry=LoadFontEx(path,48,nullptr,0);
                break;
            }
            SetTextureFilter(body.texture,TEXTURE_FILTER_BILINEAR);
            SetTextureFilter(telemetry.texture,TEXTURE_FILTER_BILINEAR);
        }
        Font ForSize(int size) const {
            return size>=32?font:body;
        }
        void Unload() {
            if(body.texture.id && body.texture.id!=GetFontDefault().texture.id)UnloadFont(body);
            if(telemetry.texture.id && telemetry.texture.id!=GetFontDefault().texture.id)UnloadFont(telemetry);
            for(Font extra:{title,report,button}) {if(extra.texture.id && extra.texture.id!=GetFontDefault().texture.id)UnloadFont(extra);} if (owned) UnloadFont(font);
            owned = false;
        }
    };
    inline InterfaceFont& Typography() {
        static InterfaceFont typography;
        return typography;
    }

Totals Resources(const GameState&); bool Has(const GameState&,int);
std::string BuildIssue(const GameState&); std::string Readiness(const GameState&);
float TextSpacing(int); int TextWidth(const char*,int);
void Text(const std::string&,float,float,int=20,Color=kText);
void WrappedText(const std::string&,float,float,int,int,Color);
void Panel(Rectangle,Color=kPanel,Color=kBorder);
bool Hovered(Rectangle,const Input&);
bool Button(Rectangle,const char*,const Input&,bool=true,bool=false);
void Notice(GameState&,const std::string&,bool error=true); void Go(GameState&,int,const AudioFeedback&);
void StartFlight(GameState&); void Next(GameState&,const AudioFeedback&);
void Moon(Vector2,float,Color=kMuted); void Stars(float);
void Ship(Vector2,float,int,float); void RocketArt(Vector2,int,float);
void Meter(const char*,float,float,float,float,Color); void Tooltip(const Input&,Rectangle,const char*);
void EventChoice(GameState&,int); void Simulate(GameState&,float,const AudioFeedback&); int Score(const GameState&);
void Screen(GameState&,const Input&,const AudioFeedback&,float); void Overlay(GameState&,const Input&,const AudioFeedback&);
std::string Money(int); std::string Clock(float seconds);
int DaysInMonth(int year,int month); int FirstWeekday(int year,int month);
LaunchWeather WeatherFor(int year,int month,int day);
LaunchWeather SelectedWeather(const GameState&); std::string DateLabel(const GameState&);
void WeatherIcon(Vector2,const LaunchWeather&,float=1);
void Scrollbar(Rectangle,float& value,float maximum,float visible,const Input&);
bool InfoButton(Rectangle,const Input&);
std::string ComposeReport(const GameState&); void SaveReport(GameState&); void DrawReport(GameState&,const Input&,const AudioFeedback&);
}
