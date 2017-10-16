/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vad.h"

#include <stdlib.h>
#include <string.h>

static const int kInitCheck = 42;
static const int kValidRates[] = { 8000, 16000, 32000, 48000 };
static const size_t kRatesSize = sizeof(kValidRates) / sizeof(*kValidRates);
static const int kMaxFrameLengthMs = 30;

int WebRtcVad_Create(VadInst** handle) {
  VadInstT* self = NULL;

  if (handle == NULL) {
    return -1;
  }

  *handle = NULL;
  self = (VadInstT*) malloc(sizeof(VadInstT));
  *handle = (VadInst*) self;

  if (self == NULL) {
    return -1;
  }

  WebRtcSpl_Init();

  self->init_flag = 0;

  return 0;
}

int WebRtcVad_Free(VadInst* handle) {
  if (handle == NULL) {
    return -1;
  }

  free(handle);

  return 0;
}

// TODO(bjornv): Move WebRtcVad_InitCore() code here.
int WebRtcVad_Init(VadInst* handle) {
  // Initialize the core VAD component.
  return WebRtcVad_InitCore((VadInstT*) handle);
}

// TODO(bjornv): Move WebRtcVad_set_mode_core() code here.
int WebRtcVad_set_mode(VadInst* handle, int mode) {
  VadInstT* self = (VadInstT*) handle;

  if (handle == NULL) {
    return -1;
  }
  if (self->init_flag != kInitCheck) {
    return -1;
  }

  return WebRtcVad_set_mode_core(self, mode);
}

int WebRtcVad_Process(VadInst* handle, int fs, int16_t* audio_frame,
                      int frame_length) {
  int vad = -1;
  VadInstT* self = (VadInstT*) handle;

  if (handle == NULL) {
    return -1;
  }

  if (self->init_flag != kInitCheck) {
    return -1;
  }
  if (audio_frame == NULL) {
    return -1;
  }
  if (WebRtcVad_ValidRateAndFrameLength(fs, frame_length) != 0) {
    return -1;
  }

  if (fs == 48000) {
      vad = WebRtcVad_CalcVad48khz(self, audio_frame, frame_length);
  } else if (fs == 32000) {
    vad = WebRtcVad_CalcVad32khz(self, audio_frame, frame_length);
  } else if (fs == 16000) {
    vad = WebRtcVad_CalcVad16khz(self, audio_frame, frame_length);
  } else if (fs == 8000) {
    vad = WebRtcVad_CalcVad8khz(self, audio_frame, frame_length);
  }

  if (vad > 0) {
    vad = 1;
  }
  return vad;
}

int WebRtcVad_ValidRateAndFrameLength(int rate, int frame_length) {
  int return_value = -1;
  size_t i;
  int valid_length_ms;
  int valid_length;

  // We only allow 10, 20 or 30 ms frames. Loop through valid frame rates and
  // see if we have a matching pair.
  for (i = 0; i < kRatesSize; i++) {
    if (kValidRates[i] == rate) {
      for (valid_length_ms = 10; valid_length_ms <= kMaxFrameLengthMs;
          valid_length_ms += 10) {
        valid_length = (kValidRates[i] / 1000 * valid_length_ms);
        if (frame_length == valid_length) {
          return_value = 0;
          break;
        }
      }
      break;
    }
  }

  return return_value;
}

/* Tables for data buffer indexes that are bit reversed and thus need to be
 * swapped. Note that, index_7[{0, 2, 4, ...}] are for the left side of the swap
 * operations, while index_7[{1, 3, 5, ...}] are for the right side of the
 * operation. Same for index_8.
 */

/* Indexes for the case of stages == 7. */
static const int16_t index_7[112] = {
  1, 64, 2, 32, 3, 96, 4, 16, 5, 80, 6, 48, 7, 112, 9, 72, 10, 40, 11, 104,
  12, 24, 13, 88, 14, 56, 15, 120, 17, 68, 18, 36, 19, 100, 21, 84, 22, 52,
  23, 116, 25, 76, 26, 44, 27, 108, 29, 92, 30, 60, 31, 124, 33, 66, 35, 98,
  37, 82, 38, 50, 39, 114, 41, 74, 43, 106, 45, 90, 46, 58, 47, 122, 49, 70,
  51, 102, 53, 86, 55, 118, 57, 78, 59, 110, 61, 94, 63, 126, 67, 97, 69,
  81, 71, 113, 75, 105, 77, 89, 79, 121, 83, 101, 87, 117, 91, 109, 95, 125,
  103, 115, 111, 123
};

/* Indexes for the case of stages == 8. */
static const int16_t index_8[240] = {
  1, 128, 2, 64, 3, 192, 4, 32, 5, 160, 6, 96, 7, 224, 8, 16, 9, 144, 10, 80,
  11, 208, 12, 48, 13, 176, 14, 112, 15, 240, 17, 136, 18, 72, 19, 200, 20,
  40, 21, 168, 22, 104, 23, 232, 25, 152, 26, 88, 27, 216, 28, 56, 29, 184,
  30, 120, 31, 248, 33, 132, 34, 68, 35, 196, 37, 164, 38, 100, 39, 228, 41,
  148, 42, 84, 43, 212, 44, 52, 45, 180, 46, 116, 47, 244, 49, 140, 50, 76,
  51, 204, 53, 172, 54, 108, 55, 236, 57, 156, 58, 92, 59, 220, 61, 188, 62,
  124, 63, 252, 65, 130, 67, 194, 69, 162, 70, 98, 71, 226, 73, 146, 74, 82,
  75, 210, 77, 178, 78, 114, 79, 242, 81, 138, 83, 202, 85, 170, 86, 106, 87,
  234, 89, 154, 91, 218, 93, 186, 94, 122, 95, 250, 97, 134, 99, 198, 101,
  166, 103, 230, 105, 150, 107, 214, 109, 182, 110, 118, 111, 246, 113, 142,
  115, 206, 117, 174, 119, 238, 121, 158, 123, 222, 125, 190, 127, 254, 131,
  193, 133, 161, 135, 225, 137, 145, 139, 209, 141, 177, 143, 241, 147, 201,
  149, 169, 151, 233, 155, 217, 157, 185, 159, 249, 163, 197, 167, 229, 171,
  213, 173, 181, 175, 245, 179, 205, 183, 237, 187, 221, 191, 253, 199, 227,
  203, 211, 207, 243, 215, 235, 223, 251, 239, 247
};

void WebRtcSpl_ComplexBitReverse(int16_t* __restrict complex_data, int stages) {
  /* For any specific value of stages, we know exactly the indexes that are
   * bit reversed. Currently (Feb. 2012) in WebRTC the only possible values of
   * stages are 7 and 8, so we use tables to save unnecessary iterations and
   * calculations for these two cases.
   */
  if (stages == 7 || stages == 8) {
    int m = 0;
    int length = 112;
    const int16_t* index = index_7;

    if (stages == 8) {
      length = 240;
      index = index_8;
    }

    /* Decimation in time. Swap the elements with bit-reversed indexes. */
    for (m = 0; m < length; m += 2) {
      /* We declare a int32_t* type pointer, to load both the 16-bit real
       * and imaginary elements from complex_data in one instruction, reducing
       * complexity.
       */
      int32_t* complex_data_ptr = (int32_t*)complex_data;
      int32_t temp = 0;

      temp = complex_data_ptr[index[m]];  /* Real and imaginary */
      complex_data_ptr[index[m]] = complex_data_ptr[index[m + 1]];
      complex_data_ptr[index[m + 1]] = temp;
    }
  }
  else {
    int m = 0, mr = 0, l = 0;
    int n = 1 << stages;
    int nn = n - 1;

    /* Decimation in time - re-order data */
    for (m = 1; m <= nn; ++m) {
      int32_t* complex_data_ptr = (int32_t*)complex_data;
      int32_t temp = 0;

      /* Find out indexes that are bit-reversed. */
      l = n;
      do {
        l >>= 1;
      } while (l > nn - mr);
      mr = (mr & (l - 1)) + l;

      if (mr <= m) {
        continue;
      }

      /* Swap the elements with bit-reversed indexes.
       * This is similar to the loop in the stages == 7 or 8 cases.
       */
      temp = complex_data_ptr[m];  /* Real and imaginary */
      complex_data_ptr[m] = complex_data_ptr[mr];
      complex_data_ptr[mr] = temp;
    }
  }
}


#define CFFTSFT 14
#define CFFTRND 1
#define CFFTRND2 16384

#define CIFFTSFT 14
#define CIFFTRND 1

static const WebRtc_Word16 kSinTable1024[] = {
      0,    201,    402,    603,    804,   1005,   1206,   1406,
   1607,   1808,   2009,   2209,   2410,   2610,   2811,   3011,
   3211,   3411,   3611,   3811,   4011,   4210,   4409,   4608,
   4807,   5006,   5205,   5403,   5601,   5799,   5997,   6195,
   6392,   6589,   6786,   6982,   7179,   7375,   7571,   7766,
   7961,   8156,   8351,   8545,   8739,   8932,   9126,   9319,
   9511,   9703,   9895,  10087,  10278,  10469,  10659,  10849,
  11038,  11227,  11416,  11604,  11792,  11980,  12166,  12353,
  12539,  12724,  12909,  13094,  13278,  13462,  13645,  13827,
  14009,  14191,  14372,  14552,  14732,  14911,  15090,  15268,
  15446,  15623,  15799,  15975,  16150,  16325,  16499,  16672,
  16845,  17017,  17189,  17360,  17530,  17699,  17868,  18036,
  18204,  18371,  18537,  18702,  18867,  19031,  19194,  19357,
  19519,  19680,  19840,  20000,  20159,  20317,  20474,  20631,
  20787,  20942,  21096,  21249,  21402,  21554,  21705,  21855,
  22004,  22153,  22301,  22448,  22594,  22739,  22883,  23027,
  23169,  23311,  23452,  23592,  23731,  23869,  24006,  24143,
  24278,  24413,  24546,  24679,  24811,  24942,  25072,  25201,
  25329,  25456,  25582,  25707,  25831,  25954,  26077,  26198,
  26318,  26437,  26556,  26673,  26789,  26905,  27019,  27132,
  27244,  27355,  27466,  27575,  27683,  27790,  27896,  28001,
  28105,  28208,  28309,  28410,  28510,  28608,  28706,  28802,
  28897,  28992,  29085,  29177,  29268,  29358,  29446,  29534,
  29621,  29706,  29790,  29873,  29955,  30036,  30116,  30195,
  30272,  30349,  30424,  30498,  30571,  30643,  30713,  30783,
  30851,  30918,  30984,  31049,
  31113,  31175,  31236,  31297,
  31356,  31413,  31470,  31525,  31580,  31633,  31684,  31735,
  31785,  31833,  31880,  31926,  31970,  32014,  32056,  32097,
  32137,  32176,  32213,  32249,  32284,  32318,  32350,  32382,
  32412,  32441,  32468,  32495,  32520,  32544,  32567,  32588,
  32609,  32628,  32646,  32662,  32678,  32692,  32705,  32717,
  32727,  32736,  32744,  32751,  32757,  32761,  32764,  32766,
  32767,  32766,  32764,  32761,  32757,  32751,  32744,  32736,
  32727,  32717,  32705,  32692,  32678,  32662,  32646,  32628,
  32609,  32588,  32567,  32544,  32520,  32495,  32468,  32441,
  32412,  32382,  32350,  32318,  32284,  32249,  32213,  32176,
  32137,  32097,  32056,  32014,  31970,  31926,  31880,  31833,
  31785,  31735,  31684,  31633,  31580,  31525,  31470,  31413,
  31356,  31297,  31236,  31175,  31113,  31049,  30984,  30918,
  30851,  30783,  30713,  30643,  30571,  30498,  30424,  30349,
  30272,  30195,  30116,  30036,  29955,  29873,  29790,  29706,
  29621,  29534,  29446,  29358,  29268,  29177,  29085,  28992,
  28897,  28802,  28706,  28608,  28510,  28410,  28309,  28208,
  28105,  28001,  27896,  27790,  27683,  27575,  27466,  27355,
  27244,  27132,  27019,  26905,  26789,  26673,  26556,  26437,
  26318,  26198,  26077,  25954,  25831,  25707,  25582,  25456,
  25329,  25201,  25072,  24942,  24811,  24679,  24546,  24413,
  24278,  24143,  24006,  23869,  23731,  23592,  23452,  23311,
  23169,  23027,  22883,  22739,  22594,  22448,  22301,  22153,
  22004,  21855,  21705,  21554,  21402,  21249,  21096,  20942,
  20787,  20631,  20474,  20317,  20159,  20000,  19840,  19680,
  19519,  19357,  19194,  19031,  18867,  18702,  18537,  18371,
  18204,  18036,  17868,  17699,  17530,  17360,  17189,  17017,
  16845,  16672,  16499,  16325,  16150,  15975,  15799,  15623,
  15446,  15268,  15090,  14911,  14732,  14552,  14372,  14191,
  14009,  13827,  13645,  13462,  13278,  13094,  12909,  12724,
  12539,  12353,  12166,  11980,  11792,  11604,  11416,  11227,
  11038,  10849,  10659,  10469,  10278,  10087,   9895,   9703,
   9511,   9319,   9126,   8932,   8739,   8545,   8351,   8156,
   7961,   7766,   7571,   7375,   7179,   6982,   6786,   6589,
   6392,   6195,   5997,   5799,   5601,   5403,   5205,   5006,
   4807,   4608,   4409,   4210,   4011,   3811,   3611,   3411,
   3211,   3011,   2811,   2610,   2410,   2209,   2009,   1808,
   1607,   1406,   1206,   1005,    804,    603,    402,    201,
      0,   -201,   -402,   -603,   -804,  -1005,  -1206,  -1406,
  -1607,  -1808,  -2009,  -2209,  -2410,  -2610,  -2811,  -3011,
  -3211,  -3411,  -3611,  -3811,  -4011,  -4210,  -4409,  -4608,
  -4807,  -5006,  -5205,  -5403,  -5601,  -5799,  -5997,  -6195,
  -6392,  -6589,  -6786,  -6982,  -7179,  -7375,  -7571,  -7766,
  -7961,  -8156,  -8351,  -8545,  -8739,  -8932,  -9126,  -9319,
  -9511,  -9703,  -9895, -10087, -10278, -10469, -10659, -10849,
 -11038, -11227, -11416, -11604, -11792, -11980, -12166, -12353,
 -12539, -12724, -12909, -13094, -13278, -13462, -13645, -13827,
 -14009, -14191, -14372, -14552, -14732, -14911, -15090, -15268,
 -15446, -15623, -15799, -15975, -16150, -16325, -16499, -16672,
 -16845, -17017, -17189, -17360, -17530, -17699, -17868, -18036,
 -18204, -18371, -18537, -18702, -18867, -19031, -19194, -19357,
 -19519, -19680, -19840, -20000, -20159, -20317, -20474, -20631,
 -20787, -20942, -21096, -21249, -21402, -21554, -21705, -21855,
 -22004, -22153, -22301, -22448, -22594, -22739, -22883, -23027,
 -23169, -23311, -23452, -23592, -23731, -23869, -24006, -24143,
 -24278, -24413, -24546, -24679, -24811, -24942, -25072, -25201,
 -25329, -25456, -25582, -25707, -25831, -25954, -26077, -26198,
 -26318, -26437, -26556, -26673, -26789, -26905, -27019, -27132,
 -27244, -27355, -27466, -27575, -27683, -27790, -27896, -28001,
 -28105, -28208, -28309, -28410, -28510, -28608, -28706, -28802,
 -28897, -28992, -29085, -29177, -29268, -29358, -29446, -29534,
 -29621, -29706, -29790, -29873, -29955, -30036, -30116, -30195,
 -30272, -30349, -30424, -30498, -30571, -30643, -30713, -30783,
 -30851, -30918, -30984, -31049, -31113, -31175, -31236, -31297,
 -31356, -31413, -31470, -31525, -31580, -31633, -31684, -31735,
 -31785, -31833, -31880, -31926, -31970, -32014, -32056, -32097,
 -32137, -32176, -32213, -32249, -32284, -32318, -32350, -32382,
 -32412, -32441, -32468, -32495, -32520, -32544, -32567, -32588,
 -32609, -32628, -32646, -32662, -32678, -32692, -32705, -32717,
 -32727, -32736, -32744, -32751, -32757, -32761, -32764, -32766,
 -32767, -32766, -32764, -32761, -32757, -32751, -32744, -32736,
 -32727, -32717, -32705, -32692, -32678, -32662, -32646, -32628,
 -32609, -32588, -32567, -32544, -32520, -32495, -32468, -32441,
 -32412, -32382, -32350, -32318, -32284, -32249, -32213, -32176,
 -32137, -32097, -32056, -32014, -31970, -31926, -31880, -31833,
 -31785, -31735, -31684, -31633, -31580, -31525, -31470, -31413,
 -31356, -31297, -31236, -31175, -31113, -31049, -30984, -30918,
 -30851, -30783, -30713, -30643, -30571, -30498, -30424, -30349,
 -30272, -30195, -30116, -30036, -29955, -29873, -29790, -29706,
 -29621, -29534, -29446, -29358, -29268, -29177, -29085, -28992,
 -28897, -28802, -28706, -28608, -28510, -28410, -28309, -28208,
 -28105, -28001, -27896, -27790, -27683, -27575, -27466, -27355,
 -27244, -27132, -27019, -26905, -26789, -26673, -26556, -26437,
 -26318, -26198, -26077, -25954, -25831, -25707, -25582, -25456,
 -25329, -25201, -25072, -24942, -24811, -24679, -24546, -24413,
 -24278, -24143, -24006, -23869, -23731, -23592, -23452, -23311,
 -23169, -23027, -22883, -22739, -22594, -22448, -22301, -22153,
 -22004, -21855, -21705, -21554, -21402, -21249, -21096, -20942,
 -20787, -20631, -20474, -20317, -20159, -20000, -19840, -19680,
 -19519, -19357, -19194, -19031, -18867, -18702, -18537, -18371,
 -18204, -18036, -17868, -17699, -17530, -17360, -17189, -17017,
 -16845, -16672, -16499, -16325, -16150, -15975, -15799, -15623,
 -15446, -15268, -15090, -14911, -14732, -14552, -14372, -14191,
 -14009, -13827, -13645, -13462, -13278, -13094, -12909, -12724,
 -12539, -12353, -12166, -11980, -11792, -11604, -11416, -11227,
 -11038, -10849, -10659, -10469, -10278, -10087,  -9895,  -9703,
  -9511,  -9319,  -9126,  -8932,  -8739,  -8545,  -8351,  -8156,
  -7961,  -7766,  -7571,  -7375,  -7179,  -6982,  -6786,  -6589,
  -6392,  -6195,  -5997,  -5799,  -5601,  -5403,  -5205,  -5006,
  -4807,  -4608,  -4409,  -4210,  -4011,  -3811,  -3611,  -3411,
  -3211,  -3011,  -2811,  -2610,  -2410,  -2209,  -2009,  -1808,
  -1607,  -1406,  -1206,  -1005,   -804,   -603,   -402,   -201
};

