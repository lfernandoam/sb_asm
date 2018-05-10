#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <ctype.h> //pode ser util para verificar o conteudo da string. Ex: se eh maisculo (converte tbm)

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
        printf("tSymb[i].value: %d\n\n",tSymb[i].value);
    }
}

void primPassagem(SymbTable* tSymb, int *tamSymb, char* token, int tam_token){
    //printf("estou na busca\n");
    int i, existe=0;
    //printf("token %s\n",token);
    //printf("tam_token= %d\n",tam_token);
    //printf("token[tam_token-1]=%c\n",token[tam_token-1]);
    if (token[tam_token-1]==':'){
        token[tam_token-1]='\0';
        //printf("\nTOOOKEN: %s\n", token);
        //printf("Achei dois pontos.\n");
        //printf("tamSymb: %d\n",*tamSymb); //tamSymb é sempre zero.
        for(i=0;i<*tamSymb;i++){
            //printf("TOKEN: %s\n",token);
            if(strcmp(token,tSymb[i].symbol)==0){
                existe=1;
                //printf("Existe na tabela.\n"); // erro semantico
            }
        }
        if (!existe){
            //printf("Não existe na tabela.\n");
            (*tamSymb) = (*tamSymb)+1;
            //printf("tamSymb: %d\n",*tamSymb);
            tSymb=realloc(tSymb,(*tamSymb)*sizeof(SymbTable));
            strcpy(tSymb[*tamSymb-1].symbol,token); //tSymb[tamSymb-1] eh a linha
            tSymb[*tamSymb-1].value=endereco;
        }
    }
    //printf("FINAL DO BUSCA tamSymb: %d\n",*tamSymb);
    //exibe(tSymb,*tamSymb);
}


int checkTable(SymbTable* tSymb, int tamSymb, char* token, int tam_token){
    //printf("estou na checkTable\n");
    int i;
    //printf("tamSymb= %d\n",tamSymb);
    //exibe(tSymb,tamSymb);
    //printf("\n\ncheckTable TOKEN: %s\n",token);
    //printf("\n\ncheckTable TOKEN: %s\n",strcat(token,":"));
    //printf("cheguei");
    for(i=0;i<tamSymb;i++){
        //printf("checkTable TOKEN: %s\n",token);
        if(strcmp(token,tSymb[i].symbol)==0){
            //printf("\n\ntSymb[i].value: %d\n\n",tSymb[i].value);
            return tSymb[i].value;
        }
    }
    return -1;
}


int desloc(char* token){
    if(offset){
        //printf("deslocccc\n");
        printf("valor de offset: %d\n",atoi(token));
        return atoi(token);
    }
    return 0;

}

int getToken(char* fonte,int *pos, char* token) {
    //printf("get token, fonte:\n%s\n", fonte);
    int i = 0, j = *pos, k=0; //k eh pra compensar os caracteres que a busca por token ignorou (util para 'pos')
    //printf("\nfonte[j]= %d ('%c')\n",fonte[j],fonte[j]);
	while(fonte[j] == '\n' || fonte[j] == '\t' || fonte[j] == ' ' || fonte[j] == '\r' || fonte[j] == '\0') {
        j++;k++;
        (*pos)=(*pos)+1;
	}
	//while(!(fonte[j] == '\n' || fonte[j] == '\t' || fonte[j] == ' ' || fonte[j] == '\r' || fonte[j] == '\0')) {
	while(fonte[j] != '\n' && fonte[j] != '\t' && fonte[j] != ' ' && fonte[j] != '\r' && fonte[j] != '\0') {
		token[i] = fonte[j];
		i++;j++;
		//printf("\nfonte[j]= %c (n-1: %c)\n",fonte[j],fonte[j-1]);
        //if((fonte[j]==':')||(fonte[j-1]==':')) break;
        if((fonte[j]=='+')||(fonte[j-1]=='+')){
            offset=1;
            //printf("setei offset\n");
            break;
        }
        if((fonte[j]=='-')||(fonte[j-1]=='-')) break;
        if((fonte[j]==',')||(fonte[j-1]==',')) break;

	}
	token[i] = '\0';
    int tam=strlen(token)+k;
	//printf("\n\ntoken %s\n",token);
	return tam;
}


