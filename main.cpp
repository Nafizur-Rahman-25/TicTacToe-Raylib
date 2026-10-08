#include "raylib.h"
#include <string>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cstdlib>

const int screenWidth = 900;
const int screenHeight = 750;

// ============================================
// কালার প্যালেট
// ============================================
const Color BG_COLOR = { 15, 15, 30, 255 };
const Color PANEL_COLOR = { 25, 30, 50, 255 };
const Color ACCENT_X = { 255, 80, 120, 255 };
const Color ACCENT_O = { 80, 200, 255, 255 };
const Color TEXT_COLOR = { 230, 230, 240, 255 };
const Color SUBTEXT_COLOR = { 130, 140, 160, 255 };
const Color BTN_COLOR = { 40, 50, 80, 255 };
const Color BTN_HOVER = { 70, 90, 150, 255 };
const Color SUCCESS = { 100, 255, 150, 255 };
const Color DANGER = { 255, 90, 90, 255 };
const Color MY_GOLD = { 255, 215, 0, 255 };

// ============================================
// গেম স্টেট
// ============================================
enum GameState {
    STATE_MAIN_MENU,
    STATE_NAME_INPUT,
    STATE_ROUND_SETUP,
    STATE_PLAYING,
    STATE_ROUND_OVER,
    STATE_MATCH_OVER,
    STATE_SETTINGS,
    STATE_CREDITS,
    STATE_EXIT
};

GameState currentState = STATE_MAIN_MENU;

// ============================================
// ফন্ট
// ============================================
Font fontWalshes;
Font fontBlackUnicorn;
Font fontDeath;

// ============================================
// সাউন্ড
// ============================================
Sound sndClick;
Sound sndPlace;
Sound sndWin;
Sound sndDraw;
Sound sndFail;
bool soundLoaded = false;

// ============================================
// গ্লোবাল স্টেট
// ============================================
char player1Name[32] = "";
char player2Name[32] = "";
int activeInputField = 0;
char roundInput[8] = "3";
int totalRounds = 3;
int currentRound = 1;
int score1 = 0, score2 = 0, draws = 0;
bool soundEnabled = true;

int board[3][3] = {0};
int currentPlayer = 1;
bool gameOver = false;
int winner = 0;
const int cellSize = 140;
const int gridStartX = (screenWidth - 3 * cellSize) / 2;
const int gridStartY = 130;

float glowTimer = 0.0f;
float resultTimer = 0.0f;

// ============================================
// পার্টিকেল সিস্টেম (Party Crackers)
// ============================================
const int MAX_PARTICLES = 250;
struct Particle {
    Vector2 pos;
    Vector2 vel;
    Color color;
    float life;
    float size;
    bool active;
};
Particle particles[MAX_PARTICLES];

void InitParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
        particles[i].life = 0.0f;
    }
}

void SpawnFirework(int count) {
    Color colors[] = { ACCENT_X, ACCENT_O, SUCCESS, YELLOW, ORANGE, PINK, SKYBLUE, VIOLET, MY_GOLD };
    for (int k = 0; k < count; k++) {
        int spawnX = screenWidth / 2 + (rand() % 500 - 250);
        int spawnY = 150 + (rand() % 250);
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) {
                float angle = (float)(rand() % 360) * DEG2RAD;
                float speed = 80.0f + (float)(rand() % 280);
                particles[i].pos = { (float)spawnX, (float)spawnY };
                particles[i].vel = { cosf(angle) * speed, sinf(angle) * speed };
                particles[i].color = colors[rand() % 9];
                particles[i].life = 1.5f + (float)(rand() % 100) / 100.0f;
                particles[i].size = 3.0f + (float)(rand() % 4);
                particles[i].active = true;
                break;
            }
        }
    }
}

void UpdateParticles() {
    float dt = GetFrameTime();
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].pos.x += particles[i].vel.x * dt;
            particles[i].pos.y += particles[i].vel.y * dt;
            particles[i].vel.y += 200.0f * dt;
            particles[i].life -= dt;
            if (particles[i].life <= 0.0f) particles[i].active = false;
        }
    }
}

void DrawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            Color c = particles[i].color;
            c.a = (unsigned char)(255 * (particles[i].life / 2.5f));
            DrawCircleV(particles[i].pos, particles[i].size, c);
        }
    }
}

