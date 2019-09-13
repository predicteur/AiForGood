#ifndef PARAMETRE
  #define PARAMETRE
//--------------------------------------------- configuration des plateformes --------------------------------
  #ifdef SIGFOX_SPI //test du board sigfox
    #define BOARDSIGFOX
    #define COMPRESSION
    //#define BOARD "sigfox"
    //#define RESEAU "LPWAN"
  #else 
    #define RESEAUWIFI
    //#define BOARD "autre"
    //#define RESEAU "WIFI"    
  #endif

//--------------------------------------------- configuration des librairies -------------------------------- 
  #ifdef BOARDSIGFOX
    #include <SigFox.h>
    #include <ArduinoLowPower.h>
  #else
    #include <ESP8266WiFi.h>
    #include <WiFiClientSecure.h>
    #include <ESP8266WiFiMulti.h>
    #include <WiFiManager.h>
    #include <ESP8266HTTPClient.h>
    #include <FS.h>
  #endif
    #include <SdsDustSensor.h>
    #include <ArduinoJson.h>
    #include <Adafruit_NeoPixel.h>
    
/*
 ****************************************************************************************************************
             Paramètres qui impactent le code
 ****************************************************************************************************************
*/
//--------------------------------------------- parametres structure de données --------------------------------
    #define   NB_MES    2                     // nombre de mesures à effectuer (ici 2 :PM25 et PM10)
    #define   M_PM25    0                     // indice de la mesure PM25 dans le tableau des mesures
    #define   M_PM10    1                     // indice de la mesure PM10 dans le tableau des mesures
    #define   FIC_BUF   "/buffer.txt"         // fichier de stockage des mesures non encore envoyées     
    #define   TAILLE_MAX_JSON   400           // taille maxi des fichiers JSON

//--------------------------------------------- parametres Sigfox --------------------------------
  #ifdef BOARDSIGFOX
    const boolean test = false;      // test uniquement des capteurs sans utilisation de Sigfox
    const boolean oneshot = true;    // affichage des infos d'envoi sigfox
  #endif

//--------------------------------------------- parametres affichage LED --------------------------------
    const int     ROUGE[3]          = {250, 0, 0};
    const int     ORANGE[3]         = {200, 100, 0};
    const int     BLEU[3]           = {0, 0, 250};
    const int     VIOLET[3]         = {250, 0, 200};
    const int     VERT[3]           = {0, 250, 0};
    const int     LUMINOSITE_FORTE  = 250;   // maxi 255
    const int     LUMINOSITE_FAIBLE = 50;
    const int     NIVEAU_FORT       = 100;                    // niveau variable de 0 à 100
    const int     NIVEAU_MOYEN      = 25;
    const int     NIVEAU_FAIBLE     = 10;                     // niveau utilisé pour le niveu bas de la batterie
    const int     NIVEAU_ETEINT     = 0;
    
//--------------------------------------------- paramètres de l'envoi --------------------------------
  #ifdef BOARDSIGFOX
    const int   TAILLE_MSG  = 32;                             // nombre de bit des messages composant le "payload" (3 messages pour un payload Sigfox de 12 octets)
    const int   TAILLE_PAY  = 96;                             // nombre de bit du "payload" (Sigfox 12 octets)
  #endif

/*
 ****************************************************************************************************************
             Paramètres modifiables sans avoir à modifier le code
 ****************************************************************************************************************
*/
//--------------------------------------------- configuration matérielle / logicielle
  #ifdef BOARDSIGFOX
    #define LED_PIN           8           // affichage de l'état des mesures
  #else
    #define LED_PIN           D1          // affichage de l'état des mesures (D8 par défaut, D1 sur ESP de test Philippe)
    #define RXPIN             14          // laison capteur
    #define TXPIN             12          // laison capteur
  #endif
    #define LED_COUNT         1           // nombre de LED dans le ruban
    #define MEM_IDENTIFIANT   1           // 0 : pas de mémorisation des accès WiFi, 1 : mémorisation  
    #define MODE_LOG          "verbose"    // "normal" : infos(0), warning(1), erreur(2), "verbose" : detail(3), "debug" : debug(4)
    const char *DEVICE_NAME = "sensor9";  // nom du device a documenter

//--------------------------------------------- configuration mesures
    #define TEMPS_CYCLE     20000         // Temps de cycle : période d envoi des mesures au serveur en millisecondes
    #define NB_MESURE       5             // nombre de mesure élémentaires dans le temps de cycle pour calcul du niveau de qualité
    #define VALEUR_MIN_PM   0.0           // limite mini autorisee pour les PM
    #define VALEUR_MAX_PM   1000.0        // limite maxi autorisee pour les PM
    #define SEUIL_BON_PM    10.0          // seuil affichage LED pour les PM
    #define SEUIL_MOYEN_PM  20.0          // seuil affichage LED pour les PM
    const int M_LED       = M_PM25;       // choix de la mesure à afficher

