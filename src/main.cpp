#include <raylib.h>
#include "Math.h"

#include <cassert>
#include <array>
#include <vector>
#include <algorithm>

const float SCREEN_SIZE = 800;

const int TILE_COUNT = 20;
const float TILE_SIZE = SCREEN_SIZE / TILE_COUNT;

enum TileType : int
{
    GRASS,      // Marks unoccupied space, can be overwritten 
    DIRT,       // Marks the path, cannot be overwritten
    WAYPOINT,   // Marks where the path turns, cannot be overwritten
    TURRET,     // Marks where a turret is
    COUNT
};

struct Cell
{
    int row;
    int col;
};

constexpr std::array<Cell, 4> DIRECTIONS{ Cell{ -1, 0 }, Cell{ 1, 0 }, Cell{ 0, -1 }, Cell{ 0, 1 } };

inline bool InBounds(Cell cell, int rows = TILE_COUNT, int cols = TILE_COUNT)
{
    return cell.col >= 0 && cell.col < cols && cell.row >= 0 && cell.row < rows;
}

void DrawTile(int row, int col, Color color)
{
    DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, color);
}

void DrawTile(int row, int col, int type)
{
    Color color = type > 0 ? BEIGE : GREEN;
    DrawTile(row, col, color);
}

Vector2 TileCenter(int row, int col)
{
    float x = col * TILE_SIZE + TILE_SIZE * 0.5f;
    float y = row * TILE_SIZE + TILE_SIZE * 0.5f;
    return { x, y };
}

Vector2 TileCorner(int row, int col)
{
    float x = col * TILE_SIZE;
    float y = row * TILE_SIZE;
    return { x, y };
}

// Returns a collection of adjacent cells that match the search value.
std::vector<Cell> FloodFill(Cell start, int tiles[TILE_COUNT][TILE_COUNT], TileType searchValue)
{
    // "open" = "places we want to search", "closed" = "places we've already searched".
    std::vector<Cell> result;
    std::vector<Cell> open;
    bool closed[TILE_COUNT][TILE_COUNT];
    for (int row = 0; row < TILE_COUNT; row++)
    {
        for (int col = 0; col < TILE_COUNT; col++)
        {
            // We don't want to search zero-tiles, so add them to closed!
            closed[row][col] = tiles[row][col] == 0;
        }
    }

    // Add the starting cell to the exploration queue & search till there's nothing left!
    open.push_back(start);
    while (!open.empty())
    {
        // Remove from queue and prevent revisiting
        Cell cell = open.back();
        open.pop_back();
        closed[cell.row][cell.col] = true;

        // Add to result if explored cell has the desired value
        if (tiles[cell.row][cell.col] == searchValue)
            result.push_back(cell);

        // Search neighbours
        for (Cell dir : DIRECTIONS)
        {
            Cell adj = { cell.row + dir.row, cell.col + dir.col };
            if (InBounds(adj) && !closed[adj.row][adj.col] && tiles[adj.row][adj.col] > 0)
                open.push_back(adj);
        }
    }

    return result;
}

struct Enemy // Creates reusable structure, enemies are built off of
{
    Vector2 position{};
    int health = 5;
    size_t curr = 0;
    size_t next = curr + 1;
    bool atEnd = false;
};

struct Turret // Creates reusable structure, turrets are built off of
{
    Vector2 position{};
    float range = 200;
    float calmDown = 0;
    float fireRate = 0.69f;
};

struct Bullet // Creates reusable structure, bullets are built off of
{
    Vector2 position{};
    Vector2 direction{};
    float time = 0.0f;
    bool enabled = true;
};