// ============================================
// হেল্পার ফাংশন
// ============================================
void DrawTextFont(Font font, const char* text, int x, int y, int fontSize, Color color) {
    DrawTextEx(font, text, {(float)x, (float)y}, (float)fontSize, 1.0f, color);
}

int MeasureTextFont(Font font, const char* text, int fontSize) {
    return (int)MeasureTextEx(font, text, (float)fontSize, 1.0f).x;
}

void DrawGlowTextFont(Font font, const char* text, int x, int y, int fontSize, Color color, float intensity) {
    for (int i = 5; i > 0; i--) {
        Color glow = color;
        glow.a = (unsigned char)(40 * intensity / i);
        DrawTextFont(font, text, x - i, y, fontSize, glow);
        DrawTextFont(font, text, x + i, y, fontSize, glow);
        DrawTextFont(font, text, x, y - i, fontSize, glow);
        DrawTextFont(font, text, x, y + i, fontSize, glow);
    }
    DrawTextFont(font, text, x, y, fontSize, color);
}

void DrawTextCentered(const char* text, int y, int fontSize, Color color) {
    int tw = MeasureText(text, fontSize);
    DrawText(text, (screenWidth - tw) / 2, y, fontSize, color);
}

void DrawX(int cx, int cy, int size, Color color) {
    int half = size / 2;
    int thick = size / 8;
    DrawLineEx({(float)(cx - half), (float)(cy - half)}, {(float)(cx + half), (float)(cy + half)}, thick, color);
    DrawLineEx({(float)(cx + half), (float)(cy - half)}, {(float)(cx - half), (float)(cy + half)}, thick, color);
}

void DrawO(int cx, int cy, int size, Color color) {
    DrawRing({(float)cx, (float)cy}, size/2 - size/10, size/2, 0, 360, 60, color);
}

void PlaySoundIfEnabled(Sound snd) {
    if (soundEnabled && soundLoaded) PlaySound(snd);
}

void ResetBoard() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = 0;
    currentPlayer = 1;
    gameOver = false;
    winner = 0;
}

void CheckWin() {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != 0 && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            winner = board[i][0]; gameOver = true; return;
        }
    }
    for (int i = 0; i < 3; i++) {
        if (board[0][i] != 0 && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            winner = board[0][i]; gameOver = true; return;
        }
    }
    if (board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        winner = board[0][0]; gameOver = true; return;
    }
    if (board[0][2] != 0 && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        winner = board[0][2]; gameOver = true; return;
    }
    bool isFull = true;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == 0) isFull = false;
    if (isFull) { winner = 3; gameOver = true; }
}

// ============================================
// বাটন (Death ফন্ট)
// ============================================
bool DrawButton(Rectangle rect, const char* text, Color baseColor, int fontSize) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);
    DrawRectangleRounded(rect, 0.3f, 8, hover ? BTN_HOVER : baseColor);
    DrawRectangleRoundedLines(rect, 0.3f, 8, hover ? WHITE : SUBTEXT_COLOR);

    int textWidth = MeasureTextFont(fontDeath, text, fontSize);
    Vector2 textPos = {
        rect.x + (rect.width - textWidth) / 2,
        rect.y + (rect.height - (float)fontSize) / 2
    };
    DrawTextFont(fontDeath, text, (int)textPos.x, (int)textPos.y, fontSize, TEXT_COLOR);

    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySoundIfEnabled(sndClick);
        return true;
    }
    return false;
}

// সংখ্যা বাটন (ডিফল্ট ফন্ট)
bool DrawNumberButton(Rectangle rect, const char* text, Color baseColor, int fontSize) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);
    DrawRectangleRounded(rect, 0.3f, 8, hover ? BTN_HOVER : baseColor);
    DrawRectangleRoundedLines(rect, 0.3f, 8, hover ? WHITE : SUBTEXT_COLOR);

    int textWidth = MeasureText(text, fontSize);
    DrawText(text,
             (int)(rect.x + (rect.width - textWidth) / 2),
             (int)(rect.y + (rect.height - fontSize) / 2) + 3,
             fontSize, TEXT_COLOR);

    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySoundIfEnabled(sndClick);
        return true;
    }
    return false;
}

