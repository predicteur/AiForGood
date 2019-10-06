/* 
  Option de compilation et téléversement :
      ESP     -> Type de carte : Node MCU 1.0(ESP-12E Module) / Options : 80 MHz, Flash, 4M (3M SPIFFS), v2 Lower Memory, Disabled, None, Only Sketch / programmateur : Arduino as ISP / Outils : ESP8266 Sketch Data Upload (envoi des fichiers Data)
      MKR1200 -> Type de carte : ARDUINO MKR FOX 1200 / programmateur : ATMEL-ICE

  Paramètres à vérifier avant compilation :
  
    const char *DEVICE_NAME = "sensorxxx";  
    #define SDS               true       (si SDS présent)
    #define GPS               true       (si GPS présent)
    #define LED_PIN           D8         (si on est sur un sac)
    String MODE_LOG_DEFAUT  = LOG_NORMAL   (si on est sur un fonctionnement de test)
    int     M_LED_DEFAUT  = M_PM25;       // choix de la mesure à afficher
    #define LED_PIN           D1          // affichage de l'état des mesures pour ESP (D8 pour les sacs, D1 sur ESP de test Philippe)
    #define RXPIN             14          // laison série capteur (GPIO14 = D5)
    #define TXPIN             12          // laison série capteur (GPIO12 = D6)
    #define RXGPS             2           // laison série GPS     (GPIO2  = D4)

  Téléchargement Data (SPIFFS) à faire avant utilisation
  
*/
//---------------------------------------------include  --------------------------------------------------------------------------------
#include "parametre.h"                                            // Déclaration de constantes, librairies et de structures de données
//---------------------------------------------variables--------------------------------------------------------------------------------
  // variables générales
  boolean autonom           = false;
  String  theme             = "Global  ";                         // précision utilisée pour les logs
  boolean etatWifiConnecte  = true;                               // etat du wifi
  boolean etatMesureSature  = false;                              // etat de saturation du stockage des mesures
  String  modeFonc          = MODE_NORMAL;                        // voir valeur initiale dans parametre.h
  String  modeLuminosite    = LUM_NORMAL;                         // voir valeur initiale dans parametre.h
  String  modeLog           = LOG_DEFAUT;                         // voir valeur initiale dans parametre.h
  String  resetWiFi         = RESET_AUCUN;                        // voir valeur initiale dans parametre.h
  int     tempsCycle        = TEMPS_CYCLE;                        // voir valeur initiale dans parametre.h
  boolean memIdentifiant    = MEM_IDENTIFIANT;                    // voir valeur initiale dans parametre.h
  int totalMesureNonReprise = 0;                                  // compteur de mesure dans SPIFFS
  int     niveauBatterie    = 100;                                // 
  boolean niveauBatterieBas = false;                              // 

  // variables affichage LED
  int     mesureLED         = M_LED_DEFAUT;                       // voir valeur initiale dans parametre.h
  int     niveauAffichage   = NIVEAU_MOYEN;                       // voir les niveaux dans parametre.h
  
  // variables périodicité
  float   temps_ref_mesure  = 0;                                  // compteur de temps intermédiaire de 0 à tempsCycle/NB_MESURE pour les mesures
  float   temps_ref_parametre = 0;                                // compteur de temps intermédiaire de 0 à tempsCycle/NB_MESURE pour les paramètres
  unsigned long timerVeille = 0;                                  // compteur de temps de clignotement en veille
  int     nbMesureGroupe    = 0;                                  // compteur de mesures pour envoi groupé Sigfox de 0 à TAILLE_ECH
  int     nbMesureElem      = 0;                                  // compteur de mesures élémentaires dans le temps de cycle pour calcul du niveau de qualité
  DateRef dateRef           = {0, 0, 0};
  // variables mesures et capteurs
  //struct WorkingStateResult etatSDS;                                     // état de fonctionnement du capteur SDS
  Mesure  mes[NB_MES];                                            // tableau des informations liées à une mesure
  double  pm [NB_MES];                                            // mesure de PM2.5 et PM10
  String  ressenti          = "normal";                           // 3 états : "bien", "normal", "pasbien"
