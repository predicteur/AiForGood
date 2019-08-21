# Intégration de remontées de données via des capteurs

## Objectif
Mettre à disposition d'une application sur serveur les données issues de capteurs pour des utilisations fixes ou mobiles.

## Fonctions
Les principales fonctions intégrées sont les suivantes :
- interfacage filaire avec les capteurs,
- génération de mesures qualifiées (calcul d'un niveau de qualité)
- restitution visuelle des mesures (affichage des valeurs sur LED)
- stockage local des données pour reprise sur défaut
- envoi des données via différents réseaux (WiFi, Sigfox, LoRa)

Des fonctions complémentaires sont également intégrées suivants les configurations mises en place :
- compressions des données pour les réseaux bas-débit (LoRa, Sigfox)
- restitution sur navigateur web (WiFi)

La fonction de géolocalisation est pour l'instant externe à l'application

## Plateforme matérielle
- utilisation mobile : ESP8266
- utilisation fixe : MKR1200 (Sigfox), MKR1300 (LoRa)