// ============================================
// মেইন মেনু
// ============================================
void DrawMainMenu() {
    ClearBackground(BG_COLOR);
    glowTimer += GetFrameTime();
    float glow = 0.5f + 0.5f * sinf(glowTimer * 2.0f);

    const char* title = "TIC TAC TOE";
    int titleWidth = MeasureTextFont(fontBlackUnicorn, title, 90);
    DrawGlowTextFont(fontBlackUnicorn, title, (screenWidth - titleWidth) / 2, 70, 90, ACCENT_O, 0.6f + glow * 0.4f);

    const char* sub = "Play With Your Friends";
    int subWidth = MeasureTextFont(fontBlackUnicorn, sub, 30);
    DrawTextFont(fontBlackUnicorn, sub, (screenWidth - subWidth) / 2, 190, 30, SUBTEXT_COLOR);

    Rectangle btnStart    = { (float)(screenWidth/2 - 150), 280, 300, 60 };
    Rectangle btnSettings = { (float)(screenWidth/2 - 150), 360, 300, 60 };
    Rectangle btnCredits  = { (float)(screenWidth/2 - 150), 440, 300, 60 };
    Rectangle btnExit     = { (float)(screenWidth/2 - 150), 520, 300, 60 };

    if (DrawButton(btnStart, "PLAY", BTN_COLOR, 30)) {
        currentState = STATE_NAME_INPUT;
        activeInputField = 0;
        player1Name[0] = '\0';
        player2Name[0] = '\0';
    }
    if (DrawButton(btnSettings, "SETTINGS", BTN_COLOR, 30)) currentState = STATE_SETTINGS;
    if (DrawButton(btnCredits, "CREDITS", BTN_COLOR, 30)) currentState = STATE_CREDITS;
    if (DrawButton(btnExit, "EXIT", {80, 30, 40, 255}, 30)) currentState = STATE_EXIT;

    DrawText("v1.0", 20, screenHeight - 30, 18, SUBTEXT_COLOR);
    if (!soundLoaded) DrawText("Warning: Sound not loaded!", 20, screenHeight - 55, 18, DANGER);
}

// ============================================
// নাম ইনপুট
// ============================================
void DrawNameInput() {
    ClearBackground(BG_COLOR);
    DrawTextCentered("ENTER PLAYER NAMES", 60, 45, ACCENT_O);

    DrawText("Player 1", 150, 180, 28, ACCENT_X);
    Rectangle box1 = { 150, 220, 600, 65 };
    DrawRectangleRounded(box1, 0.2f, 8, PANEL_COLOR);
    DrawRectangleRoundedLines(box1, 0.2f, 8, activeInputField == 0 ? ACCENT_X : SUBTEXT_COLOR);
    DrawText(player1Name, 170, 235, 40, TEXT_COLOR);
    if (activeInputField == 0 && ((int)(GetTime() * 2) % 2 == 0)) {
        DrawText("|", 170 + MeasureText(player1Name, 40), 235, 40, TEXT_COLOR);
    }

    DrawText("Player 2", 150, 330, 28, ACCENT_O);
    Rectangle box2 = { 150, 370, 600, 65 };
    DrawRectangleRounded(box2, 0.2f, 8, PANEL_COLOR);
    DrawRectangleRoundedLines(box2, 0.2f, 8, activeInputField == 1 ? ACCENT_O : SUBTEXT_COLOR);
    DrawText(player2Name, 170, 385, 40, TEXT_COLOR);
    if (activeInputField == 1 && ((int)(GetTime() * 2) % 2 == 0)) {
        DrawText("|", 170 + MeasureText(player2Name, 40), 385, 40, TEXT_COLOR);
    }

    DrawTextCentered("Tip: TAB to switch, ENTER to continue", 470, 18, SUBTEXT_COLOR);

    Rectangle btnNext = { (float)(screenWidth/2 - 150), 540, 300, 60 };
    Rectangle btnBack = { (float)(screenWidth/2 - 150), 620, 300, 55 };

    if (DrawButton(btnNext, "NEXT", {30, 100, 60, 255}, 30)) {
        if (strlen(player1Name) == 0) strcpy(player1Name, "Player 1");
        if (strlen(player2Name) == 0) strcpy(player2Name, "Player 2");
        currentState = STATE_ROUND_SETUP;
        strcpy(roundInput, "3");
    }
    if (DrawButton(btnBack, "BACK", {80, 50, 50, 255}, 28)) currentState = STATE_MAIN_MENU;

    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 125) {
            if (activeInputField == 0 && strlen(player1Name) < 20) {
                int len = strlen(player1Name);
                player1Name[len] = (char)key; player1Name[len+1] = '\0';
            } else if (activeInputField == 1 && strlen(player2Name) < 20) {
                int len = strlen(player2Name);
                player2Name[len] = (char)key; player2Name[len+1] = '\0';
            }
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (activeInputField == 0 && strlen(player1Name) > 0) player1Name[strlen(player1Name)-1] = '\0';
        else if (activeInputField == 1 && strlen(player2Name) > 0) player2Name[strlen(player2Name)-1] = '\0';
    }
    if (IsKeyPressed(KEY_TAB)) activeInputField = 1 - activeInputField;
    if (IsKeyPressed(KEY_ENTER)) {
        if (strlen(player1Name) == 0) strcpy(player1Name, "Player 1");
        if (strlen(player2Name) == 0) strcpy(player2Name, "Player 2");
        currentState = STATE_ROUND_SETUP;
        strcpy(roundInput, "3");
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, box1)) activeInputField = 0;
        else if (CheckCollisionPointRec(mouse, box2)) activeInputField = 1;
    }
}

