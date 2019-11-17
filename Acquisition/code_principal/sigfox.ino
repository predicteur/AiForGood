#ifdef COMPRESSION
//-----------------------------------------------------------------------------------------------------------------------------
void GroupeMesure() {
    for (int i = 0; i < NB_MES; ++i) {
      mesEnvoi[i][nbMesureGroupe].nom          = mes[i].nom;
      mesEnvoi[i][nbMesureGroupe].nombre       = mes[i].nombre;
      mesEnvoi[i][nbMesureGroupe].nombreOk     = mes[i].nombreOk;
      mesEnvoi[i][nbMesureGroupe].valeur       = mes[i].valeur;
      mesEnvoi[i][nbMesureGroupe].ecartType    = mes[i].ecartType;
      mesEnvoi[i][nbMesureGroupe].date         = mes[i].date;
      mesEnvoi[i][nbMesureGroupe].valeurMin    = mes[i].valeurMin;
      mesEnvoi[i][nbMesureGroupe].valeurMax    = mes[i].valeurMax;
      mesEnvoi[i][nbMesureGroupe].tauxErreur   = mes[i].tauxErreur;  }
    
    //stockage en mémoire à mettre

  }
//-----------------------------------------------------------------------------------------------------------------------------
  void GenereGroupe() {
    for (int i = 0; i < TAILLE_ECH; ++i) y0i[i] = mesEnvoi[mesureLED][i].valeur;
    for (int i = 0; i < NB_MES; ++i) {
      for (int j = 0; j < TAILLE_ECH; ++j) {
        mesEnvoi[i][j].nombre       = 0;
        mesEnvoi[i][j].nombreOk     = 0;
        mesEnvoi[i][j].valeur       = 0;
        mesEnvoi[i][j].ecartType    = 0;
        mesEnvoi[i][j].date         = 0;
        mesEnvoi[i][j].tauxErreur   = 0;  }  }
  }
#endif
#ifdef BOARDSIGFOX
//-----------------------------------------------------------------------------------------------------------------------------
  void reboot() {
    NVIC_SystemReset();
    while (1);
  }
//-----------------------------------------------------------------------------------------------------------------------------
  void EnvoiSigfox(){
    theme = "sigfox  ";
    if (!test) { SigFox.begin(); delay(100); }          // Start the module, Wait at least 30ms after first configuration (100ms before)
    if (oneshot && !test) {
      Log(4, "SigFox FW version " + SigFox.SigVersion(), "");
      Log(4, "ID  = " + SigFox.ID() + "  PAC = " + SigFox.PAC(), "");  }
    if (oneshot || test) {
      Log(4, "mes[mesureLED].valeur : " + String(mes[mesureLED].valeur), "");
      Log(4, "payload: " + String(payload.msg1) + " " + String(payload.msg2) + " " + String(payload.msg3), "");  }
    if (!test) {
      SigFox.status(); delay(1);
      SigFox.beginPacket();
      SigFox.write((uint8_t*)&payload, 12);
      lastMessageStatus = SigFox.endPacket();
      SigFox.end();  }
    if (oneshot) Log(4, " LastStatus (0 = OK): " + String(lastMessageStatus), "");
    // spin forever, so we can test that the backend is behaving correctly
    //if (oneshot && !test) {
    //  while (1) {}
    //}
    //Sleep pendant la période considérée
    //if (!test) {
      //LowPower.sleep(periode * 1000);
    //}
  }  
#endif
