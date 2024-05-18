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
#define FILE_PATH "registers.bin"
#define FILE_SIZE 1024  // Tamanho do arquivo de registros
#define LED_DISPLAY_REGISTERS 8
#define OPERATION_LED_REGISTER 9
#define RGB_LED_REGISTER 10
#define TEMPERATURE_SENSOR_REGISTER 11
#define BATTERY_REGISTER 12
WINDOW *painel; // Painel global para uso na thread de animação

// Definindo enumeração para cores
enum Colors {
    RED,
    GREEN,
    BLUE
};

#define RED_MASK 0xF800
#define GREEN_MASK 0x07E0
#define BLUE_MASK   0xFF

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
    value |= ((intensity << 8) & RED_MASK);

    // Escreve o novo valor no registrador R1
    *((unsigned short *)(base_address + (1 * sizeof(unsigned short)))) = value;

    printf("Valor do registrador R1 após definir a intensidade do componente vermelho: %hu\n", *((unsigned short *)(base_address + (1 * sizeof(unsigned short)))));
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
    value |= ((intensity << 2) & GREEN_MASK);

    // Escreve o novo valor no registrador R1
    *((unsigned short *)(base_address + (1 * sizeof(unsigned short)))) = value;

    printf("Valor do registrador R1 após definir a intensidade do componente verde: %hu\n", *((unsigned short *)(base_address + (1 * sizeof(unsigned short)))));
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
    //printf("Valor do registrador R1 após definir a intensidade do componente azul: %hu\n", value);
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