// ============================================
// রাউন্ড সেটআপ
// ============================================
void DrawRoundSetup() {
    ClearBackground(BG_COLOR);
    DrawTextCentered("HOW MANY ROUNDS?", 70, 50, ACCENT_O);
    DrawTextCentered("Enter a number between 1 and 100", 165, 22, SUBTEXT_COLOR);

    Rectangle box = { (float)(screenWidth/2 - 150), 230, 300, 80 };
    DrawRectangleRounded(box, 0.2f, 8, PANEL_COLOR);
    DrawRectangleRoundedLines(box, 0.2f, 8, ACCENT_X);

    int tw2 = MeasureText(roundInput, 55);
    DrawText(roundInput, (screenWidth - tw2) / 2, 240, 55, TEXT_COLOR);
    if ((int)(GetTime() * 2) % 2 == 0) {
        DrawText("|", (screenWidth + tw2) / 2, 240, 55, TEXT_COLOR);
    }

    Rectangle b1  = { 200, 360, 100, 50 };
    Rectangle b3  = { 320, 360, 100, 50 };
    Rectangle b5  = { 440, 360, 100, 50 };
    Rectangle b10 = { 560, 360, 100, 50 };

    if (DrawNumberButton(b1, "1", BTN_COLOR, 28)) strcpy(roundInput, "1");
    if (DrawNumberButton(b3, "3", BTN_COLOR, 28)) strcpy(roundInput, "3");
    if (DrawNumberButton(b5, "5", BTN_COLOR, 28)) strcpy(roundInput, "5");
    if (DrawNumberButton(b10, "10", BTN_COLOR, 28)) strcpy(roundInput, "10");

    Rectangle btnStart = { (float)(screenWidth/2 - 150), 470, 300, 65 };
    Rectangle btnBack  = { (float)(screenWidth/2 - 150), 560, 300, 55 };

    if (DrawButton(btnStart, "START MATCH", {30, 130, 70, 255}, 28)) {
        int val = atoi(roundInput);
        if (val < 1) val = 1;
        if (val > 100) val = 100;
        totalRounds = val;
        currentRound = 1;
        score1 = 0; score2 = 0; draws = 0;
        ResetBoard();
        InitParticles();
        currentState = STATE_PLAYING;
    }
    if (DrawButton(btnBack, "BACK", {80, 50, 50, 255}, 28)) currentState = STATE_NAME_INPUT;

    int key = GetCharPressed();
    while (key > 0) {
        if (key >= '0' && key <= '9' && strlen(roundInput) < 3) {
            int len = strlen(roundInput);
            roundInput[len] = (char)key; roundInput[len+1] = '\0';
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && strlen(roundInput) > 0) {
        roundInput[strlen(roundInput)-1] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER)) {
        int val = atoi(roundInput);
        if (val < 1) val = 1;
        if (val > 100) val = 100;
        totalRounds = val;
        currentRound = 1;
        score1 = 0; score2 = 0; draws = 0;
        ResetBoard();
        InitParticles();
        currentState = STATE_PLAYING;
    }
}