int WebRtcSpl_ComplexFFT(WebRtc_Word16 frfi[], int stages, int mode)
{
    int i, j, l, k, istep, n, m;
    WebRtc_Word16 wr, wi;
    WebRtc_Word32 tr32, ti32, qr32, qi32;

    /* The 1024-value is a constant given from the size of kSinTable1024[],
     * and should not be changed depending on the input parameter 'stages'
     */
    n = 1 << stages;
    if (n > 1024)
        return -1;

    l = 1;
    k = 10 - 1; /* Constant for given kSinTable1024[]. Do not change
         depending on the input parameter 'stages' */

    if (mode == 0)
    {
        // mode==0: Low-complexity and Low-accuracy mode
        while (l < n)
        {
            istep = l << 1;

            for (m = 0; m < l; ++m)
            {
                j = m << k;

                /* The 256-value is a constant given as 1/4 of the size of
                 * kSinTable1024[], and should not be changed depending on the input
                 * parameter 'stages'. It will result in 0 <= j < N_SINE_WAVE/2
                 */
                wr = kSinTable1024[j + 256];
                wi = -kSinTable1024[j];

                for (i = m; i < n; i += istep)
                {
                    j = i + l;

                    tr32 = WEBRTC_SPL_RSHIFT_W32((WEBRTC_SPL_MUL_16_16(wr, frfi[2 * j])
                            - WEBRTC_SPL_MUL_16_16(wi, frfi[2 * j + 1])), 15);

                    ti32 = WEBRTC_SPL_RSHIFT_W32((WEBRTC_SPL_MUL_16_16(wr, frfi[2 * j + 1])
                            + WEBRTC_SPL_MUL_16_16(wi, frfi[2 * j])), 15);

                    qr32 = (WebRtc_Word32)frfi[2 * i];
                    qi32 = (WebRtc_Word32)frfi[2 * i + 1];
                    frfi[2 * j] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(qr32 - tr32, 1);
                    frfi[2 * j + 1] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(qi32 - ti32, 1);
                    frfi[2 * i] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(qr32 + tr32, 1);
                    frfi[2 * i + 1] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(qi32 + ti32, 1);
                }
            }

            --k;
            l = istep;

        }

    } else
    {
        // mode==1: High-complexity and High-accuracy mode
        while (l < n)
        {
            istep = l << 1;

            for (m = 0; m < l; ++m)
            {
                j = m << k;

                /* The 256-value is a constant given as 1/4 of the size of
                 * kSinTable1024[], and should not be changed depending on the input
                 * parameter 'stages'. It will result in 0 <= j < N_SINE_WAVE/2
                 */
                wr = kSinTable1024[j + 256];
                wi = -kSinTable1024[j];

#ifdef WEBRTC_ARCH_ARM_V7
                WebRtc_Word32 wri;
                WebRtc_Word32 frfi_r;
                __asm__("pkhbt %0, %1, %2, lsl #16" : "=r"(wri) :
                    "r"((WebRtc_Word32)wr), "r"((WebRtc_Word32)wi));
#endif

                for (i = m; i < n; i += istep)
                {
                    j = i + l;

#ifdef WEBRTC_ARCH_ARM_V7
                    __asm__("pkhbt %0, %1, %2, lsl #16" : "=r"(frfi_r) :
                        "r"((WebRtc_Word32)frfi[2*j]), "r"((WebRtc_Word32)frfi[2*j +1]));
                    __asm__("smlsd %0, %1, %2, %3" : "=r"(tr32) :
                        "r"(wri), "r"(frfi_r), "r"(CFFTRND));
                    __asm__("smladx %0, %1, %2, %3" : "=r"(ti32) :
                        "r"(wri), "r"(frfi_r), "r"(CFFTRND));
    
#else
                    tr32 = WEBRTC_SPL_MUL_16_16(wr, frfi[2 * j])
                            - WEBRTC_SPL_MUL_16_16(wi, frfi[2 * j + 1]) + CFFTRND;

                    ti32 = WEBRTC_SPL_MUL_16_16(wr, frfi[2 * j + 1])
                            + WEBRTC_SPL_MUL_16_16(wi, frfi[2 * j]) + CFFTRND;
#endif

                    tr32 = WEBRTC_SPL_RSHIFT_W32(tr32, 15 - CFFTSFT);
                    ti32 = WEBRTC_SPL_RSHIFT_W32(ti32, 15 - CFFTSFT);

                    qr32 = ((WebRtc_Word32)frfi[2 * i]) << CFFTSFT;
                    qi32 = ((WebRtc_Word32)frfi[2 * i + 1]) << CFFTSFT;

                    frfi[2 * j] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(
                            (qr32 - tr32 + CFFTRND2), 1 + CFFTSFT);
                    frfi[2 * j + 1] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(
                            (qi32 - ti32 + CFFTRND2), 1 + CFFTSFT);
                    frfi[2 * i] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(
                            (qr32 + tr32 + CFFTRND2), 1 + CFFTSFT);
                    frfi[2 * i + 1] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(
                            (qi32 + ti32 + CFFTRND2), 1 + CFFTSFT);
                }
            }

            --k;
            l = istep;
        }
    }
    return 0;
}

int WebRtcSpl_ComplexIFFT(WebRtc_Word16 frfi[], int stages, int mode)
{
    int i, j, l, k, istep, n, m, scale, shift;
    WebRtc_Word16 wr, wi;
    WebRtc_Word32 tr32, ti32, qr32, qi32;
    WebRtc_Word32 tmp32, round2;

    /* The 1024-value is a constant given from the size of kSinTable1024[],
     * and should not be changed depending on the input parameter 'stages'
     */
    n = 1 << stages;
    if (n > 1024)
        return -1;

    scale = 0;

    l = 1;
    k = 10 - 1; /* Constant for given kSinTable1024[]. Do not change
         depending on the input parameter 'stages' */

    while (l < n)
    {
        // variable scaling, depending upon data
        shift = 0;
        round2 = 8192;

        tmp32 = (WebRtc_Word32)WebRtcSpl_MaxAbsValueW16(frfi, 2 * n);
        if (tmp32 > 13573)
        {
            shift++;
            scale++;
            round2 <<= 1;
        }
        if (tmp32 > 27146)
        {
            shift++;
            scale++;
            round2 <<= 1;
        }

        istep = l << 1;

        if (mode == 0)
        {
            // mode==0: Low-complexity and Low-accuracy mode
            for (m = 0; m < l; ++m)
            {
                j = m << k;

                /* The 256-value is a constant given as 1/4 of the size of
                 * kSinTable1024[], and should not be changed depending on the input
                 * parameter 'stages'. It will result in 0 <= j < N_SINE_WAVE/2
                 */
                wr = kSinTable1024[j + 256];
                wi = kSinTable1024[j];

                for (i = m; i < n; i += istep)
                {
                    j = i + l;

                    tr32 = WEBRTC_SPL_RSHIFT_W32((WEBRTC_SPL_MUL_16_16_RSFT(wr, frfi[2 * j], 0)
                            - WEBRTC_SPL_MUL_16_16_RSFT(wi, frfi[2 * j + 1], 0)), 15);

                    ti32 = WEBRTC_SPL_RSHIFT_W32(
                            (WEBRTC_SPL_MUL_16_16_RSFT(wr, frfi[2 * j + 1], 0)
                                    + WEBRTC_SPL_MUL_16_16_RSFT(wi,frfi[2*j],0)), 15);

                    qr32 = (WebRtc_Word32)frfi[2 * i];
                    qi32 = (WebRtc_Word32)frfi[2 * i + 1];
                    frfi[2 * j] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(qr32 - tr32, shift);
                    frfi[2 * j + 1] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(qi32 - ti32, shift);
                    frfi[2 * i] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(qr32 + tr32, shift);
                    frfi[2 * i + 1] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(qi32 + ti32, shift);
                }
            }
        } else
        {
            // mode==1: High-complexity and High-accuracy mode

            for (m = 0; m < l; ++m)
            {
                j = m << k;

                /* The 256-value is a constant given as 1/4 of the size of
                 * kSinTable1024[], and should not be changed depending on the input
                 * parameter 'stages'. It will result in 0 <= j < N_SINE_WAVE/2
                 */
                wr = kSinTable1024[j + 256];
                wi = kSinTable1024[j];

#ifdef WEBRTC_ARCH_ARM_V7
                WebRtc_Word32 wri;
                WebRtc_Word32 frfi_r;
                __asm__("pkhbt %0, %1, %2, lsl #16" : "=r"(wri) :
                    "r"((WebRtc_Word32)wr), "r"((WebRtc_Word32)wi));
#endif

                for (i = m; i < n; i += istep)
                {
                    j = i + l;

#ifdef WEBRTC_ARCH_ARM_V7
                    __asm__("pkhbt %0, %1, %2, lsl #16" : "=r"(frfi_r) :
                        "r"((WebRtc_Word32)frfi[2*j]), "r"((WebRtc_Word32)frfi[2*j +1]));
                    __asm__("smlsd %0, %1, %2, %3" : "=r"(tr32) :
                        "r"(wri), "r"(frfi_r), "r"(CIFFTRND));
                    __asm__("smladx %0, %1, %2, %3" : "=r"(ti32) :
                        "r"(wri), "r"(frfi_r), "r"(CIFFTRND));
#else

                    tr32 = WEBRTC_SPL_MUL_16_16(wr, frfi[2 * j])
                            - WEBRTC_SPL_MUL_16_16(wi, frfi[2 * j + 1]) + CIFFTRND;

                    ti32 = WEBRTC_SPL_MUL_16_16(wr, frfi[2 * j + 1])
                            + WEBRTC_SPL_MUL_16_16(wi, frfi[2 * j]) + CIFFTRND;
#endif
                    tr32 = WEBRTC_SPL_RSHIFT_W32(tr32, 15 - CIFFTSFT);
                    ti32 = WEBRTC_SPL_RSHIFT_W32(ti32, 15 - CIFFTSFT);

                    qr32 = ((WebRtc_Word32)frfi[2 * i]) << CIFFTSFT;
                    qi32 = ((WebRtc_Word32)frfi[2 * i + 1]) << CIFFTSFT;

                    frfi[2 * j] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32((qr32 - tr32+round2),
                                                                       shift+CIFFTSFT);
                    frfi[2 * j + 1] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(
                            (qi32 - ti32 + round2), shift + CIFFTSFT);
                    frfi[2 * i] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32((qr32 + tr32 + round2),
                                                                       shift + CIFFTSFT);
                    frfi[2 * i + 1] = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(
                            (qi32 + ti32 + round2), shift + CIFFTSFT);
                }
            }

        }
        --k;
        l = istep;
    }
    return scale;
}


/* C version of WebRtcSpl_CrossCorrelation() for generic platforms. */
void WebRtcSpl_CrossCorrelationC(int32_t* cross_correlation,
                                 const int16_t* seq1,
                                 const int16_t* seq2,
                                 int16_t dim_seq,
                                 int16_t dim_cross_correlation,
                                 int16_t right_shifts,
                                 int16_t step_seq2) {
  int i = 0, j = 0;

  for (i = 0; i < dim_cross_correlation; i++) {
    *cross_correlation = 0;
    /* Unrolling doesn't seem to improve performance. */
    for (j = 0; j < dim_seq; j++) {
      *cross_correlation += (seq1[j] * seq2[step_seq2 * i + j]) >> right_shifts;
    }
    cross_correlation++;
  }
}


WebRtc_UWord32 WebRtcSpl_DivU32U16(WebRtc_UWord32 num, WebRtc_UWord16 den)
{
    // Guard against division with 0
    if (den != 0)
    {
        return (WebRtc_UWord32)(num / den);
    } else
    {
        return (WebRtc_UWord32)0xFFFFFFFF;
    }
}

WebRtc_Word32 WebRtcSpl_DivW32W16(WebRtc_Word32 num, WebRtc_Word16 den)
{
    // Guard against division with 0
    if (den != 0)
    {
        return (WebRtc_Word32)(num / den);
    } else
    {
        return (WebRtc_Word32)0x7FFFFFFF;
    }
}

WebRtc_Word16 WebRtcSpl_DivW32W16ResW16(WebRtc_Word32 num, WebRtc_Word16 den)
{
    // Guard against division with 0
    if (den != 0)
    {
        return (WebRtc_Word16)(num / den);
    } else
    {
        return (WebRtc_Word16)0x7FFF;
    }
}

WebRtc_Word32 WebRtcSpl_DivResultInQ31(WebRtc_Word32 num, WebRtc_Word32 den)
{
    WebRtc_Word32 L_num = num;
    WebRtc_Word32 L_den = den;
    WebRtc_Word32 div = 0;
    int k = 31;
    int change_sign = 0;

    if (num == 0)
        return 0;

    if (num < 0)
    {
        change_sign++;
        L_num = -num;
    }
    if (den < 0)
    {
        change_sign++;
        L_den = -den;
    }
    while (k--)
    {
        div <<= 1;
        L_num <<= 1;
        if (L_num >= L_den)
        {
            L_num -= L_den;
            div++;
        }
    }
    if (change_sign == 1)
    {
        div = -div;
    }
    return div;
}

WebRtc_Word32 WebRtcSpl_DivW32HiLow(WebRtc_Word32 num, WebRtc_Word16 den_hi,
                                    WebRtc_Word16 den_low)
{
    WebRtc_Word16 approx, tmp_hi, tmp_low, num_hi, num_low;
    WebRtc_Word32 tmpW32;

    approx = (WebRtc_Word16)WebRtcSpl_DivW32W16((WebRtc_Word32)0x1FFFFFFF, den_hi);
    // result in Q14 (Note: 3FFFFFFF = 0.5 in Q30)

    // tmpW32 = 1/den = approx * (2.0 - den * approx) (in Q30)
    tmpW32 = (WEBRTC_SPL_MUL_16_16(den_hi, approx) << 1)
            + ((WEBRTC_SPL_MUL_16_16(den_low, approx) >> 15) << 1);
    // tmpW32 = den * approx

    tmpW32 = (WebRtc_Word32)0x7fffffffL - tmpW32; // result in Q30 (tmpW32 = 2.0-(den*approx))

    // Store tmpW32 in hi and low format
    tmp_hi = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(tmpW32, 16);
    tmp_low = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32((tmpW32
            - WEBRTC_SPL_LSHIFT_W32((WebRtc_Word32)tmp_hi, 16)), 1);

    // tmpW32 = 1/den in Q29
    tmpW32 = ((WEBRTC_SPL_MUL_16_16(tmp_hi, approx) + (WEBRTC_SPL_MUL_16_16(tmp_low, approx)
            >> 15)) << 1);

    // 1/den in hi and low format
    tmp_hi = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(tmpW32, 16);
    tmp_low = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32((tmpW32
            - WEBRTC_SPL_LSHIFT_W32((WebRtc_Word32)tmp_hi, 16)), 1);

    // Store num in hi and low format
    num_hi = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32(num, 16);
    num_low = (WebRtc_Word16)WEBRTC_SPL_RSHIFT_W32((num
            - WEBRTC_SPL_LSHIFT_W32((WebRtc_Word32)num_hi, 16)), 1);

    // num * (1/den) by 32 bit multiplication (result in Q28)

    tmpW32 = (WEBRTC_SPL_MUL_16_16(num_hi, tmp_hi) + (WEBRTC_SPL_MUL_16_16(num_hi, tmp_low)
            >> 15) + (WEBRTC_SPL_MUL_16_16(num_low, tmp_hi) >> 15));

    // Put result in Q31 (convert from Q28)
    tmpW32 = WEBRTC_SPL_LSHIFT_W32(tmpW32, 3);

    return tmpW32;
}

// TODO(Bjornv): Change the function parameter order to WebRTC code style.
// C version of WebRtcSpl_DownsampleFast() for generic platforms.
int WebRtcSpl_DownsampleFastC(const int16_t* data_in,
                              int data_in_length,
                              int16_t* data_out,
                              int data_out_length,
                              const int16_t* __restrict coefficients,
                              int coefficients_length,
                              int factor,
                              int delay) {
  int i = 0;
  int j = 0;
  int32_t out_s32 = 0;
  int endpos = delay + factor * (data_out_length - 1) + 1;

  // Return error if any of the running conditions doesn't meet.
  if (data_out_length <= 0 || coefficients_length <= 0
                           || data_in_length < endpos) {
    return -1;
  }

  for (i = delay; i < endpos; i += factor) {
    out_s32 = 2048;  // Round value, 0.5 in Q12.

    for (j = 0; j < coefficients_length; j++) {
      out_s32 += coefficients[j] * data_in[i - j];  // Q12.
    }

    out_s32 >>= 12;  // Q0.

    // Saturate and store the output.
    *data_out++ = WebRtcSpl_SatW32ToW16(out_s32);
  }

  return 0;
}

