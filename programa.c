@@ -5,9 +5,6 @@
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>

#define FILE_PATH "registers.bin"
#define FILE_SIZE 1024  // Tamanho do arquivo de registros
#define LED_DISPLAY_REGISTERS 8
	@@ -16,21 +13,16 @@
#define TEMPERATURE_SENSOR_REGISTER 11
#define BATTERY_REGISTER 12


// Definindo enumeração para cores
enum Colors {
    RED,
    GREEN,
    BLUE
};

// Corrigindo a máscara para o componente vermelho
#define RED_MASK   0xFF00

// Corrigindo a máscara para o componente verde
#define GREEN_MASK 0xFF0000

#define BLUE_MASK   0xFF

int fd = -1; // Descritor de arquivo global

	@@ -76,110 +68,50 @@ int registers_release(void* map, int file_size) {
    return 0;
}

// Função para definir o valor do componente vermelho (R)
void set_valor_R(char* base_address, int red_value) {
    unsigned short control_register_value = *((unsigned short *)(base_address + (2 * sizeof(unsigned short))));

    // Se red_value for 1, liga o bit 10 do registrador de controle, caso contrário, desliga
    if (red_value == 1) {
        control_register_value |= (1 << 10); // Bit 10: Controla o componente vermelho (R)
    } else {
        control_register_value &= ~(1 << 10);
    }

    *((unsigned short *)(base_address + (2 * sizeof(unsigned short)))) = control_register_value;
}

// Função para definir o valor do componente verde (G)
void set_valor_G(char* base_address, int green_value) {
    unsigned short control_register_value = *((unsigned short *)(base_address + (2 * sizeof(unsigned short))));

    // Se green_value for 1, liga o bit 11 do registrador de controle, caso contrário, desliga
    if (green_value == 1) {
        control_register_value |= (1 << 11); // Bit 11: Controla o componente verde (G)
    } else {
        control_register_value &= ~(1 << 11);
    }

    *((unsigned short *)(base_address + (2 * sizeof(unsigned short)))) = control_register_value;
}

// Função para definir o valor do componente azul (B)
void set_valor_B(char* base_address, int blue_value) {
    unsigned short control_register_value = *((unsigned short *)(base_address + (2 * sizeof(unsigned short))));

    // Se blue_value for 1, liga o bit 12 do registrador de controle, caso contrário, desliga
    if (blue_value == 1) {
        control_register_value |= (1 << 12); // Bit 12: Controla o componente azul (B)
    } else {
        control_register_value &= ~(1 << 12);
    }

    *((unsigned short *)(base_address + (2 * sizeof(unsigned short)))) = control_register_value;
}

void set_intensity_R(char* base_address, int intensity) {
    // Garante que a intensidade esteja dentro do intervalo válido (0-255)
    if (intensity < 0 || intensity > 255) {
        fprintf(stderr, "Erro: Intensidade do componente R fora do intervalo válido (0-255)\n");
        return;
    }

    // Lê o valor atual do registrador R1
    unsigned short value = *((unsigned short *)(base_address + (1 * sizeof(unsigned short))));

    // Limpa os bits correspondentes ao componente vermelho
    value &= ~RED_MASK;

    // Define a intensidade do componente vermelho
    value |= (intensity & 0xFF); // Aplica a máscara corretamente

    // Escreve o novo valor no registrador R1
    *((unsigned short *)(base_address + (1 * sizeof(unsigned short)))) = value;
}
void set_intensity_G(char* base_address, int intensity) {
    // Garante que a intensidade esteja dentro do intervalo válido (0-255)
    if (intensity < 0 || intensity > 255) {
        fprintf(stderr, "Erro: Intensidade do componente G fora do intervalo válido (0-255)\n");
        return;
    }

    // Lê o valor atual do registrador R1
    unsigned short value = *((unsigned short *)(base_address + (1 * sizeof(unsigned short))));

    // Limpa os bits correspondentes ao componente verde
    value &= ~GREEN_MASK;

    // Define a intensidade do componente verde
    value |= (intensity & 0xFF0000); // Aplica a máscara corretamente

    // Escreve o novo valor no registrador R1
    *((unsigned short *)(base_address + (1 * sizeof(unsigned short)))) = value;
}