int segPassagem(char* fonte, SymbTable* tSymb, int tamSymb, char* token, int tam_token){
    int i=0,j=0;
    int pos=0;
    int aux;
    char auxtoken[20];
    int tam_fonte=strlen(fonte);
    //printf("segPassagem, fonte:\n%s\n", fonte);

    pos+=getToken(fonte, &pos, token);
    tam_token=strlen(token);

    //printf("tam_fonte: %d\n",tam_fonte);
    //printf("tam_token: %d\n",tam_token);

    //printf("pos: %d\n",pos);

    while(pos<=tam_fonte){
        //tam_token=strlen(token);
  //  primPassagem(tSymb, &tamSymb, token, tam_token);
//void primPassagem(SymbTable* tSymb, int *tamSymb, char* token, int tam_token){
        if(strcmp(token,"ADD")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);

            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("1 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
        }
        else if(strcmp(token,"COPY")==0){
            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);

            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("9 %d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG

            pos+=getToken(fonte, &pos, token);
            tam_token=strlen(token);

            strcpy(auxtoken,token);
            if (offset){
                pos+=getToken(fonte, &pos, token);
                tam_token=strlen(token);
            }
            printf("%d ",checkTable(tSymb,tamSymb,auxtoken,tam_token) + desloc(token));
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
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
            offset=0; // VERIFICAFR SE EH AQUI QUE DEVEMOS ZERAR A FLAG
        }
        else if(strcmp(token,"STOP")==0){
            printf("14 ");
        }
        else if(strcmp(token,"SPACE")==0){
            aux=pos;
            pos+=getToken(fonte, &pos, token);
            j=atoi(token);
            if (j==0){
                j=1;
                pos=aux;
            }
            //printf("j: %d\n",j);
            for(i=1;i<=j;i++){
                printf("0 ");
            }
        }
        else if(strcmp(token,"CONST")==0){
            pos+=getToken(fonte, &pos, token);
            j=atoi(token);
            printf("%d ",j);
        }
        //printf("endereco: %d\n",endereco);
        //printf("tamSymb: %d\n\n",tamSymb);
        pos+=getToken(fonte, &pos, token);
        //printf("tamSymb2: %d\n\n",tamSymb);
        tam_token=strlen(token);
        //pos+=tam_token;
        //printf("pos: %d, tam_fonte: %d\n\n",pos,tam_fonte);
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

    //printf("tam_fonte: %d\n",tam_fonte);
    //printf("tam_token: %d\n",tam_token);

    //printf("pos: %d\n",pos);

    while(pos<=tam_fonte){
        //tam_token=strlen(token);
        if(strcmp(token,"ADD")==0){
            //printf("1 ");
            endereco+=2;
        }
        else if(strcmp(token,"SUB")==0){
            //printf("2 ");
            endereco+=2;
        }
        else if(strcmp(token,"MULT")==0){
            //printf("3 ");
            endereco+=2;
        }
        else if(strcmp(token,"DIV")==0){
            //printf("4 ");
            endereco+=2;
        }
        else if(strcmp(token,"JMP")==0){
            //printf("5 ");
            endereco+=2;
        }
        else if(strcmp(token,"JMPN")==0){
            //printf("6 ");
            endereco+=2;
        }
        else if(strcmp(token,"JMPP")==0){
            //printf("7 ");
            endereco+=2;
        }
        else if(strcmp(token,"JMPZ")==0){
            //printf("8 ");
            endereco+=2;
        }
        else if(strcmp(token,"COPY")==0){
            //printf("9 ");
            endereco+=3; // CUIDADO!
        }
        else if(strcmp(token,"LOAD")==0){
            //printf("10 ");
            endereco+=2;
        }
        else if(strcmp(token,"STORE")==0){
            //printf("11 ");
            endereco+=2;
        }
        else if(strcmp(token,"INPUT")==0){
            //printf("12 ");
            endereco+=2;
        }
        else if(strcmp(token,"OUTPUT")==0){
            //printf("13 ");
            endereco+=2;
        }
        else if(strcmp(token,"STOP")==0){
            //printf("14 ");
            endereco+=1;
        }
        else if(strcmp(token,"SPACE")==0){
            aux=pos;
            pos+=getToken(fonte, &pos, token);
            j=atoi(token);
            if (j==0){
                j=1;
                pos=aux;
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
        else{
            primPassagem(tSymb, &tamSymb, token, tam_token); //veficar o que ocorre se for ADD ADD (label for reservado com nome ADD)
        }
        //printf("endereco: %d\n",endereco);
        //printf("tamSymb: %d\n\n",tamSymb);
        pos+=getToken(fonte, &pos, token);
        //printf("tamSymb2: %d\n\n",tamSymb);
        tam_token=strlen(token);
        //pos+=tam_token;
        //printf("pos: %d, tam_fonte: %d\n\n",pos,tam_fonte);

    }
  //  primPassagem(tSymb, &tamSymb, token, tam_token);
//void primPassagem(SymbTable* tSymb, int *tamSymb, char* token, int tam_token){

    segPassagem(fonte, tSymb, tamSymb,token,tam_token);
//int segPassagem(char* fonte, SymbTable* tSymb, int tamSymb, char* token, int tam_token){

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
        printf("main, fonte:\n%s\n", fonte);
//        while(pos<tam_fonte-1){
//            //printf("fonte[pos]: %d ('%c')\n",fonte[pos],fonte[pos]);
//            //pos++;
//            pos+=getToken(fonte, &pos, token);
//            printf("\n\ntoken: %s\npos: %d\n",token,pos);
//        }
        //printf("Montagem:\n");
        monta(fonte);

        //printf("\n\nendereco: %d\n",endereco);

        //printf("token: %s\n",token);
    }

}
