/*
 * @file macro.c
 * @brief Implementation of macro.h
 * @author Li Yanhao <1400012849@pku.edu.cn>
 */

#include "common.h"

bool cmp_lt(any_t x, any_t y){return x < y;}
void swap(any_t* x, any_t* y){any_t tmp = *x; *x = *y; *y = tmp;}
