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
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

COUNT++;  // Increment note duration counter
Vibrato_Count++; // Increment the note vibrato effect counter

/* This code applies vibrato to the current note that is playing  */
if (Vibrato_Count >= Vibrato_Rate)
{
	Vibrato_Count = 0;
	if (Song[INDEX].note > 0)
		{
			Song[INDEX].note += Vibrato_Depth;
			if (Song[INDEX].note > (Save_Note + Vibrato_Depth)) Song[INDEX].note = Save_Note - Vibrato_Depth;

		}
}

typedef struct map_s {
	int horizontal[3][8];
	int vertical[2][16];
};

void clear_map(Map *m)
{
    int r, c;

    for (r = 0; r < 3; r++)
    {
        for (c = 0; c < 8; c++)
        {
            m->horizontal[r][c] = 0;
        }
    }

    for (r = 0; r < 2; r++)
    {
        for (c = 0; c < 16; c++)
        {
            m->vertical[r][c] = 0;
        }
    }
}

void build_pot_map(Map *m)
{
    uint16_t adc_h;
    uint16_t adc_v;
    int h_index;
    int v_index;
    int h_row, h_col;
    int v_row, v_col;

    clear_map(m);

    adc_h = read_adc(0);   // PA0 = horizontal selector
    adc_v = read_adc(1);   // PA1 = vertical selector

    // horizontal has 24 total positions: 3 rows x 8 cols
    h_index = (adc_h * 24) / 4096;
    if (h_index > 23) h_index = 23;

    h_row = h_index / 8;
    h_col = h_index % 8;

    m->horizontal[h_row][h_col] = 1;

    // vertical has 32 total positions: 2 rows x 16 cols
    v_index = (adc_v * 32) / 4096;
    if (v_index > 31) v_index = 31;

    v_row = v_index / 16;
    v_col = v_index % 16;

    m->vertical[v_row][v_col] = 1;
}

void map_to_digits(Map *m, unsigned char digits[8])
{
    int i;

    for (i = 0; i < 8; i++)
    {
        unsigned char seg = 0;

        // horizontal segments
        if (m->horizontal[0][i]) seg |= 0x01;   // top
        if (m->horizontal[1][i]) seg |= 0x40;   // middle
        if (m->horizontal[2][i]) seg |= 0x08;   // bottom

        // vertical segments
        if (m->vertical[0][2*i])     seg |= 0x20; // upper left
        if (m->vertical[0][2*i + 1]) seg |= 0x02; // upper right
        if (m->vertical[1][2*i])     seg |= 0x10; // lower left
        if (m->vertical[1][2*i + 1]) seg |= 0x04; // lower right

        digits[i] = seg;
    }
}

void draw_board(Map *m)
{
    int i;

    map_to_digits(m, display_digits);

    for (i = 0; i < 8; i++)
    {
        Seven_Segment_Digit(i, display_digits[i], 0);
    }
}



if (Animate_On > 0)
{
	Delay_counter++;
	if (Delay_counter > Delay_msec)
	{
		Delay_counter = 0;
		Seven_Segment_Digit(7,*(Message_Pointer),0);
		Seven_Segment_Digit(6,*(Message_Pointer+1),0);
		Seven_Segment_Digit(5,*(Message_Pointer+2),0);
		Seven_Segment_Digit(4,*(Message_Pointer+3),0);
		Seven_Segment_Digit(3,*(Message_Pointer+4),0);
		Seven_Segment_Digit(2,*(Message_Pointer+5),0);
		Seven_Segment_Digit(1,*(Message_Pointer+6),0);
		Seven_Segment_Digit(0,*(Message_Pointer+7),0);
		Message_Pointer++;
		if ((Message_Pointer - Save_Pointer) >= (Message_Length-8)) Message_Pointer = Save_Pointer;

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
	if ((Music_ON > 0) && (Song[INDEX].note > 0) && ((Song[INDEX].tempo/Song[INDEX].size - Song[INDEX].space) > COUNT))
	{

		if (Song[INDEX].note <= TONE)
		{
			GPIOD->ODR ^= 1;
			TONE = 0;
		}
	}
	else if ((Music_ON > 0) && Song[INDEX].tempo/Song[INDEX].size > COUNT)
	{
		TONE = 0;
	}
	else if ((Music_ON > 0) && Song[INDEX].tempo/Song[INDEX].size == COUNT)
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
