#include "game.hpp"
#include <cassert>
#include <iostream>
using namespace mission;

// Exercise actual screen controls without sending input to the user's desktop.
int main(){
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(960,540,"UI regression checks");
    assert(IsWindowReady());
    Typography().Load();Typography().LoadBodyFonts();Skin().Load();
    auto canvas=LoadRenderTexture(kWidth,kHeight);
    AudioFeedback silent;GameState game;game.stage=4;game.reducedMotion=true;
    auto click=[&](Vector2 mouse,bool pressed=true){
        Input in{mouse,pressed,true,true};
        BeginTextureMode(canvas);Stars(0);BeginTooltips(in,game.stage);
        Screen(game,in,silent,0);DrawTooltips();EndTextureMode();
    };
    int initial=Resources(game).cost;
    click({440,270});assert(Has(game,0)&&Resources(game).cost==initial+60);
    click({440,270});assert(std::count(game.installed.begin(),game.installed.end(),0)==2);
    click({500,270});assert(Resources(game).cost==initial+60);
    click({500,270});assert(Resources(game).cost==initial);
    click({500,270});assert(Resources(game).cost==initial); // disabled refund
    for(int i=0;i<7;++i)click({440,270});
    auto full=game.installed;click({440,270});
    assert(game.installed==full&&game.notice.find("NOT ENOUGH SLOTS")!=std::string::npos);
    game.installed.fill(-1);game.craft=2;game.rocket=2;game.team={true,true,true,false};
    for(int i=0;i<3;++i)click({440,370});
    assert(game.notice.find("NOT ENOUGH BUDGET")!=std::string::npos);
    game=GameState{};game.stage=4;game.installed.fill(1); // mass restriction, within slot count
    for(int i=5;i<11;++i)game.installed[i]=-1;
    click({440,670});assert(game.notice.find("PAYLOAD TOO HEAVY")!=std::string::npos);

    // Pin a tooltip, move away, click outside, and ensure covered pixels are opaque.
    auto tipFrame=[&](Vector2 mouse,bool pressed){
        Input in{mouse,pressed,true,true};BeginTextureMode(canvas);ClearBackground(RED);
        BeginTooltips(in,44);InfoTip({100,100,26,26},in,"BATTERY","Reserve energy keeps equipment running in shadow.");
        DrawTooltips();EndTextureMode();
        Image shot=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&shot);
        Color c=GetImageColor(shot,160,160);UnloadImage(shot);return c;
    };
    Color opaque=tipFrame({110,110},false);assert(ColorIsEqual(opaque,kBackground));
    tipFrame({110,110},true);opaque=tipFrame({20,880},false);assert(ColorIsEqual(opaque,kBackground));
    Color closed=tipFrame({20,880},true);assert(ColorIsEqual(closed,RED));
    ClearTooltips();

    game=GameState{};game.stage=9;game.paused=true;
    click({1200,430});assert(!game.flight.camera);
    click({1200,500});assert(!game.flight.transmit);
    click({790,760});assert(game.speed==3);
    click({900,760});assert(game.speed==5);
    click({690,760});assert(game.speed==1);
    click({1200,755});assert(!game.paused);
    click({1200,755});assert(game.paused);

    game=GameState{};game.stage=1;game.reducedMotion=true;
    click({800,400},false);click({250,670});assert(!game.moon); // locked Mars preview
    click({20,880},false);click({1300,400},false);click({250,670});assert(!game.moon);
    click({20,880},false);click({300,400},false);click({250,670});assert(game.moon);
    assert(Skin().cursor.id&&Skin().cursor.width==32&&Skin().cursor.height==32);
    UnloadRenderTexture(canvas);Skin().Unload();Typography().Unload();CloseWindow();
    std::cout<<"PASS: payload add/refund/limits, opaque hover and pinned tooltips, operations toggles, speed, pause, locked previews, Moon selection, cursor asset.\n";
}