#if GPS
  String  longitude         = "";
  String  latitude          = "";
#endif
#ifdef RESEAUWIFI
  File    ficMes;                                                 // fichier de stockage temporaire des mesures non envoyées
  String        tokenValeur;
  boolean       tokenPresence = false;
  unsigned long tokenExpire = 0;
#endif
#ifdef COMPRESSION                                                // variables series de mesures
  Mesure   mesEnvoi[NB_MES][TAILLE_ECH];                          // tableau des mesures à envoyer
  CoefReg  coef     = {0.0, 0.0, 0.0};                            // paramètre de chaque régression unitaire
  CoefComp coefc    = {0.0, 0.0, 0.0, {}, {}, 0.0};               // coefc : paramètres issue de la compression ou de l'optimisation
  CoefComp coefp    = {0.0, 0.0, 0.0, {}, {}, 0.0};               // coefp : paramètres issus du decodage
  CoefCode coefi    = {0, 0, 0.0, {}, {}, 0.0};                   // paramètre après codage
  float y0init[TAILLE_ECH], y0n[TAILLE_ECH], y0fon[TAILLE_ECH];   // y0init : mesures à compresser, Y0n : mesures normalisées (entre 0 et 1), Y0fon : mesures issues de la compressipon
#endif
#ifdef BOARDSIGFOX
  SigfoxMessage payload;                                          // message envoyé par sigfox (12 octets découpés en trois variables de 4 octets)
  int lastMessageStatus;                                          // message de retour d'un envoi 
#endif
#if Z14A
  byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};  // donnees pour le Z14A
  char reponse[9];
#endif
//--------------------------------------------- initialisation des objets --------------------------------------------------------------------------------
  Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);      // afficheur LED associé au capteur
#ifdef RESEAUWIFI
  ADC_MODE(ADC_VCC);                                                      // autorise la lecture de tension (non compatible avec l'utilisation de A0 qui doit rester en l'air) 
  ESP8266WebServer server(80);                                            // ecoute sur le port 80
  WiFiManager wifiManager;                                                // WiFi option non fixe
#endif
#if SDS
 #ifdef BOARDSIGFOX
  SdsDustSensor sds(Serial1);                                             // capteur utilisé avec liaison série intégrée (à revoir sur ESP)
 #else
  SdsDustSensor sds(RXPIN, TXPIN);                                        // capteur utilisé avec liaison SPI
 #endif
#endif
#if GPS
  TinyGPSPlus gps;
  SoftwareSerial GpsSerial(RXGPS, SW_SERIAL_UNUSED_PIN);                  // (RX, TX)
#endif
#if NEXTPM
  PMS pms(Serial1);
#endif
#if CCS811
  Adafruit_CCS811 ccs;
