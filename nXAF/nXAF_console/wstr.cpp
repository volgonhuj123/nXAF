#include "memalloc.h"

#define SWCHAR sizeof(wchar_t)
#ifndef NC_WSTR_GROWTH_FACTOR
#define NC_WSTR_GROWTH_FACTOR 8
#endif


namespace nc {
	struct wstr {
		wchar_t* data = 0;
		size_t size = 0, capacity = 0;
		void add(const wchar_t wc) {
			size_t sum = size + 1;
			wchar_t* temp;
			if (size == capacity) {
				capacity += NC_WSTR_GROWTH_FACTOR;
				temp = (wchar_t*)memalloc(SWCHAR * capacity);
				memcpy(temp, data, SWCHAR * size);
				memfree(data);
				data = temp;
			}
			data[size] = wc;
			size = sum;
		}
		void add(const wchar_t* wcstr) {
			size_t wcstr_len = wcslen(wcstr),sum= size + wcstr_len;
			wchar_t* temp;
			if(sum > capacity){
				capacity = sum + NC_WSTR_GROWTH_FACTOR;
				temp = (wchar_t*)memalloc(SWCHAR * capacity);
				memcpy(temp, data, SWCHAR * size);
				memfree(data);
				data = temp;
			}
			memcpy(data+ size, wcstr, wcstr_len * SWCHAR);
			size = sum;
		}

		


		void add(const wchar_t* wcstr, size_t wcstr_len) {
			size_t sum = size + wcstr_len;
			wchar_t* temp;
			if (sum > capacity) {
				capacity = sum + NC_WSTR_GROWTH_FACTOR;
				temp = (wchar_t*)memalloc(SWCHAR * capacity);
				memcpy(temp, data, SWCHAR * size);
				memfree(data);
				data = temp;
			}
			memcpy(data+ size, wcstr, wcstr_len * SWCHAR);
			size = sum;
		}

		void add_front(const wchar_t* wcstr, size_t wcstr_len) {
			size_t sum = size + wcstr_len;
			wchar_t* temp;
			if (sum > capacity) {
				capacity = sum + NC_WSTR_GROWTH_FACTOR;
				temp = (wchar_t*)memalloc(SWCHAR * capacity);
				memcpy(temp, wcstr, SWCHAR * wcstr_len);
				memcpy(temp+ wcstr_len, data, SWCHAR * size);
				memfree(data);
				data = temp;
			}
			else {
				memmove(data + wcstr_len, data, size * SWCHAR);
				memcpy(data, wcstr, SWCHAR* wcstr_len);
			}
			size = sum;
			

		}


		void copy_from(wstr& other) {
			if (other.size > capacity || other.size*2< capacity) {
				memfree(data);
				capacity = other.capacity;
				data = (wchar_t*)memalloc(SWCHAR * capacity);
			}
			size = other.size;
			memcpy(data, other.data, SWCHAR * size);
		}
		void copy_from(const wchar_t* other,size_t other_size) {
			if (other_size > capacity || other_size * 2 < capacity) {
				memfree(data);
				capacity = other_size+ NC_WSTR_GROWTH_FACTOR;
				data = (wchar_t*)memalloc(SWCHAR * capacity);
			}
			size = other_size;
			memcpy(data, other, SWCHAR * size);
		}
		void add( nc::wstr& other) {
			size_t sum = size + other.size;
			wchar_t* temp;
			if (sum > capacity) {
				capacity = sum + NC_WSTR_GROWTH_FACTOR;
				temp = (wchar_t*)memalloc(SWCHAR * capacity);
				memcpy(temp, data, SWCHAR * size);
				memfree(data);
				data = temp;
			}
			memcpy(data+size, other.data, other.size * SWCHAR);
			size = sum;
		}

		void alloc(size_t new_size) {
			memfree(data);
			capacity = new_size;
			size = 0;
			data = (wchar_t*)memalloc(SWCHAR * capacity);
		}
		
		void reserve(size_t new_size) {
			wchar_t* temp = (wchar_t*)memalloc(SWCHAR * new_size);

			if (new_size < size) {
				memcpy(temp, data, SWCHAR * new_size);
				size = new_size;
			}
			else {
				memcpy(temp, data, SWCHAR * size);
			}
			
			memfree(data);
			data = temp;
			capacity = new_size;
		}

		const wchar_t operator [](size_t i) {
			return data[i];
		}
	
		wchar_t* c_str() {
			wchar_t* ret = (wchar_t*)memalloc(SWCHAR * (size + 1));;
			memcpy(ret, data, SWCHAR * size);
			ret[size] = 0;
			return ret;
		}

		void destroy() { memfree(data); memset(this, 0, sizeof(wstr)); }

		int previous_directory() {
			while(size){
				size--;
				if ((data[size] == L'/') || (data[size] == L'\\')){break; }
			}
			return size != 0;

		}
	};



}