/*
 * spi.c
 *
 *  Created on: 29.06.2019
 *      Author: Erich Styger
 */

#include "platform.h"
#include "spi.h"
#include "fsl_spi.h"

#define EXAMPLE_SPI_MASTER SPI7
#define EXAMPLE_SPI_MASTER_IRQ FLEXCOMM7_IRQn
#define EXAMPLE_SPI_MASTER_CLK_SRC kCLOCK_Flexcomm7
#define EXAMPLE_SPI_MASTER_CLK_FREQ CLOCK_GetFreq(kCLOCK_Flexcomm7)
#define EXAMPLE_SPI_SSEL 1
#define SPI_MASTER_IRQHandler FLEXCOMM7_IRQHandler
#define EXAMPLE_SPI_SPOL kSPI_SpolActiveAllLow

#define BUFFER_SIZE (64)
static uint8_t srcBuff[BUFFER_SIZE];
static uint8_t destBuff[BUFFER_SIZE];
static uint32_t txIndex             = BUFFER_SIZE;
static uint32_t rxIndex             = BUFFER_SIZE;
static volatile bool masterFinished = false;
static volatile bool slaveFinished  = false;

void SPI_MASTER_IRQHandler(void)
{
    if ((SPI_GetStatusFlags(EXAMPLE_SPI_MASTER) & kSPI_RxNotEmptyFlag) && (rxIndex > 0))
    {
        destBuff[BUFFER_SIZE - rxIndex] = SPI_ReadData(EXAMPLE_SPI_MASTER);
        rxIndex--;
    }
    if ((SPI_GetStatusFlags(EXAMPLE_SPI_MASTER) & kSPI_TxNotFullFlag) && (txIndex > 0))
    {
        if (txIndex == 1)
        {
            /* need to disable interrupts before write last data */
            SPI_DisableInterrupts(EXAMPLE_SPI_MASTER, kSPI_TxLvlIrq);
            SPI_WriteData(EXAMPLE_SPI_MASTER, (uint16_t)(srcBuff[BUFFER_SIZE - txIndex]), kSPI_FrameAssert);
        }
        else
        {
            SPI_WriteData(EXAMPLE_SPI_MASTER, (uint16_t)(srcBuff[BUFFER_SIZE - txIndex]), 0);
        }
        txIndex--;
    }
    if ((txIndex == 0U) && (rxIndex == 0U))
    {
        masterFinished = true;
        SPI_DisableInterrupts(EXAMPLE_SPI_MASTER, kSPI_RxLvlIrq);
    }
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

void SPI_WriteByte(uint8_t data, uint8_t *rx) {

}

void SPI_Init(void) {
  spi_master_config_t masterConfig = {0};
  uint32_t sourceClock             = 0U;
  uint32_t i                       = 0U;
  uint32_t err                     = 0U;

  /* attach 12 MHz clock to SPI3 */
  CLOCK_AttachClk(kFRO12M_to_FLEXCOMM7);

  /* Init SPI master */
  /*
   * masterConfig->enableLoopback = false;
   * masterConfig->enableMaster = true;
   * masterConfig->polarity = kSPI_ClockPolarityActiveHigh;
   * masterConfig->phase = kSPI_ClockPhaseFirstEdge;
   * masterConfig->direction = kSPI_MsbFirst;
   * masterConfig->baudRate_Bps = 500000U;
   */
  SPI_MasterGetDefaultConfig(&masterConfig);
  sourceClock          = EXAMPLE_SPI_MASTER_CLK_FREQ;
  masterConfig.sselNum = (spi_ssel_t)EXAMPLE_SPI_SSEL;
  masterConfig.sselPol = (spi_spol_t)EXAMPLE_SPI_SPOL;
  SPI_MasterInit(EXAMPLE_SPI_MASTER, &masterConfig, sourceClock);
#if 0
  /* Init source buffer */
  for (i = 0U; i < BUFFER_SIZE; i++)
  {
      srcBuff[i] = i;
  }

  NVIC_SetPriority(EXAMPLE_SPI_MASTER_IRQ, 1U);
  /* Enable interrupt, first enable slave and then master. */
  EnableIRQ(EXAMPLE_SPI_MASTER_IRQ);
  SPI_EnableInterrupts(EXAMPLE_SPI_MASTER, kSPI_TxLvlIrq | kSPI_RxLvlIrq);

  SPI_WriteData(EXAMPLE_SPI_MASTER, 0x1234, kSPI_FrameAssert);

  while (masterFinished != true)
  {
  }

  /* Check the data received */
  for (i = 0U; i < BUFFER_SIZE; i++)
  {
      if (destBuff[i] != srcBuff[i])
      {
 //         PRINTF("\r\nThe %d data is wrong, the data received is %d \r\n", i, destBuff[i]);
          err++;
      }
  }
  if (err == 0U)
  {
//      PRINTF("\r\nSPI transfer finished!\r\n");
  }

  while (1)
  {
  }
#endif
}
