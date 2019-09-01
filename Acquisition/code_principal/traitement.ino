//-----------------------------------------------------------------------------------------------------------------------------
  void UpdateLed() { 
    if        (mes[0].valeur < SEUIL_BON_PM)    { StripAffiche("correct");
    } else if (mes[0].valeur < SEUIL_MOYEN_PM)  { StripAffiche("moyen");
    } else                                      { StripAffiche("mauvais");
    }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  String  CalculDate(unsigned long dateInt) {
    String chaine = DateToString(dateInt + dateRef.decalage);
    if (dateRef.dateExt == 0) {
        chaine = " - ";
    }
    return chaine;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  String DateToString( unsigned long date){
    String mois;
    int milli = date % 1000;
    date /= 1000;
    int seconde = date % 60;
    date /= 60;
    int minute = date % 60;
    date /= 60;
    int heure = date % 24;
    int jour = date / 24;
    if (jour <= 31) {
      mois = "08";  
    } else if (jour <= 61) {
      mois = "09";
      jour -= 31;
    }
    //String chaine = "2019-" + mois + "-" + (String)jour + "T" + (String)heure + ":" + (String)minute + ":" + (String)seconde + "." + (String)milli + "Z";
    //Serial.println( chaine + " DtoS JHMS : " + String(jour) + String(heure) + String(minute) + String(seconde));
    return "2019-" + mois + "-" + (String)jour + "T" + (String)heure + ":" + (String)minute + ":" + (String)seconde + "." + (String)milli + "Z";
  }
//-----------------------------------------------------------------------------------------------------------------------------
  unsigned long StringToDate(String chaine){                      // millisecondes à partir du 1/8/2019 à 0h
    int annee     = chaine.substring(0,4).toInt();
    int mois      = chaine.substring(5,7).toInt();
    int jour      = chaine.substring(8,10).toInt();
    int heure     = chaine.substring(11,13).toInt();
    int minute    = chaine.substring(14,16).toInt();
    int seconde   = chaine.substring(17,19).toInt();
    int milli     = chaine.substring(20,23).toInt();
    int date      = 0;
    //Serial.println(String(annee) + String(mois) + String(jour) + String(heure) + String(minute) + String(seconde) + String(milli));
    if (mois == 9)  { date += 31*24*60*60*1000;}
    if (mois == 10) { date += 61*24*60*60*1000;}
    if (mois == 11) { date += 61*24*60*60*1000;}
    date += jour*24*60*60*1000;
    date += heure*60*60*1000;
    date += minute*60*1000;
    date += seconde*1000;
    date += milli;    
    //Serial.println( chaine + " StoD JHMS : " + String(jour) + String(heure) + String(minute) + String(seconde));
    return date;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  boolean TestBatterieBasse(){
    
    // à construire
    
    return false;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void StripAffiche(String modeStrip){
    int niveau = niveauAffichage; 
    //Serial.println(modeStrip);                                                              // mode debug
    if (niveauBatterieBas) { niveau = max(niveauAffichage, NIVEAU_FAIBLE); }     
        strip.setBrightness(LUMINOSITE_FAIBLE/niveau);
    if        (modeStrip == "démarrage") {     
        strip.setBrightness(LUMINOSITE_FAIBLE/niveau);
        strip.setPixelColor(0, ORANGE[0], ORANGE[1], ORANGE[2]);
    } else if (modeStrip == "démarré") {       
        strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]);
    } else if (modeStrip == "mesure non envoyée") {
        strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]);
    } else if (modeStrip == "correct") {     
        strip.setPixelColor(0, VERT[0], VERT[1], VERT[2]);
    } else if (modeStrip == "moyen") {     
        strip.setPixelColor(0, ORANGE[0], ORANGE[1], ORANGE[2]);
    } else if (modeStrip == "mauvais") {         
        strip.setPixelColor(0, ROUGE[0], ROUGE[1], ROUGE[2]);
    } else if (modeStrip == "début mesure") { 
        strip.setBrightness(LUMINOSITE_FORTE/niveau);
    } else if (modeStrip == "fin mesure") {  
        strip.setBrightness(LUMINOSITE_FAIBLE/niveau);
    }
    strip.show();
  }
