#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char sec_TEXT_DATA[2]= {'\0','\0'};
enum {TEXT, DATA};
short textStart=0;
short dataStart=0;
char MACRONAME[10];
int rotfinder, RotQuebraLinha;
int line,len;
int erro=0;
char buffer[512], buffer2[512], rot[100], instruction[20], operand1[20], invalid_operand[20];

int offset=0;
int linham=1; // contador de linhas utilizada na funcao de montagem
int fim=0;

//typedef struct line_fonte{  /* linhas na saida preprocessado correspondente ao fonte */
//	int value;
//}line_fonte;
//line_fonte *tline;
//int tlineSize=0;

typedef struct line_macro{  /* linha na saida macros correspondente ao fonte */
	int value;
}line_macro;
line_macro *tlmac;
int tlinemacSize=0;

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
} macroDefTable;
macroDefTable *MDT;
int tMacroDefSize=0;


typedef struct SymbTable { // Tabela de Simbolos
	char symbol[20]; // nome do simbolo
	char end; // endereco
	long int const_value; //valor de sua constante (-99999 se nao eh const)
} SymbTable;

int Verificar_ENDMACRO(FILE* fileIN);
int scanner(char* buffer);
int passer(char* buffer);
void verificar_secao(char*inst, int mode);
int analisar_Instrucao(char *rot, char *inst, char *op1, char *op2);
int jumpline(FILE* fileIN);
int pre_processamento(FILE *fileIN, FILE *fileOUT);
int macros(FILE *fileIN, FILE *fileOUT);
int findrot_macro(char* buffer);

int getToken(char* fonte,int *pos, char* token);
int monta(char* fonte, FILE *fileOUT);
long int getConstValue(char* fonte,int j);
int desloc(char* token);
int passar_pra_string(FILE *fileIN, char* fonte, long tam_fonte);
void insertTable(SymbTable* tSymb, int *tamSymb, char* token, int endereco, int const_value);
void exibe(SymbTable *tSymb, int tamSymb);
int checkEnd(SymbTable* tSymb, int tamSymb, char* token);
void segPassagem(char* fonte, SymbTable* tSymb, int tamSymb, char* token, int endf, FILE *fileOUT);

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
			printf("\n\tERRO LÉXICO: linha %d, Token \'%c\' invalido\n",line,buffer[i]);
			erro=1;
		}
		i++;
	}

	return 1;
}

