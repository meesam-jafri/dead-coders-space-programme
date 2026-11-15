#include "game.hpp"
int main(int argc,char** argv){
    using namespace mission;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_VSYNC_HINT|FLAG_MSAA_4X_HINT);
    InitWindow(1280,720,"DEAD CODERS Space Mission");
    if(!IsWindowReady())return 1;
    SetWindowMinSize(960,540);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    Skin().Load();
    Typography().Load();
    Typography().LoadBodyFonts();
    auto canvas=LoadRenderTexture(1600,900);
    if(!canvas.id){
        Typography().Unload();
        Skin().Unload();
        CloseWindow();
        return 1;
    }
    SetTextureFilter(canvas.texture,TEXTURE_FILTER_BILINEAR);
    AudioFeedback audio;
    audio.Init();
    GameState game; int frames=0;
    bool capture=argc>2&&std::string(argv[1])=="--capture-stage";
    bool smoke=capture||(argc>1&&std::string(argv[1])=="--smoke-test");
    int captureStage=capture?std::clamp(std::atoi(argv[2]),0,10):0;
    std::string captureScene=capture&&argc>3?argv[3]:"default";
    if(capture){
        game.moon=true;game.craft=1;game.rocket=1;game.orbit=1;game.date=1;
        game.installed[0]=0;game.installed[1]=4;game.installed[2]=5;game.installed[3]=6;
        AudioFeedback silent;StartFlight(game);game.reportSaved=true;game.stage=9;
        for(int i=0;i<2500&&game.stage==9;++i){if(game.flight.event>=0){EventChoice(game,0);game.flight.camera=true;}Simulate(game,0.05f,silent);}
        game.reportText=ComposeReport(game);game.stage=captureStage;game.paused=true;game.transition=0;game.noticeTime=0;
        if(captureStage==9){game.flight.time=38;game.flight.energy=18;game.flight.stored=7;}
        if(captureScene=="empty")game.installed.fill(-1);
        if(captureScene=="failure"){game.craft=0;for(int i=0;i<7;++i)game.installed[i]=0;}
        game.soundEnabled=false;std::filesystem::create_directories("build/review");
    }
    uiAudio=&audio;uiGame=&game;
    while(!WindowShouldClose()&&!game.exitRequested&&(!smoke||frames++<(capture?45:8))){
        audio.Update(game.soundEnabled);
        float dt=Clamp(GetFrameTime(),0,0.05f);
        game.transition=fmaxf(0,game.transition-dt);
        game.noticeTime=fmaxf(0,game.noticeTime-dt);
        int before=game.stage;
        bool hadOverlay=game.overlay!=0||game.flight.event>=0;
        if(IsKeyPressed(KEY_F11))ToggleFullscreen();
        if(IsKeyPressed(KEY_ESCAPE)){
            if(game.overlay)game.overlay=0;
            else if(game.stage==8||game.stage==9)game.overlay=3;
            else if(game.stage>0&&game.stage<8)Go(game,game.stage-1,audio);
            else if(game.stage==10)Go(game,0,audio);
        }
        if(game.stage==8&&!game.overlay){
            int tick=int(game.flight.launch);
            game.flight.launch+=dt;
            if(tick!=int(game.flight.launch))audio.Play(game);
            if(game.flight.launch>=18){
                game.flight.energy=fmaxf(0,Resources(game).battery-SelectedWeather(game).delay);
                Go(game,9,audio);
            }
        }
        Simulate(game,dt*game.speed,audio);
        float scale=fmaxf(0.001f,fminf(GetScreenWidth()/1600.0f,GetScreenHeight()/900.0f));
        Rectangle dest{
            (GetScreenWidth()-1600*scale)/2,(GetScreenHeight()-900*scale)/2,1600*scale,900*scale};
        auto mouse=GetMousePosition();
        Input in{
            {
                (mouse.x-dest.x)/scale,(mouse.y-dest.y)/scale}
            ,IsMouseButtonPressed(MOUSE_BUTTON_LEFT),!game.overlay&&game.flight.event<0&&before==game.stage&&game.transition<=0,game.reducedMotion};
        // Optional deterministic UI review input; normal gameplay uses the actual mouse.
        if(capture){
            in.mouse={20,880};in.clicked=false;
            if(captureScene=="moon")in.mouse={300,400};
            if(captureScene=="mars")in.mouse={800,400};
            if(captureScene=="jupiter")in.mouse={1300,400};
            if(captureScene=="tooltip")in.mouse={391,269};
            if(captureScene=="failure"){in.mouse={440,270};in.clicked=frames==35;}
        }
        // Completed flights may retain a pending event; results must always remain interactive.
        if(game.stage!=9&&game.flight.event>=0)game.flight.event=-1;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        BeginTooltips(in,game.stage*100+game.overlay*10+(game.flight.event>=0?1:0));
        BeginTextureMode(canvas);
        float time=game.reducedMotion?0:float(GetTime());
        Stars(time);
        Text("DEAD CODERS",50,25,24,kAccent);
        for(int i=0;game.stage>0&&i<11;++i){
            float x=560+i*36;
            if(i<10)DrawLine(int(x),40,int(x+36),40,kBorder);
            float size=i==game.stage?20.0f:12.0f;
            SkinPatch(Skin().marker,{x-size/2,40-size/2,size,size},8,i<=game.stage?kAccent:Color{70,64,60,255});
        }
        InfoTip({1490,20,38,38},in,"PREFLIGHT ENGINEERING",
            "CubeSats undergo vibration and environmental checks before flight. Here, readiness checks help you spot missing equipment before launching. Costs, weather and events are fictional.");
        if(game.stage){
            std::string heading=stages[game.stage];
            for(char& ch:heading)if(ch>='a'&&ch<='z')ch=char(ch-'a'+'A');
            Text(heading,62,109,46,Fade(BLACK,0.55f));Text(heading,60,105,46,kText);
            DrawRectangle(60,163,68,3,kAccent);
        }
        int drawn=game.stage;
        Flight prior=game.flight;
        Screen(game,in,audio,time);
        if(drawn==9&&game.stage==9&&(prior.camera!=game.flight.camera||prior.transmit!=game.flight.transmit||prior.mode!=game.flight.mode))game.timeline.push_back(std::string(TextFormat("Hour %.1f: Decision: manual control change -> science %s, transmission %s, mode %d. Science off preserves energy; transmission off retains data onboard; safe mode suspends collection.",game.flight.time*kMissionHoursPerSecond,game.flight.camera?"on":"off",game.flight.transmit?"on":"off",game.flight.mode)));
        if(game.stage>0)DrawCost(game,in,dt);
        Input footer=in;
        footer.enabled=in.enabled&&drawn==game.stage&&!game.overlay;
        if(game.stage>0&&game.stage<8){
            if(Button({
                60,835,190,50}
            ,"BACK",footer))Go(game,game.stage-1,audio);
            if(Button({
                1290,835,250,50}
            ,game.stage==7?"LAUNCH":"CONTINUE",footer,true,true))Next(game,audio);
        }
        Text("Fictional USD costs / F11 fullscreen / Esc menu",320,850,18,kMuted);
        if(game.noticeTime>0){
            Panel({
                720,90,820,70}
            ,game.noticeError?Color{56,32,33,255}:kPanel,game.noticeError?kDanger:kAccent);
            WrappedText(game.notice,740,104,780,20,game.noticeError?kDanger:kText);
        }
        if(game.transition>0&&!game.reducedMotion)DrawRectangle(0,170,1600,650,Fade(kBackground,game.transition*2));
        if(game.overlay||(game.flight.event>=0&&game.stage==9)){
            Input overlay=in;
            overlay.enabled=hadOverlay;
            Overlay(game,overlay,audio);
        }
        DrawTooltips();
        EndTextureMode();
        if(capture&&frames==40){
            Image snapshot=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&snapshot);
            ExportImage(snapshot,TextFormat("build/review/stage-%d-%s.png",captureStage,captureScene.c_str()));UnloadImage(snapshot);
        }
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(canvas.texture,{
            0,0,1600,-900}
        ,dest,{
            0,0}
        ,0,WHITE);
        // Draw at physical mouse coordinates after scaling the canvas. The supplied
        // cursor stays a crisp 32px with its tip at (0,0), even in letterboxed views.
        bool customCursor=Skin().cursor.id&&IsWindowFocused()&&IsCursorOnScreen()&&CheckCollisionPointRec(mouse,dest);
        if(customCursor){HideCursor();DrawTextureV(Skin().cursor,{std::floor(mouse.x),std::floor(mouse.y)},WHITE);}
        else ShowCursor();
        EndDrawing();
    }
    ShowCursor();
    uiAudio=nullptr;uiGame=nullptr;audio.Close();
    UnloadRenderTexture(canvas);
    Typography().Unload();
    Skin().Unload();
    CloseWindow();
    return 0;
}
