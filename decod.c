#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

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

char *size[] = {
	"1 GiB",
	"2 GiB",
	"4 GiB",
	"8 GiB",
	"16 GiB",
	"128 MiB",
	"256 MiB",
	"512 MiB"
};

char *vendors[] = {
 "Cirrus Logic", "National Instruments", "ILC Data Device", "Alcatel Mietec",
 "Micro Linear", "Univ. of NC", "JTAG Technologies", "BAE Systems",
 "Nchip", "Galileo Tech", "Bestlink Systems", "Graychip",
 "GENNUM", "VideoLogic", "Robert Bosch", "Chip Express",
 "DATARAM", "United Microelec Corp.", "TCSI", "Smart Modular",
 "Hughes Aircraft", "Lanstar Semiconductor", "Qlogic", "Kingston",
 "Music Semi", "Ericsson Components", "SpaSE", "Eon Silicon Devices",
 "Programmable Micro Corp", "DoD", "Integ. Memories Tech.", "Corollary Inc.",
 "Dallas Semiconductor", "Omnivision", "EIV(Switzerland)", "Novatel Wireless",
 "Zarlink (former Mitel)", "Clearpoint", "Cabletron", "STEC (former Silicon Technology)",
 "Vanguard", "Hagiwara Sys-Com", "Vantis", "Celestica",
 "Century", "Hal Computers", "Rohm Company Ltd.", "Juniper Networks",
 "Libit Signal Processing", "Mushkin Enhanced Memory", "Tundra Semiconductor", "Adaptec Inc.",
 "LightSpeed Semi.", "ZSP Corp.", "AMIC Technology", "Adobe Systems",
 "Dynachip", "PNY Electronics", "Newport Digital", "MMC Networks",
 "T Square", "Seiko Epson", "Broadcom", "Viking Components",
 "V3 Semiconductor", "Flextronics (former Orbit)", "Suwa Electronics", "Transmeta",
 "Micron CMS", "American Computer & Digital Components Inc", "Enhance 3000 Inc", "Tower Semiconductor",
"CPU Design", "Price Point", "Maxim Integrated Product", "Tellabs",
 "Centaur Technology", "Unigen Corporation", "Transcend Information", "Memory Card Technology",
 "CKD Corporation Ltd.", "Capital Instruments, Inc.", "Aica Kogyo, Ltd.", "Linvex Technology",
 "MSC Vertriebs GmbH", "AKM Company, Ltd.", "Dynamem, Inc.", "NERA ASA",
 "GSI Technology", "Dane-Elec (C Memory)", "Acorn Computers", "Lara Technology",
 "Oak Technology, Inc.", "Itec Memory", "Tanisys Technology", "Truevision",
 "Wintec Industries", "Super PC Memory", "MGV Memory", "Galvantech",
 "Gadzoox Nteworks", "Multi Dimensional Cons.", "GateField", "Integrated Memory System",
 "Triscend", "XaQti", "Goldenram", "Clear Logic",
 "Cimaron Communications", "Nippon Steel Semi. Corp.", "Advantage Memory", "AMCC",
 "LeCroy", "Yamaha Corporation", "Digital Microwave", "NetLogic Microsystems",
 "MIMOS Semiconductor", "Advanced Fibre", "BF Goodrich Data.", "Epigram",
 "Acbel Polytech Inc.", "Apacer Technology", "Admor Memory", "FOXCONN",
 "Quadratics Superconductor", "3COM"};


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

/*void ddr2_sdram_atime(uint8_t *addr)
{
	int i;
	float t;

	t = ((addr >> 4) * 0.1 + (addr & 0xf) * 0.01);

	return t;
}
*/
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

int main (int argc, char *argv[])
{
	int i;
	FILE *fp;
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
	fclose(fp);

	dump(&record[0], 256);

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

	printf("\n");

	printf("---=== Memory Characteristics ===---\n");
	printf("#Maximum module speed\t\t\t\t d MHz (PC2-d)\n");
	printf("Size\t\t\t\t\t\t %s \n", size[record[31]] );
	printf("Banks x Rows x Columns x Bits\t\t\t %d x %d x %d x %d\n", record[17], record[3], record[4], record[6]);
	printf("Ranks\t\t\t\t\t\t %d\n", (record[5] & 0x7) + 1);
	printf("SDRAM Device Width\t\t\t\t %d bits\n", record[13]);
	printf("Module Height\t\t\t\t\t %s mm\n", heights[record[5] >> 5]);
	printf("#Module Type\t\t\t\t\t SO-DIMM (67.6 mm)\n" );
	printf("DRAM Package\t\t\t\t\t ");
	if ((record[5] & 0x10) == 1) {
		printf("Stack\n");
	} else {
		printf("Planar\n");
	}
	printf("Voltage Interface Level\t\t\t\t %s\n", sdram_voltage_interface_level[record[8]]);
	printf("#Module Configuration Type\t\t\t s\n" );
	printf("Refresh Rate\t\t\t\t\t Reduced (%0.2lf us) - Self Refresh\n", record[12] * 0.1);
	printf("Supported Burst Lengths\t\t\t\t %d, %d\n", record[16] & 4, record[16] & 8);
	printf("#tCL-tRCD-tRP-tRAS\t\t\t\t 6-6-6-18\n" );
	printf("#Supported CAS Latencies (tCL)\t\t\t 6T, 5T, 4T\n" );
	printf("#Minimum Cycle Time\t\t\t\t 2.50 ns at CAS 6 (tCK min)\n\t\t\t\t\t\t 3.00 ns at CAS 5\n\t\t\t\t\t\t 3.75 ns at CAS 4\n" );
	printf("#Maximum Access Time\t\t\t\t 0.40 ns at CAS 6 (tAC)\n\t\t\t\t\t\t 0.45 ns at CAS 5\n\t\t\t\t\t\t 0.50 ns at CAS 4\n" );
	printf("Maximum Cycle Time (tCK max)\t\t\t %0.2lf ns\n", (record[43] >> 4) * 1.0 + (record[43] & 0x0f) * 0.1);




	printf("\n");

	printf("---=== Timing Parameters ===---\n");
	printf("Address/Command Setup Time Before Clock (tIS)\t %0.2lf ns\n", ((record[32] >> 4) * 0.1 + (record[32] & 0xf) * 0.01));
	printf("Address/Command Hold Time After Clock (tIH)\t %0.2lf ns\n", ((record[33] >> 4) * 0.1 + (record[33] & 0xf) * 0.01));
	printf("Data Input Setup Time Before Strobe (tDS)\t %0.2lf ns\n", ((record[34] >> 4) * 0.1 + (record[34] & 0xf) * 0.01));
	printf("Data Input Hold Time After Strobe (tDH)\t\t %0.2lf ns\n", ((record[35] >> 4) * 0.1 + (record[35] & 0xf) * 0.01));

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





	printf("\n");
	printf("---=== Manufacturing Information ===---\n");

	printf("Manufacturer JEDEC ID\t\t\t\t");
	for (i = 64; i < 72; i++) {
		printf(" %02x", record[i]);
	}
	printf("\n");
	printf("Manufacturing Location Code\t\t\t 0x%02x\n", record[72]);
	printf("Part Number\n");
	printf("Manufacturing Date\t\t\t\t 20%d-W%d\n", record[93], record[94]);
	printf("Assembly Serial Number\t\t\t\t 0x");
	for (i = 95; i < 99; i++) {
		printf("%02X", record[i]);
	}

	printf("\n\n\nNumber of SDRAM DIMMs detected and decoded: %d\n", argc - 1);

	return 0;
}
