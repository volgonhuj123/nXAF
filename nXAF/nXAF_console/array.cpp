#include <string.h>
#include"memalloc.h"
#define NC_ARR_GROWTH_SIZE 8
#define NC_ARR_T_SIZE sizeof(T)
namespace nc {

	template  <typename T>struct arr {
		T* data=NULL;
		size_t size = 0, capacity = 0;
		
		void add(const T* e) {
			if (size == capacity) {
				capacity += NC_ARR_GROWTH_SIZE;
				data =(T *) rememalloc(data, capacity* NC_ARR_T_SIZE);
			}
			memcpy(data + size++, e, NC_ARR_T_SIZE);

		}

		void add(const T* e,  size_t count) {
			if (size+ count > capacity) {
				capacity += NC_ARR_GROWTH_SIZE+ count;
				data = (T*)rememalloc(data,NC_ARR_T_SIZE * capacity);
			}
			memcpy(data + size, e, NC_ARR_T_SIZE* count);
			size += count;
		}
		void destroy(){
			memfree(data);
			size = 0;
			capacity = 0;
			data = 0;
		}
		// ~arr() {
		//	 destroy();
		//}


		 T operator [](size_t i) {
			 return data[i];
		 }

		 void realloc(size_t n) {
			 capacity = n;
			 size = size>n?n:size;
			 data =(T*) rememalloc(data,n* NC_ARR_T_SIZE);
		 }


		 operator void*() {
			 return data;
		 }

		
	};

}