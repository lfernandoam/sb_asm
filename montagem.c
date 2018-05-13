#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <ctype.h> //pode ser util para verificar o conteudo da string. Ex: se eh maisculo (converte tbm)

// verificar erro do tipo ADD Y+2 pro caso que Y+2 não exista
// PERGUNTAR SOBRE UTILIZAR ENDERECOS Q NAO FAZEM PARTE DA VARIAVEL SHIFTADA. EX: LOAD Y+5 sendo Y: SPACE 2
// ISTO SERIA POSSIVEL SE O CARREGADOR ALOCASSE A MEMORIA CONSECUTIVAMENTE NA MEMORIA E A PROXIMA VARIAVEL CONTENHA ESTE ENDERECO.

//ADD rotulo de linha na secao TEXT deve dar erro?
//POSSO CONSIDERAR QUE O "STOP" SEMPRE SERA A ULTIMA INSTRUCAO DA SEÇÃO TEXT? creio que nao.


//<FIM eh SECTION TEXT; =>FIM eh SECTION DATA
//OBS: NAO PODE HAVER LABEL/ROTULO NA LINHA DO SECTION DATA (L1: SECTION DATA), POIS CONSIDERO <FIM COMO SECTION TEXT
//...MAS NA VDD <FIM TBM INCLUI A LINHA DO SECTION DATA

//int endereco=0;
int offset=0;
int linha=1;
int fim=0;
int erro=0;
FILE *fp_o;

typedef struct SymbTable{ /*Tabela de Simbolos*/
    char symbol[20]; //nome do simbolo
    char end; //endereco
    int const_value; //valor de sua constante (-9999 se nao eh const)
    //int space_size; //tamanho do campo space (quantos enderecos esta diretiva aloca)
    }SymbTable;

//typedef struct LineTable{ /*Tabela de equivalencia de linhas*/
//    int orig;
//    int macro;
//    }LineTable;
//

int getToken(char* fonte,int *pos, char* token);
int monta(char* fonte);
int passar_pra_string(FILE *fp_mcr, char* fonte, long tam_fonte);
void insertTable(SymbTable* tSymb, int *tamSymb, char* token, int endereco, int const_value);
int checkEnd(SymbTable* tSymb, int tamSymb, char* token);
int segPassagem(char* fonte, SymbTable* tSymb, int tamSymb, char* token, int endf);

void exibe(SymbTable *tSymb, int tamSymb){
    int i;
    printf("\n");
    for(i=0;i<tamSymb;i++){
        printf("tSymb[i].symbol: %s | ",tSymb[i].symbol);
        printf("tSymb[i].end: %d | ",tSymb[i].end);
        printf("tSymb[i].const_value: %d | ",tSymb[i].const_value);
        //printf("tSymb[i].space_size: %d\n",tSymb[i].space_size);
    }
}

