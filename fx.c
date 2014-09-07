/*
 * fx.c
 *
 *  Created on: 2013/02/23
 *      Author: Tonsuke
 */

#include <math.h>

#include "fx.h"
#include "sound.h"

#include "arm_math.h"

#include "usart.h"

#define M_PI_F ((float)(M_PI))

void REVERB_Set_Prams(REVERB_Struct_Typedef *RFX)
{
	RFX->amp[0] = 0.0f;
	RFX->delay[0] = 0;
	RFX->repeat[0] = 0;
	REVERB_Init(RFX, 0);

	RFX->amp[1] = 0.5f;
	RFX->delay[1] = RFX->fs * 0.05f;
	RFX->repeat[1] = 6;
	REVERB_Init(RFX, 1);

	RFX->amp[2] = 0.5f;
	RFX->delay[2] = RFX->fs * 0.10f;
	RFX->repeat[2] = 3;
	REVERB_Init(RFX, 2);

	RFX->amp[3] = 0.55f;
	RFX->delay[3] = RFX->fs * 0.15f;
	RFX->repeat[3] = 2;
	REVERB_Init(RFX, 3);
}

void REVERB_Init(REVERB_Struct_Typedef *RFX, int n)
{
	int i;

	RFX->clipper[n] = 1.0f;

	for(i = 1;i <= RFX->repeat[n];i++){
		RFX->clipper[n] += powf(RFX->amp[n], (float)i);
		RFX->coeff_table[n][i] = powf(RFX->amp[n], (float)i);
	}

	RFX->clipper[n] = 1.0f / RFX->clipper[n];

	RFX->delay_buffer->idx = 0;
}


void REVERB(REVERB_Struct_Typedef *RFX, uint32_t *out_ptr)
{
	uint32_t left_in, right_in, left_out, right_out;
	int i, j, m;

	if((RFX->delay_buffer->idx -= RFX->num_blocks) < 0){
		RFX->delay_buffer->idx += RFX->delay_buffer->size;
	}

	for(i = 0;i < RFX->num_blocks;i++){

		left_out  = LOWER_OF_WORD(out_ptr[i]);
		right_out = UPPER_OF_WORD(out_ptr[i]);

		for(j = 1;j <= RFX->repeat[RFX->number];j++){
			m = RFX->delay_buffer->idx - RFX->delay[RFX->number] * j;
			if(m < 0){
				m += RFX->delay_buffer->size;
			}
			left_in  = LOWER_OF_WORD(RFX->delay_buffer->ptr[m]);
			right_in = UPPER_OF_WORD(RFX->delay_buffer->ptr[m]);

			left_out += RFX->coeff_table[RFX->number][j] * left_in;
			right_out += RFX->coeff_table[RFX->number][j] * right_in;
		}

		left_out *= RFX->clipper[RFX->number];
		right_out *= RFX->clipper[RFX->number];

		left_out = __USAT(left_out, 16);
		right_out = __USAT(right_out, 16);

		out_ptr[i] = __PKHBT(left_out, right_out, 16);

		if(++RFX->delay_buffer->idx >= RFX->delay_buffer->size){
			RFX->delay_buffer->idx = 0;
		}
	}
}


void IIR_Set_Params(IIR_Filter_Struct_Typedef *IIR)
{
	IIR->fc = 250.0f / IIR->fs; // cut off freq
	IIR->Q = 1.0f / sqrtf(2.0f);

//	int m;
//
//	IIR->g = 0.0f;
//	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);
//
//	for (m = 0; m <= 2; m++)
//	{
//		IIR->A[0][m] = IIR->a[m];
//		IIR->B[0][m] = IIR->b[m];
//	}
//	a0:1.000000 a1:-1.949638 a2:0.950875
//	b0:1.000000 b1:-1.949638 b2:0.950875
	IIR->A[0][0] = 1.000000f, IIR->A[0][1] = -1.949638f, IIR->A[0][2] = 0.950875f;
	IIR->B[0][0] = 1.000000f, IIR->B[0][1] = -1.949638f, IIR->B[0][2] = 0.950875f;

//	IIR->g = 0.6f;
//	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);
//
//	for (m = 0; m <= 2; m++)
//	{
//		IIR->A[1][m] = IIR->a[m];
//		IIR->B[1][m] = IIR->b[m];
//	}
//	a0:1.000000 a1:-1.949638 a2:0.950875
//	b0:1.006693 b1:-1.949266 b2:0.944554
	IIR->A[1][0] = 1.000000f, IIR->A[1][1] = -1.949638f, IIR->A[1][2] = 0.950875f;
	IIR->B[1][0] = 1.006693f, IIR->B[1][1] = -1.949266f, IIR->B[1][2] = 0.944554f;

//	IIR->g = 1.1f;
//	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);
//
//	for (m = 0; m <= 2; m++)
//	{
//		IIR->A[2][m] = IIR->a[m];
//		IIR->B[2][m] = IIR->b[m];
//	}
//	a0:1.000000 a1:-1.949638 a2:0.950875
//	b0:1.011372 b1:-1.948957 b2:0.940183
	IIR->A[2][0] = 1.000000f, IIR->A[2][1] = -1.949638f, IIR->A[2][2] = 0.950875f;
	IIR->B[2][0] = 1.011372f, IIR->B[2][1] = -1.948957f, IIR->B[2][2] = 0.940183f;


//	IIR->g = 2.0f;
//	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);
//
//	for (m = 0; m <= 2; m++)
//	{
//		IIR->A[3][m] = IIR->a[m];
//		IIR->B[3][m] = IIR->b[m];
//	}
//	a0:1.000000 a1:-1.949638 a2:0.950875
//	b0:1.018600 b1:-1.948400 b2:0.933513
	IIR->A[3][0] = 1.000000f, IIR->A[3][1] = -1.949638f, IIR->A[3][2] = 0.950875f;
	IIR->B[3][0] = 1.018600f, IIR->B[3][1] = -1.948400f, IIR->B[3][2] = 0.933513f;

}

