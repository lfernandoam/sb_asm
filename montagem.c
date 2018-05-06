#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <ctype.h> //pode ser util para verificar o conteudo da string. Ex: se eh maisculo (converte tbm)

int pc=0;
int linha=0;
//int pos=0;


int getToken(char* fonte,int *pos, char* token) {

    int i = 0, j = *pos, k=0; //k eh pra compensar os caracteres que a busca por token ignorou (util para 'pos')
    //printf("\nfonte[j]= %d ('%c')\n",fonte[j],fonte[j]);
	while(fonte[j] == '\n' || fonte[j] == '\t' || fonte[j] == ' ' || fonte[j] == '\r' || fonte[j] == '\0') {
        j++;*pos++,k++;
	}
	//while(!(fonte[j] == '\n' || fonte[j] == '\t' || fonte[j] == ' ' || fonte[j] == '\r' || fonte[j] == '\0')) {
	while(fonte[j] != '\n' && fonte[j] != '\t' && fonte[j] != ' ' && fonte[j] != '\r' && fonte[j] != '\0') {
		token[i] = fonte[j];
		i++;j++;
		//printf("\nfonte[j]= %c (n-1: %c)\n",fonte[j],fonte[j-1]);
        if((fonte[j]==':')||(fonte[j-1]==':')) break;
        if((fonte[j]=='+')||(fonte[j-1]=='+')) break;
        if((fonte[j]=='-')||(fonte[j-1]=='-')) break;
        if((fonte[j]==',')||(fonte[j-1]==',')) break;

	}
	token[i] = '\0';
    int tam=strlen(token)+k;
	return tam;
}

//int montagem(){
//    char linha[512];
//    char inst[7]; // a maior instrucao (OUTPUT) tem 6 caracteres e um NULL (\0)
//    char op1[20];op2[20]; //a especificacao informa que 20 eh o tamanho maximo
//    while(fscanf(fp,"%s",token) != EOF){        //strcpy(buffer,""); strcpy(rot,""); strcpy(instruction,""); strcpy(operand1,""); strcpy(operand2,"");
//        if((scanLine(buffer, fpIN))!=0)
//            return 1;
//        if(strchr(buffer,':')!=NULL)
//            sscanf(buffer,"%s %s %s %s",rot, instruction, operand1, operand2);
//        else
//            sscanf(buffer,"%s %s %s",instruction, operand1, operand2);
//        doInstruction(fpOUT, instruction, operand1, operand2);
//    }
//    return 0;
//}

void leitura(FILE *fp, char* token, char* fonte){
    while(fscanf(fp,"%s",token) != EOF){
        strcat(fonte,token);
    }
    fclose(fp);
}

int passar_pra_string(FILE *fp, char* fonte, long tam_fonte){
    if (!fonte) return 0; //se deu erro de memoria
    fread(fonte, sizeof(char), tam_fonte, fp);
    //printf("fonte:\n%s", fonte);
    return 1;
//    while(fgets(buffer,20,fp)){
//        strcpy(fonte[i],buffer);
//        printf("Line %d: %s",i,fonte[i]);
//        i++;
//    }
}

int main(int argc, char* argv[]) {
    //printf("<DEBUG> argc: %d\n",argc);
    char token[21]; // max. 20 caracteres (especificacao) + \0
    //char fonte[255];
    int pos=0;
    int i=0;
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
        printf("fonte:\n%s", fonte);
        while(pos<tam_fonte-1){
            //printf("fonte[pos]: %d ('%c')\n",fonte[pos],fonte[pos]);
            //pos++;
            pos+=getToken(fonte, &pos, token);
            printf("\n\ntoken: %s\npos: %d\n",token,pos);
        }
        //printf("token: %s\n",token);
    }

}