int main()
{
    int tiles[TILE_COUNT][TILE_COUNT]
    {
        //col:0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19    row:
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0 }, // 0
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 1
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 2
            { 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0 }, // 3
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 4
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 5
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 6
            { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0 }, // 7
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 8
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 9
            { 0, 0, 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0 }, // 10
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 11
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 12
            { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 13
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 14
            { 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 3 }, // 15
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 16
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 17
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 18
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0 }  // 19
    };

    std::vector<Cell> waypoints = FloodFill({ 0, 12 }, tiles, WAYPOINT);// Created array
    std::vector<Enemy> bullies;                                         // Created array
    std::vector<Turret> nerds;                                          // Created array
    for (int row = 0; row < TILE_COUNT; ++row) // Looks at every row
    {
        for (int col = 0; col < TILE_COUNT; ++col) // Looks at every column within that row
        if (tiles[row][col] == 3) // Checks to see if tile is equal to 3, if it is, create turret
        {
            Turret turret;
            turret.position = TileCenter(row, col);
            nerds.push_back(turret);
        }
    }


    const float enemySpeed = 250.0f;
    const float enemyRadius = 20.0f;

    const float bulletTime = 1.0f;
    const float bulletSpeed = 500.0f;
    const float bulletRadius = 15.0f;

    std::vector<Bullet> bullets;
    float shootCurrent = 0.0f;
    float shootTotal = 0.42f;

    float enemyTime = 0.00f;
    int totalEnemies = 10; // Maximum amount of spawnable enemies
    int enemyCount = 0; // Amount of enemies that have been spawned

    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Tower Defense");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();  // For Assignment 1
        Cell mouseCell;                      // For Assignment 1
        mouseCell.col = mouse.x / TILE_SIZE; // For Assignment 1
        mouseCell.row = mouse.y / TILE_SIZE; // For Assignment 1
        enemyTime += dt; // Enemy spawn timer, going up by DeltaTime

        if (enemyTime >= 1 && enemyCount < totalEnemies) // If time since enemy spawned is greater than or equal to 1, spawn a new enemy
        {
            Enemy enemy;
            enemy.position = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
            enemy.curr = 0;
            enemy.next = enemy.curr + 1;
            enemyTime = 0;
            enemy.atEnd = false;
            bullies.push_back(enemy); // Pushes to the bullies vector
            enemyCount++;
        }

        // Path following
        for (Enemy& enemy:bullies)
        {
            if (!enemy.atEnd) // If the enemy is not at the end, continue through path following
            {
                Vector2 from = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
                Vector2 to = TileCenter(waypoints[enemy.next].row, waypoints[enemy.next].col);
                Vector2 direction = Normalize(to - from);
                enemy.position = enemy.position + direction * enemySpeed * dt;
                if (CheckCollisionPointCircle(enemy.position, to, enemyRadius))
                {
                    enemy.curr++;
                    enemy.next++;
                    enemy.atEnd = enemy.next == waypoints.size();
                    enemy.position = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
                }
            }
        }

        for (Turret& turret:nerds) 
        {
            turret.calmDown += dt;
            Enemy* meanies = nullptr;
            
            for (Enemy& enemy:bullies) // For every enemy, check to see if the distance is within the turrets range, then declaring them a meanie if they are
            {
                float enemyDistance = Distance(turret.position, enemy.position);
                if (enemyDistance < turret.range)
                {
                    meanies = &enemy;
                }
            }
            if (meanies && turret.calmDown >= turret.fireRate) // If turret isn't on cooldown, fire at the meanies and reset cooldown
            {
                turret.calmDown = 0.0f;

                Bullet bullet;
                bullet.position = turret.position;
                bullet.direction = Normalize(meanies->position - bullet.position); // Accesses position of a specific target, the meanies
                bullets.push_back(bullet); // Pushes to the bullet vector
            }
        }

        // Bullet update
        for (Bullet& bullet : bullets)
        {
            bullet.position = bullet.position + bullet.direction * bulletSpeed * dt;
            bullet.time += dt;

            bool expired = bullet.time >= bulletTime;
            for (int i = 0; i < bullies.size();)
            {
                Enemy& enemy = bullies[i]; // Referencing a specific enemy within enemies vector
                bool collision = CheckCollisionCircles(enemy.position, enemyRadius, bullet.position, bulletRadius);
                if (collision)
                {
                    enemy.health--; // Takes a health away from bully for every time they get hit
                    if (enemy.health <= 0) // Erases the bully that has no more health
                    {
                        bullies.erase(bullies.begin() + i);
                        bullet.enabled = false;
                        break;
                    }
                }
                else
                {
                    i++;
                }
            }
            bullet.enabled = !expired && bullet.enabled; // Makes it so it is enabled as long as it is not expired and enabled
        }
        
        // Bullet removal
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [&bullets](Bullet bullet) {
                return !bullet.enabled;
            }), bullets.end());

        BeginDrawing();
        ClearBackground(BLACK);
        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                DrawTile(row, col, tiles[row][col]);
            }
        }

        // Render bullets
        for (const Bullet& bullet : bullets)
            DrawCircleV(bullet.position, bulletRadius, BLUE);
        // Render turrets
        for (const Turret& turret : nerds)
            DrawCircleV(turret.position, bulletRadius, DARKGRAY);
        // Render enemies
        for (const Enemy& enemy : bullies)
            DrawCircleV(enemy.position, bulletRadius, RED);
        // Printing text to make sure bullets are firing and deleting, and that only ten enemies are spawning
        DrawText(TextFormat("Total bullets: %i", bullets.size()), 10, 10, 20, BLUE);
        DrawText(TextFormat("Total enemies: %i", enemyCount), 10, 30, 20, RED);
        DrawTile(mouseCell.row, mouseCell.col, PURPLE);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
