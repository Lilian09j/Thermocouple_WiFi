/* Create a WiFi access point and provide a web server on it.
Just a little test message.  Go to http://192.168.4.1 in a web browser
   connected to this access point to see it.*/   
#include <ESP8266WiFi.h>
#include <SPI.h>

#define Vref 3.3 // tension de reference du MCP3301 en V ici 3.3V
#define Gain 56.22 // gain de l'amplification de la tension mesuree résultant du circuit des 2 AOP = 28.11 mais en pratique environ le double

int led = D0; //led sur le pin D0
const int csPin = D8;  // Chip Select MCP3301
const char *ssid = "Thermocouple-WiFi";// Identifiant WiFi
const char *password = "ENSIBS2023";// Mot de passe WiFi
const int LM335 = A0;

bool sign=0;
int i=0, adcValue=0, z=0;
float tempSF=0, maxi=0, tensionmV=0, mini=0, moy=0, acc=0;

byte highByte=0x00, lowByte=0x00;

// Définir le port de communication TCP
const int serverPort = 12345;

// Créer un objet WiFiServer sur le port spécifié
WiFiServer server(serverPort);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  pinMode(led, OUTPUT); //Instanciation de la LED comme une sortie
  digitalWrite(led, HIGH);
  pinMode(csPin, OUTPUT);
  pinMode(LM335, INPUT);
  digitalWrite(csPin, HIGH);  // Désactive le MCP3301 (slave)
  
  //Creation du point d acces wifi AP
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  // Commencer à écouter sur le port spécifié
  server.begin();
  Serial.print("Serveur en ecoute sur le port ");
  Serial.println(serverPort);
  Serial.print("En attente de connexions clients");

  delay(100);
  digitalWrite(led, LOW);
}

void loop() {
  digitalWrite(led, LOW);
  // Verifier si un client est connecte
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Nouvelle connexion");
    while (client.connected()) {
      if(z%20==0){digitalWrite(led, HIGH); z=1;}//allume la led tous les 20 cycles d'émissions
      //Gestion mesure Soudure Froidre SF
      tempSF = analogRead(LM335)*3.3/1023;//Converti la valeur numerique en tension
      tempSF = tempSF*100-273;//Tension > kelvin > celsius (temperature soudure froide SF)
      Serial.print("SF : ");Serial.print(tempSF);Serial.println(" °C");
      if(tempSF >= maxi){maxi=tempSF;}
      if(tempSF <= mini | i==1){mini=tempSF;}
      acc += tempSF;
      moy = acc / i;
      i++;

      //Gestion mesure Soudure Chaude SC
      digitalWrite(csPin, LOW);  // Active le MCP3301 (slave)
      // Lit les deux octets de données renvoyés par le MCP3301
      highByte = SPI.transfer(0);
      lowByte = SPI.transfer(0);
      digitalWrite(csPin, HIGH); // Désactive le MCP3301
      // Calcule la valeur de tension à partir des octets lus
      sign = ((highByte << 8) | lowByte) & 0x1000;
      adcValue = ((highByte << 8) | lowByte) & 0x0FFF;//le 13eme bit est le bit de signe
      if(sign==0){//tension positive
        tensionmV = 1000 * adcValue * Vref / 4096 / Gain;
        // Affiche la valeur de tension sur la console serie
        Serial.print("signe : ");Serial.print(sign, BIN);Serial.print("   adcValue : ");Serial.println(adcValue, BIN);
        Serial.print("Tension : ");Serial.print(tensionmV);Serial.println(" mV");
        // Envoyer des données au client
        String data = String(tensionmV) + "," + String(tempSF);
        client.print(data);
        delay(50);
        digitalWrite(led, LOW); 
        delay(50);// periode 100ms soit 10Hz de frequence de transmission
        z++;
        }
      else if(sign==1){//tension negative
        tensionmV = 1000 * (-4096 + adcValue) * Vref / 4096 / Gain;//Complement a deux
        Serial.print("signe : ");Serial.print(sign, BIN);Serial.print("   adcValue : ");Serial.println(adcValue, BIN);
        Serial.print("Tension : ");Serial.print(tensionmV);Serial.println(" mV");
        String data = String(tensionmV) + "," + String(tempSF);
        client.print(data);
        delay(50);
        digitalWrite(led, LOW);     
        delay(50);
        z++;
        }
      else{Serial.print("erreur signe");}
    }
    
    Serial.println("Connexion terminee");
    WiFi.softAPdisconnect(true);//arret du point d acces
    Serial.println("Point d'accès WiFi fermé");
    server.close();
        
    //Creation du point d acces wifi AP
    Serial.print("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    // Commencer à écouter sur le port spécifié
    server.begin();
    Serial.print("Serveur en ecoute sur le port ");
    Serial.println(serverPort);
    Serial.print("En attente de connexions clients");
  }

  delay(500);
  Serial.print(".");
  digitalWrite(led, 0);
}