int WebRtcSpl_GetScalingSquare(WebRtc_Word16 *in_vector, int in_vector_length, int times)
{
    int nbits = WebRtcSpl_GetSizeInBits(times);
    int i;
    WebRtc_Word16 smax = -1;
    WebRtc_Word16 sabs;
    WebRtc_Word16 *sptr = in_vector;
    int t;
    int looptimes = in_vector_length;

    for (i = looptimes; i > 0; i--)
    {
        sabs = (*sptr > 0 ? *sptr++ : -*sptr++);
        smax = (sabs > smax ? sabs : smax);
    }
    t = WebRtcSpl_NormW32(WEBRTC_SPL_MUL(smax, smax));

    if (smax == 0)
    {
        return 0; // Since norm(0) returns 0
    } else
    {
        return (t > nbits) ? 0 : nbits - t;
    }
}


// Maximum absolute value of word16 vector. C version for generic platforms.
int16_t WebRtcSpl_MaxAbsValueW16C(const int16_t* vector, int length) {
  int i = 0, absolute = 0, maximum = 0;

  if (vector == NULL || length <= 0) {
    return -1;
  }

  for (i = 0; i < length; i++) {
    absolute = abs((int)vector[i]);

    if (absolute > maximum) {
      maximum = absolute;
    }
  }

  // Guard the case for abs(-32768).
  if (maximum > WEBRTC_SPL_WORD16_MAX) {
    maximum = WEBRTC_SPL_WORD16_MAX;
  }

  return (int16_t)maximum;
}

// Maximum absolute value of word32 vector. C version for generic platforms.
int32_t WebRtcSpl_MaxAbsValueW32C(const int32_t* vector, int length) {
  // Use uint32_t for the local variables, to accommodate the return value
  // of abs(0x80000000), which is 0x80000000.

  uint32_t absolute = 0, maximum = 0;
  int i = 0;

  if (vector == NULL || length <= 0) {
    return -1;
  }

  for (i = 0; i < length; i++) {
    absolute = abs((int)vector[i]);
    if (absolute > maximum) {
      maximum = absolute;
    }
  }

  maximum = WEBRTC_SPL_MIN(maximum, WEBRTC_SPL_WORD32_MAX);

  return (int32_t)maximum;
}

// Maximum value of word16 vector. C version for generic platforms.
int16_t WebRtcSpl_MaxValueW16C(const int16_t* vector, int length) {
  int16_t maximum = WEBRTC_SPL_WORD16_MIN;
  int i = 0;

  if (vector == NULL || length <= 0) {
    return maximum;
  }

  for (i = 0; i < length; i++) {
    if (vector[i] > maximum)
      maximum = vector[i];
  }
  return maximum;
}

// Maximum value of word32 vector. C version for generic platforms.
int32_t WebRtcSpl_MaxValueW32C(const int32_t* vector, int length) {
  int32_t maximum = WEBRTC_SPL_WORD32_MIN;
  int i = 0;

  if (vector == NULL || length <= 0) {
    return maximum;
  }

  for (i = 0; i < length; i++) {
    if (vector[i] > maximum)
      maximum = vector[i];
  }
  return maximum;
}

// Minimum value of word16 vector. C version for generic platforms.
int16_t WebRtcSpl_MinValueW16C(const int16_t* vector, int length) {
  int16_t minimum = WEBRTC_SPL_WORD16_MAX;
  int i = 0;

  if (vector == NULL || length <= 0) {
    return minimum;
  }

  for (i = 0; i < length; i++) {
    if (vector[i] < minimum)
      minimum = vector[i];
  }
  return minimum;
}

// Minimum value of word32 vector. C version for generic platforms.
int32_t WebRtcSpl_MinValueW32C(const int32_t* vector, int length) {
  int32_t minimum = WEBRTC_SPL_WORD32_MAX;
  int i = 0;

  if (vector == NULL || length <= 0) {
    return minimum;
  }

  for (i = 0; i < length; i++) {
    if (vector[i] < minimum)
      minimum = vector[i];
  }
  return minimum;
}

// Index of maximum absolute value in a word16 vector.
int WebRtcSpl_MaxAbsIndexW16(const int16_t* vector, int length) {
  // Use type int for local variables, to accomodate the value of abs(-32768).

  int i = 0, absolute = 0, maximum = 0, index = 0;

  if (vector == NULL || length <= 0) {
    return -1;
  }

  for (i = 0; i < length; i++) {
    absolute = abs((int)vector[i]);

    if (absolute > maximum) {
      maximum = absolute;
      index = i;
    }
  }

  return index;
}

// Index of maximum value in a word16 vector.
int WebRtcSpl_MaxIndexW16(const int16_t* vector, int length) {
  int i = 0, index = 0;
  int16_t maximum = WEBRTC_SPL_WORD16_MIN;

  if (vector == NULL || length <= 0) {
    return -1;
  }

  for (i = 0; i < length; i++) {
    if (vector[i] > maximum) {
      maximum = vector[i];
      index = i;
    }
  }

  return index;
}

// Index of maximum value in a word32 vector.
int WebRtcSpl_MaxIndexW32(const int32_t* vector, int length) {
  int i = 0, index = 0;
  int32_t maximum = WEBRTC_SPL_WORD32_MIN;

  if (vector == NULL || length <= 0) {
    return -1;
  }

  for (i = 0; i < length; i++) {
    if (vector[i] > maximum) {
      maximum = vector[i];
      index = i;
    }
  }

  return index;
}

// Index of minimum value in a word16 vector.
int WebRtcSpl_MinIndexW16(const int16_t* vector, int length) {
  int i = 0, index = 0;
  int16_t minimum = WEBRTC_SPL_WORD16_MAX;

  if (vector == NULL || length <= 0) {
    return -1;
  }

  for (i = 0; i < length; i++) {
    if (vector[i] < minimum) {
      minimum = vector[i];
      index = i;
    }
  }

  return index;
}

// Index of minimum value in a word32 vector.
int WebRtcSpl_MinIndexW32(const int32_t* vector, int length) {
  int i = 0, index = 0;
  int32_t minimum = WEBRTC_SPL_WORD32_MAX;

  if (vector == NULL || length <= 0) {
    return -1;
  }

  for (i = 0; i < length; i++) {
    if (vector[i] < minimum) {
      minimum = vector[i];
      index = i;
    }
  }

  return index;
}


struct RealFFT {
  int order;
};

struct RealFFT* WebRtcSpl_CreateRealFFT(int order) {
  struct RealFFT* self = NULL;

  // This constraint comes from ComplexFFT().
  if (order > 10 || order < 0) {
    return NULL;
  }

  self = malloc(sizeof(struct RealFFT));
  self->order = order;

  return self;
}

void WebRtcSpl_FreeRealFFT(struct RealFFT* self) {
  free(self);
}

// WebRtcSpl_ComplexFFT and WebRtcSpl_ComplexIFFT use in-place algorithm,
// so copy data from data_in to data_out in the next two functions.

int WebRtcSpl_RealForwardFFTC(struct RealFFT* self,
                              const int16_t* data_in,
                              int16_t* data_out) {
  memcpy(data_out, data_in, sizeof(int16_t) * (1 << (self->order + 1)));
  WebRtcSpl_ComplexBitReverse(data_out, self->order);
  return WebRtcSpl_ComplexFFT(data_out, self->order, 1);
}

int WebRtcSpl_RealInverseFFTC(struct RealFFT* self,
                              const int16_t* data_in,
                              int16_t* data_out) {
  memcpy(data_out, data_in, sizeof(int16_t) * (1 << (self->order + 1)));
  WebRtcSpl_ComplexBitReverse(data_out, self->order);
  return WebRtcSpl_ComplexIFFT(data_out, self->order, 1);
}

#if defined(WEBRTC_DETECT_ARM_NEON) || defined(WEBRTC_ARCH_ARM_NEON)
// TODO(kma): Replace the following function bodies into optimized functions
// for ARM Neon.
int WebRtcSpl_RealForwardFFTNeon(struct RealFFT* self,
                                 const int16_t* data_in,
                                 int16_t* data_out) {
  return WebRtcSpl_RealForwardFFTC(self, data_in, data_out);
}

int WebRtcSpl_RealInverseFFTNeon(struct RealFFT* self,
                                 const int16_t* data_in,
                                 int16_t* data_out) {
  return WebRtcSpl_RealInverseFFTC(self, data_in, data_out);
}
#endif

// 48 -> 16 resampler
void WebRtcSpl_Resample48khzTo16khz(const WebRtc_Word16* in, WebRtc_Word16* out,
                                    WebRtcSpl_State48khzTo16khz* state, WebRtc_Word32* tmpmem)
{
    ///// 48 --> 48(LP) /////
    // WebRtc_Word16  in[480]
    // WebRtc_Word32 out[480]
    /////
    WebRtcSpl_LPBy2ShortToInt(in, 480, tmpmem + 16, state->S_48_48);

    ///// 48 --> 32 /////
    // WebRtc_Word32  in[480]
    // WebRtc_Word32 out[320]
    /////
    // copy state to and from input array
    memcpy(tmpmem + 8, state->S_48_32, 8 * sizeof(WebRtc_Word32));
    memcpy(state->S_48_32, tmpmem + 488, 8 * sizeof(WebRtc_Word32));
    WebRtcSpl_Resample48khzTo32khz(tmpmem + 8, tmpmem, 160);

    ///// 32 --> 16 /////
    // WebRtc_Word32  in[320]
    // WebRtc_Word16 out[160]
    /////
    WebRtcSpl_DownBy2IntToShort(tmpmem, 320, out, state->S_32_16);
}

// initialize state of 48 -> 16 resampler
void WebRtcSpl_ResetResample48khzTo16khz(WebRtcSpl_State48khzTo16khz* state)
{
    memset(state->S_48_48, 0, 16 * sizeof(WebRtc_Word32));
    memset(state->S_48_32, 0, 8 * sizeof(WebRtc_Word32));
    memset(state->S_32_16, 0, 8 * sizeof(WebRtc_Word32));
}

////////////////////////////
///// 16 kHz -> 48 kHz /////
////////////////////////////

// 16 -> 48 resampler
void WebRtcSpl_Resample16khzTo48khz(const WebRtc_Word16* in, WebRtc_Word16* out,
                                    WebRtcSpl_State16khzTo48khz* state, WebRtc_Word32* tmpmem)
{
    ///// 16 --> 32 /////
    // WebRtc_Word16  in[160]
    // WebRtc_Word32 out[320]
    /////
    WebRtcSpl_UpBy2ShortToInt(in, 160, tmpmem + 16, state->S_16_32);

    ///// 32 --> 24 /////
    // WebRtc_Word32  in[320]
    // WebRtc_Word32 out[240]
    // copy state to and from input array
    /////
    memcpy(tmpmem + 8, state->S_32_24, 8 * sizeof(WebRtc_Word32));
    memcpy(state->S_32_24, tmpmem + 328, 8 * sizeof(WebRtc_Word32));
    WebRtcSpl_Resample32khzTo24khz(tmpmem + 8, tmpmem, 80);

    ///// 24 --> 48 /////
    // WebRtc_Word32  in[240]
    // WebRtc_Word16 out[480]
    /////
    WebRtcSpl_UpBy2IntToShort(tmpmem, 240, out, state->S_24_48);
}

// initialize state of 16 -> 48 resampler
void WebRtcSpl_ResetResample16khzTo48khz(WebRtcSpl_State16khzTo48khz* state)
{
    memset(state->S_16_32, 0, 8 * sizeof(WebRtc_Word32));
    memset(state->S_32_24, 0, 8 * sizeof(WebRtc_Word32));
    memset(state->S_24_48, 0, 8 * sizeof(WebRtc_Word32));
}

////////////////////////////
///// 48 kHz ->  8 kHz /////
////////////////////////////

// 48 -> 8 resampler
void WebRtcSpl_Resample48khzTo8khz(const WebRtc_Word16* in, WebRtc_Word16* out,
                                   WebRtcSpl_State48khzTo8khz* state, WebRtc_Word32* tmpmem)
{
    ///// 48 --> 24 /////
    // WebRtc_Word16  in[480]
    // WebRtc_Word32 out[240]
    /////
    WebRtcSpl_DownBy2ShortToInt(in, 480, tmpmem + 256, state->S_48_24);

    ///// 24 --> 24(LP) /////
    // WebRtc_Word32  in[240]
    // WebRtc_Word32 out[240]
    /////
    WebRtcSpl_LPBy2IntToInt(tmpmem + 256, 240, tmpmem + 16, state->S_24_24);

    ///// 24 --> 16 /////
    // WebRtc_Word32  in[240]
    // WebRtc_Word32 out[160]
    /////
    // copy state to and from input array
    memcpy(tmpmem + 8, state->S_24_16, 8 * sizeof(WebRtc_Word32));
    memcpy(state->S_24_16, tmpmem + 248, 8 * sizeof(WebRtc_Word32));
    WebRtcSpl_Resample48khzTo32khz(tmpmem + 8, tmpmem, 80);

    ///// 16 --> 8 /////
    // WebRtc_Word32  in[160]
    // WebRtc_Word16 out[80]
    /////
    WebRtcSpl_DownBy2IntToShort(tmpmem, 160, out, state->S_16_8);
}

// initialize state of 48 -> 8 resampler
void WebRtcSpl_ResetResample48khzTo8khz(WebRtcSpl_State48khzTo8khz* state)
{
    memset(state->S_48_24, 0, 8 * sizeof(WebRtc_Word32));
    memset(state->S_24_24, 0, 16 * sizeof(WebRtc_Word32));
    memset(state->S_24_16, 0, 8 * sizeof(WebRtc_Word32));
    memset(state->S_16_8, 0, 8 * sizeof(WebRtc_Word32));
}

////////////////////////////
/////  8 kHz -> 48 kHz /////
////////////////////////////

// 8 -> 48 resampler
void WebRtcSpl_Resample8khzTo48khz(const WebRtc_Word16* in, WebRtc_Word16* out,
                                   WebRtcSpl_State8khzTo48khz* state, WebRtc_Word32* tmpmem)
{
    ///// 8 --> 16 /////
    // WebRtc_Word16  in[80]
    // WebRtc_Word32 out[160]
    /////
    WebRtcSpl_UpBy2ShortToInt(in, 80, tmpmem + 264, state->S_8_16);

    ///// 16 --> 12 /////
    // WebRtc_Word32  in[160]
    // WebRtc_Word32 out[120]
    /////
    // copy state to and from input array
    memcpy(tmpmem + 256, state->S_16_12, 8 * sizeof(WebRtc_Word32));
    memcpy(state->S_16_12, tmpmem + 416, 8 * sizeof(WebRtc_Word32));
    WebRtcSpl_Resample32khzTo24khz(tmpmem + 256, tmpmem + 240, 40);

    ///// 12 --> 24 /////
    // WebRtc_Word32  in[120]
    // WebRtc_Word16 out[240]
    /////
    WebRtcSpl_UpBy2IntToInt(tmpmem + 240, 120, tmpmem, state->S_12_24);

    ///// 24 --> 48 /////
    // WebRtc_Word32  in[240]
    // WebRtc_Word16 out[480]
    /////
    WebRtcSpl_UpBy2IntToShort(tmpmem, 240, out, state->S_24_48);
}

// initialize state of 8 -> 48 resampler
void WebRtcSpl_ResetResample8khzTo48khz(WebRtcSpl_State8khzTo48khz* state)
{
    memset(state->S_8_16, 0, 8 * sizeof(WebRtc_Word32));
    memset(state->S_16_12, 0, 8 * sizeof(WebRtc_Word32));
    memset(state->S_12_24, 0, 8 * sizeof(WebRtc_Word32));
    memset(state->S_24_48, 0, 8 * sizeof(WebRtc_Word32));
}

// allpass filter coefficients.
static const WebRtc_Word16 kResampleAllpass[2][3] = {
        {821, 6110, 12382},
        {3050, 9368, 15063}
};

//
//   decimator
// input:  WebRtc_Word32 (shifted 15 positions to the left, + offset 16384) OVERWRITTEN!
// output: WebRtc_Word16 (saturated) (of length len/2)
// state:  filter state array; length = 8

