/*
 * ESP8266.c
 *
 *  Created on: Mar 30, 2017
 *      Author: neu
 */

#include <string.h>
#include <inttypes.h>
#include "io.h"
#include "esp8266.h"

esp8266_dev esp8266_inst(void *base) {
    esp8266_dev dev;
    dev.base = base;

    return dev;
}

/*
 * Returns the value in the CTRL register of the ESP8266 component.
 * name: WIFI_get_CTRL
 * @param dev  : The ESP8266 device struct.
 * @return The CTRL register value
 * 
 * example: uint32_t ctrl = WIFI_get_CTRL(&dev);
 * if(ctrl & WIFI_I_ENABLE_MASK == WIFI_I_ENABLE_RCV) //WIFI_I_ENABLE_RCV is on
 */
uint32_t WIFI_get_CTRL(esp8266_dev *dev) {
	return IORD_32DIRECT(dev->base, WIFI_CTRL_REG);
}

/*
 * Set the value of the CTRl register of the ESP8266 component.
 * name: WIFI_set_CTRL
 * @param dev  : The ESP8266 device struct.
 *        val  : the value to write to the CTRL register.
 * @return void
 * 
 * example: WIFI_set_CTRL(&dev, WIFI_UART_ON | WIFI_I_ENABLE_RCV | WIFI_STOP_0 | WIFI_ODD_PARITY);
 */
void WIFI_set_CTRL(esp8266_dev *dev, uint32_t val) {
	IOWR_32DIRECT(dev->base, WIFI_CTRL_REG, val);
}

/*
 * Returns the value of the UART_wait_cycles register of the ESP8266 component.
 * name: WIFI_get_baud_rate
 * @param dev  : The ESP8266 device struct.
 * @return The UART_wait_cycles register value
 *
 * example: baud_rate r = WIFI_get_baud_rate(&dev);
 * switch(r) {
 *   case b4800: break;//the baud rate is 4800 bits/s
 *   case b9600: break;//the baud rate is 9600 bits/s
 *   ...
 *   case b1382400: break;//the baud rate is 1382400 bits/s
 * }
 */
baud_rate WIFI_get_baud_rate(esp8266_dev *dev) {
	return IORD_32DIRECT(dev->base, WIFI_UART_WAIT_CYCLES);
}

/*
 * Set the value of the UART_wait_cycles register of the ESP8266 component.
 * name: WIFI_set_wait_cycles
 * @param dev  : The ESP8266 device struct,
 *        val  : the value to write to the UART_wait_cycles register.
 * @return void
 *
 * example: WIFI_set_baud_rate(&dev, b38400);
 */
void WIFI_set_baud_rate(esp8266_dev *dev, baud_rate rate) {
	IOWR_32DIRECT(dev->base, WIFI_UART_WAIT_CYCLES, rate);
}

/*
 * Returns the value of the STATUS register of the ESP8266 component
 * i.e. the i_pending bits.
 * name: WIFI_get_i_pending
 * @param dev  : The ESP8266 device struct.
 * @return the value of the status register.     
 * 
 * example: uint32_t i_pending = WIFI_get_i_pending(&dev);
 * if(i_pending & WIFI_I_PENDING_MASK == WIFI_I_PENDING_RCV | WIFI_I_PENDING_DROP)
 *     //There is an interrupt pending.
 */
uint32_t WIFI_get_i_pending(esp8266_dev *dev) {
	return IORD_32DIRECT(dev->base, WIFI_STATUS_REG);
}

/*
 * Clear the i_pending bits in the STATUS register of the ESP8266 component.
 * name: WIFI_clear_i_pending
 * @param dev  : The ESP8266 device struct.
 * @return void
 * 
 * example: WIFI_clear_i_pending(&dev);
 */
void WIFI_clear_i_pending(esp8266_dev *dev) {
	IOWR_32DIRECT(dev->base, WIFI_STATUS_REG, 0);
}

/*
 * Return the Amount of free space in the output FIFO
 * i.e. the number of bytes that can be send.
 * name: WIFI_get_free_space
 * @param dev  : The ESP8266 device struct.
 * @return the amount of free space in the FIFO_out.
 * 
 * example: uint32_t free_space = WIFI_get_free_space(&dev);
 * free_space = 1023 => 1023 words can be send to the FIFO_out.
 */
uint32_t WIFI_get_free_space(esp8266_dev *dev) {
	return IORD_32DIRECT(dev->base, WIFI_FIFO_OUT_FREE_SPACE);
}

