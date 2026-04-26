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

/* declarations for functions defined later in the file  */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM7_Init(void);
void MX_USB_HOST_Process(void);
void Layered_Display(void);

/* USER CODE BEGIN 0 */

unsigned char ramp = 0; /* ramp is used as an unsigned char cycling 0-255 at 1640Hz via TIM7 interrupt Used in Add_Shots_To_Display in stm32f4xx_it.c to allow PWM dimming on missed shots. It is unsigned so it wraps 0->255->0 without going negative */
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



char Animate_On = 0; // when 1 = SysTick is activley scrolling a message
char Message_Length = 0; // total number of characters in current message (stores how many char in current message array)
char *Message_Pointer; // current read position within the message array (used to starts at first character of an message array helps create the scrolling effect moves forward by one becuase only 8 char allowed at once)
char *Save_Pointer; // beginning of message array for wrap around reset
int Delay_msec = 0; // how many ms to wait between each scroll step (scroll speed)
int Delay_counter = 0;// ms counter and resets when it hits Delay_msec (counts the seconds until it reaches



/* Battle Ship Player 1 Place ships */
char Message1[] =
    {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_B, CHAR_A, CHAR_T, CHAR_T, CHAR_L, CHAR_E, SPACE,
     CHAR_S, CHAR_H, CHAR_I, CHAR_P,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_Y, CHAR_E, CHAR_R, CHAR_1, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_C, CHAR_E, SPACE,
     CHAR_S, CHAR_H, CHAR_I, CHAR_P, CHAR_S,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE};

/* Player 2 Place Ships */
char Message2[] =
    {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_Y,CHAR_E, CHAR_R, CHAR_2, SPACE, CHAR_P, CHAR_L, CHAR_A, CHAR_C, CHAR_E, SPACE,
     CHAR_S, CHAR_H, CHAR_I, CHAR_P, CHAR_S,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE};

/* Player 1 attack */
char Message3[] =
    {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_Y, CHAR_E, CHAR_R, CHAR_1, SPACE,
     CHAR_A, CHAR_T, CHAR_T, CHAR_A, CHAR_C, CHAR_K, SPACE,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE};

/* Player 2 attack */
char Message4[] =
    {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_E, CHAR_R, CHAR_2, SPACE,
     CHAR_A, CHAR_T, CHAR_T, CHAR_A, CHAR_C, CHAR_K,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE};


/* Player 1 Wins */
char Message5[] =
    {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_Y, CHAR_E, CHAR_R, CHAR_1, SPACE,
     CHAR_W, CHAR_I, CHAR_N, CHAR_S,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE};

/* Player 2 Wins */
char Message6[] =
    {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
     CHAR_P, CHAR_L, CHAR_A, CHAR_Y, CHAR_E, CHAR_R, CHAR_2, SPACE,
     CHAR_W, CHAR_I, CHAR_N, CHAR_S,
     SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE};



void Start_Message(char *msg, int length) // initializes the marquee scroll system disables cursors)
{
  Cursor_On = 0; // cursor off during message scroll
  Animate_On = 1; // signals the systic to start scrolling
  Message_Pointer = msg;// start reading at begining of message
  Save_Pointer = msg;// remember the start of message for reset
  Message_Length = length;// total characters in message
  Delay_msec = 200; // scroll one position or char every 200ms
}

Music Song[100];





// check win to determine if a player has sunk all opponent ships
int check_win(map_t hit_map, map_t boat_map)
{
    for (int i = 0; i < 8; i++) {

    	// checks all 7 segment types across all 8 digit positions to see if boats exist
    	// at position but if no hit recorded there game continues
        if (boat_map.horizontal[0][i] && !hit_map.horizontal[0][i]) return 0;
        if (boat_map.horizontal[1][i] && !hit_map.horizontal[1][i]) return 0;
        if (boat_map.horizontal[2][i] && !hit_map.horizontal[2][i]) return 0;
        if (boat_map.vertical[0][i]   && !hit_map.vertical[0][i])   return 0;
        if (boat_map.vertical[1][i]   && !hit_map.vertical[1][i])   return 0;
        if (boat_map.vertical[0][i+8] && !hit_map.vertical[0][i+8]) return 0;
        if (boat_map.vertical[1][i+8] && !hit_map.vertical[1][i+8]) return 0;
    }
    return 1; // if every boat segment hit theres a winner
}