void WebRtcSpl_DownBy2IntToShort(WebRtc_Word32 *in, WebRtc_Word32 len, WebRtc_Word16 *out,
                                 WebRtc_Word32 *state)
{
    WebRtc_Word32 tmp0, tmp1, diff;
    WebRtc_Word32 i;

    len >>= 1;

    // lower allpass filter (operates on even input samples)
    for (i = 0; i < len; i++)
    {
        tmp0 = in[i << 1];
        diff = tmp0 - state[1];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[0] + diff * kResampleAllpass[1][0];
        state[0] = tmp0;
        diff = tmp1 - state[2];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[1] + diff * kResampleAllpass[1][1];
        state[1] = tmp1;
        diff = tmp0 - state[3];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[3] = state[2] + diff * kResampleAllpass[1][2];
        state[2] = tmp0;

        // divide by two and store temporarily
        in[i << 1] = (state[3] >> 1);
    }

    in++;

    // upper allpass filter (operates on odd input samples)
    for (i = 0; i < len; i++)
    {
        tmp0 = in[i << 1];
        diff = tmp0 - state[5];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[4] + diff * kResampleAllpass[0][0];
        state[4] = tmp0;
        diff = tmp1 - state[6];
        // scale down and round
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[5] + diff * kResampleAllpass[0][1];
        state[5] = tmp1;
        diff = tmp0 - state[7];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[7] = state[6] + diff * kResampleAllpass[0][2];
        state[6] = tmp0;

        // divide by two and store temporarily
        in[i << 1] = (state[7] >> 1);
    }

    in--;

    // combine allpass outputs
    for (i = 0; i < len; i += 2)
    {
        // divide by two, add both allpass outputs and round
        tmp0 = (in[i << 1] + in[(i << 1) + 1]) >> 15;
        tmp1 = (in[(i << 1) + 2] + in[(i << 1) + 3]) >> 15;
        if (tmp0 > (WebRtc_Word32)0x00007FFF)
            tmp0 = 0x00007FFF;
        if (tmp0 < (WebRtc_Word32)0xFFFF8000)
            tmp0 = 0xFFFF8000;
        out[i] = (WebRtc_Word16)tmp0;
        if (tmp1 > (WebRtc_Word32)0x00007FFF)
            tmp1 = 0x00007FFF;
        if (tmp1 < (WebRtc_Word32)0xFFFF8000)
            tmp1 = 0xFFFF8000;
        out[i + 1] = (WebRtc_Word16)tmp1;
    }
}

//
//   decimator
// input:  WebRtc_Word16
// output: WebRtc_Word32 (shifted 15 positions to the left, + offset 16384) (of length len/2)
// state:  filter state array; length = 8

void WebRtcSpl_DownBy2ShortToInt(const WebRtc_Word16 *in,
                                  WebRtc_Word32 len,
                                  WebRtc_Word32 *out,
                                  WebRtc_Word32 *state)
{
    WebRtc_Word32 tmp0, tmp1, diff;
    WebRtc_Word32 i;

    len >>= 1;

    // lower allpass filter (operates on even input samples)
    for (i = 0; i < len; i++)
    {
        tmp0 = ((WebRtc_Word32)in[i << 1] << 15) + (1 << 14);
        diff = tmp0 - state[1];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[0] + diff * kResampleAllpass[1][0];
        state[0] = tmp0;
        diff = tmp1 - state[2];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[1] + diff * kResampleAllpass[1][1];
        state[1] = tmp1;
        diff = tmp0 - state[3];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[3] = state[2] + diff * kResampleAllpass[1][2];
        state[2] = tmp0;

        // divide by two and store temporarily
        out[i] = (state[3] >> 1);
    }

    in++;

    // upper allpass filter (operates on odd input samples)
    for (i = 0; i < len; i++)
    {
        tmp0 = ((WebRtc_Word32)in[i << 1] << 15) + (1 << 14);
        diff = tmp0 - state[5];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[4] + diff * kResampleAllpass[0][0];
        state[4] = tmp0;
        diff = tmp1 - state[6];
        // scale down and round
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[5] + diff * kResampleAllpass[0][1];
        state[5] = tmp1;
        diff = tmp0 - state[7];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[7] = state[6] + diff * kResampleAllpass[0][2];
        state[6] = tmp0;

        // divide by two and store temporarily
        out[i] += (state[7] >> 1);
    }

    in--;
}

//
//   interpolator
// input:  WebRtc_Word16
// output: WebRtc_Word32 (normalized, not saturated) (of length len*2)
// state:  filter state array; length = 8
void WebRtcSpl_UpBy2ShortToInt(const WebRtc_Word16 *in, WebRtc_Word32 len, WebRtc_Word32 *out,
                               WebRtc_Word32 *state)
{
    WebRtc_Word32 tmp0, tmp1, diff;
    WebRtc_Word32 i;

    // upper allpass filter (generates odd output samples)
    for (i = 0; i < len; i++)
    {
        tmp0 = ((WebRtc_Word32)in[i] << 15) + (1 << 14);
        diff = tmp0 - state[5];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[4] + diff * kResampleAllpass[0][0];
        state[4] = tmp0;
        diff = tmp1 - state[6];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[5] + diff * kResampleAllpass[0][1];
        state[5] = tmp1;
        diff = tmp0 - state[7];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[7] = state[6] + diff * kResampleAllpass[0][2];
        state[6] = tmp0;

        // scale down, round and store
        out[i << 1] = state[7] >> 15;
    }

    out++;

    // lower allpass filter (generates even output samples)
    for (i = 0; i < len; i++)
    {
        tmp0 = ((WebRtc_Word32)in[i] << 15) + (1 << 14);
        diff = tmp0 - state[1];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[0] + diff * kResampleAllpass[1][0];
        state[0] = tmp0;
        diff = tmp1 - state[2];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[1] + diff * kResampleAllpass[1][1];
        state[1] = tmp1;
        diff = tmp0 - state[3];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[3] = state[2] + diff * kResampleAllpass[1][2];
        state[2] = tmp0;

        // scale down, round and store
        out[i << 1] = state[3] >> 15;
    }
}

//
//   interpolator
// input:  WebRtc_Word32 (shifted 15 positions to the left, + offset 16384)
// output: WebRtc_Word32 (shifted 15 positions to the left, + offset 16384) (of length len*2)
// state:  filter state array; length = 8
void WebRtcSpl_UpBy2IntToInt(const WebRtc_Word32 *in, WebRtc_Word32 len, WebRtc_Word32 *out,
                             WebRtc_Word32 *state)
{
    WebRtc_Word32 tmp0, tmp1, diff;
    WebRtc_Word32 i;

    // upper allpass filter (generates odd output samples)
    for (i = 0; i < len; i++)
    {
        tmp0 = in[i];
        diff = tmp0 - state[5];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[4] + diff * kResampleAllpass[0][0];
        state[4] = tmp0;
        diff = tmp1 - state[6];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[5] + diff * kResampleAllpass[0][1];
        state[5] = tmp1;
        diff = tmp0 - state[7];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[7] = state[6] + diff * kResampleAllpass[0][2];
        state[6] = tmp0;

        // scale down, round and store
        out[i << 1] = state[7];
    }

    out++;

    // lower allpass filter (generates even output samples)
    for (i = 0; i < len; i++)
    {
        tmp0 = in[i];
        diff = tmp0 - state[1];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[0] + diff * kResampleAllpass[1][0];
        state[0] = tmp0;
        diff = tmp1 - state[2];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[1] + diff * kResampleAllpass[1][1];
        state[1] = tmp1;
        diff = tmp0 - state[3];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[3] = state[2] + diff * kResampleAllpass[1][2];
        state[2] = tmp0;

        // scale down, round and store
        out[i << 1] = state[3];
    }
}

//
//   interpolator
// input:  WebRtc_Word32 (shifted 15 positions to the left, + offset 16384)
// output: WebRtc_Word16 (saturated) (of length len*2)
// state:  filter state array; length = 8
void WebRtcSpl_UpBy2IntToShort(const WebRtc_Word32 *in, WebRtc_Word32 len, WebRtc_Word16 *out,
                               WebRtc_Word32 *state)
{
    WebRtc_Word32 tmp0, tmp1, diff;
    WebRtc_Word32 i;

    // upper allpass filter (generates odd output samples)
    for (i = 0; i < len; i++)
    {
        tmp0 = in[i];
        diff = tmp0 - state[5];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[4] + diff * kResampleAllpass[0][0];
        state[4] = tmp0;
        diff = tmp1 - state[6];
        // scale down and round
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[5] + diff * kResampleAllpass[0][1];
        state[5] = tmp1;
        diff = tmp0 - state[7];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[7] = state[6] + diff * kResampleAllpass[0][2];
        state[6] = tmp0;

        // scale down, saturate and store
        tmp1 = state[7] >> 15;
        if (tmp1 > (WebRtc_Word32)0x00007FFF)
            tmp1 = 0x00007FFF;
        if (tmp1 < (WebRtc_Word32)0xFFFF8000)
            tmp1 = 0xFFFF8000;
        out[i << 1] = (WebRtc_Word16)tmp1;
    }

    out++;

    // lower allpass filter (generates even output samples)
    for (i = 0; i < len; i++)
    {
        tmp0 = in[i];
        diff = tmp0 - state[1];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[0] + diff * kResampleAllpass[1][0];
        state[0] = tmp0;
        diff = tmp1 - state[2];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[1] + diff * kResampleAllpass[1][1];
        state[1] = tmp1;
        diff = tmp0 - state[3];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[3] = state[2] + diff * kResampleAllpass[1][2];
        state[2] = tmp0;

        // scale down, saturate and store
        tmp1 = state[3] >> 15;
        if (tmp1 > (WebRtc_Word32)0x00007FFF)
            tmp1 = 0x00007FFF;
        if (tmp1 < (WebRtc_Word32)0xFFFF8000)
            tmp1 = 0xFFFF8000;
        out[i << 1] = (WebRtc_Word16)tmp1;
    }
}

//   lowpass filter
// input:  WebRtc_Word16
// output: WebRtc_Word32 (normalized, not saturated)
// state:  filter state array; length = 8
void WebRtcSpl_LPBy2ShortToInt(const WebRtc_Word16* in, WebRtc_Word32 len, WebRtc_Word32* out,
                               WebRtc_Word32* state)
{
    WebRtc_Word32 tmp0, tmp1, diff;
    WebRtc_Word32 i;

    len >>= 1;

    // lower allpass filter: odd input -> even output samples
    in++;
    // initial state of polyphase delay element
    tmp0 = state[12];
    for (i = 0; i < len; i++)
    {
        diff = tmp0 - state[1];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[0] + diff * kResampleAllpass[1][0];
        state[0] = tmp0;
        diff = tmp1 - state[2];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[1] + diff * kResampleAllpass[1][1];
        state[1] = tmp1;
        diff = tmp0 - state[3];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[3] = state[2] + diff * kResampleAllpass[1][2];
        state[2] = tmp0;

        // scale down, round and store
        out[i << 1] = state[3] >> 1;
        tmp0 = ((WebRtc_Word32)in[i << 1] << 15) + (1 << 14);
    }
    in--;

    // upper allpass filter: even input -> even output samples
    for (i = 0; i < len; i++)
    {
        tmp0 = ((WebRtc_Word32)in[i << 1] << 15) + (1 << 14);
        diff = tmp0 - state[5];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[4] + diff * kResampleAllpass[0][0];
        state[4] = tmp0;
        diff = tmp1 - state[6];
        // scale down and round
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[5] + diff * kResampleAllpass[0][1];
        state[5] = tmp1;
        diff = tmp0 - state[7];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[7] = state[6] + diff * kResampleAllpass[0][2];
        state[6] = tmp0;

        // average the two allpass outputs, scale down and store
        out[i << 1] = (out[i << 1] + (state[7] >> 1)) >> 15;
    }

    // switch to odd output samples
    out++;

    // lower allpass filter: even input -> odd output samples
    for (i = 0; i < len; i++)
    {
        tmp0 = ((WebRtc_Word32)in[i << 1] << 15) + (1 << 14);
        diff = tmp0 - state[9];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[8] + diff * kResampleAllpass[1][0];
        state[8] = tmp0;
        diff = tmp1 - state[10];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[9] + diff * kResampleAllpass[1][1];
        state[9] = tmp1;
        diff = tmp0 - state[11];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[11] = state[10] + diff * kResampleAllpass[1][2];
        state[10] = tmp0;

        // scale down, round and store
        out[i << 1] = state[11] >> 1;
    }

    // upper allpass filter: odd input -> odd output samples
    in++;
    for (i = 0; i < len; i++)
    {
        tmp0 = ((WebRtc_Word32)in[i << 1] << 15) + (1 << 14);
        diff = tmp0 - state[13];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[12] + diff * kResampleAllpass[0][0];
        state[12] = tmp0;
        diff = tmp1 - state[14];
        // scale down and round
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[13] + diff * kResampleAllpass[0][1];
        state[13] = tmp1;
        diff = tmp0 - state[15];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[15] = state[14] + diff * kResampleAllpass[0][2];
        state[14] = tmp0;

        // average the two allpass outputs, scale down and store
        out[i << 1] = (out[i << 1] + (state[15] >> 1)) >> 15;
    }
}

//   lowpass filter
// input:  WebRtc_Word32 (shifted 15 positions to the left, + offset 16384)
// output: WebRtc_Word32 (normalized, not saturated)
// state:  filter state array; length = 8
void WebRtcSpl_LPBy2IntToInt(const WebRtc_Word32* in, WebRtc_Word32 len, WebRtc_Word32* out,
                             WebRtc_Word32* state)
{
    WebRtc_Word32 tmp0, tmp1, diff;
    WebRtc_Word32 i;

    len >>= 1;

    // lower allpass filter: odd input -> even output samples
    in++;
    // initial state of polyphase delay element
    tmp0 = state[12];
    for (i = 0; i < len; i++)
    {
        diff = tmp0 - state[1];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[0] + diff * kResampleAllpass[1][0];
        state[0] = tmp0;
        diff = tmp1 - state[2];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[1] + diff * kResampleAllpass[1][1];
        state[1] = tmp1;
        diff = tmp0 - state[3];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[3] = state[2] + diff * kResampleAllpass[1][2];
        state[2] = tmp0;

        // scale down, round and store
        out[i << 1] = state[3] >> 1;
        tmp0 = in[i << 1];
    }
    in--;

    // upper allpass filter: even input -> even output samples
    for (i = 0; i < len; i++)
    {
        tmp0 = in[i << 1];
        diff = tmp0 - state[5];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[4] + diff * kResampleAllpass[0][0];
        state[4] = tmp0;
        diff = tmp1 - state[6];
        // scale down and round
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[5] + diff * kResampleAllpass[0][1];
        state[5] = tmp1;
        diff = tmp0 - state[7];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[7] = state[6] + diff * kResampleAllpass[0][2];
        state[6] = tmp0;

        // average the two allpass outputs, scale down and store
        out[i << 1] = (out[i << 1] + (state[7] >> 1)) >> 15;
    }

    // switch to odd output samples
    out++;

    // lower allpass filter: even input -> odd output samples
    for (i = 0; i < len; i++)
    {
        tmp0 = in[i << 1];
        diff = tmp0 - state[9];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[8] + diff * kResampleAllpass[1][0];
        state[8] = tmp0;
        diff = tmp1 - state[10];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[9] + diff * kResampleAllpass[1][1];
        state[9] = tmp1;
        diff = tmp0 - state[11];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[11] = state[10] + diff * kResampleAllpass[1][2];
        state[10] = tmp0;

        // scale down, round and store
        out[i << 1] = state[11] >> 1;
    }

    // upper allpass filter: odd input -> odd output samples
    in++;
    for (i = 0; i < len; i++)
    {
        tmp0 = in[i << 1];
        diff = tmp0 - state[13];
        // scale down and round
        diff = (diff + (1 << 13)) >> 14;
        tmp1 = state[12] + diff * kResampleAllpass[0][0];
        state[12] = tmp0;
        diff = tmp1 - state[14];
        // scale down and round
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        tmp0 = state[13] + diff * kResampleAllpass[0][1];
        state[13] = tmp1;
        diff = tmp0 - state[15];
        // scale down and truncate
        diff = diff >> 14;
        if (diff < 0)
            diff += 1;
        state[15] = state[14] + diff * kResampleAllpass[0][2];
        state[14] = tmp0;

        // average the two allpass outputs, scale down and store
        out[i << 1] = (out[i << 1] + (state[15] >> 1)) >> 15;
    }
}

// interpolation coefficients
static const WebRtc_Word16 kCoefficients48To32[2][8] = {
        {778, -2050, 1087, 23285, 12903, -3783, 441, 222},
        {222, 441, -3783, 12903, 23285, 1087, -2050, 778}
};

static const WebRtc_Word16 kCoefficients32To24[3][8] = {
        {767, -2362, 2434, 24406, 10620, -3838, 721, 90},
        {386, -381, -2646, 19062, 19062, -2646, -381, 386},
        {90, 721, -3838, 10620, 24406, 2434, -2362, 767}
};

static const WebRtc_Word16 kCoefficients44To32[4][9] = {
        {117, -669, 2245, -6183, 26267, 13529, -3245, 845, -138},
        {-101, 612, -2283, 8532, 29790, -5138, 1789, -524, 91},
        {50, -292, 1016, -3064, 32010, 3933, -1147, 315, -53},
        {-156, 974, -3863, 18603, 21691, -6246, 2353, -712, 126}
};

//   Resampling ratio: 2/3
// input:  WebRtc_Word32 (normalized, not saturated) :: size 3 * K
// output: WebRtc_Word32 (shifted 15 positions to the left, + offset 16384) :: size 2 * K
//      K: number of blocks

void WebRtcSpl_Resample48khzTo32khz(const WebRtc_Word32 *In, WebRtc_Word32 *Out,
                                    const WebRtc_Word32 K)
{
    /////////////////////////////////////////////////////////////
    // Filter operation:
    //
    // Perform resampling (3 input samples -> 2 output samples);
    // process in sub blocks of size 3 samples.
    WebRtc_Word32 tmp;
    WebRtc_Word32 m;

    for (m = 0; m < K; m++)
    {
        tmp = 1 << 14;
        tmp += kCoefficients48To32[0][0] * In[0];
        tmp += kCoefficients48To32[0][1] * In[1];
        tmp += kCoefficients48To32[0][2] * In[2];
        tmp += kCoefficients48To32[0][3] * In[3];
        tmp += kCoefficients48To32[0][4] * In[4];
        tmp += kCoefficients48To32[0][5] * In[5];
        tmp += kCoefficients48To32[0][6] * In[6];
        tmp += kCoefficients48To32[0][7] * In[7];
        Out[0] = tmp;

        tmp = 1 << 14;
        tmp += kCoefficients48To32[1][0] * In[1];
        tmp += kCoefficients48To32[1][1] * In[2];
        tmp += kCoefficients48To32[1][2] * In[3];
        tmp += kCoefficients48To32[1][3] * In[4];
        tmp += kCoefficients48To32[1][4] * In[5];
        tmp += kCoefficients48To32[1][5] * In[6];
        tmp += kCoefficients48To32[1][6] * In[7];
        tmp += kCoefficients48To32[1][7] * In[8];
        Out[1] = tmp;

        // update pointers
        In += 3;
        Out += 2;
    }
}

