//-----------------------------------------------------------------------------------------------------------------------------
  void CreerMesure(){
    device.setVal("type", TYPE_DEVICE);
    device.setVal("materiel", VERSION_MATERIEL);
    device.setVal("logiciel", VERSION_LOGICIEL);

    device.setVal("ressenti", RESSENTI_NORMAL);
    device.setVal("niveauBatterie", 100);
    device.setVal("longitude", "");
    device.setVal("latitude", "");
    device.setVal("tempsCycle", float(TEMPS_CYCLE));
    device.setVal("mesureLED", float(M_LED_DEFAUT));

    mes[M_PM25].setVal("capteur", CAPTEUR);
    mes[M_PM25].setVal("type", T_PM25);
    mes[M_PM25].setVal("valeurMin", VALEUR_MIN_PM);
    mes[M_PM25].setVal("valeurMax", VALEUR_MAX_PM);
    mes[M_PM25]["valeurFiltree"].init(-1);

    mes[M_PM10].setVal("capteur", CAPTEUR);
    mes[M_PM10].setVal("type", T_PM10);
    mes[M_PM10].setVal("valeurMin", VALEUR_MIN_PM);
    mes[M_PM10].setVal("valeurMax", VALEUR_MAX_PM);
    mes[M_PM10]["valeurFiltree"].init(-1);
}
//-----------------------------------------------------------------------------------------------------------------------------
  void RefreshMesure(){
    for (int nMes = 0; nMes < NB_MES; ++nMes) mes[nMes].refresh(0);
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void CalculMesure(){
    for (int nMes = 0; nMes < NB_MES; ++nMes) {
      mes[nMes]["nombre"][0] += 1;
      mes[nMes]["date"][0] += millis();
      if ((pm[nMes] > mes[nMes].getValF("valeurMin")) and (pm[nMes] < mes[nMes].getValF("valeurMax"))) {
        mes[nMes]["valeur"][0] += pm[nMes];
        mes[nMes]["nombreOk"][0] += 1;
        mes[nMes]["ecartType"][0] += pm[nMes] * pm[nMes];
      }
    }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void GenereMesure(){
    float variance;
    for (int nMes = 0; nMes < NB_MES; ++nMes) {
      bool debut = mes[nMes]["valeurFiltree"][1] == -1;
      mes[nMes]["date"][0] = mes[nMes]["date"][0] / mes[nMes]["nombre"][0];
      if (mes[nMes]["nombreOk"][0] > 0) {
        mes[nMes]["valeur"][0] /= float(mes[nMes]["nombreOk"][0]);
        mes[nMes]["valeurFiltree"].lissES(mes[nMes]["valeur"], debut, COEF_FILTRAGE, false);
        variance = (mes[nMes]["ecartType"][0] / float(mes[nMes]["nombreOk"][0]) - pow(mes[nMes]["valeur"][0], 2.0));
        if (variance < 0.000001) mes[nMes]["ecartType"][0] = 0.0;
        else mes[nMes]["ecartType"][0] = sqrt(variance);
        mes[nMes]["tauxErreur"][0] = 1.0 - float(mes[nMes]["nombreOk"][0]) / float(mes[nMes]["nombre"][0]);
      }
    }
}
//--------------- API V1--------------------------------------------------------------------------------------------------------------
  String MesureGenereJSON() { 
    const size_t capacity = JSON_OBJECT_SIZE(13);
    DynamicJsonDocument data(capacity);
    String JSONmessage = "";
    data["device"]            = DEVICE_NAME;
    data["pm25"]              = String(mes[M_PM25]["valeur"][0], 2);
    data["pm10"]              = String(mes[M_PM10]["valeur"][0], 2);
    data["pm25_filtree"]      = String(mes[M_PM25]["valeurFiltree"][0], 2);
    data["pm10_filtree"]      = String(mes[M_PM10]["valeurFiltree"][0], 2);
    data["date_mesure"]       = dateToString(calculDate(mes[M_PM25]["date"][0]));
    data["ecart_type_pm25"]   = String(mes[M_PM25]["ecartType"][0], 2);
    data["ecart_type_pm10"]   = String(mes[M_PM10]["ecartType"][0], 2);
    data["taux_erreur_pm25"]  = String(mes[M_PM25]["tauxErreur"][0], 2);
    data["taux_erreur_pm10"]  = String(mes[M_PM10]["tauxErreur"][0], 2);
    data["feeling"]           = device.getValS("ressenti");
#if GPS
    data["latitude"]          = device.getValS("latitude");
    data["longitude"]         = device.getValS("longitude");
#else
    data["latitude"]          = "";
    data["longitude"]         = "";
#endif
    serializeJson(data, JSONmessage);
    return JSONmessage;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  boolean MesureOk() {
  return (mes[M_PM25]["nombreOk"][0] > 0) and (mes[M_PM10]["nombreOk"][0] > 0);
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void PrMesure(int level){
    theme = "capteur ";
    int nmes = round(device.getValF("mesureLED"));
    Log(level, "   Nombre de mesures : " + String(mes[nmes]["nombre"][0]) + " Taux d erreur : " + String(mes[nmes]["tauxErreur"][0]), "");
    Log(level, "   Valeur(inst+filt) : " + String(mes[nmes]["valeur"][0]) + "  " + String(mes[nmes]["valeurFiltree"][0]) + " Ecart-type : " + String(mes[nmes]["ecartType"][0]),"");
    Log(level, "   Ressenti          : " + device.getValS("ressenti") + " Date : " + dateToString(calculDate(mes[nmes]["date"][0])), "");
  }
  //-----------------------------------------------------------------------------------------------------------------------------
#ifdef COMPRESSION
  void PrSerie(int level, Serie yi) {
    theme = "compres ";
    String valeur =  yi.lenom() + " :[ ";
    for (int i=0; i<yi.len(); i++) valeur += String(yi[i]) + ", ";
    valeur += "] ";
    Log(level, valeur, "");
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void GenereGroupe() {
    String res = combo.calcul(mes[M_PM25]["valeurFiltree"], true);
    Serie valComp = combo.compress();

    payload.msg1 = sn.decbin(valComp.sousSerie(0, 32), 32);
    payload.msg2 = sn.decbin(valComp.sousSerie(32, 32), 32);
    payload.msg3 = sn.decbin(valComp.sousSerie(64, 32), 32);
  }
#endif
