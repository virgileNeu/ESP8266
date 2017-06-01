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
#include "ressources/mcp3204.h"
#include "ressources/ws2812.h"

int main() {
	ws2812_dev ws2812 = ws2812_inst(WS2812_0_BASE);
	ws2812_setConfig(&ws2812, WS2812_DEFAULT_LOW_PULSE,
		WS2812_DEFAULT_HIGH_PULSE, WS2812_DEFAULT_BREAK_PULSE,
		WS2812_DEFAULT_CLOCK_DIVIDER);
	ws2812_setPower(&ws2812, 0);
	ws2812_writePixel(&ws2812, 0, 0, 0, 0);
	ws2812_setIntensity(&ws2812, 0);

	i2c_pio_dev pio = i2c_pio_inst(I2C_PIO_0_BASE);
	mcp3204_dev mcp = mcp3204_inst(MCP3204_0_BASE);
	esp8266_dev esp8266 = esp8266_inst(ESP8266_0_BASE);

	i2c_pio_write(&pio, 0);
	i2c_pio_writebit(&pio, BIT_WIFI_PD_CH, 1);
	i2c_pio_writebit(&pio, BIT_WIFI_RESETn, 1);


	usleep(1000000);
	WIFI_set_CTRL(&esp8266, WIFI_UART_ON | WIFI_STOP_0);
	WIFI_set_baud_rate(&esp8266, b115200);
	WIFI_reset_FIFO(&esp8266, WIFI_RESET_FIFO_IN | WIFI_RESET_FIFO_OUT);

	//commands to connect the client
	char message[100];

	WIFI_send_command(&esp8266, "ATE0", 4); //turn off echo
	WIFI_get_data_terminator(&esp8266, message); //the echo (ATE0\r\n)
	WIFI_get_data_terminator(&esp8266, message); //the "\r\n" before the answer
	WIFI_get_data_terminator(&esp8266, message); //the answer
	if(strncmp(message, "OK", 2)) {
		printf("UART command not ok\n");
		return -1;
	}
	WIFI_send_command(&esp8266, "AT+CWMODE=3", 11); //allow softAP + station
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));


	WIFI_send_command(&esp8266, "AT+CIPMUX=0", 11); //single connection (client)
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));

	char ssid[100], pass[100];
	printf("SSID?");
	scanf("%s", ssid);
	printf("Password?");
	scanf("%s", pass);
	sprintf(message, "AT+CWJAP=\"%s\",\"%s\"", ssid, pass);
	WIFI_send_command(&esp8266, message, strnlen(message, 1000)); //connect to router
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));

	char ip[100];
	printf("IP?");
	scanf("%s", ip);
	sprintf(message, "AT+CIPSTART=\"TCP\",\"%s\",333", ip);
	WIFI_send_command(&esp8266, message, strnlen(message, 1000)); //connect to server
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));


	WIFI_send_command(&esp8266, "AT+CIPMODE=1", 12); //transparent comm on
	do {
		WIFI_get_data_terminator(&esp8266, message);
		printf("%s",message);
	} while(strncmp(message, "OK", 2));
	WIFI_send_command(&esp8266, "AT+CIPSEND", 10); //start comm

	printf("PRESS RIGHT JOY TO START, LEFT TO PAUSE, BOTH TO STOP\n");
	printf("LEFT-JOY Y AXIS for BLUE\nRIGHT-JOY Y AXIS for RED\nRIGHT-JOY X AXIS for GREEN\n");
	printf("y=0 is up, y=255 is down, x=0 is left, x=255 is right\n");
	fflush(stdout);

	int loop = 1;
	int stop = 1;
	while(loop) {
		usleep(10000);
		WIFI_get_all_data(&esp8266, message);
		printf("%s", message);
		if(stop) {
			i2c_pio_writebit(&pio, BIT_J0SWRn, 1);
			i2c_pio_writebit(&pio, BIT_J1SWRn, 1);
			if(!i2c_pio_readbit(&pio, BIT_J1SWRn) && !i2c_pio_readbit(&pio, BIT_J0SWRn)) {
				//WIFI_send_command(&esp8266, "AT+CIPSEND=5", 12);
				WIFI_send_message(&esp8266, "OFF\r\n", 5);
				loop = 0;
			} else if(!i2c_pio_readbit(&pio, BIT_J1SWRn)){
				//WIFI_send_command(&esp8266, "AT+CIPSEND=7", 12);
				WIFI_send_message(&esp8266, "START\r\n", 7);
				stop = 0;
			}
		} else {
			i2c_pio_writebit(&pio, BIT_J0SWRn, 1);
			if(!i2c_pio_readbit(&pio, BIT_J0SWRn)) { //joy 0 pressed3
				//WIFI_send_command(&esp8266, "AT+CIPSEND=6", 12);
				WIFI_send_message(&esp8266, "STOP\r\n", 6);
				stop = 1;
			}
			//we shift by 4 on the left to get values
			//from 4096 range to 256 range (2^12 -> 2^8)
			uint32_t r = mcp3204_read(&mcp, 0) >> 4;
			uint32_t g = mcp3204_read(&mcp, 1) >> 4;
			uint32_t b = mcp3204_read(&mcp, 2) >> 4;
			//send rgb
			sprintf(message, "R%03" PRIu32 "\r\nG%03" PRIu32 "\r\nB%03" PRIu32 "\r\n", r,g,b);
			//WIFI_send_command(&esp8266, "AT+CIPSEND=XXXXXXX", 12);
			WIFI_send_message(&esp8266, message, 18);
			/*do {
				WIFI_get_data_terminator(&esp8266, message);
				printf("%s",message);
			} while(strncmp(message, "SEND OK", 7));*/
			/*//send green
			sprintf(message, "G%03" PRIu32 "\r\n", g);
			WIFI_send_command(&esp8266, "AT+CIPSEND=6", 12);
			WIFI_send_message(&esp8266, message, strnlen(message, 100));
			//send blue
			sprintf(message, "B%03" PRIu32 "\r\n", b);
			WIFI_send_command(&esp8266, "AT+CIPSEND=6", 12);
			WIFI_send_message(&esp8266, message, strnlen(message, 100));*/
		}
	}
	printf("DONE");
	return 0;
}

