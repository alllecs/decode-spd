#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifndef __BAREBOX__
static inline int fls(int x)
{
       int r = 32;
 
        if (!x)
                return 0;
         if (!(x & 0xffff0000u)) {
                x <<= 16;
                r -= 16;
        }
        if (!(x & 0xff000000u)) {
                x <<= 8;
                r -= 8;
        }
        if (!(x & 0xf0000000u)) {
                x <<= 4;
                r -= 4;
        }
        if (!(x & 0xc0000000u)) {
                x <<= 2;
                r -= 2;
        }
        if (!(x & 0x80000000u)) {
                x <<= 1;
                r -= 1;
        }
        return r;
}
#endif

char *heights[] = {
	"<25.4",
	"25.4",
	"25.4 - 30.0",
	"30.0",
	"30.5",
	"> 30.5"
};

char *sdram_voltage_interface_level[] = {
	"TTL (5V tolerant)",
	"LVTTL (not 5V tolerant)",
	"HSTL 1.5V",
	"SSTL 3.3V",
	"SSTL 2.5V",
	"SSTL 1.8V"
};

char *ddr2_module_types[] = {
	"RDIMM (133.35 mm)",
	"UDIMM (133.25 mm)",
	"SO-DIMM (67.6 mm)",
	"Micro-DIMM (45.5 mm)",
	"Mini-RDIMM (82.0 mm)",
	"Mini-UDIMM (82.0 mm)"
};

char *refresh[] = {
	"15.625",
	"3.9",
	"7.8",
	"31.3",
	"62.5",
	"125"
};

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

double funct(uint8_t addr)
{
	double t;

	t = ((addr >> 4) * 0.1 + (addr & 0xf) * 0.01);

	return t;
}

double ddr2_sdram_atime(uint8_t addr)
{
	double t;

	t = ((addr >> 4) * 0.1 + (addr & 0xf) * 0.01);

	return t;
}

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

	printf("\n");
}

double ddr2_sdram_ctime(uint8_t byte)
{
	double ctime;

	ctime = byte >> 4;
	if ((byte & 0xf) <= 9) {
		ctime += (byte & 0xf) * 0.1;
	} else if ((byte & 0xf) == 10) {
		ctime += 0.25;
	} else if ((byte & 0xf) == 11) {
		ctime += 0.33;
	} else if ((byte & 0xf) == 12) {
		ctime += 0.66;
	} else if ((byte & 0xf) == 13) {
		ctime += 0.75;
	}
	return ctime;
}

