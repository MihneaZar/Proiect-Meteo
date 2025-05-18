/** 
 * --------------------------------------------------------------------------------------+
 * @desc        Two Wire Interface / I2C Communication
 * --------------------------------------------------------------------------------------+
 *              Copyright (C) 2020 Marian Hrinko.
 *              Written by Marian Hrinko (mato.hrinko@gmail.com)
 *
 * @author      Marian Hrinko
 * @datum       06.09.2020
 * @file        twi.c
 * @tested      AVR Atmega16, ATmega8, Atmega328
 *
 * @depend      twi.h
 * --------------------------------------------------------------------------------------+
 * @usage       Master Transmit Operation
 */
 
// include libraries
#include "twi.h"
#include <stdio.h>

extern uint8_t TWI_ErrorCode;

/**
 * @desc    TWI init - initialize frequency
 *
 * @param   void
 *
 * @return  void
 */
void TWI_Init (void)
{
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Calculation fclk:
  //
  // fclk = (fcpu)/(16+2*TWBR*4^Prescaler) m16
  // fclk = (fcpu)/(16+2*TWBR*Prescaler) m328p
  // -------------------------------------------------------------------------------------
  // Calculation TWBR:
  // 
  // TWBR = {(fcpu/fclk) - 16 } / (2*4^Prescaler)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // @param1 value of TWBR (m16) 
  //  fclk = 400kHz; TWBR = 3
  //  fclk = 100kHz; TWBR = 20
  // @param1 value of TWBR (m328p)
  //  fclk = 400kHz; TWBR = 2
  // @param2 value of Prescaler = 1
  TWI_FREQ (2, 1);
}

/**
 * @desc    TWI MT Start
 *
 * @param   void
 *
 * @return  char
 */
char TWI_MT_Start (void)  
{
  // null status flag
  TWI_TWSR &= ~0xA8;
  // START
  // -------------------------------------------------------------------------------------
  // request for bus
  TWI_START();
  // wait till flag set
  TWI_WAIT_TILL_TWINT_IS_SET();
  // test if start or repeated start acknowledged
  if ((TWI_STATUS != TWI_START_ACK) && (TWI_STATUS != TWI_REP_START_ACK)) {
    // return status
    return TWI_STATUS;
  }
  // success
  return SUCCESS;
}

void showbits (int n)
{
  int i, k, andmask;

  for (i = 15; i >= 0;i--)
  {
    andmask = 1 << i;
    k = n & andmask;
    k == 0 ? printf ("0") : printf ("1");
  }
  printf("\n");
} 

char TWI_MT_Start_SLAW(char address) {
  uint8_t status = SUCCESS;

  // TWI: start
  // -------------------------------------------------------------------------------------
  status = TWI_MT_Start();
  if (status != SUCCESS) {
    return status;
  }
  TWDR = address;
  // enable
  TWI_ENABLE();
  // wait till flag set
  TWI_WAIT_TILL_TWINT_IS_SET();
  // status = TWI_MT_Send_SLAW(address);
  // printf("%04x\n", status);
  return status;
}

/**
 * @desc    TWI Send address + write
 *
 * @param   char
 *
 * @return  char
 */
char TWI_MT_Send_SLAW (char address)
{
  // SLA+W
  // -------------------------------------------------------------------------------------
  TWI_TWDR = (address << 1);
  // enable
  TWI_ENABLE();
  // wait till flag set
  TWI_WAIT_TILL_TWINT_IS_SET();

  // test if SLA with WRITE acknowledged
  if (TWI_STATUS != TWI_MT_SLAW_ACK) {
    // return status
    return TWI_STATUS;
  }
  // success
  return SUCCESS;
}

/**
 * @desc    TWI Send data
 *
 * @param   char
 *
 * @return  char
 */
char TWI_MT_Send_Data (char data)
{
  // DATA
  // -------------------------------------------------------------------------------------
  TWI_TWDR = data;
  // enable
  TWI_ENABLE();
  // wait till flag set
  TWI_WAIT_TILL_TWINT_IS_SET();

  // test if data acknowledged
  if (TWI_STATUS != TWI_MT_DATA_ACK) {
    // return status
    return TWI_STATUS;
  }
  // success
  return SUCCESS;
}

/**
 * @desc    TWI Send address + read
 *
 * @param   char
 *
 * @return  char
 */
char TWI_MR_Send_SLAR (char address)
{
  // SLA+R
  // -------------------------------------------------------------------------------------
  TWI_TWDR = (address << 1) | 0x01;
  // enable
  TWI_ENABLE();
  // wait till flag set
  TWI_WAIT_TILL_TWINT_IS_SET();

  // test if SLA with READ acknowledged
  if (TWI_STATUS != TWI_MR_SLAR_ACK) {
    // return status
    return TWI_STATUS;
  }
  // success
  return SUCCESS;
}

/**********************************************
 Public Function: TWI_readAck
 
 Purpose: read acknowledge from TWI/I2C Interface
 
 Input Parameter: none
 
 Return Value: uint8_t
  - TWDR: recieved value at TWI/I2C-Interface, 0 at timeout
  - 0:    Error at read
 **********************************************/
uint8_t TWI_readAck(void){
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    uint16_t timeout = F_CPU/F_TWI*2.0;
    while((TWCR & (1 << TWINT)) == 0 &&
		  timeout !=0){
		timeout--;
		if(timeout == 0){
			// TWI_ErrorCode |= (1 << TWI_READACK);
			return 0;
		}
	};
  return TWDR;
}

 /**********************************************
 Public Function: TWI_readNAck
 
 Purpose: read non-acknowledge from TWI/I2C Interface
 
 Input Parameter: none
 
 Return Value: uint8_t
  - TWDR: recieved value at TWI/I2C-Interface
  - 0:    Error at read
 **********************************************/
uint8_t TWI_readNAck(void){
    TWCR = (1<<TWINT)|(1<<TWEN);
    uint16_t timeout = F_CPU/F_TWI*2.0;
    while((TWCR & (1 << TWINT)) == 0 &&
		  timeout !=0){
		timeout--;
		if(timeout == 0){
			// TWI_ErrorCode |= (1 << TWI_READNACK);
      return 0;
		}
	};
  return TWDR;
}

/**
 * @desc    TWI stop
 *
 * @param   void
 *
 * @return  void
 */
void TWI_Stop (void)
{
  // End TWI
  // -------------------------------------------------------------------------------------
  // send stop sequence
  TWI_STOP ();
  // wait for TWINT flag is set
//  TWI_WAIT_TILL_TWINT_IS_SET();
}
