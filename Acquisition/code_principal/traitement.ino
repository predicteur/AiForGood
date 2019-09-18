//-----------------------------------------------------------------------------------------------------------------------------
  void Log(int type, String objet, String parametre) {
    String url = SERVEUR_AI4GOOD_LOG;
    String typeS = "infos---";
    String JSONlog = "";
    String JSONretour = "";
    if        (type == 1) {
      typeS = "warning-";
    } else if (type == 2) {
      typeS = "error---";
    } else if (type == 3) {
      typeS = "detail--";
    } else if (type == 4) {
      typeS = "debug---";
    }
    if ((MODE_LOG == "normal" & type < 3) | (MODE_LOG == "verbose" & type < 4) | (MODE_LOG == "debug" & type <5)) {
      Serial.println(theme + " : " + typeS + " " + objet + " -- " + parametre);
    }
    if ((type < 3) & (tokenExpire > 0)) {                                           // envoi des logs
      root.clear();
      root["equipement"]        = DEVICE_NAME;
      root["date"]              = CalculDate(millis()/100);
      root["type"]              = typeS;
      root["theme"]             = theme;
      root["objet"]             = objet;
      root["parametre"]         = parametre;
      if (measureJson(root) < (TAILLE_MAX_JSON-10)) {
        serializeJson(root, JSONlog);
      }
      if (JSONlog == "") {
        Log(2, "taille JSON superieure a la taille maxi", "");
      } else {
        JSONretour = EnvoiJSON(url, JSONlog);
        if ((JSONretour == "") & (objet != "retour serveur log vide")) {
          Log(2, "retour serveur log vide", "");
        }
      }
    }
  }

//-----------------------------------------------------------------------------------------------------------------------------
  void UpdateLed() { 
    if        (mesValeurLED < SEUIL_BON_PM)    { StripAffiche("correct");
    } else if (mesValeurLED < SEUIL_MOYEN_PM)  { StripAffiche("moyen");
    } else                                     { StripAffiche("mauvais");
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
    int milli = date % 10;
    date /= 10;
    int seconde = date % 60;
    date /= 60;
    int minute = date % 60;
    date /= 60;
    int heure = date % 24;
    int jour = date / 24;
    if (jour <= 31) {
      mois = "08";  
    } else if (jour <= 61) {                                        //à compléter pour les autres mois
      mois = "09";
      jour -= 31;
    }
    String chaine = "2019-" + mois + "-" + (String)jour + "T" + (String)heure + ":" + (String)minute + ":" + (String)seconde + "." + (String)milli + "Z";
    //Log(4, chaine + " DtoS JHMS : " + String(jour) + String(heure) + String(minute) + String(seconde), "");
    return "2019-" + mois + "-" + (String)jour + "T" + (String)heure + ":" + (String)minute + ":" + (String)seconde + "." + (String)milli + "00Z";
  }
//-----------------------------------------------------------------------------------------------------------------------------
  unsigned long StringToDate(String chaine){                        // millisecondes à partir du 1/8/2019 à 0h
    unsigned long date      = 0;
    if (chaine.length() > 15) {
      int           annee     = chaine.substring(0,4).toInt();
      int           mois      = chaine.substring(5,7).toInt();
      int           jour      = chaine.substring(8,10).toInt();
      int           heure     = chaine.substring(11,13).toInt();
      int           minute    = chaine.substring(14,16).toInt();
      int           seconde   = chaine.substring(17,19).toInt();
      int           milli     = chaine.substring(20,21).toInt();
      //Serial.println(String(annee) + String(mois) + String(jour) + String(heure) + String(minute) + String(seconde) + String(milli));
      if ((annee < 2019) || (annee > 2020) || (mois > 12) || (jour > 31) || (heure > 23) || (minute > 59) || (seconde > 59) || (milli > 9)) {
        Log(2, "date serveur non correcte ", "");
      } else {
        if (mois == 9)  { date += 31*24*60*60*10;}
        if (mois == 10) { date += 61*24*60*60*10;}
        if (mois == 11) { date += 92*24*60*60*10;}                      //à compléter pour les autres mois
        date += jour*24*60*60*10;
        date += heure*60*60*10;
        date += minute*60*10;
        date += seconde*10;
        date += milli;    
        //Serial.println( chaine + " StoD JHMS : " + String(jour) + String(heure) + String(minute) + String(seconde));
      }
    } else {
      Log(2, "date serveur non exploitable (taille < 15) : ", String(chaine.length()));
    }
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
    Log(4, modeStrip, "");
    if (niveauBatterieBas) { niveau = min(niveauAffichage, niveauFaible); }     
        strip.setBrightness(LUMINOSITE_FAIBLE * niveau / 100);
    if        (modeStrip == "démarrage") {     
        strip.setBrightness(LUMINOSITE_FAIBLE * niveau / 100);
        strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]);
    } else if (modeStrip == "démarré") {       
        strip.setPixelColor(0, VERT[0], VERT[1], VERT[2]);
        Log(0, modeStrip, "");
    } else if (modeStrip == "mode veille SDS") {       
        strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]);
        if ((millis() - timerVeille) > 6000) {                                                // clignotement
          timerVeille = millis();
        } else if ((millis() - timerVeille) > 4000){
          strip.setBrightness(LUMINOSITE_FAIBLE * niveauFaible / 100);
        } else {
          strip.setBrightness(NIVEAU_ETEINT);          
        }
    } else if (modeStrip == "mesure non envoyee") {
        strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]);
        Log(0, modeStrip, "");
    } else if (modeStrip == "mesure capteur sds erronee") {
        strip.setPixelColor(0, VIOLET[0], VIOLET[1], VIOLET[2]);
    } else if (modeStrip == "correct") {     
        strip.setPixelColor(0, VERT[0], VERT[1], VERT[2]);
    } else if (modeStrip == "moyen") {     
        strip.setPixelColor(0, ORANGE[0], ORANGE[1], ORANGE[2]);
    } else if (modeStrip == "mauvais") {         
        strip.setPixelColor(0, ROUGE[0], ROUGE[1], ROUGE[2]);
    } else if (modeStrip == "début mesure") { 
        strip.setBrightness(LUMINOSITE_FORTE * niveau / 100);
    } else if (modeStrip == "fin mesure") {  
        strip.setBrightness(LUMINOSITE_FAIBLE * niveau / 100);
    }
    strip.show();
  }