//-----------------------------------------------------------------------------------------------------------------------------
  boolean MesureOk() {
    return (mes[0].nombreOk > 0) and (mes[1].nombreOk > 0);
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void PrMesure(){
    Serial.println(" Nombre de mesures : " + String(mes[0].nombre) + " Taux d erreur : " + String(mes[0].tauxErreur));
    Serial.println(" Valeur : " + String(mes[0].valeur) + " Ecart-type : " + String(mes[0].ecartType));
    Serial.println(" Ressenti : " + String(sensorVal1) + String(sensorVal2) + String(sensorVal3) + " Date : " + CalculDate(mes[0].date));
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void LireCapteur(){
    PmResult pm_val = sds.queryPm();
    if (pm_val.isOk()) {
      pm[0] = pm_val.pm25;
      pm[1] = pm_val.pm10;
    } else {
      Serial.print("capteur sds non présent,  ");
#ifdef BOARDSIGFOX
      SigFox.begin(); delay(100);      
      pm[0] = SigFox.internalTemperature();
      pm[1] = SigFox.internalTemperature();
      SigFox.end();
#else
      pm[0] = float(ESP.getVcc())/10.0;
      pm[1] = 100.0 * cos(3.14159 * millis());
#endif
    }
    Serial.println("mesure capteur : " + String(pm[0]));
  }

#ifdef RESEAUWIFI
//-----------------------------------------------------------------------------------------------------------------------------
  void GenereJSON() { 
    root.clear();
    JSONmessage = "";
    root["device"]            = DEVICE_NAME;
    root["PM2_5"]             = String(mes[0].valeur, 2);
    root["PM10"]              = String(mes[1].valeur, 2);
    root["date_mesure"]       = CalculDate(mes[0].date);
    root["ecart-type_PM25"]   = String(mes[0].ecartType, 2);
    root["ecart-type_PM10"]   = String(mes[1].ecartType, 2);
    root["taux-erreur_PM25"]  = String(mes[0].tauxErreur, 2);
    root["taux-erreur_PM10"]  = String(mes[1].tauxErreur, 2);
    root["positive_feeling"]  = String(sensorVal1, 2);
    root["mixed_feeling"]     = String(sensorVal2, 2);
    root["negative_feeling"]  = String(sensorVal3, 2);
    if (measureJson(root) > (TAILLE_MAX_JSON-10)) {
      Serial.println("taille JSON supérieure à la taille maxi");
    } else {
      serializeJson(root, JSONmessage);
    }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  int EnvoiJSON() { 
    int httpCode = 0;
    if (WiFi.status() == WL_CONNECTED) {                                  
      HTTPClient http;
      http.begin(SERVEUR_AI4GOOD);
      http.addHeader("Content-Type", "application/json");                 
      httpCode = http.POST(JSONmessage);                                  
      payload = http.getString();                                  
      http.end();        
      //httpCode = 205;                                             // pour tester absence de réseau
      Serial.println("JSONmessage : " + JSONmessage);
      Serial.println("payload     : " + payload);
    }
    Serial.println("httpCode : " + String(httpCode));
    return httpCode;    
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void RecalageDate(unsigned long dateReponseInt) {               // lecture de la date serveur pour recalage
    root.clear();
    DeserializationError err = deserializeJson(root, payload);
    if (err) {
      Serial.println("deserialize ko");
    } else {
      const char* dateServeur = root["updated_at"];
      //Serial.println("av : " + CalculDate(dateReponseInt) +" ap : " + dateServeur);
      dateRef.dateExt = StringToDate(dateServeur);
      dateRef.dateInt = dateReponseInt;
      dateRef.decalage = dateRef.dateExt - dateRef.dateInt;
    }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void AjouteMesure() {                                  // Stockage des donnnées non envoyées
    StripAffiche("mesure non envoyée");
    ficMes = SPIFFS.open(FIC_BUF, "r");
    String total = ficMes.readString();
    ficMes.close();
    delay(50);
    ficMes = SPIFFS.open(FIC_BUF, "w");
    total = total + "\n" + JSONmessage;
    //total = total + JSONmessage;
    ficMes.print(total);
    ficMes.close();
    Serial.println("total stocké : ");
    Serial.println(total);
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void EnvoiWifi() { 
    GenereJSON();
    int httpCode = EnvoiJSON();
    unsigned long dateReponseInt = millis();
    if (httpCode != 201) { 
      StripAffiche("mesure non envoyée");
      AjouteMesure();
    } else {
      RecalageDate(dateReponseInt);       
    }
  }
//-----------------------------------------------------------------------------------------------------------------------------  
  void RepriseEnvoiWifi() {
    String mesureNonReprise = "";
    ficMes = SPIFFS.open(FIC_BUF, "r");
    while(ficMes.available()>5) {    
      JSONmessage = ficMes.readStringUntil('\n');
      if ((JSONmessage.length() > 10) && (JSONmessage.length() < TAILLE_MAX_JSON))  {
        unsigned long dateDebutReponseInt = millis();
        int httpCode = EnvoiJSON();
        if (httpCode != 201) {
          mesureNonReprise += ("\n" + JSONmessage);
        } else {
          unsigned long dateReponseInt = millis();
          RecalageDate((dateReponseInt + dateDebutReponseInt)/2);         
        }
      }
    }
    ficMes = SPIFFS.open(FIC_BUF, "w");
    ficMes.print(mesureNonReprise);
    ficMes.close();
    Serial.println("total stocké : ");
    Serial.println(mesureNonReprise);
  }
//-----------------------------------------------------------------------------------------------------------------------------  
  void SendMesureWeb(){      // construire le JSON pour la page web
    String valeurInst = "{\"PM25\" : " + String(pm[0]) + ", \"PM10\" : " + String(pm[1]) + "}";
    Serial.print("json mesure: "); Serial.println(valeurInst);
    server.send(200, "application/json", valeurInst);
    Serial.println("envoi mesures");
  }
#endif
#ifdef COMPRESSION
//-----------------------------------------------------------------------------------------------------------------------------
void GroupeMesure() {
    for (int i = 0; i < NB_MES; ++i) {
      mesEnvoi[i][nbMesureGroupe].nom          = mes[i].nom;
      mesEnvoi[i][nbMesureGroupe].nombre       = mes[i].nombre;
      mesEnvoi[i][nbMesureGroupe].nombreOk     = mes[i].nombreOk;
      mesEnvoi[i][nbMesureGroupe].valeur       = mes[i].valeur;
      mesEnvoi[i][nbMesureGroupe].ecartType    = mes[i].ecartType;
      mesEnvoi[i][nbMesureGroupe].date         = mes[i].date;
      mesEnvoi[i][nbMesureGroupe].valeurMin    = mes[i].valeurMin;
      mesEnvoi[i][nbMesureGroupe].valeurMax    = mes[i].valeurMax;
      mesEnvoi[i][nbMesureGroupe].tauxErreur   = mes[i].tauxErreur;
    }
    //stockage en mémoire à mettre
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void GenereGroupe() {
    for (int i = 0; i < TAILLE_ECH; ++i) {
      y0init[i] = mesEnvoi[0][i].valeur;
    }
    for (int i = 0; i < NB_MES; ++i) {
      for (int j = 0; j < TAILLE_ECH; ++j) {
        mesEnvoi[i][j].nombre       = 0;
        mesEnvoi[i][j].nombreOk     = 0;
        mesEnvoi[i][j].valeur       = 0;
        mesEnvoi[i][j].ecartType    = 0;
        mesEnvoi[i][j].date         = 0;
        mesEnvoi[i][j].tauxErreur   = 0;
      }
    }
  }
#endif
#ifdef BOARDSIGFOX
//-----------------------------------------------------------------------------------------------------------------------------
  void reboot() {
    NVIC_SystemReset();
    while (1);
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void EnvoiSigfox(){
    if (!test) {
      SigFox.begin(); delay(100);             // Start the module, Wait at least 30ms after first configuration (100ms before)
    }
    if (oneshot && !test) {
      Serial.println("SigFox FW version " + SigFox.SigVersion());
      Serial.println("ID  = " + SigFox.ID() + "  PAC = " + SigFox.PAC());
    }
    if (oneshot || test) {
      Serial.println("mes[0].valeur : " + String(mes[0].valeur));
      Serial.println("payload: " + String(payload.msg1) + " " + String(payload.msg2) + " " + String(payload.msg3));
    }
    if (!test) {
      SigFox.status(); delay(1);
      SigFox.beginPacket();
      SigFox.write((uint8_t*)&payload, 12);
      lastMessageStatus = SigFox.endPacket();
      SigFox.end();
    }
    if (oneshot) {
      Serial.println(" LastStatus (0 = OK): " + String(lastMessageStatus));
    }
    // spin forever, so we can test that the backend is behaving correctly
    //if (oneshot && !test) {
    //  while (1) {}
    //}
    //Sleep pendant la période considérée
    //if (!test) {
      //LowPower.sleep(periode * 1000);
    //}
  }  
#endif
