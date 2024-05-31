/*
  Special Thanks to Mahjong

  mahjongAsst Library - mahjongAsst.h
  A Library for legacy scoring system of Japanese mahjong tables.
  Legacy mahjong scorers implement special score sticks containing electrical elements such as R, L, or C.
  This library measures parallel resistances/capacitances of stack-piled score sticks,
  and convert the values into actual scores of 4 mahjong players.

 *  Copyright (c) 2023, Chito Kim
 *  All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 
  Codes for capacitance measurement is based on a guide of Circuit Basics,
  and library "arduino-capacitor" written by Jonathan Nethercott.
  https://www.circuitbasics.com/how-to-make-an-arduino-capacitance-meter/
  The core idea of using internal pull-ups and capacitance calculation through
  logarithm is first designed by Jonathan Nethercott.
  https://github.com/codewrite/arduino-capacitor
*/


#ifndef _MAHJONGASST_H
#define _MAHJONGASST_H

#if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_RP2040) || \
defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_ESP32)  || \
defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32)
#define ADC_RESOLUTION_MUTABLE
#endif

#include  "components.h"
#include  "MUX.h"
#define   DEFAULT_ADC_MAX  1024
#define   DEFAULT_NSLOT    4
#define   DEFAULT_NUMPIN   16
#define   DEFAULT_HONBA    0
#define   MAXSTICK         50
#define   MAXSTICK_100P    50

#define   RES             0
#define   CAP             1
#define   PIN_NONE       -1

#ifndef   PULLUP
#define   PULLUP          5
#endif

#ifndef   PULLDOWN
#define   PULLDOWN        -1
#endif
//INPUT_PULLUP

#define  MAXTIME          sizeof(unsigned long)

#define  NORMAL           1
#define  DIFF             0
#define  PM               2
#define  SUM              1000

class mahjongAsst
{
private:
  MUX *mux_p;
  ENV *env_p;
  PIN *pin_p;
  VAL *val_p;
public:
  mahjongAsst(MUX *mux, ENV *env, PIN *pin, VAL *val);
  mahjongAsst(int charge[], int analog[], float v_unit[], float ref[]);
  mahjongAsst(int charge[], int analog, float v_unit[], float ref[]);
  mahjongAsst(int analog, float v_unit[], float ref[]);
  mahjongAsst(int analog[], float v_unit[], float ref[]);
  MUX*  getMUX();
  ENV*  getENV();
  PIN*  getPIN();
  VAL*  getVAL();
  void  setMUX4051(int a[], int b[]);
  void  setMUX4067(int a[]);
  void  initMUX();
  void  slotSelect(int slot_num);
  void  setNSlot(int a);
  void  setPullType(int a);
  void  setMesType(int a);
  void  setModeButton(int a[]);
  void  setHonbaButton(int a);
  void  setADCResolution(int a);
  void  setWeight(float a[]);
  void  setOffset(int a);

  void  getScore(int scr[]);
  int   *getScore();
  void  getError(int err[]);
  int   *getError();
  void  getMode(int mode[]);
  int   *getMode();
  int   getHonba();

  int   boolRead(int pin);
  int   adcRead(int pin);
  void  pullAnalog(int apin);
  
  void  mesLoop(float RLC[]);
  void  numLoop(float RLC[], int num[]);
  void  scoreLoop(int num[]);
  void  modeLoop();
  void  loop();
  void  loop(int period_ms);
  void  loop(float RLC[], int num[]);

  void  begin();  
  void  prepMes(int slot_num);
  float mesRLC(int slot_num);
  int   RLCToNum(float RLC, int slot_num);

  //resistance specific////
  ////
  float adcToRes(int adc, float r);
  //capacitance specific///
  ////
  void  setParRes(float f[]);
  int   hasParRes(float f);
  float adcToCap(unsigned long t , int adc, float r, float c_unit, float r_par);
  void  discharge(int cpin, int apin);
  void  charge(int cpin);
  //
};
void _HONBA();
#endif