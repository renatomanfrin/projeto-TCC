# 🌱 Sistema de Controle e Temperatura

![Status](https://img.shields.io/badge/status-em%20desenvolvimento-yellow)
![Plataforma](https://img.shields.io/badge/platform-ESP32-blue)
![Linguagem](https://img.shields.io/badge/language-C++-brightgreen)

## 📌 Descrição

Este projeto implementa um **sistema de controle e temperatura** utilizando um **ESP32**, sensores ambientais e controle via nuvem (TagoIO).

O sistema monitora:
- 🌡️ Temperatura
- 💧 Umidade do ar
- 🌱 Umidade do solo  

E realiza o controle de irrigação utilizando um **controlador PID**, permitindo ajuste dinâmico da umidade do solo.

---

## ⚙️ Funcionalidades

✅ Leitura de sensor DHT11 (temperatura e umidade)  
✅ Leitura de sensor de umidade do solo  
✅ Controle de válvula de irrigação via PWM  
✅ Implementação de controle PID  
✅ Comunicação com a nuvem (TagoIO)  
✅ Atualização de setpoint remotamente  
✅ Envio de dados em tempo real  

---

## 🧠 Arquitetura do Sistema

```mermaid
graph TD
A[Sensor DHT11] --> ESP32
B[Sensor Umidade Solo] --> ESP32
ESP32 --> C[Controle PID]
ESP32 --> G[Controle On/Off]
G --> H[Bomba  de Irrigação]
C --> D[Lâmpada Incadescente (atuador)]
ESP32 --> E[WiFi]
E --> F[TagoIO Cloud]
```

---

## 🔌 Hardware Utilizado

| Componente           | Descrição                  |
|---------------------|---------------------------|
| ESP32               | Microcontrolador principal |
| DHT11               | Sensor de temperatura/umidade do ar |
| Sensor de Solo      | Leitura de umidade do solo |
| Bomba de Água       | Controle de umidade do solo|
| Lâmpada Incadescente| Controle de temperatura do ar |

---

## 📍 Pinagem

```cpp
#define pCONTROLE 14
#define DHTPIN 27
#define pinosensor 34
#define pinovalvula 25
```

---

## 🌐 Conectividade

O sistema utiliza Wi-Fi para comunicação com a plataforma TagoIO:

```cpp
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
```

---

## ☁️ Integração com TagoIO

### 📥 Leitura de dados da nuvem
- Permite atualizar variáveis remotamente (ex: setpoint)

### 📤 Envio de dados
- SetPoints
- Temperatura do ar
- Umidade do ar
- Umidade do solo
- Sinal de controle
- Índice de Calor
- Tempo

---

## 🎯 Controle PID

O sistema implementa um **PID incremental discreto**:

### Parâmetros:
- `kP` → Proporcional  
- `kI` → Integral  
- `kD` → Derivativo  

### Equação aplicada:

```text
u[k] = u[k-1] + a0*e[k] + a1*e[k-1] + a2*e[k-2]
```

### Configuração atual:

```cpp
PID meuPid(20, 0.1, 5);
```

---

## 📊 Variáveis Importantes

| Variável        | Descrição |
|----------------|----------|
| `setPoint`     | SetPoint de temperatura |
| `umidadesolo`  | Umidade do solo atual |
| `t`            | Temperatura atual |
| `u_k`          | Saída do controlador |
| `Ts`           | Tempo de amostragem |

---

## 🔄 Fluxo de Execução

1. Conecta ao Wi-Fi 📡  
2. Lê sensores 🌡️🌱  
3. Atualiza dados da nuvem ☁️  
4. Calcula PID 🧠  
4. Ajusta lâmpada incadescente 💡  
5. Aciona bomba de água 🚿  
6. Envia dados ao TagoIO 📤  
7. Repete ciclo ⏱️  

---

## ⚠️ Observações

- O sensor de solo pode precisar de calibração
- O DHT11 possui baixa precisão (pode ser substituído por DHT22)
- O PID pode precisar de ajuste fino dependendo do ambiente
- A comunicação HTTP pode introduzir latência

---

## 🛠️ Como Usar

1. Configure Wi-Fi e Token do TagoIO  
2. Faça upload para o ESP32  
3. Configure dashboard no TagoIO  
4. Monitore e ajuste o sistema remotamente  

---

## 🔐 Segurança

⚠️ **IMPORTANTE**  
Modifique a configuração de Rede e Integração com API:
- Token do TagoIO  
- Nome e Senha do Wi-Fi  

---

## 👨‍💻 Autor

Desenvolvido por Renato Manfrin Benedicto para Trabalho de Conclusão de Curso em bacharelado em Engenharia Elétrica na Universidade Federal de São Carlos.

---