//--------------------------------------------- parametres wifi
  #ifdef RESEAUWIFI
    #define SERVEUR_AI4GOOD_VAR     "http://ai-for-good-api.herokuapp.com/api/v1/var"
    #define SERVEUR_AI4GOOD_LOGIN   "https://ai-for-good-api.herokuapp.com/login"
    #define SERVEUR_AI4GOOD_FINGER  "08 3B 71 72 02 43 6E CA ED 42 86 93 BA 7E DF 81 C4 BC 62 30‎"
    #define SERVEUR_AI4GOOD_DATA    "https://ai-for-good-api.herokuapp.com/send/data"
    #define SERVEUR_AI4GOOD_LOG     "https://ai-for-good-api.herokuapp.com/send/log"
    #define AI4GOOD_USERNAME        "Ai4Good"
    #define AI4GOOD_PASSWORD        "eGXyyne2RTp4JxGJ6cX8Ggn3"
    const char *AUTO_CONNECT      = "AI for GOOD";
  #endif

  #ifdef COMPRESSION
//--------------------------------------------- parametres compression
    const float SEUIL_ECT = 0.001; // seuil minimum de l'écart-type (évite division par 0)
    const int   ECRET   = 2;       // coef d'écrétage des écarts pour la régression initiale ex. 2 (coef multiplié à l'écart-type)
    const int   NBREG   = 8;       // nombre de régression de niveau 1 ex. 8
    const float RACINE  = 0.5;     // fonction de normalisation des données (données ** racine) racine <= 1.0 ex. racine = 0.5
    const float MINI    = 0.0;     // plage mini et maxi des mesures prise en compte (écrétage sinon)
    const float MAXI    = 500.0;   // plage mini et maxi des mesures prise en compte (écrétage sinon)
  
//--------------------------------------------- parametres codage
    const float PLA     = 10.0;     // plage pour les coefficients a de niveau 1 : abs(a) < pla * ecart-type ex. 10.0
    const float PLB     = 3.0;      // plage pour les coefficients b de niveau 1 : abs(b) < plb * ecart-type ex. 3.0
    const int   BITS    = 8;        // nb de bits pour les coeff de niveau 0 ex. 8
    const int   BITC    = 4;        // nb de bits pour les coeff de niveau 1 ex. 4
    const int   TOTAL_BIT   = (4 * BITS + NBREG * 2 * BITC);  // nombre de bit à envoyer
  
//--------------------------------------------- paramètres de l'échantillon
    const int   TAILLE_ECH  = 32;                             // nombre de mesures d'un échantillon de la régression principale ex.32
    const int   TAILLE_ECH2 = TAILLE_ECH / NBREG;             // nombre de mesures des échantillons de la régression complémentaire
  #endif


//--------------------------------------------- structures de données --------------------------------
  struct DateRef {
    unsigned long  dateInt;         // date interne correspondante à la date externe (en millisecondes)
    unsigned long  dateExt;         // date externe issue du serveur lors de l'envoi des données (en millisecondes)
    unsigned long  decalage;        // complément à ajouter à une date interne pour avoir une date absolue (en millisecondes)
  };
  struct Mesure {
    String  nom;                    // nom de la mesure ex. PM10
    int     nombre;                 // nombre de mesures effectuées dans le temps de cycle
    int     nombreOk;               // nombre de mesures correctes dans le temps de cycle
    double  valeur;                 // valeur de la mesure calculée dans le temps de cycle
    float   ecartType;              // écart-type des valeurs bonnes
    unsigned int  date;             // date de la mesure
    float   valeurMin;              // valeur min autorisée pour la mesure
    float   valeurMax;              // valeur max autorisée pour la mesure
    float   tauxErreur;             // ratio nombre de mesures correctes / nombre de mesures effectuées
  };
  #ifdef COMPRESSION
    struct CoefReg {
      float   a;                    // coef a régression linéaire y = a * x + b
      float   b;                    // coef b régression linéaire y = a * x + b
      float   ect;                  // écart-type entre la valeut y et l'estimation y = a x + b
    };    
    struct CoefComp {
      float   a0;                   // coef a régression linéaire principale
      float   b0;                   // coef b régression linéaire principale
      float   et0;                  // écart-type entre la valeut y et l'estimation principale
      float   a1[NBREG];            // coef a régression linéaire secondaire
      float   b1[NBREG];            // coef b régression linéaire secondaire
      float   ect;                  // écart-type global entre la valeut y et l'estimation complète
    };
    struct CoefCode {
      int   a0;                     // coef a régression linéaire principale
      int   b0;                     // coef b régression linéaire principale
      int   et0;                    // écart-type entre la valeut y et l'estimation principale
      int   a1[NBREG];              // coef a régression linéaire secondaire
      int   b1[NBREG];              // coef b régression linéaire secondaire
      int   ect;                    // écart-type global entre la valeut y et l'estimation complète
    };
  #endif
  #ifdef BOARDSIGFOX
    typedef struct __attribute__ ((packed)) sigfox_message {
      uint32_t msg1;
      uint32_t msg2;
      uint32_t msg3;
    } SigfoxMessage;
  #endif
//--------------------------------------------- affichages sur la liaison série --------------------------------
/*#ifdef DEBUG
  #define Log_ln(x)     Serial.println(x)
  #define Log_ln2(x,y)  Serial.println(x,y)
  #define Log(x)        Serial.print(x)
  #define Log2(x,y)     Serial.print(x,y)
#else
  #define Log_ln(x)     
  #define Log_ln2(x,y)  
  #define Log(x)        
  #define Log2(x,y)     
#endif*/


#endif
