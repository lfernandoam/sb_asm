#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//---------------- NASIR ---------------------
char sec_TEXT_DATA[2]= {'\0','\0'};
enum {TEXT, DATA};
short textStart=0;
short dataStart=0;
char MACRONAME[10];
int rotfinder, RotQuebraLinha;
int line=0,linha=-1, len=0;
char buffer[512], buffer2[512], rot[100], instruction[20], operand1[20], operand2[20], operand3[20], invalid_operand[20];
//char file_in[100],file_out[100];

typedef struct EquTable { /* A tabela para Instrução EQU */
	char symbol[100];
	char value;
} equTable;
equTable *tEqu;
int tEquSize=0;

typedef struct MacTable {   /* Tabela para armazenar os nomes de macros */
	char name[100];
} macTable;
macTable *MAC;
int tMacSize = 0;

typedef struct MacroNameTable {     /* tabela de nome de macros no prgrama */
	char name[100];
	short lineNumber;
} macroNameTable;
macroNameTable *MNT;
int tMacroSize=0;

typedef struct MacroDefTable {   /* A tabela para a defini da MACRO */
	char instro[100];
	char opera1[10];
	char opera2[10];
} macroDefTable;
macroDefTable *MDT;
int tMacroDefSize=0;


int Verificar_ENDMACRO(FILE* fileIN);
int scanner(char* buffer);
int passer(char* buffer);
int jumpline(FILE* fileIN);
int pre_processamento(FILE *fileIN, FILE *fileOUT);
int macros(FILE *fileIN, FILE *fileOUT);


int scanner(char* buffer) {
	int i=0;
	while (i<=len) {

		if (buffer[i]==';') {   /* tirar os Comentarios */
			break;
		}

		buffer[i]=toupper(buffer[i]);  /* Converter carecteres para maiscula*/
		/* verifcar os tokens */
		if(!(((buffer[i]>='A')&&(buffer[i]<='Z'))||((buffer[i]>='0')&&(buffer[i]<='9'))||(buffer[i]==';')
		        ||(buffer[i]=='+')||(buffer[i]=='-')||(buffer[i]==',')||(buffer[i]==':')||(buffer[i]==' ')
		        ||(buffer[i]=='&')||(buffer[i]==0x09)||(buffer[i]=='_')||(buffer[i]=='\0')||(buffer[i]=='\n'))) {
			printf("\nERRO LÉXICO: linha %d, Token \'%c\' inválido\n",line,buffer[i]);
			printf("\n");
			exit(0);
		}
		i++;
	}

	return 10;
}

int passer(char* buffer) {
	int i=0,j=0;
	strcpy(buffer2,"");
	rotfinder = 0;
	line = line+1;
	len = strlen(buffer);
	scanner(buffer);
	while (i<=len) {
		buffer[i]=toupper(buffer[i]);
		if((buffer[i]==0xa)||(buffer[i]==0xd)) {  /*retirar ENTER (new line e carry return)*/
			i++;
		}
		if (buffer[i]==';') {   /* tirar os Comentarios */
			break;
		}
		if(buffer[i]==':') {  /* procurar Rotulos */
			if(rotfinder==1) {
				printf("\nERRO SINTÁTICO: linha %d, Dois rótulos na mesma linha\n", line);
				printf("\n");
				exit(0);
			} else {
				rotfinder=1;
			}
		}

		buffer2[j]=buffer[i];
		i++;
		j++;
	}

	buffer2[j]=0;

	if(rotfinder) {
		sscanf(buffer2,"%s %s %s %s %s",rot,instruction,operand1,operand2,invalid_operand);
		if(strcmp(instruction,"")==0) {
			RotQuebraLinha = 1;
		} else {
			RotQuebraLinha = 0;
		}
		if(strcmp(instruction,"MACRO")==0) {
			char rot2[100];
			strncpy(rot2,rot,strlen(rot)-1);
			strcpy(MACRONAME,rot2);
			strcpy(rot,rot2);
			tMacSize++;
			MAC=realloc(MAC,(tMacSize)*sizeof(macTable));
			strncpy(MAC[tMacSize-1].name,rot,100);

		}

		if(strcmp(instruction,"EQU")==0) {
			tEquSize++;
			tEqu=realloc(tEqu,(tEquSize)*sizeof(equTable));
			strncpy(tEqu[tEquSize-1].symbol,rot,100);
			tEqu[tEquSize-1].value = operand1[0];
		}

	} else {
		sscanf(buffer2,"%s %s %s %s",instruction,operand1,operand2, invalid_operand);
		if(strcmp(instruction,MAC[tMacSize-1].name)==0) {
			tMacroSize++;
			MNT=realloc(MNT,(tMacroSize)*sizeof(macroNameTable));
			strncpy(MNT[tMacroSize-1].name,instruction,100);
			MNT[tMacroSize-1].lineNumber = (char)line;
		}

	}
	return 11;
}

