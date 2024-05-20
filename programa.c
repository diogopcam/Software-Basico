#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ncurses.h>

#define FILE_PATH "registers.bin"
#define FILE_SIZE 1024  // Tamanho do arquivo de registros
#define LED_DISPLAY_REGISTERS 24

// Função para abrir ou criar o arquivo e mapeá-lo na memória
char* registers_map(const char* file_path, int file_size) {
    int fd = open(file_path, O_RDWR | O_CREAT, 0666);
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

    close(fd);  // Fechamos o descritor de arquivo porque não é mais necessário
    return map;
}

// Função para liberar a memória mapeada e fechar o descritor de arquivo
int registers_release(void* map, int file_size) {
    if (munmap(map, file_size) == -1) {
        perror("Erro ao desmapear o arquivo");
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

// Função para limpar a mensagem armazenada no display de LED

// Função para limpar a mensagem armazenada no display de LED
void clear_text_display(char* base_address) {
    // Preenche os registradores de dados com espaços em branco
    for (int i = 3; i < LED_DISPLAY_REGISTERS; i++) {
        *((unsigned short *)(base_address + (i * sizeof(unsigned short)))) = ' ';
    }
}

// Função para configurar a mensagem no display de LED
// Função para configurar a mensagem no display de LED
void configure_text_display(char* base_address, const char* message) {
    // Limpa a mensagem armazenada no display
    clear_text_display(base_address);

    // Calcula o comprimento da nova mensagem
    int message_length = strlen(message) + 1;

    // Verifica se a mensagem excede o limite de caracteres do display
    if (message_length > LED_DISPLAY_REGISTERS) {
        fprintf(stderr, "Erro: mensagem excede o limite de caracteres do display\n");
        return;
    }

    // Escreve a nova mensagem nos registradores de dados (R4 a R11)
    for (int i = 0; i < message_length; i++) {
        *((unsigned short *)(base_address + ((i + 3) * sizeof(unsigned short)))) = message[i - 1];
    }
}


// Função para ler a mensagem dos registradores de R4 a R11
char* read_message_registers(char* base_address) {
    char* message = (char*)malloc(LED_DISPLAY_REGISTERS * sizeof(char)); // Não é necessário espaço para o caractere nulo
    if (message == NULL) {
        perror("Erro ao alocar memória para a mensagem");
        return NULL;
    }

    // Lê a mensagem dos registradores de R4 a R11
    for (int i = 0; i < LED_DISPLAY_REGISTERS; i++) {
        message[i] = *((unsigned short *)(base_address + ((i + 4) * sizeof(unsigned short))));
    }

    return message;
}

// Função para imprimir a mensagem com a cor especificada
void print_message_with_color_and_rgb(const char* message, char* base_address) {
    unsigned short control_register_value = *((unsigned short *)(base_address + (2 * sizeof(unsigned short))));
    
    // Verifica o status dos componentes RGB
    int red_on = (control_register_value >> 10) & 0x01;
    int green_on = (control_register_value >> 11) & 0x01;
    int blue_on = (control_register_value >> 12) & 0x01;

    // Calcula o valor RGB com base nos bits de controle
    unsigned int color = 0;
    if (red_on) {
        color |= 0xFF0000;
    }
    if (green_on) {
        color |= 0x00FF00;
    }
    if (blue_on) {
        color |= 0x0000FF;
    }

        // Imprime a mensagem com a cor especificada
    printf("\x1b[38;2;%d;%d;%dm", (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    printf("%s\n", message);
    printf("\x1b[0m");
}

// Função para exibir o menu principal
void exibir_menu_principal() {
    printf("Seja bem vindo ao sistema de LED.\n");
    printf("Digite 0 para sair do programa\n");
    printf("Digite 1 para manipular as cores\n");
}

// Função para exibir o menu de manipulação das cores
void exibir_menu_cores() {
    printf("\nEscolha qual componente você deseja manipular:\n");
    printf("R - 1\n");
    printf("G - 2\n");
    printf("B - 3\n");
}
int main() {
    // Abrir o arquivo e mapeá-lo na memória
    char* map = registers_map(FILE_PATH, FILE_SIZE);
    if (map == NULL) {
        return EXIT_FAILURE;
    }

    // Configurar a mensagem no display de LED
    configure_text_display(map, "ollllllllllllllllllllll");

    // Ler a mensagem dos registradores de R4 a R11
    char* message = read_message_registers(map);
    if (message != NULL) {
        printf("Mensagem armazenada nos registradores: %s\n", message);
        set_valor_R(map, 1);
        print_message_with_color_and_rgb(message, map);
        free(message);  // Liberar a memória alocada para a mensagem
    }

    // Liberar recursos
    if (registers_release(map, FILE_SIZE) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