// ============================================
// গেম স্ক্রিন
// ============================================
void DrawGame() {
    ClearBackground(BG_COLOR);
    DrawRectangle(0, 0, screenWidth, 110, PANEL_COLOR);

    if (currentPlayer == 1 && !gameOver) {
        glowTimer += GetFrameTime();
        float glow = 0.6f + 0.4f * sinf(glowTimer * 6.0f);
        for (int i = 5; i > 0; i--) {
            Color gc = ACCENT_X; gc.a = (unsigned char)(40 * glow / i);
            DrawText(player1Name, 40 - i, 25, 38, gc);
            DrawText(player1Name, 40 + i, 25, 38, gc);
            DrawText(player1Name, 40, 25 - i, 38, gc);
            DrawText(player1Name, 40, 25 + i, 38, gc);
        }
        DrawText(player1Name, 40, 25, 38, ACCENT_X);
    } else {
        DrawText(player1Name, 40, 25, 38, SUBTEXT_COLOR);
    }
    DrawText(TextFormat("Score: %d", score1), 40, 72, 22, ACCENT_X);

    if (currentPlayer == 2 && !gameOver) {
        float glow = 0.6f + 0.4f * sinf(glowTimer * 6.0f);
        int p2w = MeasureText(player2Name, 38);
        int p2x = screenWidth - 200 - p2w;
        for (int i = 5; i > 0; i--) {
            Color gc = ACCENT_O; gc.a = (unsigned char)(40 * glow / i);
            DrawText(player2Name, p2x - i, 25, 38, gc);
            DrawText(player2Name, p2x + i, 25, 38, gc);
            DrawText(player2Name, p2x, 25 - i, 38, gc);
            DrawText(player2Name, p2x, 25 + i, 38, gc);
        }
        DrawText(player2Name, p2x, 25, 38, ACCENT_O);
    } else {
        int p2w = MeasureText(player2Name, 38);
        DrawText(player2Name, screenWidth - 200 - p2w, 25, 38, SUBTEXT_COLOR);
    }
    char s2[32]; sprintf(s2, "Score: %d", score2);
    DrawText(s2, screenWidth - 200 - MeasureText(s2, 22), 72, 22, ACCENT_O);

    const char* roundInfo = TextFormat("Round %d / %d", currentRound, totalRounds);
    int riw = MeasureText(roundInfo, 24);
    DrawText(roundInfo, (screenWidth - riw) / 2, 30, 24, TEXT_COLOR);

    const char* drawsInfo = TextFormat("Draws: %d", draws);
    int diw = MeasureText(drawsInfo, 18);
    DrawText(drawsInfo, (screenWidth - diw) / 2, 62, 18, SUBTEXT_COLOR);

    Rectangle btnExitGame = { (float)(screenWidth - 130), 25, 100, 45 };
    Vector2 mousePos = GetMousePosition();
    bool exitHover = CheckCollisionPointRec(mousePos, btnExitGame);
    DrawRectangleRounded(btnExitGame, 0.4f, 8, exitHover ? (Color){200, 60, 80, 255} : (Color){90, 30, 45, 255});
    DrawRectangleRoundedLines(btnExitGame, 0.4f, 8, exitHover ? WHITE : SUBTEXT_COLOR);
    int exitTw = MeasureText("EXIT", 22);
    DrawText("EXIT", (int)(btnExitGame.x + (btnExitGame.width - exitTw) / 2),
             (int)(btnExitGame.y + (btnExitGame.height - 22) / 2) + 2, 22, TEXT_COLOR);

    for (int i = 1; i < 3; i++) {
        DrawLineEx({(float)(gridStartX + i * cellSize), (float)gridStartY},
                   {(float)(gridStartX + i * cellSize), (float)(gridStartY + 3 * cellSize)},
                   4.0f, SUBTEXT_COLOR);
        DrawLineEx({(float)gridStartX, (float)(gridStartY + i * cellSize)},
                   {(float)(gridStartX + 3 * cellSize), (float)(gridStartY + i * cellSize)},
                   4.0f, SUBTEXT_COLOR);
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int cx = gridStartX + j * cellSize + cellSize / 2;
            int cy = gridStartY + i * cellSize + cellSize / 2;
            if (board[i][j] == 1) DrawX(cx, cy, 80, ACCENT_X);
            else if (board[i][j] == 2) DrawO(cx, cy, 80, ACCENT_O);
        }
    }

    if (!gameOver) {
        const char* turnMsg = TextFormat("%s's Turn", currentPlayer == 1 ? player1Name : player2Name);
        int tw = MeasureText(turnMsg, 32);
        DrawText(turnMsg, (screenWidth - tw) / 2, 615, 32,
                 currentPlayer == 1 ? ACCENT_X : ACCENT_O);
    }
}

