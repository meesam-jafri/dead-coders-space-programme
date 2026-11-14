#include "raylib.h"
#include "raymath.h"

#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace mission {

constexpr int kWidth = 1600;
constexpr int kHeight = 900;
constexpr int kStageCount = 11;
constexpr float kTransitionSeconds = 0.22f;
constexpr Color kBackground{8, 15, 29, 255};
constexpr Color kPanel{17, 30, 48, 255};
constexpr Color kBorder{44, 65, 88, 255};
constexpr Color kText{232, 241, 250, 255};
constexpr Color kMuted{151, 174, 196, 255};
constexpr Color kAccent{98, 230, 201, 255};
constexpr Color kGold{250, 199, 113, 255};

// This order is the finalized README flow. Main Menu is stage 01.
enum class Stage {
    MainMenu, SelectMission, SelectSpacecraft, MakeTeam, SelectPayloads,
    SelectRocket, SelectOrbit, SelectLaunchDate, Launch, Operations, Results
};
enum class Overlay { None, HowToPlay, Archive, Settings };

struct StageInfo {
    const char* title;
    const char* shortName;
    const char* description;
    const char* planned;
};
constexpr std::array<StageInfo, kStageCount> kStages{{
    {"Mission: Lunar Recon", "Main menu", "Your agency. Your spacecraft. Your mission.", ""},
    {"Select Mission", "Mission", "Choose the destination for your next science mission.", ""},
    {"Select Spacecraft", "Spacecraft", "Choose a robotic chassis. Its limits will shape your future payload.", ""},
    {"Make Your Team", "Team", "Build the ground team behind your spacecraft.",
     "Recruit scientists, engineers and communications operators within a staffing budget."},
    {"Select Payloads", "Payloads", "Equip your spacecraft for the mission ahead.",
     "Fit instruments and support systems within the chosen chassis mass, slot and power limits."},
    {"Select Rocket", "Rocket", "Choose a ride that can deliver your spacecraft.",
     "Compare rocket costs, destination compatibility and payload delivery capability."},
    {"Select Orbit", "Orbit", "Plan where your spacecraft will gather science.",
     "Choose a compatible lunar science orbit, observation coverage and simplified fuel needs."},
    {"Select Launch Date", "Launch date", "Find a window for your mission to begin.",
     "Compare launch windows, weather risks, readiness and possible delays."},
    {"Launch", "Launch", "Take your design from the launch pad to lunar orbit.",
     "Verify readiness, then experience countdown, ascent, separation, transfer and insertion."},
    {"Mission Operations", "Operations", "Keep your spacecraft safe and bring its discoveries home.",
     "Manage instruments, power, data, communications and anomalies with help from your team."},
    {"Mission Results", "Results", "Discover what your design achieved.",
     "Evaluate returned science, requirements, spacecraft safety and cost efficiency."}
}};
static_assert(static_cast<int>(Stage::Results) + 1 == kStageCount, "Keep all 11 stages in order.");

struct MissionConfig {
    const char* name;
    const char* subtitle;
    const char* objective;
    bool unlocked;
    int budget;
    int observations;
    int returnPercent;
    Color color;
};

// FICTIONAL GAME BALANCE: credits, requirements and ALL chassis values below
// are illustrative, configurable game values, NOT NASA specifications.
constexpr std::array<MissionConfig, 3> kMissions{{
    {"Moon", "LUNAR RECON / LEVEL 01", "Map lunar terrain and scout promising exploration sites.",
     true, 1200, 10, 70, {185, 203, 219, 255}},
    {"Mars", "FUTURE MISSION", "A new frontier for your agency. Mission details to come.",
     false, 0, 0, 0, {211, 124, 96, 255}},
    {"Jupiter", "FUTURE MISSION", "Explore farther from home. Mission details to come.",
     false, 0, 0, 0, {211, 175, 135, 255}}
}};

