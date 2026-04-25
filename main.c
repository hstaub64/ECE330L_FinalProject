/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c  LAB6
 * @brief          : Main program body
 ******************************************************************************
 */
/* USER CODE END Header */

#include "main.h"
#include "stdio.h"
#include "usb_host.h"
#include "seg7.h"

I2C_HandleTypeDef hi2c1;
I2S_HandleTypeDef hi2s3;
SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim7;

/* USER CODE BEGIN PV */
int DelayValue = 50;
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM7_Init(void);
void MX_USB_HOST_Process(void);
void Layered_Display(void);

/* USER CODE BEGIN 0 */

char ramp = 0;
char RED_BRT = 0;
char GREEN_BRT = 0;
char BLUE_BRT = 0;
char RED_STEP = 1;
char GREEN_STEP = 2;
char BLUE_STEP = 3;
char DIM_Enable = 0;
char Music_ON = 0;
int TONE = 0;
int COUNT = 0;
int INDEX = 0;
int Note = 0;
int Save_Note = 0;
int Vibrato_Depth = 1;
int Vibrato_Rate = 40;
int Vibrato_Count = 0;
char Animate_On = 0;
char Message_Length = 0;
char *Message_Pointer;
char *Save_Pointer;
int Delay_msec = 0;
int Delay_counter = 0;
int CRC_Tx = 0xaaddf4d0;
int CRC_Rx = 0;

void message_display(char[]);

/* Battle Ship Player 1 Place ships */
char Message1[] =
    {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_B, CHAR_A, CHAR_T, CHAR_T, CHAR_L, CHAR_E, SPACE,
     CHAR_S, CHAR_H, CHAR_I, CHAR_P,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_Y, CHAR_E, CHAR_R, CHAR_1, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_C, CHAR_E, SPACE,
     CHAR_S, CHAR_H, CHAR_I, CHAR_P, CHAR_S,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE};  // <-- add these 8

/* Player 2 Place Ships */
char Message2[] =
    {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_E, CHAR_R, CHAR_2, SPACE, CHAR_P, CHAR_L, CHAR_A, CHAR_C, CHAR_E, SPACE,
     CHAR_S, CHAR_H, CHAR_I, CHAR_P, CHAR_S,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE};

/* Player 1 attack */
char Message3[] =
    {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_E, CHAR_R, CHAR_1, SPACE,
     CHAR_A, CHAR_T, CHAR_T, CHAR_A, CHAR_C, CHAR_K, SPACE,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE};

/* Player 2 attack */
char Message4[] =
    {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_E, CHAR_R, CHAR_2, SPACE,
     CHAR_A, CHAR_T, CHAR_T, CHAR_A, CHAR_C, CHAR_K,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE};

void Start_Message(char *msg, int length)
{
  Cursor_On = 0;
  Animate_On = 1;
  Message_Pointer = msg;
  Save_Pointer = msg;
  Message_Length = length;
  Delay_msec = 200;
}

Music Song[100];

// NOTE: Game_Display is defined in stm32f4xx_it.c

static char seg_cycle[] = {
    (1<<0),  // top
    (1<<6),  // middle
    (1<<3),  // bottom
    (1<<5),  // upper-left
    (1<<4),  // lower-left
    (1<<1),  // upper-right
    (1<<2),  // lower-right
};

int count_map_segments(map_t m)
{
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if (m.horizontal[0][i]) count++;
        if (m.horizontal[1][i]) count++;
        if (m.horizontal[2][i]) count++;
        if (m.vertical[0][i])   count++;
        if (m.vertical[1][i])   count++;
        if (m.vertical[0][i+8]) count++;
        if (m.vertical[1][i+8]) count++;
    }
    return count;
}

