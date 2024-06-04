// Standard library includes
#include "raylib.h"
#include "raymath.h"

// Standard library includes
#include <stdio.h>  // For snprintf
#include <stdlib.h> // For atoi

// Constants
#define MAX_LASER_NODES 16
#define MAX_OBJECTS 16
#define MODE_BUTTON_SIZE 60
#define MODE_BUTTON_PADDING 20

// Colors
#define ACCENT0  CLITERAL(Color){ 9, 132, 227, 255 }
#define ACCENT1  CLITERAL(Color){ 0, 184, 148, 255 }
#define ACCENT2  CLITERAL(Color){ 253, 203, 110, 255 }
#define ACCENT3  CLITERAL(Color){ 214, 48, 49, 255 }
#define BOX_COLOR  CLITERAL(Color){ 99, 110, 114, 255 }

// Enums
typedef enum Direction { NO_DIRECTION, UP, RIGHT, DOWN, LEFT } Direction;
typedef enum NodeType { NONE, EMITTER, REFLECT_UP, REFLECT_DOWN, OPAQUE } NodeType;
typedef enum GameState { MENU, LEVEL } GameState;

// Structures
typedef struct LaserNode {
    NodeType type;
    Direction direction;
    Vector2 position;
    float value;
} LaserNode;

// Global Variables
int screenWidth = 800;
int screenHeight = 800;
int boardSize = 10;
int boardType = 0;
int seed = 123123;
GameState gameState = MENU;
LaserNode laserNode[MAX_LASER_NODES][MAX_LASER_NODES] = { 0 };
float nodeSize = 32.0f;
char seedText[9] = "1234"; // to store user input seed
int letterCount = 4;
Vector2 modeButtonPositions[4]; // Define modeButtonPositions array
Color modeButtonColors[4]; // Define modeButtonColors array

// OBJECTS definition
NodeType OBJECTS[6][3][3] = {
    { { REFLECT_UP, OPAQUE, REFLECT_DOWN },
      { OPAQUE, OPAQUE, OPAQUE },
      { REFLECT_DOWN, OPAQUE, REFLECT_UP } },
    { { REFLECT_UP, OPAQUE, REFLECT_DOWN },
      { NONE, NONE, NONE },
      { REFLECT_DOWN, OPAQUE, REFLECT_UP } },
    { { REFLECT_UP, NONE, REFLECT_DOWN },
      { OPAQUE, NONE, OPAQUE },
      { REFLECT_DOWN, NONE, REFLECT_UP } },
    { { NONE, NONE, NONE },
      { NONE, OPAQUE, NONE },
      { NONE, NONE, NONE } },
    { { REFLECT_UP, NONE, REFLECT_DOWN },
      { NONE, REFLECT_DOWN, NONE },
      { REFLECT_DOWN, NONE, REFLECT_UP } },
    { { REFLECT_UP, NONE, REFLECT_DOWN },
      { NONE, REFLECT_UP, NONE},
      { REFLECT_DOWN, NONE, REFLECT_UP } }
};

void UpdateNodePosition() {
    for (int col = 0; col < boardSize; col++) {
        for (int row = 0; row < boardSize; row++) {
            laserNode[col][row].position = (Vector2){ 0.5 * screenHeight / boardSize + (screenHeight / boardSize) * col,
                0.5 * screenHeight / boardSize + (screenHeight / boardSize) * row };
        }
    }
}

void DrawBox() {
    // Draw the box
    DrawRectangleLinesEx((Rectangle) { laserNode[1][1].position.x - nodeSize, laserNode[1][1].position.y - nodeSize, 2 * nodeSize * (boardSize)-2.5 * nodeSize, 2 * nodeSize * (boardSize)-2.5 * nodeSize }, 4, BOX_COLOR);

    if (gameState == MENU) {
        float textWidth = MeasureText("Box", 256);
        DrawText("Box", screenWidth / 2 - textWidth / 2, screenHeight / 2 - 128, 256, BOX_COLOR);
    }
}

void DrawModeButtons() {
    for (int i = 0; i < 4; i++) {
        DrawCircleV(modeButtonPositions[i], nodeSize, ColorTint(darkAccentColor, WHITE));
        DrawCircleV(modeButtonPositions[i], nodeSize * 0.8, modeButtonColors[i]);
        DrawText(TextFormat("TYPE %d", i + 1), modeButtonPositions[i].x - 40, modeButtonPositions[i].y + 20 + nodeSize, 20, BOX_COLOR);
    }
}