struct SpacecraftConfig {
    const char* name;
    const char* role;
    const char* description;
    int cost;
    int payloadMassKg;
    int equipmentSlots;
    int payloadPowerW;
    int crewSeats;
    bool lunarCompatible;
    Color color;
};
constexpr std::array<SpacecraftConfig, 3> kSpacecraft{{
    {"Wren", "COMPACT SCOUT", "Low cost. A small, focused instrument suite.",
     240, 180, 3, 350, 0, true, {98, 230, 201, 255}},
    {"Kestrel", "BALANCED SURVEYOR", "More room for a versatile science mission.",
     460, 420, 5, 650, 0, true, {128, 182, 255, 255}},
    {"Albatross", "HEAVY OBSERVER", "Largest capacity. Less budget left for later.",
     680, 800, 7, 1000, 0, true, {250, 199, 113, 255}}
}};

struct GameState {
    Stage stage = Stage::MainMenu;
    Overlay overlay = Overlay::None;
    // Session persistence: Back, Next and the menu never clear these choices.
    // No save file is written; closing the application ends the session.
    int missionIndex = -1;
    int spacecraftIndex = -1;
    float transition = 0.0f;
    float selectionPulse = 0.0f;
    float noticeTime = 0.0f;
    std::string notice;
    bool soundEnabled = true;
};

bool HasMission(const GameState& game) {
    return game.missionIndex >= 0 && game.missionIndex < static_cast<int>(kMissions.size())
        && kMissions[game.missionIndex].unlocked;
}

bool CanChooseSpacecraft(const GameState& game, int index) {
    if (!HasMission(game) || index < 0 || index >= static_cast<int>(kSpacecraft.size())) return false;
    const auto& craft = kSpacecraft[index];
    return game.missionIndex == 0 && craft.lunarCompatible && craft.crewSeats == 0
        && craft.cost <= kMissions[game.missionIndex].budget;
}

bool HasSpacecraft(const GameState& game) {
    return CanChooseSpacecraft(game, game.spacecraftIndex);
}

bool CanAdvance(const GameState& game) {
    if (game.stage == Stage::Results) return false;
    if (game.stage == Stage::MainMenu) return true;
    if (!HasMission(game)) return false;
    return game.stage == Stage::SelectMission || HasSpacecraft(game);
}

struct AudioFeedback {
    Sound select{};
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
        return LoadSoundFromWave(wave); // raylib copies the samples.
    }

    void Init() {
        InitAudioDevice();
        ready = IsAudioDeviceReady();
        if (ready) {
            select = MakeTone(740.0f);
            navigate = MakeTone(520.0f);
        }
    }
    void Play(const GameState& game, bool selection = false) const {
        if (ready && game.soundEnabled) PlaySound(selection ? select : navigate);
    }
    void Close() {
        if (ready) {
            UnloadSound(select);
            UnloadSound(navigate);
            CloseAudioDevice();
        }
    }
};

struct Input {
    Vector2 mouse;
    bool clicked;
    bool enabled;
};

void Text(const std::string& text, float x, float y, int size = 20, Color color = kText) {
    DrawText(text.c_str(), static_cast<int>(x), static_cast<int>(y), size, color);
}

void WrappedText(const std::string& text, float x, float y, int width, int size, Color color) {
    std::string line;
    std::string word;
    auto appendWord = [&]() {
        const std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && MeasureText(candidate.c_str(), size) > width) {
            Text(line, x, y, size, color);
            y += size + 9;
            line = word;
        } else line = candidate;
        word.clear();
    };
    for (char ch : text) {
        if (ch == ' ') appendWord();
        else word += ch;
    }
    appendWord();
    Text(line, x, y, size, color);
}

void Panel(Rectangle bounds, Color fill = kPanel, Color border = kBorder) {
    DrawRectangleRounded(bounds, 0.035f, 8, fill);
    DrawRectangleRoundedLinesEx(bounds, 0.035f, 8, 1.0f, border);
}

