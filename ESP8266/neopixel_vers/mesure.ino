
void InitMesure(){
  int nMes;
  for (nMes = 0; nMes < nbMes; ++nMes) {
    mes[nMes].nombre = 0;
    mes[nMes].nombreOk = 0;
    mes[nMes].valeur = 0;
    mes[nMes].date = 0;
    mes[nMes].ecartType = 0;
    mes[nMes].tauxErreur = 0;
  }
}

void CreerMesure(){
  InitMesure();
  
  mes[0].nom = "PM25";
  mes[0].valeurMin = VALEUR_MIN_PM;
  mes[0].valeurMax = VALEUR_MAX_PM;

  mes[1].nom = "PM101";
  mes[1].valeurMin = VALEUR_MIN_PM;
  mes[1].valeurMax = VALEUR_MAX_PM;
}

void CalculMesure(){
  //Serial.println(nb);
  //Serial.println(pm[0]);
  for (int nMes = 0; nMes < nbMes; ++nMes) {
    mes[nMes].nombre += 1;
    mes[nMes].date += millis();
    if ((pm[nMes] > mes[nMes].valeurMin) and (pm[nMes] < mes[nMes].valeurMax)) {
        mes[nMes].valeur += pm[nMes];
        mes[nMes].nombreOk += 1;
        mes[nMes].ecartType += pm[nMes]*pm[nMes];
    }
  }
}

void GenereMesure(){
  float variance;
  for (int nMes = 0; nMes < nbMes; ++nMes) {
    mes[nMes].date /= float(mes[nMes].nombre);
    if (mes[nMes].nombreOk > 0) {
        mes[nMes].valeur /= float(mes[nMes].nombreOk);
        variance = (mes[nMes].ecartType / float(mes[nMes].nombreOk) - pow(mes[nMes].valeur, 2.0));
        if (variance < 0.000001) {
          mes[nMes].ecartType =  0.0;
        } else {
          mes[nMes].ecartType = sqrt(variance);
        }
        mes[nMes].tauxErreur = 1.0 - float(mes[nMes].nombreOk) / float(mes[nMes].nombre);
        //Serial.println('mesure ');
        //Serial.print(mes[0].valeur,  mes[0].ecartType);
        //Serial.println(mes[0].nombre );
    }
  }
}