void InitModeButtons() {
    // Set positions and colors for mode buttons
    int xStart = (screenWidth - (4 * nodeSize + 3 * 80)) / 2;
    int y = screenHeight - 200; // Adjusted to be at the bottom of the box
    for (int i = 0; i < 4; i++) {
        modeButtonPositions[i] = (Vector2){ xStart + (nodeSize + 80) * i + nodeSize / 2, y };
        switch (i) {
        case 0: modeButtonColors[i] = ACCENT0; break;
        case 1: modeButtonColors[i] = ACCENT1; break;
        case 2: modeButtonColors[i] = ACCENT2; break;
        case 3: modeButtonColors[i] = ACCENT3; break;
        }
    }
}

Vector2 backButtonPosition = { 100, 680 };

void DrawLaserNode() {
    for (int col = 0; col < boardSize; col++) {
        for (int row = 0; row < boardSize; row++) {
            if (laserNode[col][row].value > 0) laserNode[col][row].value -= 0.25f;
            if (laserNode[col][row].type == EMITTER) {
                Color darkAccentColor = ACCENT0; // Default dark accent color

                // Change accent colors based on boardType
                switch (boardType) {
                case 0:
                    darkAccentColor = ACCENT0;
                    break;
                case 1:
                    darkAccentColor = ACCENT1;
                    break;
                case 2:
                    darkAccentColor = ACCENT2;
                    break;
                case 3:
                    darkAccentColor = ACCENT3;
                    break;
                    // Add more cases if needed
                }

                DrawCircleV(laserNode[col][row].position, nodeSize, ColorTint(darkAccentColor, WHITE));
                DrawCircleV(laserNode[col][row].position, nodeSize * 0.8, darkAccentColor);
                if (laserNode[col][row].value > 1) {
                    float textWidth = MeasureText(TextFormat("%0.0f", laserNode[col][row].value), 32);
                    float centerX = laserNode[col][row].position.x - textWidth / 2;
                    DrawText(TextFormat("%0.0f", laserNode[col][row].value), centerX, laserNode[col][row].position.y - 14, 32, ColorAlpha(WHITE, laserNode[col][row].value / 100.0f));
                }
            }
        }
    }
    DrawText("BACK", backButtonPosition.x, backButtonPosition.y, 20, BOX_COLOR);
}

void PlaceObjects() {
    int numObjects = GetRandomValue(2, 4);
    int objectType = 0;
    int row = 0;
    int col = 0;
    bool collision = false;
    for (int i = 0; i < numObjects; i++) {
        do {
            collision = false;
            row = GetRandomValue(1, boardSize - 1);
            col = GetRandomValue(1, boardSize - 1);
            switch (boardType) {
            case 0: // For boardType 0, only use objects from OBJECTS[0]
                objectType = 0;
                break;
            case 1: // For boardType 1, randomly select from OBJECTS[1] or OBJECTS[2]
                objectType = GetRandomValue(1, 2);
                break;
            case 2: // For boardType 2, only use objects from OBJECTS[3]
                objectType = 3;
                break;
            case 3: // For boardType 3, randomly select from OBJECTS[4] or OBJECTS[5]
                objectType = GetRandomValue(4, 5);
                break;
                // Add more cases if needed
            }
            for (int c = 0; c < 3; c++) {
                for (int r = 0; r < 3; r++) {
                    if (laserNode[col + c][row + r].type != NONE) {
                        collision = true;
                    }
                }
            }
        } while (collision);
        for (int c = 0; c < 3; c++) {
            for (int r = 0; r < 3; r++) {
                laserNode[col + c][row + r].type = OBJECTS[objectType][c][r];
            }
        }
    }
}

void InitBoard() {
    SetRandomSeed(seed + boardType);
    InitModeButtons();
    nodeSize = 0.45 * screenHeight / boardSize;
    for (int col = 0; col < MAX_LASER_NODES; col++) for (int row = 0; row < MAX_LASER_NODES; row++) laserNode[col][row] = (LaserNode){ 0 };
    for (int i = 1; i < boardSize - 1; i++) {
        laserNode[i][0] = (LaserNode){ EMITTER,  DOWN };
        laserNode[i][boardSize - 1] = (LaserNode){ EMITTER, UP };
        laserNode[0][i] = (LaserNode){ EMITTER, RIGHT };
        laserNode[boardSize - 1][i] = (LaserNode){ EMITTER,  LEFT };
    }
    PlaceObjects();
    UpdateNodePosition();
}

