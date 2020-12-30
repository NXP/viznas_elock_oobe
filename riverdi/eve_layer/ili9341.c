/*
 * Copyright (c) Bridgetek Pte Ltd
 * Copyright (c) Riverdi Sp. z o.o. sp. k. <riverdi@riverdi.com>
 * Copyright (c) Skalski Embedded Technologies <contact@lukasz-skalski.com>
 *
 * THIS SOFTWARE IS PROVIDED BY BRIDGETEK PTE LTD "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL BRIDGETEK PTE LTD BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES
 * LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * BRIDGETEK DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON
 * BRIDGETEK PARTS.
 *
 * BRIDGETEK DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE
 * INFORMATION IS NOT MODIFIED.
 *
 * IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED,
 * IT IS THE RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES
 * AND SUBSEQUENT WHQL RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.
 */
#if RTVISION_BOARD

#include "ili9341.h"

#define ILI_DELAY_US (10)

static void ILI_DelayUs(uint32_t us)
{

    uint32_t i;
    uint32_t loopPerUs = SystemCoreClock / 3000000;

    while (us--) {
        i = loopPerUs;

        while (i--) {
            __NOP();
        }
    }
}

static void
ILI_CS_Send (Gpu_Hal_Context_t  *host,
             uint8_t             data)
{
  unsigned char m=0x80;

  for (int i=0; i<8; i++)
    {
      platform_gpio_value (host, GPIO_CLK, GPIO_LOW);
      if(data&m)
        platform_gpio_value (host, GPIO_SDO, GPIO_HIGH);
      else
        platform_gpio_value (host, GPIO_SDO, GPIO_LOW);

      platform_gpio_value (host, GPIO_CLK, GPIO_HIGH);
      ILI_DelayUs(ILI_DELAY_US);
      m=m>>1;
    }
    platform_gpio_value (host, GPIO_CLK, GPIO_LOW);
}


static void
CS_ILI (Gpu_Hal_Context_t  *host,
	uint8_t             mode)
{
  switch (mode)
    {
      case 0:
	    platform_gpio_value (host, GPIO_CS, GPIO_LOW);

        ILI_CS_Send (host, ((REG_GPIO >> 16) & 0xBF) | 0x80);
        ILI_CS_Send (host, (REG_GPIO & 0xFF00) >> 8);
        ILI_CS_Send (host, (REG_GPIO & 0xFF));
        ILI_CS_Send (host, 0x00);

	    platform_gpio_value (host, GPIO_CS, GPIO_HIGH);
      break;

      case 1:
        platform_gpio_value (host, GPIO_CS, GPIO_LOW);

        ILI_CS_Send (host, ((REG_GPIO >> 16) & 0xBF) | 0x80);
        ILI_CS_Send (host, (REG_GPIO & 0xFF00) >> 8);
        ILI_CS_Send (host, (REG_GPIO & 0xFF));
        ILI_CS_Send (host, 0x83);

	    platform_gpio_value (host, GPIO_CS, GPIO_HIGH);
        platform_gpio_value (host, GPIO_SDO, GPIO_LOW);
        platform_gpio_value (host, GPIO_CLK, GPIO_LOW);
      break;
    }
}


static void
ILI_Send (Gpu_Hal_Context_t  *host,
          DC                  type,
          uint8_t             data)
{
  unsigned char m=0x80;

  if(type == COMMAND)
    {
      platform_gpio_value (host, GPIO_CLK, GPIO_LOW);

      platform_gpio_value (host, GPIO_SDO, GPIO_LOW);
      platform_gpio_value (host, GPIO_CLK, GPIO_HIGH);
    }
  else if(type == DATA)
    {
      platform_gpio_value (host, GPIO_CLK, GPIO_LOW);

      platform_gpio_value (host, GPIO_SDO, GPIO_HIGH);
      platform_gpio_value (host, GPIO_CLK, GPIO_HIGH);
    }

  for (int i=0;i<8;i++)
    {
      platform_gpio_value (host, GPIO_CLK, GPIO_LOW);

      if (data&m)
        platform_gpio_value (host, GPIO_SDO, GPIO_HIGH);
      else
        platform_gpio_value (host, GPIO_SDO, GPIO_LOW);

      platform_gpio_value (host, GPIO_CLK, GPIO_HIGH);

      m=m>>1;
    }

  platform_sleep_ms (1);
}


static void
ILI_SPI (Gpu_Hal_Context_t  *host)

{
  platform_gpio_init (host, GPIO_SDO);
  platform_gpio_init (host, GPIO_CLK);
}