int findrot_macro(char* buffer) {
	int i=0,j=0;
	strcpy(buffer2,"");
	rotfinder = 0;
	line = line+1;
	len = strlen(buffer);
	while (i<=len) {
		buffer[i]=toupper(buffer[i]);
		if(buffer[i]==':') {  /* procurar Rotulos */
			if(rotfinder==1) {
				printf("\n\tERRO SINTÁTICO: linha %d, Dois rótulos na mesma linha\n", line);
				erro=1;
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
		sscanf(buffer2,"%s %s %s %s",rot,instruction,operand1,invalid_operand);

		if(strcmp(instruction,"MACRO")==0) {
			char rot2[100];
			strncpy(rot2,rot,strlen(rot)-1);
			rot2[strlen(rot2)-1]='\0';
			strcpy(rot,rot2);
			tMacSize++;
			MAC=realloc(MAC,(tMacSize)*sizeof(macTable));
			strncpy(MAC[tMacSize-1].name,rot,100);

		}

	} else {
		sscanf(buffer2,"%s %s %s",instruction,operand1, invalid_operand);
		if(strcmp(instruction,MAC[tMacSize-1].name)==0) {
			tMacroSize++;
			MNT=realloc(MNT,(tMacroSize)*sizeof(macroNameTable));
			strncpy(MNT[tMacroSize-1].name,instruction,100);
			MNT[tMacroSize-1].lineNumber = (char)line;
		}

	}
	return 1;
}

int passer(char* buffer) {
	int i=0,j=0;
	strcpy(buffer2,"");
	rotfinder=0;
	RotQuebraLinha=0;
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
				printf("\n\tERRO SINTÁTICO: linha %d, Dois rótulos na mesma linha\n", line);
				erro=1;
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
		sscanf(buffer2,"%s %s %s %s",rot,instruction,operand1,invalid_operand);
		if(strcmp(instruction,"")==0) {
			RotQuebraLinha = 1;
		} else {
			RotQuebraLinha = 0;
		}

		if(strcmp(instruction,"EQU")==0) {
			tEquSize++;
			tEqu=realloc(tEqu,(tEquSize)*sizeof(equTable));
			strncpy(tEqu[tEquSize-1].symbol,rot,100);
			tEqu[tEquSize-1].value = operand1[0];
		}

		if(strcmp(instruction,"MACRO")==0) {
			char rot2[100];
			strncpy(rot2,rot,strlen(rot)-1);
			rot2[strlen(rot2)-1]='\0';
			strcpy(MACRONAME,rot2);
		}

	} else {
		sscanf(buffer2,"%s %s %s",instruction,operand1, invalid_operand);
	}
	return 1;
}

void verificar_secao(char*inst, int mode) {
	switch(mode) {
	case TEXT:
		if(sec_TEXT_DATA[TEXT]) {
			if(line<textStart) {
				printf("\n\tERRO SEMÂNTICO: linha %d, Instrução \"%s\" não está na secao TEXT\n",line,inst);
				erro=1;
			} else if(sec_TEXT_DATA[DATA])
				if(line>dataStart) {
					printf("\n\tERRO SEMÂNTICO: linha %d, Instrução \"%s\" não está na secao TEXT\n",line,inst);
					erro=1;
				}
		}
		break;
	case DATA:
		if((!sec_TEXT_DATA[DATA])&&sec_TEXT_DATA[TEXT]) {
			printf("\n\tERRO SEMÂNTICO: linha %d, Diretiva \"%s\' não está na secao DATA\n",line,inst);
			erro=1;
		}
	}
}

int analisar_Instrucao(char *rot, char *inst, char *op1, char *op2){ /*essa funcao recebe 3 operandos para verificar um possivel operando extra*/
	int opnum=0;     /*variavel pra contar o numero de operandos da funcao*/
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
		if(strcmp(op2,"")!=0)
			opnum+=1;
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
	} else if(strcmp(inst,"MACRO")==0) {
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"ENDMACRO")==0) {
		verificar_secao(inst,TEXT);
	} else if(strcmp(inst,"SECTION")==0) {  /*diretivas*/
		if(strcmp(op1,"TEXT")==0) {
			sec_TEXT_DATA[TEXT]=1;
			textStart=line;
		} else if(strcmp(op1,"DATA")==0) {
			sec_TEXT_DATA[DATA]=1;
			dataStart=line;
		} else {
			printf("\n\tERRO SEMÂNTICO: linha %d, secao \"%s\" invalida\n",line,op1);
			erro=1;
		}
	} else if(strcmp(inst,"SPACE")==0) {
		verificar_secao(inst,DATA);
	} else if(strcmp(inst,"CONST")==0) {
		verificar_secao(inst,DATA);
	} else {
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==0)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Instrução \"%s\" invalida\n",line,inst);
			erro=1;
		}
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==1)){
			printf("\n\tERRO SINTÁTICO: linha %d, Diretiva \"%s\" invalida\n",line,inst);
			erro=1;
		}
		if ((sec_TEXT_DATA[TEXT]==0)&&(strcmp(instruction,"EQU")!=0)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Diretiva \"%s\" invalida\n",line,inst);
			erro=1;
		}
	}
	if(opnum>0) {
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==0)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Instrução \"%s\" com operandos a mais\n",line,inst);
			erro=1;
		}
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==1)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Diretiva \"%s\" com operandos a mais\n",line,inst);
            erro=1;
		}
	}

	else if(opnum<0) {
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==0)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Instrução \"%s\" com operandos a menos\n",line,inst);
			erro=1;
		}
		if ((sec_TEXT_DATA[TEXT]==1)&&(sec_TEXT_DATA[DATA]==1)) {
			printf("\n\tERRO SINTÁTICO: linha %d, Diretiva \"%s\" com operandos a menos\n",line,inst);
			erro=1;
		}
	}

	return 1;
}

int Verificar_ENDMACRO(FILE* fileIN) {
	do {
		if(fgets(buffer2, 511, fileIN)!=NULL) {
			sscanf(buffer2, "%s %s",instruction,operand1);
			tMacroDefSize++;
			MDT=realloc(MDT,(tMacroDefSize)*sizeof(macroDefTable));
			strncpy(MDT[tMacroDefSize-1].instro,instruction,10);
			strncpy(MDT[tMacroDefSize-1].opera1,operand1,10);
			line++;
		}
	} while(strcmp(instruction,"ENDMACRO")!=0);
	return 1;
}

