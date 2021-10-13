/*
 * Transmite la lectura del sensor utrasonico mediante MQTT
 * Por: Eduardo Rodriguez-Martinez
 * Fecha: 1 de octubre de 2021
 * 
 * Este programa transmite la lectura del sensor utrasonico
 * hc-sr04 y la transmite usando el protocolo MQTT
 * 
 * HC-SC04      PinESP32CAM   
 * Vcc----------5V
 * Trig---------GPIO 12
 * Echo---------GPIO 13
 * Gnd----------GND
 */

//Bibliotecas
#include <WiFi.h>        // Biblioteca para el control de WiFi
#include <PubSubClient.h>// Biblioteca para conexion MQTT
#include <Ultrasonic.h>  // Biblioteca para el control del HC-SC04

//Definición de pines
#define LED_STATUS 33 // GPIO33 indicará el estado de la conexión WiFi
#define Trig 12       // GPIO12 mandará la señal Trigger
#define Echo 13       // GPIO13 mandará la señal Echo

//Datos de WiFi
#define ssid "IZZI-F16E"           // Nombre de red
#define password "3C046100F16E"    // Contraseña de red

//Datos de MQTT
//#define mqtt_server "192.168.0.16" // IP de mi broker local
#define mqtt_server "3.122.36.163" 
#define mqtt_port 1883           // Puerto de conexión
//#define clientID "ESP32CAMClient"  // ID del cliente
#define clientID "rome810407"
//#define topic "esp32/data"         // Tema para publicar/subscribirse
#define topic "codigoiot/distancia/rome810407"

// Objetos
//IPAddress server(3,122,36,163);
WiFiClient espClient;           // Este objeto maneja los datos de conexion WiFi
PubSubClient client(espClient); // Este objeto maneja los datos de conexion al broker
Ultrasonic ultrasonic(Trig, Echo); // Este objeto maneja la lecutra del sensor HC-SR04

// Variables Globales
long timeNow, timeLast; // Variables de control de tiempo no bloqueante
int distance = 0, marca_tiempo = 0, segundos = 0;

// Funciones
void start_wifi();  //Funcion encargada de conectarse al WiFi
void enviar_datos_mqtt(); //Funcion que envia la suma y la cadena con el valor binario de la secuencia de leds
void conectar_Broker();  //Funcion que se conecta al broker

// Inicialización del programa
void setup() {
  // Configuración de pines
  pinMode(LED_STATUS, OUTPUT);  // Se configura GPIO33 como salida
  // Conexión a WiFi
  start_wifi();
  // Conexuón con el broker MQTT
  client.setServer(mqtt_server, mqtt_port); // Define el servidor MQTT
  conectar_Broker(); // Función que establece la conexión al servidor MQTT
  timeLast = millis(); // Inicia el control de tiempo
}// fin del void setup()

void loop() {
    timeNow = millis ();  // Seguimiento de tiempo
    if (timeNow - timeLast > 1000) {  //Comprueba que ya paso un segundo
      marca_tiempo++;
      //Contar los segundos
      segundos++; //Aumenta el contador de los segundos
      if(marca_tiempo%5==0){
        printf("Segundos: %i\n", segundos);
        distance = ultrasonic.read();
        printf("Distance in CM: %i\n", distance);
        enviar_datos_mqtt();  //Envia los datos al mqtt
      }
      timeLast = millis (); // Inicia el control de tiempo
    }
}

void start_wifi(){
  // Se imprimen mensajes descriptivos en la consola serial
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.print("Conectandose a ");
  Serial.print(ssid);
  // Inicia la conexión WiFi
  WiFi.begin(ssid, password); // Esta es la función que realiza la conexión a WiFi
  while (WiFi.status() != WL_CONNECTED) { // Este bucle espera a que se realice la conexión
    digitalWrite (LED_STATUS, HIGH);
    delay(500); //dado que es de suma importancia esperar a la conexión, debe usarse espera bloqueante
    digitalWrite (LED_STATUS, LOW);
    Serial.print(".");  // Indicador de progreso
    delay (5);
  }  
}

void enviar_datos_mqtt(){
  if (client.connected()){
    char payload[25];
    //String str = "Suma: "+ String(marca_tiempo)+ "\nDist: "+ String(distance);
    String str = String(distance);
    str.toCharArray(payload, 24);
    client.publish(topic, payload);
  }else{
    conectar_Broker(); //Se conecta a al Broker
    enviar_datos_mqtt();  //Envia los datos al mqtt
  }
}

void conectar_Broker() {
  while (!client.connected()) { // Pregunta si hay conexión
    Serial.print("Intentando conexión Mqtt...");
    //Intentamos conectar
    if (client.connect(clientID)) {
      Serial.println("¡Conectado!");
      // Nos suscribimos
      if(client.subscribe(topic)){
        Serial.println("Suscripcion ok");
      }else{
        Serial.println("fallo Suscripciión");
      }
    }else {
      Serial.print("falló :( con error -> ");
      Serial.print(client.state());
      Serial.println(" Intentamos de nuevo en 5 segundos");
      delay(5000);
    }
  }
}
