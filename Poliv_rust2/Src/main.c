
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "wh1602.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;

osThreadId defaultTaskHandle;
osThreadId alarm_controlHandle;
osThreadId reservHandle;


//------------------global------------------------//
RTC_TimeTypeDef TimeNOW;
RTC_TimeTypeDef TimeDuriation[3];
RTC_TimeTypeDef TimeRepeat[3],TimeEND[3],TimeON[3];
uint8_t chanelON[3]={0,0,0};
uint8_t DayRepeat[3],dayRepeatOld[3],startuem[3];
RTC_DateTypeDef DateNOW;
uint8_t update_screen=1, blynk=0,screen=1,button,dayWork=0,kanal=1,encValOld,n,flash_flag=0, t1,t2;
uint32_t set[6]={0,0,0,0,0,0};
uint16_t EncoderVal;
char R[16]="Ch323";
FLASH_EraseInitTypeDef Erase;
//------------------------------------------------//

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_IWDG_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_IWDG_Init();
  MX_RTC_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_ALL);
  EncoderVal=__HAL_TIM_GET_COUNTER(&htim3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_ALL);
  TIM2->CCR1=1000;
  TIM2->CCR2=1000;
  //-------read from flash--------  


  set[0]=*(__IO uint32_t*)User_Page_Adress;    //4 adresa
  set[1]=*(__IO uint32_t*)User_Page_Adress4;    //4 adresa
  set[2]=*(__IO uint32_t*)User_Page_Adress8;    //4 adresa
  set[3]=*(__IO uint32_t*)User_Page_Adress12;    //4 adresa
  set[4]=*(__IO uint32_t*)User_Page_Adress16;    //4 adresa
  set[5]=*(__IO uint32_t*)User_Page_Adress20;    //4 adresa
  if (set[0]!=0xFFFFFFFF)
  {
    for (n=0;n<3;n++)
    {
      TimeDuriation[n].Seconds=set[n]>>8;
      TimeDuriation[n].Minutes=set[n]>>16;
      TimeDuriation[n].Hours=set[n]>>24;
      TimeRepeat[n].Hours=set[n+3]>>8;
      TimeRepeat[n].Minutes=set[n+3]>>16;
      DayRepeat[n]=set[n+3]>>24;

    }
    t1=set[0];
    t2=set[1];
  }
  /* USER CODE END 2 */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of alarm_control */
  osThreadDef(alarm_control, StartTask02, osPriorityNormal, 0, 48);
  alarm_controlHandle = osThreadCreate(osThread(alarm_control), NULL);


  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
 // RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* IWDG init function */
static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_128;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* RTC init function */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 719;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim2);

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_Encoder_InitTypeDef sConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 120;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 15;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 15;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_HalfDuplex_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA3 PA4 PA5 PA6 
                           PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB10 PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