int jumpline(FILE* fileIN) {    /*Esta funcao pula a proxima linha, foi usada para IF invalido*/
	if(fgets(buffer2, 511, fileIN)!=NULL) {
		line = line+1;
	}
	return 1;
}

int pre_processamento(FILE *fileIN, FILE *fileOUT) {   /* A funcao que faz o pre_processamento de arquivo de entrada para gerar arquivo de saida processado*/
	int i=0;
	line=0;
	while(!feof(fileIN)) {
		strcpy(buffer,"");
		strcpy(rot,"");
		strcpy(instruction,"");
		strcpy(operand1,"");
		strcpy(invalid_operand,"");
		if (fgets(buffer, 511, fileIN)!=NULL) {
			passer(buffer);
			analisar_Instrucao(rot,instruction,operand1,invalid_operand);
			if(rotfinder) {
				if(strcmp(instruction,"EQU")==0) {
					continue;
				} else {
					//tlineSize++;
					//tline=realloc(tline,(tlineSize)*sizeof(line_fonte));
					//tline[tlineSize-1].value = line;
					fprintf(fileOUT,"%s %s %s\n",rot,instruction,operand1);
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
				//tlineSize++;
				//tline=realloc(tline,(tlineSize)*sizeof(line_fonte));
				//tline[tlineSize-1].value = line;
				fprintf(fileOUT,"%s %s\n",instruction,operand1);
			}

		}
	}

	if(!sec_TEXT_DATA[TEXT]) {
		printf("\n\tERRO SEMÂNTICO: secao TEXT não declarada");
		erro=1;
	}
	if(!sec_TEXT_DATA[DATA]) {
		printf("\n\tERRO SEMÂNTICO: secao DATA não declarada");
		erro=1;
	}
	  return 1;
  }

int macros(FILE *fileIN, FILE *fileOUT) { /* A funcao que expande a MACRO e gerar um arquivo de saida com MACROS expandidas*/
	int i=0;
	line=0;
	while(!feof(fileIN)) {
		strcpy(buffer,"");
		strcpy(rot,"");
		strcpy(instruction,"");
		strcpy(operand1,"");
		strcpy(invalid_operand,"");
		if (fgets(buffer, 511, fileIN)!=NULL) {
			findrot_macro(buffer);
			if(rotfinder) {
						if (strcmp(instruction,"MACRO")==0) {
							Verificar_ENDMACRO(fileIN);
							continue;
						}else{
							fprintf(fileOUT,"%s %s %s\n",rot, instruction,operand1);
						}
			} else {
				/* quando a linha de chamada de macro for encontrada, expande os elementos de macro ate encontra "ENDMACRO"*/
					if(line == MNT[tMacroSize-1].lineNumber) {
						for(i=0; i<tMacroDefSize-1; i++) {
								fprintf(fileOUT,"%s %s\n",MDT[i].instro,MDT[i].opera1);
							}
						}
					if (strcmp(instruction,MNT[tMacroSize-1].name)==0) {
					continue;
					}
				fprintf(fileOUT,"%s %s\n",instruction,operand1);
			}
		}
	}
	return 1;
}

//<FIM eh SECTION TEXT; =>FIM eh SECTION DATA
//OBS: NAO PODE HAVER LABEL/ROTULO NA LINHA DO SECTION DATA (L1: SECTION DATA), POIS CONSIDERO <FIM COMO SECTION TEXT
//...MAS NA VDD <FIM TBM INCLUI A LINHA DO SECTION DATA

//CORRIGIR DIRETIVA CONST ACEITAR QUALQUER COISA COMO ARGUMENTO. ALEM DISSO, DEVE ACEITAR VALORES EM HEXA.

//typedef struct LineTable{ /*Tabela de equivalencia de linhas*/
//    int orig;
//    int macro;
//    }LineTable;
//

//LEMBRAR DE PASSAR O PARAMETRO PRA SO PRINTAR AS COISAS DO PRE E MACROS SE FOR -o
void exibe(SymbTable *tSymb, int tamSymb) { // Função utilizada apenas para Debug.
	int i;
	printf("\n");
	for(i=0; i<tamSymb; i++) {
		printf("tSymb[i].symbol: %s | ",tSymb[i].symbol);
		printf("tSymb[i].end: %d | ",tSymb[i].end);
		printf("tSymb[i].const_value: %ld | ",tSymb[i].const_value);
	}
}

void insertTable(SymbTable* tSymb, int *tamSymb, char* token, int endereco, int const_value) {
	int i, existe=0;
	int tam_token=strlen(token);
	token[tam_token-1]='\0';
	for(i=0; i<*tamSymb; i++) {
		if(strcmp(token,tSymb[i].symbol)==0) {
			existe=1;
			printf("ERRO SEMANTICO: Linha %d com Rotulo repetido.\n",linham);
			erro=1;
		}
	}
	if (!existe) {
		//printf("<DEBUG> INSERÇÃO: %s\n", token);
		++*tamSymb;
		tSymb=realloc(tSymb,(*tamSymb)*sizeof(SymbTable));
		strcpy(tSymb[*tamSymb-1].symbol,token); //tSymb[tamSymb-1] eh a linha
		tSymb[*tamSymb-1].end=endereco;
		tSymb[*tamSymb-1].const_value=const_value;
	}
	//exibe(tSymb,*tamSymb);
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
		return atoi(token);
	}
	return 0;
}