void print_message_with_color_and_rgb(const char* message, char* base_address) {
    // Verifica o status do LED (bit 9)
    unsigned short control_register_value = *((unsigned short *)(base_address + (2 * sizeof(unsigned short))));
    int led_status = (control_register_value >> 9) & 0x01;

    // Se o LED estiver desligado, não imprime a mensagem
    if (led_status == 0) {
        printf("Led está desligado!");
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

    unsigned short blue_intensity = *((unsigned short *)(base_address + (2 * sizeof(unsigned short)))) & BLUE_MASK;

    // Calcula o valor RGB com base nos bits de controle
    int red_on = (control_register_value >> 10) & 0x01;
    int green_on = (control_register_value >> 11) & 0x01;
    int blue_on = (control_register_value >> 12) & 0x01;
    unsigned int color = 0;
    if (red_on) {
        color |= 0xFF0000; // Define o componente vermelho como máximo
        // DEFINE O TOM DO VERMELHO:
        //color |= ((blue_intensity & 0xFF) << 16); // Ajuste para o deslocamento correto
    }
    if (green_on) {
        color |= 0x00FF00; // Define o componente verde como máximo
    }
    if (blue_on) {
        //color |= 0x0000FF; // Define o componente azul como máximo
        color |= ((blue_intensity & 0xFF) << 0);
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


// Menu de LED no painel
void exibir_menu_led(WINDOW *painel) {
    werase(painel);  // Limpa o painel antes de exibir o novo menu
    mvwprintw(painel, 1, 2, "Seja bem vindo ao sistema de LED.");
    mvwprintw(painel, 2, 2, "Digite 0 para desligar o LED");
    mvwprintw(painel, 3, 2, "Digite 1 para ligar o LED");
    wrefresh(painel);  // Atualiza o painel para mostrar as mudanças
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


// Função para definir a temperatura do LED nos bits do 6 ao 15 do registrador R3
void set_led_temperature(char* base_address, int temperature) {
    // Garante que a temperatura esteja dentro do intervalo válido (0-1023)
    if (temperature < 0 || temperature > 1023) {
        fprintf(stderr, "Erro: Temperatura do LED fora do intervalo válido (0-1023)\n");
        return;
    }

    // Lê o valor atual do registrador R3
    unsigned short register_value = *((unsigned short *)(base_address + (3 * sizeof(unsigned short))));

    printf("Valor armazenado nos bits do 6 ao 15 antes da escrita: %d\n", register_value);
    
    // Limpa os bits do 6 ao 15 do registrador R3
    register_value &= ~(0b111111111111 << 6);

    // Define a temperatura do LED nos bits do 6 ao 15 do registrador R3 (dividida por 10)
    register_value |= ((temperature / 10) << 6);

    // Escreve o novo valor no registrador R3
    *((unsigned short *)(base_address + (3 * sizeof(unsigned short)))) = register_value;

    printf("Valor armazenado nos bits do 6 ao 15 após a escrita: %d\n", register_value);

    printf("Temperatura do LED definida como: %d\n", temperature);
}

// MENUS VALIDOS PARA BAIXO

void exibir_menu_painel_led(WINDOW *painel) {
    mvwprintw(painel, 1, 2, "Seja bem vindo ao sistema de LED.");
    mvwprintw(painel, 2, 2, "Digite 0 para ligar ou desligar o LED");
    mvwprintw(painel, 3, 2, "Digite 1 para manipular as cores");
    mvwprintw(painel, 4, 2, "Digite 2 para controlar o nível da bateria");
    mvwprintw(painel, 5, 2, "Digite 3 para controlar o nível da temperatura");
    wrefresh(painel);
}

void exibir_menu_liga_led(WINDOW *painel){
    mvwprintw(painel, 1, 2, "Seja bem vindo ao sistema de LED.");
    mvwprintw(painel, 2, 2, "Digite 0 para desligar o LED");
    mvwprintw(painel, 3, 2, "Digite 1 para ligar o LED");
    wrefresh(painel);
}

// Função para exibir o menu de manipulação das cores
void exibir_menu_cores(WINDOW *painel) {
    mvwprintw(painel, 1, 2, "Escolha qual componente você deseja manipular:");
    mvwprintw(painel, 2, 2, "R - 1");
    mvwprintw(painel, 3, 2, "G - 2");
    mvwprintw(painel, 4, 2, "B - 3");
    wrefresh(painel);
}

void exibir_menu_bateria(WINDOW *painel){
    mvwprintw(painel, 1, 2, "Defina o nível de bateria (0, 1, 2 ou 3): ");
    wrefresh(painel);
}

void exibir_menu_temperatura(WINDOW *painel){
    mvwprintw(painel, 1, 2, "Defina a temperatura do LED (de 0-1023):");
    wrefresh(painel);
}

void limpar_linhas_painel(WINDOW *painel){
    char espacamento[] = "                                               ";
    mvwprintw(painel, 1, 2, espacamento);
    mvwprintw(painel, 2, 2, espacamento);
    mvwprintw(painel, 3, 2, espacamento);
    mvwprintw(painel, 4, 2, espacamento);
    mvwprintw(painel, 5, 2, espacamento);
    mvwprintw(painel, 6, 2, espacamento);
    mvwprintw(painel, 7, 2, espacamento);
    wrefresh(painel);
}
// Para implementar:
// se o valor for 0, apresente o menu de controle do LED

void *exibir_animacao_hello_world(void *arg) {
    char texto[] = "hello world";
    int comprimento_texto = strlen(texto);
    int coluna_inicial = 2; // Coluna inicial
    int linha = 12; // Linha onde o texto será exibido
    int largura_painel = getmaxx(painel) - 4; // Largura do painel, descontando bordas

    while (true) {
        for (int i = 0; i <= largura_painel - comprimento_texto; ++i) {
            mvwprintw(painel, linha, coluna_inicial + i, "%s", texto);
            wrefresh(painel);
            usleep(100000); // Pausa de 0,1 segundo
            mvwprintw(painel, linha, coluna_inicial + i, "           "); // Apaga o texto na posição anterior
        }
    }
    return NULL;
}

void exibir_painel(char* base_address) {
    // Inicializa o modo ncurses
    initscr();

    // Desativa o modo de eco de caracteres digitados
    noecho();

    // Define a tecla Enter como fim de entrada
    keypad(stdscr, TRUE);

    // Obtém as dimensões da tela
    int linhas, colunas;
    getmaxyx(stdscr, linhas, colunas);

    // Calcula a posição central para o painel
    int inicio_linha = linhas / 2 - 2; // Diminui 2 para centralizar verticalmente
    int inicio_coluna = colunas / 2 - 12; // Diminui 12 para centralizar horizontalmente

    // Cria uma nova janela para o painel
    painel = newwin(50, 50, 6, 6);

    // Define o estilo da janela
    box(painel, 0, 0); // Adiciona uma borda à janela


    // Exibe a animação "hello world" na linha 12 do painel
    exibir_menu_painel_led(painel);

    // Cria a thread para a animação
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, exibir_animacao_hello_world, NULL);
    
    // Atualiza a tela
    wrefresh(painel);

    // Captura a entrada do usuário
    char entrada[100];
    mvwgetstr(painel, 6, 2, entrada); // Ajusta as coordenadas para capturar a entrada
    
    int opcao_inicial = atoi(entrada);

// Manipulação para ligar ou desligar o LED
switch(opcao_inicial) {
    case 0:
        mvwprintw(painel, 6, 2, "                                ");
        mvwprintw(painel, 7, 2, "Funcionou essa merda");
        wrefresh(painel); // Atualiza o painel após imprimir o texto

        // Aguarda um tempo para que o usuário possa ler a mensagem
        sleep(1); // Importe <unistd.h> para usar a função sleep

        // Limpa o painel antes de exibir o novo menu
        limpar_linhas_painel(painel);
        // Exibe o novo menu
        exibir_menu_liga_led(painel); // Mostra o novo menu
        
        // Obtém a nova entrada do usuário
        mvwgetstr(painel, 6, 2, entrada);
        int opcao_led = atoi(entrada);
        // Atualiza o painel após imprimir o novo menu
        wrefresh(painel);

        // Verifica se a entrada é válida (1 ou 0)
        if (opcao_led == 0 || opcao_led == 1 ) {
            limpar_linhas_painel(painel);
            mvwprintw(painel, 7, 2, "Ligar ou desligar o LED");
            mvwprintw(painel, 8, 2, entrada);
            set_led_status(base_address, opcao_led);
            wrefresh(painel); // Atualiza o painel após imprimir o novo menu
        } else {
            limpar_linhas_painel(painel);
            mvwprintw(painel, 7, 2, "Entrada inválida");
            wrefresh(painel); // Atualiza o painel após imprimir o novo menu
        }
        break;
    
    case 1:
        mvwprintw(painel, 6, 2, "                                ");
        mvwprintw(painel, 7, 2, "Funcionou essa merda da cor");
        wrefresh(painel); // Atualiza o painel após imprimir o texto

        // Aguarda um tempo para que o usuário possa ler a mensagem
        sleep(1); // Importe <unistd.h> para usar a função sleep

        
        // Limpa o painel antes de exibir o novo menu
        //werase(painel);
        limpar_linhas_painel(painel);
        exibir_menu_cores(painel); // Mostra o novo menu
        wrefresh(painel); // Atualiza o painel após imprimir o novo menu

        // Obtém a nova entrada do usuário
        mvwgetstr(painel, 6, 2, entrada);
        int opcao_cor = atoi(entrada);

        if (opcao_cor == 1 || opcao_cor == 2 || opcao_cor == 3) {
            if(opcao_cor == 1){
                set_valor_R(base_address, opcao_cor);
                mvwprintw(painel, 7, 2, "VERMELHO");
            }
            if(opcao_cor == 2){
                set_valor_G(base_address, opcao_cor);
                mvwprintw(painel, 7, 2, "VERDE");
            }
            if(opcao_cor == 3){
                set_intensity_B(base_address, opcao_cor);
                mvwprintw(painel, 7, 2, "AZUL");
            }
            limpar_linhas_painel(painel);
            mvwprintw(painel, 8, 2, entrada);
            //set_led_status(base_address, opcao_cor);
            wrefresh(painel); // Atualiza o painel após imprimir o novo menu
        } else {
            limpar_linhas_painel(painel);
            mvwprintw(painel, 7, 2, "Entrada inválida");
            wrefresh(painel); // Atualiza o painel após imprimir o novo menu
        }
        break;

    case 2:
        mvwprintw(painel, 6, 2, "                                ");
        mvwprintw(painel, 7, 2, "Funcionou essa merda da bateria");
        wrefresh(painel); // Atualiza o painel após imprimir o texto

        // Aguarda um tempo para que o usuário possa ler a mensagem
        sleep(1); // Importe <unistd.h> para usar a função sleep

        // Limpa o painel antes de exibir o novo menu
        //werase(painel);
        limpar_linhas_painel(painel);
        exibir_menu_bateria(painel); // Mostra o novo menu
        wrefresh(painel); // Atualiza o painel após imprimir o novo menu

        // Obtém a nova entrada do usuário
        mvwgetstr(painel, 6, 2, entrada);
        int opcao_bateria = atoi(entrada);
        // Verifica se a entrada é válida (1 ou 0)
        if (opcao_bateria == 0 || opcao_bateria == 1 || opcao_bateria == 2 || opcao_bateria == 3) {
            limpar_linhas_painel(painel);
            mvwprintw(painel, 7, 2, "Definicao do nivel de bateria");
            mvwprintw(painel, 8, 2, entrada);
            set_battery_level(base_address, opcao_bateria);
            wrefresh(painel); // Atualiza o painel após imprimir o novo menu
        } else {
            limpar_linhas_painel(painel);
            mvwprintw(painel, 7, 2, "Entrada inválida");
            wrefresh(painel); // Atualiza o painel após imprimir o novo menu
        }
        break;

    case 3:
        mvwprintw(painel, 6, 2, "                                ");
        mvwprintw(painel, 7, 2, "Funcionou essa merda da temperatura");
        wrefresh(painel); // Atualiza o painel após imprimir o texto

        // Aguarda um tempo para que o usuário possa ler a mensagem
        sleep(1); // Importe <unistd.h> para usar a função sleep

        // Limpa o painel antes de exibir o novo menu
        //werase(painel);
        limpar_linhas_painel(painel);
        exibir_menu_temperatura(painel); // Mostra o novo menu
        wrefresh(painel); // Atualiza o painel após imprimir o novo menu

        // Obtém a nova entrada do usuário
        mvwgetstr(painel, 6, 2, entrada);
        int opcao_temp = atoi(entrada);
        // Atualiza o painel após imprimir o novo menu
        wrefresh(painel);

        // Verifica se a entrada é válida (1 ou 0)
        if (opcao_temp <= 1033) {
            limpar_linhas_painel(painel);
            mvwprintw(painel, 7, 2, "Temperatura definida");
            mvwprintw(painel, 8, 2, entrada);
            set_led_temperature(base_address, opcao_temp);
            wrefresh(painel); // Atualiza o painel após imprimir o novo menu
        } else {
            limpar_linhas_painel(painel);
            mvwprintw(painel, 7, 2, "Entrada inválida");
            wrefresh(painel); // Atualiza o painel após imprimir o novo menu
        }
    // Outros casos para outras opções do menu...
    default:
        //mvwprintw(painel, 6, 2, "Opção inválida");
        wrefresh(painel); // Atualiza o painel para exibir a mensagem de opção inválida
        break;
    }

    // Espera o usuário pressionar uma tecla antes de sair
    getch();

    // Finaliza a biblioteca ncurses
    endwin();
}


int main() {
    // Abrir o arquivo e mapeá-lo na memória
    char* map = registers_map(FILE_PATH, FILE_SIZE);
    if (map == NULL) {
        return EXIT_FAILURE;
    }

    int opcao_led;
    int nivel_bateria;
    int opcao_principal;
    int opcao_cores;
    int valor_componente;
    int valor_intensidade;

    do {
        exibir_painel(map);
        //scanf("%d", &opcao_principal);
    } while (opcao_principal != 100);

    // Liberar recursos
    if (registers_release(map, FILE_SIZE) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
