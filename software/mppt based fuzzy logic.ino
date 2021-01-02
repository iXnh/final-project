#include <Fuzzy.h>
Fuzzy *fuzzy = new Fuzzy();

// FuzzyInput delta P (V*I)baru-(V*I)lama
FuzzySet *slopeN = new FuzzySet(-10, -10, -8, 0);
FuzzySet *slopeZ = new FuzzySet(-2, 0, 0, 2);
FuzzySet *slopeP = new FuzzySet(0, 8, 10, 10);

// FuzzyInput delta Tegangan (V)baru-(V)lama
FuzzySet *dSlopeN = new FuzzySet(-2, -2, -1, 0);
FuzzySet *dSlopeZ = new FuzzySet(-1, 0, 0, 1);
FuzzySet *dSlopeP = new FuzzySet(0, 1, 2, 2);

// FuzzyOutput, naik turun nilai PWM
FuzzySet *dN = new FuzzySet(-3, -3, -1, -1);
FuzzySet *dZ = new FuzzySet(-1, 0, 0, 1);
FuzzySet *dP = new FuzzySet(1, 1, 3, 3);

float pwmA;
int pwm1;

float slope,deltaSlope,slopeL;


void setup()
{
  Serial.begin(9600);

  // FuzzyInput delta Power
  FuzzyInput *dPower = new FuzzyInput(1);
  dPower->addFuzzySet(slopeN);
  dPower->addFuzzySet(slopeZ);
  dPower->addFuzzySet(slopeP);
  fuzzy->addFuzzyInput(dPower);

  // FuzzyInput delta tegangan
  FuzzyInput *dVolt = new FuzzyInput(2);
  dVolt->addFuzzySet(dSlopeN);
  dVolt->addFuzzySet(dSlopeZ);
  dVolt->addFuzzySet(dSlopeP);
  fuzzy->addFuzzyInput(dVolt);

  // FuzzyOutput kenaikan PWM
  FuzzyOutput *slopePWM = new FuzzyOutput(1);
  slopePWM->addFuzzySet(dN);
  slopePWM->addFuzzySet(dZ);
  slopePWM->addFuzzySet(dP);
  fuzzy->addFuzzyOutput(slopePWM);

  // FuzzyRule #1
  FuzzyRuleAntecedent *if_dSlopeNslopeN = new FuzzyRuleAntecedent();
  if_dSlopeNslopeN->joinWithAND(dSlopeN, slopeN);
  // FuzzyRule #2
  FuzzyRuleAntecedent *if_dSlopeNslopeZ = new FuzzyRuleAntecedent();
  if_dSlopeNslopeZ->joinWithAND(dSlopeN, slopeZ);
  // FuzzyRule #3
  FuzzyRuleAntecedent *if_dSlopeNslopeP = new FuzzyRuleAntecedent();
  if_dSlopeNslopeP->joinWithAND(dSlopeN, slopeP);
  // FuzzyRule #4
  FuzzyRuleAntecedent *if_dSlopeZslopeN = new FuzzyRuleAntecedent();
  if_dSlopeZslopeN->joinWithAND(dSlopeZ, slopeN);
  // FuzzyRule #5
  FuzzyRuleAntecedent *if_dSlopeZslopeZ = new FuzzyRuleAntecedent();
  if_dSlopeZslopeZ->joinWithAND(dSlopeZ, slopeZ);
  // FuzzyRule #6
  FuzzyRuleAntecedent *if_dSlopeZslopeP = new FuzzyRuleAntecedent();
  if_dSlopeZslopeP->joinWithAND(dSlopeZ, slopeP);
  // FuzzyRule #7
  FuzzyRuleAntecedent *if_dSlopePslopeN = new FuzzyRuleAntecedent();
  if_dSlopePslopeN->joinWithAND(dSlopeP, slopeN);
  // FuzzyRule #8
  FuzzyRuleAntecedent *if_dSlopePslopeZ = new FuzzyRuleAntecedent();
  if_dSlopePslopeZ->joinWithAND(dSlopeP, slopeZ);
  // FuzzyRule #9
  FuzzyRuleAntecedent *if_dSlopePslopeP = new FuzzyRuleAntecedent();
  if_dSlopePslopeP->joinWithAND(dSlopeP, slopeP);

  //implikasi
  FuzzyRuleConsequent *then_dPP = new FuzzyRuleConsequent();
  then_dPP->addOutput(dP);
  FuzzyRuleConsequent *then_dPZ = new FuzzyRuleConsequent();
  then_dPZ->addOutput(dZ);
  FuzzyRuleConsequent *then_dPN = new FuzzyRuleConsequent();
  then_dPN->addOutput(dN);

  FuzzyRule *fuzzyRule01 = new FuzzyRule(1, if_dSlopeNslopeN, then_dPP);
  fuzzy->addFuzzyRule(fuzzyRule01);
  FuzzyRule *fuzzyRule02 = new FuzzyRule(2, if_dSlopeNslopeZ, then_dPZ);
  fuzzy->addFuzzyRule(fuzzyRule02);
  FuzzyRule *fuzzyRule03 = new FuzzyRule(3, if_dSlopeNslopeP, then_dPN);
  fuzzy->addFuzzyRule(fuzzyRule03);

  FuzzyRule *fuzzyRule04 = new FuzzyRule(4, if_dSlopeZslopeN, then_dPP);
  fuzzy->addFuzzyRule(fuzzyRule04);
  FuzzyRule *fuzzyRule05 = new FuzzyRule(5, if_dSlopeZslopeZ, then_dPZ);
  fuzzy->addFuzzyRule(fuzzyRule05);
  FuzzyRule *fuzzyRule06 = new FuzzyRule(6, if_dSlopeZslopeP, then_dPN);
  fuzzy->addFuzzyRule(fuzzyRule06);

  FuzzyRule *fuzzyRule07 = new FuzzyRule(7, if_dSlopePslopeN, then_dPP);
  fuzzy->addFuzzyRule(fuzzyRule07);
  FuzzyRule *fuzzyRule08 = new FuzzyRule(8, if_dSlopePslopeZ, then_dPZ);
  fuzzy->addFuzzyRule(fuzzyRule08);
  FuzzyRule *fuzzyRule09 = new FuzzyRule(9, if_dSlopePslopeP, then_dPN);
  fuzzy->addFuzzyRule(fuzzyRule09);

  pwmA = 127;
}

