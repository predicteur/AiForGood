
#ifndef PARAMETRE
  #define PARAMETRE
/*
 ****************************************************************************************************************
             Configuration 
 ****************************************************************************************************************
*/
//--------------------------------------------- configuration des plateformes --------------------------------
  #ifdef SIGFOX_SPI                       //test du board sigfox
    #define BOARDSIGFOX
    #define COMPRESSION
  #else 
    #define RESEAUWIFI
  #endif
  
//--------------------------------------------- configuration physique ------------------------------------
    #define VERSION_LOGICIEL  "V2.1"          // 
    #define VERSION_MATERIEL  "V1"            // 
    #define CAPTEUR           "SDS"           // 
    #define TYPE_DEVICE       "mobile"        // 
    #define LED_COUNT         1               // nombre de LED dans le ruban
    #define GPS               false           // GPS      - fonctionnement sur RX/TX
    #define SDS               false           // PM       - fonctionnement sur RX/TX
    #define Z14A              false           // CO2      - fonctionnement sur RX/TX
    #define CCS811            false           // eCO2/COV - fonctionnement sur SDA/SCL
    #define NEXTPM            false           // PM       - fonctionnement sur RX/TX
  #ifdef BOARDSIGFOX
    #define LED_PIN           8               // affichage de l'état des mesures pour Sigfox
  #else
    #define LED_PIN           D1              // affichage de l'état des mesures pour ESP (15 ou D8 pour les sacs, D1 sur ESP de test Philippe)
    #define RXPIN             14              // laison série capteur (GPIO14 = D5)
    #define TXPIN             12              // laison série capteur (GPIO12 = D6)
  #endif
  #if GPS
    #define RXGPS             2               // laison série GPS     (GPIO2  = D4)
  #endif

//--------------------------------------------- configuration des librairies -------------------------------- 
  #ifdef BOARDSIGFOX
    #include <SigFox.h>
    #include <ArduinoLowPower.h>
    #include <Compressor.h>
  #else
    #include <ESP8266WiFi.h>
    #include <WiFiClientSecure.h>
    #include <ESP8266WiFiMulti.h>
    #include <WiFiManager.h>
    #include <ESP8266HTTPClient.h>
    #include <FS.h>
    #include <DTime.h>
  #endif
  #if GPS
    #include <TinyGPS++.h>     
    #include <SoftwareSerial.h>
  #endif
  #if SDS
    #include <SdsDustSensor.h>
  #endif
  #if CCS811
    #include "Adafruit_CCS811.h"              // librairie pour capteur CJMCU 811 : eCO2 / TCOV
  #endif
  #if NEXTPM
    #include <PMS.h>                          // librairie PMS pour NextPM
  #endif
    #include <ArduinoJson.h>
    #include <Adafruit_NeoPixel.h>    
/*
 ****************************************************************************************************************
             Paramètres qui impactent le code
 ****************************************************************************************************************
*/
//--------------------------------------------- parametres génériques ------------------------------------------
    #define   RESET_AUCUN       "aucun"       // mode de reset wifi
    #define   RESET_AUTO        "autoconnect" // mode de reset wifi
    #define   RESET_MANU        "manuconnect" // mode de reset wifi
    #define   LUM_ECO           "economie"    // mode d'affichage
    #define   LUM_NORMAL        "normal"      // mode d'affichage
    #define   LUM_FORT          "fort"        // mode d'affichage
    #define   MODE_VEILLE       "veille"      // mode de fonctionnement
    #define   MODE_NORMAL       "normal"      // mode de fonctionnement
    #define   MODE_AUTONOME     "autonome"    // mode de fonctionnement
    #define   LOG_NORMAL        "normal"      // mode de log
    #define   LOG_VERBOSE       "verbose"     // mode de log
    #define   LOG_DEBUG         "debug"       // mode de log
    #define   RESSENTI_BIEN     "bien"        // valeur de ressenti
    #define   RESSENTI_NORMAL   "normal"      // valeur de ressenti
    #define   RESSENTI_PASBIEN  "pasbien"     // valeur de ressenti

