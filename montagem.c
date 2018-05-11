#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <ctype.h> //pode ser util para verificar o conteudo da string. Ex: se eh maisculo (converte tbm)

// verificar erro do tipo ADD Y+2 pro caso que Y+2 não exista

int endereco=0;
int offset=0;

typedef struct SymbTable{ /* A tabela para instrucao EQU */
    char symbol[20];
    char value;
    }SymbTable;


int getToken(char* fonte,int *pos, char* token);
int monta(char* fonte);
int passar_pra_string(FILE *fp, char* fonte, long tam_fonte);
void primPassagem(SymbTable* tSymb, int *tamSymb, char* token, int tam_token);
int checkTable(SymbTable* tSymb, int tamSymb, char* token, int tam_token);
int segPassagem(char* fonte, SymbTable* tSymb, int tamSymb, char* token, int tam_token);

void exibe(SymbTable *tSymb, int tamSymb){
    int i;
    for(i=0;i<tamSymb;i++){
        printf("tSymb[i].symbol: %s\n",tSymb[i].symbol);
        printf("tSymb[i].value: %d\n",tSymb[i].value);
    }
}

void primPassagem(SymbTable* tSymb, int *tamSymb, char* token, int tam_token){
    int i, existe=0;
    if (token[tam_token-1]==':'){
        token[tam_token-1]='\0';
        //printf("INSERÇÃO: %s\n", token);
        for(i=0;i<*tamSymb;i++){
            //printf("TOKEN: %s\n",token);
            if(strcmp(token,tSymb[i].symbol)==0){
                existe=1;
                //printf("Existe na tabela.\n"); // erro semantico
            }
        }
        if (!existe){
            ++*tamSymb;
            tSymb=realloc(tSymb,(*tamSymb)*sizeof(SymbTable));
            strcpy(tSymb[*tamSymb-1].symbol,token); //tSymb[tamSymb-1] eh a linha
            tSymb[*tamSymb-1].value=endereco;
        }
    //exibe(tSymb,*tamSymb);
    }
    //printf("FINAL DO BUSCA tamSymb: %d\n",*tamSymb);
}


