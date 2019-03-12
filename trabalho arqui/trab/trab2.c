/****************************************************
*  Trabalho 2 - Arquitetura de Computadores I
*  Professora: Nahri Moreano
*        
*
*  Para compilar: gcc trab2.c -o simmips -Wall
*  Execução: simmips <programa>.text <programa>.data <programa>.txt
*
****************************************************/

#include<stdio.h>
#include<stdlib.h>

#define MAX 16384

unsigned char memoria[MAX];
int registrador[32];
unsigned int IR;
int Hi, Lo;

typedef struct IR_campos {
	unsigned int op;
	unsigned int rs;
	unsigned int rd;
	unsigned int rt;
	unsigned int shamt;
	unsigned int funct;
	int immediate;
	int adress;
} campos;


/*************** Protótipos das funções ***************/
void inicializa (int *, FILE *, FILE *);
unsigned int busca_instrucao (int);
unsigned int extrai(unsigned int, unsigned int, int);
void decodifica_instrucao(unsigned int, campos *);
int executa_instrucao (campos, int);
int ext_signal (int);
int zero_ext (int);
void finaliza (FILE*);
/*****************************************************/

int main (int argc, char * argv[]) {
	
	int PC;
	campos campo;
	FILE * arquivo_reg;
	FILE * arquivo_text;
	FILE * arquivo_data;
	
	arquivo_text = fopen(argv[1], "rb");
	arquivo_data = fopen(argv[2], "rb");
	arquivo_reg  = fopen(argv[3], "w");
	
	if(arquivo_text == NULL || arquivo_data == NULL || arquivo_reg == NULL) {
		printf("Erro: não foi possível ler/criar um ou mais arquivos\n");
	}
	else {
		if (argc == 4) {

			inicializa(&PC, arquivo_text, arquivo_data);

			while (PC >= 0) {

				IR = busca_instrucao(PC);
				
				PC = PC + 4;

				decodifica_instrucao(IR, &campo);

				PC = executa_instrucao(campo, PC);
			}

			printf("\n");

			finaliza(arquivo_reg);
		}
		else printf("Erro: quantidade de argumentos inválida!\n");
	}

	return 0;
}

void inicializa (int * PC, FILE * arquivo_text, FILE * arquivo_data) {

	int i, j = 0;

	*PC= 12288;
	registrador[29] = 12284; // $sp
	registrador[0] = 0;		// $zero

	i = *PC;

	// carrega as instruções do arquivo .text na memória
	while(!feof(arquivo_text)) {
		fread(&memoria[i], sizeof(char), 1, arquivo_text);
		i = i + 1;
	}

	// carrega os dados do arquivo .data na memória
	while(!feof(arquivo_data)) {
		fread(&memoria[j], sizeof(char), 1, arquivo_data);
		j = j + 1;
	}

	fclose(arquivo_text);
	fclose(arquivo_data);

}

unsigned int busca_instrucao (int PC) {

	unsigned int IR = 0;
	int posicao = PC+3;

	while (posicao >= PC) {
		IR  = IR | memoria[posicao];
		if(posicao != PC)
			IR = IR << 8;
		posicao--;
	}

	return IR;

}

void decodifica_instrucao(unsigned int IR, campos * campo) {

	campo->op = 		extrai(0xfc000000, IR, 6);
	campo->rs = 		extrai(0x03e00000, IR, 11);
	campo->rt = 		extrai(0x001f0000, IR, 16);
	campo->rd = 		extrai(0x0000f800, IR, 21);
	campo->shamt =		extrai(0x000007c0, IR, 26);
	campo->funct = 	  	extrai(0x0000003f, IR, 32);
	campo->immediate = 	extrai(0x0000ffff, IR, 32);
	campo->adress = 	extrai(0x003fffff, IR, 32);

}


unsigned int extrai(unsigned int mascara, unsigned int IR, int n) {

	unsigned int campo;

	campo = (IR & mascara) >> (32 - n);

	return campo;
}

