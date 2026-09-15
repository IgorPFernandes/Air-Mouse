#pragma once

#include <stdint.h>

// Leva o cursor ao centro da tela.
//
// Um mouse HID reporta deslocamento relativo, nunca posicao absoluta: nao ha
// como dizer "va para 960, 540". A saida e encostar o cursor no canto superior
// esquerdo, onde o sistema operacional o prende, e de la caminhar meia tela.
//
// A primeira metade e exata, porque o cursor fica presa no canto por mais que
// se empurre. A segunda nao: a aceleracao de ponteiro do sistema multiplica o
// deslocamento conforme a velocidade, entao a chegada e proxima do centro, nao
// exata. Para achar o cursor perdido, proxima basta.
namespace Recenter {

// Inicia a sequencia. Enquanto ela roda, o movimento do giroscopio e ignorado.
void start();

bool running();

void cancel();

// Chamar uma vez por quadro enquanto running(). Emite um relatorio por
// chamada e devolve false quando termina.
bool update();

}  // namespace Recenter