void verificar_secao(char*inst, int mode) {
	switch(mode) {
	case TEXT:
		if(sec_TEXT_DATA[TEXT]) {
			if(line<textStart) {
				printf("\n\tERRO SEMÂNTICO: linha %d, Instrução \"%s\" não está na seção TEXT\n",line,inst);
				printf("\n");
				exit(0);
			} else if(sec_TEXT_DATA[DATA])
				if(line>dataStart) {
					printf("\n\tERRO SEMÂNTICO: linha %d, Instrução \"%s\" não está na seção TEXT\n",line,inst);
					printf("\n");
					exit(0);
				}
		}
		break;
	case DATA:
		if((!sec_TEXT_DATA[DATA])&&sec_TEXT_DATA[TEXT]) {
			printf("\n\tERRO SEMÂNTICO: linha %d, Diretiva \"%s\' não está na seção DATA\n",line,inst);
			printf("\n");
			exit(0);
		}
	}
}

short analisar_Instrucao(char *rot, char *inst, char *op1, char *op2, char *op3) {       /*essa funcao recebe 3 operandos para verificar um possivel operando extra*/
	int opnum=0;              /*variavel pra contar o numero de operandos da funcao*/
	if(strcmp(op3,"")!=0)
		opnum+=1;
	if(strcmp(inst,"ADD")==0) {              /*instrucoes*/
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"SUB")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"MULT")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"DIV")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"JMP")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"JMPN")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"JMPP")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"JMPZ")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"COPY")==0) {
		if(strcmp(op2,"")==0)
			opnum-=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"LOAD")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"STORE")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"INPUT")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	}

	else if(strcmp(inst,"OUTPUT")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,MACRONAME)==0) {
		if(strcmp(op1,"")!=0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"STOP")==0) {
		if(strcmp(op1,"")!=0)
			opnum+=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"IF")==0) {
		if(strcmp(op2,"")!=0)
			opnum+=1;
		if(strcmp(op1,"")==0)
			opnum-=1;
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"MACRO")==0) { // TEN QUR aceitar parametros
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"ENDMACRO")==0) { // TEN QUR aceitar parametros
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"SECTION")==0) {              /*diretivas*/
		if(strcmp(op1,"TEXT")==0) {
			sec_TEXT_DATA[TEXT]=1;
			textStart=line;
		} else if(strcmp(op1,"DATA")==0) {
			sec_TEXT_DATA[DATA]=1;
			dataStart=line;
		} else {
			printf("\n\tERRO SEMÂNTICO: linha %d, Seção \"%s\" invalida\n",line,op1);
			printf("\n");
			exit(0);
		}
	} else if(strcmp(inst,"SPACE")==0) {
		verificar_secao(inst,DATA);
	} else if(strcmp(inst,"CONST")==0) {
		verificar_secao(inst,DATA);
	} else {
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==0)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Instrução \"%s\" invalida\n",line,inst);
			printf("\n");
			exit(0);
		}
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==1)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Diretiva \"%s\" invalida\n",line,inst);
			printf("\n");
			exit(0);
		}
	}
	if(opnum>0) {
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==0)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Instrução \"%s\" com operandos a mais\n",line,inst);
			printf("\n");
			exit(0);
		}
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==1)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Diretiva \"%s\" com operandos a mais\n",line,inst);
			printf("\n");
			exit(0);
		}
	}

	else if(opnum<0) {
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==0)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Instrução \"%s\" com operandos a menos\n",line,inst);
			printf("\n");
			exit(0);
		}
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==1)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Diretiva \"%s\" com operandos a menos\n",line,inst);
			printf("\n");
			exit(0);
		}
	}

	return 20;
}

int Verificar_ENDMACRO(FILE* fileIN) {
	do {
		if(fgets(buffer2, 511, fileIN)!=NULL) {
			sscanf(buffer2, "%s %s %s",instruction,operand1,operand2);
			tMacroDefSize++;
			MDT=realloc(MDT,(tMacroDefSize)*sizeof(macroDefTable));
			strncpy(MDT[tMacroDefSize-1].instro,instruction,10);
			strncpy(MDT[tMacroDefSize-1].opera1,operand1,10);
			strncpy(MDT[tMacroDefSize-1].opera2,operand2,10);
			line++;
		}
	} while(strcmp(instruction,"ENDMACRO")!=0);
	return 20;
}

int jumpline(FILE* fileIN) {    /*Esta funcao pula a proxima linha, foi usada para IF invalido*/
	if(fgets(buffer2, 511, fileIN)!=NULL) {
		line = line+1;
	}
	return 12;
}

