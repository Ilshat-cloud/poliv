
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "ssd1306.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM3_Init(void);


void startScreen(void);
void screen10(void);
void screen1(void);
void screen2(void);
void screen3(void);
void screen4(void);
void screen5(void);

RTC_TimeTypeDef RTime,SetTime;
RTC_DateTypeDef DateNow,SetDate;
uint16_t EncoderVal,N1_time,N2_time,N1_working=0,N2_working=0;

char R[10]="Ch323";
char *state_R="Ch323";
uint8_t Blynk=0,N1on=0,N2on=0,N1N2block=0,LL,HH;
uint8_t Humidity[250];
uint8_t Temperature[250];
uint8_t median[250];
uint8_t median_H, median_T=125,PolHour=0,PolMin=4,state=0,flag=1,PolSetHour=255,PolSetMin=1,set_H=200,set_T=200,flash_flag,Button=0,stratPol=0,choise;    
volatile uint16_t dma[2];
volatile uint16_t adc[2];




void  HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
  {
    adc[0]=dma[0];
    adc[1]=dma[1];
    HAL_ADC_Stop_DMA(&hadc1);
  }


int main(void)
{
 
  HAL_Init();
  SystemClock_Config();

  
  
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_TIM3_Init();
  ssd1306_Init();

  HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_ALL);
 
  //------------massiv start data---------------
  for (EncoderVal=0; EncoderVal<250; EncoderVal++)
  {
    Humidity[EncoderVal]=0;
    Temperature[EncoderVal]=120;
  }
  

  //----------Horse---------------
  startScreen();
  ssd1306_Fill(Black);
  ssd1306_SetCursor(32,0); 
  startScreen();
  ssd1306_Fill(Black);
  ssd1306_SetCursor(64,0); 
  startScreen();
  ssd1306_Fill(Black);
  ssd1306_SetCursor(96,0); 
  startScreen();
  
  //----------Encoder count---------
  uint32_t oldtimeB=HAL_GetTick();
  uint8_t oMin,h,oHour,oSec;
  uint32_t t_sum=0,flash_ret,set,set2,set3;
  uint16_t i,j;
  //-------read from flash--------  
  FLASH_EraseInitTypeDef Erase;
  set=*(__IO uint32_t*)User_Page_Adress;    //4 adresa
  set3=*(__IO uint32_t*)User_Page_Adress4;    //4 adresa
  set_H=set;
  set_T=set>>8;
  PolSetMin=set>>16;
  PolSetHour=set>>24;
  N2_time=set3>>16;
  N1_time=set3;
  if (N1_time>1000){N1_time=100;}
  if (N2_time>1000){N2_time=100;}

  while (1)
  {

    
    
         
    
    
    
    
    
    //----------Every scan---------------
    HAL_RTC_GetDate(&hrtc,&DateNow,RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc,&RTime,RTC_FORMAT_BIN);
    EncoderVal=__HAL_TIM_GET_COUNTER(&htim3);
    
    
    //-------------screen choise-------------
     switch(flag){
     case 1:
       screen1();
     break;
     case 2:
       screen2();
     break;
     case 3:
       screen3();
     break;
     case 4:
       screen4();
     break;
     case 5:
       screen5();
     break;
     }
    
    //-------------Status--------------------
     switch(state){
     case 0:
       state_R="      GOTOB" ;
     break;
     case 1:
       state_R="    Polivaet" ;
     break;
     case 2:
       state_R=" BbICOK. Blajn" ;
        N1N2block=0;
        N2on=0; 
        N1on=0; 
        
     break;
     case 3:
       state_R="   Hi3K. TEMP" ;
        N1N2block=0;
        N2on=0; 
        N1on=0;
        
     break;
     case 4:
       state_R=" ABAP. HACOC-1" ;
        N1N2block=0;
        N2on=0; 
        N1on=0;
        
     break;
     case 5:
       state_R=" ABAP. HACOC-2" ;
        N1N2block=0;
        N2on=0; 
        N1on=0;
        
     break;
     }
     
     
    //----------start_poliv_time or prin------------
    if ((PolHour==0 && PolMin==0)||(stratPol==2))
    {
      PolHour=PolSetHour;
      PolMin=PolSetMin;
      
      if (state==0)
      {
        N1_working=0;
        N2_working=0;
        state=1;
        stratPol=1;
        
      }else{state=0; stratPol=0;}  //sbros 
    } 
    
    //----------control_vlaj--------------
    if (set_H<median_H)
    {
      state=2;
    }
    
    //----------control_TEMP--------------
    if (set_T>median_T)
    {
      state=3;
    }
    
    
    //-----------startyem---------
    if ((stratPol==1)&&(state==1))
    {
      N1N2block=1;
      if ((N1_time>0)&&(!N2on)&&(!HH))
      {
        N1on=1;
      }else{N1on=0;N1_working=0;}
      
      if(!N1on){
        if ((N2_time>0)&&(N2_working<N2_time))
        {
          N2on=1;
        }else{
          N2_working=0;
          N2on=0; 
          state=0;
          N1N2block=0;
      }
      }
    }

    //-----------time_sec avarii-------
    if (RTime.Seconds!=oSec)
    {
      oSec=RTime.Seconds;
      //----------control_Pump_1--------------
      if (N1on)
      {
        N1_working++;
        if (N1_working>N1_time){
          state=4;
        }
      }
      //----------control_Pump_1--------------
      if (N2on)
      {
        N2_working++;
        if (LL){
          state=5;
        }
      }
      
    }  
    
    
    //-----------time_int GPIO-------
    if (HAL_GetTick()>(oldtimeB+250))
    {
      oldtimeB=HAL_GetTick();
      Blynk=~Blynk&0x01;
      if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)==GPIO_PIN_RESET) 
      {
        Button=1;
      }else{Button=0;}
      
       if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4)==GPIO_PIN_SET) 
      {
        HH=1;
      }else{HH=0;}
      if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5)==GPIO_PIN_SET) 
      {
        LL=1;
      }else{LL=0;}
      if (N1N2block==1) 
      {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
      }else{HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);}
      if (N1on==1) 
      {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);
      }else{HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);}
      if (N2on==1) 
      {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
      }else{HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);}
    }  

    
    //----flash_write---
    if (flash_flag==2)  
    {

      set2=(PolSetHour<<24)&0xFF000000;
      set=set2;
      set2=(PolSetMin<<16)&0x00FF0000;
      set=set|set2;
      set2=(set_T<<8)&0x0000FF00;
      set|=set2;
      set2=set_H&0xFF;  
      set|=set2;
      set2=(N2_time<<16)&0xFFFF0000;
      set3=set2;
      set2=N1_time;   
      set3=set3|set2;
      HAL_FLASH_Unlock();
      Erase.TypeErase=FLASH_TYPEERASE_PAGES;
      Erase.PageAddress=User_Page_Adress;
      Erase.NbPages=1;
      HAL_FLASHEx_Erase(&Erase,&flash_ret);
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD ,User_Page_Adress,set);
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD ,User_Page_Adress4,set3);
      HAL_FLASH_Lock();
      flash_flag=0;
      
    }

    
    //----update_RTC---
    if (flash_flag==3)  {

    if (HAL_RTC_SetTime(&hrtc, &SetTime, RTC_FORMAT_BIN) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    if (HAL_RTC_SetDate(&hrtc, &SetDate, RTC_FORMAT_BIN) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }
    
    
    flash_flag=0;
    }
    
    //--------time_hour-------
    if (RTime.Hours!=oHour)
    {
      oHour=RTime.Hours;
    }
    
    //--------time_min-------
    if (RTime.Minutes!=oMin)
    {
      oMin=RTime.Minutes;
       if (PolMin>0)
      {
        PolMin--;
      } else {
      if (PolHour>0)
        {
          PolHour--;
          PolMin=59;
        }
      }
      
      //----------DMA---------
      HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&dma,2);
      
      //-------- median average-------
      for (i=1; i<250; i++)
      {
        Humidity[i-1]=Humidity[i];
        Temperature[i-1]=Temperature[i];
      }    
      Humidity[249]=adc[0]/16;
      Temperature[249]=adc[1]/16;
      for (i=0; i<250; i++)
      {
        median[i]=Humidity[i];
      }
      h=0;
      for (i=0; i<250; i++)
      {
        for (j=0;j<249;j++)
        {
          if (median[j]>median[j+1])
          {
            h=median[j];
            median[j]=median[j+1];
            median[j+1]=h;
          }
        }
      }
      median_H=median[125];
      t_sum=0;
      for (i=0; i<250; i++)
      {
        t_sum=Temperature[i]+t_sum;
        
      }
      median_T=t_sum/250;
      
    } 
    
  }
 
}

