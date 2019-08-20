void reg(float serie[], int len){
    float sx = 0, sy = 0, sxy = 0, sx2 = 0, a = 0, b = 0;
    float secart[len];
    if (len == 1){
        coef.b = serie[0];
        coef.a = 0.0;
    } else if (len == 2){
        coef.b = 2.0 * serie[0] - serie[1];
        coef.a = serie[1] - serie[0];
    } else {
        for (int i=0; i<len; i++){
          sx += (i+1);
          sy += serie[i];
          sxy += (i+1) * serie[i];
          sx2 += (i+1) * (i+1);
        }
        coef.a = (sx * sy - len * sxy) / (sx * sx - len * sx2);
        coef.b = sy / len - coef.a * sx / len;
    }
    ecart(coef.a, coef.b, serie, secart, len);
    coef.ect = et(secart, len);
}
float rega(float serie[], float a, int len){   // a fixé, calcul de b (b = moyenne(yi) - a * moyenne(xi))
    float sx = 0.0, sy = 0.0, b = 0.0;
    if (len == 1){
        b = serie[0];
    } else {
        for (int i=0; i<len; i++){
            sx += (i+1);
            sy += serie[i];
        }
        b = sy / len - a * sx / len;
    }
    return b;
}
void normalisation(float serie[], float sserie[], float mini, float maxi, float c, int len){
    float minic = pow(mini, c);
    float maxic = pow(maxi, c);
    for (int i=0; i<len; i++){
        sserie[i] = (pow(min(maxi, max(mini, serie[i])), c) - minic)/(maxic - minic);
    }
    prSerie(sserie,TAILLE_ECH, "sserie");
}
void denormalisation(float serie[], float mini, float maxi, float c, int len){
    float minic = pow(mini, c);
    float maxic = pow(maxi, c);
    for (int i=0; i<len; i++){
          serie[i] = min(maxi, max(mini, float(pow(minic + (maxic - minic) * serie[i], 1.0 / c))));
    }
}
void codage(){
    float et0 = coefc.et0;
    coefi.a0  = conversion(coefc.a0 , -1.0/float(TAILLE_ECH), 1.0/float(TAILLE_ECH) , BITS);
    coefi.b0  = conversion(coefc.b0 , 0                     , 2.0                   , BITS);
    coefi.et0 = conversion(coefc.et0, 0                     , 0.5                   , BITS);
    coefi.ect = conversion(coefc.ect, 0                     , 0.5                   , BITS);
    for (int i=0; i<NBREG; i++){
        coefi.a1[i] = conversion(coefc.a1[i], -PLA*et0/TAILLE_ECH*2.0, PLA*et0/TAILLE_ECH*2.0, BITC);
        coefi.b1[i] = conversion(coefc.b1[i], -PLB*et0               , PLB*et0               , BITC);
    }
}
void decodage(){
    coefp.a0  = conversionb(coefi.a0 , -1.0/float(TAILLE_ECH), 1.0/float(TAILLE_ECH), BITS);
    coefp.b0  = conversionb(coefi.b0 , 0                     , 2.0                  , BITS);
    coefp.et0 = conversionb(coefi.et0, 0                     , 0.5                  , BITS);
    coefp.ect = conversionb(coefi.ect, 0                     , 0.5                  , BITS);
    for (int i=0; i<NBREG; i++){
        coefp.a1[i] = conversionb(coefi.a1[i], -PLA*coefp.et0/TAILLE_ECH*2.0, PLA * coefp.et0/TAILLE_ECH*2.0, BITC);
        coefp.b1[i] = conversionb(coefi.b1[i], -PLB*coefp.et0               , PLB * coefp.et0               , BITC);
    }
}
void compression(){
    float y1ecr[TAILLE_ECH], y1est[TAILLE_ECH], y2[TAILLE_ECH2];
    normalisation(y0init, y0n, MINI, MAXI, RACINE, TAILLE_ECH);
    reg(y0n, TAILLE_ECH);
    coefc.a0 = coef.a;
    coefc.b0 = coef.b;
    coefc.et0 = coef.ect;
    ecart(coefc.a0, coefc.b0, y0n, y1est, TAILLE_ECH);
    ecretage(y1est, -ECRET*coefc.et0, ECRET*coefc.et0, y1ecr, TAILLE_ECH);
    for (int i=0; i<NBREG; i++){
        sousSerie(y1ecr, y2, i, TAILLE_ECH2);
        reg(y2, TAILLE_ECH2);
        coefc.a1[i] = coef.a;
        coefc.b1[i] = coef.b;
    }
}
void decompression(){
    estim(coefp.a0, coefp.b0, TAILLE_ECH, y0fon);
    for (int i=0; i<NBREG; i++){
      for (int j=0; j<TAILLE_ECH2; j++){
          y0fon[i*TAILLE_ECH2+j] += (coefp.a1[i] * (j +1) + coefp.b1[i]);
      }
    }
    denormalisation(y0fon, MINI, MAXI, RACINE, TAILLE_ECH);
}
void optimisation(){
    float y1[TAILLE_ECH], y2[TAILLE_ECH2];
    coefc.a0 = coefp.a0;
    coefc.b0 = rega(y0n, coefp.a0, TAILLE_ECH);
    coefc.ect = coefp.ect;
    ecart(coefc.a0, coefc.b0, y0n, y1, TAILLE_ECH);
    coefc.et0 = et(y1, TAILLE_ECH);
    for (int i=0; i<NBREG; i++){
      coefc.a1[i] = coefp.a1[i];
      sousSerie(y1, y2, i, TAILLE_ECH2);
      coefc.b1[i] = rega(y2, coefc.a1[i], TAILLE_ECH2);
    }
}
void codbin(){
    int payl[TAILLE_PAY];
    addbin(coefi.a0,  payl, BITS, 0      );
    addbin(coefi.b0,  payl, BITS, BITS   );
    addbin(coefi.et0, payl, BITS, 2*BITS );
    addbin(coefi.ect, payl, BITS, 3*BITS );
    for (int i=0; i<NBREG; i++){
        addbin(coefi.a1[i], payl, BITC, 4*BITS+(i*2)  *BITC );
        addbin(coefi.b1[i], payl, BITC, 4*BITS+(i*2+1)*BITC );
    }
    payload.msg1 = decbin(payl, TAILLE_MSG, 0);
    payload.msg2 = decbin(payl, TAILLE_MSG, 32);
    payload.msg3 = decbin(payl, TAILLE_MSG, 64);
}
void decodbin(){
    int payl[TAILLE_PAY];
    addbin(payload.msg1, payl, TAILLE_MSG, 0);
    addbin(payload.msg2, payl, TAILLE_MSG, 32);
    addbin(payload.msg3, payl, TAILLE_MSG, 64);
    coefi.a0  = decbin(payl, BITS, 0);
    coefi.b0  = decbin(payl, BITS, BITS);
    coefi.et0 = decbin(payl, BITS, 2*BITS);
    coefi.ect = decbin(payl, BITS, 3*BITS);
    for (int i=0; i<NBREG; i++){
        coefi.a1[i] = decbin(payl, BITC, 4*BITS+(i*2)  *BITC);
        coefi.b1[i] = decbin(payl, BITC, 4*BITS+(i*2+1)*BITC);
    }
}
void compress(){
    float y1[TAILLE_ECH], ecartType;
    compression();
    // optimisation
    prSerie(y0n, TAILLE_ECH, "y0n apres compression");
    codage();
    decodage();        
    optimisation();
    // calcul de l'écart-type final
    codage; 
    decodage();
    decompression();
    prSerie(y0fon, TAILLE_ECH, "y0fon apres compression");
    diff(y0init, y0fon, y1, TAILLE_ECH);
    ecartType = et(y1, TAILLE_ECH);
    // codage et envoi du payload
    coefc.ect = (pow(min(MAXI, max(MINI, ecartType)), RACINE) - pow(MINI, RACINE))/(pow(MAXI, RACINE) - pow(MINI, RACINE)); 
    codage();   
    codbin();
}
float decompress(){
    float ecartType;
    decodbin();
    decodage();
    ecartType = float(pow(min(MAXI, max(MINI, MINI + (MAXI - MINI) * coefp.ect)), 1.0 / RACINE));
    decompression();
    return ecartType;
}   

