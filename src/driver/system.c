#include "system.h"
#include "time.h"
#include "config.h"
#include "defines.h"
#include "hardware.h"
#include "misc_gpio.h"
#include "bus_softspi.h"
#include "serial_uart.h"
#include "bus_i2c.h"
#include "pwm.h"
#include "mpu6050.h"
#include "adc.h"
#include "rx_bayang.h"
#include "imu.h"
#include "util.h"
#include "led.h"
#include "osd.h"
#include "flash.h"
#include "usbd_cdc_core.h"
#include  "usbd_usr.h"
#include "usb_dcd.h"
#include "usbd_cdc_vcp.h"


USB_CORE_HANDLE  USB_Device_dev ;

extern unsigned char vtx_index;
extern uint8_t openLogBuff[20];
extern float vbattfilt;
extern float vreffilt;

uint8_t systemInit(void)
{
    delay(1000);

#ifdef ENABLE_OVERCLOCK
    //overclock 64M
    setclock();
#endif
    
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;	
    SYSCFG->CFGR1 |= SYSCFG_CFGR1_PA11_PA12_RMP;
    
    USBD_Init(&USB_Device_dev,
            &USR_desc, 
            &USBD_CDC_cb, 
            &USR_cb);
    
    
    //sysTick 1ms
    sysTick_init();

    delay_ms(10);

    flash_load();
    
    //gpio
    gpio_init();

    //softspi
    spi_init();

    //serial uart
    uart_init(4800);

    delay_ms(10);

    osd_init();

    int sa_cnt=0;


    while(sa_cnt <5000)
    {
        openLogBuff[0] =0xAA;
        openLogBuff[1] = 0x55;
        openLogBuff[2] = 0x07;
        openLogBuff[3] = 0x01;
        openLogBuff[4] = vtx_index;
        openLogBuff[5] = CRC8(openLogBuff,5);
        openLogBuff[6] = 0;
        openLogBuff[7] = 0;
        openLogBuff[8] = 0;
        openLogBuff[9] = 0;
        openLogBuff[10] = 0;
        openLogBuff[11] = 0;

        UART2_DMA_Send();

        sa_cnt ++;
    }

    //i2c
    i2c_init();

    //pwm
    pwm_init();
    for(int i=0; i<4; i++)
    {
        pwm_set(i,0);
    }

    //mpu6050
    mpu6050_init();

    if (mpu6050_check())
    {
    }
    else
    {
        return 1;
    }

    //adc
    adc_init();

    delay_ms(10);

    int count = 0;

    while ( count < 5000 )
    {
        float bootadc = adc_read(0)*vreffilt;
        lpf ( &vreffilt, adc_read(1), 0.9968f);
        lpf ( &vbattfilt, bootadc, 0.9968f);
        count++;
    }

    led_init();


    //rx
    rx_init();


    gyro_cal();

    imu_init();


    return 0;
}