//-----------------------------------------------------------------------------------------------------------------------------
  boolean MesureOk() {
    return (mes[M_PM25].nombreOk > 0) and (mes[M_PM10].nombreOk > 0);
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void PrMesure(int level){
    theme = "capteur ";
    Log(level, "   Nombre de mesures : " + String(mes[mesureLED].nombre) + " Taux d erreur : " + String(mes[mesureLED].tauxErreur), "");
    Log(level, "   Valeur : " + String(mes[mesureLED].valeur) + " Ecart-type : " + String(mes[mesureLED].ecartType),"");
    Log(level, "   Ressenti : " + ressenti + " Date : " + CalculDate(mes[mesureLED].date), "");
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void LireCapteur(){
    theme = "capteur ";
    PmResult pm_val = sds.queryPm();
    if (pm_val.isOk()) {
      UpdateLed();                                    // remet la bonne couleur si une mesure mauvaise a eu lieu avant
      pm[M_PM25] = pm_val.pm25;
      pm[M_PM10] = pm_val.pm10;
    } else {
      Log(2, "mesure capteur sds erronee ", "");
      StripAffiche("mesure capteur sds erronee");
#ifdef BOARDSIGFOX
      SigFox.begin(); delay(100);      
      pm[M_PM25] = SigFox.internalTemperature();
      pm[M_PM10] = SigFox.internalTemperature();
      SigFox.end();
#else
      pm[M_PM25] = float(ESP.getVcc())/10.0;
      pm[M_PM10] = 100.0 * cos(3.14159 * millis());
#endif
    }
    Log(3, "mesure capteur : " + String(pm[mesureLED]), "");
  }

#ifdef RESEAUWIFI
//-----------------------------------------------------------------------------------------------------------------------------
  String GenereJSON() { 
    root.clear();
    String JSONmessage = "";
    root["device"]            = DEVICE_NAME;
    root["PM25"]              = String(mes[M_PM25].valeur, 2);
    root["PM10"]              = String(mes[M_PM10].valeur, 2);
    root["date_mesure"]       = CalculDate(mes[M_PM25].date);
    root["ecart_type_PM25"]   = String(mes[M_PM25].ecartType, 2);
    root["ecart_type_PM10"]   = String(mes[M_PM10].ecartType, 2);
    root["taux_erreur_PM25"]  = String(mes[M_PM25].tauxErreur, 2);
    root["taux_erreur_PM10"]  = String(mes[M_PM10].tauxErreur, 2);
    root["niv_batt"]          = " ";
    root["feeling"]           = ressenti;
    root["latitude"]          = String(48.748010);
    root["longitude"]         = String(2.293491);
    if (measureJson(root) < (TAILLE_MAX_JSON-10)) {
      serializeJson(root, JSONmessage);
    }
    return JSONmessage;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void MajToken() { 
    int httpCode = 0;
    HTTPClient http;
    http.begin(SERVEUR_AI4GOOD_LOGIN, SERVEUR_AI4GOOD_FINGER);
    //http.addHeader("Content-Type", "application/json");                 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    //  Serial.println("username=" + String(AI4GOOD_USERNAME) + "&password=" + String(AI4GOOD_PASSWORD));    
    httpCode = http.POST("username=" + String(AI4GOOD_USERNAME) + "&password=" + String(AI4GOOD_PASSWORD));
    String retourLogin = http.getString();                                  
    http.end(); 
    root.clear();
    DeserializationError err = deserializeJson(root, retourLogin);
    if (err) {
      Log(2, "deserialisation reponse envoi login errone : " + String(err.c_str()), "");
    } else {

      // calcul de token
      Log(4, "payload token :" + retourLogin, "");    
      String sensor = root["token"]; 
      tokenValeur   = String("Bearer " + sensor);
      String expDate = root["exp_date"];
      tokenExpire   = StringToDate(expDate);
      Log(4, "token (en sec) : " + String(tokenExpire/10) + tokenValeur, "");    

      // calcul de date
      unsigned long dateLogin = millis()/100;
      const char* dateServeur = root["date"];
      if (StringToDate(dateServeur) > 0) {
        dateRef.dateExt = StringToDate(dateServeur);
        dateRef.dateInt = dateLogin;
        dateRef.decalage = dateRef.dateExt - dateRef.dateInt;
      }
    }
    delay(10000);  
  }
//-----------------------------------------------------------------------------------------------------------------------------
  String EnvoiJSON(String url, String JSONmes) { 
    String retour = "";
    int httpCode = 0;
    if (WiFi.status() == WL_CONNECTED) {                                     
      Log(4, "délai token restant en sec : " + String((tokenExpire - millis()/100 - dateRef.decalage)/10), "");
      if (tokenExpire - millis()/100 - dateRef.decalage < 10*60*10) { MajToken(); }         // marge de 10 minutes avant expiration
      HTTPClient http;
      //http.begin("https://ai-for-good-api.herokuapp.com/send/data", "08 3B 71 72 02 43 6E CA ED 42 86 93 BA 7E DF 81 C4 BC 62 30‎"); // url pour envoyer les mesures
      http.begin(url, SERVEUR_AI4GOOD_FINGER);
      http.addHeader("Authorization", tokenValeur);
      http.addHeader("Content-Type", "application/json");             
      httpCode = http.POST(JSONmes);                       
      retour  = http.getString();                                
      http.end(); 
      if (httpCode == 401 | httpCode == 403) {
        tokenExpire = 0;
        Log(1, "token absent, expire ou non valide ", String(httpCode));
        MajToken();
      }
      Log(4, "JSONmes DATA : " + url + "  " + JSONmes, "" );
      Log(4, "retour DATA  : " + url + "  " + retour, "");
      Log(4, "httpCode     : " + String(httpCode) + " " + url, "");
    } else {
      Log(2, "wifi deconnecte", "");
    } 
    return retour;    
  }
//-----------------------------------------------------------------------------------------------------------------------------
  /*void RecalageDate(unsigned long dateReponseInt, String JSONdata) {               // lecture de la date serveur pour recalage
    theme = "wifi    ";    
    root.clear();
    Log(4, "JSONdata recalage : " + JSONdata, "");
    DeserializationError err = deserializeJson(root, JSONdata);
    if (err) {
      Log(2, "deserialisation reponse envoi data errone : " + String(err.c_str()), "");
    } else {
      const char* dateServeur = root["date"];
      Log(4, "av : " + CalculDate(dateReponseInt) +" ap : " + dateServeur, "");
      if (StringToDate(dateServeur) > 0) {
        dateRef.dateExt = StringToDate(dateServeur);
        dateRef.dateInt = dateReponseInt;
        dateRef.decalage = dateRef.dateExt - dateRef.dateInt;
      }
    }
  }*/
//-----------------------------------------------------------------------------------------------------------------------------
  void AjouteMesure(String JSONmesure) {                                  // Stockage des donnnées non envoyées
    theme = "esp     ";
    StripAffiche("mesure non envoyee");
    ficMes = SPIFFS.open(FIC_BUF, "r");
    if (!ficMes) {
      Log(2, "ouverture fichier impossible", "");
    } else {
      String total = ficMes.readString();
      ficMes.close(); delay(50);
      ficMes = SPIFFS.open(FIC_BUF, "w");
      if (!ficMes) {
        Log(2, "ouverture fichier impossible", "");
      } else {
        total = total + "\n" + JSONmesure;
        //total = total + JSONmesure;
        ficMes.print(total);
        ficMes.close();
        Log(3, "total stocke apres ajout : \n" + total, "");
      }
    }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void EnvoiWifi() { 
    String url = SERVEUR_AI4GOOD_DATA;
    String retourData = "";
    theme = "wifi    ";
    String JSONdata = GenereJSON();
    if (JSONdata == "") {
      Log(2, "taille JSON superieure a la taille maxi", "");
      StripAffiche("mesure non envoyee");
    } else {
      retourData = EnvoiJSON(url, JSONdata);
      Log(4, "retour data avant envoi : " + retourData, "");
      //unsigned long dateReponseInt = millis()/100;
      if (retourData == "") { 
        StripAffiche("mesure non envoyee");
        Log(2, "retour serveur data vide ", "");
        AjouteMesure(JSONdata);
      //} else {
      //  RecalageDate(dateReponseInt, retourData);          // calcul de date en dixieme de seconde
      }
    }
  }
//-----------------------------------------------------------------------------------------------------------------------------  
  void RepriseEnvoiWifi() {
    String url = SERVEUR_AI4GOOD_DATA;
    String mesureNonReprise = "";
    String JSONdata = "";
    String retourData = "";
    theme = "wifi    ";
    ficMes = SPIFFS.open(FIC_BUF, "r");
    if (!ficMes) {
      Log(2, "ouverture fichier impossible", "");
    } else {
      while(ficMes.available()>5) {    
        JSONdata = ficMes.readStringUntil('\n');
        if ((JSONdata.length() > 10) && (JSONdata.length() < TAILLE_MAX_JSON))  {
          retourData = EnvoiJSON(url, JSONdata);
          if (retourData == "") {
            mesureNonReprise += ("\n" + JSONdata);
          }
        }
      }
      ficMes.close(); delay(50);
      ficMes = SPIFFS.open(FIC_BUF, "w");
      if (!ficMes) {
        Log(2, "ouverture fichier impossible", "");
      } else {
        ficMes.print(mesureNonReprise);
        ficMes.close();
      }
      if (mesureNonReprise.length() > 10) {
        Log(3, "total stocke apres reprise : \n" + mesureNonReprise, "");
      }
    }
  }
//-----------------------------------------------------------------------------------------------------------------------------  
  void AjustementParametres(){
    theme = "param   ";
    if (WiFi.status() == WL_CONNECTED) {                                  
      HTTPClient http;
      http.begin(SERVEUR_AI4GOOD_VAR);
      http.addHeader("Content-Type", "application/json");                 
      int httpCode = http.GET();
      String parametres = http.getString();                                  
      http.end();        
      Log(4, "ajustement des parametres  : " + parametres, "");
      if ((parametres == "")|(httpCode != 200)|(httpCode != 201)) { 
        Log(2, "lecture des parametres indisponible", ""); 
      } else {
        boolean erreur = ActualisationParametres(parametres);    
      }
    } else {
      Log(2, "wifi déconnecte", "");
    }
  }

//-----------------------------------------------------------------------------------------------------------------------------  
  boolean ActualisationParametres(String parametres){      
    theme = "param   ";
    root.clear();
    DeserializationError err = deserializeJson(root, parametres);
    if (err) {
      Log(2, "impossible de decoder les parametres", "");
    } else {
      
      const char* pModeFonctionnement = root["modeFonctionnement"];
      if ((pModeFonctionnement != nullptr) & ((pModeFonctionnement == MODE_ECO)
        | (pModeFonctionnement == MODE_NORMAL) | (pModeFonctionnement == MODE_FORT)
        |  (pModeFonctionnement == MODE_VEILLE))) { 
        if (String(pModeFonctionnement) != modeFonctionnement) {
          Log(0,"nouveau mode de fonctionnement : ", String(pModeFonctionnement)); }
        modeFonctionnement = String(pModeFonctionnement); 
      } else if (pModeFonctionnement != "null") {
        Log(2,"mode de fonctionnement incorrect : ", "");        
      }
      const char* pRessenti = root["ressenti"];
      if ((pRessenti != nullptr) & ((pRessenti == RESSENTI_BIEN)
        | (pRessenti == RESSENTI_NORMAL) | (pRessenti == RESSENTI_PASBIEN))) { 
        if (String(pRessenti) != ressenti) {
          Log(0,"nouveau ressenti : ", String(pRessenti)); }
        ressenti = String(pRessenti); 
      } else if (pRessenti != "null")  {
        Log(2,"ressenti incorrect : ", "");        
      }
      const int pMesureLED = root["MESURE_LED"];
      if ((pMesureLED > -1) & (pMesureLED < NB_MES)) {
        if (pMesureLED != mesureLED) {
          Log(0,"nouvelle mesure a afficher sur la LED : ", String(pMesureLED)); }
        mesureLED = pMesureLED;
      } else {
        Log(2,"mesure a afficher sur la LED incorrecte : ", "");
      }
      const int pTempsCycle = root["TEMPS_CYCLE"];
      if ((pTempsCycle > 0) & (pTempsCycle < 3600000)) {
        if (pTempsCycle != tempsCycle) {
          Log(0,"nouveau temps de cycle en ms : ", String(pTempsCycle)); }
        tempsCycle = pTempsCycle;
      } else if (pTempsCycle != 0) {
        Log(2,"temps de cycle incorrect : ", "");
      }
      const int pNiveauFort = root["NIVEAU_FORT"];
      if ((pNiveauFort > 0) & (pNiveauFort < 101)) {
        if (pNiveauFort != niveauFort) {
          Log(0,"nouveau niveau fort : ", String(pNiveauFort)); }
        niveauFort = pNiveauFort;
      } else if (pNiveauFort != 0) {
        Log(2,"niveau fort incorrect : ", "");
      }
      const int pNiveauMoyen = root["NIVEAU_MOYEN"];
      if ((pNiveauMoyen > 0) & (pNiveauMoyen < 101)) {
        if (pNiveauMoyen != niveauMoyen) {
          Log(0,"nouveau niveau moyen : ", String(pNiveauMoyen)); }
        niveauMoyen = pNiveauMoyen;
      } else if (pNiveauMoyen != 0) {
        Log(2,"niveau moyen incorrect : ", "");
      }
      const int pNiveauFaible = root["NIVEAU_FAIBLE"];
      if ((pNiveauFaible > 0) & (pNiveauFaible < 101)) {
        if (pNiveauFaible != niveauFaible) {
          Log(0,"nouveau niveau faible : ", String(pNiveauFaible)); }
        niveauFaible = pNiveauFaible;
      } else if (pNiveauFaible != 0) {
        Log(2,"niveau faible incorrect : ", "");
      }
    }
    return err;
  }  
//-----------------------------------------------------------------------------------------------------------------------------  
  void SendMesureWeb(){      
    theme = "page web";
    if( ! server.hasArg("parametre") || server.arg("parametre") == NULL ) {
      server.send(400, "text/plain", "400: Invalid Request");
      Log(2, "retour page web errone", "");
      return;
    }
    // récupération des parametres
    String parametres = server.arg("parametre");
    Log(4, "parametres : " + parametres, "");
    boolean erreur = ActualisationParametres(parametres);
    
    // envoi des valeurs instantnées
    String valeurInst = "{\"PM25\" : " + String(pm[M_PM25]) + ", \"PM10\" : " + String(pm[M_PM10]) + "}";
    Log(4, "json mesure: " + valeurInst, "");
    server.send(200, "application/json", valeurInst);
    Log(3, "envoi mesures", "");
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
      y0init[i] = mesEnvoi[mesureLED][i].valeur;
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
    theme = "sigfox  ";
    if (!test) {
      SigFox.begin(); delay(100);             // Start the module, Wait at least 30ms after first configuration (100ms before)
    }
    if (oneshot && !test) {
      Log(4, "SigFox FW version " + SigFox.SigVersion(), "");
      Log(4, "ID  = " + SigFox.ID() + "  PAC = " + SigFox.PAC(), "");
    }
    if (oneshot || test) {
      Log(4, "mes[mesureLED].valeur : " + String(mes[mesureLED].valeur), "");
      Log(4, "payload: " + String(payload.msg1) + " " + String(payload.msg2) + " " + String(payload.msg3), "");
    }
    if (!test) {
      SigFox.status(); delay(1);
      SigFox.beginPacket();
      SigFox.write((uint8_t*)&payload, 12);
      lastMessageStatus = SigFox.endPacket();
      SigFox.end();
    }
    if (oneshot) {
      Log(4, " LastStatus (0 = OK): " + String(lastMessageStatus), "");
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
