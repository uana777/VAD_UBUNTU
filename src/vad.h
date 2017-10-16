#ifndef WEBRTC_TYPEDEFS_H_
#define WEBRTC_TYPEDEFS_H_

// Reserved words definitions
// TODO(andrew): Remove this.
#define G_CONST const

// For access to standard POSIXish features, use WEBRTC_POSIX instead of a
// more specific macro.

#define WEBRTC_LINUX 1

#if defined(WEBRTC_MAC) || defined(WEBRTC_LINUX) || \
    defined(WEBRTC_ANDROID)
#define WEBRTC_POSIX
#endif

// TODO(andrew): replace WEBRTC_LITTLE_ENDIAN with WEBRTC_ARCH_LITTLE_ENDIAN.
#if defined(_M_X64) || defined(__x86_64__)
#define WEBRTC_ARCH_X86_FAMILY
#define WEBRTC_ARCH_X86_64
#define WEBRTC_ARCH_64_BITS
#define WEBRTC_ARCH_LITTLE_ENDIAN
#define WEBRTC_LITTLE_ENDIAN
#elif defined(_M_IX86) || defined(__i386__)
#define WEBRTC_ARCH_X86_FAMILY
#define WEBRTC_ARCH_X86
#define WEBRTC_ARCH_32_BITS
#define WEBRTC_ARCH_LITTLE_ENDIAN
#define WEBRTC_LITTLE_ENDIAN
#elif defined(__ARMEL__)
// TODO(andrew): We'd prefer to control platform defines here, but this is
// currently provided by the Android makefiles. Commented to avoid duplicate
// definition warnings.
//#define WEBRTC_ARCH_ARM
// TODO(andrew): Chromium uses the following two defines. Should we switch?
//#define WEBRTC_ARCH_ARM_FAMILY
//#define WEBRTC_ARCH_ARMEL
#define WEBRTC_ARCH_32_BITS
#define WEBRTC_ARCH_LITTLE_ENDIAN
#define WEBRTC_LITTLE_ENDIAN
#elif defined(__MIPSEL__)
#define WEBRTC_ARCH_32_BITS
#define WEBRTC_ARCH_LITTLE_ENDIAN
#define WEBRTC_LITTLE_ENDIAN
#else
#error Please add support for your architecture in typedefs.h
#endif

#if defined(__SSE2__) || defined(_MSC_VER)
#define WEBRTC_USE_SSE2
#endif

#if !defined(_MSC_VER)
#include <stdint.h>
#else
// Define C99 equivalent types, since MSVC doesn't provide stdint.h.
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef __int64             int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned __int64    uint64_t;
#endif

// TODO(andrew): remove WebRtc_ types:
// http://code.google.com/p/webrtc/issues/detail?id=314
typedef int8_t              WebRtc_Word8;
typedef int16_t             WebRtc_Word16;
typedef int32_t             WebRtc_Word32;
typedef int64_t             WebRtc_Word64;
typedef uint8_t             WebRtc_UWord8;
typedef uint16_t            WebRtc_UWord16;
typedef uint32_t            WebRtc_UWord32;
typedef uint64_t            WebRtc_UWord64;

#endif  // WEBRTC_TYPEDEFS_H_


#ifndef WEBRTC_COMMON_AUDIO_VAD_INCLUDE_WEBRTC_VAD_H_  // NOLINT
#define WEBRTC_COMMON_AUDIO_VAD_INCLUDE_WEBRTC_VAD_H_

#ifndef WEBRTC_COMMON_AUDIO_SIGNAL_PROCESSING_INCLUDE_REAL_FFT_H_
#define WEBRTC_COMMON_AUDIO_SIGNAL_PROCESSING_INCLUDE_REAL_FFT_H_

typedef struct WebRtcVadInst VadInst;

struct RealFFT;

#ifdef __cplusplus
extern "C" {
#endif

// Creates an instance to the VAD structure.
// - handle [o] : Pointer to the VAD instance that should be created.
// returns      : 0 - (OK), -1 - (Error)
int WebRtcVad_Create(VadInst** handle);

// Frees the dynamic memory of a specified VAD instance.
// - handle [i] : Pointer to VAD instance that should be freed.
// returns      : 0 - (OK), -1 - (NULL pointer in)
int WebRtcVad_Free(VadInst* handle);

// Initializes a VAD instance.
// - handle [i/o] : Instance that should be initialized.
// returns        : 0 - (OK),
//                 -1 - (NULL pointer or Default mode could not be set).
int WebRtcVad_Init(VadInst* handle);

// Sets the VAD operating mode. A more aggressive (higher mode) VAD is more
// restrictive in reporting speech. Put in other words the probability of being
// speech when the VAD returns 1 is increased with increasing mode. As a
// consequence also the missed detection rate goes up.
//
// - handle [i/o] : VAD instance.
// - mode   [i]   : Aggressiveness mode (0, 1, 2, or 3).
//
// returns        : 0 - (OK),
//                 -1 - (NULL pointer, mode could not be set or the VAD instance
//                       has not been initialized).
int WebRtcVad_set_mode(VadInst* handle, int mode);

// Calculates a VAD decision for the |audio_frame|. For valid sampling rates
// frame lengths, see the description of WebRtcVad_ValidRatesAndFrameLengths().
//
// - handle       [i/o] : VAD Instance. Needs to be initialized by
//                        WebRtcVad_Init() before call.
// - fs           [i]   : Sampling frequency (Hz): 8000, 16000, or 32000
// - audio_frame  [i]   : Audio frame buffer.
// - frame_length [i]   : Length of audio frame buffer in number of samples.
//
// returns              : 1 - (Active Voice),
//                        0 - (Non-active Voice),
//                       -1 - (Error)
int WebRtcVad_Process(VadInst* handle, int fs, int16_t* audio_frame,
                      int frame_length);

// Checks for valid combinations of |rate| and |frame_length|. We support 10,
// 20 and 30 ms frames and the rates 8000, 16000 and 32000 Hz.
//
// - rate         [i] : Sampling frequency (Hz).
// - frame_length [i] : Speech frame buffer length in number of samples.
//
// returns            : 0 - (valid combination), -1 - (invalid combination)
int WebRtcVad_ValidRateAndFrameLength(int rate, int frame_length);

typedef int (*RealForwardFFT)(struct RealFFT* self,
                              const int16_t* data_in,
                              int16_t* data_out);
typedef int (*RealInverseFFT)(struct RealFFT* self,
                              const int16_t* data_in,
                              int16_t* data_out);

extern RealForwardFFT WebRtcSpl_RealForwardFFT;
extern RealInverseFFT WebRtcSpl_RealInverseFFT;

struct RealFFT* WebRtcSpl_CreateRealFFT(int order);
void WebRtcSpl_FreeRealFFT(struct RealFFT* self);

// TODO(kma): Implement FFT functions for real signals.

// Compute the forward FFT for a complex signal of length 2^order.
// Input Arguments:
//   self - pointer to preallocated and initialized FFT specification structure.
//   data_in - the input signal.
//
// Output Arguments:
//   data_out - the output signal; must be different to data_in.
//
// Return Value:
//   0  - FFT calculation is successful.
//   -1 - Error
//
int WebRtcSpl_RealForwardFFTC(struct RealFFT* self,
                              const int16_t* data_in,
                              int16_t* data_out);

#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
int WebRtcSpl_RealForwardFFTNeon(struct RealFFT* self,
                                 const int16_t* data_in,
                                 int16_t* data_out);
#endif

// Compute the inverse FFT for a complex signal of length 2^order.
// Input Arguments:
//   self - pointer to preallocated and initialized FFT specification structure.
//   data_in - the input signal.
//
// Output Arguments:
//   data_out - the output signal; must be different to data_in.
//
// Return Value:
//   0 or a positive number - a value that the elements in the |data_out| should
//                            be shifted left with in order to get correct
//                            physical values.
//   -1                     - Error
int WebRtcSpl_RealInverseFFTC(struct RealFFT* self,
                              const int16_t* data_in,
                              int16_t* data_out);

#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
int WebRtcSpl_RealInverseFFTNeon(struct RealFFT* self,
                                 const int16_t* data_in,
                                 int16_t* data_out);
#endif

#ifdef __cplusplus
}
#endif
#endif
#endif  // WEBRTC_COMMON_AUDIO_VAD_INCLUDE_WEBRTC_VAD_H_  // NOLINT


#ifndef WEBRTC_SYSTEM_WRAPPERS_INTERFACE_CPU_FEATURES_WRAPPER_H_
#define WEBRTC_SYSTEM_WRAPPERS_INTERFACE_CPU_FEATURES_WRAPPER_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
// List of features in x86.
typedef enum {
  kSSE2,
  kSSE3
} CPUFeature;

// List of features in ARM.
enum {
  kCPUFeatureARMv7       = (1 << 0),
  kCPUFeatureVFPv3       = (1 << 1),
  kCPUFeatureNEON        = (1 << 2),
  kCPUFeatureLDREXSTREX  = (1 << 3)
};

typedef int (*WebRtc_CPUInfo)(CPUFeature feature);

