  
//-----------------------------------------------------------------------------------------------------------------------------
  void InitMesureRessenti(){
    int nMes;
    for (nMes = 0; nMes < NB_MES; ++nMes) {
      mes[nMes].nombre        = 0;
      mes[nMes].nombreOk      = 0;
      mes[nMes].valeur        = 0;
      mes[nMes].date          = 0;
      mes[nMes].ecartType     = 0;
      mes[nMes].tauxErreur    = 0;  }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void CreerMesure(){
    InitMesureRessenti();

    devType.type              = TYPE_DEVICE;
    devType.materiel          = VERSION_MATERIEL;
    devType.logiciel          = VERSION_LOGICIEL;

    device.ressenti           = RESSENTI_NORMAL;
    device.niveauBatterie     = 100;
    device.longitude          = "";
    device.latitude           = "";
    device.tempsCycle         = TEMPS_CYCLE;
    device.mesureLED          = M_LED_DEFAUT;
    
    mesType[M_PM25].capteur   = CAPTEUR;
    mesType[M_PM25].type      = T_PM25;
    mesType[M_PM25].valeurMin = VALEUR_MIN_PM;
    mesType[M_PM25].valeurMax = VALEUR_MAX_PM;
    mes[M_PM25].valeurFiltree = -1;
  
    mesType[M_PM10].capteur   = CAPTEUR;
    mesType[M_PM10].type      = T_PM10;
    mesType[M_PM10].valeurMin = VALEUR_MIN_PM;
    mesType[M_PM10].valeurMax = VALEUR_MAX_PM;
    mes[M_PM10].valeurFiltree = -1;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void CalculMesure(){
    for (int nMes = 0; nMes < NB_MES; ++nMes) {
      mes[nMes].nombre += 1;
      mes[nMes].date += millis();
      if ((pm[nMes] > mesType[nMes].valeurMin) and (pm[nMes] < mesType[nMes].valeurMax)) {
          mes[nMes].valeur += pm[nMes];
          mes[nMes].nombreOk += 1;
          mes[nMes].ecartType += pm[nMes]*pm[nMes];  }  }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void GenereMesure(){
    float variance;
    for (int nMes = 0; nMes < NB_MES; ++nMes) {
      mes[nMes].date = mes[nMes].date / mes[nMes].nombre;
      if (mes[nMes].nombreOk > 0) {
          mes[nMes].valeur /= float(mes[nMes].nombreOk);
          if (mes[nMes].valeurFiltree < 0) mes[nMes].valeurFiltree = mes[nMes].valeur;
          else mes[nMes].valeurFiltree = COEF_FILTRAGE * mes[nMes].valeurFiltree + (1-COEF_FILTRAGE) * mes[nMes].valeur;
          variance = (mes[nMes].ecartType / float(mes[nMes].nombreOk) - pow(mes[nMes].valeur, 2.0));
          if (variance < 0.000001) mes[nMes].ecartType =  0.0;
          else mes[nMes].ecartType = sqrt(variance);
          mes[nMes].tauxErreur = 1.0 - float(mes[nMes].nombreOk) / float(mes[nMes].nombre);  }  }
    
  }
//--------------- API V1--------------------------------------------------------------------------------------------------------------
  String MesureGenereJSON() { 
    const size_t capacity = JSON_OBJECT_SIZE(13);
    DynamicJsonDocument data(capacity);
    String JSONmessage = "";
    data["device"]            = DEVICE_NAME;
    data["pm25"]              = String(mes[M_PM25].valeur, 2);
    data["pm10"]              = String(mes[M_PM10].valeur, 2);
    data["pm25_filtree"]      = String(mes[M_PM25].valeurFiltree, 2);
    data["pm10_filtree"]      = String(mes[M_PM10].valeurFiltree, 2);
    data["date_mesure"]       = dateToString(calculDate(mes[M_PM25].date));
    data["ecart_type_pm25"]   = String(mes[M_PM25].ecartType, 2);
    data["ecart_type_pm10"]   = String(mes[M_PM10].ecartType, 2);
    data["taux_erreur_pm25"]  = String(mes[M_PM25].tauxErreur, 2);
    data["taux_erreur_pm10"]  = String(mes[M_PM10].tauxErreur, 2);
    data["feeling"]           = device.ressenti;
#if GPS
    data["latitude"]          = device.latitude;
    data["longitude"]         = device.longitude;
#else
    data["latitude"]          = "";
    data["longitude"]         = "";
#endif
    serializeJson(data, JSONmessage);
    return JSONmessage;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  boolean MesureOk() {
    return (mes[M_PM25].nombreOk > 0) and (mes[M_PM10].nombreOk > 0);
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void PrMesure(int level){
    theme = "capteur ";
    Log(level, "   Nombre de mesures : " + String(mes[device.mesureLED].nombre) + " Taux d erreur : " + String(mes[device.mesureLED].tauxErreur), "");
    Log(level, "   Valeur(inst+filt) : " + String(mes[device.mesureLED].valeur) + "  " + String(mes[device.mesureLED].valeurFiltree) + " Ecart-type : " + String(mes[device.mesureLED].ecartType),"");
    Log(level, "   Ressenti          : " + device.ressenti + " Date : " + dateToString(calculDate(mes[device.mesureLED].date)), "");
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
void GroupeMesure() {
    for (int i = 0; i < NB_MES; ++i) {
      mesEnvoi[i][nbMesureGroupe].nombre       = mes[i].nombre;
      mesEnvoi[i][nbMesureGroupe].nombreOk     = mes[i].nombreOk;
      mesEnvoi[i][nbMesureGroupe].valeur       = mes[i].valeur;
      mesEnvoi[i][nbMesureGroupe].ecartType    = mes[i].ecartType;
      mesEnvoi[i][nbMesureGroupe].date         = mes[i].date;
      mesEnvoi[i][nbMesureGroupe].tauxErreur   = mes[i].tauxErreur;  }
    
    //stockage en mémoire à mettre

  }
//-----------------------------------------------------------------------------------------------------------------------------
  void GenereGroupe() {
    for (int i = 0; i < TAILLE_ECH; ++i) y0i[i] = mesEnvoi[device.mesureLED][i].valeur;
    for (int i = 0; i < NB_MES; ++i) {
      for (int j = 0; j < TAILLE_ECH; ++j) {
        mesEnvoi[i][j].nombre       = 0;
        mesEnvoi[i][j].nombreOk     = 0;
        mesEnvoi[i][j].valeur       = 0;
        mesEnvoi[i][j].ecartType    = 0;
        mesEnvoi[i][j].date         = 0;
        mesEnvoi[i][j].tauxErreur   = 0;  }  }
  }
#endif