//---------------------LP main tread---------------------//
void StartDefaultTask(void const * argument)
{
  InitializeLCD(); 
  osDelay(300);
  ClearLCDScreen(); 
  uint8_t choise=0;
  RTC_TimeTypeDef TimeSet;
  while(1)
  {  
    HAL_IWDG_Refresh(&hiwdg);
    osDelay(300);
    blynk=~blynk;
    HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
    
    //-------screen choise----------//
    switch(screen){
    case 0:
    if (update_screen==1)
    {
      ClearLCDScreen(); 
      Cursor(0,0);
      SendByte(0xA4,1); //C
      SendByte(0xBD,1); //i
      PrintStr("a");
      SendByte(0xC0,1); //?
      PrintStr("e");
      SendByte(0xBD,1); //i
      SendByte(0xB8,1); //e
      PrintStr("e-");
    }
    update_screen=0;
    Cursor(0,9);
    sprintf(R,"%03d",EncoderVal);
    PrintStr(R);
    Cursor(1,0);
    sprintf(R,"%02d%s%02d%s%02d",TimeNOW.Hours,":",TimeNOW.Minutes,":",TimeNOW.Seconds);
    PrintStr(R);
    
    break;
    case 1:
       if (update_screen==1)
    {
        ClearLCDScreen(); 
    }
    update_screen=0;
    Cursor(0,0);
    sprintf(R,"%03d",dayWork);
    PrintStr(R);    
    SendByte(0xE3,1); //a      
    SendByte(0xBD,1); //i
    Cursor(0,7);
    sprintf(R,"%02d%s%02d%s%02d",TimeNOW.Hours,":",TimeNOW.Minutes,":",TimeNOW.Seconds);
    PrintStr(R);    
    Cursor(1,0);
    sprintf(R,"%s%d%s%d%s%d","1K-",chanelON[0],", 2K-",chanelON[1],", 3K-",chanelON[2]);
    PrintStr(R);
    
    switch(EncoderVal%4)
      {
      case 0:
        Cursor(0,7);
        if(button==2)
        {
        button=0;
        update_screen=1;
        screen=2; 
        }
        if(button==1)
        {
        button=0;
        }
      break;
      case 1:
        Cursor(1,3);
        kanal=1;
        if(button==2)
        {
        button=0;
        update_screen=1;
        screen=3; 
        }
        if(button==1)
        {
        button=0;
        startuem[kanal-1]=1;
        }
      break;
      case 2:
        Cursor(1,9);
        kanal=2;
        if(button==2)
        {
        button=0;
        update_screen=1;
        
        screen=3; 
        }
        if(button==1)
        {
        button=0;
        startuem[kanal-1]=1;
        }
      break;
      case 3:
        Cursor(1,15);
        kanal=3;
        if(button==2)
        {
        button=0;
        update_screen=1;
        screen=3; 
        }
        if(button==1)
        {
        button=0;
        startuem[kanal-1]=1;
        }
      break;
      }
    
    
    break;
    case 2:
       if (update_screen==1)
    {
        ClearLCDScreen(); 
        Cursor(0,0);
        SendByte(0xA9,1); //O
        PrintStr("c");
        SendByte(0xBF,1); //o
        PrintStr(". ");
        SendByte(0xBF,1); //o
        PrintStr("e");
        SendByte(0xBA,1); //e
        PrintStr(". ");
        SendByte(0xB3,1); //a
        PrintStr("pe");
        SendByte(0xBC,1); //i
        PrintStr(".");
        //SendByte(0xC7,1); //y
        Cursor(1,0);
        SendByte(0xC0,1); //?
        PrintStr(":");
        SendByte(0xBC,1); //i
        PrintStr(":c");
        choise=0;
        TimeSet=TimeNOW;
        __HAL_TIM_SET_COUNTER(&htim3,TimeSet.Hours);
    }
        update_screen=0;
        Cursor(1,8);
        sprintf(R,"%02d%s%02d%s%02d",TimeSet.Hours,":",TimeSet.Minutes,":",TimeSet.Seconds);
        PrintStr(R);   
        if (button==1)
        {
          choise++;
          button=0;
          if (choise==0){__HAL_TIM_SET_COUNTER(&htim3,TimeSet.Hours);}
          if (choise==1){__HAL_TIM_SET_COUNTER(&htim3,TimeSet.Minutes);}
          if (choise==2){__HAL_TIM_SET_COUNTER(&htim3,TimeSet.Seconds);}
          if (choise==3){choise=0; __HAL_TIM_SET_COUNTER(&htim3,TimeSet.Hours);}
        }
        if (button==2)  //save current time
        {
          button=0;
          screen=1;
          update_screen=1;   
          if (HAL_RTC_SetTime(&hrtc, &TimeSet, RTC_FORMAT_BIN) != HAL_OK)
          {
            _Error_Handler(__FILE__, __LINE__);
          }
        }       
         switch(choise)
          {
          case 0:
            Cursor(1,8);  //setHour
            TimeSet.Hours=(EncoderVal%24);
          break;
          case 1:
            Cursor(1,11);  //setMin
            TimeSet.Minutes=(EncoderVal%60);
          break;
          case 2:
            Cursor(1,14);  //setSec
            TimeSet.Seconds=(EncoderVal%60);
          break;
          }
    
    break;
    case 3:
       if (update_screen==1)
    {
      ClearLCDScreen(); 
      Cursor(0,0);
      sprintf(R,"%s%d%s","K-",kanal," ");
      PrintStr(R);    
      SendByte(0xB3,1); //a
      PrintStr("p. ");
      SendByte(0xB7,1); //c
      PrintStr("a");
      SendByte(0xBE,1); //i
      PrintStr("yc");
      SendByte(0xBA,1); //e
      PrintStr("a");
      Cursor(1,0);
      SendByte(0xC0,1); //?
      PrintStr(":");
      SendByte(0xBC,1); //i
      choise=0;
      __HAL_TIM_SET_COUNTER(&htim3,TimeON[kanal-1].Hours);
    }
    update_screen=0;
    Cursor(1,11);
    sprintf(R,"%02d%s%02d",TimeON[kanal-1].Hours,":",TimeON[kanal-1].Minutes);
    PrintStr(R); 
    if (button==1)
        {
          choise++;
          button=0;
          if (choise==0){__HAL_TIM_SET_COUNTER(&htim3,TimeON[kanal-1].Hours);}
          if (choise==1){__HAL_TIM_SET_COUNTER(&htim3,TimeON[kanal-1].Minutes);}
          if (choise==2){choise=0; __HAL_TIM_SET_COUNTER(&htim3,TimeON[kanal-1].Hours);}
        }
    if (button==2)  //save current time
        {
          button=0;
          screen=4;
          update_screen=1;   
        }       
    switch(choise)
    {
      case 0:
            Cursor(1,11);  //setHour
            TimeON[kanal-1].Hours=(EncoderVal%24);
      break;
      case 1:
            Cursor(1,14);  //setMin
            TimeON[kanal-1].Minutes=(EncoderVal%60);
      break;
     }
     
    
    break;
    case 4:
        if (update_screen==1)
    {
      ClearLCDScreen(); 
      Cursor(0,0);
      sprintf(R,"%s%d%s","K-",kanal," ");
      PrintStr(R);    
      SendByte(0xBE,1); //i
      PrintStr("o");
      SendByte(0xB3,1); //a
      SendByte(0xBF,1); //o
      PrintStr(". ");  
      SendByte(0xC0,1); //?  
      PrintStr("epe");  
      SendByte(0xB7,1); //c  
      Cursor(1,0);
      SendByte(0xE3,1); //a
      PrintStr(";");
      SendByte(0xC0,1); //?
      PrintStr(":");
      SendByte(0xBC,1); //i  
      choise=0;
      __HAL_TIM_SET_COUNTER(&htim3,DayRepeat[kanal-1]);
    }
    update_screen=0;
    //-------------------
    Cursor(1,8);
    sprintf(R,"%d%s%02d%s%02d",DayRepeat[kanal-1],";",TimeRepeat[kanal-1].Hours,":",TimeRepeat[kanal-1].Minutes);
    PrintStr(R); 
    if (button==1)
        {
          choise++;
          button=0;
          if (choise==1){__HAL_TIM_SET_COUNTER(&htim3,TimeRepeat[kanal-1].Hours);}
          if (choise==2){__HAL_TIM_SET_COUNTER(&htim3,TimeRepeat[kanal-1].Minutes);}
          if (choise==3){choise=0; __HAL_TIM_SET_COUNTER(&htim3,DayRepeat[kanal-1]);}
        }
    if (button==2)  //save current time
        {
          button=0;
          screen=5;
          update_screen=1;   
        }       
    switch(choise)
    {
      case 0:
            Cursor(1,8);  //setDay
            DayRepeat[kanal-1]=(EncoderVal%10);
            dayRepeatOld[kanal-1]=DayRepeat[kanal-1];
      break;
      case 1:
            Cursor(1,10);  //setHour
            TimeRepeat[kanal-1].Hours=(EncoderVal%24);
      break;
      case 2:
            Cursor(1,13);  //setMin
            TimeRepeat[kanal-1].Minutes=(EncoderVal%60);
      break;
     }
     
    
    break;
    case 5:             
        if (update_screen==1)
    {
         ClearLCDScreen();
         Cursor(0,0);
         sprintf(R,"%s%d%s","K-",kanal," ");
         PrintStr(R);
         SendByte(0xBE,1); //i
         PrintStr("po");
         SendByte(0xE3,1); //a
         PrintStr(". pa");    
         SendByte(0xB2,1); //a
         PrintStr("o");    
         SendByte(0xBF,1); //o
         SendByte(0xC3,1); //u
         Cursor(1,0);
         SendByte(0xC0,1); //?
         PrintStr(":");
         SendByte(0xBC,1); //i  
         PrintStr(":c");
         choise=0;
         __HAL_TIM_SET_COUNTER(&htim3,TimeDuriation[kanal-1].Hours);
    }
      update_screen=0;
      Cursor(1,7);      
      sprintf(R,"%02d%s%02d%s%02d",TimeDuriation[kanal-1].Hours,":",TimeDuriation[kanal-1].Minutes,":",TimeDuriation[kanal-1].Seconds);
      PrintStr(R); 
      if (button==1)
        {
          choise++;
          button=0;
          if (choise==1){__HAL_TIM_SET_COUNTER(&htim3,TimeDuriation[kanal-1].Minutes);}
          if (choise==2){__HAL_TIM_SET_COUNTER(&htim3,TimeDuriation[kanal-1].Seconds);}
          if (choise==3){choise=0; __HAL_TIM_SET_COUNTER(&htim3,TimeDuriation[kanal-1].Hours);}
        }
      if (button==2)  //save current time
        {
          button=0;
          screen=1;
          update_screen=1;   
          osThreadDef(reserv, StartTask03, osPriorityRealtime, 0, 40);
          reservHandle = osThreadCreate(osThread(reserv), NULL);
        }       
      switch(choise)
      {
        case 0:
            Cursor(1,7);  //setHours
            TimeDuriation[kanal-1].Hours=(EncoderVal%24);
        break;
        case 1:
            Cursor(1,10);  //setMin
            TimeDuriation[kanal-1].Minutes=(EncoderVal%60);
        break;
        case 2:
            Cursor(1,13);  //setSec
            TimeDuriation[kanal-1].Seconds=(EncoderVal%60);
        break;
      }
     break;
    
    
    
    
    }
    
  }
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the alarm_control thread.
* @param argument: Not used
* @retval None
*/
//----------------------100 ms Normal tread-------------------------------//
void StartTask02(void const * argument)
{
  osDelay(100);
  uint8_t dayOld,k,i,minOld,trigLED=0,secOld;
  HAL_RTC_GetDate(&hrtc,&DateNOW,RTC_FORMAT_BIN);
  dayOld=DateNOW.Date;
  minOld=TimeNOW.Minutes;
  
  for(;;)
  {
    osDelay(100);
    EncoderVal=__HAL_TIM_GET_COUNTER(&htim3);
    HAL_RTC_GetTime(&hrtc,&TimeNOW,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&DateNOW,RTC_FORMAT_BIN);
    if (encValOld!=EncoderVal)
    {
        encValOld=EncoderVal;    
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
        trigLED=TimeNOW.Seconds;
    }



   //--------------------button--------------------------
    if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)==GPIO_PIN_RESET) 
      {
        k++;
      }
    if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)==GPIO_PIN_SET) 
      {
        if(k>6)
        {
          button=2;
        }else if(k>0)
        {
          button=1;
        }
        k=0;
      }
   //--------------------------------------------------------
    
   //-------------------startuem---------------------------
    if (DateNOW.Date!=dayOld)
      {
      dayOld=DateNOW.Date;
      dayWork++;
      for(i=0;i<3;i++){
      dayRepeatOld[i]--;
      if  (dayRepeatOld[i]==255){dayRepeatOld[i]=DayRepeat[i];}}
      }
    for(i=0;i<3;i++)
    {
      if ((TimeDuriation[i].Hours>0)||(TimeDuriation[i].Minutes>0)||(TimeDuriation[i].Seconds>0)){
        chanelON[i]=1;
      }else{chanelON[i]=0;}
      
    

      if ((secOld!=TimeNOW.Seconds)&&(chanelON[i]==1)&&(TimeON[i].Hours==TimeNOW.Hours)&&(TimeON[i].Minutes==TimeNOW.Minutes)&&(TimeON[i].Seconds==TimeNOW.Seconds)&&(dayRepeatOld[i]==0))      
      {
        startuem[i]=1;
        TimeON[i].Seconds=(TimeNOW.Seconds+TimeRepeat[i].Seconds)%60;
        TimeON[i].Minutes=(TimeNOW.Minutes+TimeRepeat[i].Minutes +(TimeNOW.Seconds+TimeRepeat[i].Seconds)/60)%60;   
        TimeON[i].Hours=(TimeNOW.Hours+TimeRepeat[i].Hours+(TimeNOW.Minutes+TimeRepeat[i].Minutes +(TimeNOW.Seconds+TimeRepeat[i].Seconds)/60)/60)%24;
      }
      if (startuem[i]==1)
      {
        startuem[i]=0;
        TimeEND[i].Seconds=(TimeNOW.Seconds+TimeDuriation[i].Seconds)%60;
        TimeEND[i].Minutes=(TimeNOW.Minutes+TimeDuriation[i].Minutes +(TimeNOW.Seconds+TimeDuriation[i].Seconds)/60)%60;
        TimeEND[i].Hours=(TimeNOW.Hours+TimeDuriation[i].Hours+(TimeNOW.Minutes+TimeDuriation[i].Minutes +(TimeNOW.Seconds+TimeDuriation[i].Seconds)/60)/60)%24;
        if (i==0)  //kanal1
        {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
        }
        if (i==1)  //kanal2
        {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);
        }
        if (i==2)  //kanal3
        {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
        }  
      }
      if ((secOld!=TimeNOW.Seconds)&&(chanelON[i]==1)&&(TimeEND[i].Hours==TimeNOW.Hours)&&(TimeEND[i].Minutes==TimeNOW.Minutes)&&(TimeEND[i].Seconds==TimeNOW.Seconds))
      {
         if (i==0)  //kanal1
        {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
        }
        if (i==1)  //kanal1
        {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);
        }
        if (i==2)  //kanal1
        {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
        } 
      }
   
    }
    
    //---------------------------------------------------------------------
    //-----------------------clear screen--------------------------------
    if ((screen==1)&(minOld!=TimeNOW.Minutes))
    {
         ClearLCDScreen();
         update_screen=1;
         minOld=TimeNOW.Minutes;
    }
    if (secOld!=TimeNOW.Seconds)
    {
         secOld=TimeNOW.Seconds;
         if ((trigLED+30)%60==TimeNOW.Seconds)
         {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
         }
    }
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the reserv thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  uint32_t flashtemp,flash_ret;
  for(;;)
  {
    //----flash_write---
      for (n=0;n<3;n++)
      {
        
        flashtemp=((TimeDuriation[n].Seconds<<8)&0x0000FF00);
        flashtemp|=((TimeDuriation[n].Minutes<<16)&0x00FF0000);
        flashtemp|=((TimeDuriation[n].Hours<<24)&0xFF000000);
        set[n]=flashtemp;      
        flashtemp=((TimeRepeat[n].Hours<<8)&0x0000FF00);
        flashtemp|=((TimeRepeat[n].Minutes<<16)&0x00FF0000);
        flashtemp|=((DayRepeat[n]<<24)&0xFF000000);
        set[n+3]=flashtemp;
      
      }
      set[0]|=t1&0x000000FF;
      set[1]|=t2&0x000000FF;
      HAL_FLASH_Unlock();
      Erase.TypeErase=FLASH_TYPEERASE_PAGES;
      Erase.PageAddress=User_Page_Adress;
      Erase.NbPages=1;
      HAL_FLASHEx_Erase(&Erase,&flash_ret);
      if (flash_ret==0xFFFFFFFF)
      {
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD ,User_Page_Adress,set[0]);
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD ,User_Page_Adress4,set[1]);
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD ,User_Page_Adress8,set[2]);
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD ,User_Page_Adress12,set[3]);
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD ,User_Page_Adress16,set[4]);
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD ,User_Page_Adress20,set[5]);
      HAL_FLASH_Lock();
      flash_ret=0;
      }
      osThreadTerminate(reservHandle);
    //  osDelay(20000);
    }

  /* USER CODE END StartTask03 */
}






/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