// ============================================
// Round Over Screen
// ============================================
void DrawRoundOver() {
    ClearBackground(BG_COLOR);

    UpdateParticles();
    DrawParticles();

    const char* title = TextFormat("ROUND %d RESULT", currentRound);
    int tw = MeasureText(title, 50);
    DrawText(title, (screenWidth - tw) / 2, 60, 50, ACCENT_O);

    if (winner == 3) {
        DrawTextCentered("It's a DRAW!", 230, 50, SUBTEXT_COLOR);
        DrawTextCentered("Both played well!", 300, 26, TEXT_COLOR);
    } else {
        const char* winnerName = (winner == 1) ? player1Name : player2Name;
        Color winnerColor = (winner == 1) ? ACCENT_X : ACCENT_O;
        const char* loserName = (winner == 1) ? player2Name : player1Name;

        DrawText("Winner:", 200, 190, 30, SUBTEXT_COLOR);
        int wm = MeasureText(winnerName, 55);
        DrawText(winnerName, (screenWidth - wm) / 2, 230, 55, winnerColor);

        DrawText("Who lost:", 200, 340, 30, SUBTEXT_COLOR);
        int lm = MeasureText(loserName, 45);
        DrawText(loserName, (screenWidth - lm) / 2, 380, 45, SUBTEXT_COLOR);

        const char* tryAgain = "Try again buddy!";
        int ta = MeasureText(tryAgain, 28);
        DrawText(tryAgain, (screenWidth - ta) / 2, 445, 28, DANGER);
    }

    // ⭐ এখানে স্কোর আপডেটেড দেখাবে
    const char* scoreLine = TextFormat("%s  %d  -  %d  %s", player1Name, score1, score2, player2Name);
    int sw = MeasureText(scoreLine, 26);
    DrawText(scoreLine, (screenWidth - sw) / 2, 520, 26, TEXT_COLOR);

    const char* info;
    if (currentRound >= totalRounds) {
        info = "Final round done — calculating winner...";
    } else {
        info = TextFormat("Next: Round %d of %d", currentRound + 1, totalRounds);
    }
    int iw = MeasureText(info, 22);
    DrawText(info, (screenWidth - iw) / 2, 580, 22, SUBTEXT_COLOR);

    DrawTextCentered("Click / Press SPACE to continue", 640, 20, SUBTEXT_COLOR);
}

void HandleRoundOver() {
    resultTimer += GetFrameTime();

    static float lastSpawn = 0.0f;
    if (resultTimer - lastSpawn > 0.4f && winner != 3) {
        SpawnFirework(2);
        lastSpawn = resultTimer;
    }

    bool timeUp = resultTimer > 2.5f;
    bool skip = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE);

    if (timeUp || skip) {
        // ⭐ স্কোর ইতিমধ্যে HandleGame() এ আপডেট হয়েছে — এখানে আর করব না

        currentRound++;

        if (currentRound > totalRounds) {
            currentState = STATE_MATCH_OVER;
            resultTimer = 0.0f;
            InitParticles();

            if (score1 > score2) PlaySoundIfEnabled(sndWin);
            else if (score2 > score1) PlaySoundIfEnabled(sndFail);
            else PlaySoundIfEnabled(sndDraw);
        } else {
            ResetBoard();
            currentState = STATE_PLAYING;
        }
    }
}

