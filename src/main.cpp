#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include <GyverButton.h>

#define BUTT_PIN_UP 13
#define BUTT_PIN_DWN 12
#define BUTT_PIN_LET 11
#define BUTT_PIN_RHT 10

#define FIELD_X1 2 // Координаты = смещение игрового поля = там, где бежит змея
#define FIELD_Y1 2
#define FIELD_X2 91
#define FIELD_Y2 61
#define FIELD_W 92 // Ширина игрового поля
#define FIELD_H 60 // Высота игрового поля

#define FIG_W 4 // Размеры фигуры (прамоугольник)
#define FIG_H 4
#define FIG_STEP 4 // Шаг перемещения

#define UP 1
#define DWN 2
#define LET 3
#define RHT 4
#define SNAKE_START_LENGTH 7

int score = 0;
int score_max;

void figShw(u8g2_uint_t x, u8g2_uint_t y);
void figClr(u8g2_uint_t x, u8g2_uint_t y);
void figNextPos(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t *x2, u8g2_uint_t *y2, int direction);
boolean snakeMove(int direction); // true - konec igri

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// Координаты фигуры относительно области движения
u8g2_uint_t point_x = 0;
u8g2_uint_t point_y = 0;

u8g2_uint_t snake[50][2]; // snake[0][0]

int direction = RHT;
int snakeHead;
unsigned long last_time;
int state = 0;

u8g2_uint_t cookie_x;
u8g2_uint_t cookie_y;
void cookiePos(u8g2_uint_t *x, u8g2_uint_t *y);

GButton butt_up(BUTT_PIN_UP);
GButton butt_dwn(BUTT_PIN_DWN);
GButton butt_let(BUTT_PIN_LET);
GButton butt_rht(BUTT_PIN_RHT);

void setup()
{
    u8g2.begin();
    u8g2.clearBuffer();
    // git test 1
    u8g2.sendBuffer();

    // git test 2
    // Устанавливаем цвет отрисовки по умолчанию (белый на чёрном)
    u8g2.setDrawColor(1);

    // u8g2.drawFrame(0, 0, 96, 64); // Рамка икрового поля (2 пикселя)
    // u8g2.drawFrame(1, 1, 95, 62); // *

    butt_up.setDebounce(1);         // git test 3
    butt_dwn.setDebounce(1);
    butt_let.setDebounce(1);
    butt_rht.setDebounce(1);
    butt_dwn.setClickTimeout(50);
    butt_up.setClickTimeout(50);
    butt_let.setClickTimeout(50);
    butt_rht.setClickTimeout(50);
    Serial.begin(9600);

    //#
    // score_max inicializacija
}