// Corrigindo a aplicação da máscara no método set_intensity_B
void set_intensity_B(char* base_address, int intensity) {
    // Garante que a intensidade esteja dentro do intervalo válido (0-255)
    if (intensity < 0 || intensity > 255) {
        fprintf(stderr, "Erro: Intensidade do componente B fora do intervalo válido (0-255)\n");
        return;
    }

    // Limpa todos os bits do valor atual do registrador R2
    //unsigned short value = 0;

    // Lê o valor atual do registrador R2
    unsigned short value = *((unsigned short *)(base_address + (2 * sizeof(unsigned short))));

    // Limpa os bits correspondentes ao componente azul
    value &= ~BLUE_MASK;

    // Define a intensidade do componente azul
    value |= (intensity & 0xFF); // <--- Aqui está a correção

    // Escreve o novo valor no registrador R2
    *((unsigned short *)(base_address + (2 * sizeof(unsigned short)))) = value;
}

void print_message_with_color_and_rgb(const char* message, char* base_address) {
	@@ -192,21 +124,12 @@ void print_message_with_color_and_rgb(const char* message, char* base_address) {
        return;
    }

    // Mapeia a mensagem nos registradores de dados (R4 a R15)
    int i;
    for (i = 0; i < strlen(message) && (i + 4) < 16; i++) {
        *((unsigned short *)(base_address + ((i + 4) * sizeof(unsigned short)))) = message[i];
    }

    // Preenche os registradores de dados restantes com espaços em branco ou caracteres nulos
    for (; i < 12; i++) {
        *((unsigned short *)(base_address + ((i + 4) * sizeof(unsigned short)))) = ' ';
    }

    // Calcula o valor RGB com base nos bits de controle
    int red_on = (control_register_value >> 10) & 0x01;
    int green_on = (control_register_value >> 11) & 0x01;
    int blue_on = (control_register_value >> 12) & 0x01;
    unsigned int color = 0;
    if (red_on) {
        color |= 0xFF0000; // Define o componente vermelho como máximo
	@@ -219,198 +142,30 @@ void print_message_with_color_and_rgb(const char* message, char* base_address) {
    }

    // Imprime a mensagem com a cor especificada
    printf("\x1b[38;2;%d;%d;%dm", (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    for (i = 0; i < strlen(message) && (i + 4) < 16; i++) {
        printf("%c", message[i]);
    }
    printf("\x1b[0m\n");
}

// Função para configurar a mensagem no display de LED
void configure_text_display(char* base_address, const char* message) {
    // Calcula o comprimento da mensagem
    int message_length = strlen(message);

    // Verifica se a mensagem excede o limite de 24 caracteres do display
    if (message_length > LED_DISPLAY_REGISTERS) {
        fprintf(stderr, "Erro: mensagem excede o limite de caracteres do display\n");
        return;
    }

    // Escreve a mensagem nos registradores de dados (R4 a R11)
    int i;
    for (i = 0; i < message_length && (i + 3) < LED_DISPLAY_REGISTERS; i++) {
        *((unsigned short *)(base_address + ((i + 3) * sizeof(unsigned short)))) = message[i];
    }

    // Preenche os registradores de dados restantes com espaços em branco
    for (; i < LED_DISPLAY_REGISTERS - 3; i++) {
        *((unsigned short *)(base_address + ((i + 3) * sizeof(unsigned short)))) = ' ';
    }
}

// Função para exibir o menu principal
void exibir_menu_principal() {
    printf("Seja bem vindo ao sistema de LED.\n");
    printf("Digite 1 para manipular as cores\n");
    printf("Digite 0 para sair do programa\n");
}

// Função para exibir o menu de manipulação das cores
void exibir_menu_cores() {
    printf("\nEscolha qual componente você deseja manipular:\n");
    printf("R - 1\n");
    printf("G - 2\n");
    printf("B - 3\n");
}

// Função para exibir o menu de ajuste de intensidade
void exibir_menu_intensidade(char componente) {
    printf("\nAjuste a intensidade do componente %c (0-255), ou digite 0 para voltar ao menu principal: ", componente);
}

// Função para imprimir as intensidades de cada componente
void print_component_intensities(char* base_address) {
    // Lê o valor atual do registrador R1 (componentes R e G)
    unsigned short r1_value = *((unsigned short *)(base_address + (1 * sizeof(unsigned short))));

    // Lê o valor atual do registrador R2 (componente B)
    unsigned short r2_value = *((unsigned short *)(base_address + (2 * sizeof(unsigned short))));

    // Extrai a intensidade do componente vermelho
    int intensity_R = r1_value & RED_MASK;

    // Extrai a intensidade do componente verde (deslocando 8 bits para a direita)
    int intensity_G = (r1_value & GREEN_MASK) >> 8;

    // Extrai a intensidade do componente azul
    int intensity_B = r2_value & BLUE_MASK;

    // Imprime as intensidades de cada componente
    printf("Intensidade do componente vermelho: %d\n", intensity_R);
    printf("Intensidade do componente verde: %d\n", intensity_G);
    printf("Intensidade do componente azul: %d\n", intensity_B);
}

void painel(const char *texto) {
    // Inicializa a biblioteca NCurses
    initscr();
    // Esconde o cursor
    curs_set(0);

    int linhas = 15;
    int colunas = 24;
    // Cria a janela
    WINDOW *win = newwin(linhas, colunas, 0, 0);

    // Obtém o tamanho da tela
    //int max_y, max_x;
    //getmaxyx(stdscr, max_y, max_x);

    // Posição inicial da string
    int x = 0;
    // int y = max_y / 2;
    int y = 0;


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
        if (x > colunas) {
            x = 0;
        }

        // Aguarda um curto período de tempo para controlar a velocidade da animação
        usleep(100000); // 100ms
    }

    // Finaliza a biblioteca NCurses
    endwin();
}



int main() {
    // Abrir o arquivo e mapeá-lo na memória
    char* map = registers_map(FILE_PATH, FILE_SIZE);
    if (map == NULL) {
        return EXIT_FAILURE;
    }


    int opcao_principal;
    int opcao_cores;
    int valor_componente;
    //int valor_intensidade;

    do {
        //painel("Radiohead");
        // Exibir menu principal
        exibir_menu_principal();
        scanf("%d", &opcao_principal);

        switch (opcao_principal) {
            case 1:
                // Exibir menu de manipulação das cores
                exibir_menu_cores();
                scanf("%d", &opcao_cores);

                // Solicitar valor do componente
                printf("Digite 0 para desligar ou 1 para ligar: ");
                scanf("%d", &valor_componente);

                switch (opcao_cores) {
                    case 1:
                        // Manipular o componente vermelho (R)
                        set_valor_R(map, valor_componente);
                        // printf("Defina a intensidade de R: ");
                        // scanf("%d", &valor_intensidade);
                        // set_intensity_R(map, valor_intensidade);
                        break;
                    case 2:
                        // Manipular o componente verde (G)
                        set_valor_G(map, valor_componente);
                        break;
                    case 3:
                        // Manipular o componente azul (B)
                        set_valor_B(map, valor_componente);
                        break;
                    default:
                        printf("Opção inválida.\n");
                        break;
                }
                break;
            case 0:
                // Imprimir os valores armazenados nos registradores de RGB
                // printf("\nValores armazenados nos registradores de RGB:\n");
                // printf("R: %d\n", (*((unsigned short *)(map + (2 * sizeof(unsigned short)))) >> 10) & 0x01);
                // printf("G: %d\n", (*((unsigned short *)(map + (2 * sizeof(unsigned short)))) >> 11) & 0x01);
                // printf("B: %d\n", (*((unsigned short *)(map + (2 * sizeof(unsigned short)))) >> 12) & 0x01);
                print_message_with_color_and_rgb("Radiohead", map);
                set_intensity_R(map, 123);
                set_intensity_G(map, 100);
                set_intensity_B(map, 30);
                print_component_intensities(map);
                // painel("Radiohead");
                //return 0;
                break;
            default:
                printf("Opção inválida.\n");
                break;
        }
    } while (opcao_principal != 0);

    // Liberar recursos
    if (registers_release(map, FILE_SIZE) == -1) {