//--------------------------------------------- parametres structure de données --------------------------------
    #define   NB_MES    2                     // nombre de mesures à effectuer (ici 2 :PM25 et PM10)
    #define   M_PM25    0                     // indice de la mesure PM25 dans le tableau des mesures
    #define   T_PM25    "PM25"                // type de la mesure PM25 dans le tableau des mesures
    #define   M_PM10    1                     // indice de la mesure PM10 dans le tableau des mesures
    #define   T_PM10    "PM10"                // type de la mesure PM10 dans le tableau des mesures
    #define   FIC_BUF   "/buffer.txt"         // fichier de stockage des mesures non encore envoyées     
    
//--------------------------------------------- parametres Sigfox --------------------------------
  #ifdef BOARDSIGFOX
    const boolean test = false;               // test uniquement des capteurs sans utilisation de Sigfox
    const boolean oneshot = true;             // affichage des infos d'envoi sigfox
  #endif

//--------------------------------------------- parametres affichage LED modifiables VAR --------------------------------
    int     ROUGE[3]          = {250, 0, 0};
    int     ORANGE[3]         = {200, 100, 0};
    int     BLEU[3]           = {0, 0, 250};
    int     VIOLET[3]         = {250, 0, 200};
    int     VERT[3]           = {0, 250, 0};
    int     LUMINOSITE_FORTE  = 250;          // maxi 255
    int     LUMINOSITE_FAIBLE = 50;
    int     NIVEAU_FORT       = 100;          // niveau variable de 0 à 100
    int     NIVEAU_MOYEN      = 25;           // niveau variable de 0 à 100 (utilisé en m
    int     NIVEAU_FAIBLE     = 10;           // niveau utilisé pour le niveau bas de la batterie et en mode économie)
    int     NIVEAU_ETEINT     = 0;            // niveau variable de 0 à 100 (utilisé en mode veille)
    
//--------------------------------------------- paramètres de l'envoi --------------------------------
  #ifdef BOARDSIGFOX
    const int   TAILLE_MSG  = 32;             // nombre de bit des messages composant le "payload" (3 messages pour un payload Sigfox de 12 octets)
    const int   TAILLE_PAY  = 96;             // nombre de bit du "payload" (Sigfox 12 octets)
  #endif
/*
 ****************************************************************************************************************
             Paramètres modifiables sans avoir à modifier le code
 ****************************************************************************************************************
*/
//--------------------------------------------- configuration logicielle -----------------------------
    int         MAX_STOCKAGE    = 300;        // nombre mxi de mesures stockées dans l'ESP
    boolean     MEM_IDENTIFIANT = true;       // false : pas de mémorisation des accès WiFi, true : mémorisation  
    String      LOG_DEFAUT      = LOG_DEBUG;  // "normal" : infos(0), warning(1), erreur(2), "verbose" : detail(3), "debug" : debug(4)
    const char *DEVICE_NAME     = "sensor4";  // nom du device a documenter

//--------------------------------------------- configuration mesures
    int         TEMPS_CYCLE   = 20000;        // Temps de cycle : période d envoi des mesures au serveur en millisecondes
    #define     COEF_FILTRAGE   0.5           // filtrage AR simple : xf(t) = coef * xf(t-1) + (1-coef) * x(t)
    #define     NB_MESURE       5             // nombre de mesure élémentaires dans le temps de cycle pour calcul du niveau de qualité
    #define     VALEUR_MIN_PM   0.0           // limite mini autorisee pour les PM
    #define     VALEUR_MAX_PM   1000.0        // limite maxi autorisee pour les PM
    #define     SEUIL_BON_PM    10.0          // seuil affichage LED pour les PM
    #define     SEUIL_MOYEN_PM  20.0          // seuil affichage LED pour les PM
    int         M_LED_DEFAUT  = M_PM25;       // choix de la mesure à afficher