bool Hovered(Rectangle bounds, const Input& input) {
    return input.enabled && CheckCollisionPointRec(input.mouse, bounds);
}

bool Button(Rectangle bounds, const char* label, const Input& input,
            bool enabled = true, bool primary = false) {
    const bool hover = enabled && Hovered(bounds, input);
    if (hover) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    const Color fill = !enabled ? Color{23, 33, 47, 255}
        : primary ? (hover ? Color{152, 248, 224, 255} : kAccent)
        : (hover ? Color{39, 61, 81, 255} : kPanel);
    Panel(bounds, fill, enabled ? (hover ? kAccent : kBorder) : kBorder);
    const Color ink = !enabled ? Color{103, 122, 141, 255} : primary ? kBackground : kText;
    Text(label, bounds.x + (bounds.width - MeasureText(label, 20)) / 2,
         bounds.y + (bounds.height - 20) / 2, 20, ink);
    return enabled && hover && input.clicked;
}

void Notify(GameState& game, const std::string& message) {
    game.notice = message;
    game.noticeTime = 3.0f;
}

void ChangeStage(GameState& game, Stage next, const AudioFeedback& audio) {
    game.stage = next;
    game.transition = 1.0f;
    game.noticeTime = 0.0f;
    game.overlay = Overlay::None;
    audio.Play(game);
}

void Advance(GameState& game, const AudioFeedback& audio) {
    if (CanAdvance(game)) ChangeStage(game, static_cast<Stage>(static_cast<int>(game.stage) + 1), audio);
}

void SelectMission(GameState& game, int index, const AudioFeedback& audio) {
    if (index < 0 || index >= static_cast<int>(kMissions.size())) return;
    if (!kMissions[index].unlocked) {
        Notify(game, "This destination is locked. Moon is available in this build.");
        return;
    }
    game.missionIndex = index;
    if (!HasSpacecraft(game)) game.spacecraftIndex = -1;
    game.selectionPulse = 1.0f;
    Notify(game, "Moon mission selected. Your next step is spacecraft selection.");
    audio.Play(game, true);
}

void SelectSpacecraft(GameState& game, int index, const AudioFeedback& audio) {
    if (!CanChooseSpacecraft(game, index)) {
        Notify(game, "Choose an unlocked mission and a compatible, affordable robotic spacecraft.");
        return;
    }
    game.spacecraftIndex = index;
    game.selectionPulse = 1.0f;
    Notify(game, std::string(kSpacecraft[index].name) + " selected. Capacity limits saved for your design.");
    audio.Play(game, true);
}

void DrawSpacecraft(Vector2 center, float scale, Color color, float time, int variant) {
    center.y += std::sin(time * 1.6f) * 5.0f * scale;
    const float wing = (70.0f + variant * 13.0f) * scale;
    DrawLineEx({center.x - wing - 35 * scale, center.y},
               {center.x + wing + 35 * scale, center.y}, 5 * scale, kMuted);
    for (int side : {-1, 1}) {
        const float x = side < 0 ? center.x - wing - 36 * scale : center.x + 36 * scale;
        Rectangle panel{x, center.y - 27 * scale, wing, 54 * scale};
        DrawRectangleRec(panel, Color{32, 67, 105, 255});
        DrawRectangleLinesEx(panel, 2 * scale, color);
        for (int cell = 1; cell < 5; ++cell)
            DrawLineEx({x + wing * cell / 5, panel.y},
                       {x + wing * cell / 5, panel.y + panel.height}, scale, Fade(color, 0.55f));
        DrawLineEx({x, center.y}, {x + wing, center.y}, scale, Fade(color, 0.55f));
    }
    DrawRectangleRounded({center.x - 30 * scale, center.y - 40 * scale, 60 * scale, 80 * scale},
                         0.2f, 6, Color{171, 189, 204, 255});
    DrawRectangleRec({center.x - 21 * scale, center.y - 28 * scale, 42 * scale, 46 * scale}, color);
    DrawCircleV({center.x, center.y + 26 * scale}, 9 * scale, kBackground);
    DrawLineEx({center.x, center.y - 40 * scale}, {center.x + 12 * scale, center.y - 62 * scale},
               3 * scale, kMuted);
    DrawCircleV({center.x + 12 * scale, center.y - 62 * scale}, 6 * scale, color);
}

