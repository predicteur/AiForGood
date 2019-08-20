void prSerie(float serie[], int len, String nom) {
  Serial.print(nom); Serial.print(" :[ ");
  for (int i=0; i<len; i++){
    Serial.print(serie[i]);
    Serial.print(", ");
  }
  Serial.println("] ");
}
void prCoef(struct CoefComp *coef){
  Serial.print("a0 : ");Serial.print(coef->a0); Serial.print(" b0 : ");Serial.println(coef->b0);
  Serial.print("a1 : ");Serial.print(coef->a1[0]);Serial.println(coef->a1[1]);
  Serial.print("b1 : ");Serial.print(coef->b1[0]);Serial.println(coef->b1[1]);
}
void estim(float a, float b, int len, float sserie[]){
  for (int i=0; i<len; i++){
     sserie[i] = a * float(i+1) + b;
  }
}
void ecart(float a, float b, float serie[], float sserie[], int len){
  for (int i=0; i<len; i++){
    sserie[i] = serie[i] - a * float(i+1) - b;
  }
}
void adds(float serie1[], float serie2[], float sserie[], int len){
  for (int i=0; i<len; i++){
    sserie[i] = serie1[i] + serie2[i];
  }
}
void diff(float serie1[], float serie2[], float sserie[], int len){
  for (int i=0; i<len; i++){
    sserie[i] = serie1[i] - serie2[i];
  }
}
float et(float ecart[], int len){
  float ecartType = 0;
  for (int i=0; i<len; i++){
    ecartType += pow(ecart[i], 2) / len;
  }
  ecartType = sqrt(ecartType);
  return ecartType;
}
float moy(float serie[], int len){
  float moyenne = 0;
  for (int i=0; i<len; i++){
    moyenne += serie[i]/len;
  }
  return moyenne;
}
float ecretage(float serie[], float mini, float maxi, float sserie[], int len){
  for (int i=0; i<len; i++){
    sserie[i] = min(maxi, max(mini, serie[i]));
  }
}
void sousSerie(float serie[], float sous[], int indice, int len2){
  for (int i=0; i<len2; i++){
    sous[i] = serie[indice * len2 + i];
  }
}
void addbin(int param, int payl[], int lon, int rang){
  for (int i=0; i<lon; i++){
    payl[rang+i] = bitRead(param, i);
  }
}
int decbin(int payl[], int lon, int rang){
  int param = 0;
  for (int i=0; i<lon; i++){
    bitWrite(param, i, payl[rang+i]);
    //param += payl[rang+i] * int(pow(2, i));
  }
  return param;
}
int conversion(float valeur, float mini, float maxi, int bits){
    int minib = 0, maxib = pow(2, bits) - 1;
    int val = minib + round(float(maxib - minib) * (valeur - mini) / (maxi - mini));
    // if (val>maxib) : print("erreurb : ", nom, valeur, mini, maxi)
    return max(minib, min(maxib, val));
}
float conversionb(int valeurb, float mini, float maxi, int bits){
    int minib = 0, maxib = pow(2, bits) - 1;
    return mini + (maxi - mini) * float(valeurb - minib) / float(maxib - minib);
}