int getToken(char* fonte,int *pos, char* token) {
	int i = 0, j = *pos;
	if(fonte[j] == '\n') {
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
		if((fonte[j]=='+')) {
			offset=1;
			break;
		}
		if((fonte[j]==',')||(fonte[j-1]==',')) {
			break;
		}

	}
	token[i] = '\0';
	int tam=strlen(token);
	return tam;
}

long int getConstValue(char* fonte,int j) {
	int i = 0;
	char valor[21];
	int ehconst=0;

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
		ehconst=1;
	}
	i=0;

	//segundo token (onde esta o numero)
	while(fonte[j] == '\n' || fonte[j] == '\t' || fonte[j] == ' ' || fonte[j] == '\r' || fonte[j] == '\0' || fonte[j]=='+' || fonte[j]==',') {
		j++;
	}
	while(fonte[j] != '\n' && fonte[j] != '\t' && fonte[j] != ' ' && fonte[j] != '\r' && fonte[j] != '\0') {
		valor[i] = fonte[j];
		i++;
		j++;
	}
	valor[i] = '\0';
	if (ehconst) {
		if((valor[1]=='X')||(valor[2]=='X')||(valor[1]=='x')||(valor[2]=='x')) {
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
	while(pos<tam_fonte) { //NAO PODE SER tam_fonte -1 pois precisa executar a funcao pro ultimo token
		offset=0;
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
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da secao TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
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
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da secao TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
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
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da secao TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
				erro=1;
			}
			//printf("3 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"3 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"DIV")==0) {
			pos+=getToken(fonte, &pos, token);

			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da secao TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
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
				printf("ERRO SEMANTICO: Linha %d com pulo para rotulo invalido.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim) {
				printf("ERRO SEMANTICO: Linha %d com pulo para secao INVALIDA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
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
				printf("ERRO SEMANTICO: Linha %d com pulo para rotulo invalido.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim) {
				printf("ERRO SEMANTICO: Linha %d com pulo para secao INVALIDA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
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
				printf("ERRO SEMANTICO: Linha %d com pulo para rotulo invalido.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim) {
				printf("ERRO SEMANTICO: Linha %d com pulo para secao INVALIDA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
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
				printf("ERRO SEMANTICO: Linha %d com pulo para rotulo invalido.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim) {
				printf("ERRO SEMANTICO: Linha %d com pulo para secao INVALIDA.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
				erro=1;
			}
			//printf("8 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"8 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"COPY")==0) {
			pos+=getToken(fonte, &pos, token);

			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da secao TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
				erro=1;
			}
			//printf("9 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"9 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			pos+=getToken(fonte, &pos, token);
			strcpy(auxtoken,token);
			if (offset) {
				pos+=getToken(fonte, &pos, token);
			}
			if(checkEnd(tSymb,tamSymb,auxtoken)==-1) {
				printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken)<fim) { //FIM eh o endereco que começa os rotulos de "SECTION DATA"
				printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da secao TEXT.\n",linham);
				erro=1;
			} else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf) {
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
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
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
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
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
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
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
				erro=1;
			}
			//printf("12 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"12 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
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
				printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereco invalido.\n",linham);
				erro=1;
			}
			//printf("13 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
			fprintf(fileOUT,"13 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
		} else if(strcmp(token,"STOP")==0) {
			//printf("14 ");
			fprintf(fileOUT,"14 ");
			pos+=getToken(fonte, &pos, token);
		} else if(strcmp(token,"SPACE")==0) {

			//VERIFICAR SE VAI DAR A LINHA CORRETA SE TIVER ERRO NA PARTE DOS SPACE
			pos+=getToken(fonte, &pos, token);
			j=atoi(token);
			if (j==0) {
				j=1;
			}
			for(i=1; i<=j; i++) {
				//printf("0 ");
				fprintf(fileOUT,"0 ");
			}
		} else if(strcmp(token,"CONST")==0) {
			pos+=getToken(fonte, &pos, token);
			if((token[1]=='X')||(token[2]=='X')||(token[1]=='x')||(token[2]=='x')) { // se for negativo, a posicao 2 conterá o X
				j=strtol(token,NULL,0);
			} else {
				j=atoi(token);
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
	long int const_value=-99999; //j eh diferente de -99999 sempre que eh const
	int space_size=0;
	int pos=0;
	int tam_fonte=strlen(fonte);
	int tam_token;

	pos+=getToken(fonte, &pos, token);

	while(pos<tam_fonte-1) {
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
			endereco+=3;
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
			pos+=getToken(fonte, &pos, token);
			space_size=atoi(token);
			if (space_size==0) { //caso nao tenha nada depois de SPACE
				space_size=1;
			}
			endereco+=space_size;
		} else if(strcmp(token,"CONST")==0) {
			endereco+=1;
			pos+=getToken(fonte, &pos, token);
		}
		tam_token=strlen(token);

		if (token[tam_token-1]==':') { // se eh rotulo
			const_value=getConstValue(fonte,pos);
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
	}
	linham=1;
	segPassagem(fonte, tSymb, tamSymb, token, endereco, fileOUT);
	free(tSymb);
	return 1;
}


//veficar o que ocorre se for ADD ADD (label for reservado com nome ADD)

int passar_pra_string(FILE *fileIN, char* fonte, long tam_fonte) { // passa todo o codigo para uma string
	if (!fonte) return 0; //se deu erro de memoria
	fread(fonte, sizeof(char), tam_fonte, fileIN);
	return 1;
}

int main (int argc, char* argv[]) {
    int releitura=0;
	char argument[3];
	char file_in[50];
	char file_out[50];
	char aux_in[50],aux_out[50],aux_out2[50];
	FILE *fileIN, *fileOUT;
	if((strlen(argv[2])>50)||(strlen(argv[3])>50)||(argc<4)||((strcmp(argv[1],"-p")!=0)&&(strcmp(argv[1],"-m")!=0)&&(strcmp(argv[1],"-o")!=0))){ // se os parametros não foram passados corretamente
		printf("\nParametros invalidos.");
		do{
            printf("\nDigite o argumento de modo de Operação: ");
            scanf("%s", argument);
            getchar();
        } while( (strcmp(argument,"-p")!=0) && (strcmp(argument,"-m")!=0) && (strcmp(argument,"-o")!=0) );
		do{
            printf("\nDigite o nome do arquivo de entrada: ");
            scanf("%s",file_in);
            getchar();
        } while(strlen(file_in)>50);
		do{
            printf("\nDigite o nome do arquivo de saida: ");
            scanf("%s",file_out);
            getchar();
		}while(strlen(file_out)>50);
		releitura=1;
	}
	if (!releitura){   /* se os parametros foram passados corretamente, coloca na variavel correta */
		strcpy(argument,argv[1]);
		strcpy(file_in,argv[2]);
        strcpy(file_out,argv[3]);
	}

	/* ###############      alocar espaco de memoria para as tabelas       ################### */
	tEqu=(equTable*)malloc(sizeof(equTable));
	MNT=(macroNameTable*)malloc(sizeof(macroNameTable));
	MDT=(macroDefTable*)malloc(sizeof(macroDefTable));
	MAC=(macTable*)malloc(sizeof(macTable));

    strcpy(aux_in,file_in);
    strcpy(aux_out,file_out); // para o arquivo final nao ter 'dupla extensão' como .pre.mcr
    strcpy(aux_out2,file_out); // para o arquivo final nao ter 'tripla extensão' como .pre.mcr.o

	if (strcmp(argument,"-p")==0) { // PREPROCESSAMENTO, Avaliacao de EQU e IF
		if((fileIN=fopen(strcat(file_in,".asm"),"r"))==NULL) {
			printf("\n\nERRO! Arquivo não encontrado.\n");
			return 0;
		}
		if((fileOUT=fopen(strcat(file_out,".pre"),"w"))==NULL) {   // abre o arquivo de saida
			printf("\n\nERRO! não foi possivel criar arquivo.\n");
			return 0;
		}
		pre_processamento(fileIN,fileOUT);
        printf("\nPre-processamento realizado com sucesso!\n");

	} else if(strcmp(argument,"-m")==0) { // PROCESSAMENTO DE MACROS, Avaliacao de EQU e IF e Substituicao de MACROS

		if((fileIN=fopen(strcat(file_in,".asm"),"r"))==NULL) {
			printf("\n\nERRO! Arquivo não encontrado.\n");
			return 0;
		}
		if((fileOUT=fopen(strcat(file_out,".pre"),"w"))==NULL) {
			printf("\n\nERRO! não foi possivel criar arquivo.\n");
			return 0;
		}
		pre_processamento(fileIN,fileOUT);

        fclose(fileIN);
        fclose(fileOUT);

		if((fileIN=fopen(file_out,"r"))==NULL) {
			printf("\n\nERRO! Arquivo não encontrado.\n");
			return 0;
		}
		strcat(aux_out,".mcr");
		if((fileOUT=fopen(aux_out,"w"))==NULL) {
			printf("\n\nERRO! não foi possivel criar arquivo.\n");
			return 0;
		}
		macros(fileIN,fileOUT);
		printf("\nExpansao de macros realizada com sucesso!\n");

	} else if(strcmp(argument,"-o")==0) { // Montagem do codigo

		if((fileIN=fopen(strcat(file_in,".asm"),"r"))==NULL) {
			printf("\n\nERRO! Arquivo não encontrado.\n");
			return 0;
		}
		if((fileOUT=fopen(strcat(file_out,".pre"),"w"))==NULL) {   /* abre o arquivo de saida */
			printf("\n\nERRO! não foi possivel criar arquivo.\n");
			return 0;
		}
		pre_processamento(fileIN,fileOUT);

        fclose(fileIN);
        fclose(fileOUT);

		if((fileIN=fopen(file_out,"r"))==NULL) {
			printf("\n\n\t\tERRO! Arquivo não encontrado.\n");
			return 0;
		}
		strcat(aux_out,".mcr"); // extensao do arquivo de saida
		if((fileOUT=fopen(aux_out,"w"))==NULL) {
            printf("<DEBUG> arquivo de .mcr saida: %s\n", aux_out);
			printf("\n\n\t\tERRO! não foi possivel criar arquivo.\n");
			return 0;
		}
		macros(fileIN,fileOUT);

        fclose(fileIN);
        fclose(fileOUT);

		if((fileIN=fopen(aux_out,"r"))==NULL) {
			printf("\n\n\t\tERRO! Arquivo não encontrado.\n");
			return 0;
		} else {
            strcat(aux_out2,".o"); // extensao do arquivo de saida
			char* fonte="";
            fseek(fileIN, 0L, SEEK_END); //posiona o fileIN no final do arquivo
            long tam_fonte = ftell(fileIN); //pega o tamanho
            fseek(fileIN, 0L, SEEK_SET); //posiona o fileIN no inicio do arquivo (mesmo que rewind(fileIN), mas com retorno)
            fonte = (char*) calloc(tam_fonte,sizeof(char)); //alocacao dinamica
            if(!passar_pra_string(fileIN, fonte, tam_fonte)) { //problema na alocacao dinamica (memoria)
                printf("Erro ao ler o fonte.\n");
                return 0;
            }
            printf("\nARQUIVO QUE SERA MONTADO:\n\n%s\n", fonte);
            printf("Relatorio da montagem:\n\n");
            fileOUT=fopen(aux_out2,"w");
            if(!fileOUT){
                printf("Erro ao criar o arquivo.\n");
                return 0;
            }
            else {
                monta(fonte,fileOUT);
                if(!erro) {
                    printf("Arquivo montado com sucesso!\n");
                } else {
                    remove(aux_out2); //apaga o arquivo de montagem se tiver dado erro. "Só monta se nao tiver erro"
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
