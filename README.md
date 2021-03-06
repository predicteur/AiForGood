# Intégration et gestion locale de données de mesure (issues de capteurs)

## Objectif
- Gestion et restitution locale de mesures issues de capteurs
- Mise à disposition des données sur serveur des données
- Utilisation fixe ou mobile

## Fonctions
Les principales fonctions intégrées sont les suivantes :
- interfacage filaire avec les capteurs,
- génération de mesures qualifiées (calcul d'un niveau de qualité)
- horodatage des mesures
- restitution visuelle des mesures (affichage des valeurs sur LED)
- stockage local des données pour reprise sur défaut de connexion
- envoi des données via différents réseaux (WiFi, Sigfox, LoRa)
- réglage du niveau de luminosité 
- avertissement LED du niveau d'alimentation trop faible
- avertissement LED de l'absence de connexion (Wifi)
- remontée de logs sur le serveur
- géolocalisation des données (GPS)

Des fonctions complémentaires sont également intégrées suivant les configurations mises en place :
- compressions des données pour les réseaux bas-débit (LoRa, Sigfox)
- restitution sur navigateur web (WiFi)
- gestion de modes de fonctionnement : autonome, veille, connecté (WiFi) 

## Mode de fonctionnement WiFi
Les modes de fonctionnement sont les suivants :
![image](https://github.com/predicteur/AiForGood/blob/arduino_sketch/%C3%A9tats.png)


## Plateforme matérielle
- utilisation mobile : ESP8266
- utilisation fixe : MKR1200 (Sigfox), MKR1300 (LoRa)

## Paramétrage
Les paramètres de fonctionnement suivants sont modifiables au fil de l'eau (via navigateur) :
- fréquence d'acquisition des données
- nombre d'échantillons de mesure du niveau de qualité
- seuils haut et bas de validité des mesures
- valeurs sur trois niveaux d'affichage LED des mesures
- niveau de luminosité
- mode de fonctionnement
- fréquence d'envoi des données (Sigfox)
