#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

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

// Função para configurar a mensagem no display de LED
void configure_led_display(char* base_address, const char* message, int display_mode, int update_speed, int color) {
    // Calcula o comprimento da mensagem
    int message_length = strlen(message);

    // Verifica se a mensagem excede o limite de 8 caracteres do display
    if (message_length > LED_DISPLAY_REGISTERS) {
        fprintf(stderr, "Erro: mensagem excede o limite de caracteres do display\n");
        return;
    }

    // Escreve o modo de exibição no registrador R0
    *((unsigned short *)(base_address + (0 * sizeof(unsigned short)))) = display_mode;
    printf("Modo de exibição: %d\n", *((unsigned short *)(base_address + (0 * sizeof(unsigned short)))));

    // Escreve a velocidade de atualização no registrador R1
    *((unsigned short *)(base_address + (1 * sizeof(unsigned short)))) = update_speed;
    printf("Velocidade de atualização: %d\n", *((unsigned short *)(base_address + (1 * sizeof(unsigned short)))));

    // Escreve a cor no registrador R2
    unsigned int color_value;
    switch(color) {
        case RED:
            color_value = RED_COLOR_VALUE;
            break;
        case GREEN:
            color_value = GREEN_COLOR_VALUE;
            break;
        case BLUE:
            color_value = BLUE_COLOR_VALUE;
            break;
        default:
            fprintf(stderr, "Erro: cor desconhecida\n");
            return;
    }
    *((unsigned int *)(base_address + (2 * sizeof(unsigned short)))) = color_value;
    printf("Cor: %u\n", *((unsigned int *)(base_address + (2 * sizeof(unsigned short)))));

    // Escreve a mensagem nos registradores de dados (R4 a R11)
    int i;
    for (i = 0; i < message_length && (i + 3) < LED_DISPLAY_REGISTERS; i++) {
        *((unsigned short *)(base_address + ((i + 3) * sizeof(unsigned short)))) = message[i];
    }

    // Preenche os registradores de dados restantes com espaços em branco
    for (; i < LED_DISPLAY_REGISTERS - 3; i++) {
        *((unsigned short *)(base_address + ((i + 3) * sizeof(unsigned short)))) = ' ';
    }

    // Imprime a mensagem configurada
    printf("Mensagem configurada: %s\n", message);
}

// Função para imprimir a mensagem com a cor mapeada no registrador
void print_message_with_color(const char* message, unsigned int color_value) {
    printf("\033[38;2;%u;%u;%um%s\033[0m\n", (color_value >> 16) & 0xFF, (color_value >> 8) & 0xFF, color_value & 0xFF, message);
}

int main() {
    // Abrir o arquivo e mapeá-lo na memória
    char* map = registers_map(FILE_PATH, FILE_SIZE);
    if (map == NULL) {
        return EXIT_FAILURE;
    }

    // Imprimir a string "Display:" na primeira linha do terminal
    printf("Display:\n");

    // Configurar a mensagem "Hello World" com cor vermelha
    configure_led_display(map, "Hello", 0, 0, GREEN); // Modo de exibição: 0, Velocidade de atualização: 0, Cor: RED

    // Obter o valor da cor armazenado no registrador R2
    unsigned int color_value = *((unsigned int *)(map + (2 * sizeof(unsigned short))));

    // Imprimir a mensagem com a cor mapeada
    print_message_with_color("Hello", color_value);



    // Liberar recursos
    if (registers_release(map, FILE_SIZE) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}