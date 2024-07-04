#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define W 20
#define H 20
#define C 20
#define S 150
#define P 20

typedef struct { int length; POINT *body; char direction; } Snake;
typedef struct { int x, y; } Food;

Snake snake;
Food food;
int game_over, score, quit_game, can_move = 1;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void setup(), draw(HDC), drawGameOver(HDC), input(WPARAM), logic();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {.lpfnWndProc = WindowProc, .hInstance = hInstance, .lpszClassName = "Snake Game"};
    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, "Snake Game", "Snake Game", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, (W + 2) * C + P, (H + 2) * C + P + 50,
        NULL, NULL, hInstance, NULL);
    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);
    setup();
    MSG msg;
    DWORD prevTime = GetTickCount();
    while (!quit_game) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!game_over) {
            DWORD currentTime = GetTickCount();
            if (currentTime - prevTime > S) {
                logic();
                InvalidateRect(hwnd, NULL, TRUE);
                prevTime = currentTime;
                can_move = 1;
            }
        }
    }
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY: PostQuitMessage(0); return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            game_over ? drawGameOver(hdc) : draw(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_KEYDOWN:
            if (!game_over) input(wParam);
            else if (wParam == VK_RETURN) { setup(); InvalidateRect(hwnd, NULL, TRUE); }
            else if (wParam == 'X') { quit_game = 1; PostQuitMessage(0); }
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void setup() {
    game_over = score = 0;
    snake.length = 1;
    snake.direction = 'R';
    snake.body = (POINT*)malloc(sizeof(POINT) * W * H);
    snake.body[0] = (POINT){W / 2, H / 2};
    srand(time(NULL));
    food.x = rand() % W;
    food.y = rand() % H;
}

void draw(HDC hdc) {
    HBRUSH brushes[] = {
        CreateSolidBrush(RGB(0, 255, 0)),
        CreateSolidBrush(RGB(255, 0, 0)),
        CreateSolidBrush(RGB(240, 240, 240)),
        CreateSolidBrush(RGB(100, 100, 100))
    };
    HPEN hPenFrame = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    RECT rect;
    GetClientRect(WindowFromDC(hdc), &rect);
    FillRect(hdc, &rect, brushes[2]);
    SelectObject(hdc, hPenFrame);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, (W + 2) * C, (H + 2) * C);
    SelectObject(hdc, brushes[3]);
    for (int i = 0; i < W + 2; i++) {
        Rectangle(hdc, i * C, 0, (i + 1) * C, C);
        Rectangle(hdc, i * C, (H + 1) * C, (i + 1) * C, (H + 2) * C);
    }
    for (int i = 1; i < H + 1; i++) {
        Rectangle(hdc, 0, i * C, C, (i + 1) * C);
        Rectangle(hdc, (W + 1) * C, i * C, (W + 2) * C, (i + 1) * C);
    }
    SelectObject(hdc, brushes[0]);
    for (int i = 0; i < snake.length; i++)
        Rectangle(hdc, (snake.body[i].x + 1) * C, (snake.body[i].y + 1) * C, 
                  (snake.body[i].x + 2) * C, (snake.body[i].y + 2) * C);
    SelectObject(hdc, brushes[1]);
    Rectangle(hdc, (food.x + 1) * C, (food.y + 1) * C, 
              (food.x + 2) * C, (food.y + 2) * C);
    char scoreStr[20];
    sprintf(scoreStr, "Score: %d", score);
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    TextOut(hdc, 10, (H + 2) * C + 5, scoreStr, strlen(scoreStr));
    for (int i = 0; i < 4; i++) DeleteObject(brushes[i]);
    DeleteObject(hPenFrame);
}

void input(WPARAM key) {
    if (can_move) {
        char new_dir = snake.direction;
        switch (key) {
            case VK_LEFT: if (snake.direction != 'R') new_dir = 'L'; break;
            case VK_RIGHT: if (snake.direction != 'L') new_dir = 'R'; break;
            case VK_UP: if (snake.direction != 'D') new_dir = 'U'; break;
            case VK_DOWN: if (snake.direction != 'U') new_dir = 'D'; break;
        }
        snake.direction = new_dir;
        can_move = 0;
    }
}

void logic() {
    POINT prev = snake.body[0], prev2;
    switch (snake.direction) {
        case 'L': snake.body[0].x--; break;
        case 'R': snake.body[0].x++; break;
        case 'U': snake.body[0].y--; break;
        case 'D': snake.body[0].y++; break;
    }
    if (snake.body[0].x < 0 || snake.body[0].x >= W || snake.body[0].y < 0 || snake.body[0].y >= H) {
        game_over = 1;
        return;
    }
    for (int i = 1; i < snake.length; i++) {
        prev2 = snake.body[i];
        snake.body[i] = prev;
        prev = prev2;
        if (snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y)
            game_over = 1;
    }
    if (snake.body[0].x == food.x && snake.body[0].y == food.y) {
        score += 10;
        food.x = rand() % W;
        food.y = rand() % H;
        snake.body[snake.length] = prev;
        snake.length++;
    }
}

void drawGameOver(HDC hdc) {
    RECT rect;
    GetClientRect(WindowFromDC(hdc), &rect);
    HBRUSH hBrushBackground = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rect, hBrushBackground);
    SetTextColor(hdc, RGB(255, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    char gameOverStr[] = "Game Over!";
    DrawText(hdc, gameOverStr, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    char scoreStr[50];
    sprintf(scoreStr, "Final Score: %d", score);
    rect.top += 30;
    DrawText(hdc, scoreStr, -1, &rect, DT_CENTER | DT_SINGLELINE);
    char restartStr[] = "Press Enter to restart";
    rect.top += 30;
    DrawText(hdc, restartStr, -1, &rect, DT_CENTER | DT_SINGLELINE);
    DeleteObject(hBrushBackground);
}