void DrawPlanet(Vector2 center, float radius, int index, float time) {
    const auto color = kMissions[index].color;
    DrawCircleV(center, radius + 10 + std::sin(time) * 2, Fade(color, 0.05f));
    DrawCircleV(center, radius, color);
    DrawCircleV({center.x - radius * 0.18f, center.y + radius * 0.08f}, radius * 0.18f, Fade(kBackground, 0.2f));
    DrawCircleV({center.x + radius * 0.35f, center.y - radius * 0.3f}, radius * 0.24f, Fade(kBackground, 0.15f));
    DrawCircleV({center.x - radius * 0.37f, center.y - radius * 0.43f}, radius * 0.1f, Fade(kBackground, 0.18f));
    DrawCircleV({center.x + radius * 0.25f, center.y + radius * 0.5f}, radius * 0.12f, Fade(kBackground, 0.18f));
}

void DrawBackground(float time) {
    ClearBackground(kBackground);
    DrawRectangleGradientV(0, 0, kWidth, kHeight, Color{14, 27, 46, 255}, kBackground);
    for (int i = 0; i < 100; ++i) {
        const int x = (i * 491 + 71) % kWidth;
        const int y = (i * 283 + 47) % kHeight;
        const float opacity = 0.14f + 0.13f * (1 + std::sin(time * 0.6f + i));
        DrawCircle(x, y, i % 5 == 0 ? 1.7f : 1.0f, Fade(kMuted, opacity));
    }
}

void DrawHeader(GameState& game, const Input& input, const AudioFeedback& audio) {
    Text("DC / SPACE PROGRAMME", 56, 28, 22, kAccent);
    Text("MISSION DESIGN LAB", 56, 59, 15, kMuted);
    std::string summary = HasMission(game) ? std::string("MISSION  ") + kMissions[game.missionIndex].name : "MISSION  Not selected";
    summary += HasSpacecraft(game) ? std::string("     /     CRAFT  ") + kSpacecraft[game.spacecraftIndex].name : "     /     CRAFT  Not selected";
    Text(summary, 610, 35, 18, kMuted);
    if (Button({1324, 22, 220, 48}, !audio.ready ? "Audio unavailable" : game.soundEnabled ? "Sound: ON [M]" : "Sound: OFF [M]", input, audio.ready)) {
        game.soundEnabled = !game.soundEnabled;
        audio.Play(game, true);
    }
    const auto& info = kStages[static_cast<int>(game.stage)];
    Text(info.title, 56, 100, 38);
    Text(info.description, 58, 149, 20, kMuted);
    const int current = static_cast<int>(game.stage);
    for (int i = 0; i < kStageCount; ++i) {
        const float x = 74.0f + i * 140.0f;
        const Color color = i == current ? kAccent : i < current ? Color{98, 145, 157, 255} : kBorder;
        if (i < kStageCount - 1) DrawLineEx({x + 19, 205}, {x + 121, 205}, 2, i < current ? color : kBorder);
        DrawCircleV({x, 205}, 16, i == current ? kAccent : kPanel);
        DrawCircleLines(static_cast<int>(x), 205, 16, color);
        const std::string number = std::to_string(i + 1);
        Text(number, x - MeasureText(number.c_str(), 16) / 2, 197, 16, i == current ? kBackground : kMuted);
        Text(kStages[i].shortName, x - 17, 230, 15, i == current ? kAccent : kMuted);
    }
}

