#ifdef RESEAUWIFI
//-----------------------------------------------------------------------------------------------------------------------------
  void DemarreWiFi(boolean memIdentifiant, WiFiManager *wifiManager) {
    if (!memIdentifiant) wifiManager->resetSettings();             // garder en mémoire ou non les anciens identifiants
    wifiManager->autoConnect(AUTO_CONNECT);                        // connexion automatique au réseau précédent si MEMIDENTIFIANT = 1
    boolean debugWiFi = (modeLog == String(LOG_DEBUG));
    wifiManager->setDebugOutput(debugWiFi);                        // à revoir, marche pas
    Log(3, "Connected to : " + WiFi.SSID(), WiFi.psk());
    Log(0, "IP address : ", WiFi.localIP().toString());
  }
//-----------------------------------------------------------------------------------------------------------------------------
  String GenereJSONparam() { 
    const size_t capacity = JSON_OBJECT_SIZE(10);
    DynamicJsonDocument param(capacity);
    String JSONmessage = "";
    param["device"]               = DEVICE_NAME;
    param["feeling"]              = ressenti;
    param["mode_fonctionnement"]  = modeFonc;
    param["mode_log"]             = modeLog;
    //param["mode_lum"]             = modeLuminosite;
    param["niv_batt"]             = String(niveauBatterie);
    serializeJson(param, JSONmessage);
    return JSONmessage;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  String GenereJSONdata() { 
    const size_t capacity = JSON_OBJECT_SIZE(13);
    DynamicJsonDocument data(capacity);
    String JSONmessage = "";
    data["device"]            = DEVICE_NAME;
    data["pm25"]              = String(mes[M_PM25].valeur, 2);
    data["pm10"]              = String(mes[M_PM10].valeur, 2);
    data["pm25_filtree"]      = String(mes[M_PM25].valeurFiltree, 2);
    data["pm10_filtree"]      = String(mes[M_PM10].valeurFiltree, 2);
    data["date_mesure"]       = CalculDate(mes[M_PM25].date);
    data["ecart_type_pm25"]   = String(mes[M_PM25].ecartType, 2);
    data["ecart_type_pm10"]   = String(mes[M_PM10].ecartType, 2);
    data["taux_erreur_pm25"]  = String(mes[M_PM25].tauxErreur, 2);
    data["taux_erreur_pm10"]  = String(mes[M_PM10].tauxErreur, 2);
    data["feeling"]           = ressenti;
#if GPS
    data["latitude"]          = latitude;
    data["longitude"]         = longitude;
#else
    data["latitude"]          = "";
    data["longitude"]         = "";
#endif
    serializeJson(data, JSONmessage);
    return JSONmessage;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void MajToken() { 
    if (!autonom) {
      int httpCode = 0;
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
        tokenExpire   = StringToDate(expDate);
        Log(4, "token (en sec) : " + String(tokenExpire/10) + tokenValeur, "");    
                    // calcul de date
        unsigned long dateLogin = millis()/100;
        const char* dateServeur = login["date"];
        if (StringToDate(dateServeur) > 0) {
          dateRef.dateExt = StringToDate(dateServeur);
          dateRef.dateInt = dateLogin;
          dateRef.decalage = dateRef.dateExt - dateRef.dateInt;  }  }
      delay(10000); }
  }
//-----------------------------------------------------------------------------------------------------------------------------
  String EnvoiJSON(String url, String JSONmes) { 
    String retour = "";
    int httpCode = 0;
    if ((WiFi.status() == WL_CONNECTED) & !autonom) {                                     
      Log(4, "délai token restant en sec : " + String((tokenExpire - millis()/100 - dateRef.decalage)/10), "");
      if (tokenExpire - millis()/100 - dateRef.decalage < 10*60*10) MajToken();         // marge de 10 minutes avant expiration
      HTTPClient http;
      http.begin(url, SERVEUR_AI4GOOD_FINGER);
      http.addHeader("Authorization", tokenValeur);
      http.addHeader("Content-Type", "application/json");             
      httpCode = http.POST(JSONmes);                       
      retour  = http.getString();                                
      http.end(); 
      if (httpCode != 200 & httpCode != 201) {
        if (httpCode == 401 | httpCode == 403) {
          tokenExpire = 0;
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
  void AjouteMesure(String JSONmesure) {                                  // Stockage des donnnées non envoyées
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
  void EnvoiWifiData() { 
    String url = SERVEUR_AI4GOOD_DATA;
    String retourData = "";
    theme = "wifi    ";
    String JSONdata = GenereJSONdata();
    if (autonom) AjouteMesure(JSONdata);
    else if (JSONdata == "") {
      Log(2, "taille JSON superieure a la taille maxi", "");
      StripAffiche("mesure non envoyee");   }
    else {
      retourData = EnvoiJSON(url, JSONdata);
      if (retourData == "") { 
        StripAffiche("mesure non envoyee");
        Log(2, "retour serveur data vide ", "");
        AjouteMesure(JSONdata);   }   }
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
      HTTPClient http;
      http.begin(SERVEUR_AI4GOOD_VAR);
      http.addHeader("Content-Type", "application/json");                 
      int httpCode = http.GET();
      String parametres = http.getString();                                  
      http.end();        
      Log(4, "ajustement des variables  : " + parametres, "");
      if ((parametres == "")|((httpCode != 200)&(httpCode != 201))) Log(2, "lecture des variables indisponible", ""); 
      else boolean erreur = ActualisationVariables(parametres);   }
    else Log(4, "pas de recuperation wifi de varibles", "");
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
        if (pTempsCycle != TEMPS_CYCLE) {
          Log(0,"nouveau temps de cycle en ms : ", String(pTempsCycle));
          TEMPS_CYCLE = pTempsCycle;  }  }
      else Log(4,"temps de cycle incorrect : ", "");
    
      int pMesureLED = param0["mesure_led"];
      if ((pMesureLED > -1) & (pMesureLED < NB_MES)) {
        if (pMesureLED != mesureLED) {
          Log(0,"nouvelle mesure a afficher sur la LED : ", String(pMesureLED));
          mesureLED = pMesureLED;  }  }
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
      ressenti = ValideListe(pRessenti, RESSENTI_BIEN, RESSENTI_NORMAL, RESSENTI_PASBIEN, ressenti, "ressenti");  }
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
#endif