void loop()
{
    int i;
    switch (state)
    {
    case 0:
        // TODO: JKKJHDKJAKJDSADHKJA

        // FIXME: hjkhkjhkjk
        u8g2.setDrawColor(0);
        u8g2.drawBox(0, 0, 128, 64);
        u8g2.setDrawColor(1);
        u8g2.setFont(u8g2_font_7x14B_mf);
        u8g2.setFontDirection(0);
        u8g2.drawStr(5, 10, "press up to start");
        u8g2.drawStr(5, 30, "good luck");
        // FIXME: fkjskjf k hk kj kkjh khkhk
        u8g2.sendBuffer();
        state= 1;
        break;
        
    case 1:
        butt_up.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
        butt_dwn.tick(); // обязательная функция отработки. Должна постоянно опрашиваться
        butt_let.tick(); // обязательная функция отработки. Должна постоянно опрашиваться
        butt_rht.tick(); // обязательная функция отработки. Должна постоянно опрашиваться
        
        if (butt_up.isSingle() || butt_up.isDouble() || butt_up.isHolded() || butt_up.isHold())
        {
            state = 2;
        }
        break;

    case 2:
        snakeHead = SNAKE_START_LENGTH - 1;
        direction = RHT;
        score = 0;

        // Рисуем игровое поле
        u8g2.drawFrame(0, 0, 96, 64); // Рамка икрового поля (2 пикселя)
        u8g2.drawFrame(1, 1, 94, 62); // *
        u8g2.setDrawColor(0);
        u8g2.drawBox(2, 2, 93, 61); // zakrashivaem staruju zmeju
        u8g2.setDrawColor(1);
        u8g2.drawBox(96, 0, 32, 64); // Поле для текста (счёт)

        u8g2.setFont(u8g2_font_7x14B_mf);
        u8g2.setFontDirection(0);

        // u8g2.setFontMode(1);
        u8g2.setDrawColor(0);
        u8g2.drawStr(100, 32, "   ");
        u8g2.setDrawColor(1);

        /*# vivodim max rezultat
            sprintf(scoreStr, "%d", score);
            u8g2.setDrawColor(0);
            u8g2.drawStr(100, 32, scoreStr);
            u8g2.setDrawColor(1);
        */
        // TODO: 3
        // zapolnjaem zmeju:
        snake[0][0] = 0; // 1. hvost
        snake[0][1] = 0;
        // 2. dalshe do golovi
        for (i = 0; i < snakeHead; i++)
        {
            figNextPos(snake[i][0], snake[i][1], &snake[i + 1][0], &snake[i + 1][1], RHT);
        }

        // vivodim zmeju na ekran
        for (i = 0; i <= snakeHead; i++)
        {
            figShw(snake[i][0], snake[i][1]);
        }
        //{
        // opredeljaem koordinatu pirogka i vivodim ...
        cookiePos(&cookie_x, &cookie_y);
        figShw(cookie_x, cookie_y);
        //}
        // Рисуем фигуру в начальном положении
        // figShw(point_x, point_y);
        state = 3;
        break;
    case 3:
        if (millis() - last_time > 300)
        {
            last_time = millis();

            if (snakeMove(direction))
            {
                state = 4;
                break;
            }
        }
        butt_up.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
        butt_dwn.tick(); // обязательная функция отработки. Должна постоянно опрашиваться
        butt_let.tick(); // обязательная функция отработки. Должна постоянно опрашиваться
        butt_rht.tick(); // обязательная функция отработки. Должна постоянно опрашиваться

        // Двигаем фигуру (точку) по полю >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        if (butt_up.isSingle() || butt_up.isDouble() || butt_up.isHolded() || butt_up.isHold())
        {
            if (direction != DWN)
            {
                direction = UP;
            }
        }

        if (butt_dwn.isSingle() || butt_dwn.isDouble() || butt_dwn.isHolded() || butt_dwn.isHold())
        {
            if (direction != UP)
            {
                direction = DWN;
            }
        }

        if (butt_let.isSingle() || butt_let.isDouble() || butt_let.isHolded() || butt_let.isHold())
        {
            if (direction != RHT)
            {
                direction = LET;
            }
        }

        if (butt_rht.isSingle() || butt_rht.isDouble() || butt_rht.isHolded() || butt_rht.isHold())
        {
            if (direction != LET)
            {
                direction = RHT;
            }
        }

        break;
    case 4:
        u8g2.setDrawColor(0);
        u8g2.drawBox(3, 3, 93, 61);
        u8g2.setDrawColor(1);
        u8g2.setFont(u8g2_font_7x14B_mf);
        u8g2.setFontDirection(0);

//# fixiruem maximalnij resultat

        u8g2.drawStr(15, 15, "Game over!");
        u8g2.drawStr(15, 35, "  press");
        u8g2.drawStr(15, 55, "   DWN");

        state = 5;
        break;
    case 5:
        butt_up.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
        butt_dwn.tick(); // обязательная функция отработки. Должна постоянно опрашиваться
        butt_let.tick(); // обязательная функция отработки. Должна постоянно опрашиваться
        butt_rht.tick(); // обязательная функция отработки. Должна постоянно опрашиваться

        if (butt_dwn.isSingle() || butt_dwn.isDouble() || butt_dwn.isHolded() || butt_dwn.isHold())
        {
            state = 0;
        }
        break;
    }
    u8g2.sendBuffer();
}