bool SelectionCard(Rectangle bounds, bool selected, bool available, const GameState& game, const Input& input) {
    const bool hover = Hovered(bounds, input);
    if (hover) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    const Color border = selected ? kAccent : hover ? kMuted : kBorder;
    Panel(bounds, selected ? Color{19, 43, 54, 255} : kPanel, border);
    if (selected) {
        DrawRectangleRoundedLinesEx(bounds, 0.035f, 8, 2 + game.selectionPulse * 2, Fade(kAccent, 0.8f));
        Text("SELECTED", bounds.x + bounds.width - 125, bounds.y + 20, 17, kAccent);
    } else Text(available ? "CLICK TO SELECT" : "LOCKED", bounds.x + bounds.width - 165, bounds.y + 20, 16, available ? kMuted : kGold);
    return hover && input.clicked;
}

void DrawMissionSelection(GameState& game, const Input& input, const AudioFeedback& audio, float time) {
    for (int i = 0; i < static_cast<int>(kMissions.size()); ++i) {
        const auto& config = kMissions[i];
        const float x = 56.0f + i * 504.0f;
        if (SelectionCard({x, 275, 480, 447}, game.missionIndex == i, config.unlocked, game, input)) SelectMission(game, i, audio);
        Text(std::to_string(i + 1), x + 24, 295, 18, kMuted);
        DrawPlanet({x + 240, 390 + std::sin(time + i) * 4}, 57, i, time);
        Text(config.name, x + 26, 472, 32);
        Text(config.subtitle, x + 26, 515, 16, config.unlocked ? kAccent : kGold);
        WrappedText(config.objective, x + 26, 550, 425, 21, kMuted);
        if (config.unlocked) {
            Text("Difficulty: Introductory", x + 26, 633, 19);
            Text("Budget: " + std::to_string(config.budget) + " credits", x + 26, 672, 22, kAccent);
        } else {
            Text("Unavailable in this build", x + 26, 643, 20, kGold);
            Text("Later missions stay locked.", x + 26, 678, 18, kMuted);
        }
    }
    Panel({56, 738, 1488, 65});
    const auto& moon = kMissions[0];
    Text("LUNAR SUCCESS TARGETS", 78, 750, 16, kAccent);
    Text(std::to_string(moon.observations) + " observations  /  Return " + std::to_string(moon.returnPercent) + "% of required data  /  Safe power  /  Compatible orbit  /  Within budget", 78, 777, 18);
}

void StatRow(float x, float y, const char* label, const std::string& value, Color color = kText) {
    Text(label, x, y, 19, kMuted);
    Text(value, x + 424 - MeasureText(value.c_str(), 20), y, 20, color);
}

void DrawSpacecraftSelection(GameState& game, const Input& input, const AudioFeedback& audio, float time) {
    for (int i = 0; i < static_cast<int>(kSpacecraft.size()); ++i) {
        const auto& craft = kSpacecraft[i];
        const float x = 56.0f + i * 504.0f;
        if (SelectionCard({x, 275, 480, 447}, game.spacecraftIndex == i, CanChooseSpacecraft(game, i), game, input)) SelectSpacecraft(game, i, audio);
        Text(std::to_string(i + 1), x + 24, 295, 18, kMuted);
        DrawSpacecraft({x + 240, 388}, 0.85f, craft.color, time, i);
        Text(craft.name, x + 26, 453, 30);
        Text(craft.role, x + 26, 491, 16, craft.color);
        Text(craft.description, x + 26, 522, 17, kMuted);
        StatRow(x + 26, 557, "Max payload mass", std::to_string(craft.payloadMassKg) + " kg");
        StatRow(x + 26, 588, "Equipment slots", std::to_string(craft.equipmentSlots));
        StatRow(x + 26, 619, "Payload power budget", std::to_string(craft.payloadPowerW) + " W");
        StatRow(x + 26, 650, "Crew seats / destination", std::to_string(craft.crewSeats) + " / Moon");
        StatRow(x + 26, 686, "Chassis cost", std::to_string(craft.cost) + " credits", craft.color);
    }
    Panel({56, 738, 1488, 65});
    Text("HARD LIMITS", 78, 750, 16, kAccent);
    Text("Future payloads must fit ALL mass, slot and power limits. Robotic only: ground team, no onboard crew.", 78, 777, 18);
    if (HasSpacecraft(game)) {
        const int remaining = kMissions[game.missionIndex].budget - kSpacecraft[game.spacecraftIndex].cost;
        Text("After chassis: " + std::to_string(remaining) + " credits left", 1030, 750, 18, kGold);
    }
}

