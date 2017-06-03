#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#include "io.h"
#include "system.h"
#include "ressources/esp8266.h"
#include "ressources/i2c_pio.h"
#include "ressources/ws2812.h"

int main() {
	esp8266_dev esp8266 = esp8266_inst(ESP8266_0_BASE);
	i2c_pio_dev pio = i2c_pio_inst(I2C_PIO_0_BASE);
	ws2812_dev ws2812 = ws2812_inst(WS2812_0_BASE);
	ws2812_setPower(&ws2812, 0);
	ws2812_setConfig(&ws2812, WS2812_DEFAULT_LOW_PULSE,
		WS2812_DEFAULT_HIGH_PULSE, WS2812_DEFAULT_BREAK_PULSE,
		WS2812_DEFAULT_CLOCK_DIVIDER);
	uint8_t red, green, blue;
	uint8_t intensity = ws2812_readIntensity(&ws2812);
	ws2812_writePixel(&ws2812, 0, 0, 0, 0);
	ws2812_setIntensity(&ws2812, 0);

	i2c_pio_write(&pio, 0);
	i2c_pio_writebit(&pio, BIT_WIFI_PD_CH, 1);
	i2c_pio_writebit(&pio, BIT_WIFI_RESETn, 1);

	char message[100];

	usleep(1000000); //skip boot info

	WIFI_set_CTRL(&esp8266, WIFI_UART_ON | WIFI_STOP_0);
	WIFI_set_baud_rate(&esp8266, b115200);
	WIFI_reset_FIFO(&esp8266, WIFI_RESET_FIFO_IN | WIFI_RESET_FIFO_OUT | WIFI_NO_PARITY);

	//commands to create the TCP server



	WIFI_send_command(&esp8266, "ATE0", 4); //turn off echo
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));
    
	WIFI_send_command(&esp8266, "AT+CWMODE=3", 11); //allow softAP + station
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));

	WIFI_send_command(&esp8266, "AT+CIPMUX=1", 11); //multi connection (TCP server)
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));

	char ssid[20], pass[20];
	printf("SSID?");
	scanf("%s", ssid);
	printf("Password?");
	scanf("%s", pass);
	sprintf(message, "AT+CWJAP=\"%s\",\"%s\"", ssid, pass);
	WIFI_send_command(&esp8266, message, strnlen(message, 100)); //connect to router
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));

	WIFI_send_command(&esp8266, "AT+CIPSERVER=1", 14); //start server on port 333
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));

	WIFI_send_command(&esp8266, "AT+CIFSR", 8); //print IP addr
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));

	int loop = 1;
	int stop = 1;
	int len = 0;
	int check = 0;
	while(loop) {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
		char tmp[10];
		check = sscanf(message, "+IPD,0,%d:%s\r\n", &len, tmp);
		if(check == 2) {
			strncpy(message, tmp, 10);
		}
		if(stop) {
			if(!strncmp(message, "OFF", 3)) {
				loop = 0;
			} else if(!strncmp(message, "START", 5)) {
				ws2812_writePixel(&ws2812, 0, red, green, blue);
				ws2812_setIntensity(&ws2812, intensity);
				stop = 0;
			}
		} else {
			if(!strncmp(message, "STOP", 5)) {
				ws2812_writePixel(&ws2812, 0, 0, 0, 0);
				ws2812_setIntensity(&ws2812, 0);
				stop = 1;
			} else {
				int tmp;
				char c;
				sscanf(message, "%c%d", &c, &tmp);
				switch(c) {
				case 'R': red = tmp; break;
				case 'G': green = tmp; break;
				case 'B': blue = tmp; break;
				}
				ws2812_writePixel(&ws2812, 0, red, green, blue);
			}
		}
	}
	printf("DONE");
	return 0;
}

