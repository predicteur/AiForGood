#ifdef RESEAUWIFI
//-----------------------------------------------------------------------------------------------------------------------------  
  void SendMesure(){      // construire le JSON pour la page web
    String valeurInst = "{\"PM25\" : " + String(pm[0]) + ", \"PM10\" : " + String(pm[1]) + "}";
    Serial.print("json mesure: "); Serial.println(valeurInst);
    server.send(200, "application/json", valeurInst);
    Serial.println("envoi mesures");
  }
#endif
//-----------------------------------------------------------------------------------------------------------------------------
  void StripAffiche(String modeStrip){
    if        (modeStrip == "démarrage") {     
        strip.setBrightness(LUMINOSITE_FAIBLE);
        strip.setPixelColor(0, ORANGE[0], ORANGE[1], ORANGE[2]);
    } else if (modeStrip == "démarré") {       
        strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]);
    } else if (modeStrip == "wifi non connecté") {
        delay(500); strip.setPixelColor(0, ORANGE[0], ORANGE[1], ORANGE[2]);
        strip.show();
        delay(500); strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]);
    } else if (modeStrip == "correct") {     
        strip.setPixelColor(0, VERT[0], VERT[1], VERT[2]);
    } else if (modeStrip == "moyen") {     
        strip.setPixelColor(0, ORANGE[0], ORANGE[1], ORANGE[2]);
    } else if (modeStrip == "mauvais") {         
        strip.setPixelColor(0, ROUGE[0], ROUGE[1], ROUGE[2]);
    } else if (modeStrip == "début mesure") { 
        strip.setBrightness(LUMINOSITE_FORTE);
    } else if (modeStrip == "fin mesure") {  
        strip.setBrightness(LUMINOSITE_FAIBLE);
    }
    strip.show();
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void TraitementMesure(){
    Serial.print("Nombre de mesures : "); Serial.print(mes[0].nombre); Serial.print(" Taux d erreur : "); Serial.println(mes[0].tauxErreur);
    Serial.print(" Valeur : "); Serial.print(mes[0].valeur); Serial.print(" Ecart-type : "); Serial.print(mes[0].ecartType);
    Serial.print(" Ressenti : "); Serial.print(sensorVal1); Serial.print(sensorVal2); Serial.println(sensorVal3);
    if ((mes[0].nombreOk > 0) and (mes[1].nombreOk > 0)) {
        sendData();
        updateLed();
    } 
    else {
        Serial.println("mesure non envoyée");
    }
#ifdef COMPRESSION
    nombre_mesure ++;
#endif
  }
  void LireCapteur(){
    PmResult pm_val = sds.queryPm();
    if (pm_val.isOk()) {
      pm[0] = pm_val.pm25;
      pm[1] = pm_val.pm10;
    } else {
#ifdef BOARDSIGFOX
      SigFox.begin(); delay(100);      
      pm[0] = SigFox.internalTemperature();
      pm[1] = SigFox.internalTemperature();
      SigFox.end();
#else
      pm[0] = 100.0 * cos(3.14159 * millis());
      pm[1] = 100.0 * cos(3.14159 * millis());
#endif
      Serial.println("mesure capteur : " + String(pm[0]));
    }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void sendData() { 
#ifdef RESEAUWIFI

    // Génération du json
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["device"]            = DEVICE_NAME;
    root["PM2_5"]             = String(mes[0].valeur, 2);
    root["PM10"]              = String(mes[1].valeur, 2);
  //  root["ecart-type_PM25"]   = String(mes[0].ecartType, 2);
  //  root["ecart-type_PM10"]   = String(mes[1].ecartType, 2);
  //  root["taux-erreur_PM25"]  = String(mes[0].tauxErreur, 2);
  //  root["taux-erreur_PM10"]  = String(mes[1].tauxErreur, 2);
    root["positive_feeling"]  = String(sensorVal1, 2);
    root["mixed_feeling"]     = String(sensorVal2, 2);
    root["negative_feeling"]  = String(sensorVal3, 2);
    root.printTo(Serial);
    char JSONmessageBuffer[300];
    root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
  
    // Envoi du json
    if (WiFi.status() == WL_CONNECTED) {                                  
      HTTPClient http;
      http.begin(SERVEUR_AI4GOOD);
      http.addHeader("Content-Type", "application/json");                 //Specify content-type header
      int httpCode = http.POST(JSONmessageBuffer);                        //Send the request
      String payload = http.getString();                                  //Get the response payload
      Serial.println(httpCode); Serial.println(payload);                  //Print HTTP return code and request response payload
      http.end();                                                         //Close connection
    } else {
      StripAffiche("wifi non connecté");
    }
#else
    for (int i = 0; i < NB_MES; ++i) {
      mesEnvoi[i][nombre_mesure].nom          = mes[i].nom;
      mesEnvoi[i][nombre_mesure].nombre       = mes[i].nombre;
      mesEnvoi[i][nombre_mesure].nombreOk     = mes[i].nombreOk;
      mesEnvoi[i][nombre_mesure].valeur       = mes[i].valeur;
      mesEnvoi[i][nombre_mesure].ecartType    = mes[i].ecartType;
      mesEnvoi[i][nombre_mesure].date         = mes[i].date;
      mesEnvoi[i][nombre_mesure].valeurMin    = mes[i].valeurMin;
      mesEnvoi[i][nombre_mesure].valeurMax    = mes[i].valeurMax;
      mesEnvoi[i][nombre_mesure].tauxErreur   = mes[i].tauxErreur;
    }
#endif
    //stockage en mémoire à mettre
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void updateLed() { 
    if        (mes[0].valeur < SEUIL_BON_PM)    { StripAffiche("correct");
    } else if (mes[0].valeur < SEUIL_MOYEN_PM)  { StripAffiche("moyen");
    } else                                      { StripAffiche("mauvais");
    }
  }
#ifdef COMPRESSION
//-----------------------------------------------------------------------------------------------------------------------------
  void genereSerie() {
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
  void envoiSigfox(){
    if (!test) {
      SigFox.begin(); delay(100);             // Start the module, Wait at least 30ms after first configuration (100ms before)
    }
    if (oneshot && !test) {
      Serial.println("SigFox FW version " + SigFox.SigVersion());
      Serial.println("ID  = " + SigFox.ID());
      Serial.println("PAC = " + SigFox.PAC());
    }
    if (oneshot || test) {
      Serial.print("mes[0].valeur : "); Serial.println(mes[0].valeur);
      Serial.print("payload: "); Serial.println(String(payload.msg1) + " " + String(payload.msg2) + " " + String(payload.msg3));
    }
    if (!test) {
      SigFox.status(); delay(1);
      SigFox.beginPacket();
      SigFox.write((uint8_t*)&payload, 12);
      lastMessageStatus = SigFox.endPacket();
      SigFox.end();
    }
    if (oneshot) {
      Serial.print("message: "); Serial.print(payload.msg1); Serial.println(" LastStatus (0 = OK): " + String(lastMessageStatus));
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