// boat placement counter track how many ships each player has placed
// doubles placed first (max 2 per player = 4 segments total)
// singles placed (3 max per player = 3 segments total)

int P1_singles = 0;
int P1_doubles = 0;
int P2_singles = 0;
int P2_doubles = 0;


// Horizontal/vertical segment sets
static char seg_horizontal[] = { (1<<0), (1<<6), (1<<3) };          // top, mid, bottom
static char seg_vertical[]   = { (1<<5), (1<<4), (1<<1), (1<<2) };  // Upper L, Lower L, Upper R, LowerR



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

  /* Set all GPIOE pins high - deselects all 7-segment digits
         Common cathode display: high on select pin = digit OFF */
  GPIOE->ODR    = 0xFFFF;


  	  	 /* Override I2S3 alternate function on PC10 and PC11
         MX_GPIO_Init assigns these to I2S3 clock/data which conflicts
         with SW6/SW7 pushbuttons - force them back to plain GPIO input
         Bits 21:20 = PC10, bits 23:22 = PC11, writing 00 = input mode */
  GPIOC->MODER &= ~((3 << 20) | (3 << 22));

  GPIOC->MODER &= ~(3 << 16); // force PC8 to input mode for SW1 slide


  RCC->APB2ENR |= 1 << 8; // enable ADC1 clock on APB2 bus
  ADC1->SMPR2  |= 1;// set ADC1 sample time register - bit 0 sets channel 1 sample time
  ADC1->CR2    |= 1; // Enable ADC1 by setting ADON but in control register 2

  RCC->AHB1ENR |= 1 << 12; // Enable CRC peripheral clock on AHB1 bus (12 bit)

  TIM7->PSC   = 199; // Prescaler 84mMHz / 200 = 420khz timer clock
  TIM7->ARR   = 1;// reload interupt every 2 ticks
  TIM7->DIER |= 1;// enable TIM7 update interupt
  TIM7->CR1  |= 1;// start TIM7