// Returns true if the CPU supports the feature.
extern WebRtc_CPUInfo WebRtc_GetCPUInfo;

// No CPU feature is available => straight C path.
extern WebRtc_CPUInfo WebRtc_GetCPUInfoNoASM;

// Return the features in an ARM device.
// It detects the features in the hardware platform, and returns supported
// values in the above enum definition as a bitmask.
extern uint64_t WebRtc_GetCPUFeaturesARM(void);

#if defined(__cplusplus) || defined(c_plusplus)
}    // extern "C"
#endif

#endif // WEBRTC_SYSTEM_WRAPPERS_INTERFACE_CPU_FEATURES_WRAPPER_H_


#ifndef WEBRTC_SPL_RESAMPLE_BY_2_INTERNAL_H_
#define WEBRTC_SPL_RESAMPLE_BY_2_INTERNAL_H_

/*******************************************************************
 * resample_by_2_fast.c
 * Functions for internal use in the other resample functions
 ******************************************************************/
void WebRtcSpl_DownBy2IntToShort(WebRtc_Word32 *in, WebRtc_Word32 len, WebRtc_Word16 *out,
                                 WebRtc_Word32 *state);

void WebRtcSpl_DownBy2ShortToInt(const WebRtc_Word16 *in, WebRtc_Word32 len,
                                 WebRtc_Word32 *out, WebRtc_Word32 *state);

void WebRtcSpl_UpBy2ShortToInt(const WebRtc_Word16 *in, WebRtc_Word32 len,
                               WebRtc_Word32 *out, WebRtc_Word32 *state);

void WebRtcSpl_UpBy2IntToInt(const WebRtc_Word32 *in, WebRtc_Word32 len, WebRtc_Word32 *out,
                             WebRtc_Word32 *state);

void WebRtcSpl_UpBy2IntToShort(const WebRtc_Word32 *in, WebRtc_Word32 len,
                               WebRtc_Word16 *out, WebRtc_Word32 *state);

void WebRtcSpl_LPBy2ShortToInt(const WebRtc_Word16* in, WebRtc_Word32 len,
                               WebRtc_Word32* out, WebRtc_Word32* state);

void WebRtcSpl_LPBy2IntToInt(const WebRtc_Word32* in, WebRtc_Word32 len, WebRtc_Word32* out,
                             WebRtc_Word32* state);

#endif // WEBRTC_SPL_RESAMPLE_BY_2_INTERNAL_H_


#ifndef WEBRTC_SPL_SIGNAL_PROCESSING_LIBRARY_H_
#define WEBRTC_SPL_SIGNAL_PROCESSING_LIBRARY_H_

#include <string.h>

// Macros specific for the fixed point implementation
#define WEBRTC_SPL_WORD16_MAX       32767
#define WEBRTC_SPL_WORD16_MIN       -32768
#define WEBRTC_SPL_WORD32_MAX       (WebRtc_Word32)0x7fffffff
#define WEBRTC_SPL_WORD32_MIN       (WebRtc_Word32)0x80000000
#define WEBRTC_SPL_MAX_LPC_ORDER    14
#define WEBRTC_SPL_MAX_SEED_USED    0x80000000L
#define WEBRTC_SPL_MIN(A, B)        (A < B ? A : B) // Get min value
#define WEBRTC_SPL_MAX(A, B)        (A > B ? A : B) // Get max value
// TODO(kma/bjorn): For the next two macros, investigate how to correct the code
// for inputs of a = WEBRTC_SPL_WORD16_MIN or WEBRTC_SPL_WORD32_MIN.
#define WEBRTC_SPL_ABS_W16(a) \
    (((WebRtc_Word16)a >= 0) ? ((WebRtc_Word16)a) : -((WebRtc_Word16)a))
#define WEBRTC_SPL_ABS_W32(a) \
    (((WebRtc_Word32)a >= 0) ? ((WebRtc_Word32)a) : -((WebRtc_Word32)a))

#ifdef WEBRTC_ARCH_LITTLE_ENDIAN
#define WEBRTC_SPL_GET_BYTE(a, nr)  (((WebRtc_Word8 *)a)[nr])
#define WEBRTC_SPL_SET_BYTE(d_ptr, val, index) \
    (((WebRtc_Word8 *)d_ptr)[index] = (val))
#else
#define WEBRTC_SPL_GET_BYTE(a, nr) \
    ((((WebRtc_Word16 *)a)[nr >> 1]) >> (((nr + 1) & 0x1) * 8) & 0x00ff)
#define WEBRTC_SPL_SET_BYTE(d_ptr, val, index) \
    ((WebRtc_Word16 *)d_ptr)[index >> 1] = \
    ((((WebRtc_Word16 *)d_ptr)[index >> 1]) \
    & (0x00ff << (8 * ((index) & 0x1)))) | (val << (8 * ((index + 1) & 0x1)))
#endif

#define WEBRTC_SPL_MUL(a, b) \
    ((WebRtc_Word32) ((WebRtc_Word32)(a) * (WebRtc_Word32)(b)))
#define WEBRTC_SPL_UMUL(a, b) \
    ((WebRtc_UWord32) ((WebRtc_UWord32)(a) * (WebRtc_UWord32)(b)))
#define WEBRTC_SPL_UMUL_RSFT16(a, b) \
    ((WebRtc_UWord32) ((WebRtc_UWord32)(a) * (WebRtc_UWord32)(b)) >> 16)
#define WEBRTC_SPL_UMUL_16_16(a, b) \
    ((WebRtc_UWord32) (WebRtc_UWord16)(a) * (WebRtc_UWord16)(b))
#define WEBRTC_SPL_UMUL_16_16_RSFT16(a, b) \
    (((WebRtc_UWord32) (WebRtc_UWord16)(a) * (WebRtc_UWord16)(b)) >> 16)
#define WEBRTC_SPL_UMUL_32_16(a, b) \
    ((WebRtc_UWord32) ((WebRtc_UWord32)(a) * (WebRtc_UWord16)(b)))
#define WEBRTC_SPL_UMUL_32_16_RSFT16(a, b) \
    ((WebRtc_UWord32) ((WebRtc_UWord32)(a) * (WebRtc_UWord16)(b)) >> 16)
#define WEBRTC_SPL_MUL_16_U16(a, b) \
    ((WebRtc_Word32)(WebRtc_Word16)(a) * (WebRtc_UWord16)(b))
#define WEBRTC_SPL_DIV(a, b) \
    ((WebRtc_Word32) ((WebRtc_Word32)(a) / (WebRtc_Word32)(b)))
#define WEBRTC_SPL_UDIV(a, b) \
    ((WebRtc_UWord32) ((WebRtc_UWord32)(a) / (WebRtc_UWord32)(b)))

#ifndef WEBRTC_ARCH_ARM_V7
// For ARMv7 platforms, these are inline functions in spl_inl_armv7.h
#define WEBRTC_SPL_MUL_16_16(a, b) \
    ((WebRtc_Word32) (((WebRtc_Word16)(a)) * ((WebRtc_Word16)(b))))
#define WEBRTC_SPL_MUL_16_32_RSFT16(a, b) \
    (WEBRTC_SPL_MUL_16_16(a, b >> 16) \
     + ((WEBRTC_SPL_MUL_16_16(a, (b & 0xffff) >> 1) + 0x4000) >> 15))
#define WEBRTC_SPL_MUL_32_32_RSFT32(a32a, a32b, b32) \
    ((WebRtc_Word32)(WEBRTC_SPL_MUL_16_32_RSFT16(a32a, b32) \
    + (WEBRTC_SPL_MUL_16_32_RSFT16(a32b, b32) >> 16)))
#define WEBRTC_SPL_MUL_32_32_RSFT32BI(a32, b32) \
    ((WebRtc_Word32)(WEBRTC_SPL_MUL_16_32_RSFT16(( \
    (WebRtc_Word16)(a32 >> 16)), b32) + \
    (WEBRTC_SPL_MUL_16_32_RSFT16(( \
    (WebRtc_Word16)((a32 & 0x0000FFFF) >> 1)), b32) >> 15)))
#endif

#define WEBRTC_SPL_MUL_16_32_RSFT11(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, (b) >> 16) << 5) \
    + (((WEBRTC_SPL_MUL_16_U16(a, (WebRtc_UWord16)(b)) >> 1) + 0x0200) >> 10))
#define WEBRTC_SPL_MUL_16_32_RSFT14(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, (b) >> 16) << 2) \
    + (((WEBRTC_SPL_MUL_16_U16(a, (WebRtc_UWord16)(b)) >> 1) + 0x1000) >> 13))
#define WEBRTC_SPL_MUL_16_32_RSFT15(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, (b) >> 16) << 1) \
    + (((WEBRTC_SPL_MUL_16_U16(a, (WebRtc_UWord16)(b)) >> 1) + 0x2000) >> 14))

#define WEBRTC_SPL_MUL_16_16_RSFT(a, b, c) \
    (WEBRTC_SPL_MUL_16_16(a, b) >> (c))

