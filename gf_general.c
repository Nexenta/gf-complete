/*
 * gf_general.c
 *
 * This file has helper routines for doing basic GF operations with any
 * legal value of w.  The problem is that w <= 32, w=64 and w=128 all have
 * different data types, which is a pain.  The procedures in this file try
 * to alleviate that pain.  They are used in gf_unit and gf_time.
 */

#include <stdio.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "gf_complete.h"
#include "gf_int.h"
#include "gf_method.h"
#include "gf_rand.h"
#include "gf_general.h"

void gf_general_set_zero(gf_general_t *v, int w)
{
  if (w <= 32) {
    v->w32 = 0;
  } else if (w <= 64) {
    v->w64 = 0;
  } else {
    v->w128[0] = 0;
    v->w128[1] = 0;
  }
}

void gf_general_set_one(gf_general_t *v, int w)
{
  if (w <= 32) {
    v->w32 = 1;
  } else if (w <= 64) {
    v->w64 = 1;
  } else {
    v->w128[0] = 0;
    v->w128[1] = 1;
  }
}

void gf_general_set_two(gf_general_t *v, int w)
{
  if (w <= 32) {
    v->w32 = 2;
  } else if (w <= 64) {
    v->w64 = 2;
  } else {
    v->w128[0] = 0;
    v->w128[1] = 2;
  }
}

int gf_general_is_zero(gf_general_t *v, int w) 
{
  if (w <= 32) {
    return (v->w32 == 0);
  } else if (w <= 64) {
    return (v->w64 == 0);
  } else {
    return (v->w128[0] == 0 && v->w128[1] == 0);
  }
}

int gf_general_is_one(gf_general_t *v, int w) 
{
  if (w <= 32) {
    return (v->w32 == 1);
  } else if (w <= 64) {
    return (v->w64 == 1);
  } else {
    return (v->w128[0] == 0 && v->w128[1] == 1);
  }
}

void gf_general_set_random(gf_general_t *v, int w, int zero_ok) 
{
  if (w <= 32) {
      v->w32 = MOA_Random_W(w, zero_ok);
  } else if (w <= 64) {
    while (1) {
      v->w64 = MOA_Random_64();
      if (v->w64 != 0 || zero_ok) return;
    }
  } else {
    while (1) {
      MOA_Random_128(v->w128);
      if (v->w128[0] != 0 || v->w128[1] != 0 || zero_ok) return;
    }
  }
}

void gf_general_val_to_s(gf_general_t *v, int w, char *s)
{
  if (w <= 32) {
    sprintf(s, "%x", v->w32);
  } else if (w <= 64) {
    sprintf(s, "%llx", (long long unsigned int) v->w64);
  } else {
    if (v->w128[0] == 0) {
      sprintf(s, "%llx", (long long unsigned int) v->w128[1]);
    } else {
      sprintf(s, "%llx%016llx", (long long unsigned int) v->w128[0], 
                                (long long unsigned int) v->w128[1]);
    }
  }
}

void gf_general_multiply(gf_t *gf, gf_general_t *a, gf_general_t *b, gf_general_t *c)
{
  gf_internal_t *h;
  int w;

  h = (gf_internal_t *) gf->scratch;
  w = h->w;

  if (w <= 32) {
    c->w32 = gf->multiply.w32(gf, a->w32, b->w32);
  } else if (w <= 64) {
    c->w64 = gf->multiply.w64(gf, a->w64, b->w64);
  } else {
    gf->multiply.w128(gf, a->w128, b->w128, c->w128);
  }
}
  
void gf_general_divide(gf_t *gf, gf_general_t *a, gf_general_t *b, gf_general_t *c)
{
  gf_internal_t *h;
  int w;

  h = (gf_internal_t *) gf->scratch;
  w = h->w;

  if (w <= 32) {
    c->w32 = gf->divide.w32(gf, a->w32, b->w32);
  } else if (w <= 64) {
    c->w64 = gf->divide.w64(gf, a->w64, b->w64);
  } else {
    gf->divide.w128(gf, a->w128, b->w128, c->w128);
  }
}
  
void gf_general_inverse(gf_t *gf, gf_general_t *a, gf_general_t *b)
{
  gf_internal_t *h;
  int w;

  h = (gf_internal_t *) gf->scratch;
  w = h->w;

  if (w <= 32) {
    b->w32 = gf->inverse.w32(gf, a->w32);
  } else if (w <= 64) {
    b->w64 = gf->inverse.w64(gf, a->w64);
  } else {
    gf->inverse.w128(gf, a->w128, b->w128);
  }
}
  