void IIR_Set_Params2(IIR_Filter_Struct_Typedef *IIR)
{
	int m;

	IIR->fc = 2000.0f / IIR->fs; // cut off freq
	IIR->Q = 1.0f / sqrtf(2.0f);

	IIR_LPF(IIR->fc, IIR->Q, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[0][m] = IIR->a[m];
		IIR->B[0][m] = IIR->b[m];
	}

	IIR->fc = 1000.0f / IIR->fs; // cut off freq
	IIR->Q = 1.0f / sqrtf(2.0f);

	IIR_HPF(IIR->fc, IIR->Q, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[1][m] = IIR->a[m];
		IIR->B[1][m] = IIR->b[m];
	}

	IIR->fc = 1000.0f / IIR->fs; // cut off freq
	IIR->Q = 5.0f / sqrtf(2.0f);

	IIR_resonator(IIR->fc, IIR->Q, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[2][m] = IIR->a[m];
		IIR->B[2][m] = IIR->b[m];
	}

	IIR->g = 2.0f;
	IIR->fc = 250.0f / IIR->fs; // cut off freq
	IIR->Q = 1.0f / sqrtf(2.0f);
	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[3][m] = IIR->a[m];
		IIR->B[3][m] = IIR->b[m];
	}
}

void IIR_Set_Params3(IIR_Filter_Struct_Typedef *IIR)
{
	int m;

	IIR->fc = 250.0f / IIR->fs; // cut off freq
	IIR->Q = 2.0f / sqrtf(2.0f);
	IIR->g = 1.0f;

	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[0][m] = IIR->a[m];
		IIR->B[0][m] = IIR->b[m];
	}

	IIR->fc = 1000.0f / IIR->fs; // cut off freq
//	IIR->Q = 1.0f / sqrtf(2.0f);
	IIR->g = -1.0f;

	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[1][m] = IIR->a[m];
		IIR->B[1][m] = IIR->b[m];
	}

	IIR->fc = 3000.0f / IIR->fs; // cut off freq
//	IIR->Q = 1.0f / sqrtf(2.0f);
	IIR->g = 1.0f;

	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[2][m] = IIR->a[m];
		IIR->B[2][m] = IIR->b[m];
	}

	IIR->fc = 5000.0f / IIR->fs; // cut off freq
//	IIR->Q = 1.0f / sqrtf(2.0f);
	IIR->g = -1.0f;

	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[3][m] = IIR->a[m];
		IIR->B[3][m] = IIR->b[m];
	}
}

typedef struct {
	float delay[2][2];
} biquad_iir_st_t;

biquad_iir_st_t s;

void IIR_Filter(IIR_Filter_Struct_Typedef *IIR, float *in_ptr, float *out_ptr)
{
	float d0_0, d1_0;
	float d0_1, d1_1;
	float tmp, input;
	int i;

	d0_0 = s.delay[0][0];
	d1_0 = s.delay[1][0];
	d0_1 = s.delay[0][1];
	d1_1 = s.delay[1][1];

	for(i = 0;i < IIR->num_blocks;i++){
		input = *in_ptr++;
		tmp = input + d0_0 * -IIR->A[IIR->number][1] + d1_0 * -IIR->A[IIR->number][2];
		*out_ptr++ = tmp * IIR->B[IIR->number][0] + d0_0 * IIR->B[IIR->number][1] + d1_0 * IIR->B[IIR->number][2];
		d1_0 = d0_0;
		d0_0 = tmp;

		input = *in_ptr++;
		tmp = input + d0_1 * -IIR->A[IIR->number][1] + d1_1 * -IIR->A[IIR->number][2];
		*out_ptr++ = tmp * IIR->B[IIR->number][0] + d0_1 * IIR->B[IIR->number][1] + d1_1 * IIR->B[IIR->number][2];
		d1_1 = d0_1;
		d0_1 = tmp;
	}

	s.delay[0][0] = d0_0;
	s.delay[1][0] = d1_0;
	s.delay[0][1] = d0_1;
	s.delay[1][1] = d1_1;
}

