
#include "raylib.h"
#include "src/class.h"
#include <string.h>

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#define MAX_TUBES 10000
#define TUBES_WIDTH 80

static const int screenWidth = 800;  // 屏幕宽度
static const int screenHeight = 450; // 屏幕高度
static bool gameOver = false;        // 游戏结束标志
static bool pause = false;           // 游戏暂停标志
static int score = 0;                // 得分
static int hiScore = 0;              // 最高得分

static Floppy floppy = {0}; // 吴翔
Texture2D wx = {0};
static Tubes tubes[MAX_TUBES * 2] = {0};  // 障碍物
static Vector2 tubesPos[MAX_TUBES] = {0}; // 障碍物位置
static int tubesSpeedX = 0;               // 障碍物速度
// static bool superfx = false;           // 屏闪

static void InitGame(void);        // Initialize game
static void UpdateGame(void);      // Update game (one frame)
static void DrawGame(Font);        // Draw game (one frame)
static void UnloadGame(void);      // Unload game
static void UpdateDrawFrame(Font); // Update and Draw (one frame)
static Font InitFont(void);

int main(void)
{

    InitWindow(screenWidth, screenHeight, "fly 吴翔");
    InitGame();
    Font fontAsian = LoadFont("resources/noto_cjk.fnt");
    Font font = InitFont();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        UpdateDrawFrame(fontAsian);
    }
#endif

    UnloadGame(); // Unload loaded data (textures, sounds, models...)

    CloseWindow(); // Close window and OpenGL context

    return 0;
}

void InitGame(void)
{
    wx = LoadTexture("resources/wx.png");
    wx.height = wx.height / 4;
    wx.width = wx.width / 4;
    floppy.radius = wx.width / 2; // todo 暂时是个球 如何安排碰撞体积
    floppy.position = (Vector2){80, screenHeight / 2};
    tubesSpeedX = 2;

    for (int i = 0; i < MAX_TUBES; i++)
    {
        tubesPos[i].x = 400 + 280 * i;
        tubesPos[i].y = -GetRandomValue(0, 120);
    }

    for (int i = 0; i < MAX_TUBES * 2; i += 2)
    {
        tubes[i].rec.x = tubesPos[i / 2].x;
        tubes[i].rec.y = tubesPos[i / 2].y;
        tubes[i].rec.width = TUBES_WIDTH;
        tubes[i].rec.height = 255;

        tubes[i + 1].rec.x = tubesPos[i / 2].x;
        tubes[i + 1].rec.y = 600 + tubesPos[i / 2].y - 255;
        tubes[i + 1].rec.width = TUBES_WIDTH;
        tubes[i + 1].rec.height = 255;

        tubes[i / 2].active = true;
    }

    score = 0;

    gameOver = false;
    pause = false;
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P'))
            pause = !pause;

        if (!pause)
        {
            for (int i = 0; i < MAX_TUBES; i++)
                tubesPos[i].x -= tubesSpeedX;

            for (int i = 0; i < MAX_TUBES * 2; i += 2)
            {
                tubes[i].rec.x = tubesPos[i / 2].x;
                tubes[i + 1].rec.x = tubesPos[i / 2].x;
            }

            if (IsKeyDown(KEY_SPACE) && !gameOver)
                floppy.position.y -= 3;
            else
                floppy.position.y += 2;

            // Check Collisions
            for (int i = 0; i < MAX_TUBES * 2; i++)
            {
                // 碰撞条件需要修改
                if (CheckCollisionCircleRec(floppy.position, floppy.radius, tubes[i].rec))
                {
                    gameOver = true;
                    pause = false;
                }
                else if ((tubesPos[i / 2].x < floppy.position.x) && tubes[i / 2].active && !gameOver)
                {
                    score += 100;
                    tubes[i / 2].active = false;
                    if (score > hiScore)
                        hiScore = score;
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}

// Draw game (one frame)
void DrawGame(Font font)
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (!gameOver)
    {
        // DrawCircle(floppy.position.x, floppy.position.y, floppy.radius, DARKGRAY);

        DrawTexture(wx, floppy.position.x - wx.width / 2, floppy.position.y - wx.height / 2, WHITE);

        // Draw tubes
        for (int i = 0; i < MAX_TUBES; i++)
        {
            DrawRectangle(tubes[i * 2].rec.x, tubes[i * 2].rec.y, tubes[i * 2].rec.width, tubes[i * 2].rec.height, GRAY);
            DrawRectangle(tubes[i * 2 + 1].rec.x, tubes[i * 2 + 1].rec.y, tubes[i * 2 + 1].rec.width, tubes[i * 2 + 1].rec.height, GRAY);
        }
        // 测试高度
        // DrawText(TextFormat("info: %04i", floppy.radius), 40, 40, 40, GRAY);
        DrawText(TextFormat("%04i", score), 20, 20, 40, GRAY);
        DrawText(TextFormat("wx-rank: %04i", hiScore), 20, 70, 20, LIGHTGRAY);

        if (pause)
            DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, GRAY);
    }
    else{
        // todo

        // Vector2 sz = MeasureTextEx(font, "让wx再飞一次", (float)font.baseSize, 1.0f);
        // if (sz.x > 300)
        // {
        //     sz.y *= sz.x / 300;
        //     sz.x = 300;
        // }
        // else if (sz.x < 160)
        //     sz.x = 160;
        Vector2 textPosition = (Vector2){GetScreenWidth() / 2 - MeasureText("让wx再飞一次", 20) / 2, GetScreenHeight() / 2 - 50, 20};

        DrawTextEx(font, TextFormat("让wx再飞一次", "Chinese"), textPosition, 30, 0, GRAY);
    }

    EndDrawing();
}

// Unload game variables
void UnloadGame(void)
{
    UnloadTexture(wx);
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(Font font)
{
    UpdateGame();
    DrawGame(font);
}

Font InitFont(void)
{
    unsigned int fileSize = 0;
    unsigned char *fileData = LoadFileData("resources/webfont.ttf", &fileSize);

    // Default font generation from TTF font
    Font fontDefault = {0};
    fontDefault.baseSize = 16;
    fontDefault.glyphCount = 95;
    fontDefault.glyphs = LoadFontData(fileData, fileSize, 16, 0, 95, FONT_DEFAULT);
    return fontDefault;
}