int gf_general_are_equal(gf_general_t *v1, gf_general_t *v2, int w)
{
  if (w <= 32) {
    return (v1->w32 == v2->w32);
  } else if (w <= 64) {
    return (v1->w64 == v2->w64);
  } else {
    return (v1->w128[0] == v2->w128[0] &&
            v1->w128[0] == v2->w128[0]);
  }
}

void gf_general_do_region_multiply(gf_t *gf, gf_general_t *a, void *ra, void *rb, int bytes, int xor)
{
  gf_internal_t *h;
  int w;

  h = (gf_internal_t *) gf->scratch;
  w = h->w;

  if (w <= 32) {
    gf->multiply_region.w32(gf, ra, rb, a->w32, bytes, xor);
  } else if (w <= 64) {
    gf->multiply_region.w64(gf, ra, rb, a->w64, bytes, xor);
  } else {
    gf->multiply_region.w128(gf, ra, rb, a->w128, bytes, xor);
  }
}

void gf_general_do_region_check(gf_t *gf, gf_general_t *a, void *orig_a, void *orig_target, void *final_target, int bytes, int xor)
{
  gf_internal_t *h;
  int w, words, i;
  gf_general_t oa, ot, ft, sb;
  char sa[50], soa[50], sot[50], sft[50], ssb[50];
  uint8_t *p;

  h = (gf_internal_t *) gf->scratch;
  w = h->w;

  words = (bytes * 8) / w;
  for (i = 0; i < words; i++) {
    if (w <= 32) {
      oa.w32 = gf->extract_word.w32(gf, orig_a, bytes, i);
      ot.w32 = gf->extract_word.w32(gf, orig_target, bytes, i);
      ft.w32 = gf->extract_word.w32(gf, final_target, bytes, i);
      sb.w32 = gf->multiply.w32(gf, a->w32, oa.w32);
      if (xor) sb.w32 ^= ot.w32;
    } else if (w <= 64) {
      oa.w64 = gf->extract_word.w64(gf, orig_a, bytes, i);
      ot.w64 = gf->extract_word.w64(gf, orig_target, bytes, i);
      ft.w64 = gf->extract_word.w64(gf, final_target, bytes, i);
      sb.w64 = gf->multiply.w64(gf, a->w64, oa.w64);
      if (xor) sb.w64 ^= ot.w64;
    } else {
      gf->extract_word.w128(gf, orig_a, bytes, i, oa.w128);
      gf->extract_word.w128(gf, orig_target, bytes, i, ot.w128);
      gf->extract_word.w128(gf, final_target, bytes, i, ft.w128);
      gf->multiply.w128(gf, a->w128, oa.w128, sb.w128);
      if (xor) {
        sb.w128[0] ^= ot.w128[0];
        sb.w128[1] ^= ot.w128[1];
      }
    }

    if (!gf_general_are_equal(&ft, &sb, w)) {
      
      printf("Problem with region multiply (all values in hex):\n");
      printf("   Target address base: 0x%lx.  Word 0x%x of 0x%x.  Xor: %d\n", 
                 (unsigned long) final_target, i, words, xor);
      gf_general_val_to_s(a, w, sa);
      gf_general_val_to_s(&oa, w, soa);
      gf_general_val_to_s(&ot, w, sot);
      gf_general_val_to_s(&ft, w, sft);
      gf_general_val_to_s(&sb, w, ssb);
      printf("   Value: %s\n", sa);
      printf("   Original source word: %s\n", soa);
      if (xor) printf("   XOR with target word: %s\n", sot);
      printf("   Product word: %s\n", sft);
      printf("   It should be: %s\n", ssb);
      exit(0);
    }
  }
}

void gf_general_set_up_single_timing_test(int w, void *ra, void *rb, int size)
{
  uint32_t *r32;
  int i;

  /* If w is 8, 16, 32, 64 or 128, this is easy -- 
     just fill the regions with random bytes.

     Otherwise, treat every four bytes as an uint32_t
     and fill it with a random value mod (1 << w).
   */

  if (w == 8 || w == 16 || w == 32 || w == 64 || w == 128) {
    MOA_Fill_Random_Region (ra, size);
    MOA_Fill_Random_Region (rb, size);
  } else {
    r32 = (uint32_t *) ra;
    for (i = 0; i < size/4; i++) r32[i] = MOA_Random_W(w, 1);
    r32 = (uint32_t *) rb;
    for (i = 0; i < size/4; i++) r32[i] = MOA_Random_W(w, 0);
  }
}

