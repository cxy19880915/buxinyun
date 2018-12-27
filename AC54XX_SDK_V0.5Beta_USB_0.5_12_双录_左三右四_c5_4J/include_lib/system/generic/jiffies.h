#ifndef JIFFIES_H
#define JIFFIES_H
/* timer interface */
/* Parameters used to convert the timespec values: */
#define HZ				100L
#define MSEC_PER_SEC	1000L
#define USEC_PER_MSEC	1000L
#define NSEC_PER_USEC	1000L
#define NSEC_PER_MSEC	1000000L
#define USEC_PER_SEC	1000000L
#define NSEC_PER_SEC	1000000000L
#define FSEC_PER_SEC	1000000000000000LL


#ifndef __ASSEMBLY__
extern volatile unsigned long jiffies;
#endif


#define time_after(a,b)						 ((long)(b) - (long)(a) <= 0)
#define time_before(a,b)							time_after(b,a)



#define msecs_to_jiffies(msec) 		((msec)/2)

#define jiffies_to_msecs(j) 		((j)*2)














#endif