int pre_processamento(FILE *fileIN, FILE *fileOUT) {   /* A funcao que faz o pre_processamento de arquivo de entrada para gerar arquivo de saida processado*/
	int i=0;
	while(!feof(fileIN)) {
		strcpy(buffer,"");
		strcpy(rot,"");
		strcpy(instruction,"");
		strcpy(operand1,"");
		strcpy(operand2,"");
		strcpy(operand3,"");
		strcpy(invalid_operand,"");
		if (fgets(buffer, 511, fileIN)!=NULL) {
			passer(buffer);
			analisar_Instrucao(rot,instruction,operand1,operand2,invalid_operand);
			if(rotfinder) {
				if(strcmp(instruction,"EQU")==0) {
					continue;
				}
				if (strcmp(instruction,"MACRO")==0) {
					fprintf(fileOUT,"%s %s %s %s\n",strcat(rot,":"),instruction,operand1,operand2);
				} else {
					fprintf(fileOUT,"%s %s %s %s\n",rot,instruction,operand1,operand2);
				}
			} else {
				if(strcmp(instruction,"IF")==0) {
					for (i = 0; i < tEquSize; i++) {
						if((strncmp(operand1,tEqu[i].symbol,(strlen(tEqu[i].symbol)-1))==0)&&(tEqu[i].value=='0')) {
							jumpline(fileIN);
						}
					}
					continue;
				}
				fprintf(fileOUT,"%s %s %s\n",instruction,operand1,operand2);
			}

		}
	}

	if(!sec_TEXT_DATA[TEXT]) {
		printf("\n\tERRO SEMÂNTICO: Seção TEXT não declarada");
		printf("\n");
		exit(0);
	}
	if(!sec_TEXT_DATA[DATA]) {
		printf("\n\tERRO SEMÂNTICO: Seção DATA não declarada");
		printf("\n");
		exit(0);
	}

	return 13;
}

int macros(FILE *fileIN, FILE *fileOUT) {    /* A funcao que expande a MACRO e gerar um arquivo de saida com MACROS expandidas*/
	int i=0;
	while(!feof(fileIN)) {
		strcpy(buffer,"");
		strcpy(rot,"");
		strcpy(instruction,"");
		strcpy(operand1,"");
		strcpy(operand2,"");
		strcpy(operand3,"");
		strcpy(invalid_operand,"");
		if (fgets(buffer, 511, fileIN)!=NULL) {
			passer(buffer);
			if(rotfinder) {
				if (strcmp(instruction,"MACRO")==0) {
					Verificar_ENDMACRO(fileIN);
					continue;
				}
				if(strcmp(instruction,"MACRO")!=0) {
					linha++;
					fprintf(fileOUT,"%s %s %s %s \n",rot, instruction,operand1,operand2);
					//printf("%d  %s %s %s %s \n",linha, rot, instruction,operand1,operand2);
				}
			} else {
				/* quando a linha de chamada de macro for encontrada, expande os elementos de macro ate encontra "ENDMACRO"*/
				if(line == MNT[tMacroSize-1].lineNumber) {
					for(i=0; i<tMacroDefSize-1; i++) {
						linha++;
						if(strcmp(MDT[i].instro,"COPY")==0) {
							fprintf(fileOUT,"%s %s %s\n",MDT[i].instro,MDT[i].opera1,MDT[i].opera2);
						} else {
							fprintf(fileOUT,"%s %s\n",MDT[i].instro,MDT[i].opera1);
						}
					}
				}
				if (strcmp(instruction,MNT[tMacroSize-1].name)==0) {
					continue;
				}
				linha++;
				fprintf(fileOUT,"%s %s %s\n",instruction,operand1,operand2);
				//printf("%d   %s %s %s\n",linha, instruction,operand1,operand2);

			}
		}
	}

	return 14;
}

//---------------- NASIR (fim)---------------------

// verificar erro do tipo ADD Y+2 pro caso que Y+2 não exista
// PERGUNTAR SOBRE UTILIZAR ENDERECOS Q NAO FAZEM PARTE DA VARIAVEL SHIFTADA. EX: LOAD Y+5 sendo Y: SPACE 2
// ISTO SERIA POSSIVEL SE O CARREGADOR ALOCASSE A MEMORIA CONSECUTIVAMENTE NA MEMORIA E A PROXIMA VARIAVEL CONTENHA ESTE ENDERECO.

//ADD rotulo de linha na secao TEXT deve dar erro?
//POSSO CONSIDERAR QUE O "STOP" SEMPRE SERA A ULTIMA INSTRUCAO DA SEÇÃO TEXT? creio que nao.


//<FIM eh SECTION TEXT; =>FIM eh SECTION DATA
//OBS: NAO PODE HAVER LABEL/ROTULO NA LINHA DO SECTION DATA (L1: SECTION DATA), POIS CONSIDERO <FIM COMO SECTION TEXT
//...MAS NA VDD <FIM TBM INCLUI A LINHA DO SECTION DATA

