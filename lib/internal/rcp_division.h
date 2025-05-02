#pragma once

#ifndef CSTL_RCP_DIVISION_H
#define CSTL_RCP_DIVISION_H

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

// Adapted from "Changing Division by a Constant to Multiplication in Two’s Complement Arithmetic" Henry S. Warren, Jr. 1992.
static inline size_t CSTL_fixed_point_euclid_rcp(size_t divisor, uint8_t* shift) {
    *shift = 0;

    const size_t sz_width = sizeof(size_t) * CHAR_BIT;
	const size_t sz_half  = SIZE_MAX / 2;
    const size_t max_pos  = sz_width * 2;

    if (divisor > sz_half) {
        return SIZE_MAX;
    }

    // Largest dividend such that dividend % divisor = divisor - 1:
    size_t dividend = SIZE_MAX - (~divisor + 1) % divisor;

    size_t first_quotient  = (sz_half + 1) / dividend;
    size_t first_remainder = (sz_half + 1) - first_quotient * dividend;

    size_t second_quotient  = sz_half / divisor;
    size_t second_remainder = sz_half - second_quotient * divisor;

    unsigned int pos = (unsigned int)sz_width - 1;

    size_t delta;
    do {
        ++pos;

        if (first_remainder >= dividend - first_remainder) {
            first_quotient  = 2 * first_quotient + 1;
            first_remainder = 2 * first_remainder - dividend;
        } else {
            first_quotient  = 2 * first_quotient;
            first_remainder = 2 * first_remainder;
        }
        if (second_remainder + 1 >= divisor - second_remainder) {
            second_quotient  = 2 * second_quotient + 1;
            second_remainder = 2 * second_remainder - divisor + 1;
        } else {
            second_quotient  = 2 * second_quotient;
            second_remainder = 2 * second_remainder + 1;
        }

        delta = divisor - second_remainder - 1;
    } while (pos < max_pos && (first_quotient < delta
        || (first_quotient == delta && first_remainder == 0)));

    *shift = pos - sz_width;
    return second_quotient + 1;
}

#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#endif

static inline size_t CSTL_divide_by_rcp(size_t dividend, size_t rcp, uint8_t shift) {
#if defined(_WIN64)
    return __umulh(dividend, rcp) >> shift;
#elif defined(_WIN32)
    return __emulu(dividend, rcp) >> shift;
#elif defined(__x86_64__) || defined(__LP64__)
    size_t hi = (size_t)(((unsigned __int128)dividend * (unsigned __int128)rcp) >> 64);
    return hi >> shift;
#else
    size_t hi = (size_t)(((uint64_t)dividend * (uint64_t)rcp) >> 32);
    return hi >> shift;
#endif
}

static inline ptrdiff_t CSTL_divide_by_rcp_signed(ptrdiff_t dividend, size_t rcp, uint8_t shift) {
    size_t abs_dividend = dividend < 0 ? -dividend : dividend;
    size_t abs_result = CSTL_divide_by_rcp(abs_dividend, rcp, shift);
    return dividend < 0 ? -(ptrdiff_t)abs_result : (ptrdiff_t)abs_result;
}

static inline size_t CSTL_remainder_by_rcp(size_t dividend, size_t rcp, uint8_t shift, size_t divisor) {
    size_t quotient = CSTL_divide_by_rcp(dividend, rcp, shift);

    return dividend - quotient * divisor;
}

#endif