int checkTable(SymbTable* tSymb, int tamSymb, char* token, int tam_token){
    int i;
    for(i=0;i<tamSymb;i++){
        if(strcmp(token,tSymb[i].symbol)==0){
            return tSymb[i].value;
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
    int i = 0, j = *pos; //k eh pra compensar os caracteres que a busca por token ignorou (util para 'pos')
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

int segPassagem(char* fonte, SymbTable* tSymb, int tamSymb, char* token, int tam_token){
    int i=0,j=0;
    int pos=0;
    char auxtoken[20];
    int tam_fonte=strlen(fonte);
    pos+=getToken(fonte, &pos, token);
    tam_token=strlen(token);
    while(pos<tam_fonte){ //NAO PODE SER -1 pois precisa executar a funcao pro ultimo token
        offset=0;
        //getchar();
        if(strcmp(token,"ADD")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token); //ACHO QUE NAO EH NECESSARIO
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("1 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"SUB")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("2 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"MULT")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("3 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"DIV")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);

            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("4 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"JMP")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("5 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"JMPN")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("6 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"JMPP")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("7 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"JMPZ")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("8 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"COPY")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);

            strcpy(auxtoken,token);
            if (offset){
                //printf("\nachei offset no 1o. arg COPY\n");
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            //printf("auxtoken1: %s\n",auxtoken);
            //printf("checkTable(auxtoken)=%d\n",checkTable(tSymb,tamSymb,auxtoken,tam_token));
            //printf("token %s\n",token);
            printf("9 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);
            strcpy(auxtoken,token);
            if (offset){
                //printf("\nachei offset no 2o. arg COPY\n");
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            //printf("auxtoken2: %s\n",auxtoken);
            //printf("checkTable(auxtoken)=%d\n",checkTable(tSymb,tamSymb,auxtoken,tam_token));
            //printf("token %s\n",token);
            printf("%d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"LOAD")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);

            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("10 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"STORE")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("11 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"INPUT")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);
            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("12 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
            //printf("checktable = %d\n",checkTable(tSymb,tamSymb,auxtoken,tam_token));
        }
        else if(strcmp(token,"OUTPUT")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);

            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("13 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
        }
        else if(strcmp(token,"STOP")==0){
            printf("14 ");
            pos+=getToken(fonte, &pos, token);
        }
        else if(strcmp(token,"SPACE")==0){
            //aux=pos;
            pos+=getToken(fonte, &pos, token);
            j=atoi(token);
            if (j==0){
                j=1;
                //pos=aux;
            }
            for(i=1;i<=j;i++){
                printf("0 ");
            }
        }
        else if(strcmp(token,"CONST")==0){
            pos+=getToken(fonte, &pos, token);
            j=atoi(token);
            printf("%d ",j);
        }else{
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);
        }
    }

    return 1;
}


int monta(char* fonte){
    int tamSymb=0;
    SymbTable *tSymb = (SymbTable*) malloc(sizeof(SymbTable));
    char token[21];
    int aux;
    int i=0,j=0;
    int pos=0;
    int tam_fonte=strlen(fonte);
    int tam_token;
    //printf("monta, fonte:\n%s\n", fonte);

    pos+=getToken(fonte, &pos, token);
    tam_token=strlen(token);
    //printf("token %s  pos: %d \n\n",token,pos);
    //printf("tam_fonte: %d\n",tam_fonte);
    //printf("tam_token: %d\n",tam_token);

    //printf("pos: %d\n",pos);

    while(pos<tam_fonte-1){
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
            aux=pos;
            pos+=getToken(fonte, &pos, token);
            j=atoi(token);
            if (j==0){
                j=1;
                //pos=aux;
            }
            //printf("j: %d\n",j);
            for(i=1;i<=j;i++){
                //printf("0 ");
            }
            endereco+=j;
        }
        else if(strcmp(token,"CONST")==0){
            endereco+=1;
            pos+=getToken(fonte, &pos, token);
            j=atoi(token);
            //printf("%d ",j);
        }
        tam_token=strlen(token);
        primPassagem(tSymb, &tamSymb, token, tam_token); //veficar o que ocorre se for ADD ADD (label for reservado com nome ADD)
        pos+=getToken(fonte, &pos, token);
        tam_token=strlen(token);
    }
    segPassagem(fonte, tSymb, tamSymb,token,tam_token);
    return 1;
}

int passar_pra_string(FILE *fp, char* fonte, long tam_fonte){
    if (!fonte) return 0; //se deu erro de memoria
    fread(fonte, sizeof(char), tam_fonte, fp);
    return 1;
}

int main(int argc, char* argv[]) {
    char* fonte="";
    if (argc<2){ //depois aumentaremos para receber mais parametros conforme especificacao
        printf("Faltou o nome do arquivo de entrada como argumento.\n");
        return 0;
    }
	char* arquivo_in=strcat(argv[1],".asm"); //a especificacao pede pra nao inserir a extensao junto
	//printf("<DEBUG> arquivo de entrada: %s\n", arquivo_in);
    FILE *fp;
    fp = fopen(arquivo_in, "r");
    if (!fp) printf("Arquivo de entrada invalido.\n"); //fp == NULL
    else {
        fseek(fp, 0L, SEEK_END); //posiona o fp no final do arquivo
        long tam_fonte = ftell(fp); //pega o tamanho
        fseek(fp, 0L, SEEK_SET); //posiona o fp no inicio do arquivo (mesmo que rewind(fp), mas com retorno)
        fonte = (char*) calloc(tam_fonte,sizeof(char)); //alocacao dinamica
        if(!passar_pra_string(fp, fonte, tam_fonte)) { //problema na alocacao dinamica (memoria)
            printf("Erro ao passar pra string!\n");
            return 0;
        }
        fclose(fp);
        printf("%s\n", fonte);
        printf("Montagem:\n");
        monta(fonte);
        //printf("\n\nendereco: %d\n",endereco);
    }
}
