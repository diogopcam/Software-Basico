#include <ncurses.h>
#include <unistd.h>

void painel(const char *texto) {
    // Inicializa a biblioteca NCurses
    initscr();
    // Esconde o cursor
    curs_set(0);

    // Obtém o tamanho da tela
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Posição inicial da string
    int x = 0;
    int y = max_y / 2;

    // Loop infinito para animação
    while (1) {
        // Limpa a tela
        clear();

        // Imprime a string na posição atual
        mvprintw(y, x, texto);
        
        // Atualiza a tela
        refresh();

        // Move a string para a direita
        x++;

        // Se a string sair da tela, volta para a esquerda
        if (x > max_x) {
            x = 0;
        }

        // Aguarda um curto período de tempo para controlar a velocidade da animação
        usleep(100000); // 100ms
    }

    // Finaliza a biblioteca NCurses
    endwin();
}

int main() {
    // Chama a função painel() passando a string desejada como parâmetro
    //painel("Radiohead");

    return 0;
}