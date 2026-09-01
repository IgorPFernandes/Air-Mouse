# Air Mouse

Mouse controlado por movimento da mão, feito com ESP32-C3 e MPU6050.

O aparelho se apresenta ao computador como um **mouse Bluetooth LE padrão (HID)**:
sem driver, sem dongle, sem software do lado do host. Funciona com Windows,
macOS, Linux, Android e iPadOS.

---

## Características

- **HID BLE nativo**, implementado direto sobre o NimBLE — sem biblioteca HID de
  terceiros.
- **Compensação de rotação do pulso** por filtro complementar: os eixos da tela
  continuam corretos mesmo com o aparelho inclinado de lado.
- **Curva de aceleração** com termo linear e quadrático: precisão no movimento
  fino, alcance no movimento rápido.
- **Acumulador de sub-pixel**, para que gestos lentos não sejam truncados a zero.
- **Calibração validada no boot** — a medição é rejeitada se o aparelho estiver
  sendo movido.
- **Modo scroll** com o botão do meio segurado.
- **Nível de bateria** reportado ao sistema operacional.
- **Sono profundo** após inatividade, com despertar por botão.

---

## Estrutura

```
├── include/
│   └── config.h              parâmetros de hardware e ajuste
├── src/
│   ├── main.cpp              inicialização e laço principal
│   ├── mpu6050.{h,cpp}       driver do sensor e calibração
│   ├── pointer.{h,cpp}       tratamento de sinal do cursor
│   ├── ble_mouse.{h,cpp}     perfil HID sobre BLE
│   ├── button.{h,cpp}        debounce
│   ├── battery.{h,cpp}       leitura do ADC e curva da célula
│   ├── status_led.{h,cpp}    estado visual
│   └── power.{h,cpp}         sono profundo
├── tools/
│   └── i2c_scanner/          diagnóstico do barramento I²C
└── docs/
```

---

## Documentação

| Documento | Conteúdo |
|---|---|
| [HARDWARE.md](docs/HARDWARE.md) | Lista de materiais, pinagem, alimentação e carga |
| [MONTAGEM.md](docs/MONTAGEM.md) | Montagem em etapas, com teste a cada uma |
| [ARQUITETURA.md](docs/ARQUITETURA.md) | Como o firmware funciona e por quê |
| [AJUSTES.md](docs/AJUSTES.md) | Ajuste de sensibilidade e diagnóstico |

---

## Início rápido

Ligações completas em [HARDWARE.md](docs/HARDWARE.md). O mínimo para funcionar:

| MPU6050 | ESP32-C3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO 5 |
| SCL | GPIO 6 |

### PlatformIO

```bash
pio run --target upload
```

### Arduino IDE

1. Instale o core **esp32 by Espressif**, versão 2.0.x.
2. Instale **NimBLE-Arduino** versão **1.4.x** (a 2.x mudou a API).
3. Copie `src/` e `include/config.h` para a pasta do sketch, renomeando
   `main.cpp` para o nome da pasta com extensão `.ino`.
4. Selecione **ESP32C3 Dev Module** com `USB CDC On Boot: Enabled`.

Se o upload falhar na SuperMini: segure **BOOT**, toque em **RESET**, solte o
**BOOT** e repita.

---

## Uso

1. Ligue e **deixe parado por cerca de 2 segundos** — o giroscópio é calibrado no
   boot. Movimento durante a calibração faz o cursor derivar depois.
2. Parear em *Bluetooth → Adicionar dispositivo → Air Mouse*.
3. LED apagado indica conexão estabelecida.

| Ação | Resultado |
|---|---|
| Girar a mão na horizontal | Move o cursor em X |
| Inclinar para cima e para baixo | Move o cursor em Y |
| Botão esquerdo | Clique esquerdo |
| Botão direito | Clique direito |
| Botão do meio segurado + movimento | Rolagem |
| 5 min de inatividade | Sono profundo |
| Botão esquerdo | Desperta |

O *bonding* está habilitado: depois do primeiro pareamento o host reconecta
sozinho.

| LED | Estado |
|---|---|
| Aceso | Iniciando |
| Piscada lenta | Anunciando, aguardando conexão |
| Apagado | Conectado |
| Piscada rápida | MPU6050 não respondeu |

---

## Estado do projeto

O firmware está completo, mas **ainda não foi compilado nem validado em
hardware**. A primeira compilação é o próximo passo. Relatos de erro de
compilação ou de comportamento são bem-vindos via issue.

---

## Referências

O firmware foi escrito do zero, sem reaproveitamento de código. Estes projetos
foram consultados durante o desenvolvimento e valem a leitura:

| Projeto | Comentário |
|---|---|
| [k8r00/comet-air-mouse](https://github.com/k8r00/comet-air-mouse) | O mais próximo deste hardware (ESP32-C3 + MPU6050). A ideia do botão segurado como scroll veio dele. |
| [n1rml/esp32_airmouse](https://github.com/n1rml/esp32_airmouse) | Implementação em ESP-IDF, para o ESP32 clássico. |
| [WhoIsMrSentry/Esp32GestureMouse](https://github.com/WhoIsMrSentry/Esp32GestureMouse) | Boa referência de tratamento de botões. |
| [T-vK/ESP32-BLE-Mouse](https://github.com/T-vK/ESP32-BLE-Mouse) | A biblioteca clássica — **não funciona no C3**, depende de Bluetooth Classic ([issue #69](https://github.com/T-vK/ESP32-BLE-Mouse/issues/69)). |

Sobre a escolha de implementar o HID direto sobre o NimBLE em vez de usar uma
dessas bibliotecas, ver [ARQUITETURA.md](docs/ARQUITETURA.md).

---

## Licença

MIT — ver [LICENSE](LICENSE).
