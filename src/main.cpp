#include "raylib.h"
#include <iostream>
#include <string>
using namespace std;

int main(){
    InitWindow(1080,720,"game");

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(WHITE);
        DrawText("Raylib is working !!!", 0, 0, 100, BLACK);
        

        EndDrawing();

    }


    return 0;
}
 