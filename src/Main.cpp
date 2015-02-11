/**
 * @file    Main.cpp
 * @brief   My attempt at the Multiplayer CodinGame "The Great Escape".
 *
 * Copyright (c) 2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include <iostream> // NOLINT(readability/streams) std::cout used for i/o and logs
#include <string>
#include <vector>
#include <algorithm>

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 *
 * @return 0
 */
int main() {
    int w; // width of the board
    int h; // height of the board
    int playerCount; // number of players (2 or 3)
    int myId; // id of my player (0 = 1st player, 1 = 2nd player, ...)
    std::cin >> w >> h >> playerCount >> myId; std::cin.ignore();

    int turn = 0;

    // game loop
    while (1) {
        for (int i = 0; i < playerCount; i++) {
            int x; // x-coordinate of the player
            int y; // y-coordinate of the player
            int wallsLeft; // number of walls available for the player
            std::cin >> x >> y >> wallsLeft; std::cin.ignore();
            if (i == myId) {
                std::cerr << "me("<< i <<"): [" << x << ", " << y << "]\n";
            } else {
                std::cerr << "player("<< i <<"): [" << x << ", " << y << "]\n";
            }
        }
        int wallCount; // number of walls on the board
        std::cin >> wallCount; std::cin.ignore();
        for (int i = 0; i < wallCount; i++) {
            int wallX; // x-coordinate of the wall
            int wallY; // y-coordinate of the wall
            std::string wallOrientation; // wall orientation ('H' or 'V')
            std::cin >> wallX >> wallY >> wallOrientation; std::cin.ignore();
            std::cerr << "wall("<< i <<"): [" << wallX << ", " << wallY << "]\n";
        }

        // Write an action using std::cout. DON'T FORGET THE "<< std::endl"
        // To debug: std::cerr << "Debug messages..." << std::endl;

        ++turn;
        std::cerr << "turn " << turn << std::endl;

        // action: LEFT, RIGHT, UP, DOWN or "putX putY putOrientation" to place a wall
        std::cout << "RIGHT" << std::endl;
    }

    return 0;
}
