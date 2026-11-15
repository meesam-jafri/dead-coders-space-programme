#include "game.hpp"
namespace mission {
    float TextSpacing(int size) {
        return size / 22.0f;
    }
    int TextWidth(const char* text, int size) {
        return static_cast<int>(std::ceil(MeasureTextEx(Typography().ForSize(size), text,
        static_cast<float>(size), TextSpacing(size)).x));
    }
    void Text(const std::string& text, float x, float y, int size, Color color) {
        DrawTextEx(Typography().ForSize(size), text.c_str(), {
            x, y}
        , static_cast<float>(size), TextSpacing(size), color);
    }
    void WrappedText(const std::string& text, float x, float y, int width, int size, Color color) {
        std::string line;
        std::string word;
        auto appendWord = [&]() {
            const std::string candidate = line.empty() ? word : line + " " + word;
            if (!line.empty() && TextWidth(candidate.c_str(), size) > width) {
                Text(line, x, y, size, color);
                y += size + 9;
                line = word;
            }
            else line = candidate;
            word.clear();
        };
        for (char ch : text) {
            if (ch == ' ') appendWord();
            else word += ch;
        }
        appendWord();
        Text(line, x, y, size, color);
    }
    void Panel(Rectangle bounds, Color fill, Color border) {
        DrawRectangleRounded({bounds.x+3,bounds.y+5,bounds.width,bounds.height},0.04f,8,Fade(BLACK,0.4f));
        DrawRectangleRounded(bounds,0.04f,8,fill);
        DrawRectangleRoundedLinesEx(bounds,0.04f,8,1.5f,border);
    }
    bool Hovered(Rectangle bounds, const Input& input) {
        return input.enabled && CheckCollisionPointRec(input.mouse, bounds);
    }
    bool Button(Rectangle bounds, const char* label, const Input& input,
    bool enabled, bool primary, bool selected) {
        const bool hover = enabled && Hovered(bounds, input);
        if (hover) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        struct HoverAnimation {
            Rectangle bounds;
            float amount;
        };
        static std::vector<HoverAnimation> animations;
        HoverAnimation* animation = nullptr;
        for (auto& item : animations) {
            if (item.bounds.x == bounds.x && item.bounds.y == bounds.y &&
            item.bounds.width == bounds.width && item.bounds.height == bounds.height) {
                animation = &item;
                break;
            }
        }
        if (animation == nullptr) {
            if(animations.size()>=128)animations.erase(animations.begin());
            animations.push_back({
                bounds, 0.0f}
            );
            animation = &animations.back();
        }
        animation->amount = input.reducedMotion ? (hover ? 1.0f : 0.0f)
        : Lerp(animation->amount, hover ? 1.0f : 0.0f, Clamp(GetFrameTime() * 14.0f, 0.0f, 1.0f));
        Rectangle visual=bounds;
        const float lift=input.reducedMotion?0:animation->amount*2;
        visual.y-=lift;
        if(hover&&IsMouseButtonDown(MOUSE_BUTTON_LEFT)) visual.y+=3;
        SkinPatch(Skin().normal,{visual.x+2,visual.y+4,visual.width,visual.height},18,Fade(BLACK,0.35f));
        Texture2D face=Skin().normal;
        Color faceColor=!enabled?Color{39,48,55,255}:selected?kSelected:primary?Color{178,62,42,255}:hover?Color{69,89,97,255}:Color{49,66,77,255};
        SkinPatch(face,visual,18,faceColor);
        if(selected){
            DrawRectangleRoundedLinesEx(visual,0.12f,8,2,kAccent);
            DrawCircleV({visual.x+13,visual.y+visual.height/2+4},4,kAccent);
        }else if(hover)DrawRectangleRoundedLinesEx(visual,0.12f,8,1,Fade(kText,0.55f));
        const Color ink=!enabled?Color{122,139,145,255}:kText;
        int fontSize=22;
        while(fontSize>14 && int(MeasureTextEx(Typography().button,label,float(fontSize),TextSpacing(fontSize)).x)>bounds.width-28) --fontSize;
        DrawTextEx(Typography().button,label,{visual.x+(visual.width-int(MeasureTextEx(Typography().button,label,float(fontSize),TextSpacing(fontSize)).x))/2,
        visual.y+18+(visual.height-26-fontSize)/2},float(fontSize),TextSpacing(fontSize),ink);
        if (enabled && hover && input.clicked) {
            input.clicked = false; if(uiAudio&&uiGame)uiAudio->Play(*uiGame);
            return true;
        }
        return false;
    }