//CORRIGIR DIRETIVA CONST ACEITAR QUALQUER COISA COMO ARGUMENTO. ALEM DISSO, DEVE ACEITAR VALORES EM HEXA.

//int endereco=0;
int offset=0;
int linham=1;
int fim=0;
int erro=0;
//FILE *fileOUT;

typedef struct SymbTable { /*Tabela de Simbolos*/
	char symbol[20]; //nome do simbolo
	char end; //endereco

	long int const_value; //valor de sua constante (-99999 se nao eh const)
	//int space_size; //tamanho do campo space (quantos enderecos esta diretiva aloca)
} SymbTable;

//typedef struct LineTable{ /*Tabela de equivalencia de linhas*/
//    int orig;
//    int macro;
//    }LineTable;
//

int getToken(char* fonte,int *pos, char* token);
int monta(char* fonte, FILE *fileOUT);
int passar_pra_string(FILE *fileIN, char* fonte, long tam_fonte);
void insertTable(SymbTable* tSymb, int *tamSymb, char* token, int endereco, int const_value);
int checkEnd(SymbTable* tSymb, int tamSymb, char* token);
void segPassagem(char* fonte, SymbTable* tSymb, int tamSymb, char* token, int endf, FILE *fileOUT);

void exibe(SymbTable *tSymb, int tamSymb) {
	int i;
	printf("\n");
	for(i=0; i<tamSymb; i++) {
		printf("tSymb[i].symbol: %s | ",tSymb[i].symbol);
		printf("tSymb[i].end: %d | ",tSymb[i].end);
		printf("tSymb[i].const_value: %ld | ",tSymb[i].const_value);
		//printf("tSymb[i].space_size: %d\n",tSymb[i].space_size);
	}
}

void insertTable(SymbTable* tSymb, int *tamSymb, char* token, int endereco, int const_value) {
	int i, existe=0;
	int tam_token=strlen(token);
	token[tam_token-1]='\0';
	for(i=0; i<*tamSymb; i++) {
		//printf("TOKEN: %s\n",token);
		if(strcmp(token,tSymb[i].symbol)==0) {
			existe=1;
			printf("ERRO SEMANTICO: Linha %d com Rotulo repetido.\n",linham); // erro semantico
			erro=1;
		}
	}
	if (!existe) {
		//printf("INSERÇÃO: %s\n", token);
		++*tamSymb;
		tSymb=realloc(tSymb,(*tamSymb)*sizeof(SymbTable));
		strcpy(tSymb[*tamSymb-1].symbol,token); //tSymb[tamSymb-1] eh a linha
		tSymb[*tamSymb-1].end=endereco;
		tSymb[*tamSymb-1].const_value=const_value;
		//tSymb[*tamSymb-1].space_size=space_size;
	}
	//exibe(tSymb,*tamSymb);
	//printf("FINAL DO BUSCA tamSymb: %d\n",*tamSymb);
}


int checkEnd(SymbTable* tSymb, int tamSymb, char* token) {
	int i;
	for(i=0; i<tamSymb; i++) {
		if(strcmp(token,tSymb[i].symbol)==0) {
			return tSymb[i].end;
		}
	}
	return -1;
}

int desloc(char* token) {
	if(offset) {
		//printf("valor de offset: %d\n",atoi(token));
		return atoi(token);
	}
	return 0;
}
// remover condicao para || fonte[j] == '\0'
int getToken(char* fonte,int *pos, char* token) {
	int i = 0, j = *pos;
	if(fonte[j] == '\n') {
		//printf("linha: %d\n",linham);
		linham++;
	}
	while(fonte[j] == '\n' || fonte[j] == '\t' || fonte[j] == ' ' || fonte[j] == '\r' || fonte[j]=='+' || fonte[j]==',') {
		j++;;
		++*pos;
	}
	while(fonte[j] != '\n' && fonte[j] != '\t' && fonte[j] != ' ' && fonte[j] != '\r' && fonte[j] != '\0') {
		token[i] = fonte[j];
		i++;
		j++;
		//printf("\nfonte[j]= %c (n-1: %c)\n",fonte[j],fonte[j-1]);
		//if((fonte[j]==':')||(fonte[j-1]==':')) break;
		if((fonte[j]=='+')) {
			offset=1;
			//printf("setei offset %c\n", token[i-1]);
			break;
		}
		if((fonte[j]==',')||(fonte[j-1]==',')) {
			//printf("virgula\n");
			//++*pos;
			break;
		}

	}
	token[i] = '\0';
	int tam=strlen(token);
	//printf("getToken %s>pos %d\n",token,*pos);
	return tam;
}

