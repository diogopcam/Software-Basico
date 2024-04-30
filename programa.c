#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>

#define FILE_PATH "registers.bin"
#define FILE_SIZE 1024  // Tamanho do arquivo de registros
#define LED_DISPLAY_REGISTERS 8
#define OPERATION_LED_REGISTER 9
#define RGB_LED_REGISTER 10
#define TEMPERATURE_SENSOR_REGISTER 11
#define BATTERY_REGISTER 12
// Definindo enumeração para cores
enum Colors {
    RED,
    GREEN,
    BLUE
};

#define RED_COLOR_VALUE 0xFF0000
#define GREEN_COLOR_VALUE 0x00FF00
#define BLUE_COLOR_VALUE 0x0000FF

int fd = -1; // Descritor de arquivo global

// Função para abrir ou criar o arquivo e mapeá-lo na memória
char* registers_map(const char* file_path, int file_size) {
    fd = open(file_path, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("Erro ao abrir ou criar o arquivo");
        return NULL;
    }

    // Garante que o arquivo tenha o tamanho correto
    if (ftruncate(fd, file_size) == -1) {
        perror("Erro ao definir o tamanho do arquivo");
        close(fd);
        return NULL;
    }

    // Mapeia o arquivo na memória
    char *map = mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Erro ao mapear o arquivo");
        close(fd);
        return NULL;
    }

    return map;
}

// Função para liberar a memória mapeada e fechar o descritor de arquivo
int registers_release(void* map, int file_size) {
    if (munmap(map, file_size) == -1) {
        perror("Erro ao desmapear o arquivo");
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        perror("Erro ao fechar o arquivo");
        return -1;
    }

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

// Função para definir a intensidade do componente vermelho (R) no display LED
void set_intensity_R(char* base_address, int intensity) {
    // Garante que a intensidade esteja dentro do intervalo válido (0-255)
    if (intensity < 0 || intensity > 255) {
        fprintf(stderr, "Erro: Intensidade do componente R fora do intervalo válido (0-255)\n");
        return;
    }

    // Escreve a intensidade do componente R no registrador de controle 1 (R1)
    *((unsigned short *)(base_address + (1 * sizeof(unsigned short)))) = intensity;
}

// Função para definir a intensidade do componente verde (G) no display LED
void set_intensity_G(char* base_address, int intensity) {
    // Garante que a intensidade esteja dentro do intervalo válido (0-255)
    if (intensity < 0 || intensity > 255) {
        fprintf(stderr, "Erro: Intensidade do componente G fora do intervalo válido (0-255)\n");
        return;
    }

    // Escreve a intensidade do componente G no registrador de controle 1 (R1)
    *((unsigned short *)(base_address + (1 * sizeof(unsigned short)))) = intensity;
}

// Função para definir a intensidade do componente azul (B) no display LED
void set_intensity_B(char* base_address, int intensity) {
    // Garante que a intensidade esteja dentro do intervalo válido (0-255)
    if (intensity < 0 || intensity > 255) {
        fprintf(stderr, "Erro: Intensidade do componente B fora do intervalo válido (0-255)\n");
        return;
    }

    // Escreve a intensidade do componente B no registrador de controle 2 (R2)
    *((unsigned short *)(base_address + (2 * sizeof(unsigned short)))) = intensity;
}

void print_message_with_color_and_rgb(const char* message, char* base_address) {
    // Verifica o status do LED (bit 9)
    unsigned short control_register_value = *((unsigned short *)(base_address + (2 * sizeof(unsigned short))));
    int led_status = (control_register_value >> 9) & 0x01;

    // Se o LED estiver desligado, não imprime a mensagem
    if (led_status == 0) {
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
        printf("Vermelho");
    }
    if (green_on) {
        color |= 0x00FF00; // Define o componente verde como máximo
        printf("Verde");
    }
    if (blue_on) {
        color |= 0x0000FF; // Define o componente azul como máximo
        printf("Azul");
    }

    // Imprime a mensagem com a cor especificada
    printf("\x1b[38;2;%d;%d;%dm", (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    for (i = 0; i < strlen(message) && (i + 4) < 16; i++) {
        printf("%c", message[i]);
    }
    printf("\x1b[0m\n");
}

// void print_message_with_color_and_rgb(const char* message, char* base_address) {
//     // Obtém os valores dos registradores R, G e B
//     int red_on = (*((unsigned short *)(base_address + (2 * sizeof(unsigned short)))) >> 10) & 0x01;
//     int green_on = (*((unsigned short *)(base_address + (2 * sizeof(unsigned short)))) >> 11) & 0x01;
//     int blue_on = (*((unsigned short *)(base_address + (2 * sizeof(unsigned short)))) >> 12) & 0x01;

//     // Calcula a cor com base nos valores dos registradores
//     unsigned int color = 0;
//     if (red_on) {
//         color |= 0xFF0000; // Componente vermelho máximo
//     }
//     if (green_on) {
//         color |= 0x00FF00; // Componente verde máximo
//     }
//     if (blue_on) {
//         color |= 0x0000FF; // Componente azul máximo
//     }

//     // Inicialização do ncurses
//     initscr();
//     start_color();
//     init_pair(1, COLOR_BLACK, color);

//     // Preenche o painel com a mensagem colorida
//     int painel_largura = 40;
//     int painel_altura = 10;
//     char painel[painel_altura][painel_largura];

//     // Limpa o painel
//     for (int linha = 0; linha < painel_altura; linha++) {
//         for (int coluna = 0; coluna < painel_largura; coluna++) {
//             painel[linha][coluna] = ' ';
//         }
//     }

//     // Exibe a mensagem colorida no painel
//     attron(COLOR_PAIR(1));
//     for (int i = 0; i < strlen(message) && (i + 4) < 16; i++) {
//         int linha = i / painel_largura;
//         int coluna = i % painel_largura;
//         painel[linha][coluna] = message[i];
//     }
//     for (int linha = 0; linha < painel_altura; linha++) {
//         mvprintw(linha, 0, painel[linha]);
//     }
//     attroff(COLOR_PAIR(1));
//     refresh();

//     // Finalização do ncurses
//     endwin();
// }
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
    // Abrir o arquivo e mapeá-lo na memória
    char* map = registers_map(FILE_PATH, FILE_SIZE);
    if (map == NULL) {
        return EXIT_FAILURE;
    }

    int opcao_principal;
    int opcao_cores;
    int valor_componente;

    do {
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
                painel("Radiohead");
                //return 0;
                break;
            default:
                printf("Opção inválida.\n");
                break;
        }
    } while (opcao_principal != 0);

    // Liberar recursos
    if (registers_release(map, FILE_SIZE) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}