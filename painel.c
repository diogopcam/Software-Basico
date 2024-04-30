#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

int main() {
  // Inicialização do ncurses
  initscr();
  cbreak();
  curs_set(0);

  // Definição do tamanho do painel
  int painel_largura = 40;
  int painel_altura = 10;

  // Aloca espaço na memória para o painel
  char painel[painel_altura][painel_largura];

  // Preenchimento do painel com espaços em branco
  for (int linha = 0; linha < painel_altura; linha++) {
    for (int coluna = 0; coluna < painel_largura; coluna++) {
      painel[linha][coluna] = ' ';
    }
  }

  // Mensagem a ser exibida
  char mensagem[] = "Olá, Mundo!";

  // Posição inicial da mensagem
  int pos_x = 0;
  int pos_y = 0;

  // Loop principal
  while (1) {
    // Limpa o painel
    for (int linha = 0; linha < painel_altura; linha++) {
      for (int coluna = 0; coluna < painel_largura; coluna++) {
        painel[linha][coluna] = ' ';
      }
    }

    // Exibe a mensagem
    int i = 0;
    while (mensagem[i] != '\0') {
      int linha = pos_y + i / painel_largura;
      int coluna = (pos_x + i) % painel_largura; // Deslocamento horizontal

      if (linha >= 0 && linha < painel_altura && coluna >= 0 && coluna < painel_largura) {
        painel[linha][coluna] = mensagem[i];
      }
      i++;
    }

    // Exibe o painel
    for (int linha = 0; linha < painel_altura; linha++) {
      mvprintw(linha, 0, painel[linha]);
    }
    refresh();

    // Animação simples (atraso entre caracteres)
    usleep(50000); // Ajuste o tempo de atraso (microsegundos)

    // Deslocamento horizontal (efeito de rolagem)
    pos_x--;
    if (pos_x < -strlen(mensagem)) {
      pos_x = painel_largura;
    }
  }

  // Finalização do ncurses
  endwin();
  return 0;
}