void propagateLight(int col, int row, float value, Direction direction) {
    laserNode[col][row].value = fminf(value, 100.0f);
    switch (laserNode[col][row].type) {
    case OPAQUE: value = 0.0f; break;
    case REFLECT_UP:
        value *= 0.9f;
        switch (direction) {
        case UP: direction = RIGHT; break;
        case DOWN: direction = LEFT; break;
        case LEFT: direction = DOWN; break;
        case RIGHT: direction = UP; break;
        } break;
    case REFLECT_DOWN:
        value *= 0.9f;
        switch (direction) {
        case UP: direction = LEFT; break;
        case DOWN: direction = RIGHT; break;
        case LEFT: direction = UP; break;
        case RIGHT: direction = DOWN; break;
        } break;
    }
    switch (direction) {
    case UP: row -= 1; break;
    case DOWN: row += 1; break;
    case RIGHT: col += 1; break;
    }
    if ((col >= 0 && col < boardSize) && (row >= 0 && row < boardSize)) propagateLight(col, row, value, direction);
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE); // Allow the window to be resizable
    InitWindow(screenWidth, screenHeight, "Box");
    SetTargetFPS(60);
    seed = GetRandomValue(1000, 9999);
    letterCount = 4;
    snprintf(seedText, 9, "%04d", seed); // Format seed into a 4-digit string
    InitBoard();
    SetWindowSize(screenWidth, screenHeight);


    while (!WindowShouldClose()) {

        if (IsKeyDown(KEY_D)) {
            for (int col = 0; col < MAX_LASER_NODES; col++) for (int row = 0; row < MAX_LASER_NODES; row++) {
                if (laserNode[col][row].type == OPAQUE) DrawText("O", laserNode[col][row].position.x + 8, laserNode[col][row].position.y + 8, 32, BOX_COLOR);
                else if (laserNode[col][row].type == REFLECT_UP) DrawText("/", laserNode[col][row].position.x + 8, laserNode[col][row].position.y + 8, 32, BOX_COLOR);
                else if (laserNode[col][row].type == REFLECT_DOWN) DrawText("\\", laserNode[col][row].position.x + 8, laserNode[col][row].position.y + 8, 32, BOX_COLOR);
            }
        }

        Vector2 mousePosition = GetMousePosition();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (gameState) {
        case MENU:
            // Draw mode buttons
            DrawModeButtons();
            // Check for button clicks
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                for (int i = 0; i < 4; i++) {
                    if (CheckCollisionPointCircle(mousePosition, modeButtonPositions[i], MODE_BUTTON_SIZE / 2)) {
                        // Set board type based on button clicked (1 to 4)
                        boardType = i;
                        // Change game state to LEVEL
                        gameState = LEVEL;
                        // Initialize the board with the selected mode
                        InitBoard();
                    }
                }
            }
            break;

        case LEVEL:
            DrawLaserNode();
            DrawText(TextFormat("Seed %i", seed), screenWidth - nodeSize * 7, nodeSize * 3, 20, BOX_COLOR);

            int key = GetCharPressed();
            while (key > 0) {

                // Handle numeric input
                if ((key >= 48) && (key <= 57) && (letterCount < 6)) {
                    // Shift the digits if the first digit is zero
                    if (letterCount == 5 && seedText[0] == '0') {
                        for (int i = 0; i < 5; i++) {
                            seedText[i] = seedText[i + 1];
                        }
                        letterCount--;
                    }
                    seedText[letterCount++] = (char)key;
                    seedText[letterCount] = '\0';
                    seed = atoi(seedText);
                    InitBoard();
                }

                // Handle backspace
                if (key == KEY_BACKSPACE && letterCount > 0) {
                    letterCount--;
                    seedText[letterCount] = '\0';
                    if (letterCount == 0) {
                        seed = 0; // Set seed to 0 if there is only one digit entered
                    }
                    else {
                        seed = atoi(seedText);
                    }
                    InitBoard();
                }

                key = GetCharPressed();
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                for (int col = 0; col < MAX_LASER_NODES; col++) for (int row = 0; row < MAX_LASER_NODES; row++) if (laserNode[col][row].type == EMITTER && CheckCollisionPointCircle(mousePosition, laserNode[col][row].position, nodeSize) && (laserNode[col][row].value < 100.0f)) propagateLight(col, row, ++laserNode[col][row].value, laserNode[col][row].direction);
                if (CheckCollisionPointRec(mousePosition, (Rectangle) { backButtonPosition.x, backButtonPosition.y, 100, 25 })) gameState = MENU;
            }
            break;
        }

        DrawBox();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
