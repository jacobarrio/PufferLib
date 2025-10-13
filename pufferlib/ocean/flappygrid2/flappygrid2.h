#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "raylib.h"

const Color PUFF_RED = (Color){187, 0, 0, 255};
const Color PUFF_CYAN = (Color){0, 187, 187, 255};
const Color PUFF_WHITE = (Color){241, 241, 241, 241};
const Color PUFF_BACKGROUND = (Color){6, 24, 24, 255};
const Color PUFF_GREEN = (Color){0, 187, 0, 255};

// Only use floats!
typedef struct {
    float perf;
    float score;
    float episode_return;
    float episode_length;
    float n; // Required as the last field 
} Log;

typedef struct {
    Log log;                     // Required field
    unsigned char* observations; // Required field
    int* actions;                // Required field
    float* rewards;              // Required field
    unsigned char* terminals;    // Required field
    
    int grid_size;
    int bird_pos;
    int obstacle_x;
    int obstacle_gap_y;
    int score;
    int steps;
} FlappyGrid2;

void c_reset(FlappyGrid2* env) {
    env->bird_pos = env->grid_size / 2;
    env->obstacle_x = env->grid_size - 1;
    env->obstacle_gap_y = 2 + (rand() % (env->grid_size - 4));
    env->score = 0;
    env->steps = 0;
    
    // Set initial observation
    env->observations[0] = env->bird_pos;
    env->observations[1] = env->obstacle_x;
    env->observations[2] = env->obstacle_gap_y;
}

void c_step(FlappyGrid2* env) {
    env->rewards[0] = 0.0f;
    env->terminals[0] = 0;
    env->steps++;
    
    // Action: 0=fall, 1=flap
    if (env->actions[0] == 1) {
        if (env->bird_pos > 0) {
            env->bird_pos -= 1;
        }
    } else {
        if (env->bird_pos < env->grid_size - 1) {
            env->bird_pos += 1;
        }
    }
    
    // Move obstacle
    env->obstacle_x -= 1;
    
    // Small reward for surviving
    env->rewards[0] = 0.01f;
    env->log.episode_return += 0.01f;
    env->log.episode_length += 1;
    
    // Check collision or pass
    if (env->obstacle_x == 0) {
        int gap_diff = abs(env->bird_pos - env->obstacle_gap_y);
        if (gap_diff <= 1) {
            // Passed obstacle
            env->rewards[0] = 1.0f;
            env->score += 1;
            env->log.episode_return += 1.0f;
        } else {
            // Collision
            env->rewards[0] = -1.0f;
            env->terminals[0] = 1;
            env->log.score = env->score;
            env->log.perf = (float)env->score / env->steps;
            env->log.episode_return += -1.0f;
            env->log.n += 1;
            c_reset(env);
            return;
        }
        
        // Reset obstacle
        env->obstacle_x = env->grid_size - 1;
        env->obstacle_gap_y = 2 + (rand() % (env->grid_size - 4));
    }
    
    // Update observation
    env->observations[0] = env->bird_pos;
    env->observations[1] = env->obstacle_x;
    env->observations[2] = env->obstacle_gap_y;
}

void c_render(FlappyGrid2* env) {
    if (!IsWindowReady()) {
        InitWindow(800, 600, "PufferLib FlappyGrid2");
        SetTargetFPS(10);
    }
    if (IsKeyDown(KEY_ESCAPE)) {
        exit(0);
    }
    
    BeginDrawing();
    ClearBackground(PUFF_BACKGROUND);
    
    int cell_size = 60;  // Try larger cells
    int offset_x = 100;
    int offset_y = 50;
    
    // Draw grid
    for (int i = 0; i < env->grid_size; i++) {
        for (int j = 0; j < env->grid_size; j++) {
            DrawRectangleLines(offset_x + j * cell_size, offset_y + i * cell_size, 
                             cell_size, cell_size, PUFF_WHITE);
        }
    }
    
    // Draw bird
    DrawRectangle(offset_x + 2 * cell_size, offset_y + env->bird_pos * cell_size, 
                  cell_size, cell_size, PUFF_CYAN);
    
    // Draw obstacle
    for (int i = 0; i < env->grid_size; i++) {
        if (abs(i - env->obstacle_gap_y) > 1) {
            DrawRectangle(offset_x + env->obstacle_x * cell_size, 
                         offset_y + i * cell_size, 
                         cell_size, cell_size, PUFF_RED);
        }
    }
    
    // Draw score
    DrawText(TextFormat("Score: %d", env->score), 20, 20, 20, PUFF_WHITE);
    
    EndDrawing();
}

void c_close(FlappyGrid2* env) {
    if (IsWindowReady()) {
        CloseWindow();
    }
}