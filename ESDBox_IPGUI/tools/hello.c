#include <stdio.h>
int main(void){FILE*f=fopen("m:\\test\\ESDBox_IPGUI\\tools\\hello.txt","w");if(f){fprintf(f,"hello\n");fclose(f);printf("done\n");}return 0;}
