#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void read_file(FILE* in, FILE* out);
void process_line(int* line, FILE* out);


int main (int argv, char *argc[])
{
	if(argv != 3)
	{
		printf("Incorrect number of arguments.\nUsage: ./a.out <input_file> <output_file>\n");
		return 0;
	}
	FILE *inFile, *outFile;
	//printf("open files\n");
	inFile = fopen(argc[1], "r");
	if(inFile == NULL)
	{
		printf("Error opening the input file.\n");
		return 0;
	}

	outFile = fopen(argc[2], "w");
	if(outFile == NULL)
	{
		printf("Error opening the output file.\n");
		return 0;
	}
	//printf("read files\n");
	read_file(inFile, outFile);

	return 0;
}

void read_file(FILE* in, FILE* out)
{
	int comm[32];
	char operation[10];
	char reg1[10], reg2[10], reg3[10];
	size_t nbytes = 0;
        int linelen=0;
	while (fscanf(in, "%s %s %s %s", operation, reg1, reg2, reg3)  != -1)
	{
		//printf("op: %s\n", operation);
		int x;
		for(x = 0; x < 32; x++)
			comm[x] = 0;
		if(strcmp(operation, "addiu") == 0)
		{
			comm[2] = 1;
			comm[5] = 1;
			//reg1 -> rt index 11-15
			if(reg1[1] == 'a')
			{
				comm[13] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg1[1] == 'v')
			{
				comm[14] = 1;
				if(reg1[2] == '1')
					comm[15] = 1;
			}
			else if(reg1[1] == 's')
			{
				comm[11] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					case '4':
						comm[13] = 1;
						break;
					case '5':
						comm[13] = 1;
						comm[15] = 1;
						break;
					case '6':
						comm[13] = 1;
						comm[14] = 1;
						break;
					case '7':
						comm[13] = 1;
						comm[14] = 1;
						comm[15] = 1;
					default:
						break;
				};
			}
			//reg2 -> rs index 6-10
			if(reg2[1] == 'a')
			{
				comm[8] = 1;
				switch(reg2[2])
				{
					case '1':
						comm[10] = 1;
						break;
					case '2':
						comm[9] = 1;
						break;
					case '3':
						comm[9] = 1;
						comm[10] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg2[1] == 'v')
			{
				comm[9] = 1;
				if(reg2[2] == '1')
					comm[10] = 1;
			}
			else if(reg2[1] == 's')
			{
				comm[6] = 1;
				switch(reg2[2])
				{
					case '1':
						comm[10] = 1;
						break;
					case '2':
						comm[9] = 1;
						break;
					case '3':
						comm[9] = 1;
						comm[10] = 1;
						break;
					case '4':
						comm[8] = 1;
						break;
					case '5':
						comm[8] = 1;
						comm[10] = 1;
						break;
					case '6':
						comm[8] = 1;
						comm[9] = 1;
						break;
					case '7':
						comm[8] = 1;
						comm[9] = 1;
						comm[10] = 1;
					default:
						break;
				};
			}
			//reg3 -> immediate, bits 16-31
			int imm = strtol(reg3, NULL, 16);

			int i, c = 1;
			for(i = 0; i < 16; i++)
			{ 
				if(imm & c)
				{
					comm[31 - i] = 1;
				}
				else
					comm[31 - i] = 0;
				c = c * 2;
			}
			if(imm < 0)
			{
				int i;
				for(i = 16; i < 32; i++)
				{
					if(comm[i] = 0)
						comm[i] = 1;
					else
						comm[i] = 0;
				}
				for(i = 31; i > 15; i--)
				{
					if(comm[i] = 0)
					{
						comm[i] = 1;
						break;
					}
					else
						comm[i] = 0;
				}
			
			}
			
		}
		else if(strcmp(operation, "add") == 0)
		{
			comm[26] = 1;
			//reg1 -> rd index 16-20
			if(reg1[1] == 'a')
			{
				comm[18] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[20] = 1;
						break;
					case '2':
						comm[19] = 1;
						break;
					case '3':
						comm[19] = 1;
						comm[20] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg1[1] == 'v')
			{
				comm[18] = 1;
				if(reg1[2] == '1')
					comm[19] = 1;
			}
			else if(reg1[1] == 's')
			{
				comm[16] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[20] = 1;
						break;
					case '2':
						comm[19] = 1;
						break;
					case '3':
						comm[19] = 1;
						comm[20] = 1;
						break;
					case '4':
						comm[18] = 1;
						break;
					case '5':
						comm[18] = 1;
						comm[20] = 1;
						break;
					case '6':
						comm[18] = 1;
						comm[19] = 1;
						break;
					case '7':
						comm[18] = 1;
						comm[19] = 1;
						comm[20] = 1;
					default:
						break;
				};
			}
			//reg2 -> rs index 6-10
			if(reg2[1] == 'a')
			{
				comm[8] = 1;
				switch(reg2[2])
				{
					case '1':
						comm[10] = 1;
						break;
					case '2':
						comm[9] = 1;
						break;
					case '3':
						comm[9] = 1;
						comm[10] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg2[1] == 'v')
			{
				comm[9] = 1;
				if(reg2[2] == '1')
					comm[10] = 1;
			}
			else if(reg2[1] == 's')
			{
				comm[6] = 1;
				switch(reg2[2])
				{
					case '1':
						comm[10] = 1;
						break;
					case '2':
						comm[9] = 1;
						break;
					case '3':
						comm[9] = 1;
						comm[10] = 1;
						break;
					case '4':
						comm[8] = 1;
						break;
					case '5':
						comm[8] = 1;
						comm[10] = 1;
						break;
					case '6':
						comm[8] = 1;
						comm[9] = 1;
						break;
					case '7':
						comm[8] = 1;
						comm[9] = 1;
						comm[10] = 1;
					default:
						break;
				};
			}
			//reg3 -> rt bits 11-15
			if(reg3[1] == 'a')
			{
				comm[13] = 1;
				switch(reg3[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg3[1] == 'v')
			{
				comm[14] = 1;
				if(reg3[2] == '1')
					comm[15] = 1;
			}
			else if(reg3[1] == 's')
			{
				comm[11] = 1;
				switch(reg3[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					case '4':
						comm[13] = 1;
						break;
					case '5':
						comm[13] = 1;
						comm[15] = 1;
						break;
					case '6':
						comm[13] = 1;
						comm[14] = 1;
						break;
					case '7':
						comm[13] = 1;
						comm[14] = 1;
						comm[15] = 1;
					default:
						break;
				};
			}
		}
		else if(strcmp(operation, "bne") == 0)
		{
			comm[3] = 1;
			comm[5] = 1;
			//reg1 -> rs bits 6-10
			if(reg3[1] == 'a')
			{
				comm[13] = 1;
				switch(reg3[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg3[1] == 'v')
			{
				comm[14] = 1;
				if(reg3[2] == '1')
					comm[15] = 1;
			}
			else if(reg3[1] == 's')
			{
				comm[11] = 1;
				switch(reg3[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					case '4':
						comm[13] = 1;
						break;
					case '5':
						comm[13] = 1;
						comm[15] = 1;
						break;
					case '6':
						comm[13] = 1;
						comm[14] = 1;
						break;
					case '7':
						comm[13] = 1;
						comm[14] = 1;
						comm[15] = 1;
					default:
						break;
				};
			}	if(reg1[1] == 'a')
			{
				comm[8] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[10] = 1;
						break;
					case '2':
						comm[9] = 1;
						break;
					case '3':
						comm[9] = 1;
						comm[10] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg1[1] == 'v')
			{
				comm[9] = 1;
				if(reg1[2] == '1')
					comm[10] = 1;
			}
			else if(reg1[1] == 's')
			{
				comm[6] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[10] = 1;
						break;
					case '2':
						comm[9] = 1;
						break;
					case '3':
						comm[9] = 1;
						comm[10] = 1;
						break;
					case '4':
						comm[8] = 1;
						break;
					case '5':
						comm[8] = 1;
						comm[10] = 1;
						break;
					case '6':
						comm[8] = 1;
						comm[9] = 1;
						break;
					case '7':
						comm[8] = 1;
						comm[9] = 1;
						comm[10] = 1;
					default:
						break;
				};
			}
			//reg2 -> rt bits 11-15
			if(reg2[1] == 'a')
			{
				comm[13] = 1;
				switch(reg2[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg2[1] == 'v')
			{
				comm[14] = 1;
				if(reg2[2] == '1')
					comm[15] = 1;
			}
			else if(reg2[1] == 's')
			{
				comm[11] = 1;
				switch(reg2[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					case '4':
						comm[13] = 1;
						break;
					case '5':
						comm[13] = 1;
						comm[15] = 1;
						break;
					case '6':
						comm[13] = 1;
						comm[14] = 1;
						break;
					case '7':
						comm[13] = 1;
						comm[14] = 1;
						comm[15] = 1;
					default:
						break;
				};
			}
			int neg = 0;
			int offset = strtol(reg3, NULL, 16);
			if(offset < 0)
			{
				offset = offset * -1;
				neg = 1;
			}
			//printf("off:%d, neg:%d\n", offset, neg);
			int i, c = 1;
			for(i = 0; i < 16; i++)
			{ 
				if(offset & c)
				{
					comm[31 - i] = 1;
				}
				else
					comm[31 - i] = 0;
				c = c * 2;
			}
			if(neg == 1)
			{
				int i;
				for(i = 16; i < 32; i++)
				{
					if(comm[i] == 0)
						comm[i] = 1;
					else
						comm[i] = 0;
				}
				for(i = 31; i > 15; i--)
				{
					if(comm[i] == 0)
					{
						comm[i] = 1;
						break;
					}
					else
						comm[i] = 0;
				}
			
			}
		}
		else if(strcmp(operation, "sub") == 0)
		{
			comm[30] = 1;
			comm[26] = 1;
			//reg1 -> rd index 16-20
			if(reg1[1] == 'a')
			{
				comm[18] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[20] = 1;
						break;
					case '2':
						comm[19] = 1;
						break;
					case '3':
						comm[19] = 1;
						comm[20] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg1[1] == 'v')
			{
				comm[18] = 1;
				if(reg1[2] == '1')
					comm[19] = 1;
			}
			else if(reg1[1] == 's')
			{
				comm[16] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[20] = 1;
						break;
					case '2':
						comm[19] = 1;
						break;
					case '3':
						comm[19] = 1;
						comm[20] = 1;
						break;
					case '4':
						comm[18] = 1;
						break;
					case '5':
						comm[18] = 1;
						comm[20] = 1;
						break;
					case '6':
						comm[18] = 1;
						comm[19] = 1;
						break;
					case '7':
						comm[18] = 1;
						comm[19] = 1;
						comm[20] = 1;
					default:
						break;
				};
			}
			//reg2 -> rs index 6-10
			if(reg2[1] == 'a')
			{
				comm[8] = 1;
				switch(reg2[2])
				{
					case '1':
						comm[10] = 1;
						break;
					case '2':
						comm[9] = 1;
						break;
					case '3':
						comm[9] = 1;
						comm[10] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg2[1] == 'v')
			{
				comm[9] = 1;
				if(reg2[2] == '1')
					comm[10] = 1;
			}
			else if(reg2[1] == 's')
			{
				comm[6] = 1;
				switch(reg2[2])
				{
					case '1':
						comm[10] = 1;
						break;
					case '2':
						comm[9] = 1;
						break;
					case '3':
						comm[9] = 1;
						comm[10] = 1;
						break;
					case '4':
						comm[8] = 1;
						break;
					case '5':
						comm[8] = 1;
						comm[10] = 1;
						break;
					case '6':
						comm[8] = 1;
						comm[9] = 1;
						break;
					case '7':
						comm[8] = 1;
						comm[9] = 1;
						comm[10] = 1;
					default:
						break;
				};
			}
			//reg3 -> rt bits 11-15
			if(reg3[1] == 'a')
			{
				comm[13] = 1;
				switch(reg3[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg3[1] == 'v')
			{
				comm[14] = 1;
				if(reg3[2] == '1')
					comm[15] = 1;
			}
			else if(reg3[1] == 's')
			{
				comm[11] = 1;
				switch(reg3[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					case '4':
						comm[13] = 1;
						break;
					case '5':
						comm[13] = 1;
						comm[15] = 1;
						break;
					case '6':
						comm[13] = 1;
						comm[14] = 1;
						break;
					case '7':
						comm[13] = 1;
						comm[14] = 1;
						comm[15] = 1;
					default:
						break;
				};
			} 
		}
		else if(strcmp(operation, "sw") == 0)
		{
			comm[0] = 1;
			comm[2] = 1;
			comm[4] = 1;
			comm[5] = 1;
			//reg1 - rt, bits 11 - 15
			if(reg1[1] == 'a')
			{
				comm[13] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg1[1] == 'v')
			{
				comm[14] = 1;
				if(reg1[2] == '1')
					comm[15] = 1;
			}
			else if(reg3[1] == 's')
			{
				comm[11] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					case '4':
						comm[13] = 1;
						break;
					case '5':
						comm[13] = 1;
						comm[15] = 1;
						break;
					case '6':
						comm[13] = 1;
						comm[14] = 1;
						break;
					case '7':
						comm[13] = 1;
						comm[14] = 1;
						comm[15] = 1;
					default:
						break;
				};
			}
			int k = 0;
			int offset;
			while(reg2[k] != '(')
			{
				k++;
			}
			int start = k + 1;
			int len;
			while(reg2[k] != ')')
			{
			    k++;
			    len ++;
			}
			int* temp = malloc(sizeof(int)*len);
			memcpy(&reg2[start], temp, len);
			int i, c;
			for(i = 0; i < 16; i++)
			{
				//c = pow(2, i);
				c = (2 ^ i);
				if(*temp & c)
					comm[31 - i] = 1;
				else
					comm[31 - i] = 0;
			}
			free(temp);
		}
		else if(strcmp(operation, "ld") == 0)
		{
			comm[0] = 1;
			comm[4] = 1;
			comm[5] = 1;
			//reg1 - rt, bits 11 - 15
			if(reg1[1] == 'a')
			{
				comm[13] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg1[1] == 'v')
			{
				comm[14] = 1;
				if(reg1[2] == '1')
					comm[15] = 1;
			}
			else if(reg3[1] == 's')
			{
				comm[11] = 1;
				switch(reg1[2])
				{
					case '1':
						comm[15] = 1;
						break;
					case '2':
						comm[14] = 1;
						break;
					case '3':
						comm[14] = 1;
						comm[15] = 1;
						break;
					case '4':
						comm[13] = 1;
						break;
					case '5':
						comm[13] = 1;
						comm[15] = 1;
						break;
					case '6':
						comm[13] = 1;
						comm[14] = 1;
						break;
					case '7':
						comm[13] = 1;
						comm[14] = 1;
						comm[15] = 1;
					default:
						break;
				};
			}
			int k = 0;
			int offset = strtol(&reg2[k], NULL, 16);
			while(reg2[k + 1] != '(')
			{
				offset = offset * 10 + strtol(&reg2[k + 1], NULL, 16);
				k++;
			}
			k += 3;
			if(reg2[k] == 'a')
			{
				comm[8] = 1;
				switch(reg2[k+1])
				{
					case '1':
						comm[10] = 1;
						break;
					case '2':
						comm[9] = 1;
						break;
					case '3':
						comm[9] = 1;
						comm[10] = 1;
						break;
					default:
						break;
				};
			}
			else if(reg2[k+1] == 'v')
			{
				comm[9] = 1;
				if(reg2[2] == '1')
					comm[10] = 1;
			}
			else if(reg2[k+1] == 's')
			{
				comm[6] = 1;
				switch(reg2[2])
				{
					case '1':
						comm[10] = 1;
						break;
					case '2':
						comm[9] = 1;
						break;
					case '3':
						comm[9] = 1;
						comm[10] = 1;
						break;
					case '4':
						comm[8] = 1;
						break;
					case '5':
						comm[8] = 1;
						comm[10] = 1;
						break;
					case '6':
						comm[8] = 1;
						comm[9] = 1;
						break;
					case '7':
						comm[8] = 1;
						comm[9] = 1;
						comm[10] = 1;
					default:
						break;
				};
			}
			int i, c;
			for(i = 0; i < 16; i++)
			{
				//c = pow(2, i);
				c = (2 ^ i);
				if(offset & c)
					comm[31 - i] = 1;
				else
					comm[31 - i] = 0;
			}
		}
		else if(strcmp(operation, "syscall") == 0)
		{
			comm[29] = 1;
			comm[28] = 1;
		}
		process_line(comm, out);		
	}
	return;
}

void process_line(int* line, FILE* out)
{	
	int i;
	int scale = 1;
	int comm = 0;
	for(i = 31; i >= 0; i--)
	{
		comm += line[i]*scale;
		scale = scale * 2;
	}
	fprintf(out, "%08x\n", comm);
	return;
}
	
	