//-------------------------screen1 MAIN------------------------
void screen1() {
  
  ssd1306_Fill(Black);
  ssd1306_SetCursor(0,0);
  sprintf(R,"%d",DateNow.Date);
  ssd1306_WriteString(R, Font_7x10, White);
  ssd1306_WriteString(".", Font_7x10, White);
  sprintf(R,"%d",DateNow.Month);
  ssd1306_WriteString(R, Font_7x10, White);
  ssd1306_WriteString(".", Font_7x10, White);
  sprintf(R,"%d",DateNow.Year);
  ssd1306_WriteString(R, Font_7x10, White);
  ssd1306_SetCursor(71, 0);
  sprintf(R,"%d",RTime.Hours);
  ssd1306_WriteString(R, Font_7x10, White);
  ssd1306_WriteString(":", Font_7x10, White);
  sprintf(R,"%d",RTime.Minutes);
  ssd1306_WriteString(R, Font_7x10, White);
  ssd1306_WriteString(":", Font_7x10, White);
  sprintf(R,"%d",RTime.Seconds);
  ssd1306_WriteString(R, Font_7x10, White);
  
  
  ssd1306_SetCursor(0, 21);
  ssd1306_WriteString("Do poliva:", Font_7x10, White);
  sprintf(R,"%d",PolHour/24);
  ssd1306_WriteString(R, Font_7x10, White);
  ssd1306_WriteString(".", Font_7x10, White);
  sprintf(R,"%d",PolHour%24);
  ssd1306_WriteString(R, Font_7x10, White);
  ssd1306_WriteString(":", Font_7x10, White);
  sprintf(R,"%d",PolMin);
  ssd1306_WriteString(R, Font_7x10, White);
  
  ssd1306_SetCursor(0, 10);
  switch(EncoderVal%7){
    case 0:
        ssd1306_WriteString(state_R, Font_7x10, White);
      break;
   case 1:
        ssd1306_WriteString("HACTPOIKA HACOCOB", Font_7x10, White);
        if (Button){
          flag=2;Button=0; choise=1;
          __HAL_TIM_SET_COUNTER(&htim3,N1_time);
        }
      break;
   case 2:
        ssd1306_WriteString("HACTP. TEK. BPEM.", Font_7x10, White);
        if (Button){
          flag=3;Button=0;choise=1;
          SetTime=RTime;
          SetDate=DateNow;
           __HAL_TIM_SET_COUNTER(&htim3,SetDate.Date-1);
        }
      break;
   case 3:
        ssd1306_WriteString("POROG Vlaj, Temp", Font_7x10, White);
        if (Button){
          flag=4;Button=0;choise=1;
          __HAL_TIM_SET_COUNTER(&htim3,set_H);
        }
               
      break;
   case 4:
        ssd1306_WriteString("YCT BP. POliBA", Font_7x10, White);
        if (Button){
          flag=5;Button=0;choise=1;
          __HAL_TIM_SET_COUNTER(&htim3,PolSetHour);
        }
      break;   
   case 5:
        ssd1306_WriteString("Blaj-", Font_7x10, White);
        sprintf(R,"%d",median_H);
        ssd1306_WriteString(R, Font_7x10, White);
        ssd1306_WriteString(" Temp-", Font_7x10, White);
        sprintf(R,"%d",median_T);
        ssd1306_WriteString(R, Font_7x10, White);
      break; 
   case 6:
        ssd1306_WriteString("POliTb CEi4AC?", Font_7x10, White);
        if (Button){
          Button=0; 
          choise++;
          if (choise>6){
          __HAL_TIM_SET_COUNTER(&htim3,7);
          stratPol=2;
          choise=0;
          }
        }
      break;
  }
  
  
  
  ssd1306_UpdateScreen();
}