#define WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(a, b, c) \
    ((WEBRTC_SPL_MUL_16_16(a, b) + ((WebRtc_Word32) \
                                  (((WebRtc_Word32)1) << ((c) - 1)))) >> (c))
#define WEBRTC_SPL_MUL_16_16_RSFT_WITH_FIXROUND(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, b) + ((WebRtc_Word32) (1 << 14))) >> 15)

// C + the 32 most significant bits of A * B
#define WEBRTC_SPL_SCALEDIFF32(A, B, C) \
    (C + (B >> 16) * A + (((WebRtc_UWord32)(0x0000FFFF & B) * A) >> 16))

#define WEBRTC_SPL_ADD_SAT_W32(a, b)    WebRtcSpl_AddSatW32(a, b)
#define WEBRTC_SPL_SAT(a, b, c)         (b > a ? a : b < c ? c : b)
#define WEBRTC_SPL_MUL_32_16(a, b)      ((a) * (b))

#define WEBRTC_SPL_SUB_SAT_W32(a, b)    WebRtcSpl_SubSatW32(a, b)
#define WEBRTC_SPL_ADD_SAT_W16(a, b)    WebRtcSpl_AddSatW16(a, b)
#define WEBRTC_SPL_SUB_SAT_W16(a, b)    WebRtcSpl_SubSatW16(a, b)

// We cannot do casting here due to signed/unsigned problem
#define WEBRTC_SPL_IS_NEG(a)            ((a) & 0x80000000)
// Shifting with negative numbers allowed
// Positive means left shift
#define WEBRTC_SPL_SHIFT_W16(x, c) \
    (((c) >= 0) ? ((x) << (c)) : ((x) >> (-(c))))
#define WEBRTC_SPL_SHIFT_W32(x, c) \
    (((c) >= 0) ? ((x) << (c)) : ((x) >> (-(c))))

// Shifting with negative numbers not allowed
// We cannot do casting here due to signed/unsigned problem
#define WEBRTC_SPL_RSHIFT_W16(x, c)     ((x) >> (c))
#define WEBRTC_SPL_LSHIFT_W16(x, c)     ((x) << (c))
#define WEBRTC_SPL_RSHIFT_W32(x, c)     ((x) >> (c))
#define WEBRTC_SPL_LSHIFT_W32(x, c)     ((x) << (c))

#define WEBRTC_SPL_RSHIFT_U16(x, c)     ((WebRtc_UWord16)(x) >> (c))
#define WEBRTC_SPL_LSHIFT_U16(x, c)     ((WebRtc_UWord16)(x) << (c))
#define WEBRTC_SPL_RSHIFT_U32(x, c)     ((WebRtc_UWord32)(x) >> (c))
#define WEBRTC_SPL_LSHIFT_U32(x, c)     ((WebRtc_UWord32)(x) << (c))

#define WEBRTC_SPL_VNEW(t, n)           (t *) malloc (sizeof (t) * (n))
#define WEBRTC_SPL_FREE                 free

#define WEBRTC_SPL_RAND(a) \
    ((WebRtc_Word16)(WEBRTC_SPL_MUL_16_16_RSFT((a), 18816, 7) & 0x00007fff))