long int getConstValue(char* fonte,int j) {
	int i = 0;
	char valor[21];
	int ehconst=0;
	//printf("fonte[j]=%c",fonte[j]);
	//primeiro token
	while(fonte[j] == '\n' || fonte[j] == '\t' || fonte[j] == ' ' || fonte[j] == '\r' || fonte[j]=='+' || fonte[j]==',') {
		j++;
	}
	while(fonte[j] != '\n' && fonte[j] != '\t' && fonte[j] != ' ' && fonte[j] != '\r' && fonte[j] != '\0') {
		valor[i] = fonte[j];
		i++;
		j++;
	}
	valor[i] = '\0';
	if(strcmp(valor,"CONST")==0) {
		//printf("eh const\n");
		ehconst=1;
	}
	i=0;
	//segundo token (onde esta o numero)
	while(fonte[j] == '\n' || fonte[j] == '\t' || fonte[j] == ' ' || fonte[j] == '\r' || fonte[j] == '\0' || fonte[j]=='+' || fonte[j]==',') {
		j++;
	}
	//printf("fonte[j]=%c",fonte[j]);
	while(fonte[j] != '\n' && fonte[j] != '\t' && fonte[j] != ' ' && fonte[j] != '\r' && fonte[j] != '\0') {
		valor[i] = fonte[j];
		i++;
		j++;
	}
	valor[i] = '\0';
	if (ehconst) {
		//printf("\n\n\nvalor:%sa\n\n",valor);
		if((valor[1]=='X')||(valor[2]=='X')||(valor[1]=='x')||(valor[2]=='x')) {
			//printf("\n\getConst,strtol(valor,NULL,0): %d\n",strtol(valor,NULL,0));
			return strtol(valor,NULL,0);
		} else {
			return atoi(valor);
		}
	}
	return -99999;
}

