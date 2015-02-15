/**
 * @file    Main.cpp
 * @brief   My attempt at the Multiplayer CodinGame "The Great Escape".
 *
 * Copyright (c) 2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <stdexcept>

/// Define directions
enum EDirection {
    eRight,
    eLeft,
    eDown,
    eUp
};

char toChar(const EDirection aDirection) {
    char direction;
    switch (aDirection) {
    case eRight:    direction = '>';    break;
    case eLeft:     direction = '<';    break;
    case eUp:       direction = '^';    break;
    case eDown:     direction = 'v';    break;
    default:        direction = '?';    break;
    }
    return direction;
}

/// coordinates
struct Coords {
    size_t x;   ///< x-coordinate (column)
    size_t y;   ///< y-coordinate (row/line)

    /// get coordinates of the next cell at the right of the current one
    Coords right() const {
        return Coords{ x + 1, y };
    }
    /// get coordinates of the next cell at the left of the current one
    Coords left() const {
        return Coords{ x - 1, y };
    }
    /// get coordinates of the next cell at the bottom of the current one
    Coords down() const {
        return Coords{ x, y + 1 };
    }
    /// get coordinates of the next cell at the top of the current one
    Coords up() const {
        return Coords{ x, y - 1 };
    }

    /// get coordinates of the next cell at the bottom-right of the current one
    Coords downright() const {
        return Coords{ x + 1, y + 1 };
    }
    /// get coordinates of the next cell at the bottom-left of the current one
    Coords downleft() const {
        return Coords{ x - 1, y + 1 };
    }
    /// get coordinates of the next cell at the top-right of the current one
    Coords upright() const {
        return Coords{ x + 1, y - 1 };
    }
    /// get coordinates of the next cell at the top-left of the current one
    Coords upleft() const {
        return Coords{ x - 1, y - 1 };
    }
};

/**
 * @brief Send commands to the game thru standard output
 *
 * Commands are string like LEFT, RIGHT, UP, DOWN or "putX putY putOrientation".
 */
class Command {
public:
    /// Move the player in the specified direction
    static void move(const EDirection aDirection) {
        switch (aDirection) {
        case eRight:
            right("go go go!");
            break;
        case eLeft:
            left("back home");
            break;
        case eDown:
            down("down the path...");
            break;
        case eUp:
            up("up to the sky :)");
            break;
        default:
            throw std::logic_error("move: default");
            break;
        }
    }
    /// Move the player to the right of the board (x++)
    static void right(const char* apMessage) {
        std::cout << "RIGHT " << apMessage << std::endl;
    }
    /// Move the player to the left of the board (x--)
    static void left(const char* apMessage) {
        std::cout << "LEFT " << apMessage << std::endl;
    }
    /// Move the player to the bottom of the board (y++)
    static void down(const char* apMessage) {
        std::cout << "DOWN " << apMessage << std::endl;
    }
    /// Move the player to the top of the board (y--)
    static void up(const char* apMessage) {
        std::cout << "UP " << apMessage << std::endl;
    }
    /// Put a new Wall to a specified location and orientation
    static void put(const Coords& aCoords, const char aOrientation, const char* apMessage) {
        std::cout << aCoords.x << " " << aCoords.y << " " << aOrientation << " " << apMessage << std::endl;
    }
};

/// player data
struct Player {
    /// Vector of players
    typedef std::vector<Player> Vector;

    size_t id;          ///< id (orientation) of the player
    Coords coords;      ///< coordinates of the player
    size_t wallsLeft;   ///< number of walls available for the player
    bool   bIsAlive;    ///< true while the player is alive
};

/// wall data for the list of walls
struct Wall {
    /// Vector of walls
    typedef std::vector<Wall> Vector;

    Coords coords;      ///< coordinates of the upper left corner of the wall
    char   orientation; ///< 'H'orizontal or 'V'ertical orientation
};

