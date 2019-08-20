
#include <ArduinoLowPower.h>
#include <SdsDustSensor.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <SigFox.h>

// configuration matérielle
#define LED_PIN       8         // affichage de l'état des mesures
#define LED_COUNT     1         // nombre de LED dans le ruban
// configuration mesures
#define TEMPS_CYCLE     15000   // période d envoi des mesures en millisecondes
#define NB_MESURE       5       // nombre de mesure élémentaires dans le temps de cycle 
#define VALEUR_MIN_PM   0.0     // limite mini autorisee pour les PM
#define VALEUR_MAX_PM   1000.0  // limite maxi autorisee pour les PM
#define SEUIL_BON_PM    10.0    // seuil affichage pour les PM
#define SEUIL_MOYEN_PM  20.0    // seuil affichage pour les PM
// configuration Sigfox
#define STATUS_OK   0
#define STATUS_1_KO 1
#define STATUS_2_KO 2

// parametres Sigfox
const boolean test = false;      // test uniquement des capteurs sans utilisation de Sigfox
const boolean oneshot = true;   // Set oneshot to false to trigger continuous mode when you finisched setting up the whole flow
// parametres mesures
const int     NB_MES  = 2;               // quantité de mesures à effectuer (ici 2 :PM25 et PM10)
const char    *DEVICE_NAME = "sensor9";   // nom du device a documenter
// parametres affichage LED
const int     ROUGE[3]  = {127, 0, 0};
const int     ORANGE[3] = {255, 165, 0};
const int     BLEU[3]   = {30, 144, 255};
const int     VERT[3]   = {0, 128, 0};
const int     LUMINOSITE_FORTE  = 250;   // maxi 255
const int     LUMINOSITE_FAIBLE = 50;
// parametres compression
const int   ECRET   = 10;       // coef d'écrétage des écarts pour la régression initiale ex. 2 (coef multiplié à l'écart-type)
const int   NBREG   = 4;        // nombre de régression de niveau 1 ex. 8
const float RACINE  = 0.5;        // fonction de normalisation des données (données ** racine) racine <= 1.0 ex. racine = 0.5
const float MINI    = 0.0;      // plage mini et maxi des mesures prise en compte (écrétage sinon)
const float MAXI    = 100.0;     // plage mini et maxi des mesures prise en compte (écrétage sinon)
// parametres codage
const float PLA     = 12.0;     // plage pour les coefficients a de niveau 1 : abs(a) < pla * ecart-type ex. 10.0
const float PLB     = 3.0;      // plage pour les coefficients b de niveau 1 : abs(b) < plb * ecart-type ex. 3.0
const int   BITS    = 8;        // nb de bits pour les coeff de niveau 0 ex. 8
const int   BITC    = 4;        // nb de bits pour les coeff de niveau 1 ex. 4
// paramètres de l'échantillon
const int   TAILLE_ECH  = 4;                             // nombre de mesures d'un échantillon de la régression principale ex.32
const int   TAILLE_ECH2 = TAILLE_ECH / NBREG;             // nombre de mesures des échantillons de la régression complémentaire
// paramètres de l'envoi
const int   TAILLE_MSG  = 32;                             // nombre de bit des messages composant le "payload" (3 messages pour un payload Sigfox de 12 octets)
const int   TAILLE_PAY  = 96;                             // nombre de bit du "payload" (Sigfox 12 octets)
const int   TOTAL_BIT   = (4 * BITS + NBREG * 2 * BITC);  // nombre de bit à envoyer

struct Mesure {
  String  nom;                  // nom de la mesure ex. PM10
  int     nombre;               // nombre de mesures effectuées dans le temps de cycle
  int     nombreOk;             // nombre de mesures correctes dans le temps de cycle
  double  valeur;               // valeur de la mesure calculée dans le temps de cycle
  float   ecartType;            // écart-type des valeurs bonnes
  float   date;                 // date de la mesure
  float   valeurMin;            // valeur min autorisée pour la mesure
  float   valeurMax;            // valeur max autorisée pour la mesure
  float   tauxErreur;           // ratio nombre de mesures correctes / nombre de mesures effectuées
} mes[NB_MES], mesEnvoi[NB_MES][TAILLE_ECH];      // mesures à envoyer

struct CoefReg {
  float   a;                    // coef a régression linéaire y = a * x + b
  float   b;                    // coef b régression linéaire y = a * x + b
  float   ect;                  // écart-type entre la valeut y et l'estimation y = a x + b
} coef = {0.0, 0.0, 0.0};                                 // paramètre de chaque régression unitaire

