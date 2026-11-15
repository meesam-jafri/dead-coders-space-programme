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
    bool enabled, bool primary) {
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
        SkinPatch(face,visual,18,!enabled?Color{44,43,43,255}:primary?kAccent:hover?Color{110,85,65,255}:Color{64,62,61,255});
        const Color ink=!enabled?kMuted:primary?kBackground:kText;
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
        DrawRectangleRounded(track,0.6f,8,{48,43,39,255});
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
}