void figShw(u8g2_uint_t x, u8g2_uint_t y)
{
    u8g2.drawBox(x + FIELD_X1, y + FIELD_Y1, (u8g2_uint_t)FIG_W - 1, (u8g2_uint_t)FIG_H - 1);
}

void figClr(u8g2_uint_t x, u8g2_uint_t y)
{
    u8g2.setDrawColor(0);
    figShw(x, y);
    u8g2.setDrawColor(1); // Востанавливаем цвет отрисовки по умолчанию
}

void figNextPos(u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t *x2, u8g2_uint_t *y2, int direction)
{
    switch (direction)
    {
    case UP:
        if (y1 > FIG_STEP)
            (*y2) = y1 - FIG_STEP;
        else
            (*y2) = 0;
        (*x2) = x1;
        break;
    case DWN:
        if (y1 < FIELD_H - FIG_H - FIG_STEP)
            (*y2) = y1 + FIG_STEP;
        else
            (*y2) = FIELD_H - FIG_H;
        (*x2) = x1;
        break;
    case LET:
        if (x1 > FIG_STEP)
            (*x2) = x1 - FIG_STEP;
        else
            (*x2) = 0;
        (*y2) = y1;
        break;
    case RHT:
        if (x1 < FIELD_W - FIG_W - FIG_STEP)
            (*x2) = x1 + FIG_STEP;
        else
            (*x2) = FIELD_W - FIG_W;
        (*y2) = y1;
        break;
    }
    return;
}

boolean snakeMove(int direction)
{
    char scoreStr[3];
    u8g2_uint_t x2, y2;
    figNextPos(snake[snakeHead][0], snake[snakeHead][1], &x2, &y2, direction);
    if (((x2 == snake[snakeHead - 1][0]) && (y2 == snake[snakeHead - 1][1])) || ((x2 == snake[snakeHead][0]) && (y2 == snake[snakeHead][1])))
        return true;
    else
    {
        if (x2 == cookie_x && y2 == cookie_y)
        { // sjeli pirog
            snakeHead++;
            snake[snakeHead][0] = x2;
            snake[snakeHead][1] = y2;
            cookiePos(&cookie_x, &cookie_y);
            figShw(cookie_x, cookie_y);

            score++;
            sprintf(scoreStr, "%d", score);
            u8g2.setDrawColor(0);
            u8g2.drawStr(100, 32, scoreStr);
            u8g2.setDrawColor(1);
            return false;
        }
        else
        { // sdvigaem zmeju

            figClr(snake[0][0], snake[0][1]);
            for (int i = 0; i < snakeHead; i++)
            {
                snake[i][0] = snake[i + 1][0];
                snake[i][1] = snake[i + 1][1];
            }
            snake[snakeHead][0] = x2;
            snake[snakeHead][1] = y2;
            figShw(snake[snakeHead][0], snake[snakeHead][1]);

            // BUDET PROVERKA PERESECHENIJA....

            for (int i = 0; i < snakeHead - 4; i++)
            {
                if (snake[snakeHead][0] == snake[i][0] && snake[snakeHead][1] == snake[i][1])
                    return true;
            }

            return false;
        }
    }
}

void cookiePos(u8g2_uint_t *x, u8g2_uint_t *y)
{
    u8g2_uint_t x2, y2;
    boolean pos_state;

    do
    {
        pos_state = true;
        x2 = (random(FIELD_W) / 4) * 4;
        y2 = (random(FIELD_H) / 4) * 4;
        for (int i = 0; i <= snakeHead; i++)
            if (snake[i][0] == x2 && snake[i][1] == y2)
                pos_state = false;
    } while (pos_state == false);
    (*x) = x2;
    (*y) = y2;
}