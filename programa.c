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

void set_led_status(char* base_address, int status) {
    // Verifica se o status é válido (0 ou 1)
    if (status != 0 && status != 1) {
        fprintf(stderr, "Erro: Status do LED inválido. Deve ser 0 (desligado) ou 1 (ligado).\n");
        return;
    }

    // Lê o valor atual do registrador de controle
    unsigned short control_register_value = *((unsigned short *)(base_address + (2 * sizeof(unsigned short))));

    // Limpa o bit correspondente ao status do LED (bit 9)
    control_register_value &= ~(1 << 9);

    // Define o novo status do LED
    control_register_value |= (status << 9);

    // Escreve o novo valor no registrador de controle
    *((unsigned short *)(base_address + (2 * sizeof(unsigned short)))) = control_register_value;

    //printf("Status do LED atualizado para: %d\n", status);
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
    int led_status = (control_register_value >> 9) & 0x01;

    if (led_status == 0) {
        printf("Led está desligado!");
        return;
    }

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
        //Imprime a mensagem com a cor especificada
    printf("\x1b[38;2;%d;%d;%dm", (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    printf("%s\n", message);
    printf("\x1b[0m");
}

void set_battery_level(char* base_address, int level) {
    // Verifica se o nível de bateria é válido (0 a 3)
    if (level < 0 || level > 3) {
        fprintf(stderr, "Erro: Nível de bateria inválido. Deve estar entre 0 e 3.\n");
        return;
    }

    // Lê o valor atual do registrador R3
    unsigned short register_value = *((unsigned short *)(base_address + (3 * sizeof(unsigned short))));

    // Limpa os bits correspondentes ao nível de bateria (bits 0 e 1)
    register_value &= ~(0b11);

    // Define o novo nível de bateria
    register_value |= level;
    int battery_level = register_value & 0b11;

    // Escreve o novo valor no registrador R3
    *((unsigned short *)(base_address + (3 * sizeof(unsigned short)))) = register_value;

    //printf("Nível de bateria definido em binário para: %d%d\n", (battery_level >> 1) & 1, battery_level & 1);
}

// Função para exibir o sensor na tela com base no nível de bateria
void display_battery_sensor(char* base_address) {
    // Lê o valor atual do registrador R3
    unsigned short register_value = *((unsigned short *)(base_address + (3 * sizeof(unsigned short))));

    // Obtém os dois primeiros bits do valor do registrador R3 (bits 0 e 1)
    int battery_level = register_value & 0b11;

    printf("Sensor de bateria: ");

    // Determine a cor do sensor com base no nível de bateria
    switch (battery_level) {
        case 0:
            printf("\033[0;31mCRÍTICO"); // Vermelho
            break;
        case 1:
            printf("\033[0;33mBAIXO"); // Amarelo
            break;
        default:
            printf("\033[0;32mNORMAL/ALTO"); // Verde
            break;
    }

    printf("\033[0m\n"); // Restaura a cor padrão do terminal
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
// Função para armazenar a velocidade definida no registrador R0, dos bits 3 até o 8
void armazenar_velocidade_R0(char* base_address, int velocidade) {
    // Verificar se a velocidade está dentro do intervalo permitido (0 a 63)
    if (velocidade < 0 || velocidade > 63) {
        // Exibir uma mensagem de erro ou tomar alguma ação apropriada, se necessário
        return;
    }

    // Obter o valor atual do registrador R0
    unsigned short r0_register_value = *((unsigned short *)(base_address));

    // Limpar os bits dos bits 3 até o 8 do registrador R0
    r0_register_value &= ~(((1 << 6) - 1) << 3);

    // Armazenar a velocidade nos bits 3 até o 8 do registrador R0
    r0_register_value |= ((velocidade & 0x3F) << 3);

    // Escrever o novo valor no registrador R0
    *((unsigned short *)(base_address)) = r0_register_value;

    // Printar o valor armazenado na tela
    printf("Valor armazenado no registrador R0: %u\n", r0_register_value);
}

void animate_text(const char *message, char *memory_address, int speed) {
    const char *space = "                    "; // Espaços suficientes para limpar o texto anterior
    int text_length = strlen(message);
    int screen_width = 150; // Largura padrão do terminal
    int i;
    int delay;

    //int vel = speed * 10;

    switch(speed) {
        case 1:
            delay = 100000; // 0.1 segundo
            break;
        case 2:
            delay = 50000; // 0.05 segundo
            break;
        case 3:
            delay = 3000; // 0.025 segundo
            break;
        default:
            delay = 100000; // Velocidade padrão: 0.1 segundo
            break;
    }

    while (true) {
        for (i = 0; i <= screen_width - text_length; i++) {
            printf("\x1b[2J\x1b[1;1H"); // Limpa a tela e reposiciona o cursor
            printf("%.*s", i, space); // Espaços para mover o texto para a direita
            print_message_with_color_and_rgb(message, memory_address); // Imprime a mensagem com cor
            usleep(delay); // Espera o atraso especificado
        }
    }
}


int main() {
    // Abrir o arquivo e mapeá-lo na memória
    char* map = registers_map(FILE_PATH, FILE_SIZE);
    if (map == NULL) {
        return EXIT_FAILURE;
    }
    set_valor_R(map, 0);
    set_valor_G(map, 1);
    set_valor_B(map, 0);
    //armazenar_velocidade_R0(map, 3);
    configure_text_display(map, "Hello, LED World!");
    char* text = read_message_registers(map);
    animate_text(text, map, 2);
    //print_message_with_color_and_rgb(map, "Hello");
    set_led_status(map, 1);

// Ler a mensagem dos registradores de R4 a R11
char* message = read_message_registers(map);
if (message != NULL) {
    // Imprimir a mensagem com as cores RGB no ncurses
    print_message_with_color_and_rgb(message, map);

    free(message);  // Liberar a memória alocada para a mensagem
}

// Liberar recursos
if (registers_release(map, FILE_SIZE) == -1) {
    return EXIT_FAILURE;
}

return EXIT_SUCCESS;
}
