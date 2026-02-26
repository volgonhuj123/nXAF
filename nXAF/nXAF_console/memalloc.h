#pragma once

#ifndef _H_NC_MEMALLOC_
#define _H_NC_MEMALLOC_

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#ifdef NC_DEBUG
namespace nc_debug {

	struct ptr_arr {
		void** data ;
		size_t size , capacity ;


		void add(void* e);

		void del(void* e);

		void print();

		void destroy();
	};
	extern ptr_arr allocs;
};
#endif

void* memalloc(size_t s);
void* rememalloc(void* ptr, size_t n);
void* memalloc(size_t s);
void* mecalloc(size_t s);
#ifndef NC_DEBUG
#define memfree(PTR) free(PTR)
#else
void memfree(void* ptr);
#endif
#endif