#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

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


	printf("---=== SPD EEPROM Information ===---\n");

	printf("EEPROM Checksum of bytes 0-62\t\t OK (0x%0X)\n", record[63]);

	printf("# of bytes written to SDRAM EEPROM\t %d\n", record[0]);
	
	printf("Total number of bytes in EEPROM\t\t %d\n", 1 << record[1]);

	if (record[2] < 11) {
		printf("Fundamental Memory type\t\t\t %s\n", type_list[record[2]]);
	} else {
		printf("Warning: unknown memory type (%02x)\n", record[2]);
	}

	printf("SPD Revision\t\t\t\t %x.%x\n", record[62] >> 4, record[62] & 0x0f);

	printf("\n");

	printf("---=== Timing Parameters ===---\n");
	printf("Address/Command Setup Time Before Clock (tIS)\t %0.2lf ns\n", ((record[32] >> 4) * 0.1 + (record[32] & 0xf) * 0.01));
	printf("Address/Command Hold Time After Clock (tIH)\t %0.2lf ns\n", ((record[33] >> 4) * 0.1 + (record[33] & 0xf) * 0.01));
	printf("Data Input Setup Time Before Strobe (tDS)\t %0.2lf ns\n", ((record[34] >> 4) * 0.1 + (record[34] & 0xf) * 0.01));
	printf("Data Input Hold Time After Strobe (tDH)\t\t %0.2lf ns\n", ((record[35] >> 4) * 0.1 + (record[35] & 0xf) * 0.01));

	printf("Minimum Row Precharge Delay (tRP)\t\t %0.2lf ns\n", (record[27] & 0xfc) / 4 * 1.0);
	printf("Minimum Row Active to Row Active Delay (tRRD)\t %0.2lf ns\n", (record[28] & 0xfc) / 4 * 1.0);
	printf("Minimum RAS# to CAS# Delay (tRCD)\t\t %0.2lf ns\n", (record[29] & 0xfc) / 4 * 1.0);
	printf("Minimum RAS# Pulse Width (tRAS)\t\t\t %0.2lf ns\n", (record[30] & 0xfc) * 1.0);

	printf("Write Recovery Time (tWR)\t\t\t %d ns\n", record[36] );
	printf("Minimum Write to Read CMD Delay (tWTR)\t\t %d ns\n", record[37]);
	printf("Minimum Read to Pre-charge CMD Delay (tRTP)\t %d ns\n", record[38]);
	printf("Minimum Active to Auto-refresh Delay (tRC)\t %d ns\n", record[41]);
	printf("Minimum Recovery Delay (tRFC)\t\t\t %d ns\n", record[42]);
	printf("Maximum DQS to DQ Skew (tDQSQ)\t\t\t %d ns\n", record[44] / 100);
	printf("Maximum Read Data Hold Skew (tQHS)\t\t %d ns\n", record[45]);





	printf("\n");
	printf("---=== Manufacturing Information ===---\n");

	printf("Manufacturer JEDEC ID\t\t\t");
	for (i = 64; i < 72; i++) {
		printf(" %02x", record[i]);
	}
	printf("\n");
	printf("Manufacturing Location Code\t\t 0x%02x\n", record[72]);
	printf("Part Number\n");
	printf("Manufacturing Date\t\t\t 20%d-W%d\n", record[93], record[94]);
	printf("Assembly Serial Number\t\t\t 0x");
	for (i = 95; i < 99; i++) {
		printf("%02X", record[i]);
	}
	printf("\n");

	return 0;
}
