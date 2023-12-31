#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include "../include/stdafx.hpp"
#include "../include/mp3player.hpp"
#include "../include/raylib.h"
#include "../include/raymath.h"
bool paused=true;
Rectangle prevButton = { 50, 400, 100, 40 };
Rectangle pauseButton = { 200, 400, 100, 40 };
Rectangle nextButton = { 350, 400, 100, 40 };
Rectangle testButton = { 500, 400, 100, 40 };

inline void send_signal(int sig){
    Mp3Player::signal =  sig;
    Mp3Player::recvdSignal = true;
}


void process_button_events(){
  if (CheckCollisionPointRec(GetMousePosition(), prevButton) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            send_signal(4);
            printf("Previous Song\n");
        }
        if (CheckCollisionPointRec(GetMousePosition(), pauseButton) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            Mp3Player::signal = paused ? 2 : 1;
            Mp3Player::recvdSignal = true;
            if(paused){
                send_signal(2);
                paused = false;
            }
            else {
                send_signal(1);
                paused = true;
            }
            printf("pause\n");
        }
        if (CheckCollisionPointRec(GetMousePosition(), nextButton) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            send_signal(3);
            printf("Next Song\n");
        }
        if (CheckCollisionPointRec(GetMousePosition(), testButton) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            Mp3Player::setVolumePercent(50.0f);
            printf("Set Volume to half\n");
        }
 
}


int main(int argc, char** argv)
{
/*
    std::thread signalThread(Mp3Player::processSignals); 
    // std::cout 
    while(1){
        std::cin >> Mp3Player::signal;
        Mp3Player::recvdSignal = true;

        if(Mp3Player::signal == 7){
            signalThread.join();
            break;
        }
    std::timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 10000;
    nanosleep(&ts, nullptr);
    }
    return 0;
*/

        // Initialization
    std::thread signalThread(Mp3Player::processSignals); 
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Hello Raylib");
    SetTargetFPS(60);
    // Main game loop
    while (!WindowShouldClose())
    {
        float scale = std::min((float)GetScreenWidth()/screenWidth, (float)GetScreenHeight()/screenHeight);
        Vector2 mouse = GetMousePosition();
        
        Vector2 virtualMouse = { 0 };
        virtualMouse.x = (mouse.x - (GetScreenWidth() - (screenWidth*scale))*0.5f)/scale;
        virtualMouse.y = (mouse.y - (GetScreenHeight() - (screenHeight*scale))*0.5f)/scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)screenWidth, (float)screenHeight });
        process_button_events(); 

        BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangleRec(prevButton, LIGHTGRAY);
        DrawText("Prev", prevButton.x + 10, prevButton.y + 10, 20, BLUE);
        DrawRectangleRec(pauseButton, LIGHTGRAY);
        DrawText("Start", pauseButton.x + 10, pauseButton.y + 10, 20, BLUE);
        DrawRectangleRec(nextButton, LIGHTGRAY);
        DrawText("Next", nextButton.x + 10, nextButton.y + 10, 20, BLUE);
        DrawRectangleRec(testButton, LIGHTGRAY);
        std::string currSongStr = Mp3Player::currentSong.songname;
        DrawText(currSongStr.c_str(), screenWidth/2, 60, 20, BLUE);
        EndDrawing();
    } 
    send_signal(7);
    signalThread.join();
    // De-Initialization
    CloseWindow();
}



/*
MIT License

Copyright (c) 2023 Robin Krämer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/