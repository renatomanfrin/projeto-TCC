#include "DHT.h"
#include <HTTPClient.h>
#include <WiFi.h>

char serverAddress[] = "https://api.tago.io/data";  // TagoIO address
char contentHeader[] = "application/json";
char tokenHeader[] = "Token_TagoIO";  // TagoIO Token
const char* ssid = "Nome_Rede";
const char* password = "Senha";

WiFiClient wifia;
// HttpClient client = HttpClient(wifia, serverAddress, port);
HTTPClient client;
int status = WL_IDLE_STATUS;

//Definição pinos
#define pCONTROLE 14   //Pino para fazer o controle
#define DHTPIN 27      // Pino conectado no DHT
#define pinosensor 34  //sensor de solo
#define pinovalvula 25
#define DHTTYPE DHT11  // Tipo de DHT
DHT dht(DHTPIN, DHTTYPE);


//Declaração de Variáveis
int freq = 60;
int resolution = 8;
int controlePwm = 50;
float umidadesolo = 0;
double setPoint = 35;
float porc_umidade = 30;
float t_anterior = 25;
float h_anterior;
float e_k = 0;   // Erro atual: e[k]
float e_k1 = 0;  // Erro anterior: e[k-1]
float e_k2 = 0;  // Erro dois passos atrás: e[k-2]
float u_k = 0;   // Saída atual do controlador: u[k]
float u_k1 = 0;  // Saída anterior do controlador: u[k-1]
double kP, kI, kD;
float a0;
float a1;
float a2;
float h, t, hic;
int a = 0;
int aux = 0;
unsigned long tempoReferencia15s = 0;
unsigned long referencia5s = 0;
int passo = 0;

//Funções
//Calculo do sinal de controle
class PID {
public:
  double sinal;
  double Ts = 5;  // Período de amostragem (em segundos)
  double pid;
  double setPoint;
  long tempo_antigo = 0;
  PID(double _kP, double _kI, double _kD) {
    kP = _kP;
    kI = _kI;
    kD = _kD;
  }

  void addNewSample(double _sinal) {  // cria um novo elemento na lista para guardar dados
    sinal = _sinal;
  }
  void setSetPoint(double _setPoint) {  // cria um novo elemento na lista para guardar dados
    setPoint = _setPoint;
  }
  double process() {  //Implementação PID

    //coeficientes dos erros
    a0 = kP + (kI * Ts / 2.0) + (2.0 * kD / Ts);
    a1 = -kP + (kI * Ts / 2.0) - (4.0 * kD / Ts);
    a2 = (2.0 * kD) / Ts;

    //Calcula erro atual
    e_k = setPoint - sinal;

    //Atualiza PID incrementalmente
    u_k = u_k1 + a0 * e_k + a1 * e_k1 + a2 * e_k2;

    //Atualiza histórico
    e_k2 = e_k1;
    e_k1 = e_k;
    u_k1 = u_k;

    if (u_k > pow(2, resolution) - 1) {
      u_k = pow(2, resolution) - 1;
    }

    if (u_k < -pow(2, resolution) + 1) {
      u_k = -pow(2, resolution) + 1;
    }

    return u_k;
NOVO ESBOÇO

  }
};

//Adquirir o valor da variável pelo TagoIO
float getDataFromTagoIO(const char* variableName, float fallbackValue) {
  float valueFromCloud = fallbackValue;
  char url[150];

  snprintf(url, sizeof(url), "https://api.tago.io/data?variable=%s&qty=1", variableName);
  client.begin(url);
  client.addHeader("Device-Token", tokenHeader);

  int httpCode = client.GET();

  if (httpCode == 200) {
    String payload = client.getString();
    int valueStart = payload.indexOf("\"value\":\"") + 9;
    int valueEnd = payload.indexOf("\"", valueStart);
    if (valueStart > 0 && valueEnd > valueStart) {
      valueFromCloud = payload.substring(valueStart, valueEnd).toFloat();
    }
  } else {
    Serial.printf("Erro ao buscar %s: %d\n", variableName, httpCode);
  }

  client.end();
  return valueFromCloud;
}

//Postar no TagoIO
void postToTagoIO(const char* variableName, float value, const char* unit) {
  char postData[200];
  int statusCode;

  snprintf(postData, sizeof(postData), "{\n\t\"variable\": \"%s\",\n\t\"value\": %.2f,\n\t\"unit\": \"%s\"\n}", variableName, value, unit);

  Serial.printf("Enviando [%s]: %s\n", variableName, postData);

  client.begin(serverAddress);
  client.addHeader("Content-Type", contentHeader);
  client.addHeader("Device-Token", tokenHeader);
  statusCode = client.POST((uint8_t*)postData, strlen(postData));

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.println("End of POST to TagoIO\n");

  client.end();
}

PID meuPid(20, 0.1, 5);