#ifdef __cplusplus
extern "C"
{
#endif

#define WEBRTC_SPL_MEMCPY_W8(v1, v2, length) \
   memcpy(v1, v2, (length) * sizeof(char))
#define WEBRTC_SPL_MEMCPY_W16(v1, v2, length) \
   memcpy(v1, v2, (length) * sizeof(WebRtc_Word16))

#define WEBRTC_SPL_MEMMOVE_W16(v1, v2, length) \
   memmove(v1, v2, (length) * sizeof(WebRtc_Word16))


// Initialize SPL. Currently it contains only function pointer initialization.
// If the underlying platform is known to be ARM-Neon (WEBRTC_ARCH_ARM_NEON
// defined), the pointers will be assigned to code optimized for Neon; otherwise
// if run-time Neon detection (WEBRTC_DETECT_ARM_NEON) is enabled, the pointers
// will be assigned to either Neon code or generic C code; otherwise, generic C
// code will be assigned.
// Note that this function MUST be called in any application that uses SPL
// functions.
void WebRtcSpl_Init();

// Get SPL Version
WebRtc_Word16 WebRtcSpl_get_version(char* version,
                                    WebRtc_Word16 length_in_bytes);

int WebRtcSpl_GetScalingSquare(WebRtc_Word16* in_vector,
                               int in_vector_length,
                               int times);

// Copy and set operations. Implementation in copy_set_operations.c.
// Descriptions at bottom of file.
void WebRtcSpl_MemSetW16(WebRtc_Word16* vector,
                         WebRtc_Word16 set_value,
                         int vector_length);
void WebRtcSpl_MemSetW32(WebRtc_Word32* vector,
                         WebRtc_Word32 set_value,
                         int vector_length);
void WebRtcSpl_MemCpyReversedOrder(WebRtc_Word16* out_vector,
                                   WebRtc_Word16* in_vector,
                                   int vector_length);
WebRtc_Word16 WebRtcSpl_CopyFromEndW16(G_CONST WebRtc_Word16* in_vector,
                                       WebRtc_Word16 in_vector_length,
                                       WebRtc_Word16 samples,
                                       WebRtc_Word16* out_vector);
WebRtc_Word16 WebRtcSpl_ZerosArrayW16(WebRtc_Word16* vector,
                                      WebRtc_Word16 vector_length);
WebRtc_Word16 WebRtcSpl_ZerosArrayW32(WebRtc_Word32* vector,
                                      WebRtc_Word16 vector_length);
WebRtc_Word16 WebRtcSpl_OnesArrayW16(WebRtc_Word16* vector,
                                     WebRtc_Word16 vector_length);
WebRtc_Word16 WebRtcSpl_OnesArrayW32(WebRtc_Word32* vector,
                                     WebRtc_Word16 vector_length);
// End: Copy and set operations.


// Minimum and maximum operation functions and their pointers.
// Implementation in min_max_operations.c.

// Returns the largest absolute value in a signed 16-bit vector.
//
// Input:
//      - vector : 16-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Maximum absolute value in vector;
//                 or -1, if (vector == NULL || length <= 0).
typedef int16_t (*MaxAbsValueW16)(const int16_t* vector, int length);
extern MaxAbsValueW16 WebRtcSpl_MaxAbsValueW16;
int16_t WebRtcSpl_MaxAbsValueW16C(const int16_t* vector, int length);
#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
int16_t WebRtcSpl_MaxAbsValueW16Neon(const int16_t* vector, int length);
#endif

// Returns the largest absolute value in a signed 32-bit vector.
//
// Input:
//      - vector : 32-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Maximum absolute value in vector;
//                 or -1, if (vector == NULL || length <= 0).
typedef int32_t (*MaxAbsValueW32)(const int32_t* vector, int length);
extern MaxAbsValueW32 WebRtcSpl_MaxAbsValueW32;
int32_t WebRtcSpl_MaxAbsValueW32C(const int32_t* vector, int length);
#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
int32_t WebRtcSpl_MaxAbsValueW32Neon(const int32_t* vector, int length);
#endif

// Returns the maximum value of a 16-bit vector.
//
// Input:
//      - vector : 16-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Maximum sample value in |vector|.
//                 If (vector == NULL || length <= 0) WEBRTC_SPL_WORD16_MIN
//                 is returned. Note that WEBRTC_SPL_WORD16_MIN is a feasible
//                 value and we can't catch errors purely based on it.
typedef int16_t (*MaxValueW16)(const int16_t* vector, int length);
extern MaxValueW16 WebRtcSpl_MaxValueW16;
int16_t WebRtcSpl_MaxValueW16C(const int16_t* vector, int length);
#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
int16_t WebRtcSpl_MaxValueW16Neon(const int16_t* vector, int length);
#endif

// Returns the maximum value of a 32-bit vector.
//
// Input:
//      - vector : 32-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Maximum sample value in |vector|.
//                 If (vector == NULL || length <= 0) WEBRTC_SPL_WORD32_MIN
//                 is returned. Note that WEBRTC_SPL_WORD32_MIN is a feasible
//                 value and we can't catch errors purely based on it.
typedef int32_t (*MaxValueW32)(const int32_t* vector, int length);
extern MaxValueW32 WebRtcSpl_MaxValueW32;
int32_t WebRtcSpl_MaxValueW32C(const int32_t* vector, int length);
#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
int32_t WebRtcSpl_MaxValueW32Neon(const int32_t* vector, int length);
#endif

// Returns the minimum value of a 16-bit vector.
//
// Input:
//      - vector : 16-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Minimum sample value in |vector|.
//                 If (vector == NULL || length <= 0) WEBRTC_SPL_WORD16_MAX
//                 is returned. Note that WEBRTC_SPL_WORD16_MAX is a feasible
//                 value and we can't catch errors purely based on it.
typedef int16_t (*MinValueW16)(const int16_t* vector, int length);
extern MinValueW16 WebRtcSpl_MinValueW16;
int16_t WebRtcSpl_MinValueW16C(const int16_t* vector, int length);
#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
int16_t WebRtcSpl_MinValueW16Neon(const int16_t* vector, int length);
#endif

// Returns the minimum value of a 32-bit vector.
//
// Input:
//      - vector : 32-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Minimum sample value in |vector|.
//                 If (vector == NULL || length <= 0) WEBRTC_SPL_WORD32_MAX
//                 is returned. Note that WEBRTC_SPL_WORD32_MAX is a feasible
//                 value and we can't catch errors purely based on it.
typedef int32_t (*MinValueW32)(const int32_t* vector, int length);
extern MinValueW32 WebRtcSpl_MinValueW32;
int32_t WebRtcSpl_MinValueW32C(const int32_t* vector, int length);
#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
int32_t WebRtcSpl_MinValueW32Neon(const int32_t* vector, int length);
#endif

// Returns the vector index to the largest absolute value of a 16-bit vector.
//
// Input:
//      - vector : 16-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Index to the maximum absolute value in vector, or -1,
//                 if (vector == NULL || length <= 0).
//                 If there are multiple equal maxima, return the index of the
//                 first. -32768 will always have precedence over 32767 (despite
//                 -32768 presenting an int16 absolute value of 32767);
int WebRtcSpl_MaxAbsIndexW16(const int16_t* vector, int length);

// Returns the vector index to the maximum sample value of a 16-bit vector.
//
// Input:
//      - vector : 16-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Index to the maximum value in vector (if multiple
//                 indexes have the maximum, return the first);
//                 or -1, if (vector == NULL || length <= 0).
int WebRtcSpl_MaxIndexW16(const int16_t* vector, int length);

// Returns the vector index to the maximum sample value of a 32-bit vector.
//
// Input:
//      - vector : 32-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Index to the maximum value in vector (if multiple
//                 indexes have the maximum, return the first);
//                 or -1, if (vector == NULL || length <= 0).
int WebRtcSpl_MaxIndexW32(const int32_t* vector, int length);

// Returns the vector index to the minimum sample value of a 16-bit vector.
//
// Input:
//      - vector : 16-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Index to the mimimum value in vector  (if multiple
//                 indexes have the minimum, return the first);
//                 or -1, if (vector == NULL || length <= 0).
int WebRtcSpl_MinIndexW16(const int16_t* vector, int length);

// Returns the vector index to the minimum sample value of a 32-bit vector.
//
// Input:
//      - vector : 32-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Index to the mimimum value in vector  (if multiple
//                 indexes have the minimum, return the first);
//                 or -1, if (vector == NULL || length <= 0).
int WebRtcSpl_MinIndexW32(const int32_t* vector, int length);

// End: Minimum and maximum operations.


// Vector scaling operations. Implementation in vector_scaling_operations.c.
// Description at bottom of file.
void WebRtcSpl_VectorBitShiftW16(WebRtc_Word16* out_vector,
                                 WebRtc_Word16 vector_length,
                                 G_CONST WebRtc_Word16* in_vector,
                                 WebRtc_Word16 right_shifts);
void WebRtcSpl_VectorBitShiftW32(WebRtc_Word32* out_vector,
                                 WebRtc_Word16 vector_length,
                                 G_CONST WebRtc_Word32* in_vector,
                                 WebRtc_Word16 right_shifts);
void WebRtcSpl_VectorBitShiftW32ToW16(int16_t* out_vector,
                                      int vector_length,
                                      const int32_t* in_vector,
                                      int right_shifts);
void WebRtcSpl_ScaleVector(G_CONST WebRtc_Word16* in_vector,
                           WebRtc_Word16* out_vector,
                           WebRtc_Word16 gain,
                           WebRtc_Word16 vector_length,
                           WebRtc_Word16 right_shifts);
void WebRtcSpl_ScaleVectorWithSat(G_CONST WebRtc_Word16* in_vector,
                                  WebRtc_Word16* out_vector,
                                  WebRtc_Word16 gain,
                                  WebRtc_Word16 vector_length,
                                  WebRtc_Word16 right_shifts);
void WebRtcSpl_ScaleAndAddVectors(G_CONST WebRtc_Word16* in_vector1,
                                  WebRtc_Word16 gain1, int right_shifts1,
                                  G_CONST WebRtc_Word16* in_vector2,
                                  WebRtc_Word16 gain2, int right_shifts2,
                                  WebRtc_Word16* out_vector,
                                  int vector_length);

// The functions (with related pointer) perform the vector operation:
//   out_vector[k] = ((scale1 * in_vector1[k]) + (scale2 * in_vector2[k])
//        + round_value) >> right_shifts,
//   where  round_value = (1 << right_shifts) >> 1.
//
// Input:
//      - in_vector1       : Input vector 1
//      - in_vector1_scale : Gain to be used for vector 1
//      - in_vector2       : Input vector 2
//      - in_vector2_scale : Gain to be used for vector 2
//      - right_shifts     : Number of right bit shifts to be applied
//      - length           : Number of elements in the input vectors
//
// Output:
//      - out_vector       : Output vector
// Return value            : 0 if OK, -1 if (in_vector1 == NULL
//                           || in_vector2 == NULL || out_vector == NULL
//                           || length <= 0 || right_shift < 0).
typedef int (*ScaleAndAddVectorsWithRound)(const int16_t* in_vector1,
                                           int16_t in_vector1_scale,
                                           const int16_t* in_vector2,
                                           int16_t in_vector2_scale,
                                           int right_shifts,
                                           int16_t* out_vector,
                                           int length);
extern ScaleAndAddVectorsWithRound WebRtcSpl_ScaleAndAddVectorsWithRound;
int WebRtcSpl_ScaleAndAddVectorsWithRoundC(const int16_t* in_vector1,
                                           int16_t in_vector1_scale,
                                           const int16_t* in_vector2,
                                           int16_t in_vector2_scale,
                                           int right_shifts,
                                           int16_t* out_vector,
                                           int length);
#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
int WebRtcSpl_ScaleAndAddVectorsWithRoundNeon(const int16_t* in_vector1,
                                              int16_t in_vector1_scale,
                                              const int16_t* in_vector2,
                                              int16_t in_vector2_scale,
                                              int right_shifts,
                                              int16_t* out_vector,
                                              int length);
#endif
// End: Vector scaling operations.

// iLBC specific functions. Implementations in ilbc_specific_functions.c.
// Description at bottom of file.
void WebRtcSpl_ReverseOrderMultArrayElements(WebRtc_Word16* out_vector,
                                             G_CONST WebRtc_Word16* in_vector,
                                             G_CONST WebRtc_Word16* window,
                                             WebRtc_Word16 vector_length,
                                             WebRtc_Word16 right_shifts);
void WebRtcSpl_ElementwiseVectorMult(WebRtc_Word16* out_vector,
                                     G_CONST WebRtc_Word16* in_vector,
                                     G_CONST WebRtc_Word16* window,
                                     WebRtc_Word16 vector_length,
                                     WebRtc_Word16 right_shifts);
void WebRtcSpl_AddVectorsAndShift(WebRtc_Word16* out_vector,
                                  G_CONST WebRtc_Word16* in_vector1,
                                  G_CONST WebRtc_Word16* in_vector2,
                                  WebRtc_Word16 vector_length,
                                  WebRtc_Word16 right_shifts);
void WebRtcSpl_AddAffineVectorToVector(WebRtc_Word16* out_vector,
                                       WebRtc_Word16* in_vector,
                                       WebRtc_Word16 gain,
                                       WebRtc_Word32 add_constant,
                                       WebRtc_Word16 right_shifts,
                                       int vector_length);
void WebRtcSpl_AffineTransformVector(WebRtc_Word16* out_vector,
                                     WebRtc_Word16* in_vector,
                                     WebRtc_Word16 gain,
                                     WebRtc_Word32 add_constant,
                                     WebRtc_Word16 right_shifts,
                                     int vector_length);
// End: iLBC specific functions.

// Signal processing operations.

// A 32-bit fix-point implementation of auto-correlation computation
//
// Input:
//      - in_vector        : Vector to calculate autocorrelation upon
//      - in_vector_length : Length (in samples) of |vector|
//      - order            : The order up to which the autocorrelation should be
//                           calculated
//
// Output:
//      - result           : auto-correlation values (values should be seen
//                           relative to each other since the absolute values
//                           might have been down shifted to avoid overflow)
//
//      - scale            : The number of left shifts required to obtain the
//                           auto-correlation in Q0
//
// Return value            :
//      - -1, if |order| > |in_vector_length|;
//      - Number of samples in |result|, i.e. (order+1), otherwise.
int WebRtcSpl_AutoCorrelation(const int16_t* in_vector,
                              int in_vector_length,
                              int order,
                              int32_t* result,
                              int* scale);

// A 32-bit fix-point implementation of the Levinson-Durbin algorithm that
// does NOT use the 64 bit class
//
// Input:
//      - auto_corr : Vector with autocorrelation values of length >=
//                    |use_order|+1
//      - use_order : The LPC filter order (support up to order 20)
//
// Output:
//      - lpc_coef  : lpc_coef[0..use_order] LPC coefficients in Q12
//      - refl_coef : refl_coef[0...use_order-1]| Reflection coefficients in
//                    Q15
//
// Return value     : 1 for stable 0 for unstable
WebRtc_Word16 WebRtcSpl_LevinsonDurbin(WebRtc_Word32* auto_corr,
                                       WebRtc_Word16* lpc_coef,
                                       WebRtc_Word16* refl_coef,
                                       WebRtc_Word16 order);

// Converts reflection coefficients |refl_coef| to LPC coefficients |lpc_coef|.
// This version is a 16 bit operation.
//
// NOTE: The 16 bit refl_coef -> lpc_coef conversion might result in a
// "slightly unstable" filter (i.e., a pole just outside the unit circle) in
// "rare" cases even if the reflection coefficients are stable.
//
// Input:
//      - refl_coef : Reflection coefficients in Q15 that should be converted
//                    to LPC coefficients
//      - use_order : Number of coefficients in |refl_coef|
//
// Output:
//      - lpc_coef  : LPC coefficients in Q12
void WebRtcSpl_ReflCoefToLpc(G_CONST WebRtc_Word16* refl_coef,
                             int use_order,
                             WebRtc_Word16* lpc_coef);

// Converts LPC coefficients |lpc_coef| to reflection coefficients |refl_coef|.
// This version is a 16 bit operation.
// The conversion is implemented by the step-down algorithm.
//
// Input:
//      - lpc_coef  : LPC coefficients in Q12, that should be converted to
//                    reflection coefficients
//      - use_order : Number of coefficients in |lpc_coef|
//
// Output:
//      - refl_coef : Reflection coefficients in Q15.
void WebRtcSpl_LpcToReflCoef(WebRtc_Word16* lpc_coef,
                             int use_order,
                             WebRtc_Word16* refl_coef);

// Calculates reflection coefficients (16 bit) from auto-correlation values
//
// Input:
//      - auto_corr : Auto-correlation values
//      - use_order : Number of coefficients wanted be calculated
//
// Output:
//      - refl_coef : Reflection coefficients in Q15.
void WebRtcSpl_AutoCorrToReflCoef(G_CONST WebRtc_Word32* auto_corr,
                                  int use_order,
                                  WebRtc_Word16* refl_coef);

// The functions (with related pointer) calculate the cross-correlation between
// two sequences |seq1| and |seq2|.
// |seq1| is fixed and |seq2| slides as the pointer is increased with the
// amount |step_seq2|. Note the arguments should obey the relationship:
// |dim_seq| - 1 + |step_seq2| * (|dim_cross_correlation| - 1) <
//      buffer size of |seq2|
//
// Input:
//      - seq1           : First sequence (fixed throughout the correlation)
//      - seq2           : Second sequence (slides |step_vector2| for each
//                            new correlation)
//      - dim_seq        : Number of samples to use in the cross-correlation
//      - dim_cross_correlation : Number of cross-correlations to calculate (the
//                            start position for |vector2| is updated for each
//                            new one)
//      - right_shifts   : Number of right bit shifts to use. This will
//                            become the output Q-domain.
//      - step_seq2      : How many (positive or negative) steps the
//                            |vector2| pointer should be updated for each new
//                            cross-correlation value.
//
// Output:
//      - cross_correlation : The cross-correlation in Q(-right_shifts)
typedef void (*CrossCorrelation)(int32_t* cross_correlation,
                                 const int16_t* seq1,
                                 const int16_t* seq2,
                                 int16_t dim_seq,
                                 int16_t dim_cross_correlation,
                                 int16_t right_shifts,
                                 int16_t step_seq2);
extern CrossCorrelation WebRtcSpl_CrossCorrelation;
void WebRtcSpl_CrossCorrelationC(int32_t* cross_correlation,
                                 const int16_t* seq1,
                                 const int16_t* seq2,
                                 int16_t dim_seq,
                                 int16_t dim_cross_correlation,
                                 int16_t right_shifts,
                                 int16_t step_seq2);
#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
void WebRtcSpl_CrossCorrelationNeon(int32_t* cross_correlation,
                                    const int16_t* seq1,
                                    const int16_t* seq2,
                                    int16_t dim_seq,
                                    int16_t dim_cross_correlation,
                                    int16_t right_shifts,
                                    int16_t step_seq2);
#endif

// Creates (the first half of) a Hanning window. Size must be at least 1 and
// at most 512.
//
// Input:
//      - size      : Length of the requested Hanning window (1 to 512)
//
// Output:
//      - window    : Hanning vector in Q14.
void WebRtcSpl_GetHanningWindow(WebRtc_Word16* window, WebRtc_Word16 size);

// Calculates y[k] = sqrt(1 - x[k]^2) for each element of the input vector
// |in_vector|. Input and output values are in Q15.
//
// Inputs:
//      - in_vector     : Values to calculate sqrt(1 - x^2) of
//      - vector_length : Length of vector |in_vector|
//
// Output:
//      - out_vector    : Output values in Q15
void WebRtcSpl_SqrtOfOneMinusXSquared(WebRtc_Word16* in_vector,
                                      int vector_length,
                                      WebRtc_Word16* out_vector);
// End: Signal processing operations.

// Randomization functions. Implementations collected in randomization_functions.c and
// descriptions at bottom of this file.
WebRtc_UWord32 WebRtcSpl_IncreaseSeed(WebRtc_UWord32* seed);
WebRtc_Word16 WebRtcSpl_RandU(WebRtc_UWord32* seed);
WebRtc_Word16 WebRtcSpl_RandN(WebRtc_UWord32* seed);
WebRtc_Word16 WebRtcSpl_RandUArray(WebRtc_Word16* vector,
                                   WebRtc_Word16 vector_length,
                                   WebRtc_UWord32* seed);
// End: Randomization functions.

// Math functions
WebRtc_Word32 WebRtcSpl_Sqrt(WebRtc_Word32 value);
WebRtc_Word32 WebRtcSpl_SqrtFloor(WebRtc_Word32 value);

// Divisions. Implementations collected in division_operations.c and
// descriptions at bottom of this file.
WebRtc_UWord32 WebRtcSpl_DivU32U16(WebRtc_UWord32 num, WebRtc_UWord16 den);
WebRtc_Word32 WebRtcSpl_DivW32W16(WebRtc_Word32 num, WebRtc_Word16 den);
WebRtc_Word16 WebRtcSpl_DivW32W16ResW16(WebRtc_Word32 num, WebRtc_Word16 den);
WebRtc_Word32 WebRtcSpl_DivResultInQ31(WebRtc_Word32 num, WebRtc_Word32 den);
WebRtc_Word32 WebRtcSpl_DivW32HiLow(WebRtc_Word32 num, WebRtc_Word16 den_hi,
                                    WebRtc_Word16 den_low);
// End: Divisions.

WebRtc_Word32 WebRtcSpl_Energy(WebRtc_Word16* vector,
                               int vector_length,
                               int* scale_factor);

// Calculates the dot product between two (WebRtc_Word16) vectors.
//
// Input:
//      - vector1       : Vector 1
//      - vector2       : Vector 2
//      - vector_length : Number of samples used in the dot product
//      - scaling       : The number of right bit shifts to apply on each term
//                        during calculation to avoid overflow, i.e., the
//                        output will be in Q(-|scaling|)
//
// Return value         : The dot product in Q(-scaling)
int32_t WebRtcSpl_DotProductWithScale(const int16_t* vector1,
                                      const int16_t* vector2,
                                      int length,
                                      int scaling);

// Filter operations.
int WebRtcSpl_FilterAR(G_CONST WebRtc_Word16* ar_coef, int ar_coef_length,
                       G_CONST WebRtc_Word16* in_vector, int in_vector_length,
                       WebRtc_Word16* filter_state, int filter_state_length,
                       WebRtc_Word16* filter_state_low,
                       int filter_state_low_length, WebRtc_Word16* out_vector,
                       WebRtc_Word16* out_vector_low, int out_vector_low_length);

void WebRtcSpl_FilterMAFastQ12(WebRtc_Word16* in_vector,
                               WebRtc_Word16* out_vector,
                               WebRtc_Word16* ma_coef,
                               WebRtc_Word16 ma_coef_length,
                               WebRtc_Word16 vector_length);

// Performs a AR filtering on a vector in Q12
// Input:
//      - data_in            : Input samples
//      - data_out           : State information in positions
//                               data_out[-order] .. data_out[-1]
//      - coefficients       : Filter coefficients (in Q12)
//      - coefficients_length: Number of coefficients (order+1)
//      - data_length        : Number of samples to be filtered
// Output:
//      - data_out           : Filtered samples
void WebRtcSpl_FilterARFastQ12(const int16_t* data_in,
                               int16_t* data_out,
                               const int16_t* __restrict coefficients,
                               int coefficients_length,
                               int data_length);

// The functions (with related pointer) perform a MA down sampling filter
// on a vector.
// Input:
//      - data_in            : Input samples (state in positions
//                               data_in[-order] .. data_in[-1])
//      - data_in_length     : Number of samples in |data_in| to be filtered.
//                               This must be at least
//                               |delay| + |factor|*(|out_vector_length|-1) + 1)
//      - data_out_length    : Number of down sampled samples desired
//      - coefficients       : Filter coefficients (in Q12)
//      - coefficients_length: Number of coefficients (order+1)
//      - factor             : Decimation factor
//      - delay              : Delay of filter (compensated for in out_vector)
// Output:
//      - data_out           : Filtered samples
// Return value              : 0 if OK, -1 if |in_vector| is too short
typedef int (*DownsampleFast)(const int16_t* data_in,
                              int data_in_length,
                              int16_t* data_out,
                              int data_out_length,
                              const int16_t* __restrict coefficients,
                              int coefficients_length,
                              int factor,
                              int delay);
extern DownsampleFast WebRtcSpl_DownsampleFast;
int WebRtcSpl_DownsampleFastC(const int16_t* data_in,
                              int data_in_length,
                              int16_t* data_out,
                              int data_out_length,
                              const int16_t* __restrict coefficients,
                              int coefficients_length,
                              int factor,
                              int delay);
#if (defined WEBRTC_DETECT_ARM_NEON) || (defined WEBRTC_ARCH_ARM_NEON)
int WebRtcSpl_DownsampleFastNeon(const int16_t* data_in,
                                 int data_in_length,
                                 int16_t* data_out,
                                 int data_out_length,
                                 const int16_t* __restrict coefficients,
                                 int coefficients_length,
                                 int factor,
                                 int delay);
#endif

// End: Filter operations.

// FFT operations

int WebRtcSpl_ComplexFFT(WebRtc_Word16 vector[], int stages, int mode);
int WebRtcSpl_ComplexIFFT(WebRtc_Word16 vector[], int stages, int mode);

// Treat a 16-bit complex data buffer |complex_data| as an array of 32-bit
// values, and swap elements whose indexes are bit-reverses of each other.
//
// Input:
//      - complex_data  : Complex data buffer containing 2^|stages| real
//                        elements interleaved with 2^|stages| imaginary
//                        elements: [Re Im Re Im Re Im....]
//      - stages        : Number of FFT stages. Must be at least 3 and at most
//                        10, since the table WebRtcSpl_kSinTable1024[] is 1024
//                        elements long.
//
// Output:
//      - complex_data  : The complex data buffer.

void WebRtcSpl_ComplexBitReverse(int16_t* __restrict complex_data, int stages);

// End: FFT operations

/************************************************************
 *
 * RESAMPLING FUNCTIONS AND THEIR STRUCTS ARE DEFINED BELOW
 *
 ************************************************************/

/*******************************************************************
 * resample.c
 *
 * Includes the following resampling combinations
 * 22 kHz -> 16 kHz
 * 16 kHz -> 22 kHz
 * 22 kHz ->  8 kHz
 *  8 kHz -> 22 kHz
 *
 ******************************************************************/

// state structure for 22 -> 16 resampler
typedef struct
{
    WebRtc_Word32 S_22_44[8];
    WebRtc_Word32 S_44_32[8];
    WebRtc_Word32 S_32_16[8];
} WebRtcSpl_State22khzTo16khz;

void WebRtcSpl_Resample22khzTo16khz(const WebRtc_Word16* in,
                                    WebRtc_Word16* out,
                                    WebRtcSpl_State22khzTo16khz* state,
                                    WebRtc_Word32* tmpmem);

void WebRtcSpl_ResetResample22khzTo16khz(WebRtcSpl_State22khzTo16khz* state);

// state structure for 16 -> 22 resampler
typedef struct
{
    WebRtc_Word32 S_16_32[8];
    WebRtc_Word32 S_32_22[8];
} WebRtcSpl_State16khzTo22khz;

void WebRtcSpl_Resample16khzTo22khz(const WebRtc_Word16* in,
                                    WebRtc_Word16* out,
                                    WebRtcSpl_State16khzTo22khz* state,
                                    WebRtc_Word32* tmpmem);

void WebRtcSpl_ResetResample16khzTo22khz(WebRtcSpl_State16khzTo22khz* state);

// state structure for 22 -> 8 resampler
typedef struct
{
    WebRtc_Word32 S_22_22[16];
    WebRtc_Word32 S_22_16[8];
    WebRtc_Word32 S_16_8[8];
} WebRtcSpl_State22khzTo8khz;

void WebRtcSpl_Resample22khzTo8khz(const WebRtc_Word16* in, WebRtc_Word16* out,
                                   WebRtcSpl_State22khzTo8khz* state,
                                   WebRtc_Word32* tmpmem);

void WebRtcSpl_ResetResample22khzTo8khz(WebRtcSpl_State22khzTo8khz* state);

// state structure for 8 -> 22 resampler
typedef struct
{
    WebRtc_Word32 S_8_16[8];
    WebRtc_Word32 S_16_11[8];
    WebRtc_Word32 S_11_22[8];
} WebRtcSpl_State8khzTo22khz;

void WebRtcSpl_Resample8khzTo22khz(const WebRtc_Word16* in, WebRtc_Word16* out,
                                   WebRtcSpl_State8khzTo22khz* state,
                                   WebRtc_Word32* tmpmem);

void WebRtcSpl_ResetResample8khzTo22khz(WebRtcSpl_State8khzTo22khz* state);

/*******************************************************************
 * resample_fractional.c
 * Functions for internal use in the other resample functions
 *
 * Includes the following resampling combinations
 * 48 kHz -> 32 kHz
 * 32 kHz -> 24 kHz
 * 44 kHz -> 32 kHz
 *
 ******************************************************************/

void WebRtcSpl_Resample48khzTo32khz(const WebRtc_Word32* In, WebRtc_Word32* Out,
                                    const WebRtc_Word32 K);

void WebRtcSpl_Resample32khzTo24khz(const WebRtc_Word32* In, WebRtc_Word32* Out,
                                    const WebRtc_Word32 K);

void WebRtcSpl_Resample44khzTo32khz(const WebRtc_Word32* In, WebRtc_Word32* Out,
                                    const WebRtc_Word32 K);

/*******************************************************************
 * resample_48khz.c
 *
 * Includes the following resampling combinations
 * 48 kHz -> 16 kHz
 * 16 kHz -> 48 kHz
 * 48 kHz ->  8 kHz
 *  8 kHz -> 48 kHz
 *
 ******************************************************************/

typedef struct
{
    WebRtc_Word32 S_48_48[16];
    WebRtc_Word32 S_48_32[8];
    WebRtc_Word32 S_32_16[8];
} WebRtcSpl_State48khzTo16khz;

void WebRtcSpl_Resample48khzTo16khz(const WebRtc_Word16* in, WebRtc_Word16* out,
                                    WebRtcSpl_State48khzTo16khz* state,
                                    WebRtc_Word32* tmpmem);

void WebRtcSpl_ResetResample48khzTo16khz(WebRtcSpl_State48khzTo16khz* state);

typedef struct
{
    WebRtc_Word32 S_16_32[8];
    WebRtc_Word32 S_32_24[8];
    WebRtc_Word32 S_24_48[8];
} WebRtcSpl_State16khzTo48khz;

void WebRtcSpl_Resample16khzTo48khz(const WebRtc_Word16* in, WebRtc_Word16* out,
                                    WebRtcSpl_State16khzTo48khz* state,
                                    WebRtc_Word32* tmpmem);

void WebRtcSpl_ResetResample16khzTo48khz(WebRtcSpl_State16khzTo48khz* state);

typedef struct
{
    WebRtc_Word32 S_48_24[8];
    WebRtc_Word32 S_24_24[16];
    WebRtc_Word32 S_24_16[8];
    WebRtc_Word32 S_16_8[8];
} WebRtcSpl_State48khzTo8khz;

void WebRtcSpl_Resample48khzTo8khz(const WebRtc_Word16* in, WebRtc_Word16* out,
                                   WebRtcSpl_State48khzTo8khz* state,
                                   WebRtc_Word32* tmpmem);

void WebRtcSpl_ResetResample48khzTo8khz(WebRtcSpl_State48khzTo8khz* state);

typedef struct
{
    WebRtc_Word32 S_8_16[8];
    WebRtc_Word32 S_16_12[8];
    WebRtc_Word32 S_12_24[8];
    WebRtc_Word32 S_24_48[8];
} WebRtcSpl_State8khzTo48khz;

void WebRtcSpl_Resample8khzTo48khz(const WebRtc_Word16* in, WebRtc_Word16* out,
                                   WebRtcSpl_State8khzTo48khz* state,
                                   WebRtc_Word32* tmpmem);

void WebRtcSpl_ResetResample8khzTo48khz(WebRtcSpl_State8khzTo48khz* state);

/*******************************************************************
 * resample_by_2.c
 *
 * Includes down and up sampling by a factor of two.
 *
 ******************************************************************/

void WebRtcSpl_DownsampleBy2(const WebRtc_Word16* in, const WebRtc_Word16 len,
                             WebRtc_Word16* out, WebRtc_Word32* filtState);

void WebRtcSpl_UpsampleBy2(const WebRtc_Word16* in, WebRtc_Word16 len, WebRtc_Word16* out,
                           WebRtc_Word32* filtState);

/************************************************************
 * END OF RESAMPLING FUNCTIONS
 ************************************************************/
void WebRtcSpl_AnalysisQMF(const WebRtc_Word16* in_data,
                           WebRtc_Word16* low_band,
                           WebRtc_Word16* high_band,
                           WebRtc_Word32* filter_state1,
                           WebRtc_Word32* filter_state2);
void WebRtcSpl_SynthesisQMF(const WebRtc_Word16* low_band,
                            const WebRtc_Word16* high_band,
                            WebRtc_Word16* out_data,
                            WebRtc_Word32* filter_state1,
                            WebRtc_Word32* filter_state2);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // WEBRTC_SPL_SIGNAL_PROCESSING_LIBRARY_H_

#ifndef WEBRTC_SPL_SPL_INL_H_
#define WEBRTC_SPL_SPL_INL_H_

#ifdef WEBRTC_ARCH_ARM_V7
#ifndef WEBRTC_SPL_SPL_INL_ARMV7_H_
#define WEBRTC_SPL_SPL_INL_ARMV7_H_

/* TODO(kma): Replace some assembly code with GCC intrinsics
 * (e.g. __builtin_clz).
 */

/* This function produces result that is not bit exact with that by the generic
 * C version in some cases, although the former is at least as accurate as the
 * later.
 */
static __inline WebRtc_Word32 WEBRTC_SPL_MUL_16_32_RSFT16(WebRtc_Word16 a,
                                                          WebRtc_Word32 b) {
  WebRtc_Word32 tmp = 0;
  __asm __volatile ("smulwb %0, %1, %2":"=r"(tmp):"r"(b), "r"(a));
  return tmp;
}

/* This function produces result that is not bit exact with that by the generic
 * C version in some cases, although the former is at least as accurate as the
 * later.
 */
static __inline WebRtc_Word32 WEBRTC_SPL_MUL_32_32_RSFT32(WebRtc_Word16 a,
                                                          WebRtc_Word16 b,
                                                          WebRtc_Word32 c) {
  WebRtc_Word32 tmp = 0;
  __asm __volatile (
    "pkhbt %[tmp], %[b], %[a], lsl #16\n\t"
    "smmulr %[tmp], %[tmp], %[c]\n\t"
    :[tmp]"+r"(tmp)
    :[a]"r"(a),
     [b]"r"(b),
     [c]"r"(c)
  );
  return tmp;
}

static __inline WebRtc_Word32 WEBRTC_SPL_MUL_32_32_RSFT32BI(WebRtc_Word32 a,
                                                            WebRtc_Word32 b) {
  WebRtc_Word32 tmp = 0;
  __asm volatile ("smmulr %0, %1, %2":"=r"(tmp):"r"(a), "r"(b));
  return tmp;
}

static __inline WebRtc_Word32 WEBRTC_SPL_MUL_16_16(WebRtc_Word16 a,
                                                   WebRtc_Word16 b) {
  WebRtc_Word32 tmp = 0;
  __asm __volatile ("smulbb %0, %1, %2":"=r"(tmp):"r"(a), "r"(b));
  return tmp;
}

// TODO(kma): add unit test.
static __inline int32_t WebRtc_MulAccumW16(int16_t a,
                                          int16_t b,
                                          int32_t c) {
  int32_t tmp = 0;
  __asm __volatile ("smlabb %0, %1, %2, %3":"=r"(tmp):"r"(a), "r"(b), "r"(c));
  return tmp;
}

static __inline WebRtc_Word16 WebRtcSpl_AddSatW16(WebRtc_Word16 a,
                                                  WebRtc_Word16 b) {
  WebRtc_Word32 s_sum = 0;

  __asm __volatile ("qadd16 %0, %1, %2":"=r"(s_sum):"r"(a), "r"(b));

  return (WebRtc_Word16) s_sum;
}

/* TODO(kma): find the cause of unittest errors by the next two functions:
 * http://code.google.com/p/webrtc/issues/detail?id=740.
 */
#if 0
static __inline WebRtc_Word32 WebRtcSpl_AddSatW32(WebRtc_Word32 l_var1,
                                                  WebRtc_Word32 l_var2) {
  WebRtc_Word32 l_sum = 0;

  __asm __volatile ("qadd %0, %1, %2":"=r"(l_sum):"r"(l_var1), "r"(l_var2));

  return l_sum;
}

static __inline WebRtc_Word32 WebRtcSpl_SubSatW32(WebRtc_Word32 l_var1,
                                                  WebRtc_Word32 l_var2) {
  WebRtc_Word32 l_sub = 0;

  __asm __volatile ("qsub %0, %1, %2":"=r"(l_sub):"r"(l_var1), "r"(l_var2));

  return l_sub;
}
#endif

static __inline WebRtc_Word16 WebRtcSpl_SubSatW16(WebRtc_Word16 var1,
                                                  WebRtc_Word16 var2) {
  WebRtc_Word32 s_sub = 0;

  __asm __volatile ("qsub16 %0, %1, %2":"=r"(s_sub):"r"(var1), "r"(var2));

  return (WebRtc_Word16)s_sub;
}

static __inline WebRtc_Word16 WebRtcSpl_GetSizeInBits(WebRtc_UWord32 n) {
  WebRtc_Word32 tmp = 0;

  __asm __volatile ("clz %0, %1":"=r"(tmp):"r"(n));

  return (WebRtc_Word16)(32 - tmp);
}

static __inline int WebRtcSpl_NormW32(WebRtc_Word32 a) {
  WebRtc_Word32 tmp = 0;

  if (a == 0) {
    return 0;
  }
  else if (a < 0) {
    a ^= 0xFFFFFFFF;
  }

  __asm __volatile ("clz %0, %1":"=r"(tmp):"r"(a));

  return tmp - 1;
}

static __inline int WebRtcSpl_NormU32(WebRtc_UWord32 a) {
  int tmp = 0;

  if (a == 0) return 0;

  __asm __volatile ("clz %0, %1":"=r"(tmp):"r"(a));

  return tmp;
}

static __inline int WebRtcSpl_NormW16(WebRtc_Word16 a) {
  WebRtc_Word32 tmp = 0;

  if (a == 0) {
    return 0;
  }
  else if (a < 0) {
    a ^= 0xFFFFFFFF;
  }

  __asm __volatile ("clz %0, %1":"=r"(tmp):"r"(a));

  return tmp - 17;
}

// TODO(kma): add unit test.
static __inline WebRtc_Word16 WebRtcSpl_SatW32ToW16(WebRtc_Word32 value32) {
  WebRtc_Word16 out16 = 0;

  __asm __volatile ("ssat %0, #16, %1" : "=r"(out16) : "r"(value32));

  return out16;
}

#endif  // WEBRTC_SPL_SPL_INL_ARMV7_H_
#else

static __inline WebRtc_Word16 WebRtcSpl_SatW32ToW16(WebRtc_Word32 value32) {
  WebRtc_Word16 out16 = (WebRtc_Word16) value32;

  if (value32 > 32767)
    out16 = 32767;
  else if (value32 < -32768)
    out16 = -32768;

  return out16;
}

static __inline WebRtc_Word16 WebRtcSpl_AddSatW16(WebRtc_Word16 a,
                                                  WebRtc_Word16 b) {
  return WebRtcSpl_SatW32ToW16((WebRtc_Word32) a + (WebRtc_Word32) b);
}

static __inline WebRtc_Word16 WebRtcSpl_SubSatW16(WebRtc_Word16 var1,
                                                  WebRtc_Word16 var2) {
  return WebRtcSpl_SatW32ToW16((WebRtc_Word32) var1 - (WebRtc_Word32) var2);
}

static __inline WebRtc_Word16 WebRtcSpl_GetSizeInBits(WebRtc_UWord32 n) {
  int bits;

  if (0xFFFF0000 & n) {
    bits = 16;
  } else {
    bits = 0;
  }
  if (0x0000FF00 & (n >> bits)) bits += 8;
  if (0x000000F0 & (n >> bits)) bits += 4;
  if (0x0000000C & (n >> bits)) bits += 2;
  if (0x00000002 & (n >> bits)) bits += 1;
  if (0x00000001 & (n >> bits)) bits += 1;

  return bits;
}

static __inline int WebRtcSpl_NormW32(WebRtc_Word32 a) {
  int zeros;

  if (a == 0) {
    return 0;
  }
  else if (a < 0) {
    a = ~a;
  }

  if (!(0xFFFF8000 & a)) {
    zeros = 16;
  } else {
    zeros = 0;
  }
  if (!(0xFF800000 & (a << zeros))) zeros += 8;
  if (!(0xF8000000 & (a << zeros))) zeros += 4;
  if (!(0xE0000000 & (a << zeros))) zeros += 2;
  if (!(0xC0000000 & (a << zeros))) zeros += 1;

  return zeros;
}

static __inline int WebRtcSpl_NormU32(WebRtc_UWord32 a) {
  int zeros;

  if (a == 0) return 0;

  if (!(0xFFFF0000 & a)) {
    zeros = 16;
  } else {
    zeros = 0;
  }
  if (!(0xFF000000 & (a << zeros))) zeros += 8;
  if (!(0xF0000000 & (a << zeros))) zeros += 4;
  if (!(0xC0000000 & (a << zeros))) zeros += 2;
  if (!(0x80000000 & (a << zeros))) zeros += 1;

  return zeros;
}

static __inline int WebRtcSpl_NormW16(WebRtc_Word16 a) {
  int zeros;

  if (a == 0) {
    return 0;
  }
  else if (a < 0) {
    a = ~a;
  }

  if (!(0xFF80 & a)) {
    zeros = 8;
  } else {
    zeros = 0;
  }
  if (!(0xF800 & (a << zeros))) zeros += 4;
  if (!(0xE000 & (a << zeros))) zeros += 2;
  if (!(0xC000 & (a << zeros))) zeros += 1;

  return zeros;
}

static __inline int32_t WebRtc_MulAccumW16(int16_t a,
                                          int16_t b,
                                          int32_t c) {
  return (a * b + c);
}

#endif  // WEBRTC_ARCH_ARM_V7

// The following functions have no optimized versions.
// TODO(kma): Consider saturating add/sub instructions in X86 platform.
static __inline WebRtc_Word32 WebRtcSpl_AddSatW32(WebRtc_Word32 l_var1,
                                                  WebRtc_Word32 l_var2) {
  WebRtc_Word32 l_sum;

  // Perform long addition
  l_sum = l_var1 + l_var2;

  if (l_var1 < 0) {  // Check for underflow.
    if ((l_var2 < 0) && (l_sum >= 0)) {
        l_sum = (WebRtc_Word32)0x80000000;
    }
  } else {  // Check for overflow.
    if ((l_var2 > 0) && (l_sum < 0)) {
        l_sum = (WebRtc_Word32)0x7FFFFFFF;
    }
  }

  return l_sum;
}

static __inline WebRtc_Word32 WebRtcSpl_SubSatW32(WebRtc_Word32 l_var1,
                                                  WebRtc_Word32 l_var2) {
  WebRtc_Word32 l_diff;

  // Perform subtraction.
  l_diff = l_var1 - l_var2;

  if (l_var1 < 0) {  // Check for underflow.
    if ((l_var2 > 0) && (l_diff > 0)) {
      l_diff = (WebRtc_Word32)0x80000000;
    }
  } else {  // Check for overflow.
    if ((l_var2 < 0) && (l_diff < 0)) {
      l_diff = (WebRtc_Word32)0x7FFFFFFF;
    }
  }

  return l_diff;
}

#endif  // WEBRTC_SPL_SPL_INL_H_

#ifndef WEBRTC_COMMON_AUDIO_VAD_VAD_CORE_H_
#define WEBRTC_COMMON_AUDIO_VAD_VAD_CORE_H_

enum { kNumChannels = 6 };  // Number of frequency bands (named channels).
enum { kNumGaussians = 2 };  // Number of Gaussians per channel in the GMM.
enum { kTableSize = kNumChannels * kNumGaussians };
enum { kMinEnergy = 10 };  // Minimum energy required to trigger audio signal.

typedef struct VadInstT_
{

    int vad;
    int32_t downsampling_filter_states[4];
    WebRtcSpl_State48khzTo8khz state_48_to_8;
    int16_t noise_means[kTableSize];
    int16_t speech_means[kTableSize];
    int16_t noise_stds[kTableSize];
    int16_t speech_stds[kTableSize];
    // TODO(bjornv): Change to |frame_count|.
    int32_t frame_counter;
    int16_t over_hang; // Over Hang
    int16_t num_of_speech;
    // TODO(bjornv): Change to |age_vector|.
    int16_t index_vector[16 * kNumChannels];
    int16_t low_value_vector[16 * kNumChannels];
    // TODO(bjornv): Change to |median|.
    int16_t mean_value[kNumChannels];
    int16_t upper_state[5];
    int16_t lower_state[5];
    int16_t hp_filter_state[4];
    int16_t over_hang_max_1[3];
    int16_t over_hang_max_2[3];
    int16_t individual[3];
    int16_t total[3];

    int init_flag;

} VadInstT;

// returns      : 0 (OK), -1 (NULL pointer in or if the default mode can't be
//                set)
int WebRtcVad_InitCore(VadInstT* self);

/****************************************************************************
 * WebRtcVad_set_mode_core(...)
 *
 * Return value     :  0 - Ok
 *                    -1 - Error
 */

int WebRtcVad_set_mode_core(VadInstT* self, int mode);

/****************************************************************************
 * WebRtcVad_CalcVad48khz(...)
 * WebRtcVad_CalcVad32khz(...) 
 * WebRtcVad_CalcVad16khz(...) 
 * WebRtcVad_CalcVad8khz(...) 
 *
 * Return value         : VAD decision
 *                        0 - No active speech
 *                        1-6 - Active speech
 */
int WebRtcVad_CalcVad48khz(VadInstT* inst, int16_t* speech_frame,
                           int frame_length);
int WebRtcVad_CalcVad32khz(VadInstT* inst, int16_t* speech_frame,
                           int frame_length);
int WebRtcVad_CalcVad16khz(VadInstT* inst, int16_t* speech_frame,
                           int frame_length);
int WebRtcVad_CalcVad8khz(VadInstT* inst, int16_t* speech_frame,
                          int frame_length);

#endif  // WEBRTC_COMMON_AUDIO_VAD_VAD_CORE_H_

#ifndef WEBRTC_COMMON_AUDIO_VAD_VAD_FILTERBANK_H_
#define WEBRTC_COMMON_AUDIO_VAD_VAD_FILTERBANK_H_

// Takes |data_length| samples of |data_in| and calculates the logarithm of the
// energy of each of the |kNumChannels| = 6 frequency bands used by the VAD:
//        80 Hz - 250 Hz
//        250 Hz - 500 Hz
//        500 Hz - 1000 Hz
//        1000 Hz - 2000 Hz
//        2000 Hz - 3000 Hz
//        3000 Hz - 4000 Hz
//
// The values are given in Q4 and written to |features|. Further, an approximate
// overall energy is returned. The return value is used in
// WebRtcVad_GmmProbability() as a signal indicator, hence it is arbitrary above
// the threshold |kMinEnergy|.
//
// - self         [i/o] : State information of the VAD.
// - data_in      [i]   : Input audio data, for feature extraction.
// - data_length  [i]   : Audio data size, in number of samples.
// - features     [o]   : 10 * log10(energy in each frequency band), Q4.
// - returns            : Total energy of the signal (NOTE! This value is not
//                        exact. It is only used in a comparison.)
int16_t WebRtcVad_CalculateFeatures(VadInstT* self, const int16_t* data_in,
                                    int data_length, int16_t* features);

#endif  // WEBRTC_COMMON_AUDIO_VAD_VAD_FILTERBANK_H_


#ifndef WEBRTC_COMMON_AUDIO_VAD_VAD_GMM_H_
#define WEBRTC_COMMON_AUDIO_VAD_VAD_GMM_H_

// Return:
//   (probability for |input|) =
//    1 / |std| * exp(-(|input| - |mean|)^2 / (2 * |std|^2));
int32_t WebRtcVad_GaussianProbability(int16_t input,
                                      int16_t mean,
                                      int16_t std,
                                      int16_t* delta);

#endif  // WEBRTC_COMMON_AUDIO_VAD_VAD_GMM_H_

#ifndef WEBRTC_COMMON_AUDIO_VAD_VAD_SP_H_
#define WEBRTC_COMMON_AUDIO_VAD_VAD_SP_H_

// Downsamples the signal by a factor 2, eg. 32->16 or 16->8.
// Output:
//      - signal_out    : Downsampled signal (of length |in_length| / 2).
void WebRtcVad_Downsampling(int16_t* signal_in,
                            int16_t* signal_out,
                            int32_t* filter_state,
                            int in_length);

// Updates and returns the smoothed feature minimum. As minimum we use the
// median of the five smallest feature values in a 100 frames long window.
// As long as |handle->frame_counter| is zero, that is, we haven't received any
// "valid" data, FindMinimum() outputs the default value of 1600.
//
// Inputs:
//      - feature_value : New feature value to update with.
//      - channel       : Channel number.
//
// Input & Output:
//      - handle        : State information of the VAD.
//
// Returns:
//                      : Smoothed minimum value for a moving window.
int16_t WebRtcVad_FindMinimum(VadInstT* handle,
                              int16_t feature_value,
                              int channel);

#endif  // WEBRTC_COMMON_AUDIO_VAD_VAD_SP_H_

/**
 * handle 结果地址
 * kFrameLengths 帧长 { 80, 120, 160, 240, 320, 480, 640, 960 }
 * kRates 处理频率（不是音频的采样频率） { 8000, 12000, 16000, 24000, 32000, 48000 }
 * datas[] 音频数据 长度是kFrameLengths
 * return -1 ERROR, 0 NO VOICE, 1 VOICE
 */
int vad_main_process(VadInst* handle, int kFrameLengths, int kRates, short datas[]);

