#ifdef RESEAUWIFI
//-----------------------------------------------------------------------------------------------------------------------------
  void CallbackNouveauReseau(WiFiManager *wifiManager) {
    StripAffiche("nouveau reseau");
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void DemarreWiFi(boolean memIdentifiant, WiFiManager *wifiManager) {
    byte mac[6];
    if (!memIdentifiant) wifiManager->resetSettings();             // garder en mémoire ou non les anciens identifiants
    boolean debugWiFi = (modeLog == String(LOG_DEBUG));
    wifiManager->setDebugOutput(debugWiFi);                        
    wifiManager->autoConnect(AUTO_CONNECT);                        // connexion automatique au réseau précédent si MEMIDENTIFIANT = 1
    StripAffiche("demarrage");
    WiFi.macAddress(mac);
    Log(3, "Connected to : " + WiFi.SSID(), WiFi.psk());
    devType.adresseIP = WiFi.localIP().toString();
    Log(0, "IP   address : ", devType.adresseIP );
    devType.deviceId = (String)mac[5] + " " + (String)mac[4] + " " + (String)mac[3] + " " + (String)mac[2] + " " + (String)mac[1] + " " + (String)mac[0];
    Log(0, "MAC  address : ", devType.deviceId );
    //Log(3, "MAC  address : ", String((char *)mac));
    MajToken();                                                    // initialisation token et date
    RepriseEnvoiWifiData();                                        // renvoi des mesures stockées dans SPIFFS
  }
//-----------------------------------------------------------------------------------------------------------------------------
  String GenereJSONparam() { 
    const size_t capacity = JSON_OBJECT_SIZE(10);
    DynamicJsonDocument param(capacity);
    String JSONmessage = "";
    param["device"]               = DEVICE_NAME;
    param["feeling"]              = device.ressenti;
    param["mode_fonctionnement"]  = modeFonc;
    param["mode_log"]             = modeLog;
    //param["mode_lum"]             = modeLuminosite;
    param["niv_batt"]             = String(device.niveauBatterie);
    serializeJson(param, JSONmessage);
    return JSONmessage;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void MajToken() { 
    if (!autonom) {
      int httpCode = 0;

      /*std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
      client->setFingerprint(SERVEUR_AI4GOOD_FINGER);
      HTTPClient http;
      http.begin(*client, SERVEUR_AI4GOOD_LOGIN);*/

      HTTPClient http;
      http.begin(SERVEUR_AI4GOOD_LOGIN, SERVEUR_AI4GOOD_FINGER);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      httpCode = http.POST("username=" + String(AI4GOOD_USERNAME) + "&password=" + String(AI4GOOD_PASSWORD));
      String retourLogin = http.getString();                                  
      http.end(); 
      const size_t capacity = JSON_OBJECT_SIZE(4) + 370;
      DynamicJsonDocument login(capacity);
      DeserializationError err = deserializeJson(login, retourLogin);
      if (err) Log(2, "deserialisation reponse envoi login errone : " + String(err.c_str()), "");
      else {
                    // calcul de token
        Log(4, "payload token :" + retourLogin, "");    
        String sensor = login["token"]; 
        tokenValeur   = String("Bearer " + sensor);
        String expDate = login["exp_date"];
                    // calcul de date

        String dateServeur = login["date"];
        dateRef = stringToDate(dateServeur);
        milliRef = millis();
        //unsigned long dateLogin = millis()/100;
        //const char* dateServeur = login["date"];
        //InitDateRef(dateLogin, dateServeur);
        tokenExpire = stringToDate(expDate);
        Serial.println(dateRef.timestamp);
        Serial.println(tokenExpire.timestamp);
        
        Log(4, "token expire (en sec) : " + String(tokenExpire.timestamp - dateRef.timestamp) + " " + tokenValeur, "");    
        
        EnvoiWifiDevice();  }
      delay(1000); }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  String EnvoiJSON(String url, String JSONmes) { 
    String retour = "";
    int httpCode = 0;
    if ((WiFi.status() == WL_CONNECTED) & !autonom) {                                     
      Log(4, "délai token restant en sec : " + String(tokenExpire.timestamp - dateRef.timestamp - (millis() - milliRef)/1000), "");
      if (tokenExpire.timestamp - dateRef.timestamp - (millis() - milliRef)/1000 < 10*60) MajToken();         // marge de 10 minutes avant expiration
      /*std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
      client->setFingerprint(SERVEUR_AI4GOOD_FINGER);    
      HTTPClient http;
      http.begin(*client, url);*/
      HTTPClient http;
      http.begin(url, SERVEUR_AI4GOOD_FINGER);
      http.addHeader("Authorization", tokenValeur);
      http.addHeader("Content-Type", "application/json");             
      httpCode = http.POST(JSONmes);                       
      retour  = http.getString();                                
      http.end(); 
      if (httpCode != 200 & httpCode != 201) {
        if (httpCode == 401 | httpCode == 403) {
          tokenExpire.setDate(1980, 1, 1);
          Log(1, "token absent, expire ou non valide ", String(httpCode));
          MajToken();  }
        retour ="";  }
      Log(4, "envoi serveur  : " + url + "  " + JSONmes,          "");
      Log(4, "retour serveur : " + url + "  " + retour,           "");
      Log(4, "httpCode       : " + url + "  " + String(httpCode), "");  }
    else Log(4, "pas d envoi wifi", "");
    return retour;    
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void AjouteJSON(String JSONmesure) {                                  // Stockage des donnnées non envoyées
    String total = "";
    theme = "esp     ";
    if (!autonom) StripAffiche("mesure non envoyee");
    else StripAffiche("mesure stockee");
    ficMes = SPIFFS.open(FIC_BUF, "r");
    if (!ficMes) Log(2, "ouverture fichier impossible", "");
    else {
      total = ficMes.readString();
      ficMes.close(); delay(50);
      ficMes = SPIFFS.open(FIC_BUF, "w");
      if (!ficMes) Log(2, "ouverture fichier impossible", "");
      else {
        total = total + "\n" + JSONmesure;
        totalMesureNonReprise++;
        etatMesureSature = (totalMesureNonReprise >= MAX_STOCKAGE - 1);
        if (etatMesureSature) Log(0, "stockage des mesures sature", "");
        ficMes.print(total);
        ficMes.close();
        Log(3, "total stocke apres ajout : ", String(totalMesureNonReprise));  }  }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void EnvoiWifiDevice() { 
    
    // à compléter, envoi devicetype et mesuretype
    
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void EnvoiWifiMesure() { 
    String url = SERVEUR_AI4GOOD_DATA;
    String retourData = "";
    theme = "wifi    ";
    String JSONdata = MesureGenereJSON();
    if (autonom) AjouteJSON(JSONdata);
    else if (JSONdata == "") {
      Log(1, "taille JSON superieure a la taille maxi", "");
      StripAffiche("mesure non envoyee");   }
    else {
      retourData = EnvoiJSON(url, JSONdata);
      if (retourData == "") { 
        StripAffiche("mesure non envoyee");
        Log(1, "retour serveur data vide ", "");
        AjouteJSON(JSONdata);   }   }
  }
//-----------------------------------------------------------------------------------------------------------------------------  
  void RepriseEnvoiWifiData() {
    String url = SERVEUR_AI4GOOD_DATA;
    String mesureNonReprise = "";
    String JSONdata = "";
    String retourData = "";
    int    total = 0;
    theme = "wifi    ";
    ficMes = SPIFFS.open(FIC_BUF, "r");
    if (!ficMes) Log(2, "ouverture fichier impossible", "");
    else {
      if(ficMes.available()>5 ) Log(0, "envoi en cours des mesures stockees", "");
      while(ficMes.available()>5) {    
        JSONdata = ficMes.readStringUntil('\n');
        if (JSONdata.length() > 10)  {
          retourData = EnvoiJSON(url, JSONdata);
          if (retourData == "") { 
            mesureNonReprise += ("\n" + JSONdata); 
            total++;  }  }  }
      ficMes.close(); delay(50);
      ficMes = SPIFFS.open(FIC_BUF, "w");
      if (!ficMes) Log(2, "ouverture fichier impossible", "");
      else {
        ficMes.print(mesureNonReprise);
        ficMes.close();
        totalMesureNonReprise = total;  }
        etatMesureSature = (totalMesureNonReprise >= MAX_STOCKAGE - 1);
      if (mesureNonReprise.length() > 10) {
        Log(3, "total stocke apres reprise : ", String(totalMesureNonReprise));  }  }
  }
//----------------------------------------------------------------------------------------------------------------------------  
  void AjustementVariables(){
    theme = "variable";
    if ((WiFi.status() == WL_CONNECTED) & !autonom) {                                  
      /*WiFiClient client;
      HTTPClient http;
      http.begin(client, SERVEUR_AI4GOOD_VAR);*/
      HTTPClient http;
      http.begin(SERVEUR_AI4GOOD_VAR);
      http.addHeader("Content-Type", "application/json");                 
      int httpCode = http.GET();
      String parametres = http.getString();                                  
      http.end();        
      Log(4, "ajustement des variables  : " + parametres, "");
      if ((parametres == "")|((httpCode != 200)&(httpCode != 201))) Log(2, "lecture des variables indisponible", ""); 
      else boolean erreur = ActualisationVariables(parametres);   }
    else Log(4, "pas de recuperation wifi de variables", "");
  }
//-----------------------------------------------------------------------------------------------------------------------------  
  boolean ActualisationVariables(String parametres){      
    theme = "variable";
    const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 5*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(15) + 260;
    DynamicJsonDocument param(capacity);
    DeserializationError err = deserializeJson(param, parametres);
    if (err) Log(2, "impossible de decoder les variables", "");
    else {
      JsonObject param0 = param[0];
      JsonObject param0bleu = param0["bleu"];
      int c1 = param0bleu["c1"];
      int c2 = param0bleu["c2"];
      int c3 = param0bleu["c3"];
      if ((c1 > -1) & (c1 < 256) & (c2 > -1) & (c2 < 256) & (c3 > -1) & (c3 < 256) ) {
        if ( (c1 != BLEU[0]) | (c2 != BLEU[1]) | (c3 != BLEU[2]) ) {
          Log(0,"nouvelle couleur bleu : ", String(c1) + " " + String(c2) + " " + String(c3));
          BLEU[0] = c1; BLEU[1] = c2; BLEU[2] = c3;  }  }
      else Log(4,"couleur bleu incorrecte : ", String(c1) + " " + String(c2) + " " + String(c3));
      int pNiveauFort = param0["niveau_fort"];
      
      if ((pNiveauFort > 0) & (pNiveauFort < 101)) {
        if (pNiveauFort != NIVEAU_FORT) {
          Log(0,"nouveau niveau fort : ", String(pNiveauFort)); 
          NIVEAU_FORT = pNiveauFort;  }  }
      else Log(4,"niveau fort incorrect : ", String(pNiveauFort));
      
      int pNiveauMoyen = param0["niveau_moyen"];
      if ((pNiveauMoyen > 0) & (pNiveauMoyen < 101)) {
        if (pNiveauMoyen != NIVEAU_MOYEN) {
          Log(0,"nouveau niveau moyen : ", String(pNiveauMoyen));
          NIVEAU_MOYEN = pNiveauMoyen;  }  }
      else Log(4,"niveau moyen incorrect : ", "");
      
      int pNiveauFaible = param0["niveau_faible"];
      if ((pNiveauFaible > 0) & (pNiveauFaible < 101)) {
        if (pNiveauFaible != NIVEAU_FAIBLE) {
          Log(0,"nouveau niveau faible : ", String(pNiveauFaible));
          NIVEAU_FAIBLE = pNiveauFaible;  }  }
      else Log(4,"niveau faible incorrect : ", "");
      
      int pTempsCycle = param0["temps_cycle"];
      if ((pTempsCycle > 2000) & (pTempsCycle < 3600000)) {
        if (pTempsCycle != device.tempsCycle) {
          Log(0,"nouveau temps de cycle en ms : ", String(pTempsCycle));
          device.tempsCycle = pTempsCycle;  }  }
      else Log(4,"temps de cycle incorrect : ", "");
    
      int pMesureLED = param0["mesure_led"];
      if ((pMesureLED > -1) & (pMesureLED < NB_MES)) {
        if (pMesureLED != device.mesureLED) {
          Log(0,"nouvelle mesure a afficher sur la LED : ", String(pMesureLED));
          device.mesureLED = pMesureLED;  }  }
      else Log(4,"mesure a afficher sur la LED incorrecte : ", String(pMesureLED));  }
    return err;
  }  
//-----------------------------------------------------------------------------------------------------------------------------  
  String ValideListe(const char* pVal, String val1, String val2, String val3, String valInit, String messageLog) {
    String valNew = String(pVal);
    String newVal = valInit;
    if ((pVal != nullptr) & ((valNew == val1) | (valNew == val2) | (valNew == val3))) { 
      if (valNew != valInit) {
        Log(0,"nouveau " + messageLog + " : ", valNew);
        newVal = valNew;  }  }
    else Log(4,messageLog +" incorrect : ", valNew);        
    return newVal;
  }
//-----------------------------------------------------------------------------------------------------------------------------  
  boolean ActualisationParametres(String parametres){      
    theme = "param   ";
    const size_t capacity = JSON_OBJECT_SIZE(10) + 180;
    DynamicJsonDocument param(capacity);
    DeserializationError err = deserializeJson(param, parametres);
    if (err) Log(2, "impossible de decoder les parametres", "");
    else {
      const char* pResetWiFi = param["reset_wifi"];
      resetWiFi = ValideListe(pResetWiFi, RESET_AUCUN, RESET_AUTO, RESET_MANU, resetWiFi, "mode de redemarrage");
      
      const char* pModeLum   = param["mode_lum"];
      modeLuminosite = ValideListe(pModeLum, LUM_NORMAL, LUM_ECO, LUM_FORT, modeLuminosite, "mode de luminosite");
      
      const char* pModeLog   = param["mode_log"];
      modeLog = ValideListe(pModeLog, LOG_NORMAL, LOG_VERBOSE, LOG_DEBUG, modeLog, "mode de log");
      
      const char* pmodeFonc = param["mode_fonc"];
      modeFonc = ValideListe(pmodeFonc, MODE_NORMAL, MODE_VEILLE, MODE_AUTONOME, modeFonc, "mode de fonctionnement");

      const char* pRessenti = param["ressenti"];
      device.ressenti = ValideListe(pRessenti, RESSENTI_BIEN, RESSENTI_NORMAL, RESSENTI_PASBIEN, device.ressenti, "ressenti");  }
    return err;
  }  
//-----------------------------------------------------------------------------------------------------------------------------  
  void SendMesureWeb(){      
    theme = "page web";
    if( !server.hasArg("par") || server.arg("par") == NULL ) {
      server.send(400, "text/plain", "400: Invalid Request");
      Log(2, "retour page web errone", "");
      return; }
                        // récupération des parametres
    String parametres = server.arg("par");
                        // envoi des valeurs instantanées à la page web
    String valeurInst = "{\"PM25\" : " + String(pm[M_PM25]) + ", \"PM10\" : " + String(pm[M_PM10]) + "}";
    //Log(4, "json mesure: " + valeurInst, "");
    server.send(200, "application/json", valeurInst);
    Log(4, "parametres : " + parametres, "");
    Log(3, "envoi mesures", valeurInst);
    boolean erreur = ActualisationParametres(parametres);  
                        // envoi des paramètres au serveur
    if (!autonom & modeFonc != MODE_VEILLE) {
      String url = SERVEUR_AI4GOOD_PARAM;
      String JSONparam = GenereJSONparam();
      if (JSONparam == "") Log(2, "JSON parametres non genere", "");
      else {
        String retour = EnvoiJSON(url, JSONparam);
        if (retour == "") Log(2, "retour envoi JSON parametres vide", "");  }  }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  String dateToString(DTime date) { return (String)date.year + "-" + (String)date.month + "-" + (String)date.day + "T" + (String)date.hour + ":" + (String)date.minute + ":" + (String)date.second + "+01:00"; }// sans les millisecondes
//-----------------------------------------------------------------------------------------------------------------------------
  DTime calculDate(unsigned long dateI) { DTime date((dateI - milliRef) / 1000 + dateRef.timestamp); return date; }
//-----------------------------------------------------------------------------------------------------------------------------
  unsigned long calculDateInt(DTime date) { return (date.timestamp - dateRef.timestamp) * 1000; }
  //-----------------------------------------------------------------------------------------------------------------------------
DTime stringToDate(String chaine) {
    DTime date;
    if (chaine.length() > 15) {
      date.setDate(chaine.substring(0, 4).toInt(), chaine.substring(5, 7).toInt(), chaine.substring(8, 10).toInt());
      date.setTime(chaine.substring(11, 13).toInt(), chaine.substring(14, 16).toInt(), chaine.substring(17, 19).toInt());
      Serial.println(chaine.substring(0, 4) + chaine.substring(5, 7) + chaine.substring(8, 10));
      Serial.println(chaine.substring(0, 4).toInt()); 
      Serial.println(date.year);
    }
    else Log(2, "date serveur non exploitable (taille < 15) : ", String(chaine.length()));
    return date;
  }
#endif