void segPassagem(char* fonte, SymbTable* tSymb, int tamSymb, char* token, int endf, FILE *fileOUT) {
	int i=0;
	long int j=0;
	int pos=0;
	char auxtoken[20];
	int tam_fonte=strlen(fonte);
	pos+=getToken(fonte, &pos, token);
	while(pos<tam_fonte) { //NAO PODE SER -1 pois precisa executar a funcao pro ultimo token
		offset=0;
		//getchar();

		if(strcmp(token,"ADD")==0) {
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("1 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"1 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"SUB")==0) {
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("2 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"2 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"MULT")==0) {
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("3 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"3 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"DIV")==0) {
			pos+=getToken(fonte, &pos, token);

			strcpy(auxtoken,token);
			if (offset) {
				//printf("\n\ntem offset e nao eh const\n\n"); //TESTAR
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			for(i=0; i<tamSymb; i++) {
				if(strcmp(auxtoken,tSymb[i].symbol)==0) {
					if(tSymb[i].const_value==0) {
						printf("ERRO SEMANTICO: Linha %d com Divisao por Zero.\n",linham);
						erro=1;
					}
				}
			}
			//printf("4 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"4 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"JMP")==0) {
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com pulo para rotulo inválido.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim) {
				printf("ERRO SEMANTICO: Linha %d com pulo para seção INVALIDA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("5 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"5 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"JMPN")==0) {
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com pulo para rotulo inválido.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim) {
				printf("ERRO SEMANTICO: Linha %d com pulo para seção INVALIDA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("6 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"6 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"JMPP")==0) {
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com pulo para rotulo inválido.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim) {
				printf("ERRO SEMANTICO: Linha %d com pulo para seção INVALIDA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("7 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"7 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"JMPZ")==0) {
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com pulo para rotulo inválido.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim) {
				printf("ERRO SEMANTICO: Linha %d com pulo para seção INVALIDA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("8 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"8 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"COPY")==0) {
			pos+=getToken(fonte, &pos, token);

			strcpy(auxtoken,token);
			if (offset) {
				//printf("\nachei offset no 1o. arg COPY\n");
				pos+=getToken(fonte, &pos, token);
			}
			//printf("auxtoken1: %s\n",auxtoken);
			//printf("checkEnd(auxtoken)=%d\n",checkEnd(tSymb,tamSymb,auxtoken));
			//printf("token %s\n",token);
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("9 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"9 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				//printf("\nachei offset no 2o. arg COPY\n");
				pos+=getToken(fonte, &pos, token);
			}
			//printf("auxtoken2: %s\n",auxtoken);
			//printf("checkEnd(auxtoken)=%d\n",checkEnd(tSymb,tamSymb,auxtoken));
			//printf("token %s\n",token);
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			for(i=0; i<tamSymb; i++) {
				if(strcmp(auxtoken,tSymb[i].symbol)==0) {
					if(tSymb[i].const_value!=-99999) {
						printf("ERRO SEMANTICO: Linha %d com modificação de um valor constante.\n",linham);
						erro=1;
					}
				}
			}
			//printf("%d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"%d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"LOAD")==0) {
			pos+=getToken(fonte, &pos, token);

			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com LOAD com argumentos definidos fora da SECTION DATA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("10 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"10 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"STORE")==0) {
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com STORE com argumentos definidos fora da SECTION DATA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("\n\n check: %d\n",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			for(i=0; i<tamSymb; i++) {
				if(strcmp(auxtoken,tSymb[i].symbol)==0) {
					if(tSymb[i].const_value!=-99999) {
						printf("ERRO SEMANTICO: Linha %d com modificação de um valor constante.\n",linham);
						erro=1;
					}
				}
			}
			//printf("11 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"11 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"INPUT")==0) {
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com INPUT com argumentos definidos fora da SECTION DATA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("12 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"12 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			//printf("checkEnd = %d\n",checkEnd(tSymb,tamSymb,auxtoken));
		} else if(strcmp(token,"OUTPUT")==0) {
			pos+=getToken(fonte, &pos, token);

			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com OUTPUT com argumentos definidos fora da SECTION DATA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linham);
				erro=1;
			}
			//printf("13 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"13 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"STOP")==0) {
			//printf("14 ");
			fprintf(fileOUT,"14 ");
			pos+=getToken(fonte, &pos, token);
		} else if(strcmp(token,"SPACE")==0) {
			//aux=pos;
			//printf("linha do space %d\n",linham);

			//VERIFICAR SE VAI DAR A LINHA CORRETA SE TIVER ERRO NA PARTE DOS SPACE
			pos+=getToken(fonte, &pos, token);
			j=atoi(token); //ou utilizar a ultima coluna da tabela de simbolos
			if (j==0) {
				j=1;
				//pos=aux;
			}
			for(i=1; i<=j; i++) {
				//printf("0 ");
				fprintf(fileOUT,"0 ");
			}
		} else if(strcmp(token,"CONST")==0) {
			pos+=getToken(fonte, &pos, token);
			if((token[1]=='X')||(token[2]=='X')||(token[1]=='x')||(token[2]=='x')) { // se for negativo, a posicao 2 conterá o X
				j=strtol(token,NULL,0);
				//printf("\n\strtol(token,NULL,0): %d\n",strtol(token,NULL,0));
			} else {
				j=atoi(token);
				//printf ("valor const: %ld.\n",j);
			}
			fprintf(fileOUT,"%ld ",j);
		} else {
			pos+=getToken(fonte, &pos, token);
		}
	}
}


int monta(char* fonte, FILE *fileOUT) {
	int tamSymb=0;
	SymbTable *tSymb = (SymbTable*) malloc(sizeof(SymbTable));
	char token[21];
	char auxtoken[21];
	int endereco=0;
	//int aux;
	long int const_value=-99999; //j eh diferente de -99999 sempre que eh const
	int space_size=0;
	int pos=0;
	int tam_fonte=strlen(fonte);
	int tam_token;
	//printf("monta, fonte:\n%s\n", fonte);

	pos+=getToken(fonte, &pos, token);
	//printf("token %s  pos: %d \n\n",token,pos);
	//printf("tam_fonte: %d\n",tam_fonte);
	//printf("tam_token: %d\n",tam_token);

	//printf("pos: %d\n",pos);

	while(pos<tam_fonte-1) {
		//printf("\npos: %d\n",pos);
		//getchar();
		//tam_token=strlen(token);
		if(strcmp(token,"ADD")==0) {
			endereco+=2;
		} else if(strcmp(token,"SUB")==0) {
			endereco+=2;
		} else if(strcmp(token,"MULT")==0) {
			endereco+=2;
		} else if(strcmp(token,"DIV")==0) {
			endereco+=2;
		} else if(strcmp(token,"JMP")==0) {
			endereco+=2;
		} else if(strcmp(token,"JMPN")==0) {
			endereco+=2;
		} else if(strcmp(token,"JMPP")==0) {
			endereco+=2;
		} else if(strcmp(token,"JMPZ")==0) {
			endereco+=2;
		} else if(strcmp(token,"COPY")==0) {
			endereco+=3; // CUIDADO!
		} else if(strcmp(token,"LOAD")==0) {
			endereco+=2;
		} else if(strcmp(token,"STORE")==0) {
			endereco+=2;
		} else if(strcmp(token,"INPUT")==0) {
			endereco+=2;
		} else if(strcmp(token,"OUTPUT")==0) {
			endereco+=2;
		} else if(strcmp(token,"STOP")==0) {
			endereco+=1;
		} else if(strcmp(token,"SPACE")==0) {
			//aux=pos;
			pos+=getToken(fonte, &pos, token);
			space_size=atoi(token);
			if (space_size==0) { //caso nao tenha nada depois de SPACE
				space_size=1;
				//pos=aux;
			}
			//printf("j: %d\n",j);
			endereco+=space_size;
			//printf("         space_size: %d\n\n\n",space_size);
		} else if(strcmp(token,"CONST")==0) {
			endereco+=1;
			pos+=getToken(fonte, &pos, token);
		}
		tam_token=strlen(token);

		if (token[tam_token-1]==':') { // se eh rotulo
			const_value=getConstValue(fonte,pos);
			//printf("\nMONTA,const_value: %ld\n",const_value);
			insertTable(tSymb, &tamSymb, token, endereco, const_value);
		}
		const_value=-99999; //const_value eh -99999 sempre que nao eh const
		space_size=0;

		strcpy(auxtoken,token);
		strcat(auxtoken," ");

		pos+=getToken(fonte, &pos, token);
		if(strcmp(strcat(auxtoken,token),"SECTION DATA")==0) {
			fim=endereco;
		}
		//printf("..pos %d\n",pos);
	}
	//endereco--; //a ultima instrucao sempre soma 1 endereco (SPACE ou CONST) pra compatibilizar com a linha seguinte
	linham=1;
	//printf("O ultimo enderereco eh: %d\n",endereco);
	//printf("O SECTION DATA encontra-se a partir do endereco %d",fim);
	//printf("\n\nSeg Passagem\n");
	segPassagem(fonte, tSymb, tamSymb, token, endereco, fileOUT);
	return 1;
}


//veficar o que ocorre se for ADD ADD (label for reservado com nome ADD)


int passar_pra_string(FILE *fileIN, char* fonte, long tam_fonte) {
	if (!fonte) return 0; //se deu erro de memoria
	fread(fonte, sizeof(char), tam_fonte, fileIN);
	return 1;
}

//int main(int argc, char* argv[]) {
//	char* fonte="";
//	if (argc<4) { //depois aumentaremos para receber mais parametros conforme especificacao
//		printf("Faltaram argumentos.\n");
//		return 0;
//	}
//	char* arquivo_mcr=strcat(argv[2],".mcr"); //a especificacao pede pra nao inserir a extensao junto
//
//	printf("<DEBUG> arquivo de entrada: %s\n", arquivo_mcr);
//
//	FILE *fileIN;
//	//---------------------------------------------------------------
//	// o arquivo de saida declarei como variavel global
//
//	fp_mcr = fopen(arquivo_mcr, "r");
//	if (!fp_mcr) printf("Arquivo de entrada invalido.\n"); //fp_mcr == NULL
//	else {
//		fseek(fp_mcr, 0L, SEEK_END); //posiona o fp_mcr no final do arquivo
//		long tam_fonte = ftell(fp_mcr); //pega o tamanho
//		fseek(fp_mcr, 0L, SEEK_SET); //posiona o fp_mcr no inicio do arquivo (mesmo que rewind(fp_mcr), mas com retorno)
//		fonte = (char*) calloc(tam_fonte,sizeof(char)); //alocacao dinamica
//		if(!passar_pra_string(fp_mcr, fonte, tam_fonte)) { //problema na alocacao dinamica (memoria)
//			printf("Erro ao passar pra string!\n");
//			return 0;
//		}
//		fclose(fp_mcr);
//		printf("%s\n", fonte);
//		printf("Relatorio da montagem:\n\n");
//		char* arquivo_o=strcat(argv[3],".o");
//		//printf("<DEBUG> arquivo de saida: %s\n", arquivo_o);
//		fp_o=fopen(arquivo_o,"w");
//		if(!fp_o) printf("Erro ao criar o arquivo.");
//		else {
//			monta(fonte);
//			if(!erro) {
//				printf("Arquivo montado com sucesso!");
//			} else {
//				remove(arquivo_o);
//			}
//		}
//		fclose(fp_o);
//	}
//}

int main (int argc, char* argv[]) {
	char *argument=(char*)malloc(3*sizeof(char));    /*alocar espaco de memoria para -m ou -p ou -o*/
	char file_in[50];   /*alocar espaco de memoria para o arquivo de entrada*/
	char file_out[50];  /*alocar espaco de memoria para para o arquivo de saida*/
	char aux_in[50],aux_out[50];
	FILE *fileIN, *fileOUT;

	if(argc<4) {      /* se os parametros não foram passados corretamente, insera os parametros corretos */
		printf("\nParametros invalidos, digite o argumento de modo de Operação: ");
		scanf("%s", argument);
		getchar();
		printf("\nDigite o nome do arquivo de entrada: ");
		scanf("%s",file_in);
		getchar();
		printf("\nDigite o nome do arquivo de saida: ");
		scanf("%s",file_out);
		getchar();
	} else {   /* se os parametros foram passados corretamente, coloca na variavel correta */
		argument=argv[1];
		strcpy(file_in,argv[2]);
        strcpy(file_out,argv[3]);
	}

	/* ###############      alocar espaco de memoria para as tabelas       ################### */
	tEqu=(equTable*)malloc(sizeof(equTable));
	MNT=(macroNameTable*)malloc(sizeof(macroNameTable));
	MDT=(macroDefTable*)malloc(sizeof(macroDefTable));
	MAC=(macTable*)malloc(sizeof(macTable));

    strcpy(aux_in,file_in);
    strcpy(aux_out,file_out); //para o arquivo final nao ter 'dupla extensão' como .pre.mcr.o

	/* PREPROCESSAMENTO, Avaliacao de EQU e IF */
	if (strcmp(argument,"-p")==0) {
		if((fileIN=fopen(strcat(file_in,".asm"),"r"))==NULL) {
            printf("<DEBUG> arquivo de entrada: %s\n", file_in);
			printf("\n\n\t\tERRO! Arquivo não encontrado.");
			return 0;
		}
		if((fileOUT=fopen(strcat(file_out,".pre"),"w"))==NULL) {   /* abre o arquivo de saida */
            printf("<DEBUG> arquivo de saida: %s\n", file_out);
			printf("\n\n\t\tERRO! não foi possivel criar arquivo.");
			return 0;
		}
		pre_processamento(fileIN,fileOUT);
		printf("Voce está no modo de Operação PREPROCESSAMENTO\n");
		printf("O arquivo de entrada sera preprocessado.\n");

	} else if(strcmp(argument,"-m")==0) { /* PROCESSAMENTO DE MACROS, Avaliacao de EQU e IF e Substituicao de MACROS */

		if((fileIN=fopen(strcat(file_in,".asm"),"r"))==NULL) {
            printf("<DEBUG> arquivo .asm de entrada: %s\n", file_in);
			printf("\n\n\t\tERRO! Arquivo não encontrado.");
			return 0;
		}
		if((fileOUT=fopen(strcat(file_out,".pre"),"w"))==NULL) {   /* abre o arquivo de saida */
            printf("<DEBUG> arquivo .pre de saida: %s\n", file_out);
			printf("\n\n\t\tERRO! não foi possivel criar arquivo.");
			return 0;
		}
		pre_processamento(fileIN,fileOUT);

        fclose(fileIN);
        fclose(fileOUT);

		//printf("<DEBUG> arquivo .pre de entrada: %s\n", file_out);
		if((fileIN=fopen(file_out,"r"))==NULL) {
            //printf("<DEBUG> arquivo .pre de entrada: %s\n", file_out);
			printf("\n\n\t\tERRO! Arquivo não encontrado.");
			return 0;
		}
		strcat(aux_out,".mcr");
		//printf("<DEBUG> arquivo de saida: %s\n", aux_out);
		if((fileOUT=fopen(aux_out,"w"))==NULL) {
            printf("<DEBUG> arquivo de .mcr saida: %s\n", aux_out);
			printf("\n\n\t\tERRO! não foi possivel criar arquivo.");
			return 0;
		}
		macros(fileIN,fileOUT);
		printf("\n\tVoce está no modo de Operação EXPANCAO DE MACROS\n");
		printf("\n\tO arquivo de saida encontrase com as MACROS expandidas\n");

		// LUIS
	} else if(strcmp(argument,"-o")==0) { // MONTAGEM
        strcpy(file_in,strcat(file_in,".mcr"));
        //printf("<DEBUG> arquivo de entrada: %s\n", file_in);
		if((fileIN=fopen(file_in,"r"))==NULL) {
			printf("\n\n\t\tERRO! Arquivo não encontrado\n\n");
			return 0;
		} else {
			char* fonte=""; // mudar isso depois pra ficar na mesma linha do calloc
            fseek(fileIN, 0L, SEEK_END); //posiona o fileIN no final do arquivo
            long tam_fonte = ftell(fileIN); //pega o tamanho
            fseek(fileIN, 0L, SEEK_SET); //posiona o fileIN no inicio do arquivo (mesmo que rewind(fileIN), mas com retorno)
            fonte = (char*) calloc(tam_fonte,sizeof(char)); //alocacao dinamica
            if(!passar_pra_string(fileIN, fonte, tam_fonte)) { //problema na alocacao dinamica (memoria)
                printf("Erro ao passar pra string!\n");
                return 0;
            }
            printf("%s\n", fonte);
            printf("Relatorio da montagem:\n\n");
            strcpy(file_out,strcat(file_out,".o"));
            fileOUT=fopen(file_out,"w");
            //printf("<DEBUG> arquivo de saida: %s\n", file_out);
            if(!fileOUT) printf("Erro ao criar o arquivo.");
            else {
                monta(fonte,fileOUT);
                if(!erro) {
                    printf("Arquivo montado com sucesso!");
                } else {
                    remove(file_out); //apaga o arquivo de montagem se tiver dado erro. "Só monta se nao tiver erro"
                }
            }
        }
	}

	fclose(fileIN);
	fclose(fileOUT);
	free(tEqu);
	free(MNT);
	free(MDT);
	free(MAC);

	return 1;
}