/*
 * Send a single byte to output FIFO without any verification.
 * name: send_word
 * @param dev  : The ESP8266 device struct,
 *        word : the word to send
 * @return void
 * 
 * example: WIFI_send_word(&dev, 'A');
 * Sends the char 'A' to the ESP8266.
 * 
 * /!\
 * The data will be dropped if the FIFO is full.
 * ONLY USE WHEN YOU KNOW HOW MUCH SPACE IS AVAILABLE.
 */
void WIFI_send_word(esp8266_dev *dev, char word) {
	IOWR_8DIRECT(dev->base, WIFI_FIFO_OUT_DATA, word);
}

/*
 * Send a single byte to the output FIFO, checking before if it is possible
 * and returning the free space after the send.
 * name: WIFI_send_word_safe
 * @param dev  : The ESP8266 device struct,
 *        word : the word to send.
 * @return the amount of free space after the send
 *          or -1 if the send couldn't be done.
 * 
 * example: uint32_t free_space = WIFI_send_word_safe(&dev, 'A');
 * Sends the char 'A' to ESP8266. free_space = 1022 chars.
 */
int WIFI_send_word_safe(esp8266_dev *dev, char word) {
	uint32_t space = WIFI_get_free_space(dev);
	if(space == 0) {
		return -1;
	} else {
		WIFI_send_word(dev, word);
		return space -1;
	}
}

/*
 * Use this function when not in the AT mode.
 * Send a string to the output FIFO, performing a check on the free space before
 * and returning the free space after the send.
 * name: WIFI_send_message
 * @param dev     : The ESP8266 device struct,
 *        message : the message to send,
 *        length  : the length of the message to send
 * @return the amount free space after the send
 *          or -1 if the send couldn't be done.
 * 
 * example: uint32_t free_space = WIFI_send_message(&dev, "Hello you!!", 11);
 * Sends the message to the Bluetooth paired device. free_space = 1012 chars.
 */
int WIFI_send_message(esp8266_dev *dev, char* message, uint32_t length) {
	uint32_t space = WIFI_get_free_space(dev);
	if(length > space) {
		return -1;
	} else {
		for(uint32_t i = 0; i < length; ++i) {
			WIFI_send_word(dev, message[i]);
		}
		return space - (length);
	}
}

/*
 * Use this function when in the AT mode.
 * Same as send_message but add the "\r\n" string at the end of the message.
 * name: WIFI_send_command
 * @param dev     : The ESP8266 device struct,
 *        message : the message to send,
 *        length  : the length of the message to send
 * @return the amount free space after the send
 *          or -1 if the send couldn't be done.
 * 
 * example: uint32_t free_space = send_command(&dev, "AT+VERSION?", 11);
 * asks the module was is the version number. free_space = 1010 chars.
 */
int WIFI_send_command(esp8266_dev *dev, char* message, uint32_t length) {
	uint32_t space = WIFI_get_free_space(dev);
		if(length+2 > space) {
			return -1;
		} else {
			for(uint32_t i = 0; i < length; ++i) {
				WIFI_send_word(dev, message[i]);
			}
			WIFI_send_word(dev, '\r');
			WIFI_send_word(dev, '\n');
			return space - (length+2);
		}
}

/*
 * Get the value of the pending data register
 * i.e. the number of byte waiting to be read.
 * name: WIFI_get_pending_data
 * @param dev  : The ESP8266 device struct.
 * @return the amount of word that can be read from the ESP8266 extension.
 * 
 * example: uint32_t pending = WIFI_get_pending_data(&dev);
 * pending = 4 => 4 words waiting in the FIFO.
 */
uint32_t WIFI_get_pending_data(esp8266_dev *dev) {
	return IORD_32DIRECT(dev->base, WIFI_FIFO_IN_PENDING_DATA);
}

/*
 * Wait until at least amount words are waiting in the FIFO_IN.
 * name: wait_for_data
 * @param dev  : The ESP8266 device struct.
 * 		  amount : The minimum amount of words expected.
 * @return the amount of word that can be read from the ESP8266 extension.
 *
 * example: uint32_t pending = WIFI_wait_for_data(&dev, 2);
 * pending will never be less than 3.
 * pending = 4 => 4 words waiting in the FIFO.
 */