// game state machine
  int game = 0;// controls which case executes
  int i;
  int phase = 0;// (sub-state) used in case 2 to run one time setup only once





  while (1)
  {
    switch (game)
    {

    case 0: // title screen — scroll "BATTLE SHIP PLAYER1 PLACE SHIPS"
    {
        Start_Message(Message1, sizeof(Message1) / sizeof(Message1[0]));

        HAL_Delay(10000);   // hold in case 0 for 10 seconds for message to scroll SysTick scrolls message in backround

        Animate_On = 0;     // stop scrolling
        HAL_Delay(1000);    // brief pause before clearing

        // clearing all 8 7-segment displays
        int i;
        for (i = 0; i < 8; i++)
            Seven_Segment_Digit(i, SPACE, 0);
        HAL_Delay(500);     // brief pause before entering case 1

        game = 1; // go to player 1 ship placement
        break;
    }


    case 1:
            {
                Display_Mode = 0;   // show Player 1 own boat map
                Animate_On   = 0;   // no scrolling during placement
                Cursor_On    = 1;   // enable blinking cursor from the SysTick
                Delay_msec   = 50;  // SysTick display refresh rate


                int seg_mode = ((GPIOC->IDR >> 0) & 1); // SW1 switch where 0 = horizontal segment mode (top/mid/bot)
                //1 = vertical segment mode (upper/lower left/right) */


                ADC1->SQR3 = 1;              // select ADC channel 1 potentiometer 1
                ADC1->CR2 |= (1 << 30);      // Analog to digital converter
                int timeout = 10000; // wait for conversion to finish
                while (!(ADC1->SR & 2) && timeout-- > 0);  // wait for end of conversion
                if (P1_doubles < 2)
                    Cursor_Digit = (ADC1->DR * 8) / 4096;  // 0-6 range for doubles
                else
                    Cursor_Digit = (ADC1->DR * 8) / 4096;  // 0-7 range for singles


                ADC1->SQR3 = 2;              // select ADC channel 2 potentiometer 2
                ADC1->CR2 |= (1 << 30);      // trigger ADC
                timeout = 10000;// wait for conversion to finish
                while (!(ADC1->SR & 2) && timeout-- > 0);  // wait for end of conversion
                int pair = (ADC1->DR * 2) / 4096;  // maps 0-4095 to 0 or 1

                if (P1_doubles < 2)
                {
//                     Double boat mode: two fixed vertical pairs
//                       pair 0 = f and e segments
//                       pair 1 = b and c segments
//                       Both segments blink together so player can see the complete boat
                    if (pair == 0) { Cursor_Segment = (1<<5); Cursor_Segment2 = (1<<4); }
                    else           { Cursor_Segment = (1<<1); Cursor_Segment2 = (1<<2); }
                }
                else
                {
//                     Single boat mode which PA2 selects one segment
//                       SW1 determines which lookup array to use
                    if (seg_mode == 0)
                        Cursor_Segment = seg_horizontal[(ADC1->DR * 3) / 4096];
                    else
                        Cursor_Segment = seg_vertical[(ADC1->DR * 4) / 4096];
                    Cursor_Segment2 = 0;  // single boats have no second segment
                }

//                 Read SW6/PC10: active-low with pull-up
//                   Pin normally HIGH, goes LOW when button pressed
//
                int PC10 = !((GPIOC->IDR >> 10) & 1);
                // Read SW7/PC11 same active-low configuration
                int PC11 = !((GPIOC->IDR >> 11) & 1);

                if (PC10 == 1)  // when SW6 pressed place a boat
                {
                    int d   = Cursor_Digit;    // capture and save current digit position
                    char s  = Cursor_Segment;  // capture and save primary segment
                    char s2 = Cursor_Segment2; // capture and save second segment (doubles only)

                    if (P1_doubles < 2)
                    {
                        //Place double boat: write 1 to both segment map positions
                         //  Each if-else maps the segment bitmask to the correct
                          // horizontal or vertical array position
                        if      (s  == (1<<0)) Player_Map.horizontal[0][d] = 1; // top
                        else if (s  == (1<<6)) Player_Map.horizontal[1][d] = 1; // mid
                        else if (s  == (1<<3)) Player_Map.horizontal[2][d] = 1; // bot
                        else if (s  == (1<<5)) Player_Map.vertical[0][d]   = 1; // UL
                        else if (s  == (1<<4)) Player_Map.vertical[1][d]   = 1; // LL
                        else if (s  == (1<<1)) Player_Map.vertical[0][d+8] = 1; // UR
                        else if (s  == (1<<2)) Player_Map.vertical[1][d+8] = 1; // LR

                        // Place second segment of the double boat on same digit
                        if      (s2 == (1<<0)) Player_Map.horizontal[0][d] = 1;
                        else if (s2 == (1<<6)) Player_Map.horizontal[1][d] = 1;
                        else if (s2 == (1<<3)) Player_Map.horizontal[2][d] = 1;
                        else if (s2 == (1<<5)) Player_Map.vertical[0][d]   = 1;
                        else if (s2 == (1<<4)) Player_Map.vertical[1][d]   = 1;
                        else if (s2 == (1<<1)) Player_Map.vertical[0][d+8] = 1;
                        else if (s2 == (1<<2)) Player_Map.vertical[1][d+8] = 1;

                        P1_doubles++;        // count double boat as placed
                        Cursor_Segment2 = 0; // clear preview after placement
                        Layered_Display();   // immediately refresh display
                    }
                    else if (P1_singles < 3) // 3 single boats (check if target position already has a boat)
                    {

                        int occupied = 0;
                        if      (s == (1<<0)) occupied = Player_Map.horizontal[0][d];
                        else if (s == (1<<6)) occupied = Player_Map.horizontal[1][d];
                        else if (s == (1<<3)) occupied = Player_Map.horizontal[2][d];
                        else if (s == (1<<5)) occupied = Player_Map.vertical[0][d];
                        else if (s == (1<<4)) occupied = Player_Map.vertical[1][d];
                        else if (s == (1<<1)) occupied = Player_Map.vertical[0][d+8];
                        else if (s == (1<<2)) occupied = Player_Map.vertical[1][d+8];

                        if (!occupied)  // only place if position is empty on map
                        {
                            if      (s == (1<<0)) Player_Map.horizontal[0][d] = 1;
                            else if (s == (1<<6)) Player_Map.horizontal[1][d] = 1;
                            else if (s == (1<<3)) Player_Map.horizontal[2][d] = 1;
                            else if (s == (1<<5)) Player_Map.vertical[0][d]   = 1;
                            else if (s == (1<<4)) Player_Map.vertical[1][d]   = 1;
                            else if (s == (1<<1)) Player_Map.vertical[0][d+8] = 1;
                            else if (s == (1<<2)) Player_Map.vertical[1][d+8] = 1;
                            P1_singles++;      // count single boat as placed
                            Layered_Display(); // refresh display
                        }
                    }

                    HAL_Delay(250);  // ignore button for 250ms after press to give time to process
                }

                if (PC11 == 1)  // SW7/PC11 pressed to advance to P2 placement
                {
                    // Only advance when all boats are placed:
                    // 7 total segments
                    if (P1_doubles >= 2 && P1_singles >= 3)
                    {
                        Cursor_On       = 0;  // hide cursor during transition
                        Cursor_Segment2 = 0;  // clear double boat preview
                        game = 2;             // go to P2 ship placement
                    }
                    HAL_Delay(250);  // helps with multiple button inputs (once one button input detected wait 250ms)
                }

                break;
            }







    case 2: // P2 placing ships doubles first then singles
    {
    	if (phase == 0)
    	{
    	 Display_Mode    = 1;    // switch display to Player 2 map
    	 Start_Message(Message2, sizeof(Message2) / sizeof(Message2[0]));
    	 HAL_Delay(6000);        // scroll message for 6 seconds
    	 Animate_On      = 0;    // stop scrolling
    	 Cursor_On       = 1;    // enable cursor for P2 placement
    	 Cursor_Segment2 = 0;    // clear any leftover double preview
    	 Delay_msec      = 50;   // display refresh rate
    	 phase = 1;              // flag to skip this block next time

    	}


        int seg_mode = ((GPIOC->IDR >> 0) & 1);

        // PA1 -> digit
        ADC1->SQR3 = 1;
        ADC1->CR2 |= (1 << 30);
        int timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);

        if (P2_doubles < 2)
            Cursor_Digit = (ADC1->DR * 8) / 4096;
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
            // single mode full segment selection
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
                // doubles first both segments on same digit
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
                // Singles now, check overlap
                int occupied = 0;
                if      (s == (1<<0)) occupied = Player2_Map.horizontal[0][d];
                else if (s == (1<<6)) occupied = Player2_Map.horizontal[1][d];
                else if (s == (1<<3)) occupied = Player2_Map.horizontal[2][d];
                else if (s == (1<<5)) occupied = Player2_Map.vertical[0][d];
                else if (s == (1<<4)) occupied = Player2_Map.vertical[1][d];
                else if (s == (1<<1)) occupied = Player2_Map.vertical[0][d+8];
                else if (s == (1<<2)) occupied = Player2_Map.vertical[1][d+8];

                // check overlap
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
    	Display_Mode    = 2;   // P1 shots P1_Hits shown on P2 map
    	Start_Message(Message3, sizeof(Message3) / sizeof(Message3[0]));
    	HAL_Delay(6000);       // show message for 6 seconds
    	Animate_On      = 0;   // stop scrolling
    	Cursor_On       = 1;   // enable cursor for targeting
    	Cursor_Digit    = 0;   // reset cursor to first digit
		Cursor_Segment  = (1<<0);  // start at top segment
		Cursor_Segment2 = 0;   // no double segment in attack mode
		Delay_msec      = 50;
		game = 4;  // go to P1 shooting
		break;
    }

    case 4: // P1 shooting at P2 map
    {
        Display_Mode = 2;
        Cursor_On    = 1;
        Animate_On   = 0;
        Delay_msec   = 50;

        // SW1 (PC0)
        //0 = horizontal, 1 = vertical
        int seg_mode = ((GPIOC->IDR >> 0) & 1);

        // PA1 to digit
        ADC1->SQR3 = 1;
        ADC1->CR2 |= (1 << 30);
        int timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);
        Cursor_Digit = (ADC1->DR * 8) / 4096;

        // PA2  segment based on SW1
        ADC1->SQR3 = 2;
        ADC1->CR2 |= (1 << 30);
        timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);

        if (seg_mode == 0)
            Cursor_Segment = seg_horizontal[(ADC1->DR * 3) / 4096];
        else
            Cursor_Segment = seg_vertical[(ADC1->DR * 4) / 4096];

        Cursor_Segment2 = 0; // single cursor during attack

        // Read buttons
        int PC10 = !((GPIOC->IDR >> 10) & 1);  // SW6 = fire shot

        if (PC10 == 1)
        {
            int d  = Cursor_Digit;
            char s = Cursor_Segment;

            // record shot in P1_Hits map Add_Shots_To_Display will compare this against Player2_Map to show hit (bright) miss(dim)
            if      (s == (1<<0)) P1_Hits.horizontal[0][d] = 1;
            else if (s == (1<<6)) P1_Hits.horizontal[1][d] = 1;
            else if (s == (1<<3)) P1_Hits.horizontal[2][d] = 1;
            else if (s == (1<<5)) P1_Hits.vertical[0][d]   = 1;
            else if (s == (1<<4)) P1_Hits.vertical[1][d]   = 1;
            else if (s == (1<<1)) P1_Hits.vertical[0][d+8] = 1;
            else if (s == (1<<2)) P1_Hits.vertical[1][d+8] = 1;

            Layered_Display(); // show shot result
            HAL_Delay(500);  // pause so player can see result

            // check if P1 wins
            if (check_win(P1_Hits, Player2_Map))
            {
                game = 7;  // P1 wins
            }
            else
            {
                game = 5;  // P2 turn
            }
        }

        break;
    }

    case 5: // P2 attack message
    {
        Display_Mode = 3;  // show P1 board with P2 shots
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

    case 6: // P2 shooting at P1 map
    {
        Display_Mode = 3;
        Cursor_On    = 1;
        Animate_On   = 0;
        Delay_msec   = 50;

        // SW1 PC0
        //0 = horizontal, 1 = vertical
        int seg_mode = ((GPIOC->IDR >> 0) & 1);

        // PA1 to target digit
        ADC1->SQR3 = 1;
        ADC1->CR2 |= (1 << 30);
        int timeout = 10000;
        while (!(ADC1->SR & 2) && timeout-- > 0);
        Cursor_Digit = (ADC1->DR * 8) / 4096;

        // PA2 target segment based on SW1
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
        Cursor_On  = 0; // hide cursor on win screen
        Cursor_Segment2 = 0;

        // scroll "PLAYER1 WINS" or "PLAYER2 WINS" based on who won
        if (check_win(P1_Hits, Player2_Map))
            Start_Message(Message5, sizeof(Message5) / sizeof(Message5[0]));
        else
            Start_Message(Message6, sizeof(Message6) / sizeof(Message6[0]));

        // stay here forever
        while (1) {}
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
  // Configure SW6 PC11 and SW7 PC11 as inputs with pull-up resistors
  //Pull-up holds (pin HIGH at rest)

  GPIO_InitStruct.Pin  = GPIO_PIN_10 | GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);



  GPIO_InitStruct.Pin  = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

  // Initialize marquee message scroll on startup
  Message_Pointer = &Message1[0]; // start at beginning of title message
  Save_Pointer    = &Message1[0]; // save start for reset or when it wraps around
  Message_Length  = sizeof(Message1) / sizeof(Message1[0]);
  Delay_msec      = 200; // scroll speed
  Animate_On      = 1; // tell SysTick to start scrolling

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
