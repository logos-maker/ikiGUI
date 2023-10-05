#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Made so you can convert BMP pictures to include easily in your audio plugins.
// The command takes the name of the BMP file as the first argument.
// It can convert any file that ends with .something to a C array to include in projects.

int main(int argc, char *argv[]){
	FILE *fin, *fout;
	fin  = fopen(argv[1],"r");
	if(!fin){perror("\nFailed!\n");return 1; }

	char name_array[50];
	char name_file[50];
	strcpy(name_array,argv[1]);
	strcpy(name_file,argv[1]);

	char* end = strrchr(name_array, '.'); 
	*end = 0;
	if(!end){
		printf("no . was found in the file name.\");
		return 1;
	}

	fseek(fin, 0L, SEEK_END);
	uint64_t sz = ftell(fin);
	rewind(fin);
	
	end = strrchr(name_file, '.'); 
	end[1] = 'h';
	end[2] = 0;
	fout = fopen(name_file,"w");

	fprintf(fout,"const unsigned char %s[%ld] = {\n",name_array,sz);

	for(int i = 0 ; i < sz ; i++){
		uint8_t op;
		fread(&op,1,1,fin);
		switch(ftell(fin) & 3){
			case 0:fprintf(fout,"0x%02X,\n ",(uint8_t)op); break;
			case 1:fprintf(fout,"\t0x%02X,",(uint8_t)op); break;
			case 2:fprintf(fout,"0x%02X, ",(uint8_t)op); break;
			case 3:fprintf(fout,"0x%02X, ",(uint8_t)op); break;
		}
	}
	fprintf(fout,"\n}");

	fclose(fin);
	fclose(fout);

	return(0);
}