void DrawMenu(GameState& game, const Input& input, const AudioFeedback& audio, float time) {
    Panel({56, 275, 790, 528});
    Text("YOUR FIRST ASSIGNMENT", 88, 308, 17, kAccent);
    Text("Design a mission.", 88, 355, 47);
    Text("Discover the Moon.", 88, 413, 47);
    WrappedText("Lead a small space agency through eleven stages, from your first design decision to the science you bring home.", 90, 491, 685, 24, kMuted);
    const char* start = HasMission(game) ? "Review mission design" : "Start mission";
    if (Button({88, 607, 330, 62}, start, input, true, true)) Advance(game, audio);
    Text("TASK 01  /  FOUNDATION PREVIEW", 88, 695, 17, kAccent);
    Text("Mission and spacecraft selection are playable.", 88, 729, 20, kMuted);
    Text("The remaining stages are labeled previews.", 88, 761, 20, kMuted);
    DrawEllipseLines(1200, 487, 280, 170, kBorder);
    DrawPlanet({1235, 461}, 112, 0, time);
    DrawSpacecraft({1094, 536}, 1.35f, kAccent, time, 1);
    Text("LUNAR ORBITAL RECONNAISSANCE", 938, 666, 20, kMuted);
    if (Button({886, 733, 204, 55}, "How to play", input)) game.overlay = Overlay::HowToPlay;
    if (Button({1107, 733, 217, 55}, "Mission archive", input)) game.overlay = Overlay::Archive;
    if (Button({1341, 733, 203, 55}, "Settings", input)) game.overlay = Overlay::Settings;
}

void DrawPlaceholder(GameState& game, const Input& input, const AudioFeedback& audio, float time) {
    const auto& info = kStages[static_cast<int>(game.stage)];
    Panel({56, 275, 940, 528});
    Text("STAGE PREVIEW / NOT IMPLEMENTED YET", 88, 312, 19, kGold);
    Text(info.title, 88, 370, 40);
    WrappedText(info.planned, 88, 444, 835, 26, kMuted);
    Text("Your mission design is retained as you navigate.", 88, 575, 22);
    WrappedText("Use Back and Next to explore the finalized flow. This preview does not run a simulation, spend credits or award results.", 88, 619, 820, 22, kMuted);
    if (game.stage == Stage::Results) {
        if (Button({88, 726, 290, 52}, "Review design", input, true, true)) ChangeStage(game, Stage::SelectMission, audio);
        Text("No mission has been flown yet.", 405, 742, 20, kGold);
    }
    Panel({1020, 275, 524, 528});
    Text("CURRENT DESIGN", 1052, 310, 18, kAccent);
    if (HasMission(game)) Text(std::string(kMissions[game.missionIndex].name) + " / Orbital reconnaissance", 1052, 352, 22);
    if (HasSpacecraft(game)) {
        const auto& craft = kSpacecraft[game.spacecraftIndex];
        DrawSpacecraft({1280, 477}, 1.1f, craft.color, time, game.spacecraftIndex);
        Text(craft.name, 1052, 565, 32, craft.color);
        StatRow(1052, 622, "Payload mass limit", std::to_string(craft.payloadMassKg) + " kg");
        StatRow(1052, 659, "Slots / payload power", std::to_string(craft.equipmentSlots) + " / " + std::to_string(craft.payloadPowerW) + " W");
        StatRow(1052, 696, "Budget after chassis", std::to_string(kMissions[game.missionIndex].budget - craft.cost) + " credits");
        Text("Robotic spacecraft / Earth-based team", 1052, 755, 20, kMuted);
    }
}