    void Scrollbar(Rectangle track,float& value,float maximum,float visible,const Input& in){
        static Rectangle active{};static float grab=0;static bool dragging=false;
        float thumbHeight=std::max(32.0f,track.height*visible/(maximum+visible));
        float travel=track.height-thumbHeight;
        value=Clamp(value,0,maximum);
        Rectangle thumb{track.x,track.y+(maximum>0?value/maximum*travel:0),track.width,thumbHeight};
        bool same=active.x==track.x&&active.y==track.y;
        if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT))dragging=false;
        if(in.enabled&&in.clicked&&CheckCollisionPointRec(in.mouse,track)&&maximum>0){
            in.clicked=false;active=track;same=true;dragging=true;
            grab=CheckCollisionPointRec(in.mouse,thumb)?in.mouse.y-thumb.y:thumbHeight/2;
        }
        if(dragging&&same&&in.enabled&&travel>0){value=Clamp((in.mouse.y-track.y-grab)/travel,0,1)*maximum;thumb.y=track.y+value/maximum*travel;}
        DrawRectangleRounded(track,0.6f,8,kPanel);
        DrawRectangleRounded(thumb,0.6f,8,maximum>0?kAccent:kBorder);
        if(Hovered(track,in))SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    bool InfoButton(Rectangle bounds,const Input& in){
        bool hover=Hovered(bounds,in);
        DrawCircleV({bounds.x+bounds.width/2,bounds.y+bounds.height/2},bounds.width/2,hover?kAccent:kMuted);
        if(Skin().info.id)DrawTexturePro(Skin().info,{0,0,float(Skin().info.width),float(Skin().info.height)},
            {bounds.x-bounds.width*0.16f,bounds.y-bounds.height*0.16f,bounds.width*1.32f,bounds.height*1.32f},{0,0},0,WHITE);
        else Text("i",bounds.x+10,bounds.y+3,22,kBackground);
        if(hover)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        if(hover&&in.clicked){in.clicked=false;if(uiAudio&&uiGame)uiAudio->Play(*uiGame);return true;}return false;
    }

    namespace {
        struct Tip {
            Rectangle anchor{},panel{};
            std::string title,text;
            bool research=false;
        };
        Tip offered,pinned;
        int tipContext=-1;
        std::vector<std::string> TipLines(const std::string& text,int width,int size){
            std::istringstream words(text);std::vector<std::string> lines;std::string line,word;
            while(words>>word){
                if(!line.empty()&&TextWidth((line+" "+word).c_str(),size)>width){lines.push_back(line);line.clear();}
                if(!line.empty())line+=" ";
                line+=word;
            }
            if(!line.empty())lines.push_back(line);
            return lines;
        }
    }
    void ClearTooltips(){offered={};pinned={};}
    void BeginTooltips(Input& in,int context){
        if(context!=tipContext||IsKeyPressed(KEY_ESCAPE)){ClearTooltips();tipContext=context;}
        offered={};
        if(pinned.text.empty())return;
        if(in.clicked&&!CheckCollisionPointRec(in.mouse,pinned.panel)&&!CheckCollisionPointRec(in.mouse,pinned.anchor))pinned={};
        // A pinned popup owns pointer input inside its panel; controls underneath cannot fire.
        if(!pinned.text.empty()&&CheckCollisionPointRec(in.mouse,pinned.panel)){in.clicked=false;in.enabled=false;}
    }
    void Tooltip(const Input& in,Rectangle bounds,const char* tip,const char* title,bool research){
        if(Hovered(bounds,in))offered={bounds,{},title,tip,research};
    }
    void InfoTip(Rectangle bounds,const Input& in,const char* title,const char* text,bool research){
        if(InfoButton(bounds,in)){
            if(pinned.title==title&&pinned.anchor.x==bounds.x&&pinned.anchor.y==bounds.y)pinned={};
            else pinned={bounds,{},title,text,research};
        }
        Tooltip(in,bounds,text,title,research);
    }
    void DrawTooltips(){
        Tip& tip=pinned.text.empty()?offered:pinned;
        if(tip.text.empty())return;
        auto lines=TipLines(tip.text,472,22);
        float height=90+float(lines.size())*31+(tip.research?34:0);
        tip.panel={Clamp(tip.anchor.x+tip.anchor.width+16,18,1070),Clamp(tip.anchor.y,90,882-height),512,height};
        auto b=tip.panel;
        DrawRectangleRounded({b.x+5,b.y+7,b.width,b.height},0.04f,8,Fade(BLACK,0.55f));
        DrawRectangleRounded(b,0.04f,8,kBackground);
        DrawRectangleRoundedLinesEx(b,0.04f,8,1.5f,kAccent);
        Text(tip.title,b.x+20,b.y+17,25,kText);
        DrawLine(int(b.x+20),int(b.y+52),int(b.x+b.width-20),int(b.y+52),kBorder);
        for(size_t i=0;i<lines.size();++i)Text(lines[i],b.x+20,b.y+66+float(i)*31,22,kText);
        if(tip.research)Text("Based on NASA CubeSat 101 (2017)",b.x+20,b.y+b.height-31,17,kAccent);
    }
    void DrawCost(const GameState& game,const Input& in,float dt){
        int cost=Resources(game).cost;
        static int previous=cost,change=0;
        static float displayed=float(cost),pulse=0;
        if(previous!=cost){change=cost-previous;previous=cost;pulse=0.7f;}
        pulse=std::max(0.0f,pulse-dt);
        displayed=game.reducedMotion?float(cost):Lerp(displayed,float(cost),1-std::exp(-dt*14));
        if(std::fabs(displayed-cost)<0.5f)displayed=float(cost);
        float grow=game.reducedMotion?0:std::sin(pulse/0.7f*PI)*2;
        Rectangle box{1018-grow,9-grow,440+2*grow,64+2*grow};
        Panel(box,kPanel,cost>kBudget?kDanger:pulse>0?kAccent:kBorder);
        DrawTextEx(Typography().button,"COST",{1035,28},23,1,kMuted);
        Text(Money(int(std::round(displayed))),1110,20,34,cost>kBudget?kDanger:kText);
        Text("/ "+Money(kBudget),1280,28,24,kMuted);
        if(pulse>0)Text((change>0?"+":"")+Money(change),1360,80+(1-pulse/0.7f)*8,20,Fade(change>0?kText:kAccent,pulse/0.7f));
        Tooltip(in,box,"Ship, team, payload and launcher costs share one budget. Removing equipment refunds its price. Dollar amounts are fictional game values.","YOUR MISSION BUDGET");
    }
}