float tegangan,teganganL,deltaV;
float arus,arusL;
float daya,dayaL,deltaP;

void loop()
{
  // arus dan tegangan dari simulasi random
  //arus = random(0, 100)/50.0;
  //tegangan = random(120, 220)/10.0;

  float faktorI = 6.5/1023;
  float faktorV = 22.0/1023;
  
  //arus dan tegangan dari ADC
  arus = analogRead(A3) * faktorI;
  tegangan = analogRead(A0) * faktorV;

  
  daya = arus * tegangan;
  dayaL = arusL * teganganL;
  deltaV = tegangan - teganganL;
  deltaP = daya - dayaL;

//peritungan slope kurva PV tegangan vs daya
  slope = deltaP /deltaV;
  deltaSlope = slope - slopeL;

  slopeL = slope;

  //data sebelum
  teganganL = tegangan;
  arusL = arus;

  float input1 = slope;
  float input2 = deltaSlope;

  Serial.print("Tegangan: ");
  Serial.print(tegangan);
//  Serial.print(" ");
  Serial.print(" Arus: ");
  Serial.println(arus);
  Serial.print("Daya: ");
  Serial.print(daya);
  Serial.print(" ");
  Serial.print("Delta Daya: ");
  Serial.print(deltaP);
  Serial.print(", Delta Tegangan: ");
  Serial.println(deltaV);

  Serial.print("Slope: ");
  Serial.print(slope);
  Serial.print(", Delta Slope: ");
  Serial.println(deltaSlope);

  fuzzy->setInput(1, input1);
  fuzzy->setInput(2, input2);
  fuzzy->fuzzify();

  Serial.println("Slope: ");
  Serial.print("Turun: ");
  Serial.print(slopeN->getPertinence());
  Serial.print(", Tetap: ");
  Serial.print(slopeZ->getPertinence());
  Serial.print(", Naik: ");
  Serial.println(slopeP->getPertinence());

  Serial.println("Perubahan Slope: ");
  Serial.print("Turun: ");
  Serial.print(dSlopeN->getPertinence());
  Serial.print(", Tetap: ");
  Serial.print(dSlopeZ->getPertinence());
  Serial.print(", Naik: ");
  Serial.println(dSlopeP->getPertinence());
  Serial.println();

  float output1 = fuzzy->defuzzify(1);
  pwmA +=output1;
  if(pwmA<0)pwmA=0;
  if(pwmA>255)pwmA=255;
  pwm1=round(pwmA);
  
  Serial.println("Output Perubahan PWM: ");
  Serial.print("PWM turun: ");
  Serial.print(dN->getPertinence());
  Serial.print(",PWM tetap: ");
  Serial.print(dZ->getPertinence());
  Serial.print(",PWM naik: ");
  Serial.println(dP->getPertinence());
  Serial.println();
  Serial.print("Perubaan PWM: ");
  Serial.println(output1);
  Serial.print("PWM: ");
  Serial.println(pwm1);
  Serial.println();

  delay(5000);
}