// ============================================
// Match Over Screen
// ============================================
void DrawMatchOver() {
    ClearBackground(BG_COLOR);

    UpdateParticles();
    DrawParticles();

    const char* title = "MATCH OVER!";
    int tw = MeasureText(title, 65);
    DrawText(title, (screenWidth - tw) / 2, 50, 65, SUCCESS);

    DrawTextCentered("Game Winner:", 160, 26, SUBTEXT_COLOR);

    if (score1 == score2) {
        const char* drawMsg = TextFormat("%s and %s", player1Name, player2Name);
        int dm = MeasureText(drawMsg, 48);
        DrawText(drawMsg, (screenWidth - dm) / 2, 200, 48, SUCCESS);
    } else {
        const char* champ = (score1 > score2) ? player1Name : player2Name;
        Color champColor = (score1 > score2) ? ACCENT_X : ACCENT_O;
        int cm = MeasureText(champ, 55);
        DrawText(champ, (screenWidth - cm) / 2, 200, 55, champColor);
    }

    const char* scoreLine = TextFormat("%d   -   %d", score1, score2);
    int sw = MeasureText(scoreLine, 60);
    DrawText(scoreLine, (screenWidth - sw) / 2, 290, 60, TEXT_COLOR);

    const char* drawsInfo = TextFormat("Draws: %d", draws);
    int dw = MeasureText(drawsInfo, 22);
    DrawText(drawsInfo, (screenWidth - dw) / 2, 365, 22, SUBTEXT_COLOR);

    DrawTextCentered("Who lost:", 420, 26, SUBTEXT_COLOR);

    if (score1 == score2) {
        DrawTextCentered("Nobody — it's a tie!", 460, 32, SUCCESS);
    } else {
        const char* loser = (score1 > score2) ? player2Name : player1Name;
        int lm = MeasureText(loser, 42);
        DrawText(loser, (screenWidth - lm) / 2, 460, 42, SUBTEXT_COLOR);

        const char* wish = "Wish you good luck for next game play!";
        int ww = MeasureText(wish, 24);
        DrawText(wish, (screenWidth - ww) / 2, 515, 24, DANGER);
    }

    Rectangle btnMenu = { (float)(screenWidth/2 - 150), 620, 300, 60 };
    if (DrawButton(btnMenu, "MAIN MENU", {80, 50, 50, 255}, 28)) {
        currentState = STATE_MAIN_MENU;
        resultTimer = 0.0f;
    }
}

void HandleMatchOver() {
    resultTimer += GetFrameTime();

    static float lastSpawn = 0.0f;
    if (resultTimer - lastSpawn > 0.5f) {
        SpawnFirework(3);
        lastSpawn = resultTimer;
    }

    if (resultTimer > 8.0f) {
        currentState = STATE_MAIN_MENU;
        resultTimer = 0.0f;
    }
}

// ============================================
// গেম হ্যান্ডেল
// ============================================
void HandleGame() {
    if (IsKeyPressed(KEY_M)) {
        ResetBoard();
        currentState = STATE_MAIN_MENU;
        return;
    }

    Rectangle btnExitGame = { (float)(screenWidth - 130), 25, 100, 45 };
    if (CheckCollisionPointRec(GetMousePosition(), btnExitGame) &&
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySoundIfEnabled(sndClick);
        ResetBoard();
        currentState = STATE_MAIN_MENU;
        return;
    }

    if (!gameOver && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        int col = (mouse.x - gridStartX) / cellSize;
        int row = (mouse.y - gridStartY) / cellSize;
        if (row >= 0 && row < 3 && col >= 0 && col < 3) {
            if (board[row][col] == 0) {
                board[row][col] = currentPlayer;
                PlaySoundIfEnabled(sndPlace);
                CheckWin();
                if (gameOver) {
                    if (winner == 3) PlaySoundIfEnabled(sndDraw);
                    else PlaySoundIfEnabled(sndWin);

                    // ⭐ সাথে সাথে স্কোর আপডেট — যাতে Round Over স্ক্রিনে নতুন স্কোর দেখায়
                    if (winner == 1) score1++;
                    else if (winner == 2) score2++;
                    else if (winner == 3) draws++;

                    resultTimer = 0.0f;
                    InitParticles();
                    if (winner != 3) SpawnFirework(3);
                    currentState = STATE_ROUND_OVER;
                } else {
                    currentPlayer = (currentPlayer == 1) ? 2 : 1;
                }
            }
        }
    }
}

// ============================================
// সেটিংস
// ============================================
void DrawSettings() {
    ClearBackground(BG_COLOR);
    DrawTextCentered("SETTINGS", 70, 55, ACCENT_O);

    DrawText("Sound Effects", 200, 230, 28, TEXT_COLOR);
    Rectangle btnToggle = { 550, 215, 180, 60 };
    if (DrawButton(btnToggle, soundEnabled ? "ON" : "OFF",
                   soundEnabled ? (Color){30, 130, 70, 255} : (Color){120, 40, 40, 255}, 30)) {
        soundEnabled = !soundEnabled;
        if (soundEnabled) PlaySound(sndClick);
    }

    DrawText("Language", 200, 330, 28, TEXT_COLOR);
    DrawText("English (only)", 550, 330, 28, SUBTEXT_COLOR);

    Rectangle btnBack = { (float)(screenWidth/2 - 150), 500, 300, 60 };
    if (DrawButton(btnBack, "BACK TO MENU", {80, 50, 50, 255}, 28)) currentState = STATE_MAIN_MENU;
}