int ext_signal (int immediate) {
	
	if ((immediate & 0x00008000) != 0)
		immediate = (immediate | 0xffff0000);

	return immediate;
}

int zero_ext (int immediate) {
	
	if ((immediate & 0x00008000) != 0)
		immediate = (immediate | 0xffff0000);
	else
		immediate = (immediate & 0x0000ffff);

	return immediate;
}


int executa_instrucao (campos campo, int PC) {

	unsigned int 		op = campo.op,
						rs = campo.rs,
						rt = campo.rt,
						rd = campo.rd,
						shamt = campo.shamt,
						funct = campo.funct;

	int 				immediate = campo.immediate,
						adress = campo.adress,
						endereco, i;

	unsigned long int 	aux;

	switch(op) {
		
		case 0: // formato R
			switch (funct) {

				case 0: // sll
					registrador[rd] = registrador[rt] << shamt;
					break;

				case 2: // srl
					registrador[rd] = registrador[rt] >> shamt;
					break;

				case 8: // jr
					PC = registrador[campo.rs];
					break;

				case 12:    // syscall
 					if(registrador[2] == 1) //syscall 1 imprime inteiro
 						printf("%d", registrador[4]);
 					else if (registrador[2] == 4) { // syscall 4 imprime string
 						i = 0;
 						while((memoria[registrador[4]+i]) != '\0') {
 							printf("%c", memoria[registrador[4]+i]);
 						i++;
 						}
 					}
 					else if (registrador[2] == 5) { // syscall 5 le inteiro
 						scanf("%d", &registrador[2]);
 					}
 					else if (registrador[2] == 8) { // syscall 8 le string
 						fgets(((char *)&(memoria[registrador[4]])), registrador[5], stdin);
 	 					}
 					else 		// syscall 10, sai do programa
 						PC = -1; // valor de retorno para finalizar programa
					break;

				case 16: // mfhi
						registrador[rd] = Hi;
					break;

				case 18: // mflo
					registrador[rd] = Lo;
					break;

				case 24: // mult
					aux = registrador[rs] * registrador[rt];
					Hi = ((aux & 0xffffffff00000000) >> 32);
					Lo = (aux & 0x00000000ffffffff);
					break;

				case 26: // div
					Lo = registrador[rs] / registrador[rt];
					Hi = registrador[rs] % registrador[rt];
					break;

				case 32: // add
					registrador[rd] = registrador[rs] + registrador[rt];
					break;

				case 34: // sub
					registrador[rd] = registrador[rs] - registrador[rt];
					break;

				case 36: // and	
					registrador[rd] = registrador[rs] & registrador[rt];
					break;

				case 37: // or
					registrador[rd] = registrador[rs] | registrador[rt];
					break;

				case 38: // xor
					registrador[rd] = registrador[rs] ^ registrador[rt];
					break;

				case 39: // nor
					registrador[rd] = ~(registrador[rs] | registrador[rt]);
					break;

				case 42: // slt
					if (registrador[rs] < registrador[rt])
						registrador[rd] = 1;
					else
						registrador[rd] = 0;
					break;
			}
			break;

		case 2: // j
			PC = ((PC) & 0xf0000000) | (adress << 2);
			break;
		
		case 3: // jal 
			registrador[31] = PC;
			PC = ((PC) & 0xf0000000) | ((adress) << 2);
			break;


		case 4: // beq
			if (registrador[rs] == registrador[rt])
				PC = PC + (ext_signal(immediate) << 2); // PC já é (PC + 4)
			break;

		case 5: // bne
			if (registrador[rs] != registrador[rt])
				PC = PC + (ext_signal(immediate) << 2); // PC já é (PC + 4)
			break;

		case 8: // addi
			registrador[rt] = registrador[rs] + ext_signal(immediate);;
			break;

		case 10: // slti
			if (registrador[rs] < immediate) 
				registrador[rt] = 1;
			else
				registrador[rt] = 0;
			break;

		case 12: // andi
			registrador[rt] = registrador[rs] & zero_ext(immediate);
			break;

		case 13: // ori
			registrador[rt] = registrador[rs] | zero_ext(immediate);
			break;

		case 15: // lui
			registrador[rt] = immediate << 31;
			break;

		case 32: // lb
			endereco = registrador[rs] + ext_signal(immediate);
			registrador[rt] = memoria[endereco];
			break;

		case 35: // instrução lw
			endereco = registrador[rs] + ext_signal(immediate);
			registrador[rt] = (memoria[endereco+3] << 8) + (memoria[endereco+2] << 8) + (memoria[endereco+1] << 8) + (memoria[endereco]);
			break;

		case 40: // sb
			endereco = registrador[rs] + ext_signal(immediate);
			memoria[endereco] = (registrador[rt] & 0x000000ff);
			break;
 

		case 43: // sw
			endereco = registrador[rs] + ext_signal(immediate);
			memoria[endereco] = (registrador[rt] & 0x000000ff);
			memoria[endereco+1] = (registrador[rt] & 0x0000ff00) >> 8;
			memoria[endereco+2] = (registrador[rt] & 0x00ff0000) >> 16;
			memoria[endereco+3] = (registrador[rt] & 0xff000000) >> 24;
			break;
	}

	return PC;

}