int main (int argc, char *argv[])
{
	int highestCAS = 0;
	int cas[256];
	int i, i_i, k;
	int ddrclk, tbits, pcclk;
	int trcd, trp, tras;
	double ctime;
	int fd;
	uint8_t record[256];
	uint8_t parity;
	char *ref;

	if (argc != 2) {
		printf("Отсутствует или указано больше 1 аргумента\n");
		return 2;
	}

	fd = open(argv[1], O_RDONLY);

	if (fd == NULL) {
		perror("Ошибка при работе с файлом");
		return 3;
	}

	read(fd, &record[0], 256);
	close(fd);

	printf("Decoding EEPROM: %s\n\n", argv[1]);

	printf("---=== SPD EEPROM Information ===---\n");
	printf("EEPROM Checksum of bytes 0-62\t\t\t OK (0x%0X)\n", record[63]);
	printf("# of bytes written to SDRAM EEPROM\t\t %d\n", record[0]);
	printf("Total number of bytes in EEPROM\t\t\t %d\n", 1 << record[1]);

	if (record[2] < 11) {
		printf("Fundamental Memory type\t\t\t\t %s\n", type_list[record[2]]);
	} else {
		printf("Warning: unknown memory type (%02x)\n", record[2]);
	}
	printf("SPD Revision\t\t\t\t\t %x.%x\n", record[62] >> 4, record[62] & 0x0f);

	printf("\n---=== Memory Characteristics ===---\n");

	ctime = ddr2_sdram_ctime(record[9]);
	ddrclk = 2 * (1000 / ctime);
	tbits = (record[7] << 8) + record[6];
	if ((record[11] & 0x03) == 1) {
		tbits = tbits - 8;
	}
	pcclk = ddrclk * tbits / 8;
	pcclk = pcclk - (pcclk % 100);
	printf("Maximum module speed\t\t\t\t %d MHz (PC2-%d)\n", ddrclk, pcclk);
	i_i= (record[3] & 0x0f) + (record[4] & 0x0f) - 17;
	k = ((record[5] & 0x7) + 1) * record[17];

	if (i_i > 0 && i_i <= 12 && k > 0) {
		printf("Size\t\t\t\t\t\t %d MB\n", ((1 << i_i) * k));
	} else {
		printf("Size\t\t\t\t\t\t INVALID: %02x %02x %02x %02x\n", record[3], record[4], record[5], record[17]);
	}
	printf("Banks x Rows x Columns x Bits\t\t\t %d x %d x %d x %d\n", record[17], record[3], record[4], record[6]);
	printf("Ranks\t\t\t\t\t\t %d\n", (record[5] & 0x7) + 1);
	printf("SDRAM Device Width\t\t\t\t %d bits\n", record[13]);

	if ((record[5] >> 5) < 7) {
		printf("Module Height\t\t\t\t\t %s mm\n", heights[(record[5] >> 5)]);
	} else {
		printf("Error height\n");
	}
	printf("Module Type\t\t\t\t\t %s\n", ddr2_module_types[fls(record[20]) - 1]);
	printf("DRAM Package\t\t\t\t\t ");
	if ((record[5] & 0x10) == 1) {
		printf("Stack\n");
	} else {
		printf("Planar\n");
	}
	if (record[8] < 7) {
		printf("Voltage Interface Level\t\t\t\t %s\n", sdram_voltage_interface_level[record[8]]);
	} else {
		printf("Error Voltage Interface Level\n");
	}
	printf("Module Configuration Type \t\t\t ");

	parity = record[11] & 0x07;
	
	if (parity == 0) {
		printf("No Parity\n");
	}
	if ((parity & 0x03) == 0x01) {
		printf("Data Parity\n");
	}
	if (parity & 0x02) {
		printf("Data ECC\n");
	}
	if (parity & 0x04) {
                printf("Address/Command Parity\n");
        }

	if ((record[12] >> 7) == 1) {
		ref = "- Self Refresh";
	} else {
		ref = " ";
	}
	printf("Refresh Rate\t\t\t\t\t Reduced (%s us) %s\n", refresh[record[12] & 0x7f], ref);
	printf("Supported Burst Lengths\t\t\t\t %d, %d\n", record[16] & 4, record[16] & 8);

	trcd = ((record[29] >> 2) + ((record[29] & 3) * 0.25)) / ctime;
	trp = ((record[27] >> 2) + ((record[27] & 3) * 0.25)) / ctime;
	tras = record[30] / ctime;

	for (i_i = 2; i_i < 7; i_i++) {
		if (record[18] & (1 << i_i)) {
			highestCAS = i_i;
			cas[highestCAS]++;
		}
	}
	printf("Supported CAS Latencies (tCL)\t\t\t %dT\n", highestCAS);
	printf("tCL-tRCD-tRP-tRAS\t\t\t\t %d-%d-%d-%d as DDR2-%d\n", highestCAS, trcd, trp, tras, ddrclk);
	printf("Minimum Cycle Time\t\t\t\t %0.2lf ns at CAS %d\n", ctime, highestCAS);
	printf("Maximum Access Time\t\t\t\t %0.2lf ns at CAS %d\n", ddr2_sdram_atime(record[10]), highestCAS);
	printf("Maximum Cycle Time (tCK max)\t\t\t %0.2lf ns\n", (record[43] >> 4) * 1.0 + (record[43] & 0x0f) * 0.1);

	printf("\n---=== Timing Parameters ===---\n");
	printf("Address/Command Setup Time Before Clock (tIS)\t %0.2lf ns\n", funct(record[32]));
	printf("Address/Command Hold Time After Clock (tIH)\t %0.2lf ns\n", funct(record[33]));
	printf("Data Input Setup Time Before Strobe (tDS)\t %0.2lf ns\n", funct(record[34]));
	printf("Data Input Hold Time After Strobe (tDH)\t\t %0.2lf ns\n", funct(record[35]));

	printf("Minimum Row Precharge Delay (tRP)\t\t %0.2lf ns\n", (record[27] & 0xfc) / 4.0);
	printf("Minimum Row Active to Row Active Delay (tRRD)\t %0.2lf ns\n", record[28] / 4.0);
	printf("Minimum RAS# to CAS# Delay (tRCD)\t\t %0.2lf ns\n", (record[29] & 0xfc) / 4.0);
	printf("Minimum RAS# Pulse Width (tRAS)\t\t\t %0.2lf ns\n", (record[30] & 0xfc) + (record[30] & 0x3) * 1.0);

	printf("Write Recovery Time (tWR)\t\t\t %0.2lf ns\n", record[36] / 4.0);
	printf("Minimum Write to Read CMD Delay (tWTR)\t\t %0.2lf ns\n", record[37] / 4.0);
	printf("Minimum Read to Pre-charge CMD Delay (tRTP)\t %0.2lf ns\n", record[38] / 4.0);
	printf("Minimum Active to Auto-refresh Delay (tRC)\t %0.2lf ns\n", record[41] * 1.0);
	printf("Minimum Recovery Delay (tRFC)\t\t\t %0.2lf ns\n", record[42] * 1.0);
	printf("Maximum DQS to DQ Skew (tDQSQ)\t\t\t %0.2lf ns\n", record[44] * 0.01);
	printf("Maximum Read Data Hold Skew (tQHS)\t\t %0.2lf ns\n", record[45] * 0.01);

	printf("\n---=== Manufacturing Information ===---\n");

	printf("Manufacturer JEDEC ID\t\t\t\t");
	for (i = 64; i < 72; i++) {
		printf(" %02x", record[i]);
	}
	printf("\n");
	if (record[72]) {
		printf("Manufacturing Location Code\t\t\t 0x%02x\n", record[72]);
	}
	printf("Part Number\t\t\t\t\t ");
	for (i = 73; i < 91; i++) {
		if (record[i] >= 32 && record[i] < 127) {
			printf("%c", record[i]);
		} else {
			printf("%d", record[i]);
		}
	}
	printf("\n");
	printf("Manufacturing Date\t\t\t\t 20%d-W%d\n", record[93], record[94]);
	printf("Assembly Serial Number\t\t\t\t 0x");
	for (i = 95; i < 99; i++) {
		printf("%02X", record[i]);
	}
	printf("\n\n");

	return 0;
}
