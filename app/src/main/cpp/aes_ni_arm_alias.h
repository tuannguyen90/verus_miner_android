//
// Created by tuann on 4/9/2025.
//

#ifndef MYNEON_AES_NI_ARM_ALIAS_H
#define MYNEON_AES_NI_ARM_ALIAS_H

#if defined(__aarch64__) || defined(__ARM_NEON) || defined(__ARM_NEON__)

// Đảm bảo dùng NEON và Crypto extensions
#include <arm_neon.h>
#include <arm_acle.h>

typedef uint8x16_t __m128i;

// Đã test
__attribute__((target("crypto")))
static inline __m128i _mm_aesenc_si128(__m128i state, __m128i round_key) {
    // Giống Intel: SubBytes → ShiftRows → MixColumns → AddRoundKey
    state = vaeseq_u8(state, vdupq_n_u8(0));     // SubBytes + ShiftRows
    state = vaesmcq_u8(state);                   // MixColumns
    return veorq_u8(state, round_key);           // AddRoundKey
}

static inline __m128i _mm_aesenclast_si128(__m128i state, __m128i round_key) {
    state = veorq_u8(state, round_key);         // XOR trước
    return vaeseq_u8(state, vdupq_n_u8(0));     // Không có MixColumns ở vòng cuối
}

static inline __m128i _mm_aesdec_si128(__m128i state, __m128i round_key) {
    return vaesimcq_u8(vaesdq_u8(state, round_key));
}

static inline __m128i _mm_aesdeclast_si128(__m128i state, __m128i round_key) {
    return vaesdq_u8(state, round_key);
}

// Đã test
static inline __m128i _mm_load_si128(const __m128i* p) {
    return vld1q_u8((const uint8_t*)p);
}

// Đã test
static inline void _mm_storeu_si128(__m128i* p, __m128i a) {
    vst1q_u8((uint8_t*)p, a);
}

// Đã test
static inline __m128i _mm_unpacklo_epi32(__m128i a, __m128i b) {
    return vzip1q_u32(vreinterpretq_u32_u8(a), vreinterpretq_u32_u8(b));
}

// đã test
static inline __m128i _mm_unpackhi_epi32(__m128i a, __m128i b) {
    return vzip2q_u32(vreinterpretq_u32_u8(a), vreinterpretq_u32_u8(b));
}

// Đã test
static inline __m128i _mm_set_epi32(int e3, int e2, int e1, int e0) {
    int32x4_t v = { e0, e1, e2, e3 }; // NEON sử dụng thứ tự ngược so với SSE
    return vreinterpretq_u8_s32(v);  // ép kiểu sang __m128i (uint8x16_t)
}

// Đã test
static inline __m128i _mm_xor_si128(__m128i a, __m128i b) {
    return veorq_u8(a, b);
}

// Đã test
static inline __m128i _mm_set_epi64x(int64_t high, int64_t low) {
    int64x2_t v = { low, high }; // NEON xếp [low, high]
    return vreinterpretq_u8_s64(v); // ép sang kiểu __m128i (uint8x16_t)
}

// Đã test
__attribute__((target("crypto")))
static inline __m128i _mm_clmulepi64_si128(__m128i a, __m128i b, const int imm) {
    uint64x2_t va = vreinterpretq_u64_u8(a);
    uint64x2_t vb = vreinterpretq_u64_u8(b);

    uint64x1_t a_part = (imm & 0x10) ? vget_high_u64(va) : vget_low_u64(va);
    uint64x1_t b_part = (imm & 0x01) ? vget_high_u64(vb) : vget_low_u64(vb);

    poly128_t result = vmull_p64(a_part[0], b_part[0]);  // nhân theo GF(2^64)

    return vreinterpretq_u8_p128(result);  // cast sang __m128i (uint8x16_t)
}

// Đã test
// 64-bit int to __m128i
static inline __m128i _mm_cvtsi64_si128(int64_t x) {
    uint64x2_t v = { (uint64_t)x, 0 };
    return vreinterpretq_u8_u64(v);
}

// Đã test
// __m128i to 64-bit int (get low 64 bits)
static inline int64_t _mm_cvtsi128_si64(__m128i a) {
    return (int64_t)vgetq_lane_u64(vreinterpretq_u64_u8(a), 0);
}