void finaliza (FILE * arquivo_reg) {

	fprintf(arquivo_reg, "$zero\t%d\n", registrador[0]);
	fprintf(arquivo_reg, "$at\t%d\n", registrador[1]);
	fprintf(arquivo_reg, "$v0\t%d\n", registrador[2]);
	fprintf(arquivo_reg, "$v1\t%d\n", registrador[3]);
	fprintf(arquivo_reg, "$a0\t%d\n", registrador[4]);
	fprintf(arquivo_reg, "$a1\t%d\n", registrador[5]);
	fprintf(arquivo_reg, "$a2\t%d\n", registrador[6]);
	fprintf(arquivo_reg, "$a3\t%d\n", registrador[7]);
	fprintf(arquivo_reg, "$t0\t%d\n", registrador[8]);
	fprintf(arquivo_reg, "$t1\t%d\n", registrador[9]);
	fprintf(arquivo_reg, "$t2\t%d\n", registrador[10]);
	fprintf(arquivo_reg, "$t3\t%d\n", registrador[11]);
	fprintf(arquivo_reg, "$t4\t%d\n", registrador[12]);
	fprintf(arquivo_reg, "$t5\t%d\n", registrador[13]);
	fprintf(arquivo_reg, "$t6\t%d\n", registrador[14]);
	fprintf(arquivo_reg, "$t7\t%d\n", registrador[15]);
	fprintf(arquivo_reg, "$s0\t%d\n", registrador[16]);
	fprintf(arquivo_reg, "$s1\t%d\n", registrador[17]);
	fprintf(arquivo_reg, "$s2\t%d\n", registrador[18]);
	fprintf(arquivo_reg, "$s3\t%d\n", registrador[19]);
	fprintf(arquivo_reg, "$s4\t%d\n", registrador[20]);
	fprintf(arquivo_reg, "$s5\t%d\n", registrador[21]);
	fprintf(arquivo_reg, "$s6\t%d\n", registrador[22]);
	fprintf(arquivo_reg, "$s7\t%d\n", registrador[23]);
	fprintf(arquivo_reg, "$t8\t%d\n", registrador[24]);
	fprintf(arquivo_reg, "$t9\t%d\n", registrador[25]);
	fprintf(arquivo_reg, "$k0\t%d\n", registrador[26]);
	fprintf(arquivo_reg, "$k1\t%d\n", registrador[27]);
	fprintf(arquivo_reg, "$gp\t%d\n", registrador[28]);
	fprintf(arquivo_reg, "$sp\t%d\n", registrador[29]);
	fprintf(arquivo_reg, "$fp\t%d\n", registrador[30]);
	fprintf(arquivo_reg, "$ra\t%d\n", registrador[31]);

	fclose(arquivo_reg);
}
