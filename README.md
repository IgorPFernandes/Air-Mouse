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
- **Gestão de energia em três estados** — ativo, ocioso com a conexão mantida e
  sono profundo. Em uso realista, ~20 dias com uma LiPo de 1000 mAh e ~133 dias
  com duas 18650 em paralelo; ver [ENERGIA.md](docs/ENERGIA.md).
- **Despertar por movimento**: pegar o aparelho já o acorda, sem apertar nada.
- **Botão de centralizar**, para reencontrar o cursor quando se perde de vista.
- **Três níveis de velocidade** com confirmação por LED RGB.

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
│   ├── recenter.{h,cpp}      centralizacao do cursor
│   ├── battery.{h,cpp}       leitura do ADC e curva da célula
│   ├── rgb_led.{h,cpp}       LED RGB: status e velocidade
│   └── power.{h,cpp}         clock, light sleep e sono profundo
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
| [ENERGIA.md](docs/ENERGIA.md) | Orçamento de consumo e autonomia |

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
| Botão centralizar | Traz o cursor para perto do centro da tela |
| Botão velocidade | Percorre lento → médio → rápido |
| 4 s parado | Estado ocioso, conexão mantida |
| 20 min parado | Sono profundo |
| Mover o aparelho, ou botão esquerdo | Desperta |

O *bonding* está habilitado: depois do primeiro pareamento o host reconecta
sozinho.

| LED RGB | Estado |
|---|---|
| Azul fixo | Iniciando |
| Azul, piscada lenta | Anunciando, aguardando conexão |
| Vermelho, piscada rápida | MPU6050 não respondeu |
| Apagado | Conectado e em uso |
| Verde / amarelo / vermelho por 1,5 s | Velocidade rápida / média / lenta |

O LED não fica aceso mostrando a velocidade o tempo todo: um LED aceso consome
mais que o aparelho inteiro em repouso. A cor aparece na troca e ao conectar.

---

## Estado do projeto

O firmware está completo e documentado, mas **nunca foi compilado nem executado
em hardware**. A primeira compilação é o próximo passo.

Esta tabela existe para quem for montar: ela diz onde procurar primeiro quando
algo não funcionar.

| Subsistema | Estado | Risco |
|---|---|---|
| Driver do MPU6050 | Escrito | Baixo — registradores padrão, amplamente documentados |
| Pipeline do cursor | Escrito, matemática simulada | Baixo — a simulação confere as contas |
| HID BLE | Escrito, API conferida nos headers do NimBLE 1.4.3 | Médio — descritor e serviços não exercitados |
| Botões e debounce | Escrito | Baixo |
| Centralização do cursor | Escrito | Médio — depende da aceleração de ponteiro do sistema |
| Níveis de velocidade e RGB | Escrito | Baixo |
| Slave latency e parâmetros de conexão | Escrito | Médio — negociação pode ser recusada pelo host |
| Light sleep automático | Escrito, com detecção de ausência de suporte | Médio — depende do core ter power management |
| **Wake-on-motion do MPU6050** | **Escrito a partir da documentação** | **Alto — sequência de registradores não verificada** |
| Consumo e autonomia | Estimado por folha de dados | **Alto — nenhuma medição feita** |

Se algo falhar na bancada, comece pelas duas últimas linhas.

Histórico completo do que foi construído em [CHANGELOG.md](CHANGELOG.md).

Relatos de erro de compilação ou de comportamento são bem-vindos via issue.

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
