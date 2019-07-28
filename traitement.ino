
void TraitementMesure(int a, int b, int c){
  Serial.print("Nombre de mesures : "); Serial.print(mes[0].nombre);
  Serial.print(" Valeur : "); Serial.print(mes[0].valeur); Serial.print(" Ecart-type : "); Serial.print(mes[0].ecartType);
  Serial.print(" Taux d erreur : "); Serial.println(mes[0].tauxErreur);
  if ((mes[0].nombreOk > 0) and (mes[1].nombreOk > 0)) {
      sendData(mes[0].valeur, mes[1].valeur, a, b, c);
      updateLed(mes[0].valeur, mes[1].valeur);
      //Serial.println("mesure envoyée");
  } 
  else {
      //Serial.println("mesure non envoyée");
  }
}

void LireCapteur(){
  PmResult pm_val = sds.queryPm();
  if (pm_val.isOk()) {
    pm[0] = pm_val.pm25;
    pm[1] = pm_val.pm10;
  }
}

void sendData(double z, double k, int a, int b, int c) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  String string_a = String(a, 2);
  String string_b = String(b, 2);
  String string_c = String(c, 2);
  String string_z = String(z, 2);
  String string_k = String(k, 2);
  root["device"] = device_name;
  root["PM2_5"] = string_z;
  root["PM10"] = string_k;
  root["mixed_feeling"] = string_b;
  root["negative_feeling"] = string_c;
  root["positive_feeling"] = string_a;

  root.printTo(Serial);
  char JSONmessageBuffer[300];
  root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println(JSONmessageBuffer);

  if (WiFi.status() == WL_CONNECTED) {                                  //Check WiFi connection status
    HTTPClient http;                                                    //Declare object of class HTTPClient
    http.begin("http://simple-ai4good-sensors-api.herokuapp.com/data"); //Specify request destination
    http.addHeader("Content-Type", "application/json");                 //Specify content-type header
    int httpCode = http.POST(JSONmessageBuffer);                        //Send the request
    String payload = http.getString();                                  //Get the response payload
    Serial.println(httpCode);                                           //Print HTTP return code
    Serial.println(payload);                                            //Print request response payload
    http.end();                                                         //Close connection
  }
  else
  {
    delay(500);
    strip.setPixelColor(0, strip.Color(255, 165, 0));
    strip.show();
    delay(500);
    strip.setPixelColor(0, strip.Color(30, 144, 255));
    strip.show();
  }
}

void updateLed(double a, double b) {        //Function UpdateOled Pm value
  int myInt = (int)a;
  switch (myInt) {
    case 0 ... 10:
      strip.setPixelColor(0, strip.Color(0, 128, 0));
      break;
    case 11 ... 20:
      strip.setPixelColor(0, strip.Color(255, 165, 0));
      break;
    case 21 ... 1000:
      strip.setPixelColor(0, strip.Color(127, 0, 0));
      break;
  }
  strip.show();
}

void  TraitementRessenti(String requete) {
    if (requete == "/BIEN") {
      sensorVal1 = 1;
      sensorVal2 = 0;
      sensorVal3 = 0;
    }
    else if (requete == "/NORMAL") {
      sensorVal1 = 0;
      sensorVal2 = 1;
      sensorVal3 = 0;
    }
    else if (requete == "/PASBIEN") {
      sensorVal1 = 0;
      sensorVal2 = 0;
      sensorVal3 = 1;
    }
}

void GenerationPage(String s) {
    s = "<html lang='fr'><head><meta http-equiv='refresh' content='60' name='viewport' content='width=device-width, initial-scale=1'/>";

    s += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'>";
    s += "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>";
    s += "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>";

    s += "<body style='background:url('https://zupimages.net/up/19/29/jblq.jpg') no-repeat center/100% 100%;'>";
    s += "<div class='container container-fluid px-5' style='min-height: 100%;'>";
    s += "<div class='row'>";
    s += "<div class='col-md-12'>";
    s += "<h3 class='text-center'>";
    s += "AlabintheAIR";
    s += "</h3>";
    s += "<br /><br />";
    s += "<table class='table text-center'>";
    s += "<tbody>";
    s += "<tr>";
    s += "<td>PM2.5</td>";
    s += "<td>";
    s += pm[0];
    s += "</td>";
    s += "</tr>";
    s += "<tr class='table-active'>";
    s += "<td>PM10</td>";
    s += "<td>";
    s += pm[1];
    s += "</td>";
    s += "</tr>";
    s += "</tbody>";
    s += "<th />";
    s += "</table>";
    s += "<h3 class='text-center'>My FEELING</h3>";
    s += "<br /><br />";
    s += "<div class='row'>";
    s += "<div class='col-md-12'>";
    s += "<a class='btn btn-success btn-lg btn-block' style='padding-top:3%;padding-bottom:3%;' href=\"BIEN\">FRESH AIR</a>";
    s += "<br /><br />";
    s += "<a class='btn btn-primary btn-lg btn-block' style='padding-top:3%;padding-bottom:3%;' href=\"NORMAL\">COMMON AIR</a>";
    s += "<br /><br />";
    s += "<a class='btn btn-danger btn-lg btn-block' style='padding-top:3%;padding-bottom:3%;' href=\"PASBIEN\">STALE AIR</a>";
    s += "<br />";
    s += "</div>";
    s += "</div>";
    s += "</div>";
    s += "</div>";
    s += "</div>";
    s += "</body> </html> \n";
}
