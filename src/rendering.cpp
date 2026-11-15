#include "game.hpp"
namespace mission {
    void Moon(Vector2 p,float r,Color c){
        constexpr Color kBackground{12,12,14,255};
        DrawCircleV(p,r+16,Fade(c,0.06f));
        DrawCircleV(p,r,c);
        for(int i=0;i<12;++i){
            float angle=i*2.4f;
            Vector2 q{
                p.x+std::cos(angle)*r*0.65f,p.y+std::sin(angle)*r*0.65f};
            DrawCircleV(q,r*(0.04f+(i%3)*0.035f),Fade(kBackground,0.16f));
        }
    }
    void Stars(float time){
        ClearBackground(kBackground);
        DrawRectangleGradientV(0,0,1600,900,kBackground,{
            18,35,45,255}
        );
        for(int i=0;i<160;++i){
            float x=std::fmod(i*491.0f+time*(i%3+1)*2,1600.0f);
            DrawCircleV({
                x,float((i*283+47)%900)}
            ,i%5?1:2,Fade(kMuted,0.25f+0.15f*std::sin(time+i)));
        }
    }
    void Ship(Vector2 p,float s,int kind,float time){
        constexpr Color kAccent{255,139,54,255}, kMuted{176,170,160,255};
        p.y+=std::sin(time*1.2f)*4;
        auto point=[&](float x,float y){return Vector2{p.x+x*s,p.y+y*s};};
        auto plate=[&](float x,float y,float w,float h,Color color){
            Rectangle rect{p.x+x*s,p.y+y*s,w*s,h*s};
            DrawRectangleRounded({rect.x+3*s,rect.y+5*s,rect.width,rect.height},0.08f,6,{8,8,10,255});
            DrawRectangleRounded(rect,0.08f,6,color);
            DrawLineEx({rect.x+3*s,rect.y+2*s},{rect.x+rect.width-3*s,rect.y+2*s},1.5f*s,Fade(WHITE,0.45f));
            DrawRectangleRoundedLinesEx(rect,0.08f,6,s,{56,57,59,255});
        };
        auto array=[&](float x,float y,float w,float h){
            plate(x,y,w,h,{37,38,42,255});
            for(int row=0;row<4;++row)for(int col=0;col<5;++col){
                float cw=(w-8)/5,ch=(h-8)/4;
                DrawRectangleRec({p.x+(x+4+col*cw)*s,p.y+(y+4+row*ch)*s,(cw-1.5f)*s,(ch-1.5f)*s},
                                 (row+col)%2?Color{97,66,44,255}:Color{126,82,49,255});
            }
            DrawLineEx(point(x+2,y+h/2),point(x+w-2,y+h/2),1.5f*s,{225,164,96,255});
        };
        DrawEllipse(int(p.x),int(p.y+102*s),122*s,14*s,Fade(BLACK,0.35f));
        const Color hull{198,193,178,255},trim{244,231,204,255};
        if(kind==0){
            DrawLineEx(point(-115,0),point(115,0),7*s,{102,99,91,255});
            array(-125,-26,78,52);array(47,-26,78,52);
            DrawPoly(point(0,4),6,48*s,30,{54,51,48,255});
            DrawPoly(p,6,44*s,30,hull);DrawPoly(point(-5,-5),6,32*s,30,trim);
            plate(-28,-12,56,23,kAccent);
        }else if(kind==1){
            for(int side:{-1,1}){
                DrawLineEx(point(side*30,-28),point(side*72,-46),5*s,kMuted);
                DrawLineEx(point(side*30,28),point(side*72,46),5*s,kMuted);
                array(side<0?-130:60,-78,70,68);array(side<0?-130:60,10,70,68);
            }
            plate(-36,-69,72,138,hull);plate(-26,-54,52,68,trim);
            plate(-24,25,48,29,kAccent);
            DrawCircleV(point(0,-83),20*s,hull);DrawCircleV(point(0,-83),13*s,{72,65,58,255});
        }else{
            DrawLineEx(point(-135,0),point(135,0),12*s,{111,106,97,255});
            array(-145,-30,60,60);array(85,-30,60,60);
            plate(-80,-49,58,98,hull);plate(22,-49,58,98,hull);
            plate(-21,-71,42,140,{99,97,90,255});
            for(int x:{-67,35}){plate(float(x),-34,32,67,trim);plate(float(x),-5,32,12,kAccent);}
            for(int x:{-63,0,63}){
                DrawCircleV(point(float(x),63),15*s,{42,43,46,255});
                DrawCircleV(point(float(x),63),9*s,{153,95,49,255});
                DrawCircleLines(int(p.x+x*s),int(p.y+63*s),15*s,kMuted);
            }
        }
        // Permanent chassis detail; installed payload choices never alter this art.
        DrawLineEx(point(18,-40),point(48,-100),3*s,trim);
        DrawCircleV(point(48,-100),21*s,{99,94,83,255});
        DrawCircleV(point(45,-103),17*s,hull);
        DrawLineEx(point(45,-103),point(34,-119),2*s,kAccent);
        DrawCircleV(point(34,-119),4*s,trim);
        DrawCircleV(point(0,3),17*s,{22,23,25,255});
        DrawCircleV(point(0,3),11*s,{72,118,119,255});
        DrawCircleV(point(-4,-1),4*s,{207,237,231,255});
        for(int i=0;i<4;++i){DrawCircleV(point(-24+i*16,kind==1?61:35),2*s,kAccent);}
        for(int i=0;i<5;++i)DrawLineEx(point(-15+i*6,-31),point(-15+i*6,-20),s,{64,60,54,255});
    }
    void WeatherIcon(Vector2 p,const LaunchWeather& weather,float s){
        if(weather.lightning){
            DrawCircleV({p.x-8*s,p.y},12*s,kMuted);DrawCircleV({p.x+7*s,p.y-4*s},15*s,kMuted);
            DrawLineEx({p.x+3*s,p.y+9*s},{p.x-5*s,p.y+23*s},4*s,kDanger);
            DrawLineEx({p.x-5*s,p.y+23*s},{p.x+6*s,p.y+20*s},4*s,kDanger);
        }else if(weather.windKph>=25){
            for(int i=0;i<3;++i)DrawLineEx({p.x-20*s,p.y+(i-1)*8*s},{p.x+(14+i*3)*s,p.y+(i-1)*8*s},2*s,kDanger);
        }else{
            DrawCircleV(p,9*s,kGold);
            for(int i=0;i<8;++i){float a=i*PI/4;DrawLineEx({p.x+std::cos(a)*13*s,p.y+std::sin(a)*13*s},{p.x+std::cos(a)*18*s,p.y+std::sin(a)*18*s},2*s,kGold);}
        }
    }
    void RocketArt(Vector2 p,int type,float flame){
        constexpr Color kText{246,241,232,255}, kAccent{255,139,54,255}, kGold{240,189,94,255}, kDanger{244,65,65,255};
        float w=30+type*12;
        DrawRectangleRounded({
            p.x-w,p.y-100,w*2,180}
        ,0.2f,10,kText);
        DrawTriangle({
            p.x-w,p.y-90}
        ,{
            p.x+w,p.y-90}
        ,{
            p.x,p.y-150}
        ,kAccent);
        DrawRectangle(int(p.x-w),int(p.y+10),int(2*w),12,kGold);
        DrawTriangle({
            p.x-w,p.y+30}
        ,{
            p.x-w-25,p.y+95}
        ,{
            p.x-w,p.y+80}
        ,kAccent);
        DrawTriangle({
            p.x+w,p.y+80}
        ,{
            p.x+w+25,p.y+95}
        ,{
            p.x+w,p.y+30}
        ,kAccent);
        if(flame>0)DrawTriangle({
            p.x-20,p.y+80}
        ,{
            p.x,p.y+80+flame}
        ,{
            p.x+20,p.y+80}
        ,kDanger);
    }
    void Meter(const char* label,float value,float limit,float x,float y,Color color) {
        Text(label,x,y,20,kMuted);
        Text(TextFormat("%.0f / %.0f",value,limit),x,y+28,28);
        Rectangle bar{x,y+66,300,20};
        SkinPatch(Skin().barTrack,bar,8,{95,105,120,255});
        float fraction=Clamp(value/fmaxf(limit,1),0,1);
        if(fraction>0) {
            // Clip a full-size fill so tiny values keep the original rounded end shape.
            BeginScissorMode(int(x),int(y+66),int(300*fraction),20);
            SkinPatch(Skin().barFill,bar,8,color);
            EndScissorMode();
        }
    }

}