// Đã test
// Set reversed 16 bytes
static inline __m128i _mm_setr_epi8(
        char b0, char b1, char b2, char b3,
        char b4, char b5, char b6, char b7,
        char b8, char b9, char b10, char b11,
        char b12, char b13, char b14, char b15
) {
    uint8_t data[16] = {
            b0, b1, b2, b3, b4, b5, b6, b7,
            b8, b9, b10, b11, b12, b13, b14, b15
    };
    return vld1q_u8(data);
}

// Đã test
// Logical shift right by bytes
static inline __m128i _mm_srli_si128(__m128i a, const int imm) {
    switch (imm) {
        case 0:  return a;
        case 1:  return vextq_u8(a, vdupq_n_u8(0), 1);
        case 2:  return vextq_u8(a, vdupq_n_u8(0), 2);
        case 3:  return vextq_u8(a, vdupq_n_u8(0), 3);
        case 4:  return vextq_u8(a, vdupq_n_u8(0), 4);
        case 5:  return vextq_u8(a, vdupq_n_u8(0), 5);
        case 6:  return vextq_u8(a, vdupq_n_u8(0), 6);
        case 7:  return vextq_u8(a, vdupq_n_u8(0), 7);
        case 8:  return vextq_u8(a, vdupq_n_u8(0), 8);
        case 9:  return vextq_u8(a, vdupq_n_u8(0), 9);
        case 10: return vextq_u8(a, vdupq_n_u8(0), 10);
        case 11: return vextq_u8(a, vdupq_n_u8(0), 11);
        case 12: return vextq_u8(a, vdupq_n_u8(0), 12);
        case 13: return vextq_u8(a, vdupq_n_u8(0), 13);
        case 14: return vextq_u8(a, vdupq_n_u8(0), 14);
        case 15: return vextq_u8(a, vdupq_n_u8(0), 15);
        case 16: return vdupq_n_u8(0); // all zero
        default: return a;
    }
}


// Đã test
// Store aligned
static inline void _mm_store_si128(__m128i* p, __m128i a) {
    vst1q_u8((uint8_t*)p, a);
}

// Đã test
// Multiply and round (approximate)
static inline __m128i _mm_mulhrs_epi16(__m128i a, __m128i b) {
    int16x8_t va = vreinterpretq_s16_u8(a);
    int16x8_t vb = vreinterpretq_s16_u8(b);

    int32x4_t mul_lo = vmull_s16(vget_low_s16(va), vget_low_s16(vb));
    int32x4_t mul_hi = vmull_s16(vget_high_s16(va), vget_high_s16(vb));

    // Round + shift right 15 bits
    int32x4_t r_lo = vshrq_n_s32(vaddq_s32(mul_lo, vdupq_n_s32(0x4000)), 15);
    int32x4_t r_hi = vshrq_n_s32(vaddq_s32(mul_hi, vdupq_n_s32(0x4000)), 15);

    // ❗ Use vmovn_s32 (no saturation) instead of vqmovn_s32
    int16x8_t result = vcombine_s16(vmovn_s32(r_lo), vmovn_s32(r_hi));

    return vreinterpretq_u8_s16(result);
}

// Đã test
__attribute__((target("crypto")))
static inline __m128i _mm_shuffle_epi8(__m128i a, __m128i mask) {
    uint8x16_t input = a;
    uint8x16_t index = mask;
    uint8x16_t selector = vandq_u8(index, vdupq_n_u8(0x0F));
    uint8x16_t zero_mask = vtstq_u8(vandq_u8(index, vdupq_n_u8(0x80)), vdupq_n_u8(0x80));
    uint8x16_t shuffled = vqtbl1q_u8(input, selector);
    return vbicq_u8(shuffled, zero_mask); // zero out where high bit was set
}

// Đã test
static inline __m128i _mm_cvtsi32_si128(int32_t x) {
    int32x4_t v = { x, 0, 0, 0 };
    return vreinterpretq_u8_s32(v);
}

// Đã test
static inline __m128i _mm_loadl_epi64(const __m128i* p) {
    uint64x1_t low = vld1_u64((const uint64_t*)p); // load 64-bit (8 byte)
    uint64x1_t zero = vdup_n_u64(0);
    return vreinterpretq_u8_u64(vcombine_u64(low, zero)); // kết hợp thành 128-bit
}

// Đã test
static inline __m128i _mm_loadu_si128(const __m128i* p) {
    return vld1q_u8((const uint8_t*)p); // unaligned load
}

static inline __m128i _mm_set1_epi8(int8_t x) {
    return vreinterpretq_u8_s8(vdupq_n_s8(x));
}


#endif

#endif //MYNEON_AES_NI_ARM_ALIAS_H
