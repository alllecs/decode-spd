#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>


char *type_list[] = {
	"Reserved",
	"FPM DRAM",
	"EDO",
	"Pipelined Nibble",
	"SDR SDRAM",
	"Multiplexed ROM",
	"DDR SGRAM",
	"DDR SDRAM",
	"DDR2 SDRAM",
	"FB-DIMM",
	"FB-DIMM Probe",
	"DDR3 SDRAM"
};

void dump(uint8_t *addr, int len)
{
	int i;
	printf("\t 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n\n");
	for (i = 0; i < len; i++) {
		if ((i % 0x10) == 0x00) {
			printf("%07x:", i);
		}
		printf(" %02x", addr[i]);
		if ((i % 0x10) == 0xf) {
			printf("\n");
		}
	}
}

int main (int argc, char *argv[])
{
	FILE *fp;
	int counter;
	int i;
	uint8_t record[256];

	if (argc != 2) {
		printf("Отсутствует или указано больше 1 аргумента\n");
		return 2;
	}

	fp = fopen(argv[1], "rb");

	if (fp == NULL) {
		perror("Ошибка при работе с файлом");
		return 3;
	}

	fread(&record[0], 256, 1, fp);
	dump(&record[0], 256);


	printf("---=== SPD EEPROM Information ===---\n");

	printf("EEPROM Checksum of bytes 0-62\t\t %0X\n", record[63]);

	printf("# of bytes written to SDRAM EEPROM\t %0x\n", record[0]);

	printf("Total number of bytes in EEPROM\t\t %.0f\n", pow(2, record[1]));


	if (record[2] < 11) {
	printf("Fundamental Memory type\t\t\t %s\n", type_list[record[2]]);
	} else {
		printf("Warning: unknown memory type (%02x)\n", record[2]);
	}

	printf("SPD Revision\t\t\t\t %0x\n", record[62]);

	fclose(fp);

	return 0;

}