uint32_t WIFI_wait_for_data(esp8266_dev *dev, uint32_t amount) {
	uint32_t pending = 0;
	while((pending = WIFI_get_pending_data(dev)) < amount) {}
	return pending;
}
/*
 * Get a single byte from the input FIFO without performing a check.
 * name: WIFI_get_data
 * @param dev  : The ESP8266 device struct.
 * @return the char read from the component.
 * 
 * example: char c = WIFI_get_data(&dev);
 * c = char read.
 * 
 * /!\
 * There is no guarantee on the data in c if the FIFO was empty.
 * ONLY USE WHEN YOU KNOW HOW MUCH DATA IS WAITING.
 */
char WIFI_get_data(esp8266_dev *dev) {
    return IORD_32DIRECT(dev->base, WIFI_FIFO_IN_DATA);
}

/*
 * Get a single byte from the input FIFO, performing a check before
 * and returning the amount of pending data after the read.
 * name: WIFI_get_data_safe
 * @param dev  : The ESP8266 device struct,
 *        data : a pointer to the data container.
 * @return the amount of waiting data after the read
 *          or -1 if there was nothing to read.
 * 
 * example: char c; int pending = get_data_safe(&dev, &c);
 * pending = amount of char still waiting, c = char read.
 */
int WIFI_get_data_safe(esp8266_dev *dev, char* data) {
	uint32_t pend = WIFI_get_pending_data(dev);
	if(pend == 0) {
		return -1;
	} else {
		*data = IORD_32DIRECT(dev->base, WIFI_FIFO_IN_DATA);
		return pend -1;
	}
}

/*
 * Get a specified amount of data waiting in the input FIFO.
 * name: WIFI_get_amount_data
 * @param dev  : The ESP8266 device struct,
 *        data : a pointer to a char array that will contain the data.
 * 		  amount : the amount of data to read in the FIFO.
 * @return the amount of char read or -1 if there was no data pending.
 *
 * example: char data[100];
 * WIFI_get_amount_data(&dev, data, 5);
 * reads 5 chars from the FIFO_IN.
 *
 * /!\
 * There is no guarantee on the data in data if the FIFO was empty
 * or had less than amount elements.
 * ONLY USE WHEN YOU KNOW HOW MUCH DATA IS WAITING.
 */
void WIFI_get_amount_data(esp8266_dev *dev, char *data, uint32_t amount) {
	char c;
    for(uint32_t i = 0; i < amount; ++i) {
    	c = WIFI_get_data(dev);
        data[i] = c;
    }
    data[amount] = '\0';
}

/*
 * Get all the pending data waiting in the input FIFO.
 * name: get_all_data
 * @param dev  : The ESP8266 device struct,
 *        data : a pointer to a char array that will contain the data.
 * @return the amount of char read or -1 if there was no data pending.
 * 
 * example: char data[100]; int read = WIFI_get_all_data(&dev, data);
 * read = amout of char read, data[0..read-1] = data received
 */
int WIFI_get_all_data(esp8266_dev *dev, char *data) {
    uint32_t pend = WIFI_get_pending_data(dev);
    if(pend == 0) {
    	data[0] = '\0';
        return -1;
    } else {
    	WIFI_get_amount_data(dev, data, pend);
        return pend;
    }
}

/*
 * Keep reading the FIFO_IN until "\r\n" is read, this is the response terminator for commands.
 * name: WIFI_get_data_terminator
 * @param dev  : The ESP8266 device struct,
 *        data : a pointer to a char array that will contain the data.
 * @return the amount of char read.
 *
 * example: char data[100]; int read = WIFI_get_data_terminator(&dev, data);
 * read = amout of char read, data[0..read-1] = data received
 */
int WIFI_get_data_terminator(esp8266_dev *dev, char *data, uint32_t max) {
	int r = 0;
	char c = '0', last = '0', tmp = '0';
	for(int i = 0; i < max && (last != '\r' || c != '\n'); ++i) {
		while(WIFI_get_data_safe(dev, &tmp) < 0) {}
		last = c;
		c = tmp;
		data[r] = tmp;
		++r;
	}
	data[r] = '\0';
	return r;
}

/*
 * Clear the specified FIFO.
 * name: WIFI_reset_FIFO
 * @param dev  : The ESP8266 device struct.
 *        val  : the value corresponding to the FIFO(s) to reset.
 * @return void
 *
 * example: WIFI_reset_FIFO(&dev, WIFI_RESET_FIFO_IN | WIFI_RESET_FIFO_OUT);
 * resets both FIFOs
 */
void WIFI_reset_FIFO(esp8266_dev *dev, uint32_t val) {
	IOWR_32DIRECT(dev->base, WIFI_RESET_FIFO, val);
}
