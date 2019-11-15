#ifdef COMPRESSION
//-----------------------------------------------------------------------------------------------------------------------------
  void PrSerie(int level, float serie[], int len, String nom) {
    theme = "compres ";
    String valeur =  nom + " :[ ";
    for (int i=0; i<len; i++) valeur += String(serie[i]) + ", ";
    valeur += "] ";
    Log(level, valeur, "");
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void PrCoef(int level, struct CoefComp *coef){
    theme = "compres ";
    Log(level, "a0 : " + String(coef->a0) + " b0 : " + String(coef->b0), "");
    Log(level, "a1 : " + String(coef->a1[0]) + String(coef->a1[1]), "");
    Log(level, "b1 : " + String(coef->b1[0]) + String(coef->b1[1]), "");
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void estim(float a, float b, int len, float sserie[]){
    for (int i=0; i<len; i++) sserie[i] = a * float(i+1) + b;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void ecart(float a, float b, float serie[], float sserie[], int len){
    for (int i=0; i<len; i++) sserie[i] = serie[i] - a * float(i+1) - b;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void adds(float serie1[], float serie2[], float sserie[], int len){
    for (int i=0; i<len; i++) sserie[i] = serie1[i] + serie2[i];
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void diff(float serie1[], float serie2[], float sserie[], int len){
    for (int i=0; i<len; i++) sserie[i] = serie1[i] - serie2[i];
  }
//-----------------------------------------------------------------------------------------------------------------------------
  float et(float ecart[], int len){
    float ecartType = 0;
    for (int i=0; i<len; i++) ecartType += pow(ecart[i], 2) / len;
    ecartType = sqrt(ecartType);
    return ecartType;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  float moy(float serie[], int len){
    float moyenne = 0;
    for (int i=0; i<len; i++) moyenne += serie[i]/len;
    return moyenne;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void ecretage(float serie[], float mini, float maxi, float sserie[], int len){
    for (int i=0; i<len; i++) sserie[i] = min(maxi, max(mini, serie[i]));
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void sousSerie(float serie[], float sous[], int indice, int len2){
    for (int i=0; i<len2; i++) sous[i] = serie[indice * len2 + i];
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void addbin(int param, int payl[], int lon, int rang){
    for (int i=0; i<lon; i++) payl[rang+i] = bitRead(param, i);
  }
//-----------------------------------------------------------------------------------------------------------------------------
  int decbin(int payl[], int lon, int rang){
    int param = 0;
    for (int i=0; i<lon; i++) bitWrite(param, i, payl[rang+i]);
    return param;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  int conversion(float valeur, float mini, float maxi, int bits){
      int minib = 0, maxib = pow(2, bits) - 1;
      int val = minib + round(float(maxib - minib) * (valeur - mini) / (maxi - mini));
      return max(minib, min(maxib, val));
  }
//-----------------------------------------------------------------------------------------------------------------------------
  float conversionb(int valeurb, float mini, float maxi, int bits){
      int minib = 0, maxib = pow(2, bits) - 1;
      return mini + (maxi - mini) * float(valeurb - minib) / float(maxib - minib);
  }
#endif