//-------------------------screen2 pump------------------------
void screen2() {
  
  ssd1306_Fill(Black);
  ssd1306_SetCursor(0,0);
  ssd1306_WriteString("BPEM. PA6. HACOCA", Font_7x10, White);
  ssd1306_SetCursor(0,10);
  ssd1306_WriteString("HACOC 1-", Font_7x10, White);  
  ssd1306_SetCursor(0,21);
  ssd1306_WriteString("HACOC 2-", Font_7x10, White);
  ssd1306_SetCursor(84,10);
  ssd1306_WriteString("CEK", Font_7x10, White);  
  ssd1306_SetCursor(84,21);
  ssd1306_WriteString("CEK", Font_7x10, White);
  if (Button){
    choise++;
    Button=0;
    if (choise==2){__HAL_TIM_SET_COUNTER(&htim3,N2_time);}
  }
  ssd1306_SetCursor(56,10);
  sprintf(R,"%d",N1_time);
  if (choise==1)
  {
    N1_time=EncoderVal;
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  sprintf(R,"%d",N2_time);
  ssd1306_SetCursor(56,21);
  if (choise==2)
  {
    N2_time=EncoderVal;
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  if (choise==3)
  {
    choise=0;
    flag=1;
    flash_flag=2;
  } 
  ssd1306_UpdateScreen();
}

//-------------------------screen3 set current time------------------------
void screen3() {
  
  ssd1306_Fill(Black);
  ssd1306_SetCursor(0,0);
  ssd1306_WriteString("HACTP. TEK. BPEM.", Font_7x10, White);
  ssd1306_SetCursor(0,10);
  ssd1306_WriteString("Data-", Font_7x10, White);

  if (Button){
    choise++;
    Button=0;
    if (choise==2){__HAL_TIM_SET_COUNTER(&htim3,SetDate.Month-1);}
    if (choise==3){__HAL_TIM_SET_COUNTER(&htim3,SetDate.Year);}
    if (choise==4){__HAL_TIM_SET_COUNTER(&htim3,SetTime.Hours);}
    if (choise==5){__HAL_TIM_SET_COUNTER(&htim3,SetTime.Minutes);}
    if (choise==6){__HAL_TIM_SET_COUNTER(&htim3,SetTime.Seconds);}
  }
  
  sprintf(R,"%d",SetDate.Date);
  if (choise==1)
  {
    SetDate.Date=EncoderVal%31+1;
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_WriteString(".", Font_7x10, White);
  sprintf(R,"%d",SetDate.Month);
  if (choise==2)
  {
    SetDate.Month=EncoderVal%12+1;
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_WriteString(".", Font_7x10, White);
  sprintf(R,"%d",SetDate.Year);
  if (choise==3)
  {
    SetDate.Year=EncoderVal%100;
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_SetCursor(0,21);
  ssd1306_WriteString("BPEM-", Font_7x10, White);
  sprintf(R,"%d",SetTime.Hours);
  if (choise==4)
  {
    SetTime.Hours=EncoderVal%24;
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_WriteString(":", Font_7x10, White);
  sprintf(R,"%d",SetTime.Minutes);
  if (choise==5)
  {
    SetTime.Minutes=EncoderVal%60;
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_WriteString(":", Font_7x10, White);
  sprintf(R,"%d",SetTime.Seconds);
  if (choise==6)
  {
    SetTime.Seconds=EncoderVal%60;
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  if (choise==7)
  {
    choise=0;
    flag=1;
    flash_flag=3;
  }
  ssd1306_UpdateScreen();
}

//-------------------------screen4 Porog Vlaj------------------------
void screen4() {
  
  ssd1306_Fill(Black);
  ssd1306_SetCursor(0,0);
  ssd1306_WriteString("porog vlajn i temp", Font_7x10, White);
  ssd1306_SetCursor(0,10);
  ssd1306_WriteString("Vlajnost>", Font_7x10, White);  
  ssd1306_SetCursor(0,21);
  ssd1306_WriteString("Temperat<", Font_7x10, White);
  ssd1306_SetCursor(84,10);
  ssd1306_WriteString(" 0-255", Font_7x10, White);  
  ssd1306_SetCursor(84,21);
  ssd1306_WriteString(" C/10", Font_7x10, White);
  if (Button){
    choise++;
    Button=0;
    
    if (choise==2){__HAL_TIM_SET_COUNTER(&htim3,set_T);}
  }
  ssd1306_SetCursor(63,10);
  sprintf(R,"%d",set_H);
  if (choise==1)
  {
    set_H=EncoderVal;
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  sprintf(R,"%d",set_T);
  ssd1306_SetCursor(63,21);
  if (choise==2)
  {
    set_T=EncoderVal;
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  if (choise==3)
  {
    choise=0;
    flag=1;
    flash_flag=2;
  } 
  ssd1306_UpdateScreen();
  
}

//-------------------------screen5_time_watering-----------------------
void screen5() {
  
  ssd1306_Fill(Black);
  ssd1306_SetCursor(0,0);
  ssd1306_WriteString("period Dn.4ac:Min", Font_7x10, White);
  ssd1306_SetCursor(0,10);
  ssd1306_WriteString("Dnei-", Font_7x10, White);  
  ssd1306_SetCursor(49,10);
  ssd1306_WriteString(" 4ac-", Font_7x10, White);  

  ssd1306_SetCursor(0,21);
  ssd1306_WriteString("Minut-", Font_7x10, White);
 
  if (Button){
    choise++;
    Button=0;
    if (choise==2){__HAL_TIM_SET_COUNTER(&htim3,PolSetHour);}
    if (choise==3){__HAL_TIM_SET_COUNTER(&htim3,PolSetMin);}
  }
  ssd1306_SetCursor(35,10);
  sprintf(R,"%d",PolSetHour/24);
  if (choise==1)
  {
    PolSetHour=PolSetHour+(EncoderVal-PolSetHour)*24; 
    __HAL_TIM_SET_COUNTER(&htim3,PolSetHour);
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_SetCursor(84,10);
  sprintf(R,"%d",PolSetHour%24);
  if (choise==2)
  {
    PolSetHour=PolSetHour+(EncoderVal%256-PolSetHour);
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_SetCursor(49,21);
  sprintf(R,"%d",PolSetMin%60);
  if (choise==3)
  {
    PolSetMin=PolSetMin+(EncoderVal%60-PolSetMin);
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  if (choise==4)
  {
    choise=0;
    flag=1;
    flash_flag=2;
  } 
  ssd1306_UpdateScreen();
}

//-------------------------screen10------------------------
void screen10() {
  
  ssd1306_Fill(Black);
  ssd1306_SetCursor(0,0);
  sprintf(R,"%d",DateNow.Date);
  if (EncoderVal%10==0)
  {
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_WriteString(".", Font_7x10, White);
  sprintf(R,"%d",DateNow.Month);
  if (EncoderVal%10==1)
  {
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_WriteString(".", Font_7x10, White);
  sprintf(R,"%d",DateNow.Year);
  if (EncoderVal%10==2)
  {
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_SetCursor(71, 0);
  sprintf(R,"%d",RTime.Hours);
  if (EncoderVal%10==3)
  {
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_WriteString(":", Font_7x10, White);
  sprintf(R,"%d",RTime.Minutes);
  if (EncoderVal%10==4)
  {
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_WriteString(":", Font_7x10, White);
  sprintf(R,"%d",RTime.Seconds);
  if (EncoderVal%10==5)
  {
    ssd1306_WriteString(R, Font_7x10, Blynk);
  } else {
    ssd1306_WriteString(R, Font_7x10, White);
  }
  ssd1306_SetCursor(0, 10);
  sprintf(R,"%d",EncoderVal);
  ssd1306_WriteString(R, Font_7x10, White);
  ssd1306_SetCursor(35, 10);
  sprintf(R,"%d",adc[0]);
  ssd1306_WriteString(R, Font_7x10, White);
  ssd1306_SetCursor(70, 10);
  sprintf(R,"%d",adc[1]);
  ssd1306_WriteString(R, Font_7x10, White);
  ssd1306_SetCursor(0, 21);
  sprintf(R,"%d",set_T);
  ssd1306_WriteString(R, Font_7x10, White);
  sprintf(R,"%d",set_H);
  ssd1306_WriteString(R, Font_7x10, Blynk);
  sprintf(R,"%d",PolSetMin);
  ssd1306_WriteString(R, Font_7x10, White);
  sprintf(R,"%d",PolSetHour);
  ssd1306_WriteString(R, Font_7x10, Blynk);
  sprintf(R,"%d",N1_time);
  ssd1306_WriteString(R, Font_7x10, White);
  sprintf(R,"%d",N2_time);
  ssd1306_WriteString(R, Font_7x10, Blynk);
  ssd1306_UpdateScreen();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
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
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
  

  
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 44;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* RTC init function */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef DateToUpdate;

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_SECOND;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /*  *Initialize RTC and set the Time and Date 
  */  
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = 0x03;
  DateToUpdate.Date = 0x19;
  DateToUpdate.Year = 0x19;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_Encoder_InitTypeDef sConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
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

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB0 PB1 PB2 PB3 
                           PB4 PB5 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 PB12 PB13 
                           PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
