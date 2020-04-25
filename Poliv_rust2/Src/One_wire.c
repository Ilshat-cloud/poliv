
#include "One_wire.h"

uint8_t OW_Reset(USART_TypeDef* USARTx) {
        uint8_t ow_presence;
        UART_HandleTypeDef huart1;
        HAL_NVIC_DisableIRQ(DMA1_Channel4_IRQn);
        HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
        huart1.Instance = USART1;
        huart1.Init.BaudRate = 9600;
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

        ow_presence=0xf0;
        if (HAL_UART_Transmit(&huart1, &ow_presence,1,1000)!=HAL_OK)    //must be Hal_Ok
          {
            taskYIELD();
          }
        if (HAL_UART_Receive(&huart1, &ow_presence,1,1000)!=HAL_OK)     //must be Hal_Ok
          {
            taskYIELD();
          }

        huart1.Init.BaudRate = 9600;
        if (HAL_HalfDuplex_Init(&huart1) != HAL_OK)
        {
          _Error_Handler(__FILE__, __LINE__);
        }

        if (ow_presence != 0xf0) {
                return 1;
        }

        return 0;
}


void OW_SendCommand(USART_TypeDef* USARTx, const uint8_t *command, uint16_t len) {
        DMA_InitTypeDef DMA_InitStructure;

        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USART2->DR);
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) command;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
        DMA_InitStructure.DMA_BufferSize = len;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize =
                        DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        DMA_Init(DMA1_Channel7, &DMA_InitStructure);

        USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
        DMA_Cmd(DMA1_Channel7, ENABLE);
}

