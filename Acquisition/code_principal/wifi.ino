#ifdef RESEAUWIFI
//-----------------------------------------------------------------------------------------------------------------------------
  String GenereJSON() { 
    root.clear();
    String JSONmessage = "";
    root["device"]            = DEVICE_NAME;
    root["pm25"]              = String(mes[M_PM25].valeur, 2);
    root["pm10"]              = String(mes[M_PM10].valeur, 2);
    root["pm25_filtree"]      = String(mes[M_PM25].valeurFiltree, 2);
    root["pm10_filtree"]      = String(mes[M_PM10].valeurFiltree, 2);
    root["date_mesure"]       = CalculDate(mes[M_PM25].date);
    root["ecart_type_pm25"]   = String(mes[M_PM25].ecartType, 2);
    root["ecart_type_pm10"]   = String(mes[M_PM10].ecartType, 2);
    root["taux_erreur_pm25"]  = String(mes[M_PM25].tauxErreur, 2);
    root["taux_erreur_pm10"]  = String(mes[M_PM10].tauxErreur, 2);
    //root["niv_batt"]          = " ";
    root["feeling"]           = ressenti;
    root["latitude"]          = latitude;
    root["longitude"]         = longitude;
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
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
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
          if (retourData == "") { mesureNonReprise += ("\n" + JSONdata); }
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
      if ((parametres == "")|((httpCode != 200)&(httpCode != 201))) { 
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
        
      const char* pModeLog = root["MODE_LOG"];
      if ((pModeLog != nullptr) & ((String(pModeLog) == "normal")
        | (String(pModeLog) == "verbose") | (String(pModeLog) == "debug"))) { 
        if (String(pModeLog) != modeLog) {
          Log(0,"nouveau mode de log : ", String(pModeLog)); }
        modeLog = String(pModeLog); 
      } else {
        Log(4,"mode de log incorrect : ", String(pModeLog));        
      }
      const char* pModeFonctionnement = root["MODE_FONCTIONNEMENT"];
      if ((pModeFonctionnement != nullptr) & ((String(pModeFonctionnement) == MODE_ECO)
        | (String(pModeFonctionnement) == MODE_NORMAL) | (String(pModeFonctionnement) == MODE_FORT)
        |  (String(pModeFonctionnement) == MODE_VEILLE))) { 
        if (String(pModeFonctionnement) != modeFonctionnement) {
          Log(0,"nouveau mode de fonctionnement : ", String(pModeFonctionnement)); }
        modeFonctionnement = String(pModeFonctionnement); 
      } else {
        Log(4,"mode de fonctionnement incorrect : ", String(pModeFonctionnement));        
      }
      const char* pRessenti = root["RESSENTI"];
      if ((pRessenti != nullptr) & ((String(pRessenti) == RESSENTI_BIEN)
        | (String(pRessenti) == RESSENTI_NORMAL) | (String(pRessenti) == RESSENTI_PASBIEN))) { 
        if (String(pRessenti) != ressenti) {
          Log(0,"nouveau ressenti : ", String(pRessenti)); }
        ressenti = String(pRessenti); 
      } else {
        Log(4,"ressenti incorrect : ", String(pRessenti));        
      }
      int pMesureLED = root["MESURE_LED"];
      if ((pMesureLED > -1) & (pMesureLED < NB_MES)) {
        if (pMesureLED != mesureLED) {
          Log(0,"nouvelle mesure a afficher sur la LED : ", String(pMesureLED)); }
        mesureLED = pMesureLED;
      } else {
        Log(4,"mesure a afficher sur la LED incorrecte : ", String(pMesureLED));
      }
      int pTempsCycle = root["TEMPS_CYCLE"];
      if ((pTempsCycle > 2000) & (pTempsCycle < 3600000)) {
        if (pTempsCycle != tempsCycle) {
          Log(0,"nouveau temps de cycle en ms : ", String(pTempsCycle)); }
        tempsCycle = pTempsCycle;
      } else {
        Log(4,"temps de cycle incorrect : ", "");
      }
      int pNiveauFort = root["NIVEAU_FORT"];
      if ((pNiveauFort > 0) & (pNiveauFort < 101)) {
        if (pNiveauFort != niveauFort) {
          Log(0,"nouveau niveau fort : ", String(pNiveauFort)); }
        niveauFort = pNiveauFort;
      } else {
        Log(4,"niveau fort incorrect : ", String(pNiveauFort));
      }
      int pNiveauMoyen = root["NIVEAU_MOYEN"];
      if ((pNiveauMoyen > 0) & (pNiveauMoyen < 101)) {
        if (pNiveauMoyen != niveauMoyen) {
          Log(0,"nouveau niveau moyen : ", String(pNiveauMoyen)); }
        niveauMoyen = pNiveauMoyen;
      } else {
        Log(4,"niveau moyen incorrect : ", "");
      }
      int pNiveauFaible = root["NIVEAU_FAIBLE"];
      if ((pNiveauFaible > 0) & (pNiveauFaible < 101)) {
        if (pNiveauFaible != niveauFaible) {
          Log(0,"nouveau niveau faible : ", String(pNiveauFaible)); }
        niveauFaible = pNiveauFaible;
      } else {
        Log(4,"niveau faible incorrect : ", "");
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
    Log(3, "envoi mesures", valeurInst);
  }
#endif
