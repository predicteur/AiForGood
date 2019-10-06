//-----------------------------------------------------------------------------------------------------------------------------
  void Log(int type, String objet, String parametre) {
    String url = SERVEUR_AI4GOOD_LOG;
    String typeS = "infos---";
    String JSONlog = "";
    String JSONretour = "";
    if      (type == 1) typeS = "warning-";
    else if (type == 2) typeS = "error---";
    else if (type == 3) typeS = "detail--";
    else if (type == 4) typeS = "debug---";
    if ((modeLog == "normal" & type < 3) | (modeLog == "verbose" & type < 4) | (modeLog == "debug" & type <5)) {
      Serial.println(theme + " : " + typeS + " " + objet + " -- " + parametre);  }
#ifdef RESEAUWIFI
    if ((type < 3) & (tokenExpire > 0) & !autonom) {                                  // envoi des logs
      const size_t capacity = JSON_OBJECT_SIZE(12);
      DynamicJsonDocument rootLog(capacity);
      rootLog["equipement"]        = DEVICE_NAME;
      rootLog["date"]              = CalculDate(millis()/100);
      rootLog["type"]              = typeS;
      rootLog["theme"]             = theme;
      rootLog["objet"]             = objet;
      rootLog["parametre"]         = parametre;
      serializeJson(rootLog, JSONlog);
      if (JSONlog == "") Log(4, "JSON log non genere", "");
      else {
        JSONretour = EnvoiJSON(url, JSONlog);
        if ((JSONretour == "") & (objet != "retour serveur log vide")) Log(2, "retour serveur log vide", "");  }  }
#endif
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void UpdateLed(float mesValeurLED) { 
    if      (mesValeurLED < SEUIL_BON_PM)   StripAffiche("correct");
    else if (mesValeurLED < SEUIL_MOYEN_PM) StripAffiche("moyen");
    else                                    StripAffiche("mauvais");
  }
//-----------------------------------------------------------------------------------------------------------------------------
  String  CalculDate(unsigned long dateInt) {
    String chaine = DateToString(dateInt + dateRef.decalage);
    if (dateRef.dateExt == 0) chaine = " - "; 
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
    if (jour <= 31) mois = "08";  
    else if (jour <= 61) {                                        //à compléter pour les autres mois
      mois = "09";
      jour -= 31;  }
    String chaine = "2019-" + mois + "-" + (String)jour + "T" + (String)heure + ":" + (String)minute + ":" + (String)seconde + "." + (String)milli + "Z";
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
      if ((annee < 2019) || (annee > 2020) || (mois > 12) || (jour > 31) || (heure > 23) || (minute > 59) || (seconde > 59) || (milli > 9)) {
        Log(2, "date serveur non correcte ", "");  }
      else {
        if (mois == 9)  { date += 31*24*60*60*10;}
        if (mois == 10) { date += 61*24*60*60*10;}
        if (mois == 11) { date += 92*24*60*60*10;}                      //à compléter pour les autres mois
        date += jour*24*60*60*10;
        date += heure*60*60*10;
        date += minute*60*10;
        date += seconde*10;
        date += milli;  }   } 
    else Log(2, "date serveur non exploitable (taille < 15) : ", String(chaine.length()));
    return date;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  int MesureBatterie(){
    int niveau = 100;
                                                                // à construire
    niveauBatterieBas = false;    
                                                                // à construire
    return niveau;
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void StripAffiche(String modeStrip){
    int niveau = niveauAffichage; 
    if (modeFonc != MODE_VEILLE & !etatMesureSature) Log(4, modeStrip, "");
    if (niveauBatterieBas) { 
        niveau = min(niveauAffichage, NIVEAU_FAIBLE);      
        strip.setBrightness(LUMINOSITE_FAIBLE * niveau / 100); }
    if      (modeStrip == "demarrage") {     
        strip.setBrightness(LUMINOSITE_FAIBLE * niveau / 100);
        strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]); }
    else if (modeStrip == "controleur démarre") {       
        strip.setPixelColor(0, VERT[0], VERT[1], VERT[2]);
        Log(0, modeStrip, ""); }
    else if (modeStrip == "mesures saturees") {       
        strip.setPixelColor(0, ROUGE[0], ROUGE[1], ROUGE[2]);
        if      ((millis() - timerVeille) > 6000) timerVeille = millis();                                      // clignotement  
        else if ((millis() - timerVeille) > 4000) strip.setBrightness(LUMINOSITE_FAIBLE * NIVEAU_FAIBLE / 100);
        else strip.setBrightness(NIVEAU_ETEINT); }
    else if (modeStrip == "mode veille") {       
        strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]);
        if      ((millis() - timerVeille) > 6000) timerVeille = millis();                                      // clignotement  
        else if ((millis() - timerVeille) > 4000) strip.setBrightness(LUMINOSITE_FAIBLE * NIVEAU_FAIBLE / 100);
        else strip.setBrightness(NIVEAU_ETEINT);  }
    else if (modeStrip == "mesure a afficher incorrecte") {
        strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]);
        Log(0, modeStrip, ""); }
    else if (modeStrip == "mesure stockee")             strip.setPixelColor(0, BLEU[0], BLEU[1], BLEU[2]);
    //else if (modeStrip == "mesure capteur sds erronee") strip.setPixelColor(0, VIOLET[0], VIOLET[1], VIOLET[2]);
    else if (modeStrip == "correct")                    strip.setPixelColor(0, VERT[0], VERT[1], VERT[2]);
    else if (modeStrip == "moyen")                      strip.setPixelColor(0, ORANGE[0], ORANGE[1], ORANGE[2]);
    else if (modeStrip == "mauvais")                    strip.setPixelColor(0, ROUGE[0], ROUGE[1], ROUGE[2]);
    else if (modeStrip == "debut mesure")               strip.setBrightness(LUMINOSITE_FORTE * niveau / 100);
    else if (modeStrip == "fin mesure")                 strip.setBrightness(LUMINOSITE_FAIBLE * niveau / 100);
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
    Log(level, "   Valeur(inst+filt) : " + String(mes[mesureLED].valeur) + "  " + String(mes[mesureLED].valeurFiltree) + " Ecart-type : " + String(mes[mesureLED].ecartType),"");
    Log(level, "   Ressenti          : " + ressenti + " Date : " + CalculDate(mes[mesureLED].date), "");
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void LireCapteur(){
    theme = "capteur ";
#if SDS
    PmResult pm_val = sds.queryPm();
    if (pm_val.isOk()) {
      //UpdateLed();                                    // remet la bonne couleur si une mesure mauvaise a eu lieu avant
      pm[M_PM25] = pm_val.pm25;
      pm[M_PM10] = pm_val.pm10; }
    else {
      Log(2, "mesure capteur sds erronee ", "");
      StripAffiche("mesure capteur sds erronee");  }
#else
 #ifdef BOARDSIGFOX
    SigFox.begin(); delay(100);      
    pm[M_PM25] = SigFox.internalTemperature();
    pm[M_PM10] = SigFox.internalTemperature();
    SigFox.end();
 #else
    pm[M_PM25] = float(ESP.getVcc())/10.0;
    pm[M_PM10] = 100.0 * cos(3.14159 * millis());
    delay(1000);
 #endif
#endif
    Log(3, "mesure capteur : " + String(pm[mesureLED]), "");
  }