//--------------------------------------------- parametres wifi
  #ifdef RESEAUWIFI
    #define     SERVEUR_AI4GOOD_FINGER  "08 3B 71 72 02 43 6E CA ED 42 86 93 BA 7E DF 81 C4 BC 62 30‎"
    #define     SERVEUR_AI4GOOD_VAR     "http://ai-for-good-api.herokuapp.com/api/v1/var"
    #define     SERVEUR_AI4GOOD_PARAM   "https://ai-for-good-api.herokuapp.com/api/v1/params"
    #define     SERVEUR_AI4GOOD_LOGIN   "https://ai-for-good-api.herokuapp.com/api/v1/login"
    #define     SERVEUR_AI4GOOD_DATA    "https://ai-for-good-api.herokuapp.com/api/v1/data"
    #define     SERVEUR_AI4GOOD_LOG     "https://ai-for-good-api.herokuapp.com/api/v1/log"
    #define     AI4GOOD_USERNAME        "Ai4Good"
    #define     AI4GOOD_PASSWORD        "eGXyyne2RTp4JxGJ6cX8Ggn3"
    const char *AUTO_CONNECT          = "AI for GOOD";
  #endif
  #ifdef COMPRESSION
//--------------------------------------------- parametres compression
    const int   NBREG   = 2;                  // nombre de régression de niveau 1 ex. 8
    const int   NBREG0  = 2;                  // nombre de points pour la régression initiale ex. 2
    const int   NBREG1  = 2;                  // nombre de points pour la régression secondaire ex. 2
    const float MINI    = 0.0;                // plage mini et maxi des mesures prise en compte (écrétage sinon)
    const float MAXI    = 500.0;              // plage mini et maxi des mesures prise en compte (écrétage sinon)
    const int   CODAGEECT  = 0;               // codage de l'écart-type réultant (0: non, 1: oui)
  
//--------------------------------------------- parametres codage
    const int   BIT_0   = 8;                  // nb de bits pour les points de niveau 0 ex. 8
    const int   BIT_1   = 4;                  // nb de bits pour les points de niveau 1 ex. 8
    const int   BITECT  = 4;                  // nb de bits pour les ecart-type ex. 4
  
//--------------------------------------------- paramètres de l'échantillon
    const int   TAILLE_ECH  = 8;                             // nombre de mesures d'un échantillon de la régression principale ex.32
  #endif
/*
 ****************************************************************************************************************
             structures de données
 ****************************************************************************************************************
*/
  struct DeviceType {
    String      deviceId;                     // adresse MAC
    String      type;                         // fixe ou mobile
    String      logiciel;                     // n° de version
    String      materiel;                     // config physique
    String      adresseIP;
  };
  struct Device {
    unsigned int  date;                       // date de la mesure
    int         niveauBatterie;
    String      ressenti;
    String      longitude;
    String      latitude;
    int         tempsCycle;                   // durée (ms) entre deux mesures voir valeur initiale dans parametre.h
    int         mesureLED;                    // numéro de mesure affichée sur la LED voir valeur initiale dans parametre.h
  };
  struct MesureType {
    String      capteur;                      // type de capteur
    float       valeurMin;                    // valeur min autorisée pour la mesure
    float       valeurMax;                    // valeur max autorisée pour la mesure
    String      type;                         // type de mesure réalisée
  };
  struct Mesure {
    int         nombre;                       // nombre de mesures effectuées dans le temps de cycle
    int         nombreOk;                     // nombre de mesures correctes dans le temps de cycle
    double      valeur;                       // valeur de la mesure calculée dans le temps de cycle
    double      valeurFiltree;                // valeur de la mesure filtrée
    float       ecartType;                    // écart-type des valeurs bonnes
    unsigned int  date;                       // date de la mesure
    float       tauxErreur;                   // ratio nombre de mesures correctes / nombre de mesures effectuées
  };
  #ifdef BOARDSIGFOX
    typedef struct __attribute__ ((packed)) sigfox_message {
      uint32_t  msg1;
      uint32_t  msg2;
      uint32_t  msg3;
    } SigfoxMessage;
  #endif
#endif