void DrawFooter(GameState& game, const Input& input, const AudioFeedback& audio) {
    DrawLine(56, 818, 1544, 818, kBorder);
    if (Button({56, 835, 165, 48}, "<  Back", input, game.stage != Stage::MainMenu))
        ChangeStage(game, static_cast<Stage>(static_cast<int>(game.stage) - 1), audio);
    const std::string position = "STAGE " + std::to_string(static_cast<int>(game.stage) + 1) + " / 11";
    Text(position, 246, 838, 18, kAccent);
    Text("Arrows / Enter: navigate    1-3: select    Esc: menu", 430, 838, 17, kMuted);
    Text("Fictional game values, not NASA specifications. Choices persist for this session.", 246, 867, 16, kMuted);
    const char* label = game.stage == Stage::MainMenu ? "Start mission  >" : game.stage == Stage::Results ? "End of preview" : "Next  >";
    if (Button({1314, 835, 230, 48}, label, input, CanAdvance(game), true)) Advance(game, audio);
    if (!CanAdvance(game) && game.stage != Stage::Results)
        Text(game.stage == Stage::SelectMission ? "Select Moon to continue" : "Select a craft to continue", 1035, 850, 18, kGold);
}

void DrawOverlay(GameState& game, const Input& input, const AudioFeedback& audio) {
    DrawRectangle(0, 0, kWidth, kHeight, Fade(BLACK, 0.7f));
    Panel({370, 260, 860, 430}, Color{20, 36, 55, 255}, kAccent);
    const char* title = game.overlay == Overlay::HowToPlay ? "How to play" : game.overlay == Overlay::Archive ? "Mission archive" : "Settings";
    Text(title, 410, 300, 34);
    if (game.overlay == Overlay::HowToPlay) {
        WrappedText("Choose Moon, then compare three fictional robotic spacecraft. Each has different mass, equipment and power limits. Back and Next retain your choices.", 410, 365, 770, 23, kText);
        WrappedText("Use the mouse or keys 1-3 to select. Left / Right or Enter navigate. Esc closes this panel or returns to the menu. Stages 4-11 are previews.", 410, 490, 770, 22, kMuted);
    } else if (game.overlay == Overlay::Archive) {
        Text("No completed missions yet.", 410, 380, 27, kAccent);
        WrappedText("Results and mission history will become available when mission operations are implemented. Your current selections stay available during this session.", 410, 438, 770, 23, kMuted);
    } else {
        WrappedText("The interface scales to a 16:9 canvas. Resize the window freely; unused space is letterboxed.", 410, 365, 770, 23, kMuted);
        if (Button({410, 470, 340, 54}, !audio.ready ? "Audio unavailable" : game.soundEnabled ? "Sound: ON" : "Sound: OFF", input, audio.ready)) {
            game.soundEnabled = !game.soundEnabled;
            audio.Play(game, true);
        }
        Text("Selection tones are generated in memory.", 410, 550, 21, kMuted);
    }
    if (Button({1010, 612, 180, 48}, "Close [Esc]", input, true, true)) game.overlay = Overlay::None;
}

void HandleKeyboard(GameState& game, const AudioFeedback& audio) {
    if (IsKeyPressed(KEY_M) && audio.ready) {
        game.soundEnabled = !game.soundEnabled;
        audio.Play(game, true);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (game.overlay != Overlay::None) game.overlay = Overlay::None;
        else if (game.stage != Stage::MainMenu) ChangeStage(game, Stage::MainMenu, audio);
        return;
    }
    if (game.overlay != Overlay::None || game.transition > 0.0f) return;
    for (int i = 0; i < 3; ++i) {
        if (IsKeyPressed(KEY_ONE + i) || IsKeyPressed(KEY_KP_1 + i)) {
            if (game.stage == Stage::SelectMission) SelectMission(game, i, audio);
            else if (game.stage == Stage::SelectSpacecraft) SelectSpacecraft(game, i, audio);
        }
    }
    if (IsKeyPressed(KEY_LEFT) && game.stage != Stage::MainMenu)
        ChangeStage(game, static_cast<Stage>(static_cast<int>(game.stage) - 1), audio);
    else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
        Advance(game, audio);
}

} // namespace mission

