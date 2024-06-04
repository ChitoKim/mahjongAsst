#include <mahjongAsst.h>
int button_mode[] =
{
  6, 7, 8, 9
};
int button_honba = 10;
float RES_AMOS_MONSTER[] =
{
  //in kiloohms
  20.0f, 100.0f, 1000.0f ,1000.0f
};
float R_REF[] =
{
  //in kiloohms
  10.0f, 10.0f, 10.0f, 10.0f
  // 2.2f, 10.0f, 100.0f, 100.0f
};

ADS1115 ADC0(0x48, &Wire);
ADS1115 ADC1(0x49, &Wire);
ADS1115 ADC2(0x4A, &Wire);
ADS1115 ADC3(0x4B, &Wire);

ADS1X15 *adc[] = {&ADC0, &ADC1, &ADC2, &ADC3};
mahjongAsst Asst(adc, RES_AMOS_MONSTER, R_REF);

float weight[] = {0.3f, 0.3f, 0.3f, 0.3f};
enum I2CPIN {
  _SDA0 = 4,
  _SCL0 = 5,
  _SDA1 = 2,
  _SCL1 = 3
};


void
setup()
{
  Asst.setNSlot(4);          //set number of slots; 5k 1k 100 (NSLOT = 3), or 5k 1k 100 100 (NSLOT = 4)

  Asst.setMesType(RES);     //choose measure type; resistance(RES) or CAP(capacitance)
  Asst.setPullType(PULLUP); //choose whether to pull up or down the reference resistors
                              //one of these: PULLUP, PULLDOWN, INPUT_PULLUP
                              
  // Asst.setOffset(200);    // uncomment to enable busting sticks
  Asst.setWeight(weight);
  // Asst.setModeButton(button_mode); // uncomment to enable mode buttons
  // Asst.setHonbaButton(button_honba); // uncomment to enable a honba button

  Wire.setSDA(_SDA0);
  Wire.setSCL(_SCL0);
  Wire.begin();

  Wire1.setSDA(_SDA1);
  Wire1.setSCL(_SCL1); // set I2C pins
  Wire1.begin(); //tweak pins_arduino.h for second I2C

  Asst.initExtADC();
  Asst.setExtADC(/*setGain*/ 1, /*ADC Resolution*/16, /*VCC*/3.3f);
  Asst.begin();
}
void
loop()
{
  int score[4] = {};
  int error[4] = {};
  Asst.loop(500);
  Asst.getScore(score);
  Asst.getError(error);
  // Now you have scores and errors.
  // Display them anyway you like
}