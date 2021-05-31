/*
 * Copyright (c) 2013 Espressif System.
 *
 *  sdio stub code for RK
 */

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

static int esp_reset_gpio = 13;
module_param(esp_reset_gpio, int, 0);
MODULE_PARM_DESC(esp_reset_gpio, "ESP8089 CH_PD reset GPIO number");

extern void ms_sdmmc_rescan(void);

void sif_platform_rescan_card(unsigned insert)
{
	if (insert)
		ms_sdmmc_rescan();
}

void sif_platform_target_poweroff(void)
{
	gpio_request(esp_reset_gpio,"esp_reset");
	gpio_direction_output(esp_reset_gpio, 1);
	gpio_free(esp_reset_gpio);
}

void sif_platform_target_poweron(void)
{
	gpio_request(esp_reset_gpio,"esp_reset");
	gpio_direction_output(esp_reset_gpio, 0);
	gpio_free(esp_reset_gpio);
}

void sif_platform_target_speed(int high_speed)
{
}

void sif_platform_check_r1_ready(struct esp_pub *epub)
{
}


#ifdef ESP_ACK_INTERRUPT
void sif_platform_ack_interrupt(struct esp_pub *epub)
{
}
#endif //ESP_ACK_INTERRUPT