/// wall collision data of a cell for the matrix of walls
struct Collision {
    bool bRight;    ///< is there a wall on the right of this Cell
    bool bLeft;     ///< is there a wall on the left of this Cell
    bool bDown;     ///< is there a wall on the bottom of this Cell
    bool bUp;       ///< is there a wall on the top of this Cell

    /// Debug dump (for the bellow generic templeted Matrix::dump() method)
    void dump() const {
        std::cerr << " " << (bLeft ? '<' : ' ') << (bDown ? 'v' : ' ')
                         << (bUp ? '^' : ' ') << (bRight ? '>' : ' ') << " |";
    }
};

/// data of a cell for the matrix of pathfinding
struct Cell {
    float       weight;     ///< weighted distance toward the destination
    EDirection  direction;  ///< direction of the shortest/best path

    /// Debug dump (for the bellow generic templeted Matrix::dump() method)
    void dump() const {
        std::cerr << std::fixed << std::setprecision(1) << std::setw(4) << weight << " "
                  << toChar(direction) << "|";
    }
};

/// templated 2D matrix of generic TElement
template <typename TElement>
class Matrix {
public:
    /// Vector of matrices
    typedef std::vector<Matrix<TElement>> Vector;

    /**
     * ctor allocating the matrix to the specified size (with default initialisation)
     *
     * @param aWidthX    Nb of columns (X coordinate)
     * @param aHeightY   Nb of lines   (Y coordinate)
     */
    Matrix(const size_t aWidthX, const size_t aHeightY) {
        mMatrix.resize(aWidthX);
        for (auto& line : mMatrix) {
            line.resize(aHeightY);
        }
    }

    /**
     * ctor allocating the matrix to the specified size with explicit initialisation
     *
     * @param aWidthX    Nb of columns (X coordinate)
     * @param aHeightY   Nb of lines   (Y coordinate)
     * @param aInitValue Initial Value of every elements
     */
    Matrix(const size_t aWidthX, const size_t aHeightY, const TElement& aInitValue) {
        mMatrix.resize(aWidthX);
        for (auto& line : mMatrix) {
            line.resize(aHeightY, aInitValue);
        }
    }

    /// width of the matrix (Nb of columns, X axis)
    size_t width() const {
        return mMatrix.size();
    }
    /// height of the matrix (Nb of lines, Y axis)
    size_t height() const {
        return mMatrix.at(0).size();
    }

    /// getter for cell at [X, Y] (const reference) uses vector::at() with safety check : can throw std::out_of_range
    const TElement& get(const size_t aX, const size_t aY) const {
        return mMatrix.at(aX).at(aY); // return mMatrix[aX][aY];
    }
    /// getter for cell at [X, Y] (const reference) uses vector::at() with safety check : can throw std::out_of_range
    const TElement& get(const Coords& aCoords) const {
        return mMatrix.at(aCoords.x).at(aCoords.y);
    }
    /// "setter" for cell at [X, Y] (reference) uses vector::at() with safety check : can throw std::out_of_range
    TElement& set(const size_t aX, const size_t aY) {
        return mMatrix.at(aX).at(aY);
    }
    /// "setter" for cell at [X, Y] (reference) uses vector::at() with safety check : can throw std::out_of_range
    TElement& set(const Coords& aCoords) {
        return mMatrix.at(aCoords.x).at(aCoords.y);
    }

    /// debug: dump content of the Matrix of TElement, using a required TElement::dump() method
    void dump() {
        std::cerr << " |";
        for (size_t x = 0; x < width(); ++x) {
            std::cerr << x << "     |";
        }
        std::cerr << std::endl;
        for (size_t y = 0; y < height(); ++y) {
            std::cerr << y << "|";
            for (size_t x = 0; x < width(); ++x) {
                get(x, y).dump();
            }
            std::cerr << std::endl;
        }
    }

private:
    /// Vector of Elements
    typedef std::vector<TElement>   Vector1D;
    /// Matrix as a vector of vectors of Elements
    typedef std::vector<Vector1D>   Vector2D;

