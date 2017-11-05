/*

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdlib.h>
#include <stdio.h>

/* Um array de strings */
typedef struct StringArray {
  char **data;
  unsigned int length;
} StringArray;

/* Uma tabela */
typedef struct Table {
  unsigned int columns; /* Número de colunas de dados */
  unsigned int lines; /* Número de linhas de dados */
  StringArray *columnNames; /* Nome das colunas */
  StringArray *lineLabels; /* Nome de cada linha, por padrão é a primeira coluna */
  double **matrix; /* Contém os dados */
} Table;

/* Cria o array de strings */
StringArray *str_createArray(unsigned int length){
  StringArray *strArray = (StringArray *)malloc(sizeof(StringArray));
  if(length > 0) strArray->data = (char **)calloc(length, sizeof(char *));
  else strArray->data = NULL;
  strArray->length = length;
  return strArray;
}

/* Lê uma linha de um arquivo */
char *file_readLine(FILE *file){
  char *line = (char *)malloc(1), c;
  unsigned int i=0; // Espaço para '\0'
  while((c = fgetc(file)) != '\n' && !feof(file)){
    line[i] = c;
    line = (char *)realloc(line, ++i+1); // Aloca para o próximo caracter
  }
  line[i] = '\0';
  return line;
}

/* Remove o último elemento de um array de strings */
void str_pop(StringArray *strArray){
  if(strArray->length > 0) {
    free(strArray->data[strArray->length-1]);
    strArray->data[strArray->length-1] = NULL;
    strArray->data = (char **)realloc(strArray->data, (--strArray->length)*sizeof(char *));
  }
}

/* Remove os últimos elementos do array de strings se eles forem "\0" */
void str_popNULL(StringArray *strArray){
	while(strArray->length && strArray->data[strArray->length-1][0] == '\0') str_pop(strArray);
}

/* Adiciona uma string ao array de strings */
void str_push(StringArray *strArray, char *str){
  unsigned int i=0;
  strArray->data = (char **)realloc(strArray->data, (++strArray->length)*sizeof(char *));
  strArray->data[strArray->length-1] = (char *)malloc(1);
  while(str[i] != '\0') {
    strArray->data[strArray->length-1][i] = str[i];
    strArray->data[strArray->length-1] = (char *)realloc(strArray->data[strArray->length-1], ++i+1);
  }
  strArray->data[strArray->length-1][i] = '\0';
}

/* Corta uma string nas posições passadas (retorna outra string) */
char *str_splice(char *str, unsigned int begin, unsigned int end){
  unsigned int i=0,size=0;
  char *tmp = (char *)malloc(1);
  for(;str[i] != '\0';i++){
    if(i >= begin && i < end){
      tmp[size] = str[i];
      tmp = (char *)realloc(tmp, ++size+1);
    }
  }
  tmp[size] = '\0';
  return tmp;
}

/* Converte uma string em um array de strings de acordo com uma chave */
StringArray *str_split(char *str, char key){
  unsigned int i=0,last=0;
  StringArray *strArray = str_createArray(0);
  char *tmp;
  
  for(;str[i] != '\0';i++){
    if(str[i] == key){
    	tmp =  str_splice(str, last, i);
      str_push(strArray, tmp);
      free(tmp);
      last = i+1;
    }
  }
  
  tmp =  str_splice(str, last, i);
  str_push(strArray, tmp);
  free(tmp);
  tmp = NULL;
  
  return strArray;
  
}

/* Limpa um array de strings */
void str_clean(StringArray **strArray){
	while((*strArray)->length) str_pop(*strArray);
	free(*strArray);
	*strArray = NULL;
}

/* Mostra um array de strings na tela */
void str_printArray(StringArray *strArray){
  unsigned int i=0;
  if(strArray->length > 0) {
    printf("[\"%s\"", strArray->data[i++]);
    for(;i<strArray->length;i++) printf(",\"%s\"", strArray->data[i]);
    printf("]");
  }
  else printf("Empty");
}

/* Cria uma tabela */
Table *table_createTable(){
  Table *table = (Table *)malloc(sizeof(Table));
  table->columns = 0;
  table->lines = 0;
  table->columnNames = NULL;
  table->lineLabels = NULL;
  table->matrix = NULL;
}