#endif
/*
 ****************************************************************************************************************
             Setup 
 ****************************************************************************************************************
*/
  void setup() {
  
//--------------------------------------------- initialisation controleur et affichage --------------------------------------------------------------------------------                                                                                 
    theme = "setup   ";
    Serial.begin(115200);                                       
    delay(10); Serial.println('\n');
                                                                // initialisation données et affichage
    CreerMesure();                                              // Création pm10 et pm2_5
    strip.begin();                                              // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();                                               // Turn OFF all pixels
    StripAffiche("demarrage");
//--------------------------------------------- initialisation capteurs et équipements --------------------------------------------------------------------------------                                                                                 
    theme = "capteur ";
#if SDS                                                         // initialisation capteur PM
    sds.begin();
    Log(3, sds.queryFirmwareVersion().toString(), "");          // prints firmware version
    Log(3, sds.setActiveReportingMode().toString(), "");        // ensures sensor is in 'active' reporting mode
    Log(3, sds.setContinuousWorkingPeriod().toString(), "");    // ensures sensor has continuous working period - default but not recommended
    //WorkingStateResult etatSDS = sds.wakeup();                // initialisation de etat SDS
#endif
#if CCS811                                                      // initialisation eCO2/COV     -- à revoir 
    if(!ccs.begin()){
      Serial.println("Failed to start sensor! Please check your wiring.");
      while(1);}
    while(!ccs.available());                                    //calibrate temperature sensor
    float temp = ccs.calculateTemperature();
    ccs.setTempOffset(temp - 25.0);
    Serial.println("calibration effectuée");
#endif
#if GPS                                                         // initialisation GPS  
    GpsSerial.begin(9600);
#endif
#if NEXTPM                                                      // initialisation NextPM  
    Serial1.begin(9600);                                        // UART hardware
#endif
#if Z14A                                                        // initialisation Z14A  
    Serial1.begin(9600);                                        // UART hardware
#endif
//--------------------------------------------- initialisation communication --------------------------------------------------------------------------------                                                                                 
#ifdef BOARDSIGFOX                                              // initialisation Sigfox
    theme = "SIGFOX  ";
    if (!SigFox.begin()) reboot();
    SigFox.end();
    if (oneshot) SigFox.debug();
#endif
#ifdef RESEAUWIFI
    theme = "ESP     ";
    if (!SPIFFS.begin()) Log(2, "SPIFFS Mount failed", "");     // initialisation serveur de fichier
    else Log(3, "SPIFFS Mount succesfull", "");
 
    /*// vidage du fichier des mesures en attente d'envoi (doit être en commentaires)
    ficMes = SPIFFS.open(FIC_BUF, "w");                           // pour vider le fichier des mesures en attente
    ficMes.println("vide");                                       // pour vider le fichier des mesures en attente
    ficMes.close();                                               // pour vider le fichier des mesures en attente
    */
                                                                  // initialisation Serveur WiFi
    theme = "wifi    ";
    WiFi.mode(WIFI_AP_STA);                                       // mode mixte server et client
    DemarreWiFi(memIdentifiant, &wifiManager);
                                                                  // initialisation token et date
    MajToken();
                                                                  // initialisation serveur web
    server.on("/mesures.json", HTTP_GET, SendMesureWeb);
    server.serveStatic("/", SPIFFS, "/index.html");
    server.serveStatic("/index.html", SPIFFS, "/index.html");
    server.serveStatic("/StyleIndex.css", SPIFFS, "/StyleIndex.css");
    server.begin();
#endif    
//--------------------------------------------- initialisation démarrage --------------------------------------------------------------------------------                                                                                 
    StripAffiche("controleur demarre");
    Log(0, "controleur demarre en mode : ", modeLog);
#ifdef RESEAUWIFI
    RepriseEnvoiWifiData();                                        // renvoi des mesures stockées dans SPIFFS
#endif
  }