    Vector2D mMatrix;    ///< Matrix as a vector of vectors of Elements
};

/// Set a wall into the collision matrix
void setWall(Matrix<Collision>& aCollisions, const Wall& aWall) {
    // TODO(SRombauts) add those walls into the 8x8 matrix of left/right/up/down walls
    if (aWall.orientation == 'H') { // 'H' --
        // x,y-1 x+1,y-1
        // x,y   x+1,y
        aCollisions.set(aWall.coords.up())     .bDown = true;
        aCollisions.set(aWall.coords.upright()).bDown = true;
        aCollisions.set(aWall.coords)          .bUp   = true;
        aCollisions.set(aWall.coords.right())  .bUp   = true;
    } else { // 'V' |
        // x-1,y   x,y
        // x-1,y-1 x,y-1
        aCollisions.set(aWall.coords.left()).bRight     = true;
        aCollisions.set(aWall.coords.downleft()).bRight = true;
        aCollisions.set(aWall.coords).bLeft             = true;
        aCollisions.set(aWall.coords.down()).bLeft      = true;
    }
}

/// Recursive shortest path algorithm
void findShortest(Matrix<Cell>& aMatrix, const Matrix<Collision>& aCollisions,
                  const Coords& aCoords, const float aWeight, const EDirection aDirection) {
    // If the weight ot this path is less than any preceding one on this cell
    // TODO(SRombauts): in case of equal weight, go into the prefered direction
    if (aMatrix.get(aCoords).weight > aWeight) {
        // Update the cell
        aMatrix.set(aCoords).weight = aWeight;
        aMatrix.set(aCoords).direction = aDirection;

        // Recurse into adjacent cells
        if ((aCoords.x > 0) && (!aCollisions.get(aCoords).bLeft)) {
            findShortest(aMatrix, aCollisions, aCoords.left(), aWeight + 1.0f, eRight);
        }
        if ((aCoords.x < aMatrix.width() - 1) && (!aCollisions.get(aCoords).bRight)) {
            findShortest(aMatrix, aCollisions, aCoords.right(), aWeight + 1.0f, eLeft);
        }
        if ((aCoords.y > 0) && (!aCollisions.get(aCoords).bUp)) {
            findShortest(aMatrix, aCollisions, aCoords.up(), aWeight + 1.0f, eDown);
        }
        if ((aCoords.y < aMatrix.height() - 1) && (!aCollisions.get(aCoords).bDown)) {
            findShortest(aMatrix, aCollisions, aCoords.down(), aWeight + 1.0f, eUp);
        }
    }
}
/// Shortest path algorithm
void findShortest(Matrix<Cell>& aMatrix, const Matrix<Collision>& aCollisions, const Player& aPlayer) {
    const float weight = 0.0f;
    size_t x;
    size_t y;

    switch (aPlayer.id) {
    case 0: {}
        x = 8;
        for (y = 0; y < aMatrix.height(); ++y) {
            findShortest(aMatrix, aCollisions, Coords{ x, y }, weight, eRight);
        }
        break;
    case 1:
        x = 0;
        for (y = 0; y < aMatrix.height(); ++y) {
            findShortest(aMatrix, aCollisions, Coords{ x, y }, weight, eLeft);
        }
        break;
    case 2:
        y = 8;
        for (x = 0; x < aMatrix.width(); ++x) {
            findShortest(aMatrix, aCollisions, Coords{ x, y }, weight, eDown);
        }
        break;
    default:
        throw std::logic_error("shortest: default");
        break;
    }
}


/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 *
 * @return 0
 */