/* Limpa uma tabela */
void table_clean(Table *table){
	unsigned int i=0;
	if(table->columnNames) str_clean(&table->columnNames);
	if(table->lineLabels) str_clean(&table->lineLabels);
	for(;i<table->lines;i++){
		free(table->matrix[i]);
		table->matrix[i] = NULL;
	}
	free(table->matrix);
	free(table);
}

/* Define as colunas de uma tabela */
void table_setColumns(Table *table, StringArray *strArray){
	unsigned int i=0;
	table->columnNames = str_createArray(0);
	for(;i<strArray->length;i++) str_push(table->columnNames, strArray->data[i]);
	table->columns = table->columnNames->length;
}

/* Adiciona uma linha a tabela de acordo com o número de colunas */
void table_pushline(Table *table, char *lineLabel, double *data){
	unsigned int i=0;
  if(!table->lines) table->lineLabels = str_createArray(0);
  table->matrix = (double **)realloc(table->matrix, (++table->lines)*sizeof(double *)); 
  table->matrix[table->lines-1] = (double *)malloc((table->columns)*sizeof(double));
  str_push(table->lineLabels, lineLabel);
  /* Supõe que data tem tamanho correto */
  for(;i<table->columns;i++) table->matrix[table->lines-1][i] = data[i]; 
}

int main(int argc, char **argv){
  unsigned int i,j;
  char *primeiralinha, *line;
  StringArray *str_line;
  Table *table = table_createTable();
  double number, *linedata;
  FILE *file;
  
  /* Leitura do arquivo */
  if(argc == 2) file = fopen(argv[1], "r");
  else file = stdin;
  
  if(file == NULL){
  	printf("[ erro ] Estou confuso\n");
  	return 0;
  }
  
  /* Leitura da primeira linha */
  primeiralinha = file_readLine(file);
  str_line = str_split(primeiralinha, '\t');
  str_popNULL(str_line);
  table_setColumns(table, str_line);
  str_clean(&str_line);
  printf("[ info ] Colunas a ler: ");
  str_printArray(table->columnNames);
  printf("\n");
  
  
  /* Leitura dos dados */
  if(table->columns > 0){
  	linedata = (double *)malloc((table->columns)*sizeof(double)); /* Vetor de double */
  	/* Lê as linhas até o talo */
    for(i=0;!feof(file);i++){
      line = file_readLine(file);
      str_line = str_split(line, '\t');
  		str_popNULL(str_line);
    
      /* Coloca na matriz de dados */
      if(str_line->length == table->columns) {
        for(j=0;j<table->columns;j++){
          sscanf(str_line->data[j], "%lf", &number);
          linedata[j] = number;
        }
        for(j=0;j<table->columns;j++){
          //printf("%lf ", table->matrix[i][j]);
        }
        //printf("\n");
        table_pushline(table, str_line->data[0], linedata); /* Segundo argumento é o label da linha */
      }
      else printf("[ warn ] Linha %u ignorada, número de colunas errado:\n\t-> \"%s\"\n",i--+1, line);
      free(line);
      str_clean(&str_line);
    }
    free(linedata);
    
    /* Análise de dados */
    if(table->lines){
      /* Vamos encontrar os máximos de cada coluna */
      double *max = (double *)calloc(table->columns, sizeof(double));
      double *tmax = (double *)malloc(table->columns*sizeof(double));
      double aux;
      for(i=0;i<table->lines;i++){
        for(j=0;j<table->columns;j++) {
          if(table->matrix[i][j] > max[j]){
            max[j] = table->matrix[i][j];
            sscanf(table->lineLabels->data[i], "%lf", &tmax[j]);
          }
          if(table->matrix[i][j] = max[j]){
            sscanf(table->lineLabels->data[i], "%lf", &aux);
            //printf("Em: %lf, o máximo %lf de %s se repetiu\n", aux, max[j], table->columnNames->data[j]);
          }
        }
      }
      for(j=0;j<table->columns;j++) 
      	printf("Máximo de %s: (%.10lf, %.10lf)\n", table->columnNames->data[j], tmax[j], max[j]);
      /* Limpamos a sujeira */
      free(max);
      max = NULL;
      free(tmax);
      tmax = NULL;
    }
    else printf("[ erro ] Não há linhas para ler\n");
  
  }
  else printf("[ erro ] Não há colunas para ler\n");
  
  /* Limpamos o resto */
  free(primeiralinha);
  table_clean(table);
  fclose(file);
  return 0;
}
