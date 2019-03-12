#include <stdio.h>
#include <string.h>

#define MAX 100
int main(void)
{
char nome1[MAX+1], nome2[MAX+1], palavra1[MAX+1], palavra2[MAX+1];
int i1, i2;
FILE *pt1, *pt2, *ptr;

pt1 = fopen("arquivo1.data", "r");
pt2 = fopen("arquivo2.txt", "r");
if (pt1 != NULL && pt2 != NULL) {
ptr = fopen("arquivo3.txt", "w");
if (ptr != NULL) {
i1 = fscanf(pt1, "%s", palavra1);
i2 = fscanf(pt2, "%s", palavra2);

while (i1 != EOF && i2 != EOF)
{
    if(strcmp(palavra1,palavra2)<0)
    {
    fprintf(ptr, "%s ", palavra1);
    i1 = fscanf(pt1, "%s", ptr);    	
    }    
    else
      fprintf(ptr, "%s ", palavra2);
      i1 = fscanf(pt2, "%s", ptr);
}
	while (i1 != EOF) 
	{
		fprintf(ptr, "%s ", palavra1);
		i1 = fscanf(pt1, "%s", palavra1);
	}
	while (i2 != EOF) 
	{
		fprintf(ptr, "%s ", palavra2);
		i2 = fscanf(pt2, "%s", palavra2);
	}


fprintf(ptr, "\n");
fclose(pt1);
fclose(pt2);
fclose(ptr);
}
else
printf("Não é possível abrir um arquivo para escrita\n");
}
else
printf("Não é possível abrir os arquivos para leitura\n");
return 0;
}



