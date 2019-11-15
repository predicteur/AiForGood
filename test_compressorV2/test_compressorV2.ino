
//#include "stdafx.h"
//#include <iostream>
//#include <string>
#include "MatrixMath.h"
#include "compressor.h"
#include <ESP8266WiFi.h>

//using namespace std;

  const int leny0 = 16;
  const int   TAILLE_ECH = 16;
  float y[leny0] = { 2, 3.5, 5, 15, 20, 12, 18, 2, 8, 3.5, 5, 15, 20, 10, 12, 18 };
  /*const int leny0 = 32;
  float y[leny0] = { 2, 3.5, 5, 15, 20, 12, 18, 2, 8, 3.5, 5, 15, 20, 10, 12, 18,
             2, 3.5, 5, 11, 20, 12, 18, 12, 8, 3.5, 5, 10, 2, 10, 12, 18 };
  */
  float* y0 = new float[leny0];
  float* y0r = new float[leny0];
  float* ye = new float[leny0];
  float* y2 = new float[leny0];
  float ectc;
  String res;
  bool regres = true, codec = true;

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i<leny0; i++) y0[i] = y[i];

  //exemple 1 : estimation unique, un seul point(moyenne)
  const int NBREG = 8, NBREG0 = 1, NBREG1 = 2;
  const int BIT0 = 3, BIT1 = 2, BITECT = 8;
  const float MINI = 0.0, MAXI = 50.0;
  compressor combo(NBREG, NBREG0, NBREG1, MINI, MAXI, BIT0, BIT1, BITECT, leny0);
  //cout << "1 bits : " << combo.taillePayload() << " taux : " << (float)combo.taillePayload() / (float)leny0 << endl;
  res = combo.calcul(y0, true);
  //cout << "res : " << res << endl;
  y2 = combo.simul();
  //cout << " y2 simul : "; for (int i = 0; i<leny0; i++) cout << y2[i] << ", "; cout << endl;
  ectc = combo.ecartTypeSimul(false);
  //cout << "ectc : " << ectc << "  " << combo.decompressEct(combo.compress()) << endl;
  y0r = combo.decompressY0(combo.compress());
  //cout << " y0r : "; for (int i = 0; i<leny0; i++) cout << y0r[i] << ", "; cout << endl;
  //cout << " pay : "; for (int i = 0; i<combo.taillePayload(); i++) cout << combo.compress()[i] << ", "; cout << endl;
}

void loop() {
  // put your main code here, to run repeatedly:

}
