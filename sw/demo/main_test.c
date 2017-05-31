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

int main() {
	esp8266_dev esp8266 = esp8266_inst(ESP8266_0_BASE);
	i2c_pio_dev pio = i2c_pio_inst(I2C_PIO_0_BASE);

	i2c_pio_write(&pio, 0);
	i2c_pio_writebit(&pio, BIT_WIFI_PD_CH, 1);
	i2c_pio_writebit(&pio, BIT_WIFI_RESETn, 1);

	WIFI_set_CTRL(&esp8266, WIFI_UART_ON | WIFI_STOP_0);
	WIFI_set_baud_rate(&esp8266, b115200);

	usleep(1000000);
	WIFI_reset_FIFO(&esp8266, WIFI_RESET_FIFO_IN | WIFI_RESET_FIFO_OUT);

	char s[1000];
	int loop = 1;
	while(loop) {
		printf("Command to send (exit to exit)?");
		scanf("%s", s);
		if(!strncmp(s, "exit", 4)) {
			loop = 0;
		} else {
			if(strnlen(s, 1000)>3) {
				WIFI_send_command(&esp8266, s, strnlen(s, 1000));
			}
			printf("\nEnter string for response");
			scanf("%s", s);
			WIFI_get_all_data(&esp8266, s);
			printf("%s", s);
			fflush(stdout);
		}
	}


	printf("End of program\n");
	return 0;
}

