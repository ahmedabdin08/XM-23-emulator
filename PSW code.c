/*First function obtained from Dr. Larry Hughes
* code for updating the PSW 
* 
*/

#include "cpu.h"

unsigned carry[2][2][2] = { 0, 0, 1, 0, 1, 0, 1, 1 };
unsigned overflow[2][2][2] = {0, 1, 0, 0, 0, 0, 1, 0};

void update_psw(unsigned short src, unsigned short dst, unsigned short res, Size wb)
{
	/* 
	 - Update the PSW bits (V, N, Z, C) 
	 - Using src, dst, and res values and whether word or byte 
	 - ADD, ADDC, SUB, SUBC, and CMP
	*/
	unsigned short mss, msd, msr; /* Most significant src, dst, and res bits */

	if (wb == WORD)
	{
		mss = B15(src);
		msd = B15(dst);
		msr = B15(res);
	}
	else /* Byte */
	{
		mss = B7(src);
		msd = B7(dst);
		msr = B7(res);
		res &= 0x00FF;	/* Mask high byte for 'z' check */
	}

	/* Carry */
	pswptr->bit.C = carry[mss][msd][msr];
	/* Zero */
	pswptr->bit.Z = (res == 0);
	/* Negative */
	pswptr->bit.N = (msr == 1);
	/* oVerflow */
	pswptr->bit.V= overflow[mss][msd][msr];
}

void update_pswNZ(unsigned short res, Size wb) 
{
	unsigned short msr;
	if (wb == WORD)
	{
		msr = B15(res);
	}
	else /* Byte */
	{
		msr = B7(res);
		res &= 0x00FF;	/* Mask high byte for 'z' check */
	}

	/* Zero */
	pswptr->bit.Z = (res == 0);
	/* Negative */
	pswptr->bit.N = (msr == 1);
}