int check_win(map_t hit_map, map_t boat_map)
{
    for (int i = 0; i < 8; i++) {
        if (boat_map.horizontal[0][i] && !hit_map.horizontal[0][i]) return 0;
        if (boat_map.horizontal[1][i] && !hit_map.horizontal[1][i]) return 0;
        if (boat_map.horizontal[2][i] && !hit_map.horizontal[2][i]) return 0;
        if (boat_map.vertical[0][i]   && !hit_map.vertical[0][i])   return 0;
        if (boat_map.vertical[1][i]   && !hit_map.vertical[1][i])   return 0;
        if (boat_map.vertical[0][i+8] && !hit_map.vertical[0][i+8]) return 0;
        if (boat_map.vertical[1][i+8] && !hit_map.vertical[1][i+8]) return 0;
    }
    return 1;
}


int P1_singles = 0;
int P1_doubles = 0;
int P2_singles = 0;
int P2_doubles = 0;
int double_started = 0;  // tracks if first segment of double boat is placed

// Horizontal/vertical segment sets
static char seg_horizontal[] = { (1<<0), (1<<6), (1<<3) };          // top, mid, bot
static char seg_vertical[]   = { (1<<5), (1<<4), (1<<1), (1<<2) };  // UL, LL, UR, LR



/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM7_Init();

  GPIOD->MODER  = 0x55555555;
  GPIOA->MODER |= 0x000000FF;
  GPIOE->MODER |= 0x55555555;
  GPIOC->MODER |= 0x0;
  GPIOE->ODR    = 0xFFFF;

  GPIOC->MODER &= ~((3 << 20) | (3 << 22));

  GPIOC->MODER &= ~(3 << 16);

  RCC->APB2ENR |= 1 << 8;
  ADC1->SMPR2  |= 1;
  ADC1->CR2    |= 1;

  RCC->AHB1ENR |= 1 << 12;

  TIM7->PSC   = 199;
  TIM7->ARR   = 1;
  TIM7->DIER |= 1;
  TIM7->CR1  |= 1;



  int game = 0;
  int i;
  int phase = 0;





  while (1)
  {
    switch (game)
    {

    case 0: // title screen — scroll "BATTLE SHIP PLAYER1 PLACE SHIPS"
    {
      Start_Message(Message1, sizeof(Message1) / sizeof(Message1[0]));

      for (i = 0; i < Message_Length; i++)
        CRC->DR = Message1[i];

      CRC_Rx = CRC->DR;
      GPIOD->ODR = CRC_Rx ^ CRC_Tx;

      HAL_Delay(10000);   // let message scroll

      Animate_On = 0;
      HAL_Delay(1000);
      for (i = 0; i < 8; i++)
        Seven_Segment_Digit(i, SPACE, 0);
      HAL_Delay(500);

      game = 1;
      break;
    }






    case 1: // P1 placing ships - DOUBLES FIRST then singles
    {
        Display_Mode = 0;
        Animate_On   = 0;
        Cursor_On    = 1;
        Delay_msec   = 50;


        int seg_mode = ((GPIOC->IDR >> 0) & 1);

        // PA1 -> which 7seg digit (0-6 for doubles, 0-7 for singles)
        ADC1->SQR3 = 1;
        ADC1->CR2 |= (1 << 30);
        int timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);

        if (P1_doubles < 2)
            Cursor_Digit = (ADC1->DR * 7) / 4096;
        else
            Cursor_Digit = (ADC1->DR * 8) / 4096;

        // PA2 -> which segment pair or single, based on SW8
        ADC1->SQR3 = 2;
        ADC1->CR2 |= (1 << 30);
        timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);
        int pair = (ADC1->DR * 2) / 4096;  // 0 or 1

        if (P1_doubles < 2)
        {
            // doubles always vertical pairs only
            if (pair == 0) { Cursor_Segment = (1<<5); Cursor_Segment2 = (1<<4); }  // f + e
            else           { Cursor_Segment = (1<<1); Cursor_Segment2 = (1<<2); }  // b + c
        }
        else
        {
            // single mode: full segment selection
            if (seg_mode == 0)
                Cursor_Segment = seg_horizontal[(ADC1->DR * 3) / 4096];
            else
                Cursor_Segment = seg_vertical[(ADC1->DR * 4) / 4096];
            Cursor_Segment2 = 0;
        }

        // Read buttons
        int PC10 = !((GPIOC->IDR >> 10) & 1);  // SW6 = place
        int PC11 = !((GPIOC->IDR >> 11) & 1);  // SW7 = done

        if (PC10 == 1)
        {
            int d   = Cursor_Digit;
            char s  = Cursor_Segment;
            char s2 = Cursor_Segment2;

            if (P1_doubles < 2)
            {
                // DOUBLES FIRST - both segments on same digit
                if      (s  == (1<<0)) Player_Map.horizontal[0][d] = 1;
                else if (s  == (1<<6)) Player_Map.horizontal[1][d] = 1;
                else if (s  == (1<<3)) Player_Map.horizontal[2][d] = 1;
                else if (s  == (1<<5)) Player_Map.vertical[0][d]   = 1;
                else if (s  == (1<<4)) Player_Map.vertical[1][d]   = 1;
                else if (s  == (1<<1)) Player_Map.vertical[0][d+8] = 1;
                else if (s  == (1<<2)) Player_Map.vertical[1][d+8] = 1;

                if      (s2 == (1<<0)) Player_Map.horizontal[0][d] = 1;
                else if (s2 == (1<<6)) Player_Map.horizontal[1][d] = 1;
                else if (s2 == (1<<3)) Player_Map.horizontal[2][d] = 1;
                else if (s2 == (1<<5)) Player_Map.vertical[0][d]   = 1;
                else if (s2 == (1<<4)) Player_Map.vertical[1][d]   = 1;
                else if (s2 == (1<<1)) Player_Map.vertical[0][d+8] = 1;
                else if (s2 == (1<<2)) Player_Map.vertical[1][d+8] = 1;

                P1_doubles++;
                Cursor_Segment2 = 0;
                Layered_Display();
            }
            else if (P1_singles < 3)
            {
                // SINGLES SECOND - check overlap
                int occupied = 0;
                if      (s == (1<<0)) occupied = Player_Map.horizontal[0][d];
                else if (s == (1<<6)) occupied = Player_Map.horizontal[1][d];
                else if (s == (1<<3)) occupied = Player_Map.horizontal[2][d];
                else if (s == (1<<5)) occupied = Player_Map.vertical[0][d];
                else if (s == (1<<4)) occupied = Player_Map.vertical[1][d];
                else if (s == (1<<1)) occupied = Player_Map.vertical[0][d+8];
                else if (s == (1<<2)) occupied = Player_Map.vertical[1][d+8];

                if (!occupied)
                {
                    if      (s == (1<<0)) Player_Map.horizontal[0][d] = 1;
                    else if (s == (1<<6)) Player_Map.horizontal[1][d] = 1;
                    else if (s == (1<<3)) Player_Map.horizontal[2][d] = 1;
                    else if (s == (1<<5)) Player_Map.vertical[0][d]   = 1;
                    else if (s == (1<<4)) Player_Map.vertical[1][d]   = 1;
                    else if (s == (1<<1)) Player_Map.vertical[0][d+8] = 1;
                    else if (s == (1<<2)) Player_Map.vertical[1][d+8] = 1;
                    P1_singles++;
                    Layered_Display();
                }
            }


            HAL_Delay(250);
        }

        if (PC11 == 1)
        {
            if (P1_doubles >= 2 && P1_singles >= 3)
            {
                Cursor_On       = 0;
                Cursor_Segment2 = 0;
                game = 2;
            }
            HAL_Delay(250);
        }

        break;
    }






    case 2: // P2 placing ships - DOUBLES FIRST then singles
    {
        if (phase == 0)
        {
            Display_Mode    = 1;
            Start_Message(Message2, sizeof(Message2) / sizeof(Message2[0]));
            HAL_Delay(6000);
            Animate_On      = 0;
            Cursor_On       = 1;
            Cursor_Segment2 = 0;
            Delay_msec      = 50;
            phase = 1;
        }



        int seg_mode = ((GPIOC->IDR >> 0) & 1);

        // PA1 -> digit (0-6 for doubles, 0-7 for singles)
        ADC1->SQR3 = 1;
        ADC1->CR2 |= (1 << 30);
        int timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);

        if (P2_doubles < 2)
            Cursor_Digit = (ADC1->DR * 7) / 4096;
        else
            Cursor_Digit = (ADC1->DR * 8) / 4096;

        // PA2 -> segment pair or single based on SW8
        ADC1->SQR3 = 2;
        ADC1->CR2 |= (1 << 30);
        timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);
        int pair = (ADC1->DR * 2) / 4096;

        if (P2_doubles < 2)
        {
            // doubles always vertical pairs only
            if (pair == 0) { Cursor_Segment = (1<<5); Cursor_Segment2 = (1<<4); }  // f + e
            else           { Cursor_Segment = (1<<1); Cursor_Segment2 = (1<<2); }  // b + c
        }

        else
        {
            // single mode: full segment selection
            if (seg_mode == 0)
                Cursor_Segment = seg_horizontal[(ADC1->DR * 3) / 4096];
            else
                Cursor_Segment = seg_vertical[(ADC1->DR * 4) / 4096];
            Cursor_Segment2 = 0;
        }

        // Read buttons
        int PC10 = !((GPIOC->IDR >> 10) & 1);
        int PC11 = !((GPIOC->IDR >> 11) & 1);

        if (PC10 == 1)
        {
            int d   = Cursor_Digit;
            char s  = Cursor_Segment;
            char s2 = Cursor_Segment2;

            if (P2_doubles < 2)
            {
                // DOUBLES FIRST - both segments on same digit
                if      (s  == (1<<0)) Player2_Map.horizontal[0][d] = 1;
                else if (s  == (1<<6)) Player2_Map.horizontal[1][d] = 1;
                else if (s  == (1<<3)) Player2_Map.horizontal[2][d] = 1;
                else if (s  == (1<<5)) Player2_Map.vertical[0][d]   = 1;
                else if (s  == (1<<4)) Player2_Map.vertical[1][d]   = 1;
                else if (s  == (1<<1)) Player2_Map.vertical[0][d+8] = 1;
                else if (s  == (1<<2)) Player2_Map.vertical[1][d+8] = 1;

                if      (s2 == (1<<0)) Player2_Map.horizontal[0][d] = 1;
                else if (s2 == (1<<6)) Player2_Map.horizontal[1][d] = 1;
                else if (s2 == (1<<3)) Player2_Map.horizontal[2][d] = 1;
                else if (s2 == (1<<5)) Player2_Map.vertical[0][d]   = 1;
                else if (s2 == (1<<4)) Player2_Map.vertical[1][d]   = 1;
                else if (s2 == (1<<1)) Player2_Map.vertical[0][d+8] = 1;
                else if (s2 == (1<<2)) Player2_Map.vertical[1][d+8] = 1;

                P2_doubles++;
                Cursor_Segment2 = 0;
                Layered_Display();
            }
            else if (P2_singles < 3)
            {
                // SINGLES SECOND - check overlap
                int occupied = 0;
                if      (s == (1<<0)) occupied = Player2_Map.horizontal[0][d];
                else if (s == (1<<6)) occupied = Player2_Map.horizontal[1][d];
                else if (s == (1<<3)) occupied = Player2_Map.horizontal[2][d];
                else if (s == (1<<5)) occupied = Player2_Map.vertical[0][d];
                else if (s == (1<<4)) occupied = Player2_Map.vertical[1][d];
                else if (s == (1<<1)) occupied = Player2_Map.vertical[0][d+8];
                else if (s == (1<<2)) occupied = Player2_Map.vertical[1][d+8];


                if (!occupied)
                {
                    if      (s == (1<<0)) Player2_Map.horizontal[0][d] = 1;
                    else if (s == (1<<6)) Player2_Map.horizontal[1][d] = 1;
                    else if (s == (1<<3)) Player2_Map.horizontal[2][d] = 1;
                    else if (s == (1<<5)) Player2_Map.vertical[0][d]   = 1;
                    else if (s == (1<<4)) Player2_Map.vertical[1][d]   = 1;
                    else if (s == (1<<1)) Player2_Map.vertical[0][d+8] = 1;
                    else if (s == (1<<2)) Player2_Map.vertical[1][d+8] = 1;
                    P2_singles++;
                    Layered_Display();
                }
            }

            GPIOD->ODR = (P2_doubles << 4) | P2_singles;  // debug LEDs
            HAL_Delay(250);
        }

        if (PC11 == 1)
        {
            if (P2_doubles >= 2 && P2_singles >= 3)
            {
                Cursor_On       = 0;
                Cursor_Segment2 = 0;
                phase = 0;
                game  = 3;
            }
            HAL_Delay(250);
        }

        break;
    }




    case 3: // P1 attack message
    {
        Display_Mode = 2;  // show P2's board with P1's shots
        Start_Message(Message3, sizeof(Message3) / sizeof(Message3[0]));
        HAL_Delay(6000);
        Animate_On  = 0;
        Cursor_On   = 1;
        Cursor_Digit   = 0;
        Cursor_Segment = (1<<0);
        Cursor_Segment2 = 0;
        Delay_msec  = 50;
        game = 4;
        break;
    }

    case 4: // P1 shooting at P2's map
    {
        Display_Mode = 2;
        Cursor_On    = 1;
        Animate_On   = 0;
        Delay_msec   = 50;

        // SW1 (PC0): 0 = horizontal, 1 = vertical
        int seg_mode = ((GPIOC->IDR >> 0) & 1);

        // PA1 -> digit
        ADC1->SQR3 = 1;
        ADC1->CR2 |= (1 << 30);
        int timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);
        Cursor_Digit = (ADC1->DR * 8) / 4096;

        // PA2 -> segment based on SW1
        ADC1->SQR3 = 2;
        ADC1->CR2 |= (1 << 30);
        timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);

        if (seg_mode == 0)
            Cursor_Segment = seg_horizontal[(ADC1->DR * 3) / 4096];
        else
            Cursor_Segment = seg_vertical[(ADC1->DR * 4) / 4096];

        Cursor_Segment2 = 0;

        // Read buttons
        int PC10 = !((GPIOC->IDR >> 10) & 1);  // SW6 = fire shot

        if (PC10 == 1)
        {
            int d  = Cursor_Digit;
            char s = Cursor_Segment;

            // record shot in P1_Hits map
            if      (s == (1<<0)) P1_Hits.horizontal[0][d] = 1;
            else if (s == (1<<6)) P1_Hits.horizontal[1][d] = 1;
            else if (s == (1<<3)) P1_Hits.horizontal[2][d] = 1;
            else if (s == (1<<5)) P1_Hits.vertical[0][d]   = 1;
            else if (s == (1<<4)) P1_Hits.vertical[1][d]   = 1;
            else if (s == (1<<1)) P1_Hits.vertical[0][d+8] = 1;
            else if (s == (1<<2)) P1_Hits.vertical[1][d+8] = 1;

            Layered_Display();
            HAL_Delay(500);  // pause so player can see result

            // check if P1 wins
            if (check_win(P1_Hits, Player2_Map))
            {
                game = 7;  // P1 wins
            }
            else
            {
                game = 5;  // P2's turn
            }
        }

        break;
    }

    case 5: // P2 attack message
    {
        Display_Mode = 3;  // show P1's board with P2's shots
        Start_Message(Message4, sizeof(Message4) / sizeof(Message4[0]));
        HAL_Delay(6000);
        Animate_On  = 0;
        Cursor_On   = 1;
        Cursor_Digit   = 0;
        Cursor_Segment = (1<<0);
        Cursor_Segment2 = 0;
        Delay_msec  = 50;
        game = 6;
        break;
    }

    case 6: // P2 shooting at P1's map
    {
        Display_Mode = 3;
        Cursor_On    = 1;
        Animate_On   = 0;
        Delay_msec   = 50;

        // SW1 (PC0): 0 = horizontal, 1 = vertical
        int seg_mode = ((GPIOC->IDR >> 0) & 1);

        // PA1 -> digit
        ADC1->SQR3 = 1;
        ADC1->CR2 |= (1 << 30);
        int timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);
        Cursor_Digit = (ADC1->DR * 8) / 4096;

        // PA2 -> segment based on SW1
        ADC1->SQR3 = 2;
        ADC1->CR2 |= (1 << 30);
        timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);

        if (seg_mode == 0)
            Cursor_Segment = seg_horizontal[(ADC1->DR * 3) / 4096];
        else
            Cursor_Segment = seg_vertical[(ADC1->DR * 4) / 4096];

        Cursor_Segment2 = 0;

        // Read buttons
        int PC10 = !((GPIOC->IDR >> 10) & 1);  // SW6 = fire shot

        if (PC10 == 1)
        {
            int d  = Cursor_Digit;
            char s = Cursor_Segment;

            // record shot in P2_Hits map
            if      (s == (1<<0)) P2_Hits.horizontal[0][d] = 1;
            else if (s == (1<<6)) P2_Hits.horizontal[1][d] = 1;
            else if (s == (1<<3)) P2_Hits.horizontal[2][d] = 1;
            else if (s == (1<<5)) P2_Hits.vertical[0][d]   = 1;
            else if (s == (1<<4)) P2_Hits.vertical[1][d]   = 1;
            else if (s == (1<<1)) P2_Hits.vertical[0][d+8] = 1;
            else if (s == (1<<2)) P2_Hits.vertical[1][d+8] = 1;

            Layered_Display();
            HAL_Delay(500);  // pause so player can see result

            // check if P2 wins
            if (check_win(P2_Hits, Player_Map))
            {
                game = 7;  // P2 wins
            }
            else
            {
                game = 3;  // P1's turn again
            }
        }

        break;
    }

    case 7: // win screen
    {
        Cursor_On   = 0;
        Animate_On  = 0;

        // figure out who won and show message
        if (check_win(P1_Hits, Player2_Map))
        {
            // P1 wins - scroll "PLAYER 1 WINS"
            Seven_Segment_Digit(7, CHAR_P, 0);
            Seven_Segment_Digit(6, CHAR_L, 0);
            Seven_Segment_Digit(5, CHAR_A, 0);
            Seven_Segment_Digit(4, CHAR_Y, 0);
            Seven_Segment_Digit(3, CHAR_E, 0);
            Seven_Segment_Digit(2, CHAR_R, 0);
            Seven_Segment_Digit(1, CHAR_1, 0);
            Seven_Segment_Digit(0, SPACE,  0);
        }
        else
        {
            // P2 wins
            Seven_Segment_Digit(7, CHAR_P, 0);
            Seven_Segment_Digit(6, CHAR_L, 0);
            Seven_Segment_Digit(5, CHAR_A, 0);
            Seven_Segment_Digit(4, CHAR_Y, 0);
            Seven_Segment_Digit(3, CHAR_E, 0);
            Seven_Segment_Digit(2, CHAR_R, 0);
            Seven_Segment_Digit(1, CHAR_2, 0);
            Seven_Segment_Digit(0, SPACE,  0);
        }

        // stay here forever until reset
        break;
    }






    default:
    {
      game = 0;
      break;
    }
    }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) Error_Handler();
}

static void MX_TIM7_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  htim7.Instance                        = TIM7;
  htim7.Init.Prescaler                  = 0;
  htim7.Init.CounterMode                = TIM_COUNTERMODE_UP;
  htim7.Init.Period                     = 65535;
  htim7.Init.AutoReloadPreload          = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK) Error_Handler();
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, LD4_Pin | LD3_Pin | LD5_Pin | LD6_Pin | Audio_RST_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin   = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin   = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin       = PDM_OUT_Pin;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin  = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  // PC10 = place ship, PC11 = done

  GPIO_InitStruct.Pin  = GPIO_PIN_10 | GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);



  GPIO_InitStruct.Pin  = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

  // Init message scroll on startup
  Message_Pointer = &Message1[0];
  Save_Pointer    = &Message1[0];
  Message_Length  = sizeof(Message1) / sizeof(Message1[0]);
  Delay_msec      = 200;
  Animate_On      = 1;

  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin       = CLK_IN_Pin;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(CLK_IN_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin   = LD4_Pin | LD3_Pin | LD5_Pin | LD6_Pin | Audio_RST_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin  = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin  = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}