void insertTable(SymbTable* tSymb, int *tamSymb, char* token, int endereco, int const_value){
    int i, existe=0;
    int tam_token=strlen(token);
    token[tam_token-1]='\0';
    for(i=0;i<*tamSymb;i++){
        //printf("TOKEN: %s\n",token);
        if(strcmp(token,tSymb[i].symbol)==0){
            existe=1;
            printf("ERRO SEMANTICO: Linha %d com Rotulo repetido.\n",linha); // erro semantico
        }
    }
    if (!existe){
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


int checkEnd(SymbTable* tSymb, int tamSymb, char* token){
    int i;
    for(i=0;i<tamSymb;i++){
        if(strcmp(token,tSymb[i].symbol)==0){
            return tSymb[i].end;
        }
    }
    return -1;
}

int desloc(char* token){
    if(offset){
        //printf("valor de offset: %d\n",atoi(token));
        return atoi(token);
    }
    return 0;
}

int getToken(char* fonte,int *pos, char* token) {
    int i = 0, j = *pos;
    if(fonte[j] == '\n'){
        //printf("linha: %d\n",linha);
        linha++;
	}
	while(fonte[j] == '\n' || fonte[j] == '\t' || fonte[j] == ' ' || fonte[j] == '\r' || fonte[j] == '\0' || fonte[j]=='+' || fonte[j]==',') {
        j++;;
        ++*pos;
	}
	while(fonte[j] != '\n' && fonte[j] != '\t' && fonte[j] != ' ' && fonte[j] != '\r' && fonte[j] != '\0') {
		token[i] = fonte[j];
		i++;j++;
		//printf("\nfonte[j]= %c (n-1: %c)\n",fonte[j],fonte[j-1]);
        //if((fonte[j]==':')||(fonte[j-1]==':')) break;
        if((fonte[j]=='+')){
            offset=1;
            //printf("setei offset %c\n", token[i-1]);
            break;
        }
        if((fonte[j]==',')||(fonte[j-1]==',')){
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

int getConstValue(char* fonte,int j) {
    int i = 0;
    char valor[21];
    int ehconst=0;
    //printf("fonte[j]=%c",fonte[j]);
    //primeiro token
	while(fonte[j] == '\n' || fonte[j] == '\t' || fonte[j] == ' ' || fonte[j] == '\r' || fonte[j] == '\0' || fonte[j]=='+' || fonte[j]==',') {
        j++;
	}
	while(fonte[j] != '\n' && fonte[j] != '\t' && fonte[j] != ' ' && fonte[j] != '\r' && fonte[j] != '\0') {
		valor[i] = fonte[j];
		i++;j++;
	}
	valor[i] = '\0';
    if(strcmp(valor,"CONST")==0){
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
		i++;j++;
	}
	valor[i] = '\0';
	if (ehconst) return atoi(valor);
	return -9999;
}

int segPassagem(char* fonte, SymbTable* tSymb, int tamSymb, char* token, int endf){
    int i=0,j=0;
    int pos=0;
    char auxtoken[20];
    int tam_fonte=strlen(fonte);
    pos+=getToken(fonte, &pos, token);
    while(pos<tam_fonte){ //NAO PODE SER -1 pois precisa executar a funcao pro ultimo token
        offset=0;
        //getchar();

        if(strcmp(token,"ADD")==0){
            pos+=getToken(fonte, &pos, token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)<fim){ //FIM eh o endereco que começa os rotulos de "SECTION DATA"
                printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linha);
                erro=1;
            }
            else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("1 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"1 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"SUB")==0){
            pos+=getToken(fonte, &pos, token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)<fim){ //FIM eh o endereco que começa os rotulos de "SECTION DATA"
                printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("2 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"2 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"MULT")==0){
            pos+=getToken(fonte, &pos, token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)<fim){ //FIM eh o endereco que começa os rotulos de "SECTION DATA"
                printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("3 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"3 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"DIV")==0){
            pos+=getToken(fonte, &pos, token);

            strcpy(auxtoken,token);
            if (offset){
                //printf("\n\ntem offset e nao eh const\n\n"); //TESTAR
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)<fim){ //FIM eh o endereco que começa os rotulos de "SECTION DATA"
                printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            for(i=0;i<tamSymb;i++){
                if(strcmp(auxtoken,tSymb[i].symbol)==0){
                    if(tSymb[i].const_value==0){
                        printf("ERRO SEMANTICO: Linha %d com Divisao por Zero.\n",linha);
                        erro=1;
                    }
                }
            }
            //printf("4 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"4 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"JMP")==0){
            pos+=getToken(fonte, &pos, token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com pulo para rotulo inválido.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim){
                printf("ERRO SEMANTICO: Linha %d com pulo para seção INVALIDA.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("5 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"5 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"JMPN")==0){
            pos+=getToken(fonte, &pos, token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com pulo para rotulo inválido.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim){
                printf("ERRO SEMANTICO: Linha %d com pulo para seção INVALIDA.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("6 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"6 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"JMPP")==0){
            pos+=getToken(fonte, &pos, token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com pulo para rotulo inválido.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim){
                printf("ERRO SEMANTICO: Linha %d com pulo para seção INVALIDA.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("7 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"7 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"JMPZ")==0){
            pos+=getToken(fonte, &pos, token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com pulo para rotulo inválido.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)>=fim){
                printf("ERRO SEMANTICO: Linha %d com pulo para seção INVALIDA.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("8 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"8 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"COPY")==0){
            pos+=getToken(fonte, &pos, token);

            strcpy(auxtoken,token);
            if (offset){
                //printf("\nachei offset no 1o. arg COPY\n");
                pos+=getToken(fonte, &pos, token);
            }
            //printf("auxtoken1: %s\n",auxtoken);
            //printf("checkEnd(auxtoken)=%d\n",checkEnd(tSymb,tamSymb,auxtoken));
            //printf("token %s\n",token);
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)<fim){ //FIM eh o endereco que começa os rotulos de "SECTION DATA"
                printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("9 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"9 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            pos+=getToken(fonte, &pos, token);
            strcpy(auxtoken,token);
            if (offset){
                //printf("\nachei offset no 2o. arg COPY\n");
                pos+=getToken(fonte, &pos, token);
            }
            //printf("auxtoken2: %s\n",auxtoken);
            //printf("checkEnd(auxtoken)=%d\n",checkEnd(tSymb,tamSymb,auxtoken));
            //printf("token %s\n",token);
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)<fim){ //FIM eh o endereco que começa os rotulos de "SECTION DATA"
                printf("ERRO SEMANTICO: Linha %d com operacao com rotulo da seção TEXT.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            for(i=0;i<tamSymb;i++){
                if(strcmp(auxtoken,tSymb[i].symbol)==0){
                    if(tSymb[i].const_value!=-9999){
                        printf("ERRO SEMANTICO: Linha %d com modificação de um valor constante.\n",linha);
                        erro=1;
                    }
                }
            }
            //printf("%d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"%d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"LOAD")==0){
            pos+=getToken(fonte, &pos, token);

            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)<fim){ //FIM eh o endereco que começa os rotulos de "SECTION DATA"
                printf("ERRO SEMANTICO: Linha %d com LOAD com argumentos definidos fora da SECTION DATA.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("10 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"10 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"STORE")==0){
            pos+=getToken(fonte, &pos, token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)<fim){ //FIM eh o endereco que começa os rotulos de "SECTION DATA"
                printf("ERRO SEMANTICO: Linha %d com STORE com argumentos definidos fora da SECTION DATA.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("\n\n check: %d\n",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            for(i=0;i<tamSymb;i++){
                if(strcmp(auxtoken,tSymb[i].symbol)==0){
                    if(tSymb[i].const_value!=-9999){
                        printf("ERRO SEMANTICO: Linha %d com modificação de um valor constante.\n",linha);
                        erro=1;
                    }
                }
            }
            //printf("11 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"11 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }
        else if(strcmp(token,"INPUT")==0){
            pos+=getToken(fonte, &pos, token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com Rotulo ausente.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)<fim){ //FIM eh o endereco que começa os rotulos de "SECTION DATA"
                printf("ERRO SEMANTICO: Linha %d com INPUT com argumentos definidos fora da SECTION DATA.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("12 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"12 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            //printf("checkEnd = %d\n",checkEnd(tSymb,tamSymb,auxtoken));
        }
        else if(strcmp(token,"OUTPUT")==0){
            pos+=getToken(fonte, &pos, token);

            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
            }
            if(checkEnd(tSymb,tamSymb,auxtoken)==-1){
                printf("ERRO SEMANTICO: Linha %d com Rotulo ausente\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken)<fim){ //FIM eh o endereco que começa os rotulos de "SECTION DATA"
                printf("ERRO SEMANTICO: Linha %d com OUTPUT com argumentos definidos fora da SECTION DATA.\n",linha);
                erro=1;
            }else if(checkEnd(tSymb,tamSymb,auxtoken) + desloc(token)>endf){
                printf("ERRO SEMANTICO: Linha %d utiliza-se de um endereço inválido.\n",linha);
                erro=1;
            }
            //printf("13 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
            fprintf(fp_o,"13 %d ",checkEnd(tSymb,tamSymb,auxtoken) + desloc(token));
        }else if(strcmp(token,"STOP")==0){
            //printf("14 ");
            fprintf(fp_o,"14 ");
            pos+=getToken(fonte, &pos, token);
        }else if(strcmp(token,"SPACE")==0){
            //aux=pos;
            //printf("linha do space %d\n",linha);

            //VERIFICAR SE VAI DAR A LINHA CORRETA SE TIVER ERRO NA PARTE DOS SPACE
            pos+=getToken(fonte, &pos, token);
            j=atoi(token); //ou utilizar a ultima coluna da tabela de simbolos
            if (j==0){
                j=1;
                //pos=aux;
            }
            for(i=1;i<=j;i++){
                //printf("0 ");
                fprintf(fp_o,"0 ");
            }
        }else if(strcmp(token,"CONST")==0){
            pos+=getToken(fonte, &pos, token);
            j=atoi(token);
            //printf("%d ",j);
            fprintf(fp_o,"%d ",j);
        }else{
            pos+=getToken(fonte, &pos, token);
        }
    }
    return 1;
}


int monta(char* fonte){
    int tamSymb=0;
    SymbTable *tSymb = (SymbTable*) malloc(sizeof(SymbTable));
    char token[21];
    char auxtoken[21];
    int endereco=0;
    //int aux;
    int const_value=-9999; //j eh diferente de -9999 sempre que eh const
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

    while(pos<tam_fonte-1){
        //printf("\npos: %d\n",pos);
        //getchar();
        //tam_token=strlen(token);
        if(strcmp(token,"ADD")==0){
            endereco+=2;
        }
        else if(strcmp(token,"SUB")==0){
            endereco+=2;
        }
        else if(strcmp(token,"MULT")==0){
            endereco+=2;
        }
        else if(strcmp(token,"DIV")==0){
            endereco+=2;
        }
        else if(strcmp(token,"JMP")==0){
            endereco+=2;
        }
        else if(strcmp(token,"JMPN")==0){
            endereco+=2;
        }
        else if(strcmp(token,"JMPP")==0){
            endereco+=2;
        }
        else if(strcmp(token,"JMPZ")==0){
            endereco+=2;
        }
        else if(strcmp(token,"COPY")==0){
            endereco+=3; // CUIDADO!
        }
        else if(strcmp(token,"LOAD")==0){
            endereco+=2;
        }
        else if(strcmp(token,"STORE")==0){
            endereco+=2;
        }
        else if(strcmp(token,"INPUT")==0){
            endereco+=2;
        }
        else if(strcmp(token,"OUTPUT")==0){
            endereco+=2;
        }
        else if(strcmp(token,"STOP")==0){
            endereco+=1;
        }
        else if(strcmp(token,"SPACE")==0){
            //aux=pos;
            pos+=getToken(fonte, &pos, token);
            space_size=atoi(token);
            if (space_size==0){ //caso nao tenha nada depois de SPACE
                space_size=1;
                //pos=aux;
            }
            //printf("j: %d\n",j);
            endereco+=space_size;
            //printf("         space_size: %d\n\n\n",space_size);
        }
        else if(strcmp(token,"CONST")==0){
            endereco+=1;
            pos+=getToken(fonte, &pos, token);
        }
        tam_token=strlen(token);

        if (token[tam_token-1]==':'){ // se eh rotulo
            const_value=getConstValue(fonte,pos);
            //printf("const_value: %d\n",const_value);
            insertTable(tSymb, &tamSymb, token, endereco, const_value);
        }
        const_value=-9999; //const_value eh -9999 sempre que nao eh const
        space_size=0;

        strcpy(auxtoken,token);
        strcat(auxtoken," ");

        pos+=getToken(fonte, &pos, token);
        if(strcmp(strcat(auxtoken,token),"SECTION DATA")==0){
            fim=endereco;
        }
    }
    //endereco--; //a ultima instrucao sempre soma 1 endereco (SPACE ou CONST) pra compatibilizar com a linha seguinte
    linha=1;
    //printf("O ultimo enderereco eh: %d\n",endereco);
    //printf("O SECTION DATA encontra-se a partir do endereco %d",fim);
    //printf("\n\nSeg Passagem\n");
    segPassagem(fonte, tSymb, tamSymb, token, endereco);
    return 1;
}


//veficar o que ocorre se for ADD ADD (label for reservado com nome ADD)


int passar_pra_string(FILE *fp_mcr, char* fonte, long tam_fonte){
    if (!fonte) return 0; //se deu erro de memoria
    fread(fonte, sizeof(char), tam_fonte, fp_mcr);
    return 1;
}

int main(int argc, char* argv[]) {
    char* fonte="";
    if (argc<4){ //depois aumentaremos para receber mais parametros conforme especificacao
        printf("Faltaram argumentos.\n");
        return 0;
    }
	char* arquivo_mcr=strcat(argv[2],".mcr"); //a especificacao pede pra nao inserir a extensao junto

	//printf("<DEBUG> arquivo de entrada: %s\n", arquivo_mcr);

    FILE *fp_mcr;
    //---------------------------------------------------------------
    // o arquivo de saida declarei como variavel global

    fp_mcr = fopen(arquivo_mcr, "r");
    if (!fp_mcr) printf("Arquivo de entrada invalido.\n"); //fp_mcr == NULL
    else {
        fseek(fp_mcr, 0L, SEEK_END); //posiona o fp_mcr no final do arquivo
        long tam_fonte = ftell(fp_mcr); //pega o tamanho
        fseek(fp_mcr, 0L, SEEK_SET); //posiona o fp_mcr no inicio do arquivo (mesmo que rewind(fp_mcr), mas com retorno)
        fonte = (char*) calloc(tam_fonte,sizeof(char)); //alocacao dinamica
        if(!passar_pra_string(fp_mcr, fonte, tam_fonte)) { //problema na alocacao dinamica (memoria)
            printf("Erro ao passar pra string!\n");
            return 0;
        }
        fclose(fp_mcr);
        printf("%s\n", fonte);
        printf("Relatorio da montagem:\n\n");
    	char* arquivo_o=strcat(argv[3],".o");
        //printf("<DEBUG> arquivo de saida: %s\n", arquivo_o);
        fp_o=fopen(arquivo_o,"w");
        if(!fp_o) printf("Erro ao criar o arquivo.");
        else{
            monta(fonte);
            if(!erro){
                printf("Arquivo montado com sucesso!");
            }else{
                remove(arquivo_o);
            }
        }
        fclose(fp_o);
    }
}