//   Resampling ratio: 3/4
// input:  WebRtc_Word32 (normalized, not saturated) :: size 4 * K
// output: WebRtc_Word32 (shifted 15 positions to the left, + offset 16384) :: size 3 * K
//      K: number of blocks

void WebRtcSpl_Resample32khzTo24khz(const WebRtc_Word32 *In, WebRtc_Word32 *Out,
                                    const WebRtc_Word32 K)
{
    /////////////////////////////////////////////////////////////
    // Filter operation:
    //
    // Perform resampling (4 input samples -> 3 output samples);
    // process in sub blocks of size 4 samples.
    WebRtc_Word32 m;
    WebRtc_Word32 tmp;

    for (m = 0; m < K; m++)
    {
        tmp = 1 << 14;
        tmp += kCoefficients32To24[0][0] * In[0];
        tmp += kCoefficients32To24[0][1] * In[1];
        tmp += kCoefficients32To24[0][2] * In[2];
        tmp += kCoefficients32To24[0][3] * In[3];
        tmp += kCoefficients32To24[0][4] * In[4];
        tmp += kCoefficients32To24[0][5] * In[5];
        tmp += kCoefficients32To24[0][6] * In[6];
        tmp += kCoefficients32To24[0][7] * In[7];
        Out[0] = tmp;

        tmp = 1 << 14;
        tmp += kCoefficients32To24[1][0] * In[1];
        tmp += kCoefficients32To24[1][1] * In[2];
        tmp += kCoefficients32To24[1][2] * In[3];
        tmp += kCoefficients32To24[1][3] * In[4];
        tmp += kCoefficients32To24[1][4] * In[5];
        tmp += kCoefficients32To24[1][5] * In[6];
        tmp += kCoefficients32To24[1][6] * In[7];
        tmp += kCoefficients32To24[1][7] * In[8];
        Out[1] = tmp;

        tmp = 1 << 14;
        tmp += kCoefficients32To24[2][0] * In[2];
        tmp += kCoefficients32To24[2][1] * In[3];
        tmp += kCoefficients32To24[2][2] * In[4];
        tmp += kCoefficients32To24[2][3] * In[5];
        tmp += kCoefficients32To24[2][4] * In[6];
        tmp += kCoefficients32To24[2][5] * In[7];
        tmp += kCoefficients32To24[2][6] * In[8];
        tmp += kCoefficients32To24[2][7] * In[9];
        Out[2] = tmp;

        // update pointers
        In += 4;
        Out += 3;
    }
}

//
// fractional resampling filters
//   Fout = 11/16 * Fin
//   Fout =  8/11 * Fin
//

// compute two inner-products and store them to output array
static void WebRtcSpl_ResampDotProduct(const WebRtc_Word32 *in1, const WebRtc_Word32 *in2,
                               const WebRtc_Word16 *coef_ptr, WebRtc_Word32 *out1,
                               WebRtc_Word32 *out2)
{
    WebRtc_Word32 tmp1 = 16384;
    WebRtc_Word32 tmp2 = 16384;
    WebRtc_Word16 coef;

    coef = coef_ptr[0];
    tmp1 += coef * in1[0];
    tmp2 += coef * in2[-0];

    coef = coef_ptr[1];
    tmp1 += coef * in1[1];
    tmp2 += coef * in2[-1];

    coef = coef_ptr[2];
    tmp1 += coef * in1[2];
    tmp2 += coef * in2[-2];

    coef = coef_ptr[3];
    tmp1 += coef * in1[3];
    tmp2 += coef * in2[-3];

    coef = coef_ptr[4];
    tmp1 += coef * in1[4];
    tmp2 += coef * in2[-4];

    coef = coef_ptr[5];
    tmp1 += coef * in1[5];
    tmp2 += coef * in2[-5];

    coef = coef_ptr[6];
    tmp1 += coef * in1[6];
    tmp2 += coef * in2[-6];

    coef = coef_ptr[7];
    tmp1 += coef * in1[7];
    tmp2 += coef * in2[-7];

    coef = coef_ptr[8];
    *out1 = tmp1 + coef * in1[8];
    *out2 = tmp2 + coef * in2[-8];
}

//   Resampling ratio: 8/11
// input:  WebRtc_Word32 (normalized, not saturated) :: size 11 * K
// output: WebRtc_Word32 (shifted 15 positions to the left, + offset 16384) :: size  8 * K
//      K: number of blocks

void WebRtcSpl_Resample44khzTo32khz(const WebRtc_Word32 *In, WebRtc_Word32 *Out,
                                    const WebRtc_Word32 K)
{
    /////////////////////////////////////////////////////////////
    // Filter operation:
    //
    // Perform resampling (11 input samples -> 8 output samples);
    // process in sub blocks of size 11 samples.
    WebRtc_Word32 tmp;
    WebRtc_Word32 m;

    for (m = 0; m < K; m++)
    {
        tmp = 1 << 14;

        // first output sample
        Out[0] = ((WebRtc_Word32)In[3] << 15) + tmp;

        // sum and accumulate filter coefficients and input samples
        tmp += kCoefficients44To32[3][0] * In[5];
        tmp += kCoefficients44To32[3][1] * In[6];
        tmp += kCoefficients44To32[3][2] * In[7];
        tmp += kCoefficients44To32[3][3] * In[8];
        tmp += kCoefficients44To32[3][4] * In[9];
        tmp += kCoefficients44To32[3][5] * In[10];
        tmp += kCoefficients44To32[3][6] * In[11];
        tmp += kCoefficients44To32[3][7] * In[12];
        tmp += kCoefficients44To32[3][8] * In[13];
        Out[4] = tmp;

        // sum and accumulate filter coefficients and input samples
        WebRtcSpl_ResampDotProduct(&In[0], &In[17], kCoefficients44To32[0], &Out[1], &Out[7]);

        // sum and accumulate filter coefficients and input samples
        WebRtcSpl_ResampDotProduct(&In[2], &In[15], kCoefficients44To32[1], &Out[2], &Out[6]);

        // sum and accumulate filter coefficients and input samples
        WebRtcSpl_ResampDotProduct(&In[3], &In[14], kCoefficients44To32[2], &Out[3], &Out[5]);

        // update pointers
        In += 11;
        Out += 8;
    }
}


/* Declare function pointers. */
MaxAbsValueW16 WebRtcSpl_MaxAbsValueW16;
MaxAbsValueW32 WebRtcSpl_MaxAbsValueW32;
MaxValueW16 WebRtcSpl_MaxValueW16;
MaxValueW32 WebRtcSpl_MaxValueW32;
MinValueW16 WebRtcSpl_MinValueW16;
MinValueW32 WebRtcSpl_MinValueW32;
CrossCorrelation WebRtcSpl_CrossCorrelation;
DownsampleFast WebRtcSpl_DownsampleFast;
ScaleAndAddVectorsWithRound WebRtcSpl_ScaleAndAddVectorsWithRound;
RealForwardFFT WebRtcSpl_RealForwardFFT;
RealInverseFFT WebRtcSpl_RealInverseFFT;

#if defined(WEBRTC_DETECT_ARM_NEON) || !defined(WEBRTC_ARCH_ARM_NEON)
/* Initialize function pointers to the generic C version. */
static void InitPointersToC() {
  WebRtcSpl_MaxAbsValueW16 = WebRtcSpl_MaxAbsValueW16C;
  WebRtcSpl_MaxAbsValueW32 = WebRtcSpl_MaxAbsValueW32C;
  WebRtcSpl_MaxValueW16 = WebRtcSpl_MaxValueW16C;
  WebRtcSpl_MaxValueW32 = WebRtcSpl_MaxValueW32C;
  WebRtcSpl_MinValueW16 = WebRtcSpl_MinValueW16C;
  WebRtcSpl_MinValueW32 = WebRtcSpl_MinValueW32C;
  WebRtcSpl_CrossCorrelation = WebRtcSpl_CrossCorrelationC;
  WebRtcSpl_DownsampleFast = WebRtcSpl_DownsampleFastC;
  WebRtcSpl_ScaleAndAddVectorsWithRound =
      WebRtcSpl_ScaleAndAddVectorsWithRoundC;
  WebRtcSpl_RealForwardFFT = WebRtcSpl_RealForwardFFTC;
  WebRtcSpl_RealInverseFFT = WebRtcSpl_RealInverseFFTC;
}
#endif

#if defined(WEBRTC_DETECT_ARM_NEON) || defined(WEBRTC_ARCH_ARM_NEON)
/* Initialize function pointers to the Neon version. */
static void InitPointersToNeon() {
  WebRtcSpl_MaxAbsValueW16 = WebRtcSpl_MaxAbsValueW16Neon;
  WebRtcSpl_MaxAbsValueW32 = WebRtcSpl_MaxAbsValueW32Neon;
  WebRtcSpl_MaxValueW16 = WebRtcSpl_MaxValueW16Neon;
  WebRtcSpl_MaxValueW32 = WebRtcSpl_MaxValueW32Neon;
  WebRtcSpl_MinValueW16 = WebRtcSpl_MinValueW16Neon;
  WebRtcSpl_MinValueW32 = WebRtcSpl_MinValueW32Neon;
  WebRtcSpl_CrossCorrelation = WebRtcSpl_CrossCorrelationNeon;
  WebRtcSpl_DownsampleFast = WebRtcSpl_DownsampleFastNeon;
  WebRtcSpl_ScaleAndAddVectorsWithRound =
      WebRtcSpl_ScaleAndAddVectorsWithRoundNeon;
  WebRtcSpl_RealForwardFFT = WebRtcSpl_RealForwardFFTNeon;
  WebRtcSpl_RealInverseFFT = WebRtcSpl_RealInverseFFTNeon;
}
#endif

static void InitFunctionPointers(void) {
#if defined(WEBRTC_DETECT_ARM_NEON)
  if ((WebRtc_GetCPUFeaturesARM() & kCPUFeatureNEON) != 0) {
    InitPointersToNeon();
  } else {
    InitPointersToC();
  }
#elif defined(WEBRTC_ARCH_ARM_NEON)
  InitPointersToNeon();
#else
  InitPointersToC();
#endif  /* WEBRTC_DETECT_ARM_NEON */
}

#if defined(WEBRTC_POSIX)
#include <pthread.h>

static void once(void (*func)(void)) {
  static pthread_once_t lock = PTHREAD_ONCE_INIT;
  pthread_once(&lock, func);
}

#elif defined(_WIN32)
#include <windows.h>

static void once(void (*func)(void)) {
  /* Didn't use InitializeCriticalSection() since there's no race-free context
   * in which to execute it.
   *
   * TODO(kma): Change to different implementation (e.g.
   * InterlockedCompareExchangePointer) to avoid issues similar to
   * http://code.google.com/p/webm/issues/detail?id=467.
   */
  static CRITICAL_SECTION lock = {(void *)-1, -1, 0, 0, 0, 0};
  static int done = 0;

  EnterCriticalSection(&lock);
  if (!done) {
    func();
    done = 1;
  }
  LeaveCriticalSection(&lock);
}

/* There's no fallback version as an #else block here to ensure thread safety.
 * In case of neither pthread for WEBRTC_POSIX nor _WIN32 is present, build
 * system should pick it up.
 */
#endif  /* WEBRTC_POSIX */

void WebRtcSpl_Init() {
  once(InitFunctionPointers);
}


// Spectrum Weighting
static const int16_t kSpectrumWeight[kNumChannels] = { 6, 8, 10, 12, 14, 16 };
static const int16_t kNoiseUpdateConst = 655; // Q15
static const int16_t kSpeechUpdateConst = 6554; // Q15
static const int16_t kBackEta = 154; // Q8
// Minimum difference between the two models, Q5
static const int16_t kMinimumDifference[kNumChannels] = {
    544, 544, 576, 576, 576, 576 };
// Upper limit of mean value for speech model, Q7
static const int16_t kMaximumSpeech[kNumChannels] = {
    11392, 11392, 11520, 11520, 11520, 11520 };
// Minimum value for mean value
static const int16_t kMinimumMean[kNumGaussians] = { 640, 768 };
// Upper limit of mean value for noise model, Q7
static const int16_t kMaximumNoise[kNumChannels] = {
    9216, 9088, 8960, 8832, 8704, 8576 };
// Start values for the Gaussian models, Q7
// Weights for the two Gaussians for the six channels (noise)
static const int16_t kNoiseDataWeights[kTableSize] = {
    34, 62, 72, 66, 53, 25, 94, 66, 56, 62, 75, 103 };
// Weights for the two Gaussians for the six channels (speech)
static const int16_t kSpeechDataWeights[kTableSize] = {
    48, 82, 45, 87, 50, 47, 80, 46, 83, 41, 78, 81 };
// Means for the two Gaussians for the six channels (noise)
static const int16_t kNoiseDataMeans[kTableSize] = {
    6738, 4892, 7065, 6715, 6771, 3369, 7646, 3863, 7820, 7266, 5020, 4362 };
// Means for the two Gaussians for the six channels (speech)
static const int16_t kSpeechDataMeans[kTableSize] = {
    8306, 10085, 10078, 11823, 11843, 6309, 9473, 9571, 10879, 7581, 8180, 7483
};
// Stds for the two Gaussians for the six channels (noise)
static const int16_t kNoiseDataStds[kTableSize] = {
    378, 1064, 493, 582, 688, 593, 474, 697, 475, 688, 421, 455 };
// Stds for the two Gaussians for the six channels (speech)
static const int16_t kSpeechDataStds[kTableSize] = {
    555, 505, 567, 524, 585, 1231, 509, 828, 492, 1540, 1079, 850 };

// Constants used in GmmProbability().
//
// Maximum number of counted speech (VAD = 1) frames in a row.
static const int16_t kMaxSpeechFrames = 6;
// Minimum standard deviation for both speech and noise.
static const int16_t kMinStd = 384;

// Constants in WebRtcVad_InitCore().
// Default aggressiveness mode.
static const short kDefaultMode = 0;

// Constants used in WebRtcVad_set_mode_core().
//
// Thresholds for different frame lengths (10 ms, 20 ms and 30 ms).
//
// Mode 0, Quality.
static const int16_t kOverHangMax1Q[3] = { 8, 4, 3 };
static const int16_t kOverHangMax2Q[3] = { 14, 7, 5 };
static const int16_t kLocalThresholdQ[3] = { 24, 21, 24 };
static const int16_t kGlobalThresholdQ[3] = { 57, 48, 57 };
// Mode 1, Low bitrate.
static const int16_t kOverHangMax1LBR[3] = { 8, 4, 3 };
static const int16_t kOverHangMax2LBR[3] = { 14, 7, 5 };
static const int16_t kLocalThresholdLBR[3] = { 37, 32, 37 };
static const int16_t kGlobalThresholdLBR[3] = { 100, 80, 100 };
// Mode 2, Aggressive.
static const int16_t kOverHangMax1AGG[3] = { 6, 3, 2 };
static const int16_t kOverHangMax2AGG[3] = { 9, 5, 3 };
static const int16_t kLocalThresholdAGG[3] = { 82, 78, 82 };
static const int16_t kGlobalThresholdAGG[3] = { 285, 260, 285 };
// Mode 3, Very aggressive.
static const int16_t kOverHangMax1VAG[3] = { 6, 3, 2 };
static const int16_t kOverHangMax2VAG[3] = { 9, 5, 3 };
static const int16_t kLocalThresholdVAG[3] = { 94, 94, 94 };
static const int16_t kGlobalThresholdVAG[3] = { 1100, 1050, 1100 };

// Calculates the weighted average w.r.t. number of Gaussians. The |data| are
// updated with an |offset| before averaging.
//
// - data     [i/o] : Data to average.
// - offset   [i]   : An offset added to |data|.
// - weights  [i]   : Weights used for averaging.
//
// returns          : The weighted average.
static int32_t WeightedAverage(int16_t* data, int16_t offset,
                               const int16_t* weights) {
  int k;
  int32_t weighted_average = 0;

  for (k = 0; k < kNumGaussians; k++) {
    data[k * kNumChannels] += offset;
    weighted_average += data[k * kNumChannels] * weights[k * kNumChannels];
  }
  return weighted_average;
}

