 /*
 * $RCSfile: testccram.h,v $ last changed on $Date: 2008-10-07 20:25:36 $ by $Author: harder $
 *
 * $Log: testccram.h,v $
 * Revision 1.2  2008-10-07 20:25:36  harder
 * CC not in deb mode, wait msleep(1), reset counter
 *
 * Revision 1.1  2008/10/07 18:08:01  rudolf
 * added basic testsoft for CC2
 *
 *
 *
 *
 */

#ifndef TESTCCRAM_H
#define TESTCCRAM_H

/* register mapping CC2 */
/* counts */

#define OFF_CC2_RAMMIN (0x00)   /* first address of usable counts RAM (RW)*/
#define OFF_CC2_RAMMAX (0xEF)   /* end of counts RAM (RW) */

/* registers */

#define CC2_SIGNATURE  (0xCC02) /* Counter Card II */

#define OFF_CC2_SIG    (0xF0)   /* card signature is found here (RO) */
#define OFF_CC2_REV    (0xF1)   /* HW revision (RO) */

#define OFF_CC2_CTRL   (0xF2)   /* control register */
#define CTRL_STARTCOPY (0x0001) /* bit 0 starts copying of counter values into RAM */

#define OFF_CC2_SPARE1 (0xF3)   /* spare */
#define OFF_CC2_SPARE2 (0xF4)   /* spare */

#define OFF_CC2_CNTDEL (0xF5)   /* counter start delay in 4ns increments after*/
                                /* falling edge of laser trigger */

#define OFF_CC2_PULSEW (0xF6)   /* modulation pulse width in 4ns increments */
#define OFF_CC2_PAUSEW (0xF7)   /* modulation pause width in 4ns increments */

#define OFF_CC2_ADC0   (0xF8)   /* ADC Channel #0, PMT Threshold */
#define OFF_CC2_ADC1   (0xF9)   /* ADC Channel #1, ext. SYNC Threshold */
#define OFF_CC2_ADC2   (0xFA)   /* ADC Channel #2, comp. Thresh. MCP1 */
#define OFF_CC2_ADC3   (0xFB)   /* ADC Channel #3, +15V monitor */
#define OFF_CC2_ADC4   (0xFC)   /* ADC Channel #4, +5V monitor */
#define OFF_CC2_ADC5   (0xFD)   /* ADC Channel #5, +28V monitor */
#define OFF_CC2_ADC6   (0xFE)   /* ADC Channel #6, +3V3 monitor */
#define OFF_CC2_ADC7   (0xFF)   /* ADC Channel #7, +1.2V core monitor */

double CalculateVoltage(unsigned char ucIs5VoltRange, unsigned int uiADCCounts, double ScaleFactor);

#endif