int main() {
    using namespace mission;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Mission: Lunar Recon | Dead Coders Space Programme");
    if (!IsWindowReady()) return 1;
    SetWindowMinSize(960, 540);
    SetExitKey(KEY_NULL); // Esc navigates; the window close button exits.
    SetTargetFPS(60);

    RenderTexture2D canvas = LoadRenderTexture(kWidth, kHeight);
    if (canvas.id == 0) {
        CloseWindow();
        return 1;
    }
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);
    AudioFeedback audio;
    audio.Init();
    GameState game;

    while (!WindowShouldClose()) {
        const float dt = Clamp(GetFrameTime(), 0.0f, 0.1f);
        const float time = static_cast<float>(GetTime());
        game.transition = Clamp(game.transition - dt / kTransitionSeconds, 0.0f, 1.0f);
        game.selectionPulse = Clamp(game.selectionPulse - dt * 2.0f, 0.0f, 1.0f);
        game.noticeTime = Clamp(game.noticeTime - dt, 0.0f, 3.0f);
        HandleKeyboard(game, audio);

        // Use the SAME letterbox transform for drawing and mouse hit testing.
        const float scale = fmaxf(0.001f, fminf(static_cast<float>(GetScreenWidth()) / kWidth,
                                              static_cast<float>(GetScreenHeight()) / kHeight));
        const Rectangle destination{(GetScreenWidth() - kWidth * scale) * 0.5f,
                                    (GetScreenHeight() - kHeight * scale) * 0.5f,
                                    kWidth * scale, kHeight * scale};
        const Vector2 mouse = GetMousePosition();
        const Input input{{(mouse.x - destination.x) / scale, (mouse.y - destination.y) / scale},
                          IsMouseButtonPressed(MOUSE_BUTTON_LEFT),
                          game.overlay == Overlay::None && game.transition <= 0.0f};
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        BeginTextureMode(canvas);
        DrawBackground(time);
        DrawHeader(game, input, audio);
        // Snapshot the stage so a click cannot navigate twice during one frame.
        const Stage drawnStage = game.stage;
        switch (drawnStage) {
            case Stage::MainMenu: DrawMenu(game, input, audio, time); break;
            case Stage::SelectMission: DrawMissionSelection(game, input, audio, time); break;
            case Stage::SelectSpacecraft: DrawSpacecraftSelection(game, input, audio, time); break;
            default: DrawPlaceholder(game, input, audio, time); break;
        }
        Input footerInput = input;
        footerInput.enabled = input.enabled && game.stage == drawnStage && game.overlay == Overlay::None;
        DrawFooter(game, footerInput, audio);
        if (game.transition > 0) DrawRectangle(0, 264, kWidth, 550, Fade(kBackground, game.transition * 0.6f));
        if (game.noticeTime > 0 && game.overlay == Overlay::None) {
            const int width = MeasureText(game.notice.c_str(), 19) + 48;
            Panel({(kWidth - width) * 0.5f, 141, static_cast<float>(width), 42}, kPanel, kAccent);
            Text(game.notice, (kWidth - width) * 0.5f + 24, 153, 19, kAccent);
        }
        if (game.overlay != Overlay::None) {
            Input overlayInput = input;
            overlayInput.enabled = true;
            DrawOverlay(game, overlayInput, audio);
        }
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(canvas.texture, {0, 0, static_cast<float>(kWidth), -static_cast<float>(kHeight)},
                       destination, {0, 0}, 0, WHITE);
        EndDrawing();
    }

    audio.Close();
    UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}