struct CoefComp {
  float   a0;                   // coef a régression linéaire principale
  float   b0;                   // coef b régression linéaire principale
  float   et0;                  // écart-type entre la valeut y et l'estimation principale
  float   a1[NBREG];            // coef a régression linéaire secondaire
  float   b1[NBREG];            // coef b régression linéaire secondaire
  float   ect;                  // écart-type global entre la valeut y et l'estimation complète
} coefc = {0.0, 0.0, 0.0, {}, {}, 0.0},  coefp = {0.0, 0.0, 0.0, {}, {}, 0.0};         // coefc : paramètres issue de la compression ou de l'optimisation, coefp : paramètres issus du decodage

struct CoefCode {
  int   a0;                     // coef a régression linéaire principale
  int   b0;                     // coef b régression linéaire principale
  int   et0;                    // écart-type entre la valeut y et l'estimation principale
  int   a1[NBREG];              // coef a régression linéaire secondaire
  int   b1[NBREG];              // coef b régression linéaire secondaire
  int   ect;                    // écart-type global entre la valeut y et l'estimation complète
} coefi = {0, 0, 0.0, {}, {}, 0.0};                       // paramètre de chaque régression unitaire

typedef struct __attribute__ ((packed)) sigfox_message {
  uint32_t msg1;
  uint32_t msg2;
  uint32_t msg3;
} SigfoxMessage;
SigfoxMessage payload;          // message envoyé par sigfox (12 octets découpés en trois variables de 4 octets)
int lastMessageStatus;

// variables périodicité
float   temps_ref_envoi   = 0;  // compteur de temps principal de 0 à TEMPS_CYCLE
float   temps_ref_mesure  = 0;  // compteur de temps intermédiaire de 0 à TEMPS_CYCLE/NB_MESURE
int     nombre_mesure     = 0;  // compteur de mesures de 0 à TAILLE_ECH
// variables mesures
double  pm[NB_MES];             // mesure de PM2.5 et PM10
// variables ressenti
int     sensorVal1;             
int     sensorVal2;
int     sensorVal3;
// variables series de mesures
float y0init[TAILLE_ECH], y0n[TAILLE_ECH], y0fon[TAILLE_ECH];   // y0init : mesures à compresser, Y0n : mesures normalisées (entre 0 et 1), Y0fon : mesures issues de la compressipon

// initialisation des objets
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
SdsDustSensor sds(Serial1);

void reboot() {
  NVIC_SystemReset();
  while (1);
}

void setup() {
  float ecartType;
  
  // initialisation liaison série
  if (oneshot || test) {
      Serial.begin(115200);     // Start the Serial communication to send messages to the computer
      delay(10);
      Serial.println('\n');
  }  
  // initialisation Sigfox
  if (!SigFox.begin()) {
    reboot();
  }
  SigFox.end();
  if (oneshot) {
    SigFox.debug();
  }
  // initialisation données et affichage
  CreerMesure();                // Création pm10 et pm2_5
  strip.begin();                // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                 // Turn OFF all pixels
  StripAffiche("démarrage");

  // initialisation capteur PM
  Serial.println(sds.queryFirmwareVersion().toString());       // prints firmware version
  Serial.println(sds.setActiveReportingMode().toString());     // ensures sensor is in 'active' reporting mode
  Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default but not recommended
  sds.begin();
  
  StripAffiche("démarré");
}
void loop() {
  // boucle de réalisation des mesures intermédiaires
  if (( (millis() - temps_ref_mesure) >= float(TEMPS_CYCLE)/float(NB_MESURE) ) and (NB_MESURE > 1)) {
      temps_ref_mesure = millis();
      StripAffiche("début mesure");
      LireCapteur();
      CalculMesure();
      StripAffiche("fin mesure");
      Serial.println("mesure inter " + String(pm[0]));
  }
  // boucle principale de mesure
  if ( (millis() - temps_ref_envoi) >= float(TEMPS_CYCLE) ){
      temps_ref_envoi = millis();
      if (NB_MESURE <= 1) {
        StripAffiche("début mesure");
        LireCapteur();
        CalculMesure();
        StripAffiche("fin mesure");
      }
      GenereMesure();
      TraitementMesure();  //envoi des données et affichage LED
      nombre_mesure ++;
      InitMesure();
      InitRessenti();
  }
  // boucle d'envoi des données
  if ( nombre_mesure >= TAILLE_ECH ){
      Serial.println("début envoi mesures ");
      nombre_mesure = 0;      
      genereSerie();
      prSerie(y0init, TAILLE_ECH, "y0init");
      compress();
      Serial.println("envoi mesures ");
      prSerie(y0fon, TAILLE_ECH, "y0fon apres compress");
      envoiSigfox();
  }
}
