/*
 * This program is decoding and printing SPD contents
 * in human readable format
 * As an argument program, you must specify the file name.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Copyright (C) 2015 Alexander Smirnov <alllecs@yandex.ru>
 *
 * Originally from https://github.com/groeck/i2c-tools/blob/master/eeprom/decode-dimms
 */

#include <unistd.h>
#include <fcntl.h>
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

static char *heights[] = {
	"<25.4",
	"25.4",
	"25.4 - 30.0",
	"30.0",
	"30.5",
	"> 30.5"
};

static char *sdram_voltage_interface_level[] = {
	"TTL (5V tolerant)",
	"LVTTL (not 5V tolerant)",
	"HSTL 1.5V",
	"SSTL 3.3V",
	"SSTL 2.5V",
	"SSTL 1.8V"
};

static char *ddr2_module_types[] = {
	"RDIMM (133.35 mm)",
	"UDIMM (133.25 mm)",
	"SO-DIMM (67.6 mm)",
	"Micro-DIMM (45.5 mm)",
	"Mini-RDIMM (82.0 mm)",
	"Mini-UDIMM (82.0 mm)"
};

static char *refresh[] = {
	"15.625",
	"3.9",
	"7.8",
	"31.3",
	"62.5",
	"125"
};

static char *type_list[] = {
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

static int funct(uint8_t addr)
{
	int t;

	t = ((addr >> 4) * 10 + (addr & 0xf));

	return t;
}

static void dump(uint8_t *addr, int len)
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

static int des(uint8_t byte)
{
	int k;
	
	k = (byte & 0x3) * 10 / 4;

	return k;
}

static int integ(uint8_t byte)
{
	int k;

	k = (byte >> 2);

	return k;
}

static int ddr2_sdram_ctime(uint8_t byte)
{
	int ctime;

	ctime = (byte >> 4) * 100;
	if ((byte & 0xf) <= 9) {
		ctime += (byte & 0xf) * 10;
	} else if ((byte & 0xf) == 10) {
		ctime += 25;
	} else if ((byte & 0xf) == 11) {
		ctime += 33;
	} else if ((byte & 0xf) == 12) {
		ctime += 66;
	} else if ((byte & 0xf) == 13) {
		ctime += 75;
	}
	return ctime;
}

static void ddr2_spd_prin_result(uint8_t *record)
{
	int highestCAS = 0;
	int cas[256];
	int i, i_i, k, x, y;
	int ddrclk, tbits, pcclk;
	int trcd, trp, tras;
	int ctime;
	uint8_t parity;
	char *ref;

	ctime = ddr2_sdram_ctime(record[9]);
	ddrclk = 2 * (1000 / ctime) * 100;
	tbits = (record[7] << 8) + record[6];
	if ((record[11] & 0x03) == 1)
		tbits = tbits - 8;

	pcclk = ddrclk * tbits / 8;
	pcclk = pcclk - (pcclk % 100);
	i_i = (record[3] & 0x0f) + (record[4] & 0x0f) - 17;
	k = ((record[5] & 0x7) + 1) * record[17];
	trcd = ((record[29] >> 2) + ((record[29] & 3) * 0.25)) / ctime * 100;
	trp = ((record[27] >> 2) + ((record[27] & 3) * 0.25)) / ctime * 100;
	tras = record[30] * 100 / ctime ;
	x = (int)(ctime / 100);
	y = (ctime - (int)((ctime / 100) * 100)) / 10;

	for (i_i = 2; i_i < 7; i_i++) {
		if (record[18] & (1 << i_i)) {
			highestCAS = i_i;
			cas[highestCAS]++;
		}
	}

	printf("---=== SPD EEPROM Information ===---\n");
	printf("%-50s OK (0x%0X)\n", "EEPROM Checksum of bytes 0-62", record[63]);
	printf("%-50s %d\n", "# of bytes written to SDRAM EEPROM", record[0]);
	printf("%-50s %d\n", "Total number of bytes in EEPROM", 1 << record[1]);

	if (record[2] < 11)
		printf("%-50s %s\n", "Fundamental Memory type", type_list[record[2]]);
	else
		printf("%-50s (%02x)\n", "Warning: unknown memory type", record[2]);
	printf("%-50s %x.%x\n", "SPD Revision", record[62] >> 4, record[62] & 0x0f);

	printf("\n---=== Memory Characteristics ===---\n");
	printf("%-50s %d MHz (PC2-%d)\n", "Maximum module speed", ddrclk, pcclk);
	if (i_i > 0 && i_i <= 12 && k > 0)
		printf("%-50s %d MB\n", "Size", ((1 << i_i) * k));
	else
		printf("%-50s INVALID: %02x %02x %02x %02x\n", "Size", record[3], record[4], record[5], record[17]);

	printf("%-50s %d x %d x %d x %d\n", "Banks x Rows x Columns x Bits", record[17], record[3], record[4], record[6]);
	printf("%-50s %d\n", "Ranks", (record[5] & 0x7) + 1);
	printf("%-50s %d bits\n", "SDRAM Device Width", record[13]);

	if ((record[5] >> 5) < 7)
		printf("%-50s %s mm\n", "Module Height", heights[(record[5] >> 5)]);
	else
		printf("Error height\n");

	printf("%-50s %s\n", "Module Type", ddr2_module_types[fls(record[20]) - 1]);
	printf("%-50s ", "DRAM Package ");
	if ((record[5] & 0x10) == 1)
		printf("Stack\n");
	else
		printf("Planar\n");

	if (record[8] < 7)
		printf("%-50s %s\n", "Voltage Interface Level", sdram_voltage_interface_level[record[8]]);
	else
		printf("Error Voltage Interface Level\n");

	printf("%-50s ", "Module Configuration Type ");

	parity = record[11] & 0x07;
	if (parity == 0)
		printf("No Parity\n");

	if ((parity & 0x03) == 0x01)
		printf("Data Parity\n");
	if (parity & 0x02)
		printf("Data ECC\n");

	if (parity & 0x04)
		printf("Address/Command Parity\n");

	if ((record[12] >> 7) == 1)
		ref = "- Self Refresh";
	else
		ref = " ";

	printf("%-50s Reduced (%s us) %s\n", "Refresh Rate", refresh[record[12] & 0x7f], ref);
	printf("%-50s %d, %d\n", "Supported Burst Lengths", record[16] & 4, record[16] & 8);

	printf("%-50s %dT\n", "Supported CAS Latencies (tCL)", highestCAS);
	printf("%-50s %d-%d-%d-%d as DDR2-%d\n", "tCL-tRCD-tRP-tRAS", highestCAS, trcd, trp, tras, ddrclk);
	printf("%-50s %d.%d ns at CAS %d\n", "Minimum Cycle Time", x, y, highestCAS);
	printf("%-50s 0.%d%d ns at CAS %d\n", "Maximum Access Time", (record[10] >> 4), (record[10] & 0xf), highestCAS);
	printf("%-50s %d ns\n", "Maximum Cycle Time (tCK max)", (record[43] >> 4) + (record[43] & 0x0f));

	printf("\n---=== Timing Parameters ===---\n");
	printf("%-50s 0.%d ns\n", "Address/Command Setup Time Before Clock (tIS)", (funct(record[32])));
	printf("%-50s 0.%d ns\n", "Address/Command Hold Time After Clock (tIH)", (funct(record[33])));
	printf("%-50s 0.%d%d ns\n", "Data Input Setup Time Before Strobe (tDS)", (record[34] >> 4), (record[34] & 0xf));
	printf("%-50s 0.%d%d ns\n", "Data Input Hold Time After Strobe (tDH)", (record[35] >> 4), (record[35] & 0xf));

	printf("%-50s %d.%d ns\n", "Minimum Row Precharge Delay (tRP)", integ(record[27]), des(record[27]));
	printf("%-50s %d.%d ns\n", "Minimum Row Active to Row Active Delay (tRRD)", integ(record[28]), des(record[28]));
	printf("%-50s %d.%d ns\n", "Minimum RAS# to CAS# Delay (tRCD)", integ(record[29]), des(record[29]));
	printf("%-50s %d ns\n", "Minimum RAS# Pulse Width (tRAS)", ((record[30] & 0xfc) + (record[30] & 0x3)));
	printf("%-50s %d.%d ns\n", "Write Recovery Time (tWR)", integ(record[36]), des(record[36]));
	printf("%-50s %d.%d ns\n", "Minimum Write to Read CMD Delay (tWTR)", integ(record[37]), des(record[37]));
	printf("%-50s %d.%d ns\n", "Minimum Read to Pre-charge CMD Delay (tRTP)", integ(record[38]), des(record[38]));
	printf("%-50s %d ns\n", "Minimum Active to Auto-refresh Delay (tRC)", record[41]);
	printf("%-50s %d ns\n", "Minimum Recovery Delay (tRFC)", record[42]);
	printf("%-50s 0.%d ns\n", "Maximum DQS to DQ Skew (tDQSQ)", record[44]);
	printf("%-50s 0.%d ns\n", "Maximum Read Data Hold Skew (tQHS)", record[45]);

	printf("\n---=== Manufacturing Information ===---\n");

	printf("%-50s", "Manufacturer JEDEC ID");
	for (i = 64; i < 72; i++) {
		printf(" %02x", record[i]);
	}
	printf("\n");
	if (record[72])
		printf("%-50s 0x%02x\n", "Manufacturing Location Code", record[72]);

	printf("%-50s ", "Part Number");
	for (i = 73; i < 91; i++) {
		if (record[i] >= 32 && record[i] < 127)
			printf("%c", record[i]);
		else
			printf("%d", record[i]);
	}
	printf("\n");
	printf("%-50s 20%d-W%d\n", "Manufacturing Date", record[93], record[94]);
	printf("%-50s 0x", "Assembly Serial Number");
	for (i = 95; i < 99; i++) {
		printf("%02X", record[i]);
	}
}


int main (int argc, char *argv[])
{
	int fd;
	uint8_t record[256];

	if (argc != 2) {
		printf("Not enough or more than one arguments to continue\n");
		return 2;
	}

	fd = open(argv[1], O_RDONLY);

	if (fd == 0) {
		perror("Error open file");
		return 3;
	}

	read(fd, &record[0], 256);
	close(fd);

	printf("Decoding EEPROM: %s\n\n", argv[1]);

	ddr2_spd_prin_result(record);
	printf("\n\n");

	return 0;
}