/*
 ****************************************************************************************************************
             Boucle 
 ****************************************************************************************************************
*/
  void loop() {    
//--------------------------------------------- mise à jour des variables serveur et paramètres page web ----------------------------------------------------------                                                                               
#ifdef RESEAUWIFI
    theme = "wifi    ";
    if ( (millis() - temps_ref_parametre) >= float(tempsCycle)/float(NB_MESURE) ) {     // mise à jour des variables(traitement des données du serveur)
      temps_ref_parametre = millis();
      if (!autonom & modeFonc != MODE_VEILLE) AjustementVariables();  }
    WiFiClient client;                                                                  // mise à jour des paramètres(traitement des requetes des pages web)
    server.handleClient();
//--------------------------------------------- gestion du reset wifi ----------------------------------------------------------------------------------------------      
    if (resetWiFi != RESET_AUCUN) {
      boolean identifiant = (resetWiFi == RESET_AUTO);
      StripAffiche("demarrage");
      DemarreWiFi(identifiant, &wifiManager);
      resetWiFi = RESET_AUCUN;  
      StripAffiche("controleur demarre");
      Log(0, "controleur redemarre en mode : ", modeLog);  
      RepriseEnvoiWifiData(); }                                       // renvoi des mesures stockées dans SPIFFS
#endif    
//--------------------------------------------- gestion des modes de fonctionnement --------------------------------------------------------------------------------                                                                               
    theme = "global  ";
    autonom = (modeFonc == MODE_AUTONOME);
    if (modeFonc == MODE_VEILLE) StripAffiche("mode veille");
    else if (etatMesureSature) {
      StripAffiche("mesures saturees"); 
      if (!autonom) RepriseEnvoiWifiData();  }                                      // renvoi des mesures stockées dans SPIFFS
#if SDS
      /*WorkingStateResult etatSDS = sds.sleep();
      if (etatSDS.isWorking()) {
        Log(1, "Probleme de mise en sommeil SDS", "");
      }*/
#endif
    else {
#if SDS
      /*WorkingStateResult etatSDS = sds.wakeup();
      if (!etatSDS.isWorking()) {
        Log(1, "Probleme de reveil SDS", "");
      }*/
#endif
      if      (modeLuminosite == LUM_ECO)   niveauAffichage = NIVEAU_FAIBLE;
      else if (modeLuminosite == LUM_FORT)  niveauAffichage = NIVEAU_FORT;        
      else                                  niveauAffichage = NIVEAU_MOYEN;        
//--------------------------------------------- mise à jour des coordonnées GPS -------------------------------------------------------------------------------------------                                                                               
#if GPS
      theme = "GPS     ";
      while (GpsSerial.available() > 0) gps.encode(GpsSerial.read());
      if (gps.location.isUpdated()) {
        longitude = String(gps.location.lng(), 7);
        latitude  = String(gps.location.lat(), 7);
        Log(4, "Cooordonnées GPS : " + longitude + " " + latitude,"");  }
#endif
//--------------------------------------------- boucle de réalisation des mesures intermédiaires --------------------------------------------------------------------------------                                                                                     
      if ( (millis() - temps_ref_mesure) >= float(TEMPS_CYCLE)/float(NB_MESURE) ) {
          theme = "mesure  ";
          temps_ref_mesure = millis();
          StripAffiche("debut mesure");
          LireCapteur();                                      // lecture des données dans pm[]
          CalculMesure();                                     // ajout des valeurs instantanées dans mes.xxx
          StripAffiche("fin mesure");
          nbMesureElem ++;  }
//--------------------------------------------- boucle principale de mesure --------------------------------------------------------------------------------                                                                               
      if ( nbMesureElem >= NB_MESURE ){
          theme = "mesure  ";
          nbMesureElem = 0;
#ifdef RESEAUWIFI
          if ((totalMesureNonReprise > 0) & (!autonom)) RepriseEnvoiWifiData();    // essai de renvoi des mesures à renvoyer stockées dans SPIFFS
#endif
          GenereMesure();                                     // calcul de la mesure moyenne dans mes.xxx
          if (MesureOk()) {
              PrMesure(3);                                     
#ifdef RESEAUWIFI
              EnvoiWifiData(); }                               // envoi sur le serveur (et stockage fichier si KO)
#else
              GroupeMesure();  }                               // ajout de la mesure au groupe de mesure à envoyer par Sigfox   
#endif
          else {
            StripAffiche("mesure a afficher incorrecte");
            delay(2000); }
          niveauBatterie = MesureBatterie();                  //  à mettre en place (mesure)
#ifdef COMPRESSION
          nbMesureGroupe ++;
#endif
          UpdateLed( mes[mesureLED].valeur );                 // affichge du niveau sur les LED
          InitMesureRessenti(); }                             // dépend de la gestion du ressenti (maintenu ou non maintenu) -> à clarifier
//--------------------------------------------- boucle d'envoi des données groupées --------------------------------------------------------------------------------                                                                               
#ifdef COMPRESSION
      if ( nbMesureGroupe >= TAILLE_ECH ){
          nbMesureGroupe = 0;      
          GenereGroupe();                                      // préparation des données à compresser avant envoi
          PrSerie(y0init, TAILLE_ECH, "y0init");
          compress();                                          // compression des données
          PrSerie(y0fon, TAILLE_ECH, "y0fon apres compress");
          EnvoiSigfox(); }                                  // envoi sigfox des données compressées
#endif
    }
  }