// ============================================
// ক্রেডিটস
// ============================================
void DrawCredits() {
    ClearBackground(BG_COLOR);
    const char* title = "CREDITS";
    int tw = MeasureTextFont(fontWalshes, title, 80);
    DrawGlowTextFont(fontWalshes, title, (screenWidth - tw) / 2, 40, 80, ACCENT_O, 0.8f);

    DrawText("Developed by", 340, 170, 22, SUBTEXT_COLOR);

    const char* name = "Md. Nafizur Rahman";
    int nw = MeasureTextFont(fontWalshes, name, 50);
    DrawTextFont(fontWalshes, name, (screenWidth - nw) / 2, 200, 50, ACCENT_X);

    const char* dept1 = "Computer Engineering And Engineering";
    const char* dept2 = "Chittagong University of Engineering & Technology (CUET)";
    int d1w = MeasureText(dept1, 22);
    int d2w = MeasureText(dept2, 22);
    DrawText(dept1, (screenWidth - d1w) / 2, 280, 22, TEXT_COLOR);
    DrawText(dept2, (screenWidth - d2w) / 2, 315, 22, TEXT_COLOR);

    DrawText("Made with", 370, 380, 22, SUBTEXT_COLOR);
    const char* tech = "C++ & Raylib";
    int tew = MeasureTextFont(fontWalshes, tech, 38);
    DrawTextFont(fontWalshes, tech, (screenWidth - tew) / 2, 410, 38, ACCENT_O);

    const char* gh = "GitHub: github.com/Nafizur-Rahman-25";
    int ghw = MeasureText(gh, 22);
    DrawText(gh, (screenWidth - ghw) / 2, 490, 22, SUCCESS);

    Rectangle btnBack = { (float)(screenWidth/2 - 150), 580, 300, 65 };
    if (DrawButton(btnBack, "BACK", {80, 50, 50, 255}, 30)) currentState = STATE_MAIN_MENU;
}

// ============================================
// মেইন
// ============================================
int main() {
    InitWindow(screenWidth, screenHeight, "Tic Tac Toe - CUET CSE Project");
    SetTargetFPS(60);

    InitAudioDevice();

    fontWalshes      = LoadFontEx("fonts/Walshes.ttf", 96, 0, 0);
    fontBlackUnicorn = LoadFontEx("fonts/BlackUnicorn.ttf", 96, 0, 0);
    fontDeath        = LoadFontEx("fonts/Death.ttf", 96, 0, 0);

    sndClick = LoadSound("sounds/click.wav");
    sndPlace = LoadSound("sounds/place.wav");
    sndWin   = LoadSound("sounds/win.wav");
    sndDraw  = LoadSound("sounds/draw.wav");
    sndFail  = LoadSound("sounds/fail.wav");

    soundLoaded = (sndClick.frameCount > 0) && (sndPlace.frameCount > 0) &&
                  (sndWin.frameCount > 0) && (sndDraw.frameCount > 0) &&
                  (sndFail.frameCount > 0);

    SetSoundVolume(sndClick, 0.7f);
    SetSoundVolume(sndPlace, 0.8f);
    SetSoundVolume(sndWin, 0.9f);
    SetSoundVolume(sndDraw, 0.7f);
    SetSoundVolume(sndFail, 0.9f);

    InitParticles();
    srand((unsigned int)GetTime());

    while (!WindowShouldClose() && currentState != STATE_EXIT) {
        BeginDrawing();
        switch (currentState) {
            case STATE_MAIN_MENU:    DrawMainMenu();    break;
            case STATE_NAME_INPUT:   DrawNameInput();   break;
            case STATE_ROUND_SETUP:  DrawRoundSetup();  break;
            case STATE_PLAYING:      DrawGame(); HandleGame(); break;
            case STATE_ROUND_OVER:   DrawRoundOver(); HandleRoundOver(); break;
            case STATE_MATCH_OVER:   DrawMatchOver(); HandleMatchOver(); break;
            case STATE_SETTINGS:     DrawSettings();    break;
            case STATE_CREDITS:      DrawCredits();     break;
            case STATE_EXIT:         break;
        }
        EndDrawing();
    }

    UnloadFont(fontWalshes);
    UnloadFont(fontBlackUnicorn);
    UnloadFont(fontDeath);

    UnloadSound(sndClick);
    UnloadSound(sndPlace);
    UnloadSound(sndWin);
    UnloadSound(sndDraw);
    UnloadSound(sndFail);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}