//==================================================SETUP===================================================
void setup() {
  Serial.begin(115200);
  delay(10);

  WiFi.mode(WIFI_STA);
  if (ssid != "")
    WiFi.begin(ssid, password);
  WiFi.begin();
  Serial.println("");

  // Esperar até conexão
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Setando PinOut
  pinMode(DHTPIN, INPUT);
  pinMode(pCONTROLE, OUTPUT);
  pinMode(pinovalvula, OUTPUT);
  ledcAttach(pCONTROLE, freq, resolution);
  digitalWrite(pinovalvula, HIGH);

  Serial.println("");
  Serial.print("Conectado a ");
  Serial.println(ssid);
  Serial.print("Endereco de IP: ");
  Serial.println(WiFi.localIP());
  Serial.println(F("DHT11 test!"));

  dht.begin();
}


//=====================================================LOOP=====================================================
void loop() {
  unsigned long agora = millis();
  if (agora - referencia5s >= 5000) {
    referencia5s = agora;

    //Pegar valores do TagoIO
    porc_umidade = getDataFromTagoIO("porc_umidade", porc_umidade);
    setPoint = getDataFromTagoIO("setpoint", setPoint);
    kP = getDataFromTagoIO("kp", kP);
    kI = getDataFromTagoIO("ki", kI);
    kD = getDataFromTagoIO("kd", kD);

    // Leitura de umidade do ar
    h = dht.readHumidity();
    // Leitura de temperatura do ar
    t = dht.readTemperature();
    // Leitura de índice de calor do ar (sensação térmica)
    hic = dht.computeHeatIndex(t, h, false);
    //Leitura de umidade do solo
    umidadesolo = analogRead(pinosensor);
    //converter a variação do sensor de 3050 a 1400 bits para 0 a 100%
    umidadesolo = map(umidadesolo, 3050, 1400, 0, 100);

    //Condições para não fazer medidas erradas
    if (isnan(h) || isnan(t)) {
      t = t_anterior;
      h = h_anterior;
    }

    if (t < 0) {
      t = t_anterior;
    }

    if (t - t_anterior < -5) {
      t = t_anterior;
    }

    if (umidadesolo < 0) {
      umidadesolo = 0;
    }
    if (umidadesolo > 100) {
      umidadesolo = 100;
    }

    if (a == 12) {  //em 5s do contador faz verificação de necessidade de umidificar solo
      if (umidadesolo < porc_umidade) {
        digitalWrite(pinovalvula, LOW);
        aux = 1;
      }
    } else {
      digitalWrite(pinovalvula, HIGH);
      aux = 0;
    }

    // Lê temperatura de setpoint
    meuPid.setSetPoint(setPoint);

    // Manda pro objeto PID!
    meuPid.addNewSample(t);
    // Converte para controle
    controlePwm = (meuPid.process());

    if (controlePwm < 0) {
      controlePwm = 0;
    }

    // Saída do controle
    ledcWrite(pCONTROLE, controlePwm);

    //Printar os dados lidos
    Serial.print("SetPoint Temperatura: ");
    Serial.println(setPoint);
    Serial.print("SetPoint Umidade: ");
    Serial.println(porc_umidade);
    Serial.print("Temperatura Ar: ");
    Serial.println(t);
    Serial.print("Umidade Ar: ");
    Serial.println(h);
    Serial.print("Índice de Calor: ");
    Serial.println(hic);
    Serial.print("Ação Controle Temperatura:");
    Serial.println(controlePwm);
    Serial.print("Umidade do Solo: ");
    Serial.println(round(umidadesolo), 2);
    Serial.print("Ação Controle Umidade: ");
    Serial.println(aux);
    Serial.print("Contador: ");
    Serial.println(a);
    Serial.print("Tempo: ");
    Serial.println(millis() / 1000);

    //Contador de tempo
    a++;
    if (a > 12) {
      a = 0;
    }

    //Setando os dados atuais como anteriores
    t_anterior = t;
    h_anterior = h;
  }

  //Posta para o TagoIO uma varaivel a cada 15s
  if (agora - tempoReferencia15s >= 15000) {
    tempoReferencia15s = agora;

    if (passo == 0) {
      postToTagoIO("Humidity", h, "%");
    } else if (passo == 1) {
      postToTagoIO("Temperature", t, "C");
    } else if (passo == 2) {
      postToTagoIO("HIndex", hic, "C");
    } else if (passo == 3) {
      postToTagoIO("UmidadeSolo", umidadesolo, "%");
    } else if (passo == 4) {
      postToTagoIO("SetPoint", setPoint, "C");
    } else if (passo == 5) {
      postToTagoIO("kp", kP, "");
    } else if (passo == 6) {
      postToTagoIO("ki", kI, "");
    } else if (passo == 7) {
      postToTagoIO("kd", kD, "");
    }

    passo++;
    if (passo > 7) {
      passo = 0;  // reinicia a sequência
    }
  }
}