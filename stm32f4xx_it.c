/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 */




/* USER CODE END Header */

#include "main.h"
#include "stm32f4xx_it.h"
#include "seg7.h"



/* USER CODE BEGIN PV */
map_t Player_Map  = {0};
map_t Player2_Map = {0};
map_t P1_Hits     = {0};
map_t P2_Hits     = {0};

int  Display_Mode    = 0;
int  Game_Stage_Mode = 0;

// FIX 1: Define Cursor variables here
char Cursor_On      = 0;
int  Cursor_Digit   = 0;
char Cursor_Segment = 0x01;
char Cursor_Visible     = 0;
int  Cursor_Blink_Count = 0;

// FIX 2: Define Game_Display here
char Game_Display[8] = {0,0,0,0,0,0,0,0};
static int display_digit = 0;
/* USER CODE END PV */




extern HCD_HandleTypeDef hhcd_USB_OTG_FS;
extern TIM_HandleTypeDef htim7;

void NMI_Handler(void)        { while (1) {} }
void HardFault_Handler(void)  { while (1) {} }
void MemManage_Handler(void)  { while (1) {} }
void BusFault_Handler(void)   { while (1) {} }
void UsageFault_Handler(void) { while (1) {} }
void SVC_Handler(void)        {}
void DebugMon_Handler(void)   {}
void PendSV_Handler(void)     {}

#define SEG_OFF  0
#define SEG_DIM  1
#define SEG_HIT  2
#define SEG_BOAT 3


void Add_Boats_To_Display(map_t boats)
{
    for (int i = 0; i < 8; i++)
    {
        if (boats.horizontal[0][i])     Game_Display[i] |= (1<<0);
        if (boats.horizontal[1][i])     Game_Display[i] |= (1<<6);
        if (boats.horizontal[2][i])     Game_Display[i] |= (1<<3);

        if (boats.vertical[0][i])       Game_Display[i] |= (1<<5);
        if (boats.vertical[1][i])       Game_Display[i] |= (1<<4);
        if (boats.vertical[0][i + 8])   Game_Display[i] |= (1<<1);
        if (boats.vertical[1][i + 8])   Game_Display[i] |= (1<<2);
    }
}

void Add_Shots_To_Display(map_t shots, map_t boats)
{
    for (int i = 0; i < 8; i++)
    {
        if (shots.horizontal[0][i])
        {
            if (boats.horizontal[0][i] || ramp < 127)
                Game_Display[i] |= (1<<0);
        }

        if (shots.horizontal[1][i])
        {
            if (boats.horizontal[1][i] || ramp < 127)
                Game_Display[i] |= (1<<6);
        }

        if (shots.horizontal[2][i])
        {
            if (boats.horizontal[2][i] || ramp < 127)
                Game_Display[i] |= (1<<3);
        }

        if (shots.vertical[0][i])
        {
            if (boats.vertical[0][i] || ramp < 127)
                Game_Display[i] |= (1<<5);
        }

        if (shots.vertical[1][i])
        {
            if (boats.vertical[1][i] || ramp < 127)
                Game_Display[i] |= (1<<4);
        }

        if (shots.vertical[0][i + 8])
        {
            if (boats.vertical[0][i + 8] || ramp < 127)
                Game_Display[i] |= (1<<1);
        }

        if (shots.vertical[1][i + 8])
        {
            if (boats.vertical[1][i + 8] || ramp < 127)
                Game_Display[i] |= (1<<2);
        }
    }
}




void Layered_Display(void)
{
    for (int i = 0; i < 8; i++)
        Game_Display[i] = 0;

    if (Display_Mode == 0)
    {
        Add_Boats_To_Display(Player_Map);
    }
    else if (Display_Mode == 1)
    {
        Add_Boats_To_Display(Player2_Map);
    }
    else if (Display_Mode == 2)
    {
        Add_Shots_To_Display(P1_Hits, Player2_Map);
    }
    else if (Display_Mode == 3)
    {
        Add_Shots_To_Display(P2_Hits, Player_Map);
    }

    if (Cursor_On && Cursor_Visible)
        Game_Display[Cursor_Digit] |= Cursor_Segment;

    GPIOE->ODR = 0xFF00 | ((unsigned char)Game_Display[display_digit]);
    GPIOE->ODR &= ~(1 << (display_digit + 8));

    display_digit++;
    if (display_digit >= 8)
        display_digit = 0;
}






void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  COUNT++;
  Vibrato_Count++;

  if (Vibrato_Count >= Vibrato_Rate)
  {
    Vibrato_Count = 0;
    if (Song[INDEX].note > 0)
    {
      Song[INDEX].note += Vibrato_Depth;
      if (Song[INDEX].note > (Save_Note + Vibrato_Depth))
        Song[INDEX].note = Save_Note - Vibrato_Depth;
    }
  }

  // Cursor display and blink
  if (Cursor_On > 0)
  {
      Cursor_Blink_Count++;
      if (Cursor_Blink_Count >= 500)
      {
          Cursor_Blink_Count = 0;
          Cursor_Visible ^= 1;
      }

      Layered_Display();
  }
  else if (Animate_On > 0)
  {
      Delay_counter++;
      if (Delay_counter > Delay_msec)
      {
          Delay_counter = 0;

          Seven_Segment_Digit(7, *(Message_Pointer),     0);
          Seven_Segment_Digit(6, *(Message_Pointer + 1), 0);
          Seven_Segment_Digit(5, *(Message_Pointer + 2), 0);
          Seven_Segment_Digit(4, *(Message_Pointer + 3), 0);
          Seven_Segment_Digit(3, *(Message_Pointer + 4), 0);
          Seven_Segment_Digit(2, *(Message_Pointer + 5), 0);
          Seven_Segment_Digit(1, *(Message_Pointer + 6), 0);
          Seven_Segment_Digit(0, *(Message_Pointer + 7), 0);

          Message_Pointer++;

          if ((Message_Pointer - Save_Pointer) >= (Message_Length - 8))
              Message_Pointer = Save_Pointer;
      }
  }



  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
}

void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */

  TONE++;
  ramp++;

  if ((Music_ON > 0) && (Song[INDEX].note > 0) && ((Song[INDEX].tempo / Song[INDEX].size - Song[INDEX].space) > COUNT))
  {
    if (Song[INDEX].note <= TONE)
    {
      GPIOD->ODR ^= 1;
      TONE = 0;
    }
  }
  else if ((Music_ON > 0) && Song[INDEX].tempo / Song[INDEX].size > COUNT)
  {
    TONE = 0;
  }
  else if ((Music_ON > 0) && Song[INDEX].tempo / Song[INDEX].size == COUNT)
  {
    COUNT = 0;
    TONE = 0;
    if (!(Song[INDEX].end))
    {
      INDEX++;
      Save_Note = Song[INDEX].note;
    }
  }
  else if (Music_ON == 0)
  {
    TONE = 0;
    COUNT = 0;
  }

  if (DIM_Enable > 0)
  {
    if (RED_BRT <= ramp)   GPIOD->ODR |= (1 << 15);
    else                   GPIOD->ODR &= ~(1 << 15);
    if (BLUE_BRT <= ramp)  GPIOD->ODR |= (1 << 14);
    else                   GPIOD->ODR &= ~(1 << 14);
    if (GREEN_BRT <= ramp) GPIOD->ODR |= (1 << 13);
    else                   GPIOD->ODR &= ~(1 << 13);
  }

  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
}

void OTG_FS_IRQHandler(void)
{
  HAL_HCD_IRQHandler(&hhcd_USB_OTG_FS);
}
