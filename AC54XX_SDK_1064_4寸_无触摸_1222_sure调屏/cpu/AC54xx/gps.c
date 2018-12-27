

 
#if 0
#define kk 20
void GPS_RMC_Parse(char *line, GPS_INFO *GPS)
{

char RX_BUF1,ns,ws,t,n3,w3,ff =0,ff1=0;
float lati_cent_tmp, lati_second_tmp;
float long_cent_tmp, long_second_tmp;
float speed_tmp;
char *buf = line;
char sf[SOILDOG_MSG_BUF_SIZE]= {0} ;
int i = 0,j = 0,hh =0,mm =0,ss =0,yy =0,yy1=0,mu =0,dd =0,t1 =0,t2=0,t3=0,t4=0,t5=0,n1 =0,n2 =0,w1 =0,w2 =0;
u8 k, count = 0;

char status = 0;
status= buf4[GetComma(2, buf4)];
ch = buf4[5];

// $GPRMC,120018.000,V,0000.0000,N,00000.0000,E,000.0,000.0,280606,,,N*76
          
 if(ch == 'C')
{
         	if((status  == 'V'))
      {
          // printf("buf 4 %s\n", buf4);
             status2= GetComma(1, buf4);
            hh = (buf4[status2+0] - '0') * 10 + (buf4[status2+1] - '0');        //时间
            mm= (buf4[status2+2] - '0') * 10 + (buf4[status2+3] - '0');
            ss = (buf4[status2+4] - '0') * 10 + (buf4[status2+5] - '0');
            tmp = GetComma(9, buf4);
            GPS->D.day1 = (buf4[tmp + 0] - '0') * 10 + (buf4[tmp + 1] - '0'); //日期
           GPS->D.month1 = (buf4[tmp + 2] - '0') * 10 + (buf4[tmp + 3] - '0');
            GPS->D.year1 = (buf4[tmp + 4] - '0') * 10 + (buf4[tmp + 5] - '0') + 2000;

			

	// GPS->year1    = yy1;
 
	
	//   printf("%d %d %d %d %d %d\n ",hh,mm,ss,GPS->D.year1,GPS->D.month1,GPS->D.day1);
	   printf("status2 %d tmp %d\n ",status2,tmp);
	 /***************************** 系统时间控件动作 ************************************/
         		}

 }


  //  return 0;
}


/*
int GPS_GGA_Parse(char *line, GPS_INFO *GPS)
{
    U8 ch, status;
    char *buf = line;
    ch = buf[4];
    status = buf[GetComma(2, buf)];

    if (ch == 'G') //$GPGGA
    {
        if (status != ',')
        {
            GPS->height = Get_Float_Number(&buf[GetComma(9, buf)]);
            GPS->satellite = Get_Int_Number(&buf[GetComma(7, buf)]);

            return 1;
        }
    }

    return 0;
}
*/


//static u8 page_main_flag = 0;
#endif
