/*
 * ESP8266.h
 *
 *  Created on: Mar 30, 2017
 *      Author: neu
 */

#ifndef ESP_8266_H_
#define ESP_8266_H_

#include <stdint.h>
#include <inttypes.h>
#include "baud_rates.h"

//REG DEFINES
#define WIFI_CTRL_REG 0*4
#define WIFI_STATUS_REG 1*4
#define WIFI_UART_WAIT_CYCLES 2*4
#define WIFI_FIFO_OUT_DATA 3*4
#define WIFI_FIFO_OUT_FREE_SPACE 4*4
#define WIFI_FIFO_IN_DATA 5*4
#define WIFI_FIFO_IN_PENDING_DATA 6*4
#define WIFI_RESET_FIFO 7*4

//CTRL DEFINES
#define WIFI_UART_ON 0b1
#define WIFI_UART_OFF 0
#define WIFI_I_ENABLE_MASK 0b110
#define WIFI_I_ENABLE_RCV 0b10
#define WIFI_I_ENABLE_DROP 0b100
#define WIFI_STOP_MASK 0b1000
#define WIFI_STOP_0 0
#define WIFI_STOP_1 0b1000
#define WIFI_PARTITY_MASK 0b110000
#define WIFI_NO_PARITY 0
#define WIFI_EVEN_PARITY 0b100000
#define WIFI_ODD_PARITY 0b110000

//STATUS DEFINES
#define WIFI_I_PENDING_MASK 0b11
#define WIFI_I_PENDING_RCV 0b1
#define WIFI_I_PENDING_DROP 0b10

//RESET DEFINES
#define WIFI_RESET_FIFO_IN 0b1
#define WIFI_RESET_FIFO_OUT 0b10

/* esp8266 device structure */
typedef struct {
    void *base; /* Base address of component */
} esp8266_dev;


esp8266_dev esp8266_inst(void *base);

uint32_t WIFI_get_CTRL(esp8266_dev *dev);

void WIFI_set_CTRL(esp8266_dev *dev, uint32_t val);

baud_rate WIFI_get_baud_rate(esp8266_dev *dev);

void WIFI_set_baud_rate(esp8266_dev *dev, baud_rate rate);

uint32_t WIFI_get_i_pending(esp8266_dev *dev);

void WIFI_clear_i_pending(esp8266_dev *dev);

uint32_t WIFI_get_free_space(esp8266_dev *dev);

void WIFI_send_word(esp8266_dev *dev, char word);

int WIFI_send_word_safe(esp8266_dev *dev, char word);

int WIFI_send_message(esp8266_dev *dev, char* message, uint32_t length);

int WIFI_send_command(esp8266_dev *dev, char* message, uint32_t length);

uint32_t WIFI_get_pending_data(esp8266_dev *dev);

uint32_t WIFI_wait_for_data(esp8266_dev *dev, uint32_t amount);

char WIFI_get_data(esp8266_dev *dev);

int WIFI_get_data_safe(esp8266_dev *dev, char* data);

void WIFI_get_amount_data(esp8266_dev *dev, char *data, uint32_t amount);

int WIFI_get_all_data(esp8266_dev *dev, char *data);

int WIFI_get_data_terminator(esp8266_dev *dev, char *data, uint32_t max);

void WIFI_reset_FIFO(esp8266_dev *dev, uint32_t val);

#endif /* ESP_8266_H_ */