void IIR_resonator(float fc, float Q, float a[], float b[])
{
  fc = tanf(M_PI_F * fc) / (2.0f * M_PI_F);

  a[0] = 1.0f + 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc;
  a[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  a[2] = (1.0f - 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[0] = 2.0f * M_PI_F * fc / Q / a[0];
  b[1] = 0.0f;
  b[2] = -2.0f * M_PI_F * fc / Q / a[0];

  a[0] = 1.0f;
/*
  b[0] /= 2.0f;
  b[1] /= 2.0f;
  b[2] /= 2.0f;
  a[0] /= 2.0f;
  a[1] /= 2.0f;
  a[2] /= 2.0f;
  */

  debug.printf("\r\n\nResonator");
  debug.printf("\r\na0:%.3f a1:%.3f a2:%.3f", a[0], a[1], a[2]);
  debug.printf("\r\nb0:%.3f b1:%.3f b2:%.3f", b[0], b[1], b[2]);

}

void IIR_LPF(float fc, float Q, float a[], float b[])
{
  fc = tanf(M_PI_F * fc) / (2.0f * M_PI_F);

  a[0] = 1.0f + 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc;
  a[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  a[2] = (1.0f - 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[0] = 4.0f * M_PI_F * M_PI_F * fc * fc / a[0];
  b[1] = 8.0f * M_PI_F * M_PI_F * fc * fc / a[0];
  b[2] = 4.0f * M_PI_F * M_PI_F * fc * fc / a[0];

  a[0] = 1.0f;
/*
  b[0] /= 2.0f;
  b[1] /= 2.0f;
  b[2] /= 2.0f;
  a[0] /= 2.0f;
  a[1] /= 2.0f;
  a[2] /= 2.0f;
*/
  debug.printf("\r\n\nLPF");
  debug.printf("\r\na0:%.3f a1:%.3f a2:%.3f", a[0], a[1], a[2]);
  debug.printf("\r\nb0:%.3f b1:%.3f b2:%.3f", b[0], b[1], b[2]);
}

void IIR_HPF(float fc, float Q, float a[], float b[])
{
  fc = tanf(M_PI_F * fc) / (2.0f * M_PI_F);

  a[0] = 1.0f + 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc;
  a[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  a[2] = (1.0f - 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[0] = 1.0f / a[0];
  b[1] = -2.0f / a[0];
  b[2] = 1.0f / a[0];

  a[0] = 1.0f;
/*
  b[0] /= 2.0f;
  b[1] /= 2.0f;
  b[2] /= 2.0f;
  a[0] /= 2.0f;
  a[1] /= 2.0f;
  a[2] /= 2.0f;
*/
  debug.printf("\r\n\nHPF");
  debug.printf("\r\na0:%.3f a1:%.3f a2:%.3f", a[0], a[1], a[2]);
  debug.printf("\r\nb0:%.3f b1:%.3f b2:%.3f", b[0], b[1], b[2]);
}

void IIR_low_shelving(float fc, float Q, float g, float a[], float b[])
{
  fc = tanf(M_PI_F * fc) / (2.0f * M_PI_F);

  a[0] = 1.0f + 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc;
  a[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  a[2] = (1.0f - 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[0] = (1.0f + sqrtf(1.0f + g) * 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc * (1.0f + g)) / a[0];
  b[1] = (8.0f * M_PI_F * M_PI_F * fc * fc * (1.0f + g) - 2.0f) / a[0];
  b[2] = (1.0f - sqrtf(1.0f + g) * 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc * (1.0f + g)) / a[0];

  a[0] = 1.0f;
/*
  b[0] /= 2.0f;
  b[1] /= 2.0f;
  b[2] /= 2.0f;
  a[0] /= 2.0f;
  a[1] /= 2.0f;
  a[2] /= 2.0f;
*/
  debug.printf("\r\n\nLow Shelving");
  debug.printf("\r\na0:%f a1:%f a2:%f", a[0], a[1], a[2]);
  debug.printf("\r\nb0:%f b1:%f b2:%f", b[0], b[1], b[2]);
}


void IIR_peaking(float fc, float Q, float g, float a[], float b[])
{
  fc = tanf(M_PI_F * fc) / (2.0f * M_PI_F);

  a[0] = 1.0f + 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc;
  a[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  a[2] = (1.0f - 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[0] = (1.0f + 2.0f * M_PI_F * fc / Q * (1.0f + g) + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  b[2] = (1.0f - 2.0f * M_PI_F * fc / Q * (1.0f + g) + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];

  a[0] = 1.0f;
/*
  b[0] /= 2.0f;
  b[1] /= 2.0f;
  b[2] /= 2.0f;
  a[0] /= 2.0f;
  a[1] /= 2.0f;
  a[2] /= 2.0f;
*/
  debug.printf("\r\n\nPeaking");
  debug.printf("\r\na0:%.3f a1:%.3f a2:%.3f", a[0], a[1], a[2]);
  debug.printf("\r\nb0:%.3f b1:%.3f b2:%.3f", b[0], b[1], b[2]);
}
