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
      Log(4, "mes[device.mesureLED].valeur : " + String(mes[device.mesureLED].valeur), "");
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