void
ILI_init (Gpu_Hal_Context_t *host)
{
  /* deinit spi and gpio */
  platform_spi_deinit (host);

  /* init SPI on GPIO */
  ILI_SPI (host);
//  platform_sleep_ms (10);

  /* software reset */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0x01);
  platform_sleep_ms(5);
  CS_ILI (host, 1);

  /* display off */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0x28);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xcf);
  ILI_Send (host, DATA, 0x00);
  ILI_Send (host, DATA, 0x81);
  ILI_Send (host, DATA, 0x30);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xed);
  ILI_Send (host, DATA, 0x64);
  ILI_Send (host, DATA, 0x03);
  ILI_Send (host, DATA, 0x12);
  ILI_Send (host, DATA, 0x81);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xe8);
  ILI_Send (host, DATA, 0x85);
  ILI_Send (host, DATA, 0x01);
  ILI_Send (host, DATA, 0x79);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xcb);
  ILI_Send (host, DATA, 0x39);
  ILI_Send (host, DATA, 0x2c);
  ILI_Send (host, DATA, 0x00);
  ILI_Send (host, DATA, 0x34);
  ILI_Send (host, DATA, 0x02);
  CS_ILI (host, 1);

  /* interface control */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xF6);
  ILI_Send (host, DATA, 0x01);
  ILI_Send (host, DATA, 0x00);
  ILI_Send (host, DATA, 0x06);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xf7);
  ILI_Send (host, DATA, 0x20);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xea);
  ILI_Send (host, DATA, 0x06);
  ILI_Send (host, DATA, 0x00);
  CS_ILI (host, 1);

  /* power control */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xc0);
  ILI_Send (host, DATA, 0x26);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xc1);
  ILI_Send (host, DATA, 0x11);
  CS_ILI (host, 1);

  /* VCOM */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xc5);
  ILI_Send (host, DATA, 0x35);
  ILI_Send (host, DATA, 0x3E);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xc7);
  ILI_Send (host, DATA, 0xBE);
  CS_ILI (host, 1);

  /* memory access control */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0x36);
  ILI_Send (host, DATA, 0x40);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0x3a);
  ILI_Send (host, DATA, 0x60);
  CS_ILI (host, 1);

  /* frame rate */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xb0);
  ILI_Send (host, DATA, 0xC0);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xb1);
  ILI_Send (host, DATA, 0x00);
  ILI_Send (host, DATA, 0x1B);
  CS_ILI (host, 1);

  /* gamma */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xf2);
  ILI_Send (host, DATA, 0x02);
  CS_ILI (host, 1);

  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0x26);
  ILI_Send (host, DATA, 0x01);
  CS_ILI (host, 1);

  /* p-gamma correction */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xE0);
  ILI_Send (host, DATA, 0x1f);
  ILI_Send (host, DATA, 0x1a);
  ILI_Send (host, DATA, 0x18);
  ILI_Send (host, DATA, 0x0a);
  ILI_Send (host, DATA, 0x0f);
  ILI_Send (host, DATA, 0x06);
  ILI_Send (host, DATA, 0x45);
  ILI_Send (host, DATA, 0x87);
  ILI_Send (host, DATA, 0x32);
  ILI_Send (host, DATA, 0x0a);
  ILI_Send (host, DATA, 0x07);
  ILI_Send (host, DATA, 0x02);
  ILI_Send (host, DATA, 0x07);
  ILI_Send (host, DATA, 0x05);
  ILI_Send (host, DATA, 0x00);
  CS_ILI (host, 1);

  /* n-gamma correction */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0xE1);
  ILI_Send (host, DATA, 0x00);
  ILI_Send (host, DATA, 0x25);
  ILI_Send (host, DATA, 0x27);
  ILI_Send (host, DATA, 0x05);
  ILI_Send (host, DATA, 0x10);
  ILI_Send (host, DATA, 0x09);
  ILI_Send (host, DATA, 0x3a);
  ILI_Send (host, DATA, 0x78);
  ILI_Send (host, DATA, 0x4d);
  ILI_Send (host, DATA, 0x05);
  ILI_Send (host, DATA, 0x18);
  ILI_Send (host, DATA, 0x0d);
  ILI_Send (host, DATA, 0x38);
  ILI_Send (host, DATA, 0x3a);
  ILI_Send (host, DATA, 0x1f);
  CS_ILI (host, 1);

  /* sleep out */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0x11);
  platform_sleep_ms (10);
  CS_ILI (host, 1);

  /* display on */
  CS_ILI (host, 0);
  ILI_Send (host, COMMAND, 0x29);
  platform_sleep_ms (10);
  CS_ILI (host, 1);
}
#endif
