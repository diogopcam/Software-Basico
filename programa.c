#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#define FILE_PATH "registers.bin"
#define FILE_SIZE 1024  // Tamanho do arquivo de registros
#define LED_DISPLAY_REGISTERS 8
#define OPERATION_LED_REGISTER 9
#define RGB_LED_REGISTER 10

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

void animate_text(const char *message, char *memory_address, int speed) {
    const char *space = "                    "; // Espaços suficientes para limpar o texto anterior
    int text_length = strlen(message);
    int screen_width = 80; // Largura padrão do terminal
    int i;
    int delay = speed * 100000; // Calcula o atraso em milissegundos

    while (true) {
        for (i = 0; i <= screen_width - text_length; i++) {
            printf("\x1b[2J\x1b[1;1H"); // Limpa a tela e reposiciona o cursor
            printf("%.*s%s\n", i, space, message); // Espaços para mover o texto para a direita
            usleep(delay); // Espera o atraso especificado
        }
    }
}

// Função para definir a velocidade da animação no registrador R0 (bits 3 a 8)
void set_velocidade_animacao(char* base_address, int velocidade) {
    // Verifica se a velocidade está dentro do intervalo válido (1-5)
    if (velocidade < 1 || velocidade > 5) {
        printf("Velocidade inválida. A velocidade deve estar entre 1 e 5.\n");
        return;
    }

    unsigned short control_register_value = *((unsigned short *)(base_address)); // Valor atual do registrador R0
    control_register_value &= ~((0x3F) << 3); // Limpa os bits de velocidade (bits 3 a 8)
    control_register_value |= (velocidade << 3); // Define a velocidade nos bits 3 a 8 do registrador R0
    *((unsigned short *)(base_address)) = control_register_value; // Atualiza o valor no registrador
}

// Função para retornar a velocidade da animação em milissegundos, baseada no valor armazenado no registrador R0
int get_velocidade_milissegundos(char* base_address) {
    unsigned short control_register_value = *((unsigned short *)(base_address)); // Valor atual do registrador R0
    int velocidade = (control_register_value >> 3) & 0x3F; // Extrai os bits de velocidade (bits 3 a 8)
    return velocidade * 100; // Converte a velocidade em milissegundos
}

int main() {
    //char *memory_address = NULL; // Endereço de memória (ainda não utilizado)
        // Abrir o arquivo e mapeá-lo na memória
    char* map = registers_map(FILE_PATH, FILE_SIZE);
    if (map == NULL) {
        return EXIT_FAILURE;
    }
    int speed = 2; // Velocidade padrão: 2 (média)
    char option;
    const char *message = "Hello, World!";

    printf("Controle de Velocidade da Animação:\n");
    printf("1. Muito Lento\n");
    printf("2. Lento\n");
    printf("3. Médio\n");
    printf("4. Rápido\n");
    printf("5. Muito Rápido\n");
    printf("Pressione 'q' para sair.\n");

    while (true) {
        printf("\nEscolha a velocidade (1-5): ");
        scanf(" %c", &option);

        if (option == 'q') {
            break;
        }

        speed = option - '0'; // Converte o caractere para um número inteiro

        if (speed < 1 || speed > 5) {
            printf("Opção inválida. Escolha uma velocidade de 1 a 5.\n");
            continue;
        }

        // Atualiza a velocidade da animação no registrador R0
        set_velocidade_animacao(map, speed);
        // Recupera e imprime a velocidade da animação em milissegundos
        printf("Velocidade da animação: %d milissegundos\n", get_velocidade_milissegundos(map));

        // Animação...
    }

    return 0;
}
