/*
 * UserMain.h
 *
 *  Created on: Jan 11, 2021
 *      Author: huang
 */


#ifndef _USERMAIN_H_
#define _USERMAIN_H_

#include "main.h"
#include "sys.h"
#include <stdint.h>
#include <stdio.h>

#define LED0_Pin GPIO_PIN_8
#define LED0_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_2
#define LED1_GPIO_Port GPIOD

void UserSysInit();

void UserLoop();

void UserConfigInit();

void UserErrorHandler();
#endif /* _USERMAIN_H_ */
