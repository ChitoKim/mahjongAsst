/*
  Special Thanks to Mahjong

  mahjongAsst Library - mahjongAsst.h
  A Library for legacy scoring system of Japanese mahjong tables.
  Legacy mahjong scorers implement special score sticks containing electrical elements such as R, L, or C.
  This library measures parallel resistances/capacitances of stack-piled score sticks,
  and convert the values into actual scores of 4 mahjong players.

 *  Copyright (c) 2023, Chito Kim
 *  All rights reserslot.
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
#include  "mahjongAsst.h"
#include  <Arduino.h>

volatile int _button_honba = PIN_NONE;
volatile int *_honba =  nullptr;
volatile unsigned long _press_t = 0L;
MUX NO_MUX(nullptr, 0, nullptr, 0); 
ENV DEFAULT_ENV = {DEFAULT_NSLOT, DEFAULT_NUMPIN, PULLDOWN, RES, DEFAULT_ADC_MAX};
PIN DEFAULT_PIN = 
{{PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE,
  PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE,
  PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE,
  PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE},
  {PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE,
  PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE,
  PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE,
  PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE}, 
  {PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE},
  PIN_NONE,
  {PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE},
  {PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE},
  {PIN_NONE, PIN_NONE, PIN_NONE, PIN_NONE},
  {0.3f, 0.3f, 0.3f, 0.3f}
};

VAL DEFAULT_VAL = {{0, 0, 0, 0}, {0, 0, 0, 0}, {NORMAL, NORMAL, NORMAL, NORMAL}, DEFAULT_HONBA, 0, 0L};

mahjongAsst::mahjongAsst(MUX *mux, ENV *env, PIN *pin, VAL *val)
{
  mux_p = mux;
  env_p = env;
  pin_p = pin;
  val_p = val;
}
mahjongAsst::mahjongAsst(int charge[], int analog[], float v_unit[], float ref[])
: mahjongAsst(&NO_MUX, &DEFAULT_ENV, &DEFAULT_PIN, &DEFAULT_VAL)
{
  memcpy(pin_p->charge_pin, charge, 16 * sizeof(int));
  memcpy(pin_p->analog_pin, analog, 16 * sizeof(int));
  memcpy(pin_p->RLC_per_unit, v_unit, 4 * sizeof(float));
  memcpy(pin_p->R_REF, ref, 4 * sizeof(float)); 
}
mahjongAsst::mahjongAsst(int analog[], float v_unit[], float ref[]) 
: mahjongAsst(&NO_MUX, &DEFAULT_ENV, &DEFAULT_PIN, &DEFAULT_VAL)
{
  memcpy(pin_p->analog_pin, analog, 16 * sizeof(int));
  memcpy(pin_p->RLC_per_unit, v_unit, 4 * sizeof(float));
  memcpy(pin_p->R_REF, ref, 4 * sizeof(float)); 
}
mahjongAsst::mahjongAsst(int charge[], int analog, float v_unit[], float ref[])
: mahjongAsst(&NO_MUX, &DEFAULT_ENV, &DEFAULT_PIN, &DEFAULT_VAL)
{
  int i;
  memcpy(pin_p->charge_pin, charge, 16 * sizeof(int));
  for(i = 0; i < 16; i++)
  {
    (pin_p->analog_pin)[i] = analog;
  }
  memcpy(pin_p->RLC_per_unit, v_unit, 4 * sizeof(float));
  memcpy(pin_p->R_REF, ref, 4 * sizeof(float)); 
}
mahjongAsst::mahjongAsst(int analog, float v_unit[], float ref[])
: mahjongAsst(&NO_MUX, &DEFAULT_ENV, &DEFAULT_PIN, &DEFAULT_VAL)
{
  int i;
  for(i = 0; i < 16; i++)
  {
    (pin_p->analog_pin)[i] = analog;
  }
  memcpy(pin_p->RLC_per_unit, v_unit, 4 * sizeof(float));
  memcpy(pin_p->R_REF, ref, 4 * sizeof(float)); 
}
MUX*
mahjongAsst::getMUX()
{
  return mux_p;
}
ENV*
mahjongAsst::getENV()
{
  return env_p;
}
PIN*
mahjongAsst::getPIN()
{
  return pin_p;
}
VAL*
mahjongAsst::getVAL()
{
  return val_p;
}
void
mahjongAsst::setMUX4051(int addr[], int enb[])
{
  mux_p->setMUX4051(addr, enb);
}
void
mahjongAsst::setMUX4067(int addr[])
{
  mux_p->setMUX4067(addr);
}
void
mahjongAsst::initMUX()
{
  //initialise the pins of MUX
  mux_p->initMUX();
}
void
mahjongAsst::slotSelect(int slot_num)
{
  //switching to corresponding slot of MUX(es)
  mux_p->slotSelect(slot_num);
}
void
mahjongAsst::setNSlot(int a)
{
  //set the number of slots; 5k 1k 5c 1c or 5k 1k 5c/1c 
  if(a > 4)
  {
    return;
  }
  env_p->NSLOT = a;
  env_p->NUMPIN = 4 * a;
  for(int i = env_p->NUMPIN; i < 16; i++)
  {
    (pin_p->charge_pin)[i] = PIN_NONE;
    (pin_p->analog_pin)[i] = PIN_NONE;
  }
}
void
mahjongAsst::setPullType(int a)
{
  //set pull type of resistor
  //PULLUP | INPUT_PULLUP | PULLDOWN 
  env_p->pull_type = a;
}
void
mahjongAsst::setMesType(int a)
{
  env_p->mes_type = a;
}
void
mahjongAsst::setADCResolution(int bit)
{
  //set ADC_MAX according to the ADC Resolution
  env_p->ADC_MAX = 1 << bit;
#ifdef ADC_RESOLUTION_MUTABLE
  analogReadResolution(bit);
#endif
}
void
mahjongAsst::setWeight(float weight[])
{
  memcpy(pin_p->weight, weight, 4 * sizeof(int));
}
void
mahjongAsst::setOffset(int offset)
{
  val_p->bust_offset = offset;
}
void
mahjongAsst::setModeButton(int btn[])
{
  int i;
  memcpy(pin_p->button_mode, btn, 4 * sizeof(int));
  for(i = 0 ; i < 4; i++)
  {
    pinMode(btn[i], INPUT_PULLUP);
  }
}
void
mahjongAsst::setHonbaButton(int btn)
{
  //set a honba button, attached to interrupt
  pin_p->button_honba = btn;
  _button_honba = btn;
  _honba = &(val_p->honba);
  pinMode(btn, INPUT_PULLUP);
  attachInterrupt(btn, _HONBA, CHANGE);
}
void
mahjongAsst::getScore(int scr[])
{
  //copy the scores to the array scr
  memcpy(scr, val_p->score, 4 * sizeof(int));
}
int
*mahjongAsst::getScore()
{
  return val_p->score;
}

void
mahjongAsst::getError(int err[])
{
  //copy the errors to the array scr
  int i;
  int *error = val_p->error;
  memcpy(err, val_p->error, 4 * sizeof(int));
}
int
*mahjongAsst::getError()
{
  return val_p->error;
}
void
mahjongAsst::getMode(int mode[])
{
  memcpy(mode, val_p->mode, 4 * sizeof(int));
}
int
*mahjongAsst::getMode()
{
  return val_p->mode;
}
int
mahjongAsst::getHonba()
{
  return val_p->honba;
}
int
mahjongAsst::boolRead(int pin)
{
  //digitalRead, if PULLDOWN, invert the output
  int pull_type = env_p->pull_type;
  if(pull_type == INPUT_PULLUP || pull_type == PULLUP)
  {
    return (digitalRead(pin));
  }
  else
  {
    return (!digitalRead(pin));
  }
}
int
mahjongAsst::adcRead(int pin)
{
  //analogRead, if PULLDOWN, invert the output
  int pull_type = env_p->pull_type;
  int ADC_MAX = env_p->ADC_MAX;
  if(pull_type == INPUT_PULLUP || pull_type == PULLUP)
  {
    return (analogRead(pin));
  }
  else
  {
    return (ADC_MAX - analogRead(pin));
  }
}
void
mahjongAsst::pullAnalog(int apin)
{
  //setting pinMode according to the pulltype
  int pin_mode = (env_p->pull_type == INPUT_PULLUP) ? INPUT_PULLUP : INPUT;
  pinMode(apin, pin_mode);
}

void
mahjongAsst::mesLoop(float RLC[])
{
 //store R or C in the array RLC
  int i;
  for(i = 0; i < env_p->NUMPIN; i++) 
  {
    RLC[i] = 0;
    prepMes(i);
    RLC[i] = mesRLC(i);
  }
}
void
mahjongAsst::numLoop(float RLC[], int num[])
{
 //store number of sticks in the array num
  int i;
  for(i = 0; i < env_p->NUMPIN; i++) 
  { 
    num[i] = 0;
    num[i] = RLCToNum(RLC[i], i);
    if((i % env_p->NSLOT) < 2)
      num[i] = (num[i] > MAXSTICK) ? -1 : num[i];
    else
      num[i] = (num[i] > MAXSTICK_100P) ? -1 : num[i];
  }
}
void
mahjongAsst::scoreLoop(int num[])
{
 //finally outputing the score
  int i;
  int NSLOT = env_p->NSLOT;
  int *error = val_p->error;
  int *score = val_p->score;
  int offset = val_p->bust_offset;
  for(i = 0; i < 4; i++)
  {
    score[i] = 0;
    error[i] = false;
    if(NSLOT == 3)
    {
      error[i] = num[3 * i] < 0 || num[3 * i + 1] < 0 || num[3 * i + 2] < 0;
      score[i] = 50 * num[3 * i] + 10 * num[3 * i + 1] + 1 * num[3 * i + 2];
      score[i] -= offset;
    }
    else if(NSLOT == 4) 
    {
      error[i] = num[4 * i] < 0 || num[4 * i + 1] < 0 || num[4 * i + 2] < 0 || num[4 * i + 3] < 0;
      score[i] = 50 * num[4 * i] + 10 * num[4 * i + 1] + 1 * num[4 * i + 2] + 1 * num[4 * i + 3];
      score[i] -= offset;
    }
  }
}
void
mahjongAsst::modeLoop()
{
  int i, tmp;
  int *button_mode = pin_p->button_mode;
  int *mode = val_p->mode;
  if(button_mode[0] == PIN_NONE)
  {
    return;
  }
  for(i = 0; i < 4; i++)
  {
    tmp = mode[i];
    mode[i] = (digitalRead(button_mode[i])) ? NORMAL : DIFF;
    if(tmp == DIFF && mode[i] == NORMAL)
      mode[i] = PM;
  }
}
void
mahjongAsst::loop()
{
  float RLC[16];
  int   num[16];
  modeLoop();
  mesLoop(RLC);
  numLoop(RLC, num);
  scoreLoop(num);
}
void
mahjongAsst::loop(int period_ms)
{
  unsigned long currentTime, dt;
  currentTime = millis();
  dt = currentTime - val_p->lastTime; 
  if(dt > period_ms)
  {
    this->loop();
    val_p->lastTime = currentTime;
  }
}
void
mahjongAsst::loop(float RLC[], int num[])
{
  modeLoop();
  mesLoop(RLC);
  numLoop(RLC, num);
  scoreLoop(num);
}
//
///// above : common functions
void
mahjongAsst::begin()
{
  int i;
  int NUMPIN = env_p->NSLOT;
  int mes_type = env_p->mes_type;

  initMUX();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}
void
mahjongAsst::prepMes(int slot_num)
{
  int mes_type = env_p->mes_type;
  int cpin = (pin_p->charge_pin)[slot_num];
  int apin = (pin_p->analog_pin)[slot_num];
  switch(mes_type)
  {
    case RES:
      pinMode(apin, OUTPUT);
      delay(1);
      slotSelect(slot_num);
      delay(1);
      break;
    case CAP:
      slotSelect(slot_num);
      discharge(cpin, apin);
      break;
    default:
      break;
  }
}
float
mahjongAsst::mesRLC(int slot_num)
{
  int NSLOT = env_p->NSLOT;
  int pull_type = env_p->pull_type;
  int mes_type = env_p->mes_type;

  int cpin = (pin_p->charge_pin)[slot_num];
  int apin = (pin_p->analog_pin)[slot_num];

  int adc, dig_val;
  float val_unit = (pin_p->val_per_unit)[slot_num % NSLOT];
  float r_ref = (pin_p->R_REF)[slot_num % NSLOT];
  float r_par = (pin_p->R_REF)[slot_num % NSLOT];
  float RC;
  unsigned long t, tf, dt, discharge_t;
  switch(mes_type)
  {
    case RES:
      pullAnalog(apin);
      delay(1);
      pinMode(apin, INPUT);
      adc = adcRead(apin);
      RC = adcToRes(adc, r_ref); // read resistor voltage and calculate resistance
      break;
    case CAP:
      charge(cpin);
      t = micros();
      pullAnalog(apin);
      //start charging
      do 
      {
        dig_val = boolRead(apin);
        tf = micros();
        dt = (tf > t) ? tf - t : MAXTIME - t + tf;
      } while(!dig_val && dt < 1000000L); //measure time until charged
      pinMode(apin, INPUT);
      adc = adcRead(apin);   
      RC = adcToCap(dt, adc, r_ref, val_unit, r_par); //read capacitor voltage and calculate capacitance
      discharge_t = 5L * dt / 1000L;
      if(pull_type == INPUT_PULLUP)
      {
        digitalWrite(cpin, HIGH); 
        pinMode(apin, INPUT_PULLUP); //HIGH to HIGH with pullup, discharge the capacitor
        delay(discharge_t);
        discharge(cpin, apin);
      }
      else
      {
        discharge(cpin, apin);
      }
      break;
    default:
      return -1.0;
      break;
  }
  return RC;
}
int
mahjongAsst::RLCToNum(float RLC, int slot_num)
{
  int   i = 0, num = 0;
  int   NSLOT = env_p->NSLOT;
  float ratio = -2.0f;
  float weight = (pin_p->weight)[slot_num % NSLOT];
  float RLC_unit = (pin_p->RLC_per_unit)[slot_num % NSLOT];
  float r_par = (pin_p->R_PAR)[slot_num % NSLOT];
  float r_ref = (pin_p->R_REF)[slot_num % NSLOT];
  switch(env_p->mes_type)
  {
    case RES:
      ratio = RLC_unit / RLC;
      num = (int) (ratio + weight);
      if(ratio < 0.8f) 
      {
        num = 0;
      }
    break;
    case CAP:
      ratio = RLC / RLC_unit;
      if(hasParRes(r_par))
      {
        ratio = correctCap(ratio, r_par, r_ref);
      }
      num = (int) ratio;
      if(num == 0 && ratio > 0.8f)
      {
        num = 1;
      }
      break;
    default:
      break;
  }
  return num;
}

//resistance specific
//////
float
mahjongAsst::adcToRes(int adc, float ref)
{
//calculate resistance
  return (float) adc * ref / (float) (env_p->ADC_MAX - adc);
}
//capacitance specific
/////

void
mahjongAsst::setParRes(float r_par[])
{
  memcpy(pin_p->R_PAR, r_par, 4 * sizeof(float));
}
int
mahjongAsst::hasParRes(float f)
{
  return (f > 0.0);
  //return false because by default parres equals to PIN_NONE
}
float
mahjongAsst::adcToCap(unsigned long t , int adc, float r_ref, float c_unit, float r_par = PIN_NONE)
{
//calculate capacitance using charge time and capacitor voltage
  float k = 0;
  float alpha = 1;
  float vRatio = (float) adc / (float) env_p->ADC_MAX;
  int i;
  
  if(r_par < 0)
  {
    return - (float) t / r_ref / log(1.0f - vRatio);
  }
  for(i = 0; i < 50; i++)
  {
    k = - ((float) t) / (alpha * r_ref * c_unit * log(1.0f - vRatio / alpha));
    alpha = r_par / (k * r_ref + r_par);
  }
  return k * c_unit;
}
void
mahjongAsst::discharge(int cpin, int apin)
{
  pinMode(cpin, OUTPUT);
  digitalWrite(cpin, LOW);
  pinMode(apin, OUTPUT);
  digitalWrite(apin, LOW);
  while(adcRead(apin)) ;
}
void
mahjongAsst::charge(int cpin)
{
  pinMode(cpin, OUTPUT);
  digitalWrite(cpin, (env_p->pull_type == INPUT_PULLUP || env_p->pull_type == PULLUP) ? LOW : HIGH);
}
void
_HONBA()
{
  if(digitalRead(_button_honba) == HIGH) //released
  {
    if(millis() - _press_t > 1000L) //if pressed long, reset honba
    {
      *_honba = 0;
    }
    else if(millis() - _press_t > 50L) // if pressed short, increment honba
    {
      (*_honba)++;
    }
  }
  else //pressed 
  {
    _press_t = millis();
  }
}