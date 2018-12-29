/*
0.5dbm
*/

int puts(char *out);

static const char phy_reg_pg[] = "\
//rtl8723b version = 28\n\
#[v1][Exact]#\n\
#[2.4G][A]#\n\
[1Tx] 0xe08	0x0000ff00	 	 0 0 46 0 // TXAGC codeword (H-byte->L-byte)={1M}\n\
[1Tx] 0x86c	0xffffff00	 	 38 40 44 0 // TXAGC codeword (H-byte->L-byte)={11M 5.5M 2M}\n\
[1Tx] 0xe00	0xffffffff		 40 42 44 46 // TXAGC codeword (H-byte->L-byte)={18M 12M 9M 6M}\n\
[1Tx] 0xe04	0xffffffff		 28 32 36 38 // TXAGC codeword (H-byte->L-byte)={54M 48M 36M 24M}\n\
[1Tx] 0xe10	0xffffffff		 38 40 42 44 // TXAGC codeword (H-byte->L-byte)=HT_{MCS3 MCS2 MCS1 MCS0}\n\
[1Tx] 0xe14	0xffffffff		 26 30 34 36 // TXAGC codeword (H-byte->L-byte)=HT_{MCS7 MCS6 MCS5 MCS4}\n\
#[END]#\n\
0xffff 0xffff\n\
\n";

//返回0代表使用WIFI驱动原始配置表格
const char *get_phy_reg_pg(void)
{
    puts("user get_phy_reg_pg \n");
    return 0;
    return phy_reg_pg;
}