// Calculates the probabilities for both speech and background noise using
// Gaussian Mixture Models (GMM). A hypothesis-test is performed to decide which
// type of signal is most probable.
//
// - self           [i/o] : Pointer to VAD instance
// - features       [i]   : Feature vector of length |kNumChannels|
//                          = log10(energy in frequency band)
// - total_power    [i]   : Total power in audio frame.
// - frame_length   [i]   : Number of input samples
//
// - returns              : the VAD decision (0 - noise, 1 - speech).
static int16_t GmmProbability(VadInstT* self, int16_t* features,
                              int16_t total_power, int frame_length) {
  int channel, k;
  int16_t feature_minimum;
  int16_t h0, h1;
  int16_t log_likelihood_ratio;
  int16_t vadflag = 0;
  int16_t shifts_h0, shifts_h1;
  int16_t tmp_s16, tmp1_s16, tmp2_s16;
  int16_t diff;
  int gaussian;
  int16_t nmk, nmk2, nmk3, smk, smk2, nsk, ssk;
  int16_t delt, ndelt;
  int16_t maxspe, maxmu;
  int16_t deltaN[kTableSize], deltaS[kTableSize];
  int16_t ngprvec[kTableSize] = { 0 };  // Conditional probability = 0.
  int16_t sgprvec[kTableSize] = { 0 };  // Conditional probability = 0.
  int32_t h0_test, h1_test;
  int32_t tmp1_s32, tmp2_s32;
  int32_t sum_log_likelihood_ratios = 0;
  int32_t noise_global_mean, speech_global_mean;
  int32_t noise_probability[kNumGaussians], speech_probability[kNumGaussians];
  int16_t overhead1, overhead2, individualTest, totalTest;

  // Set various thresholds based on frame lengths (80, 160 or 240 samples).
  if (frame_length == 80) {
    overhead1 = self->over_hang_max_1[0];
    overhead2 = self->over_hang_max_2[0];
    individualTest = self->individual[0];
    totalTest = self->total[0];
  } else if (frame_length == 160) {
    overhead1 = self->over_hang_max_1[1];
    overhead2 = self->over_hang_max_2[1];
    individualTest = self->individual[1];
    totalTest = self->total[1];
  } else {
    overhead1 = self->over_hang_max_1[2];
    overhead2 = self->over_hang_max_2[2];
    individualTest = self->individual[2];
    totalTest = self->total[2];
  }

  if (total_power > kMinEnergy) {
    // The signal power of current frame is large enough for processing. The
    // processing consists of two parts:
    // 1) Calculating the likelihood of speech and thereby a VAD decision.
    // 2) Updating the underlying model, w.r.t., the decision made.

    // The detection scheme is an LRT with hypothesis
    // H0: Noise
    // H1: Speech
    //
    // We combine a global LRT with local tests, for each frequency sub-band,
    // here defined as |channel|.
    for (channel = 0; channel < kNumChannels; channel++) {
      // For each channel we model the probability with a GMM consisting of
      // |kNumGaussians|, with different means and standard deviations depending
      // on H0 or H1.
      h0_test = 0;
      h1_test = 0;
      for (k = 0; k < kNumGaussians; k++) {
        gaussian = channel + k * kNumChannels;
        // Probability under H0, that is, probability of frame being noise.
        // Value given in Q27 = Q7 * Q20.
        tmp1_s32 = WebRtcVad_GaussianProbability(features[channel],
                                                 self->noise_means[gaussian],
                                                 self->noise_stds[gaussian],
                                                 &deltaN[gaussian]);
        noise_probability[k] = kNoiseDataWeights[gaussian] * tmp1_s32;
        h0_test += noise_probability[k];  // Q27

        // Probability under H1, that is, probability of frame being speech.
        // Value given in Q27 = Q7 * Q20.
        tmp1_s32 = WebRtcVad_GaussianProbability(features[channel],
                                                 self->speech_means[gaussian],
                                                 self->speech_stds[gaussian],
                                                 &deltaS[gaussian]);
        speech_probability[k] = kSpeechDataWeights[gaussian] * tmp1_s32;
        h1_test += speech_probability[k];  // Q27
      }

      // Calculate the log likelihood ratio: log2(Pr{X|H1} / Pr{X|H1}).
      // Approximation:
      // log2(Pr{X|H1} / Pr{X|H1}) = log2(Pr{X|H1}*2^Q) - log2(Pr{X|H1}*2^Q)
      //                           = log2(h1_test) - log2(h0_test)
      //                           = log2(2^(31-shifts_h1)*(1+b1))
      //                             - log2(2^(31-shifts_h0)*(1+b0))
      //                           = shifts_h0 - shifts_h1
      //                             + log2(1+b1) - log2(1+b0)
      //                          ~= shifts_h0 - shifts_h1
      //
      // Note that b0 and b1 are values less than 1, hence, 0 <= log2(1+b0) < 1.
      // Further, b0 and b1 are independent and on the average the two terms
      // cancel.
      shifts_h0 = WebRtcSpl_NormW32(h0_test);
      shifts_h1 = WebRtcSpl_NormW32(h1_test);
      if (h0_test == 0) {
        shifts_h0 = 31;
      }
      if (h1_test == 0) {
        shifts_h1 = 31;
      }
      log_likelihood_ratio = shifts_h0 - shifts_h1;

      // Update |sum_log_likelihood_ratios| with spectrum weighting. This is
      // used for the global VAD decision.
      sum_log_likelihood_ratios +=
          (int32_t) (log_likelihood_ratio * kSpectrumWeight[channel]);

      // Local VAD decision.
      if ((log_likelihood_ratio << 2) > individualTest) {
        vadflag = 1;
      }

      // TODO(bjornv): The conditional probabilities below are applied on the
      // hard coded number of Gaussians set to two. Find a way to generalize.
      // Calculate local noise probabilities used later when updating the GMM.
      h0 = (int16_t) (h0_test >> 12);  // Q15
      if (h0 > 0) {
        // High probability of noise. Assign conditional probabilities for each
        // Gaussian in the GMM.
        tmp1_s32 = (noise_probability[0] & 0xFFFFF000) << 2;  // Q29
        ngprvec[channel] = (int16_t) WebRtcSpl_DivW32W16(tmp1_s32, h0);  // Q14
        ngprvec[channel + kNumChannels] = 16384 - ngprvec[channel];
      } else {
        // Low noise probability. Assign conditional probability 1 to the first
        // Gaussian and 0 to the rest (which is already set at initialization).
        ngprvec[channel] = 16384;
      }

      // Calculate local speech probabilities used later when updating the GMM.
      h1 = (int16_t) (h1_test >> 12);  // Q15
      if (h1 > 0) {
        // High probability of speech. Assign conditional probabilities for each
        // Gaussian in the GMM. Otherwise use the initialized values, i.e., 0.
        tmp1_s32 = (speech_probability[0] & 0xFFFFF000) << 2;  // Q29
        sgprvec[channel] = (int16_t) WebRtcSpl_DivW32W16(tmp1_s32, h1);  // Q14
        sgprvec[channel + kNumChannels] = 16384 - sgprvec[channel];
      }
    }

    // Make a global VAD decision.
    vadflag |= (sum_log_likelihood_ratios >= totalTest);

    // Update the model parameters.
    maxspe = 12800;
    for (channel = 0; channel < kNumChannels; channel++) {

      // Get minimum value in past which is used for long term correction in Q4.
      feature_minimum = WebRtcVad_FindMinimum(self, features[channel], channel);

      // Compute the "global" mean, that is the sum of the two means weighted.
      noise_global_mean = WeightedAverage(&self->noise_means[channel], 0,
                                          &kNoiseDataWeights[channel]);
      tmp1_s16 = (int16_t) (noise_global_mean >> 6);  // Q8

      for (k = 0; k < kNumGaussians; k++) {
        gaussian = channel + k * kNumChannels;

        nmk = self->noise_means[gaussian];
        smk = self->speech_means[gaussian];
        nsk = self->noise_stds[gaussian];
        ssk = self->speech_stds[gaussian];

        // Update noise mean vector if the frame consists of noise only.
        nmk2 = nmk;
        if (!vadflag) {
          // deltaN = (x-mu)/sigma^2
          // ngprvec[k] = |noise_probability[k]| /
          //   (|noise_probability[0]| + |noise_probability[1]|)

          // (Q14 * Q11 >> 11) = Q14.
          delt = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(ngprvec[gaussian],
                                                     deltaN[gaussian],
                                                     11);
          // Q7 + (Q14 * Q15 >> 22) = Q7.
          nmk2 = nmk + (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(delt,
                                                           kNoiseUpdateConst,
                                                           22);
        }

        // Long term correction of the noise mean.
        // Q8 - Q8 = Q8.
        ndelt = (feature_minimum << 4) - tmp1_s16;
        // Q7 + (Q8 * Q8) >> 9 = Q7.
        nmk3 = nmk2 + (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(ndelt, kBackEta, 9);

        // Control that the noise mean does not drift to much.
        tmp_s16 = (int16_t) ((k + 5) << 7);
        if (nmk3 < tmp_s16) {
          nmk3 = tmp_s16;
        }
        tmp_s16 = (int16_t) ((72 + k - channel) << 7);
        if (nmk3 > tmp_s16) {
          nmk3 = tmp_s16;
        }
        self->noise_means[gaussian] = nmk3;

        if (vadflag) {
          // Update speech mean vector:
          // |deltaS| = (x-mu)/sigma^2
          // sgprvec[k] = |speech_probability[k]| /
          //   (|speech_probability[0]| + |speech_probability[1]|)

          // (Q14 * Q11) >> 11 = Q14.
          delt = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(sgprvec[gaussian],
                                                     deltaS[gaussian],
                                                     11);
          // Q14 * Q15 >> 21 = Q8.
          tmp_s16 = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(delt,
                                                        kSpeechUpdateConst,
                                                        21);
          // Q7 + (Q8 >> 1) = Q7. With rounding.
          smk2 = smk + ((tmp_s16 + 1) >> 1);

          // Control that the speech mean does not drift to much.
          maxmu = maxspe + 640;
          if (smk2 < kMinimumMean[k]) {
            smk2 = kMinimumMean[k];
          }
          if (smk2 > maxmu) {
            smk2 = maxmu;
          }
          self->speech_means[gaussian] = smk2;  // Q7.

          // (Q7 >> 3) = Q4. With rounding.
          tmp_s16 = ((smk + 4) >> 3);

          tmp_s16 = features[channel] - tmp_s16;  // Q4
          // (Q11 * Q4 >> 3) = Q12.
          tmp1_s32 = WEBRTC_SPL_MUL_16_16_RSFT(deltaS[gaussian], tmp_s16, 3);
          tmp2_s32 = tmp1_s32 - 4096;
          tmp_s16 = sgprvec[gaussian] >> 2;
          // (Q14 >> 2) * Q12 = Q24.
          tmp1_s32 = tmp_s16 * tmp2_s32;

          tmp2_s32 = tmp1_s32 >> 4;  // Q20

          // 0.1 * Q20 / Q7 = Q13.
          if (tmp2_s32 > 0) {
            tmp_s16 = (int16_t) WebRtcSpl_DivW32W16(tmp2_s32, ssk * 10);
          } else {
            tmp_s16 = (int16_t) WebRtcSpl_DivW32W16(-tmp2_s32, ssk * 10);
            tmp_s16 = -tmp_s16;
          }
          // Divide by 4 giving an update factor of 0.025 (= 0.1 / 4).
          // Note that division by 4 equals shift by 2, hence,
          // (Q13 >> 8) = (Q13 >> 6) / 4 = Q7.
          tmp_s16 += 128;  // Rounding.
          ssk += (tmp_s16 >> 8);
          if (ssk < kMinStd) {
            ssk = kMinStd;
          }
          self->speech_stds[gaussian] = ssk;
        } else {
          // Update GMM variance vectors.
          // deltaN * (features[channel] - nmk) - 1
          // Q4 - (Q7 >> 3) = Q4.
          tmp_s16 = features[channel] - (nmk >> 3);
          // (Q11 * Q4 >> 3) = Q12.
          tmp1_s32 = WEBRTC_SPL_MUL_16_16_RSFT(deltaN[gaussian], tmp_s16, 3);
          tmp1_s32 -= 4096;

          // (Q14 >> 2) * Q12 = Q24.
          tmp_s16 = (ngprvec[gaussian] + 2) >> 2;
          tmp2_s32 = tmp_s16 * tmp1_s32;
          // Q20  * approx 0.001 (2^-10=0.0009766), hence,
          // (Q24 >> 14) = (Q24 >> 4) / 2^10 = Q20.
          tmp1_s32 = tmp2_s32 >> 14;

          // Q20 / Q7 = Q13.
          if (tmp1_s32 > 0) {
            tmp_s16 = (int16_t) WebRtcSpl_DivW32W16(tmp1_s32, nsk);
          } else {
            tmp_s16 = (int16_t) WebRtcSpl_DivW32W16(-tmp1_s32, nsk);
            tmp_s16 = -tmp_s16;
          }
          tmp_s16 += 32;  // Rounding
          nsk += tmp_s16 >> 6;  // Q13 >> 6 = Q7.
          if (nsk < kMinStd) {
            nsk = kMinStd;
          }
          self->noise_stds[gaussian] = nsk;
        }
      }

      // Separate models if they are too close.
      // |noise_global_mean| in Q14 (= Q7 * Q7).
      noise_global_mean = WeightedAverage(&self->noise_means[channel], 0,
                                          &kNoiseDataWeights[channel]);

      // |speech_global_mean| in Q14 (= Q7 * Q7).
      speech_global_mean = WeightedAverage(&self->speech_means[channel], 0,
                                           &kSpeechDataWeights[channel]);

      // |diff| = "global" speech mean - "global" noise mean.
      // (Q14 >> 9) - (Q14 >> 9) = Q5.
      diff = (int16_t) (speech_global_mean >> 9) -
          (int16_t) (noise_global_mean >> 9);
      if (diff < kMinimumDifference[channel]) {
        tmp_s16 = kMinimumDifference[channel] - diff;

        // |tmp1_s16| = ~0.8 * (kMinimumDifference - diff) in Q7.
        // |tmp2_s16| = ~0.2 * (kMinimumDifference - diff) in Q7.
        tmp1_s16 = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(13, tmp_s16, 2);
        tmp2_s16 = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(3, tmp_s16, 2);

        // Move Gaussian means for speech model by |tmp1_s16| and update
        // |speech_global_mean|. Note that |self->speech_means[channel]| is
        // changed after the call.
        speech_global_mean = WeightedAverage(&self->speech_means[channel],
                                             tmp1_s16,
                                             &kSpeechDataWeights[channel]);

        // Move Gaussian means for noise model by -|tmp2_s16| and update
        // |noise_global_mean|. Note that |self->noise_means[channel]| is
        // changed after the call.
        noise_global_mean = WeightedAverage(&self->noise_means[channel],
                                            -tmp2_s16,
                                            &kNoiseDataWeights[channel]);
      }

      // Control that the speech & noise means do not drift to much.
      maxspe = kMaximumSpeech[channel];
      tmp2_s16 = (int16_t) (speech_global_mean >> 7);
      if (tmp2_s16 > maxspe) {
        // Upper limit of speech model.
        tmp2_s16 -= maxspe;

        for (k = 0; k < kNumGaussians; k++) {
          self->speech_means[channel + k * kNumChannels] -= tmp2_s16;
        }
      }

      tmp2_s16 = (int16_t) (noise_global_mean >> 7);
      if (tmp2_s16 > kMaximumNoise[channel]) {
        tmp2_s16 -= kMaximumNoise[channel];

        for (k = 0; k < kNumGaussians; k++) {
          self->noise_means[channel + k * kNumChannels] -= tmp2_s16;
        }
      }
    }
    self->frame_counter++;
  }

  // Smooth with respect to transition hysteresis.
  if (!vadflag) {
    if (self->over_hang > 0) {
      vadflag = 2 + self->over_hang;
      self->over_hang--;
    }
    self->num_of_speech = 0;
  } else {
    self->num_of_speech++;
    if (self->num_of_speech > kMaxSpeechFrames) {
      self->num_of_speech = kMaxSpeechFrames;
      self->over_hang = overhead2;
    } else {
      self->over_hang = overhead1;
    }
  }
  return vadflag;
}

// Initialize the VAD. Set aggressiveness mode to default value.
int WebRtcVad_InitCore(VadInstT* self) {
  int i;

  if (self == NULL) {
    return -1;
  }

  // Initialization of general struct variables.
  self->vad = 1;  // Speech active (=1).
  self->frame_counter = 0;
  self->over_hang = 0;
  self->num_of_speech = 0;

  // Initialization of downsampling filter state.
  memset(self->downsampling_filter_states, 0,
         sizeof(self->downsampling_filter_states));

  // Initialization of 48 to 8 kHz downsampling.
  WebRtcSpl_ResetResample48khzTo8khz(&self->state_48_to_8);

  // Read initial PDF parameters.
  for (i = 0; i < kTableSize; i++) {
    self->noise_means[i] = kNoiseDataMeans[i];
    self->speech_means[i] = kSpeechDataMeans[i];
    self->noise_stds[i] = kNoiseDataStds[i];
    self->speech_stds[i] = kSpeechDataStds[i];
  }

  // Initialize Index and Minimum value vectors.
  for (i = 0; i < 16 * kNumChannels; i++) {
    self->low_value_vector[i] = 10000;
    self->index_vector[i] = 0;
  }

  // Initialize splitting filter states.
  memset(self->upper_state, 0, sizeof(self->upper_state));
  memset(self->lower_state, 0, sizeof(self->lower_state));

  // Initialize high pass filter states.
  memset(self->hp_filter_state, 0, sizeof(self->hp_filter_state));

  // Initialize mean value memory, for WebRtcVad_FindMinimum().
  for (i = 0; i < kNumChannels; i++) {
    self->mean_value[i] = 1600;
  }

  // Set aggressiveness mode to default (=|kDefaultMode|).
  if (WebRtcVad_set_mode_core(self, kDefaultMode) != 0) {
    return -1;
  }

  self->init_flag = kInitCheck;

  return 0;
}

// Set aggressiveness mode
int WebRtcVad_set_mode_core(VadInstT* self, int mode) {
  int return_value = 0;

  switch (mode) {
    case 0:
      // Quality mode.
      memcpy(self->over_hang_max_1, kOverHangMax1Q,
             sizeof(self->over_hang_max_1));
      memcpy(self->over_hang_max_2, kOverHangMax2Q,
             sizeof(self->over_hang_max_2));
      memcpy(self->individual, kLocalThresholdQ,
             sizeof(self->individual));
      memcpy(self->total, kGlobalThresholdQ,
             sizeof(self->total));
      break;
    case 1:
      // Low bitrate mode.
      memcpy(self->over_hang_max_1, kOverHangMax1LBR,
             sizeof(self->over_hang_max_1));
      memcpy(self->over_hang_max_2, kOverHangMax2LBR,
             sizeof(self->over_hang_max_2));
      memcpy(self->individual, kLocalThresholdLBR,
             sizeof(self->individual));
      memcpy(self->total, kGlobalThresholdLBR,
             sizeof(self->total));
      break;
    case 2:
      // Aggressive mode.
      memcpy(self->over_hang_max_1, kOverHangMax1AGG,
             sizeof(self->over_hang_max_1));
      memcpy(self->over_hang_max_2, kOverHangMax2AGG,
             sizeof(self->over_hang_max_2));
      memcpy(self->individual, kLocalThresholdAGG,
             sizeof(self->individual));
      memcpy(self->total, kGlobalThresholdAGG,
             sizeof(self->total));
      break;
    case 3:
      // Very aggressive mode.
      memcpy(self->over_hang_max_1, kOverHangMax1VAG,
             sizeof(self->over_hang_max_1));
      memcpy(self->over_hang_max_2, kOverHangMax2VAG,
             sizeof(self->over_hang_max_2));
      memcpy(self->individual, kLocalThresholdVAG,
             sizeof(self->individual));
      memcpy(self->total, kGlobalThresholdVAG,
             sizeof(self->total));
      break;
    default:
      return_value = -1;
      break;
  }

  return return_value;
}

// Calculate VAD decision by first extracting feature values and then calculate
// probability for both speech and background noise.

int WebRtcVad_CalcVad48khz(VadInstT* inst, int16_t* speech_frame,
                           int frame_length) {
  int vad;
  int i;
  int16_t speech_nb[240];  // 30 ms in 8 kHz.
  // |tmp_mem| is a temporary memory used by resample function, length is
  // frame length in 10 ms (480 samples) + 256 extra.
  int32_t tmp_mem[480 + 256] = { 0 };
  const int kFrameLen10ms48khz = 480;
  const int kFrameLen10ms8khz = 80;
  int num_10ms_frames = frame_length / kFrameLen10ms48khz;

  for (i = 0; i < num_10ms_frames; i++) {
    WebRtcSpl_Resample48khzTo8khz(speech_frame,
                                  &speech_nb[i * kFrameLen10ms8khz],
                                  &inst->state_48_to_8,
                                  tmp_mem);
  }

  // Do VAD on an 8 kHz signal
  vad = WebRtcVad_CalcVad8khz(inst, speech_nb, frame_length / 6);

  return vad;
}

int WebRtcVad_CalcVad32khz(VadInstT* inst, int16_t* speech_frame,
                           int frame_length)
{
    int len, vad;
    int16_t speechWB[480]; // Downsampled speech frame: 960 samples (30ms in SWB)
    int16_t speechNB[240]; // Downsampled speech frame: 480 samples (30ms in WB)


    // Downsample signal 32->16->8 before doing VAD
    WebRtcVad_Downsampling(speech_frame, speechWB, &(inst->downsampling_filter_states[2]),
                           frame_length);
    len = WEBRTC_SPL_RSHIFT_W16(frame_length, 1);

    WebRtcVad_Downsampling(speechWB, speechNB, inst->downsampling_filter_states, len);
    len = WEBRTC_SPL_RSHIFT_W16(len, 1);

    // Do VAD on an 8 kHz signal
    vad = WebRtcVad_CalcVad8khz(inst, speechNB, len);

    return vad;
}

int WebRtcVad_CalcVad16khz(VadInstT* inst, int16_t* speech_frame,
                           int frame_length)
{
    int len, vad;
    int16_t speechNB[240]; // Downsampled speech frame: 480 samples (30ms in WB)

    // Wideband: Downsample signal before doing VAD
    WebRtcVad_Downsampling(speech_frame, speechNB, inst->downsampling_filter_states,
                           frame_length);

    len = WEBRTC_SPL_RSHIFT_W16(frame_length, 1);
    vad = WebRtcVad_CalcVad8khz(inst, speechNB, len);

    return vad;
}

int WebRtcVad_CalcVad8khz(VadInstT* inst, int16_t* speech_frame,
                          int frame_length)
{
    int16_t feature_vector[kNumChannels], total_power;

    // Get power in the bands
    total_power = WebRtcVad_CalculateFeatures(inst, speech_frame, frame_length,
                                              feature_vector);

    // Make a VAD
    inst->vad = GmmProbability(inst, feature_vector, total_power, frame_length);

    return inst->vad;
}


#include <assert.h>

// Constants used in LogOfEnergy().
static const int16_t kLogConst = 24660;  // 160*log10(2) in Q9.
static const int16_t kLogEnergyIntPart = 14336;  // 14 in Q10

// Coefficients used by HighPassFilter, Q14.
static const int16_t kHpZeroCoefs[3] = { 6631, -13262, 6631 };
static const int16_t kHpPoleCoefs[3] = { 16384, -7756, 5620 };

// Allpass filter coefficients, upper and lower, in Q15.
// Upper: 0.64, Lower: 0.17
static const int16_t kAllPassCoefsQ15[2] = { 20972, 5571 };

// Adjustment for division with two in SplitFilter.
static const int16_t kOffsetVector[6] = { 368, 368, 272, 176, 176, 176 };

// High pass filtering, with a cut-off frequency at 80 Hz, if the |data_in| is
// sampled at 500 Hz.
//
// - data_in      [i]   : Input audio data sampled at 500 Hz.
// - data_length  [i]   : Length of input and output data.
// - filter_state [i/o] : State of the filter.
// - data_out     [o]   : Output audio data in the frequency interval
//                        80 - 250 Hz.
static void HighPassFilter(const int16_t* data_in, int data_length,
                           int16_t* filter_state, int16_t* data_out) {
  int i;
  const int16_t* in_ptr = data_in;
  int16_t* out_ptr = data_out;
  int32_t tmp32 = 0;


  // The sum of the absolute values of the impulse response:
  // The zero/pole-filter has a max amplification of a single sample of: 1.4546
  // Impulse response: 0.4047 -0.6179 -0.0266  0.1993  0.1035  -0.0194
  // The all-zero section has a max amplification of a single sample of: 1.6189
  // Impulse response: 0.4047 -0.8094  0.4047  0       0        0
  // The all-pole section has a max amplification of a single sample of: 1.9931
  // Impulse response: 1.0000  0.4734 -0.1189 -0.2187 -0.0627   0.04532

  for (i = 0; i < data_length; i++) {
    // All-zero section (filter coefficients in Q14).
    tmp32 = WEBRTC_SPL_MUL_16_16(kHpZeroCoefs[0], *in_ptr);
    tmp32 += WEBRTC_SPL_MUL_16_16(kHpZeroCoefs[1], filter_state[0]);
    tmp32 += WEBRTC_SPL_MUL_16_16(kHpZeroCoefs[2], filter_state[1]);
    filter_state[1] = filter_state[0];
    filter_state[0] = *in_ptr++;

    // All-pole section (filter coefficients in Q14).
    tmp32 -= WEBRTC_SPL_MUL_16_16(kHpPoleCoefs[1], filter_state[2]);
    tmp32 -= WEBRTC_SPL_MUL_16_16(kHpPoleCoefs[2], filter_state[3]);
    filter_state[3] = filter_state[2];
    filter_state[2] = (int16_t) (tmp32 >> 14);
    *out_ptr++ = filter_state[2];
  }
}

// All pass filtering of |data_in|, used before splitting the signal into two
// frequency bands (low pass vs high pass).
// Note that |data_in| and |data_out| can NOT correspond to the same address.
//
// - data_in            [i]   : Input audio signal given in Q0.
// - data_length        [i]   : Length of input and output data.
// - filter_coefficient [i]   : Given in Q15.
// - filter_state       [i/o] : State of the filter given in Q(-1).
// - data_out           [o]   : Output audio signal given in Q(-1).
static void AllPassFilter(const int16_t* data_in, int data_length,
                          int16_t filter_coefficient, int16_t* filter_state,
                          int16_t* data_out) {
  // The filter can only cause overflow (in the w16 output variable)
  // if more than 4 consecutive input numbers are of maximum value and
  // has the the same sign as the impulse responses first taps.
  // First 6 taps of the impulse response:
  // 0.6399 0.5905 -0.3779 0.2418 -0.1547 0.0990

  int i;
  int16_t tmp16 = 0;
  int32_t tmp32 = 0;
  int32_t state32 = ((int32_t) (*filter_state) << 16);  // Q15

  for (i = 0; i < data_length; i++) {
    tmp32 = state32 + WEBRTC_SPL_MUL_16_16(filter_coefficient, *data_in);
    tmp16 = (int16_t) (tmp32 >> 16);  // Q(-1)
    *data_out++ = tmp16;
    state32 = (((int32_t) (*data_in)) << 14); // Q14
    state32 -= WEBRTC_SPL_MUL_16_16(filter_coefficient, tmp16);  // Q14
    state32 <<= 1;  // Q15.
    data_in += 2;
  }

  *filter_state = (int16_t) (state32 >> 16);  // Q(-1)
}

// Splits |data_in| into |hp_data_out| and |lp_data_out| corresponding to
// an upper (high pass) part and a lower (low pass) part respectively.
//
// - data_in      [i]   : Input audio data to be split into two frequency bands.
// - data_length  [i]   : Length of |data_in|.
// - upper_state  [i/o] : State of the upper filter, given in Q(-1).
// - lower_state  [i/o] : State of the lower filter, given in Q(-1).
// - hp_data_out  [o]   : Output audio data of the upper half of the spectrum.
//                        The length is |data_length| / 2.
// - lp_data_out  [o]   : Output audio data of the lower half of the spectrum.
//                        The length is |data_length| / 2.
static void SplitFilter(const int16_t* data_in, int data_length,
                        int16_t* upper_state, int16_t* lower_state,
                        int16_t* hp_data_out, int16_t* lp_data_out) {
  int i;
  int half_length = data_length >> 1;  // Downsampling by 2.
  int16_t tmp_out;

  // All-pass filtering upper branch.
  AllPassFilter(&data_in[0], half_length, kAllPassCoefsQ15[0], upper_state,
                hp_data_out);

  // All-pass filtering lower branch.
  AllPassFilter(&data_in[1], half_length, kAllPassCoefsQ15[1], lower_state,
                lp_data_out);

  // Make LP and HP signals.
  for (i = 0; i < half_length; i++) {
    tmp_out = *hp_data_out;
    *hp_data_out++ -= *lp_data_out;
    *lp_data_out++ += tmp_out;
  }
}

// Calculates the energy of |data_in| in dB, and also updates an overall
// |total_energy| if necessary.
//
// - data_in      [i]   : Input audio data for energy calculation.
// - data_length  [i]   : Length of input data.
// - offset       [i]   : Offset value added to |log_energy|.
// - total_energy [i/o] : An external energy updated with the energy of
//                        |data_in|.
//                        NOTE: |total_energy| is only updated if
//                        |total_energy| <= |kMinEnergy|.
// - log_energy   [o]   : 10 * log10("energy of |data_in|") given in Q4.
static void LogOfEnergy(const int16_t* data_in, int data_length,
                        int16_t offset, int16_t* total_energy,
                        int16_t* log_energy) {
  // |tot_rshifts| accumulates the number of right shifts performed on |energy|.
  int tot_rshifts = 0;
  // The |energy| will be normalized to 15 bits. We use unsigned integer because
  // we eventually will mask out the fractional part.
  uint32_t energy = 0;

  assert(data_in != NULL);
  assert(data_length > 0);

  energy = (uint32_t) WebRtcSpl_Energy((int16_t*) data_in, data_length,
                                       &tot_rshifts);

  if (energy != 0) {
    // By construction, normalizing to 15 bits is equivalent with 17 leading
    // zeros of an unsigned 32 bit value.
    int normalizing_rshifts = 17 - WebRtcSpl_NormU32(energy);
    // In a 15 bit representation the leading bit is 2^14. log2(2^14) in Q10 is
    // (14 << 10), which is what we initialize |log2_energy| with. For a more
    // detailed derivations, see below.
    int16_t log2_energy = kLogEnergyIntPart;

    tot_rshifts += normalizing_rshifts;
    // Normalize |energy| to 15 bits.
    // |tot_rshifts| is now the total number of right shifts performed on
    // |energy| after normalization. This means that |energy| is in
    // Q(-tot_rshifts).
    if (normalizing_rshifts < 0) {
      energy <<= -normalizing_rshifts;
    } else {
      energy >>= normalizing_rshifts;
    }

    // Calculate the energy of |data_in| in dB, in Q4.
    //
    // 10 * log10("true energy") in Q4 = 2^4 * 10 * log10("true energy") =
    // 160 * log10(|energy| * 2^|tot_rshifts|) =
    // 160 * log10(2) * log2(|energy| * 2^|tot_rshifts|) =
    // 160 * log10(2) * (log2(|energy|) + log2(2^|tot_rshifts|)) =
    // (160 * log10(2)) * (log2(|energy|) + |tot_rshifts|) =
    // |kLogConst| * (|log2_energy| + |tot_rshifts|)
    //
    // We know by construction that |energy| is normalized to 15 bits. Hence,
    // |energy| = 2^14 + frac_Q15, where frac_Q15 is a fractional part in Q15.
    // Further, we'd like |log2_energy| in Q10
    // log2(|energy|) in Q10 = 2^10 * log2(2^14 + frac_Q15) =
    // 2^10 * log2(2^14 * (1 + frac_Q15 * 2^-14)) =
    // 2^10 * (14 + log2(1 + frac_Q15 * 2^-14)) ~=
    // (14 << 10) + 2^10 * (frac_Q15 * 2^-14) =
    // (14 << 10) + (frac_Q15 * 2^-4) = (14 << 10) + (frac_Q15 >> 4)
    //
    // Note that frac_Q15 = (|energy| & 0x00003FFF)

    // Calculate and add the fractional part to |log2_energy|.
    log2_energy += (int16_t) ((energy & 0x00003FFF) >> 4);

    // |kLogConst| is in Q9, |log2_energy| in Q10 and |tot_rshifts| in Q0.
    // Note that we in our derivation above have accounted for an output in Q4.
    *log_energy = (int16_t) (WEBRTC_SPL_MUL_16_16_RSFT(
        kLogConst, log2_energy, 19) +
        WEBRTC_SPL_MUL_16_16_RSFT(tot_rshifts, kLogConst, 9));

    if (*log_energy < 0) {
      *log_energy = 0;
    }
  } else {
    *log_energy = offset;
    return;
  }

  *log_energy += offset;

  // Update the approximate |total_energy| with the energy of |data_in|, if
  // |total_energy| has not exceeded |kMinEnergy|. |total_energy| is used as an
  // energy indicator in WebRtcVad_GmmProbability() in vad_core.c.
  if (*total_energy <= kMinEnergy) {
    if (tot_rshifts >= 0) {
      // We know by construction that the |energy| > |kMinEnergy| in Q0, so add
      // an arbitrary value such that |total_energy| exceeds |kMinEnergy|.
      *total_energy += kMinEnergy + 1;
    } else {
      // By construction |energy| is represented by 15 bits, hence any number of
      // right shifted |energy| will fit in an int16_t. In addition, adding the
      // value to |total_energy| is wrap around safe as long as
      // |kMinEnergy| < 8192.
      *total_energy += (int16_t) (energy >> -tot_rshifts);  // Q0.
    }
  }
}

int16_t WebRtcVad_CalculateFeatures(VadInstT* self, const int16_t* data_in,
                                    int data_length, int16_t* features) {
  int16_t total_energy = 0;
  // We expect |data_length| to be 80, 160 or 240 samples, which corresponds to
  // 10, 20 or 30 ms in 8 kHz. Therefore, the intermediate downsampled data will
  // have at most 120 samples after the first split and at most 60 samples after
  // the second split.
  int16_t hp_120[120], lp_120[120];
  int16_t hp_60[60], lp_60[60];
  const int half_data_length = data_length >> 1;
  int length = half_data_length;  // |data_length| / 2, corresponds to
                                  // bandwidth = 2000 Hz after downsampling.

  // Initialize variables for the first SplitFilter().
  int frequency_band = 0;
  const int16_t* in_ptr = data_in;  // [0 - 4000] Hz.
  int16_t* hp_out_ptr = hp_120;  // [2000 - 4000] Hz.
  int16_t* lp_out_ptr = lp_120;  // [0 - 2000] Hz.

  assert(data_length >= 0);
  assert(data_length <= 240);
  assert(4 < kNumChannels - 1);  // Checking maximum |frequency_band|.

  // Split at 2000 Hz and downsample.
  SplitFilter(in_ptr, data_length, &self->upper_state[frequency_band],
              &self->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);

  // For the upper band (2000 Hz - 4000 Hz) split at 3000 Hz and downsample.
  frequency_band = 1;
  in_ptr = hp_120;  // [2000 - 4000] Hz.
  hp_out_ptr = hp_60;  // [3000 - 4000] Hz.
  lp_out_ptr = lp_60;  // [2000 - 3000] Hz.
  SplitFilter(in_ptr, length, &self->upper_state[frequency_band],
              &self->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);

  // Energy in 3000 Hz - 4000 Hz.
  length >>= 1;  // |data_length| / 4 <=> bandwidth = 1000 Hz.

  LogOfEnergy(hp_60, length, kOffsetVector[5], &total_energy, &features[5]);

  // Energy in 2000 Hz - 3000 Hz.
  LogOfEnergy(lp_60, length, kOffsetVector[4], &total_energy, &features[4]);

  // For the lower band (0 Hz - 2000 Hz) split at 1000 Hz and downsample.
  frequency_band = 2;
  in_ptr = lp_120;  // [0 - 2000] Hz.
  hp_out_ptr = hp_60;  // [1000 - 2000] Hz.
  lp_out_ptr = lp_60;  // [0 - 1000] Hz.
  length = half_data_length;  // |data_length| / 2 <=> bandwidth = 2000 Hz.
  SplitFilter(in_ptr, length, &self->upper_state[frequency_band],
              &self->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);

  // Energy in 1000 Hz - 2000 Hz.
  length >>= 1;  // |data_length| / 4 <=> bandwidth = 1000 Hz.
  LogOfEnergy(hp_60, length, kOffsetVector[3], &total_energy, &features[3]);

  // For the lower band (0 Hz - 1000 Hz) split at 500 Hz and downsample.
  frequency_band = 3;
  in_ptr = lp_60;  // [0 - 1000] Hz.
  hp_out_ptr = hp_120;  // [500 - 1000] Hz.
  lp_out_ptr = lp_120;  // [0 - 500] Hz.
  SplitFilter(in_ptr, length, &self->upper_state[frequency_band],
              &self->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);

  // Energy in 500 Hz - 1000 Hz.
  length >>= 1;  // |data_length| / 8 <=> bandwidth = 500 Hz.
  LogOfEnergy(hp_120, length, kOffsetVector[2], &total_energy, &features[2]);

  // For the lower band (0 Hz - 500 Hz) split at 250 Hz and downsample.
  frequency_band = 4;
  in_ptr = lp_120;  // [0 - 500] Hz.
  hp_out_ptr = hp_60;  // [250 - 500] Hz.
  lp_out_ptr = lp_60;  // [0 - 250] Hz.
  SplitFilter(in_ptr, length, &self->upper_state[frequency_band],
              &self->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);

  // Energy in 250 Hz - 500 Hz.
  length >>= 1;  // |data_length| / 16 <=> bandwidth = 250 Hz.
  LogOfEnergy(hp_60, length, kOffsetVector[1], &total_energy, &features[1]);

  // Remove 0 Hz - 80 Hz, by high pass filtering the lower band.
  HighPassFilter(lp_60, length, self->hp_filter_state, hp_120);

  // Energy in 80 Hz - 250 Hz.
  LogOfEnergy(hp_120, length, kOffsetVector[0], &total_energy, &features[0]);

  return total_energy;
}

static const int32_t kCompVar = 22005;
static const int16_t kLog2Exp = 5909;  // log2(exp(1)) in Q12.

// For a normal distribution, the probability of |input| is calculated and
// returned (in Q20). The formula for normal distributed probability is
//
// 1 / s * exp(-(x - m)^2 / (2 * s^2))
//
// where the parameters are given in the following Q domains:
// m = |mean| (Q7)
// s = |std| (Q7)
// x = |input| (Q4)
// in addition to the probability we output |delta| (in Q11) used when updating
// the noise/speech model.
int32_t WebRtcVad_GaussianProbability(int16_t input,
                                      int16_t mean,
                                      int16_t std,
                                      int16_t* delta) {
  int16_t tmp16, inv_std, inv_std2, exp_value = 0;
  int32_t tmp32;

  // Calculate |inv_std| = 1 / s, in Q10.
  // 131072 = 1 in Q17, and (|std| >> 1) is for rounding instead of truncation.
  // Q-domain: Q17 / Q7 = Q10.
  tmp32 = (int32_t) 131072 + (int32_t) (std >> 1);
  inv_std = (int16_t) WebRtcSpl_DivW32W16(tmp32, std);

  // Calculate |inv_std2| = 1 / s^2, in Q14.
  tmp16 = (inv_std >> 2);  // Q10 -> Q8.
  // Q-domain: (Q8 * Q8) >> 2 = Q14.
  inv_std2 = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(tmp16, tmp16, 2);
  // TODO(bjornv): Investigate if changing to
  // |inv_std2| = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(|inv_std|, |inv_std|, 6);
  // gives better accuracy.

  tmp16 = (input << 3);  // Q4 -> Q7
  tmp16 = tmp16 - mean;  // Q7 - Q7 = Q7

  // To be used later, when updating noise/speech model.
  // |delta| = (x - m) / s^2, in Q11.
  // Q-domain: (Q14 * Q7) >> 10 = Q11.
  *delta = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(inv_std2, tmp16, 10);

  // Calculate the exponent |tmp32| = (x - m)^2 / (2 * s^2), in Q10. Replacing
  // division by two with one shift.
  // Q-domain: (Q11 * Q7) >> 8 = Q10.
  tmp32 = WEBRTC_SPL_MUL_16_16_RSFT(*delta, tmp16, 9);

  // If the exponent is small enough to give a non-zero probability we calculate
  // |exp_value| ~= exp(-(x - m)^2 / (2 * s^2))
  //             ~= exp2(-log2(exp(1)) * |tmp32|).
  if (tmp32 < kCompVar) {
    // Calculate |tmp16| = log2(exp(1)) * |tmp32|, in Q10.
    // Q-domain: (Q12 * Q10) >> 12 = Q10.
    tmp16 = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(kLog2Exp, (int16_t) tmp32, 12);
    tmp16 = -tmp16;
    exp_value = (0x0400 | (tmp16 & 0x03FF));
    tmp16 ^= 0xFFFF;
    tmp16 >>= 10;
    tmp16 += 1;
    // Get |exp_value| = exp(-|tmp32|) in Q10.
    exp_value >>= tmp16;
  }

  // Calculate and return (1 / s) * exp(-(x - m)^2 / (2 * s^2)), in Q20.
  // Q-domain: Q10 * Q10 = Q20.
  return WEBRTC_SPL_MUL_16_16(inv_std, exp_value);
}


#include <assert.h>
// Allpass filter coefficients, upper and lower, in Q13.
// Upper: 0.64, Lower: 0.17.
static const int16_t kAllPassCoefsQ13[2] = { 5243, 1392 };  // Q13.
static const int16_t kSmoothingDown = 6553;  // 0.2 in Q15.
static const int16_t kSmoothingUp = 32439;  // 0.99 in Q15.

// TODO(bjornv): Move this function to vad_filterbank.c.
// Downsampling filter based on splitting filter and allpass functions.
void WebRtcVad_Downsampling(int16_t* signal_in,
                            int16_t* signal_out,
                            int32_t* filter_state,
                            int in_length) {
  int16_t tmp16_1 = 0, tmp16_2 = 0;
  int32_t tmp32_1 = filter_state[0];
  int32_t tmp32_2 = filter_state[1];
  int n = 0;
  int half_length = (in_length >> 1);  // Downsampling by 2 gives half length.

  // Filter coefficients in Q13, filter state in Q0.
  for (n = 0; n < half_length; n++) {
    // All-pass filtering upper branch.
    tmp16_1 = (int16_t) ((tmp32_1 >> 1) +
        WEBRTC_SPL_MUL_16_16_RSFT(kAllPassCoefsQ13[0], *signal_in, 14));
    *signal_out = tmp16_1;
    tmp32_1 = (int32_t) (*signal_in++) -
        WEBRTC_SPL_MUL_16_16_RSFT(kAllPassCoefsQ13[0], tmp16_1, 12);

    // All-pass filtering lower branch.
    tmp16_2 = (int16_t) ((tmp32_2 >> 1) +
        WEBRTC_SPL_MUL_16_16_RSFT(kAllPassCoefsQ13[1], *signal_in, 14));
    *signal_out++ += tmp16_2;
    tmp32_2 = (int32_t) (*signal_in++) -
        WEBRTC_SPL_MUL_16_16_RSFT(kAllPassCoefsQ13[1], tmp16_2, 12);
  }
  // Store the filter states.
  filter_state[0] = tmp32_1;
  filter_state[1] = tmp32_2;
}

// Inserts |feature_value| into |low_value_vector|, if it is one of the 16
// smallest values the last 100 frames. Then calculates and returns the median
// of the five smallest values.
int16_t WebRtcVad_FindMinimum(VadInstT* self,
                              int16_t feature_value,
                              int channel) {
  int i = 0, j = 0;
  int position = -1;
  // Offset to beginning of the 16 minimum values in memory.
  const int offset = (channel << 4);
  int16_t current_median = 1600;
  int16_t alpha = 0;
  int32_t tmp32 = 0;
  // Pointer to memory for the 16 minimum values and the age of each value of
  // the |channel|.
  int16_t* age = &self->index_vector[offset];
  int16_t* smallest_values = &self->low_value_vector[offset];

  assert(channel < kNumChannels);

  // Each value in |smallest_values| is getting 1 loop older. Update |age|, and
  // remove old values.
  for (i = 0; i < 16; i++) {
    if (age[i] != 100) {
      age[i]++;
    } else {
      // Too old value. Remove from memory and shift larger values downwards.
      for (j = i; j < 16; j++) {
        smallest_values[j] = smallest_values[j + 1];
        age[j] = age[j + 1];
      }
      age[15] = 101;
      smallest_values[15] = 10000;
    }
  }

  // Check if |feature_value| is smaller than any of the values in
  // |smallest_values|. If so, find the |position| where to insert the new value
  // (|feature_value|).
  if (feature_value < smallest_values[7]) {
    if (feature_value < smallest_values[3]) {
      if (feature_value < smallest_values[1]) {
        if (feature_value < smallest_values[0]) {
          position = 0;
        } else {
          position = 1;
        }
      } else if (feature_value < smallest_values[2]) {
        position = 2;
      } else {
        position = 3;
      }
    } else if (feature_value < smallest_values[5]) {
      if (feature_value < smallest_values[4]) {
        position = 4;
      } else {
        position = 5;
      }
    } else if (feature_value < smallest_values[6]) {
      position = 6;
    } else {
      position = 7;
    }
  } else if (feature_value < smallest_values[15]) {
    if (feature_value < smallest_values[11]) {
      if (feature_value < smallest_values[9]) {
        if (feature_value < smallest_values[8]) {
          position = 8;
        } else {
          position = 9;
        }
      } else if (feature_value < smallest_values[10]) {
        position = 10;
      } else {
        position = 11;
      }
    } else if (feature_value < smallest_values[13]) {
      if (feature_value < smallest_values[12]) {
        position = 12;
      } else {
        position = 13;
      }
    } else if (feature_value < smallest_values[14]) {
      position = 14;
    } else {
      position = 15;
    }
  }

  // If we have detected a new small value, insert it at the correct position
  // and shift larger values up.
  if (position > -1) {
    for (i = 15; i > position; i--) {
      smallest_values[i] = smallest_values[i - 1];
      age[i] = age[i - 1];
    }
    smallest_values[position] = feature_value;
    age[position] = 1;
  }

  // Get |current_median|.
  if (self->frame_counter > 2) {
    current_median = smallest_values[2];
  } else if (self->frame_counter > 0) {
    current_median = smallest_values[0];
  }

  // Smooth the median value.
  if (self->frame_counter > 0) {
    if (current_median < self->mean_value[channel]) {
      alpha = kSmoothingDown;  // 0.2 in Q15.
    } else {
      alpha = kSmoothingUp;  // 0.99 in Q15.
    }
  }
  tmp32 = WEBRTC_SPL_MUL_16_16(alpha + 1, self->mean_value[channel]);
  tmp32 += WEBRTC_SPL_MUL_16_16(WEBRTC_SPL_WORD16_MAX - alpha, current_median);
  tmp32 += 16384;
  self->mean_value[channel] = (int16_t) (tmp32 >> 15);

  return self->mean_value[channel];
}

void WebRtcSpl_VectorBitShiftW16(WebRtc_Word16 *res,
                             WebRtc_Word16 length,
                             G_CONST WebRtc_Word16 *in,
                             WebRtc_Word16 right_shifts)
{
    int i;

    if (right_shifts > 0)
    {
        for (i = length; i > 0; i--)
        {
            (*res++) = ((*in++) >> right_shifts);
        }
    } else
    {
        for (i = length; i > 0; i--)
        {
            (*res++) = ((*in++) << (-right_shifts));
        }
    }
}

void WebRtcSpl_VectorBitShiftW32(WebRtc_Word32 *out_vector,
                             WebRtc_Word16 vector_length,
                             G_CONST WebRtc_Word32 *in_vector,
                             WebRtc_Word16 right_shifts)
{
    int i;

    if (right_shifts > 0)
    {
        for (i = vector_length; i > 0; i--)
        {
            (*out_vector++) = ((*in_vector++) >> right_shifts);
        }
    } else
    {
        for (i = vector_length; i > 0; i--)
        {
            (*out_vector++) = ((*in_vector++) << (-right_shifts));
        }
    }
}

void WebRtcSpl_VectorBitShiftW32ToW16(int16_t* out, int length,
                                      const int32_t* in, int right_shifts) {
  int i;
  int32_t tmp_w32;

  if (right_shifts >= 0) {
    for (i = length; i > 0; i--) {
      tmp_w32 = (*in++) >> right_shifts;
      (*out++) = WebRtcSpl_SatW32ToW16(tmp_w32);
    }
  } else {
    int16_t left_shifts = -right_shifts;
    for (i = length; i > 0; i--) {
      tmp_w32 = (*in++) << left_shifts;
      (*out++) = WebRtcSpl_SatW32ToW16(tmp_w32);
    }
  }
}

void WebRtcSpl_ScaleVector(G_CONST WebRtc_Word16 *in_vector, WebRtc_Word16 *out_vector,
                           WebRtc_Word16 gain, WebRtc_Word16 in_vector_length,
                           WebRtc_Word16 right_shifts)
{
    // Performs vector operation: out_vector = (gain*in_vector)>>right_shifts
    int i;
    G_CONST WebRtc_Word16 *inptr;
    WebRtc_Word16 *outptr;

    inptr = in_vector;
    outptr = out_vector;

    for (i = 0; i < in_vector_length; i++)
    {
        (*outptr++) = (WebRtc_Word16)WEBRTC_SPL_MUL_16_16_RSFT(*inptr++, gain, right_shifts);
    }
}

void WebRtcSpl_ScaleVectorWithSat(G_CONST WebRtc_Word16 *in_vector, WebRtc_Word16 *out_vector,
                                 WebRtc_Word16 gain, WebRtc_Word16 in_vector_length,
                                 WebRtc_Word16 right_shifts)
{
    // Performs vector operation: out_vector = (gain*in_vector)>>right_shifts
    int i;
    WebRtc_Word32 tmpW32;
    G_CONST WebRtc_Word16 *inptr;
    WebRtc_Word16 *outptr;

    inptr = in_vector;
    outptr = out_vector;

    for (i = 0; i < in_vector_length; i++)
    {
        tmpW32 = WEBRTC_SPL_MUL_16_16_RSFT(*inptr++, gain, right_shifts);
        (*outptr++) = WebRtcSpl_SatW32ToW16(tmpW32);
    }
}

void WebRtcSpl_ScaleAndAddVectors(G_CONST WebRtc_Word16 *in1, WebRtc_Word16 gain1, int shift1,
                                  G_CONST WebRtc_Word16 *in2, WebRtc_Word16 gain2, int shift2,
                                  WebRtc_Word16 *out, int vector_length)
{
    // Performs vector operation: out = (gain1*in1)>>shift1 + (gain2*in2)>>shift2
    int i;
    G_CONST WebRtc_Word16 *in1ptr;
    G_CONST WebRtc_Word16 *in2ptr;
    WebRtc_Word16 *outptr;

    in1ptr = in1;
    in2ptr = in2;
    outptr = out;

    for (i = 0; i < vector_length; i++)
    {
        (*outptr++) = (WebRtc_Word16)WEBRTC_SPL_MUL_16_16_RSFT(gain1, *in1ptr++, shift1)
                + (WebRtc_Word16)WEBRTC_SPL_MUL_16_16_RSFT(gain2, *in2ptr++, shift2);
    }
}

// C version of WebRtcSpl_ScaleAndAddVectorsWithRound() for generic platforms.
int WebRtcSpl_ScaleAndAddVectorsWithRoundC(const int16_t* in_vector1,
                                           int16_t in_vector1_scale,
                                           const int16_t* in_vector2,
                                           int16_t in_vector2_scale,
                                           int right_shifts,
                                           int16_t* out_vector,
                                           int length) {
  int i = 0;
  int round_value = (1 << right_shifts) >> 1;

  if (in_vector1 == NULL || in_vector2 == NULL || out_vector == NULL ||
      length <= 0 || right_shifts < 0) {
    return -1;
  }

  for (i = 0; i < length; i++) {
    out_vector[i] = (int16_t)((
        WEBRTC_SPL_MUL_16_16(in_vector1[i], in_vector1_scale)
        + WEBRTC_SPL_MUL_16_16(in_vector2[i], in_vector2_scale)
        + round_value) >> right_shifts);
  }

  return 0;
}

int32_t WebRtcSpl_Energy(int16_t* vector, int vector_length, int* scale_factor)
{
    int32_t en = 0;
    int i;
    int scaling = WebRtcSpl_GetScalingSquare(vector, vector_length, vector_length);
    int looptimes = vector_length;
    int16_t *vectorptr = vector;

    for (i = 0; i < looptimes; i++)
    {
        en += WEBRTC_SPL_MUL_16_16_RSFT(*vectorptr, *vectorptr, scaling);
        vectorptr++;
    }
    *scale_factor = scaling;

    return en;
}

#include<stdio.h>

/**
 * handle 结果地址
 * kFrameLengths 帧长 { 80, 120, 160, 240, 320, 480, 640, 960 }
 * kRates 处理频率（不是音频的采样频率） { 8000, 12000, 16000, 24000, 32000, 48000 }
 * datas[] 音频数据 长度是kFrameLengths
 * return -1 ERROR, 0 NO VOICE, 1 VOICE
 */
int vad_main_process(VadInst* handle, int kFrameLengths, int kRates, short datas[])
{
	int result = -1;
	result = WebRtcVad_Process(handle, kRates, datas, kFrameLengths);
	printf("result = %d\n",result);

	if(result==-1)
	{	
		printf("WebRtcVad_Process is error\n");
	}	
	
	return result;
}
