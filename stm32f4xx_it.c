/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
#include "seg7.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

map_t Player_Map  = {0};
map_t Player2_Map = {0};
map_t P1_Hits     = {0};
map_t P2_Hits     = {0};


// Pointer to whichever map SysTick should draw

// main.c swaps this pointer each game phase
map_t *Boat_Map = &Player_Map;
map_t *Hit_Map = &P1_Hits;

char Cursor_Visible    = 0;
int  Cursor_Blink_Count = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern HCD_HandleTypeDef hhcd_USB_OTG_FS;
extern TIM_HandleTypeDef htim7;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
 * @brief This function handles System tick timer.
 */


// Segment value constants map int values to display
#define SEG_OFF   0   // empty
#define SEG_DIM   1   // miss shown at 50% brightness via PWM
#define SEG_HIT   2   // hit shown at 100% brightness
#define SEG_BOAT  3   // placed boat in placement phase only


// PWM brightness threshold ramp counts 0-255 in TIM7
// SEG_DIM turns off at halfway through the ramp cycle

#define DIM_THRESHOLD 127

// layering boats, hits, and cursor
void Composite_Display(void)
{
    for (int i = 0; i < 8; i++) Game_Display[i] = 0;

    for (int i = 0; i < 8; i++)
    {
    						// Horizontal segments
        // Top bit 0
        {
            int boat = Boat_Map->horizontal[0][i]; // Save value at current i position for the top row of placed boats
            int hit  = Hit_Map->horizontal[0][i]; // Save value at current i position for the top row on the hit map
            // If boat is not null, save the value for blinking the light
            // Else, if hit is not null, save the value for a miss
            // else, save the value for keeping the lights off
            int val  = boat ? SEG_BOAT : (hit ? SEG_DIM : SEG_OFF); 
            if (boat && hit) val = SEG_HIT; // if boat is not null and hit is not null, save the value for a hit
            if (val == SEG_BOAT || val == SEG_HIT)
                Game_Display[i] |= (1 << 0); // if value is for placing boats or a hit, turn on the top seg of the 7 seg
            else if (val == SEG_DIM && ramp < DIM_THRESHOLD)
                Game_Display[i] |= (1 << 0); // if the value is for a miss and ramp is less than the dim threshold, turn on the top segment of 7 seg
        }
        // Middle bit 6
        {
            int boat = Boat_Map->horizontal[1][i];
            int hit  = Hit_Map->horizontal[1][i];
            int val  = boat ? SEG_BOAT : (hit ? SEG_DIM : SEG_OFF);
            if (boat && hit) val = SEG_HIT;
            if (val == SEG_BOAT || val == SEG_HIT)
                Game_Display[i] |= (1 << 6);
            else if (val == SEG_DIM && ramp < DIM_THRESHOLD)
                Game_Display[i] |= (1 << 6);
        }
        // Bottom (bit 3)
        {
            int boat = Boat_Map->horizontal[2][i];
            int hit  = Hit_Map->horizontal[2][i];
            int val  = boat ? SEG_BOAT : (hit ? SEG_DIM : SEG_OFF);
            if (boat && hit) val = SEG_HIT;
            if (val == SEG_BOAT || val == SEG_HIT)
                Game_Display[i] |= (1 << 3);
            else if (val == SEG_DIM && ramp < DIM_THRESHOLD)
                Game_Display[i] |= (1 << 3);
        }

        					// Vertical segments

        // Upper-left bit 5
        {
            int boat = Boat_Map->vertical[0][i];
            int hit  = Hit_Map->vertical[0][i];
            if (boat || (hit == SEG_HIT))      Game_Display[i] |= (1 << 5);
            else if (hit && ramp < DIM_THRESHOLD) Game_Display[i] |= (1 << 5);
        }
        // Lower-left bit 4
        {
            int boat = Boat_Map->vertical[1][i];
            int hit  = Hit_Map->vertical[1][i];
            if (boat || (hit == SEG_HIT))         Game_Display[i] |= (1 << 4);
            else if (hit && ramp < DIM_THRESHOLD)  Game_Display[i] |= (1 << 4);
        }
        // Upper-right bit 1
        {
            int boat = Boat_Map->vertical[0][i + 8];
            int hit  = Hit_Map->vertical[0][i + 8];
            if (boat || (hit == SEG_HIT))         Game_Display[i] |= (1 << 1);
            else if (hit && ramp < DIM_THRESHOLD)  Game_Display[i] |= (1 << 1);
        }
        // Lower-right bit 2
        {
            int boat = Boat_Map->vertical[1][i + 8];
            int hit  = Hit_Map->vertical[1][i + 8];
            if (boat || (hit == SEG_HIT))         Game_Display[i] |= (1 << 2);
            else if (hit && ramp < DIM_THRESHOLD)  Game_Display[i] |= (1 << 2);
        }
    }






    // Map Layer 3: cursor blink on top of everything
    if (Cursor_On && Cursor_Visible)
        Game_Display[Cursor_Digit] |= Cursor_Segment;

    //  Write bitmasks to display on "map"
    for (int i = 0; i < 8; i++)
    {
        GPIOE->ODR = (0xFF00 | (unsigned char)Game_Display[i]) & ~(1 << (i + 8));
        GPIOE->ODR |= 0xFF00;
    }
}






void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  COUNT++;         // Increment note duration counter
  Vibrato_Count++; // Increment the note vibrato effect counter

  /* This code applies vibrato to the current note that is playing  */
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





  // when cursor is on blink
  if (Cursor_On > 0)
  {
      // Blink toggle
      Cursor_Blink_Count++;
      if (Cursor_Blink_Count >= 500)
      {
          Cursor_Blink_Count = 0;
          Cursor_Visible ^= 1;
      }


      Delay_counter++;
      if (Delay_counter > Delay_msec)
      {
          Delay_counter = 0;
          Composite_Display();
      }
  }
  else if (Animate_On > 0)
  {
      // scrolling message — unchanged
      Delay_counter++;
      if (Delay_counter > Delay_msec)
      {
          Delay_counter = 0;
          Seven_Segment_Digit(7, *(Message_Pointer),0);
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
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */


}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles TIM7 global interrupt.
 */
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */

  /* Increment TONE counter and dimming ramp counter */
  TONE++;
  ramp++;

  /* This code plays the song from the song array structure */
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

  /* This code dims the RGB LEDs using PWM */
  if (DIM_Enable > 0)
  {
    if (RED_BRT <= ramp)
    {
      GPIOD->ODR |= (1 << 15);
    }
    else
    {
      GPIOD->ODR &= ~(1 << 15);
    }
    if (BLUE_BRT <= ramp)
    {
      GPIOD->ODR |= (1 << 14);
    }
    else
    {
      GPIOD->ODR &= ~(1 << 14);
    }
    if (GREEN_BRT <= ramp)
    {
      GPIOD->ODR |= (1 << 13);
    }
    else
    {
      GPIOD->ODR &= ~(1 << 13);
    }
  }
  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
  /* USER CODE BEGIN TIM7_IRQn 1 */

  /* USER CODE END TIM7_IRQn 1 */
}

/**
 * @brief This function handles USB On The Go FS global interrupt.
 */
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_HCD_IRQHandler(&hhcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
