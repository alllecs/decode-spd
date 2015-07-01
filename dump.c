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