/* This sucks, but in order to time, you really need to avoid putting ifs in 
   the inner loops.  So, I'm doing a separate timing test for each w: 
   8, 16, 32, 64, 128 and everything else.  Fortunately, the "everything else"
   tests can be equivalent to w=32.

   I'm also putting the results back into ra, because otherwise, the optimizer might
   figure out that we're not really doing anything in the inner loops and it 
   will chuck that. */

int gf_general_do_single_timing_test(gf_t *gf, void *ra, void *rb, int size, char test)
{
  gf_internal_t *h;
  void *top;
  uint8_t *r8a, *r8b, *top8;
  uint16_t *r16a, *r16b, *top16;
  uint32_t *r32a, *r32b, *top32;
  uint64_t *r64a, *r64b, *top64, *r64c;
  int w, rv;

  h = (gf_internal_t *) gf->scratch;
  w = h->w;
  top = ra + size;

  if (w == 8) {
    r8a = (uint8_t *) ra; 
    r8b = (uint8_t *) rb; 
    top8 = (uint8_t *) top;
    if (test == 'M') {
      while (r8a < top8) {
        *r8a = gf->multiply.w32(gf, *r8a, *r8b);
        r8a++;
        r8b++;
      }
    } else if (test == 'D') {
      while (r8a < top8) {
        *r8a = gf->divide.w32(gf, *r8a, *r8b);
        r8a++;
        r8b++;
      }
    } else if (test == 'I') {
      while (r8a < top8) {
        *r8a = gf->inverse.w32(gf, *r8a);
        r8a++;
      }
    }
    return (top8 - (uint8_t *) ra);
  }

  if (w == 16) {
    r16a = (uint16_t *) ra; 
    r16b = (uint16_t *) rb; 
    top16 = (uint16_t *) top;
    if (test == 'M') {
      while (r16a < top16) {
        *r16a = gf->multiply.w32(gf, *r16a, *r16b);
        r16a++;
        r16b++;
      }
    } else if (test == 'D') {
      while (r16a < top16) {
        *r16a = gf->divide.w32(gf, *r16a, *r16b);
        r16a++;
        r16b++;
      }
    } else if (test == 'I') {
      while (r16a < top16) {
        *r16a = gf->inverse.w32(gf, *r16a);
        r16a++;
      }
    }
    return (top16 - (uint16_t *) ra);
  }
  if (w <= 32) {
    r32a = (uint32_t *) ra; 
    r32b = (uint32_t *) rb; 
    top32 = (uint32_t *) ra + (size/4); /* This is for the "everything elses" */
    
    if (test == 'M') {
      while (r32a < top32) {
        *r32a = gf->multiply.w32(gf, *r32a, *r32b);
        r32a++;
        r32b++;
      }
    } else if (test == 'D') {
      while (r32a < top32) {
        *r32a = gf->divide.w32(gf, *r32a, *r32b);
        r32a++;
        r32b++;
      }
    } else if (test == 'I') {
      while (r32a < top32) {
        *r32a = gf->inverse.w32(gf, *r32a);
        r32a++;
      }
    }
    return (top32 - (uint32_t *) ra);
  }
  if (w == 64) {
    r64a = (uint64_t *) ra; 
    r64b = (uint64_t *) rb; 
    top64 = (uint64_t *) top;
    if (test == 'M') {
      while (r64a < top64) {
        *r64a = gf->multiply.w64(gf, *r64a, *r64b);
        r64a++;
        r64b++;
      }
    } else if (test == 'D') {
      while (r64a < top64) {
        *r64a = gf->divide.w64(gf, *r64a, *r64b);
        r64a++;
        r64b++;
      }
    } else if (test == 'I') {
      while (r64a < top64) {
        *r64a = gf->inverse.w64(gf, *r64a);
        r64a++;
      }
    }
    return (top64 - (uint64_t *) ra);
  }
  if (w == 128) {
    r64a = (uint64_t *) ra; 
    r64c = r64a;
    r64a += 2;
    r64b = (uint64_t *) rb; 
    top64 = (uint64_t *) top;
    rv = (top64 - r64a)/2;
    if (test == 'M') {
      while (r64a < top64) {
        gf->multiply.w128(gf, r64a, r64b, r64c);
        r64a += 2;
        r64b += 2;
      }
    } else if (test == 'D') {
      while (r64a < top64) {
        gf->divide.w128(gf, r64a, r64b, r64c);
        r64a += 2;
        r64b += 2;
      }
    } else if (test == 'I') {
      while (r64a < top64) {
        gf->inverse.w128(gf, r64a, r64c);
        r64a += 2;
      }
    }
    return rv;
  }
  return 0;
}
