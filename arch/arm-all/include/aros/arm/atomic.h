/*
 * atomic.h
 *
 *  Created on: Oct 23, 2010
 *      Author: misc
 */

#ifndef AROS_ARM_ATOMIC_H
#define AROS_ARM_ATOMIC_H

#define __AROS_ATOMIC_INC_B(var) \
do { \
   unsigned long temp; \
   int result; \
   __asm__ __volatile__("\n1: ldrexb %0, [%2]; add %0, %0, #1; strexb %1, %0, [%2]; teq %1, #0; bne 1b" \
		   	   	   	   	   :"=&r"(result), "=&r"(temp) \
		   	   	   	   	   :"r"(var) \
		   	   	   	   	   :"cc"); \
} while (0)
#define __AROS_ATOMIC_INC_W(var) \
do { \
   unsigned long temp; \
   int result; \
   __asm__ __volatile__("\n1: ldrexh %0, [%2]; add %0, %0, #1; strexh %1, %0, [%2]; teq %1, #0; bne 1b" \
		   	   	   	   	   :"=&r"(result), "=&r"(temp) \
		   	   	   	   	   :"r"(var) \
		   	   	   	   	   :"cc"); \
} while (0)
#define __AROS_ATOMIC_INC_L(var) \
do { \
   unsigned long temp; \
   int result; \
   __asm__ __volatile__("\n1: ldrex %0, [%2]; add %0, %0, #1; strex %1, %0, [%2]; teq %1, #0; bne 1b" \
		   	   	   	   	   :"=&r"(result), "=&r"(temp) \
		   	   	   	   	   :"r"(var) \
		   	   	   	   	   :"cc"); \
} while (0)

#define __AROS_ATOMIC_DEC_B(var) \
do { \
   unsigned long temp; \
   int result; \
   __asm__ __volatile__("\n1: ldrexb %0, [%2]; sub %0, %0, #1; strexb %1, %0, [%2]; teq %1, #0; bne 1b" \
		   	   	   	   	   :"=&r"(result), "=&r"(temp) \
		   	   	   	   	   :"r"(var) \
		   	   	   	   	   :"cc"); \
} while (0)
#define __AROS_ATOMIC_DEC_W(var) \
do { \
   unsigned long temp; \
   int result; \
   __asm__ __volatile__("\n1: ldrexh %0, [%2]; sub %0, %0, #1; strexh %1, %0, [%2]; teq %1, #0; bne 1b" \
		   	   	   	   	   :"=&r"(result), "=&r"(temp) \
		   	   	   	   	   :"r"(var) \
		   	   	   	   	   :"cc"); \
} while (0)
#define __AROS_ATOMIC_DEC_L(var) \
do { \
   unsigned long temp; \
   int result; \
   __asm__ __volatile__("\n1: ldrex %0, [%2]; sub %0, %0, #1; strex %1, %0, [%2]; teq %1, #0; bne 1b" \
		   	   	   	   	   :"=&r"(result), "=&r"(temp) \
		   	   	   	   	   :"r"(var) \
		   	   	   	   	   :"cc"); \
} while (0)

#define __AROS_ATOMIC_AND_B(var, mask) \
do { \
    unsigned long temp; int result; \
    __asm__ __volatile__("\n1: ldrexb %0, [%2]; and %0, %0, %3; strexb %1, %0, [%2]; teq %1, #0; bne 1b" \
                        :"=&r"(result), "=&r"(temp) \
                        :"r"(var), "I"(mask) \
                        :"cc"); \
} while(0)
#define __AROS_ATOMIC_AND_W(var, mask) \
do { \
    unsigned long temp; int result; \
    __asm__ __volatile__("\n1: ldrexh %0, [%2]; and %0, %0, %3; strexh %1, %0, [%2]; teq %1, #0; bne 1b" \
                        :"=&r"(result), "=&r"(temp) \
                        :"r"(var), "Ir"(mask) \
                        :"cc"); \
} while(0)
#define __AROS_ATOMIC_AND_L(var, mask) \
do { \
    unsigned long temp; int result; \
    __asm__ __volatile__("\n1: ldrex %0, [%2]; and %0, %0, %3; strex %1, %0, [%2]; teq %1, #0; bne 1b" \
                        :"=&r"(result), "=&r"(temp) \
                        :"r"(var), "Ir"(mask) \
                        :"cc"); \
} while(0)

#define __AROS_ATOMIC_OR_B(var, mask) \
do { \
    unsigned long temp; int result; \
    __asm__ __volatile__("\n1: ldrexb %0, [%2]; or %0, %0, %3; strexb %1, %0, [%2]; teq %1, #0; bne 1b" \
                        :"=&r"(result), "=&r"(temp) \
                        :"r"(var), "I"(mask) \
                        :"cc"); \
} while(0)
#define __AROS_ATOMIC_OR_W(var, mask) \
    do { \
    unsigned long temp; int result; \
    __asm__ __volatile__("\n1: ldrexh %0, [%2]; or %0, %0, %3; strexh %1, %0, [%2]; teq %1, #0; bne 1b" \
                        :"=&r"(result), "=&r"(temp) \
                        :"r"(var), "Ir"(mask) \
                        :"cc"); \
} while(0)
#define __AROS_ATOMIC_OR_L(var, mask) \
do { \
    unsigned long temp; int result; \
    __asm__ __volatile__("\n1: ldrex %0, [%2]; or %0, %0, %3; strex %1, %0, [%2]; teq %1, #0; bne 1b" \
                        :"=&r"(result), "=&r"(temp) \
                        :"r"(var), "Ir"(mask) \
                        :"cc"); \
} while(0)

#endif /* AROS_ARM_ATOMIC_H */