int main() {
    size_t w; // width of the board
    size_t h; // height of the board
    size_t playerCount; // number of players (2 or 3)
    size_t myId; // id of my player (0 = 1st player, 1 = 2nd player, ...)
    std::cin >> w >> h >> playerCount >> myId; std::cin.ignore();

    // all players statuses
    Player::Vector players(playerCount);
    Player& MySelf = players[myId];

    // game loop
    for (size_t turn = 0; turn < 100; ++turn) {
        for (size_t id = 0; id < playerCount; ++id) {
            int x; // x-coordinate of the player
            int y; // y-coordinate of the player
            size_t wallsLeft; // number of walls available for the player
            std::cin >> x >> y >> wallsLeft; std::cin.ignore();

            players[id].id          = id; // redundant, but useful for debug
            players[id].wallsLeft   = wallsLeft;

            // if player still playing
            if ((x >= 0) && (y >= 0)) {
                players[id].coords.x = static_cast<size_t>(x);
                players[id].coords.y = static_cast<size_t>(y);
                players[id].bIsAlive = true;

                // debug:
                if (id == myId) {
                    std::cerr << "myself(" << players[id].id << "): [" << players[id].coords.x
                              << ", " << players[id].coords.y << "] (left=" << players[id].wallsLeft << ")\n";
                } else {
                     std::cerr << "player(" << players[id].id << "): [" << players[id].coords.x
                              << ", " << players[id].coords.y << "] (left=" << players[id].wallsLeft << ")\n";
                }
            } else {
                players[id].bIsAlive = false;
                std::cerr << "_dead_(" << id << "): [" << x << ", " << y << "]\n";
            }
        }

        size_t          wallCount; // number of walls on the board
        std::cin >> wallCount; std::cin.ignore();

        Wall::Vector        walls(wallCount);
        Matrix<Collision>   collisions(w, h);
        for (size_t idx = 0; idx < wallCount; ++idx) {
            std::cin >> walls[idx].coords.x >> walls[idx].coords.y >> walls[idx].orientation; std::cin.ignore();
            std::cerr << "idx(" << idx << "): [" << walls[idx].coords.x << ", " << walls[idx].coords.y << "] '"
                      << walls[idx].orientation <<"'\n";
            setWall(collisions, walls[idx]);
        }

        std::cerr << "turn " << turn << std::endl;

        // debug dump:
        std::cerr << "matrix of walls:" << std::endl;
        collisions.dump();

        // TODO(SRombauts) use one grid for pathfinding for each player
        Matrix<Cell> paths(w, h, Cell{ std::numeric_limits<float>::max(), eRight });
        for (size_t id = 0; id < playerCount; ++id) {
            // debug:
            if (id != myId) {
                // set the player on the path : 0.0 weight to forbid this cell
                // TODO(SRombauts): only put the player onto the grid if not far from us?
                paths.set(players[id].coords.x, players[id].coords.y).weight = 0.0;
            }
        }

        // Test of a simple pathfinding algorithm:
        findShortest(paths, collisions, MySelf);
        std::cerr << "matrix of paths:" << std::endl;
        paths.dump();

        // TODO(SRombauts): put walls in time and with intelligence
        if (turn == 6) {
            // for now, only put one wall at the last minute (only way to keep it safe)
            if ((myId != 0) && (players[0].bIsAlive)) {
                if (players[0].coords.y < h - 2) {
                    Command::put(players[0].coords.right(), 'V', "stop here");
                } else {
                    Command::put(players[0].coords.upright(), 'V', "stop there");
                }
            } else if ((myId != 1) && (players[1].bIsAlive)) {
                if (players[1].coords.y < h - 2) {
                    Command::put(players[1].coords, 'V', "stop here");
                } else {
                    Command::put(players[1].coords.up(), 'V', "stop there");
                }
            } else {
                if (players[2].coords.x < w - 2) {
                    Command::put(players[2].coords.down(), 'H', "stop here");
                } else {
                    Command::put(players[2].coords.downleft(), 'H', "stop there");
                }
            }
        } else {
            // use the matrix of shortest paths to issue a command
            EDirection bestDirection = paths.get(MySelf.coords.x, MySelf.coords.y).direction;
            std::cerr << "[" << MySelf.coords.x << ", " << MySelf.coords.y << "]=>" << bestDirection << std::endl;
            Command::move(bestDirection);
        }

        // return 1; // for debug purpose
    }

    return 0;
}
