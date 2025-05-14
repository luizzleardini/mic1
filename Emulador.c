#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tipos
typedef unsigned char byte; // 8 bits
typedef unsigned int palavra; // 32 bits
typedef unsigned long int microinstrucao; // 64 bits, no caso de acordo com a arquitetura cada microinstrução usa apenas 36 bits de espaço 

// Registradores 
palavra MAR = 0, MDR = 0, PC = 0; // Acesso da Memoria
byte MBR = 0; // Acesso da Memoria

palavra SP = 0, LV = 0, TOS = 0; // Operação da ULA 
palavra OPC = 0, CPP = 0, H = 0; // Operação da ULA 

microinstrucao MIR; // Contém a Microinstrução Atual
palavra MPC = 0; // Contém o endereço para a próxima Microinstrução

// Barramentos
palavra Barramento_B, Barramento_C;

// Flip-Flops
byte N, Z;

// Auxiliares para Decodificar Microinstrução
byte MIR_B, MIR_Operacao, MIR_Deslocador, MIR_MEM, MIR_pulo;
palavra MIR_C;

// Armazenamento de Controle
microinstrucao Armazenamento[512];

// Memória Principal
byte Memoria[100000000];

// Prototipo das Funções
void carregar_microprogram_de_controle();
void carregar_programa(const char *programa);
void exibir_processos();
void decodificar_microinstrucao();
void atribuir_barramento_B();
void realizar_operacao_ALU();
void atribuir_barramento_C();
void operar_memoria();
void pular();
void binario(void* valor, int tipo);

// Laço Principal
int main(int argc, const char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <programa>\n", argv[0]);
        return EXIT_FAILURE;
    }

    carregar_microprogram_de_controle();
    carregar_programa(argv[1]);

    while (1) {
        exibir_processos();
        MIR = Armazenamento[MPC];

        decodificar_microinstrucao();
        atribuir_barramento_B();
        realizar_operacao_ALU();
        atribuir_barramento_C();
        operar_memoria();
        pular();
    }

    return 0;
}

// Implementação das Funções
void carregar_microprogram_de_controle() {
    FILE* MicroPrograma = fopen("microprog.rom", "rb");
    if (MicroPrograma == NULL) {
        fprintf(stderr, "Erro ao abrir microprog.rom\n");
        exit(EXIT_FAILURE);
    }

    fread(Armazenamento, sizeof(microinstrucao), 512, MicroPrograma);
    fclose(MicroPrograma);
}

void carregar_programa(const char* prog) {
    FILE* Programa = fopen(prog, "rb");
    if (Programa == NULL) {
        fprintf(stderr, "Erro ao abrir o programa: %s\n", prog);
        exit(EXIT_FAILURE);
    }

    palavra tamanho;
    byte tamanho_temp[4];
    fread(tamanho_temp, sizeof(byte), 4, Programa); // Lendo o tamanho em bytes do Programa
    memcpy(&tamanho, tamanho_temp, 4);

    fread(Memoria, sizeof(byte), 20, Programa); // Lendo os 20 bytes de Inicialização
    fread(&Memoria[0x0401], sizeof(byte), tamanho - 20, Programa); // Lendo o Programa

    fclose(Programa);
}

void decodificar_microinstrucao() {
    MIR_B = MIR & 0b1111;
    MIR_MEM = (MIR >> 4) & 0b111;
    MIR_C = (MIR >> 7) & 0b111111111;
    MIR_Operacao = (MIR >> 16) & 0b111111;
    MIR_Deslocador = (MIR >> 22) & 0b11;
    MIR_pulo = (MIR >> 24) & 0b111;
    MPC = (MIR >> 27) & 0b111111111;
}

void atribuir_barramento_B() {
    switch (MIR_B) {
        case 0: Barramento_B = MDR; break;
        case 1: Barramento_B = PC; break;
        case 2: 
            Barramento_B = MBR;
            if (MBR & 0b10000000) {
                Barramento_B |= (0b111111111111111111111111 << 8);
            }
