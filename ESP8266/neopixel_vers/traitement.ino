
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
}

void LireCapteur(){
  PmResult pm_val = sds.queryPm();
  if (pm_val.isOk()) {
    pm[0] = pm_val.pm25;
    pm[1] = pm_val.pm10;
  }
}

void sendData() {
  
  // Génération du json
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["device"]            = DEVICE_NAME;
  root["PM2_5"]             = String(mes[0].valeur, 2);
  root["PM10"]              = String(mes[1].valeur, 2);
//  root["ecart-type_PM25"]   = String(mes[0].ecartType, 2);
//  root["ecart-type_PM10"]   = String(mes[1].ecartType, 2);
//  root["taux-erreur_PM25"]   = String(mes[0].tauxErreur, 2);
//  root["taux-erreur_PM10"]   = String(mes[1].tauxErreur, 2);
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
}

void updateLed() { 
  if        (mes[0].valeur < SEUIL_BON_PM)    { StripAffiche("correct");
  } else if (mes[0].valeur < SEUIL_MOYEN_PM)  { StripAffiche("moyen");
  } else                                      { StripAffiche("mauvais");
  }
}

