//#include "framework.h"
//#include "stack.h"
#define CONSOLE_APP
#include <io.h>
#include <string.h>
//#include"memalloc.h"
#include "basic_linked_list.h"
#include <pthread.h>
#include"array.h"
#include"wstr.h"
#include<SDL.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#pragma warning( disable : 4996)
#define str(a) sssss(a)
#define sssss(a) #a
#define byte unsigned char
#define UINT unsigned int
#define BYTE unsigned char
#define UINT64 unsigned long long
#define INT64 long long
#define UINT16 unsigned short
#define PTRSIZE sizeof(void*)
#include"LinkedList.h"

namespace nc {
enum {
	XAF_FOLDER,XAF_FILE
};

struct error_return { UINT64 code = 0; void* data = 0; const char* error_message = 0; };


union error{
	BYTE raw[64];
	error_return error_return;
	error(){
		memset(raw, 0, sizeof(error));
	}
};

const char* xaf_get_error() {
	return "ERROR";
}


union error last_error;

#define STRCAT_SUB(A,B) A ## B
#define STRCAT(A,B) STRCAT_SUB(A,B)
#define T_ARR(T) STRCAT(T,_arr)
#define nc_arr_build(T)    STRCAT(STRCAT(struc,t T_ARR(T)) ,\
			{T * data=0;size_t size=0 ;size_t capacity=0;};)\
			int STRCAT(T_ARR(T),_add) (T_ARR(T) * t_arr,T *e){\
				T* temp;\
				if(t_arr->capacity==t_arr->size) {   \
						t_arr->capacity+=4;temp=(T*)malloc(sizeof(T)*t_arr->capacity);if(!temp){return 1;}	\			\
						memcpy(temp,t_arr->data,sizeof(T)*t_arr->size);memfree(t_arr->data);t_arr->data=temp;\
				}\
				memcpy(&t_arr->data[t_arr->size++],e,sizeof(T));	\
				return 0;\
			}
			
#define PTRSIZE sizeof(void*)
struct xaf;
struct folder;
struct file {
	UINT filetype = XAF_FILE;
	UINT struct_size = sizeof(file);
	wchar_t* name = 0;
	UINT64 name_size = 0;
	xaf* parent = 0;
	void* content = 0;
	UINT64 content_size = 0;
	
	void build(const void* data, UINT size, const wchar_t* filename, UINT filename_size) {
		this->filetype = XAF_FILE;
		this->struct_size = sizeof(file);
		this->name_size = filename_size;
		this->name = (wchar_t*)memalloc(filename_size * sizeof(wchar_t));
		memcpy(this->name, filename, filename_size * sizeof(wchar_t));
		this->content = memalloc(size);
		memcpy(this->content, data, size);
		this->content_size = size;
	}
	void destroy() {
		this->name_size = 0;
		memfree(this->name);
		memfree(this->content);
		this->content_size = 0;
		this->name = 0;
		this->content = 0;
	}
};

struct __xaf_file_reader{
	FILE* file=0;
	size_t size = 0;
	INT64 pos = 0;
	size_t read(void* buffer,size_t count) {
		INT64 comeback_pos = _ftelli64(file);
		_fseeki64(file, pos, SEEK_SET);
		count = count > size ? size : count;
		_fseeki64(file, comeback_pos, SEEK_SET);
		return fread(buffer, count, 1, file);
	}
};


struct file_cell {
	file_cell* prev = 0, * next = 0;
	file data;
	

	void build(const void* data, UINT size, const wchar_t* filename, UINT filename_size) {
		this->data.build(data, size, filename, filename_size);
	}
	void destroy() {
		this->data.destroy();
	}

};

struct __temp_xaf {
	file_cell* start = 0, * end = 0;
	UINT64 size = 0,space_occupied = 0;
	UINT64 is_folder = 1;
};
size_t decrease_to_folder(const wchar_t* filename, size_t filename_size) {
	filename += filename_size - 1;
	while ((*filename != L'\\')&& (*filename != L'/') && filename_size) {
		//wprintf(L"decrease_to_folder  filename : %c\n",*filename);
		filename--; filename_size--;
	}
	return filename_size;
}
size_t distance_to_folder(const wchar_t* filename, size_t filename_size) {
	filename += filename_size - 1;
	size_t ret = 0;
	while ((*filename != L'\\') && (*filename != L'/') && filename_size) {
		filename--; filename_size--; ret++;
	}
	return ret;
}
struct folder {
	UINT filetype = XAF_FOLDER;
	UINT struct_size = sizeof(folder);
	wchar_t* name = 0;
	UINT64 name_size = 0;
	xaf* parent = 0;
	__temp_xaf files;
	
	void build(const wchar_t* filename, UINT64 filename_size) {
		this->filetype = XAF_FOLDER;
		this->struct_size = sizeof(folder);
		this->name = (wchar_t*)memalloc(filename_size * sizeof(wchar_t));
		memcpy(this->name, filename, filename_size * sizeof(wchar_t));
		memset(&files, 0, sizeof(files)-8);
		this->name_size = filename_size;
		this->files.is_folder = 1;
	}
	void destroy() {
		file_cell* temp ,*last;
		folder* folder_temp;
		nc::basic_linked_list<file_cell*> ll;
		memfree(name);
		name_size = 0; name = 0;
		if (!files.size) { return; }
		ll.add_front(&files.start);
		while (ll.count) {
			temp = ll.pop_front();
			while (temp) {
				if (temp->data.filetype == XAF_FILE) {
					memfree(temp->data.name);
					memfree(temp->data.content);
				}
				else {
					folder_temp =(folder *) &temp->data;
					ll.add_front(&folder_temp->files.start);
					memfree(folder_temp->name);
				}
				last = temp;
				temp = temp->next;
				memfree(last);
			}
		}
		files.size = 0;
		files.start = 0;
		files.end = 0;
		files.space_occupied = 0;
	}

	xaf* subfiles() { return (xaf*)&files; }
	int add(file* f);
	int add(folder* f);

	int add(const void* content, size_t content_size, const wchar_t* filename, BYTE filename_size);

	void copyto(xaf* dest);
	void print();
	file* travel(const wchar_t* path, size_t str_path_size);
	file_cell* travel_cell(const wchar_t* path, size_t str_path_size);

};
#include<assert.h>
#include <immintrin.h>
struct xaf_password {
	byte* data = 0;
	size_t size = 0;
	void set(const byte* data, size_t size) {
		this->data = (byte*)memalloc(size);
		memcpy(this->data, data, size);
		this->size = size;

	}
	int load(const wchar_t* filename) {
		FILE* input = _wfopen(filename, L"rb");
		fseek(input, 0, SEEK_END);
		size = _ftelli64(input);
		fseek(input, 0, SEEK_SET);

		assert(sizeof(byte) == 1);
		data = (byte*)memalloc(size);
		fread(data, size, 1, input);
		fclose(input);
	}

	void encrypt(void* data, size_t data_size) {
		byte* temp;
		__m256i avx2_pass,avx2_data;
		__m128i sse_pass, sse_data;
		size_t count= data_size>>5;
		assert(sizeof(UINT64) == 8);
		UINT64 i64_data, i64_pass,*i64_ptr;
		assert(sizeof(UINT) == 4);
		UINT i32_data, i32_pass,*i32_ptr;

		UINT16  * i16_ptr;
		BYTE  * i8_ptr;
		while(count--) {
			avx2_data = _mm256_loadu_si256((__m256i*)data);
			for (size_t i = 0; i != size; i++) {
				//BROADCAST
				avx2_pass = _mm256_set1_epi8(this->data[i]);
				avx2_data = _mm256_xor_si256(avx2_pass, avx2_data);
			}
			_mm256_storeu_si256((__m256i*)data, avx2_data);
			data = ((byte*)data) + 32;
		}
		data_size &= 0x1f;
		if (data_size &16) {
			sse_data = _mm_loadu_si128((__m128i*)data);
			for (size_t i = 0; i != size; i++) {
				//BROADCAST
				sse_pass = _mm_set1_epi8(this->data[i]);
				sse_data = _mm_xor_si128(sse_pass, sse_data);
			}
			_mm_storeu_si128((__m128i*)data, sse_data);
			data = ((byte*)data) + 16;
			data_size &= ~16;
		}
		if (data_size &8) {
			i64_ptr = (UINT64*)data;
			i64_data = *i64_ptr;
			for (size_t i = 0; i != size; i++) {
				//BROADCAST
				i64_pass = this->data[i];
				i64_pass |= i64_pass << 8;
				i64_pass |= i64_pass << 16;
				i64_pass |= i64_pass << 32;
				i64_data = i64_data^i64_pass;
			}
			*i64_ptr = i64_data;
			data = ((byte*)data) + 8;
			data_size &= ~8;
		}


		if (data_size &4) {
			i32_ptr = (UINT*)data;
			i32_data = *i32_ptr;
			for (size_t i = 0; i != size; i++) {
				//BROADCAST
				i32_pass= this->data[i] ;
				i32_pass |= i32_pass << 8;
				i32_pass |= i32_pass << 16;

				i32_data = i32_data ^ i32_pass;
			}
			*i32_ptr = i32_data;
			data = ((byte*)data) + 4;
			data_size &= ~4;
		}

		if (data_size &2) {
			i16_ptr = (UINT16*)data;
			i32_data = *i16_ptr;
			for (size_t i = 0; i != size; i++) {
				//BROADCAST
				i32_pass = this->data[i];
				i32_pass |= i32_pass << 8;
				i32_data ^= i32_pass;
			}
			*i16_ptr = i32_data;
			data = ((byte*)data) + 2;
			data_size &= ~2;
		}

		if (data_size) {
			i8_ptr = (BYTE*)data;
			i32_data = *i8_ptr;
			for (size_t i = 0; i != size; i++) {
				//BROADCAST
				i32_data ^= this->data[i];
			}
			*i8_ptr = i32_data;
		}

	}

	void decrypt(void* data, size_t data_size) {
		byte* temp;
		__m256i avx2_pass, avx2_data;
		__m128i sse_pass, sse_data;
		size_t count = data_size >> 5;
		assert(sizeof(UINT64) == 8);
		UINT64 i64_data, i64_pass, * i64_ptr;
		assert(sizeof(UINT) == 4);
		UINT i32_data, i32_pass, * i32_ptr;

		UINT16* i16_ptr;
		BYTE* i8_ptr;
		while (count--) {
			avx2_data = _mm256_loadu_si256((__m256i*)data);
			for (size_t i = size-1; i !=(size_t) -1; i--) {
				//BROADCAST
				avx2_pass = _mm256_set1_epi8(this->data[i]);
				avx2_data = _mm256_xor_si256(avx2_pass, avx2_data);
			}
			_mm256_storeu_si256((__m256i*)data, avx2_data);
			data = ((byte*)data) + 32;
		}
		data_size &= 0x1f;
		if (data_size & 16) {
			sse_data = _mm_loadu_si128((__m128i*)data);
			for (size_t i = size - 1; i != (size_t)-1; i--) {
				//BROADCAST
				sse_pass = _mm_set1_epi8(this->data[i]);
				sse_data = _mm_xor_si128(sse_pass, sse_data);
			}
			_mm_storeu_si128((__m128i*)data, sse_data);
			data = ((byte*)data) + 16;
			data_size &= ~16;
		}
		if (data_size & 8) {
			i64_ptr = (UINT64*)data;
			i64_data = *i64_ptr;
			for (size_t i = size - 1; i != (size_t)-1; i--) {
				//BROADCAST
				i64_pass = this->data[i];
				i64_pass |= i64_pass << 8;
				i64_pass |= i64_pass << 16;
				i64_pass |= i64_pass << 32;
				i64_data = i64_data ^ i64_pass;
			}
			*i64_ptr = i64_data;
			data = ((byte*)data) + 8;
			data_size &= ~8;
		}


		if (data_size & 4) {
			i32_ptr = (UINT*)data;
			i32_data = *i32_ptr;
			for (size_t i = size - 1; i != (size_t)-1; i--) {
				//BROADCAST
				i32_pass = this->data[i];
				i32_pass |= i32_pass << 8;
				i32_pass |= i32_pass << 16;
				i32_data = i32_data ^ i32_pass;
			}
			*i32_ptr = i32_data;
			data = ((byte*)data) + 4;
			data_size &= ~4;
		}

		if (data_size & 2) {
			i16_ptr = (UINT16*)data;
			i32_data = *i16_ptr;
			for (size_t i = size - 1; i != (size_t)-1; i--) {
				//BROADCAST
				i32_pass = this->data[i];
				i32_pass |= i32_pass << 8;
				i32_data ^= i32_pass;
			}
			*i16_ptr = i32_data;
			data = ((byte*)data) + 2;
			data_size &= ~2;
		}

		if (data_size) {
			i8_ptr = (BYTE*)data;
			i32_data = *i8_ptr;
			for (size_t i = size - 1; i != (size_t)-1; i--) {
				//BROADCAST
				i32_data ^= this->data[i];
			}
			*i8_ptr = i32_data;
		}

	}

	void destroy() {
		memfree(data);
		data = 0;
		size = 0;
	}
	
};

struct folder_cell {
	folder_cell* prev = 0, * next = 0;
	folder data;
};

static int filename_cmp(const wchar_t* fln1, const wchar_t* fln2, size_t len) {
	int temp;
	while(len--){
		if (*fln1 != *fln2) {
			if ((*fln1 >= L'a' && *fln1 <= L'z') && (*fln2 >= L'A' && *fln2 <= L'Z')) {
				temp = *fln1 - 0x20;
				if (temp != *fln2) {
					return temp - *fln2;
				}
			}
			else if ((*fln1 >= L'A' && *fln1 <= L'Z') && (*fln2 >= L'a' && *fln2 <= L'z')) {
				temp = *fln1 + 0x20;
				if (temp != *fln2) {
					return temp - *fln2;
				}
			}
			else {
				return *fln1 - *fln2;
			}
		}
		else {
			fln1++;
			fln2++;
		}
	}
	return 0;
}
struct __temp_xaf_pair {
	__temp_xaf* src, * dst;
 };


struct ___xaf_stack {
	size_t size = 0, start = 0;

	
	void push(size_t thread_id){
		size_t* data = (&start)+1;
		data[--start] = thread_id;
	}
	size_t pop() {
		size_t* data = (&start) + 1;
		return data[start++];
	}

};
#include<pthread.h>


struct th_copy_args {
	size_t thread_id;
	___xaf_stack* stack;
	pthread_mutex_t* stack_mutex ;
	pthread_t *threads;			//256 bits
	th_copy_args* th_args;		//64 bits
	__temp_xaf_pair txp;		//STRUCT SIZE    448 bits  MULTIPLE 3?
};
void* th_copy(void* arg_ptr);


//void *vvstr=u8"554";

struct xaf{
	file_cell* start = 0,*end=0;
	UINT64 size = 0,space_occupied = 0;
	UINT64 is_folder = 0;
	int add(file* f) {
		folder* parent;
		xaf* arc;
		if (size == 0) {
			start = (file_cell*)memalloc(f->struct_size+ PTRSIZE*2);
			end = start;
			start->data.build(f->content, f->content_size, f->name, f->name_size);
			start->next = 0;
			start->prev = 0;
			space_occupied += f->content_size;
			start->data.parent = this;
			parent = get_folder();
			while (parent) {
				arc = parent->parent;
				if (arc) {
					arc->space_occupied += f->content_size;
					parent = arc->get_folder();
				}
				else {
					parent = 0;
				}
			}
			size = 1;
			
			wprintf(L"Added %.*s \n", start->data.name_size, start->data.name);
			return 0;
		}
		file_cell* temp_cell = NULL,
			*temp=start;
		UINT64 i = size,len;
		int res;
		while (i--) {
			len = temp->data.name_size > f->name_size ? f->name_size : temp->data.name_size;
			//wprintf(L"len %u %u\n", temp->data.name_size, f->name_size);
			res = filename_cmp(temp->data.name, f->name, len);
			//temp Before
			if (res == 0) {
				if (temp->data.name_size == f->name_size) {
					wprintf(L"There is already a %s called %.*s", temp->data.filetype == XAF_FOLDER ? L"folder" : L"file"
						, temp->data.name_size, temp->data.name);
					last_error.error_return.code = 0x5001;
					last_error.error_return.data = temp;
					return 1;
				}

				temp_cell = (file_cell*)memalloc(sizeof(file_cell));
				if (temp->data.name_size < f->name_size) {	//FOUND2		AFTER
					temp_cell->next = temp->next;
					temp_cell->prev = temp;
					if (temp->next) {
						temp->next->prev = temp_cell;
					}
					else {
						end = temp_cell;
					}
					temp->next = temp_cell;
				}
				else {							//BEFORE
					temp_cell->next = temp;
					temp_cell->prev = temp->prev;
					if (temp->prev) {
						temp->prev->next = temp_cell;
					}
					else {
						start = temp_cell;
					}
					temp->prev = temp_cell;
				}
				break;
			}
			//temp After
			else {
				if (res > 0) {
					temp_cell = (file_cell*)memalloc(sizeof(file_cell));
					temp_cell->next = temp;
					temp_cell->prev = temp->prev;
					if (temp->prev) {
						temp->prev->next = temp_cell;
					}
					else {
						start = temp_cell;
					}
					temp->prev = temp_cell;
					break;
				}
				else {
					if (i) {
						temp = temp->next;
					}
					else {
						temp_cell = (file_cell*)memalloc(sizeof(file_cell));
						temp_cell->next = NULL;
						temp_cell->prev = (file_cell*)temp;
						end = (file_cell*)temp_cell;
						temp->next = (file_cell*)temp_cell;
					}
				}
			}
		}
		space_occupied += f->content_size;
		temp_cell->data.parent = this;
		parent = get_folder();
		while (parent) {
			arc = parent->parent;
			if (arc) {
				arc->space_occupied += f->content_size;
				parent = arc->get_folder();
			}
			else {
				parent = 0;
			}
		}

		size++;
		temp_cell->build(f->content, f->content_size, f->name, f->name_size);
		wprintf(L"Added %.*s \n", temp_cell->data.name_size, temp_cell->data.name);
		

		return 0;
	}
	
	int add(folder* f,folder_cell **ret) {


		nc::basic_linked_list<__temp_xaf_pair> sub_folders;
		__temp_xaf_pair xaf_pair;
		folder* parent;
		xaf* arc;
		if (size == 0) {
			folder* folder_data;
			start = (file_cell*)memalloc(f->struct_size + PTRSIZE * 2);
			*ret = (folder_cell*)start;
			end = start;
			start->next = 0;
			start->prev = 0;
			size = 1;
			folder_data = (folder*)&start->data;
			space_occupied += f->files.space_occupied;
			folder_data->build(f->name, f->name_size);
			folder_data->parent = this;
			if (f->files.size == 0) { return 0; }
			parent = get_folder();
			while (parent) {
				arc = parent->parent;
				if (arc) {
					arc->space_occupied += folder_data->files.space_occupied;
					parent = arc->get_folder();
				}
				else { break; }
			}
			xaf_pair.dst = &folder_data->files;
			xaf_pair.src = &f->files;
			sub_folders.add_front(&xaf_pair);
			
			file_cell** dst_start, * last_dst_start, * src_start;
			while (sub_folders.count) {
				xaf_pair = sub_folders.pop_front();
				dst_start = (file_cell**)&xaf_pair.dst->start;
				src_start = (file_cell*)xaf_pair.src->start;
				xaf_pair.dst->size = xaf_pair.src->size;
				xaf_pair.dst->space_occupied = xaf_pair.src->space_occupied;
				last_dst_start = 0;
				while (src_start) {
					*dst_start = (file_cell*)memalloc(src_start->data.struct_size + PTRSIZE * 2);
					file_cell* temp_dst_start = *dst_start;
					temp_dst_start->data.parent =(xaf*)xaf_pair.dst;
					temp_dst_start->prev = last_dst_start;
					if (last_dst_start) {
						last_dst_start->next = temp_dst_start;
					}
					last_dst_start = temp_dst_start;
					if (src_start->data.filetype == XAF_FILE) {
						temp_dst_start->build(src_start->data.content, src_start->data.content_size,
							src_start->data.name, src_start->data.name_size);
					}
					else {
						folder* temp_dst_folder_ptr = (folder*)&temp_dst_start->data,
							* temp_src_folder_ptr = (folder*)&src_start->data;
						temp_dst_folder_ptr->build(temp_src_folder_ptr->name, temp_src_folder_ptr->name_size);
						xaf_pair.dst = &temp_dst_folder_ptr->files;
						xaf_pair.src = &temp_src_folder_ptr->files;
						sub_folders.add_front(&xaf_pair);
					}
					dst_start = &temp_dst_start->next;
					src_start = src_start->next;
				}
				*dst_start = 0;
			}
			sub_folders.destroy();
			return 0;
		}
		folder_cell* temp_cell=NULL;
		file_cell* temp = start;
		UINT64 i = size, len;
		int res;
		while (i--) {
			len = temp->data.name_size > f->name_size ? f->name_size : temp->data.name_size;
			//wprintf(L"len %u %u\n", temp->data.name_size, f->name_size);
			res = filename_cmp(temp->data.name, f->name, len);
			//temp Before
			if (res == 0) {
				if (temp->data.name_size == f->name_size) {
					wprintf(L"There is already a %s called %.*s", temp->data.filetype == XAF_FOLDER ? L"folder" : L"file"
						, temp->data.name_size, temp->data.name);
					last_error.error_return.code = 0x5001;
					last_error.error_return.data = temp;
					*ret = NULL;
					return 1;
				}

				temp_cell = (folder_cell*)memalloc(sizeof(folder_cell));
				if (temp->data.name_size < f->name_size) {	//FOUND2		AFTER
					temp_cell->next = (folder_cell*)temp->next;
					temp_cell->prev = (folder_cell*)temp;
					if (temp->next) {
						temp->next->prev = (file_cell*)temp_cell;
					}
					else {
						end = (file_cell*)temp_cell;
					}
					temp->next = (file_cell*)temp_cell;
				}
				else {							//BEFORE
					temp_cell->next = (folder_cell*)temp;
					temp_cell->prev = (folder_cell*)temp->prev;
					if (temp->prev) {
						temp->prev->next = (file_cell*)temp_cell;
					}
					else {
						start = (file_cell*)temp_cell;
					}
					temp->prev = (file_cell*)temp_cell;
				}
				break;
			}
			//temp After
			else {
				if (res > 0) {
					temp_cell = (folder_cell*)memalloc(sizeof(folder_cell));
					temp_cell->next = (folder_cell*)temp;
					temp_cell->prev = (folder_cell*)temp->prev;
					if (temp->prev) {
						temp->prev->next = (file_cell*)temp_cell;
					}
					else {
						start = (file_cell*)temp_cell;
					}
					temp->prev = (file_cell*)temp_cell;
					break;
				}
				else {
					if (i) {
						temp = temp->next;
					}
					else {
						temp_cell = (folder_cell*)memalloc(sizeof(folder_cell));
						temp_cell->next = NULL;
						temp_cell->prev = (folder_cell*)temp;
						end = (file_cell*)temp_cell;
						temp->next = (file_cell*)temp_cell;
					}
				}
			}
		}
		*ret = temp_cell;

		size++;

		folder* folder_data = (folder*)&temp_cell->data;
		space_occupied += f->files.space_occupied;
		folder_data->build(f->name, f->name_size);
		folder_data->parent = this;
		if (f->files.size == 0) { return 0; }
		parent = get_folder();
		while (parent) {
			arc = (xaf*)parent->parent;
			if (arc) {
				arc->space_occupied += folder_data->files.space_occupied;
				parent = arc->get_folder();
			}
			else { break; }
		}
		xaf_pair.dst = &folder_data->files;
		xaf_pair.src = &f->files;
		sub_folders.add_front(&xaf_pair);
		//wprintf(L"Adding folder %.*s{\n", folder_data->name_size, folder_data->name);
		file_cell** dst_start, * last_dst_start, * src_start;
		while (sub_folders.count) {
			xaf_pair = sub_folders.pop_front();
			dst_start = (file_cell**)&xaf_pair.dst->start;
			src_start = (file_cell*)xaf_pair.src->start;
			xaf_pair.dst->size = xaf_pair.src->size;
			xaf_pair.dst->space_occupied = xaf_pair.src->space_occupied;
			last_dst_start = 0;
			while (src_start) {
				*dst_start = (file_cell*)memalloc(src_start->data.struct_size + PTRSIZE * 2);
				file_cell* temp_dst_start = *dst_start;
				temp_dst_start->data.parent =(xaf*) xaf_pair.dst;
				temp_dst_start->prev = last_dst_start;
				if (last_dst_start) {
					last_dst_start->next = temp_dst_start;
				}
				temp_dst_start->next = 0;

				last_dst_start = temp_dst_start;
				if (src_start->data.filetype == XAF_FILE) {
					//wprintf(L"Copying file %.*s\n", src_start->data.name_size, src_start->data.name);
					temp_dst_start->build(src_start->data.content, src_start->data.content_size,
						src_start->data.name, src_start->data.name_size);
					//wprintf(L"Succesfully copied file %.*s\n", temp_dst_start->data.name_size, temp_dst_start->data.name);
					dst_start = &temp_dst_start->next;
					src_start = src_start->next;
				}
				else {
					//wprintf(L"Copying subfolder %.*s\n", src_start->data.name_size, src_start->data.name);
					folder* temp_dst_folder_ptr = (folder*)&temp_dst_start->data,
						* temp_src_folder_ptr = (folder*)&src_start->data;

					temp_dst_folder_ptr->build(temp_src_folder_ptr->name, temp_src_folder_ptr->name_size);
					//wprintf(L"Succesfully copied folder %.*s\n", temp_src_folder_ptr->name_size, temp_src_folder_ptr->name);
					xaf_pair.dst = &temp_dst_folder_ptr->files;
					xaf_pair.src = &temp_src_folder_ptr->files;
					xaf_pair.dst->size = xaf_pair.src->size;
					sub_folders.add_front(&xaf_pair);
					dst_start = (file_cell**)&((folder_cell*)temp_dst_start)->next;
					src_start = (file_cell*)((folder_cell*)src_start)->next;
				}
			}
			*dst_start = 0;
		}
		//printf("}\n");
		sub_folders.destroy();
		return 0;
	}
	
	int add_mt(folder* f,size_t thread_count) {
		if (thread_count == 0) { 
			last_error.error_return.code = 0x5050;
			return -3; }
		thread_count--;
		xaf* arc;
		folder* parent;
		pthread_t* threads=(pthread_t*)memalloc((sizeof(*threads)+sizeof(th_copy_args)+
			sizeof(size_t))*thread_count+ sizeof(___xaf_stack));
		th_copy_args* th_args = (th_copy_args*)(threads + thread_count) ;
		nc::basic_linked_list<__temp_xaf_pair> sub_folders;
		___xaf_stack* stack =(___xaf_stack*)(th_args + thread_count);
		stack->size = thread_count;
		stack->start = size;
		size_t* stack_data = (&stack->start) + 1;
		pthread_mutex_t mutex;
		pthread_mutex_init(&mutex, NULL);
		for (size_t i = 0; i != thread_count; i++) {
			stack_data[i] = i;
			th_args[i].thread_id = i;
			th_args[i].stack = stack;
			th_args[i].stack_mutex = &mutex;
			th_args[i].threads = threads;
			th_args[i].th_args = th_args;
		}
		__temp_xaf_pair xaf_pair;
		if (size == 0) {
			folder* folder_data;
			start = (file_cell*)memalloc(f->struct_size + PTRSIZE * 2);
			end = start;
			start->next = 0;
			start->prev = 0;
			folder_data = (folder*)&start->data;
			size = 1;
			space_occupied += folder_data->files.space_occupied;
			//wprintf(L"Added %.*s \n", f->name_size, f->name);
			folder_data->parent = this;
			if (f->files.size == 0) { return 0; }
			xaf_pair.dst = &folder_data->files;
			xaf_pair.src = &f->files;
			sub_folders.add_front(&xaf_pair);

			parent = get_folder();
			file_cell** dst_start, * last_dst_start, * src_start;
			while (sub_folders.count) {
				xaf_pair = sub_folders.pop_front();
				dst_start = (file_cell**)&xaf_pair.dst->start;
				src_start = (file_cell*)xaf_pair.src->start;
				//wprintf(L"Copying %.*s\n", src_start->data.name_size, src_start->data.name);
				xaf_pair.dst->size = xaf_pair.src->size;
				xaf_pair.dst->space_occupied = xaf_pair.src->space_occupied;
				last_dst_start = 0;
				while (src_start) {
					*dst_start = (file_cell*)memalloc(src_start->data.struct_size + PTRSIZE * 2);
					file_cell* temp_dst_start = *dst_start;
					temp_dst_start->data.parent = (xaf*)xaf_pair.dst;
					temp_dst_start->prev = last_dst_start;
					if (last_dst_start) {
						last_dst_start->next = temp_dst_start;
					}
					last_dst_start = temp_dst_start;
					if (src_start->data.filetype == XAF_FILE) {
						temp_dst_start->build(src_start->data.content, src_start->data.content_size,
							src_start->data.name, src_start->data.name_size);
					}
					else {
						folder* temp_dst_folder_ptr = (folder*)&temp_dst_start->data,
							* temp_src_folder_ptr = (folder*)&src_start->data;
						temp_dst_folder_ptr->build(temp_src_folder_ptr->name, temp_src_folder_ptr->name_size);
						xaf_pair.dst = &temp_dst_folder_ptr->files;
						xaf_pair.src = &temp_src_folder_ptr->files;
						sub_folders.add_front(&xaf_pair);
					}
					dst_start = &temp_dst_start->next;
					src_start = src_start->next;
				}
			}
			sub_folders.destroy();
			return 0;
		}
		folder_cell* temp_cell;
		file_cell* temp = start;
		UINT i = size, len;
		int res;
		while (i--) {
			len = temp->data.name_size > f->name_size ? f->name_size : temp->data.name_size;
			//wprintf(L"len %u %u\n", temp->data.name_size, f->name_size);
			res = filename_cmp(temp->data.name, f->name, len);
			//temp Before
			if (res == 0) {
				if (temp->data.name_size == f->name_size) {
					wprintf(L"There is already a %s called %.*s", temp->data.filetype == XAF_FOLDER ? L"folder" : L"file"
						, temp->data.name_size, temp->data.name);
					last_error.error_return.code = 0x5001;
					last_error.error_return.data = temp;
					return 1;
				}

				temp_cell = (folder_cell*)memalloc(sizeof(folder_cell));
				if (temp->data.name_size < f->name_size) {	//FOUND2		AFTER
					temp_cell->next = (folder_cell*)temp->next;
					temp_cell->prev = (folder_cell*)temp;
					if (temp->next) {
						temp->next->prev = (file_cell*)temp_cell;
					}
					else {
						end = (file_cell*)temp_cell;
					}
					temp->next = (file_cell*)temp_cell;
				}
				else {							//BEFORE
					temp_cell->next = (folder_cell*)temp;
					temp_cell->prev = (folder_cell*)temp->prev;
					if (temp->prev) {
						temp->prev->next = (file_cell*)temp_cell;
					}
					else {
						start = (file_cell*)temp_cell;
					}
					temp->prev = (file_cell*)temp_cell;
				}
				break;
			}
			//temp After
			else {
				if (res > 0) {
					temp_cell = (folder_cell*)memalloc(sizeof(folder_cell));
					temp_cell->next = (folder_cell*)temp;
					temp_cell->prev = (folder_cell*)temp->prev;
					if (temp->prev) {
						temp->prev->next = (file_cell*)temp_cell;
					}
					else {
						start = (file_cell*)temp_cell;
					}
					temp->prev = (file_cell*)temp_cell;
					break;
				}
				else {
					if (i) {
						temp = temp->next;
					}
					else {
						temp_cell = (folder_cell*)memalloc(sizeof(folder_cell));
						temp_cell->next = NULL;
						temp_cell->prev = (folder_cell*)temp;
						end = (file_cell*)temp_cell;
						temp->next = (file_cell*)temp_cell;
					}
				}
			}
		}

		temp_cell->data.parent = this;
		parent = get_folder();
		while (parent) {
			arc = parent->parent;
			if (arc) {
				arc->space_occupied += f->files.space_occupied;
				parent = arc->get_folder();
			}
			else { break; }
		}
		size_t th_id;
		size++;
		folder* folder_data = (folder*)&temp_cell->data;
		space_occupied += f->files.space_occupied;
		folder_data->build(f->name, f->name_size);
		folder_data->parent = this;
		if (f->files.size == 0) { return 0; }
		parent = get_folder();
		while (parent) {
			arc = parent->parent;
			if (arc) {
				arc->space_occupied += folder_data->files.space_occupied;
				parent = arc->get_folder();
			}
			else { break; }
		}
		pthread_attr_t th_attr;


		pthread_attr_init(&th_attr);
		pthread_attr_setdetachstate(&th_attr, PTHREAD_CREATE_DETACHED);


		
		
		xaf_pair.dst = &folder_data->files;
		xaf_pair.src = &f->files;
		sub_folders.add_front(&xaf_pair);
		//wprintf(L"Adding folder %.*s{\n", folder_data->name_size, folder_data->name);
		file_cell** dst_start, * last_dst_start, * src_start;
		while (sub_folders.count) {
			xaf_pair = sub_folders.pop_front();
			dst_start = (file_cell**)&xaf_pair.dst->start;
			src_start = (file_cell*)xaf_pair.src->start;
			xaf_pair.dst->size = xaf_pair.src->size;
			xaf_pair.dst->space_occupied = xaf_pair.src->space_occupied;
			last_dst_start = 0;
			while (src_start) {
				*dst_start = (file_cell*)memalloc(src_start->data.struct_size + PTRSIZE * 2);
				file_cell* temp_dst_start = *dst_start;
				temp_dst_start->prev = last_dst_start;
				temp_dst_start->data.parent = (xaf*)xaf_pair.dst;
				if (last_dst_start) {
					last_dst_start->next = temp_dst_start;
				}
				temp_dst_start->next = 0;
				last_dst_start = temp_dst_start;
				if (src_start->data.filetype == XAF_FILE) {
					//wprintf(L"Copying file %.*s\n", src_start->data.name_size, src_start->data.name);
					temp_dst_start->build(src_start->data.content, src_start->data.content_size,
						src_start->data.name, src_start->data.name_size);
					//wprintf(L"Succesfully copied file %.*s\n", temp_dst_start->data.name_size, temp_dst_start->data.name);
					dst_start = &temp_dst_start->next;
					src_start = src_start->next;
				}
				else {
					//wprintf(L"Copying subfolder %.*s\n", src_start->data.name_size, src_start->data.name);
					folder* temp_dst_folder_ptr = (folder*)&temp_dst_start->data,
						* temp_src_folder_ptr = (folder*)&src_start->data;

					temp_dst_folder_ptr->build(temp_src_folder_ptr->name, temp_src_folder_ptr->name_size);
					//wprintf(L"Succesfully copied folder %.*s\n", temp_src_folder_ptr->name_size, temp_src_folder_ptr->name);
					xaf_pair.dst = &temp_dst_folder_ptr->files;
					xaf_pair.src = &temp_src_folder_ptr->files;
					xaf_pair.dst->size = xaf_pair.src->size;
					pthread_mutex_lock(&mutex);
					if (stack->start == stack->size) {
						sub_folders.add_front(&xaf_pair);
					}
					else {
						th_id = stack->pop();
						th_args[th_id].txp = xaf_pair;
						pthread_create(threads + th_id, &th_attr, th_copy,th_args + th_id);
					}
					pthread_mutex_unlock(&mutex);
					dst_start = (file_cell**)&((folder_cell*)temp_dst_start)->next;
					src_start = (file_cell*)((folder_cell*)src_start)->next;
				}
			}
			*dst_start = 0;
		}
		//printf("}\n");
		sub_folders.destroy();

		size_t unfinished = 0;



		while (unfinished) {
			pthread_mutex_lock(&mutex);
			unfinished = stack->start	;
			pthread_mutex_unlock(&mutex);
		}
		memfree(threads);
		pthread_mutex_destroy(&mutex);
		pthread_attr_destroy(&th_attr);
		return 0;
	}
	int add(const void* content,UINT64 content_size,const wchar_t * filename,BYTE filename_size) {
		folder* parent;
		xaf* arc;
		if (size == 0) {
			start = (file_cell*)memalloc(sizeof(file) + PTRSIZE * 2);
			end = start;
			//start->data.build(f->content, f->content_size, f->name, f->name_size);
			start->data.filetype = XAF_FILE;
			start->data.struct_size = sizeof(file);
			start->data.name_size = filename_size;
			start->data.name = (wchar_t*)memalloc(filename_size * sizeof(wchar_t));
			memcpy(start->data.name, filename, filename_size * sizeof(wchar_t));
			start->data.content = memalloc(content_size);
			memcpy(start->data.content, content, content_size);
			start->data.content_size = content_size;
			start->next = 0;
			start->prev = 0;
			size = 1;
			start->data.parent = this;
			parent = get_folder();
			while (parent) {
				arc = parent->parent;
				if (arc) {
					arc->space_occupied += content_size;
					parent = arc->get_folder();
				}
				else { break; }
			}
			space_occupied += content_size;
			wprintf(L"Added %.*s \n", filename_size, filename);
			return 0;
		}
		file_cell* temp_cell=NULL,
			* temp = start;
		UINT i = size, len;
		int res;
		while (i--) {
			len = temp->data.name_size > filename_size ? filename_size : temp->data.name_size;
			//wprintf(L"len %u %u\n", temp->data.name_size, f->name_size);
			res = filename_cmp(temp->data.name, filename, len);
			//temp Before
			if (res == 0) {
				if (temp->data.name_size == filename_size) {
					wprintf(L"There is already a %s called %.*s", temp->data.filetype == XAF_FOLDER ? L"folder" : L"file"
						, temp->data.name_size, temp->data.name);
					last_error.error_return.code = 0x5001;
					last_error.error_return.data = temp;
					return 1;
				}

				temp_cell = (file_cell*)memalloc(sizeof(file_cell));
				if (temp->data.name_size < filename_size) {	//FOUND2		AFTER
					temp_cell->next = temp->next;
					temp_cell->prev = temp;
					if (temp->next) {
						temp->next->prev = temp_cell;
					}
					else {
						end = temp_cell;
					}
					temp->next = temp_cell;
				}
				else {							//BEFORE
					temp_cell->next = temp;
					temp_cell->prev = temp->prev;
					if (temp->prev) {
						temp->prev->next = temp_cell;
					}
					else {
						start = temp_cell;
					}
					temp->prev = temp_cell;
				}
				break;
			}
			//temp After
			else {
				if (res > 0) {
					temp_cell = (file_cell*)memalloc(sizeof(file_cell));
					temp_cell->next = temp;
					temp_cell->prev = temp->prev;
					if (temp->prev) {
						temp->prev->next = temp_cell;
					}
					else {
						start = temp_cell;
					}
					temp->prev = temp_cell;
					break;
				}
				else {
					if (i) {
						temp = temp->next;
					}
					else {
						temp_cell = (file_cell*)memalloc(sizeof(file_cell));
						temp_cell->next = NULL;
						temp_cell->prev = (file_cell*)temp;
						end = (file_cell*)temp_cell;
						temp->next = (file_cell*)temp_cell;
					}
				}
			}
		}
		space_occupied += content_size;
		temp_cell->build(content, content_size, filename, filename_size);
		size++;
		temp_cell->data.parent = this;
		parent = get_folder();
		while (parent) {
			arc = parent->parent;
			if (arc) {
				arc->space_occupied += content_size;
				parent = arc->get_folder();
			}
			else { break; }
		}
		wprintf(L"Added %.*s \n", temp_cell->data.name_size, temp_cell->data.name);
		return 0;
	}
	
	int add(const wchar_t* filename) {
		FILE* input = _wfopen(filename, L"rb");
		if (!input) {
			last_error.error_return.code = 0x5556;
			return 2;
		}
		fseek(input, 0, SEEK_END);
		size_t filename_size = wcslen(filename), filename_size2= filename_size, content_size = _ftelli64(input);
		fseek(input, 0, SEEK_SET);
		filename += filename_size2 - 1;
		while (filename_size2-- && *filename!=L'/'&& *filename != L'\\') {
			filename--;
		}
		filename++;
		filename_size2 = filename_size - filename_size2 -1;
		folder* parent;
		xaf* arc;
		if (size == 0) {
			start = (file_cell*)memalloc(sizeof(file) + PTRSIZE * 2);
			end = start;
			//start->data.build(f->content, f->content_size, f->name, f->name_size);
			start->data.filetype = XAF_FILE;
			start->data.struct_size = sizeof(file);
			start->data.name_size = filename_size2;
			start->data.name = (wchar_t*)memalloc(filename_size * sizeof(wchar_t));
			memcpy(start->data.name, filename, filename_size * sizeof(wchar_t));
			start->data.content = memalloc(content_size);
			fread(start->data.content, content_size, 1, input);
			start->data.content_size = content_size;
			start->next = 0;
			start->prev = 0;
			size = 1;
			start->data.parent = this;
			parent = get_folder();
			while (parent) {
				arc = parent->parent;
				if (arc) {
					arc->space_occupied += content_size;
					parent = arc->get_folder();
				}
				else { break; }
			}
			space_occupied += content_size;
			wprintf(L"Added %.*s ---\n", filename_size, filename);
			return 0;
		}
		file_cell* temp_cell=NULL,
			* temp = start;
		UINT64 i = size, len;
		int res;

		while (i--) {
			len = temp->data.name_size > filename_size ? filename_size : temp->data.name_size;
			//wprintf(L"len %u %u\n", temp->data.name_size, f->name_size);
			res = filename_cmp(temp->data.name, filename, len);
			//temp Before
			if (res == 0) {
				if (temp->data.name_size == filename_size) {
					wprintf(L"There is already a %s called %.*s", temp->data.filetype == XAF_FOLDER ? L"folder" : L"file"
						, temp->data.name_size, temp->data.name);
					last_error.error_return.code = 0x5001;
					last_error.error_return.data = temp;
					fclose(input);
					return 1;
				}

				temp_cell = (file_cell*)memalloc(sizeof(file_cell));
				if (temp->data.name_size < filename_size) {	//FOUND2		AFTER
					temp_cell->next = temp->next;
					temp_cell->prev = temp;
					if (temp->next) {
						temp->next->prev = temp_cell;
					}
					else {
						end = temp_cell;
					}
					temp->next = temp_cell;
				}
				else {							//BEFORE
					temp_cell->next = temp;
					temp_cell->prev = temp->prev;
					if (temp->prev) {
						temp->prev->next = temp_cell;
					}
					else {
						start = temp_cell;
					}
					temp->prev = temp_cell;
				}
				break;
			}
			//temp After
			else {
				if (res > 0) {
					temp_cell = (file_cell*)memalloc(sizeof(file_cell));
					temp_cell->next = temp;
					temp_cell->prev = temp->prev;
					if (temp->prev) {
						temp->prev->next = temp_cell;
					}
					else {
						start = temp_cell;
					}
					temp->prev = temp_cell;
					break;
				}
				else {
					if (i) {
						temp = temp->next;
					}
					else {
						temp_cell = (file_cell*)memalloc(sizeof(file_cell));
						temp_cell->next = NULL;
						temp_cell->prev = (file_cell*)temp;
						end = (file_cell*)temp_cell;
						temp->next = (file_cell*)temp_cell;
					}
				}
			}
		}
		//1 New cell is inserted before
		//>1 New cell is inserted after
		//printf("found:%i\n",found);
		/*
		if (found == 2) {
			temp_cell->next = temp->next;
			temp_cell->prev = temp;
			if (temp->next) {
				temp->next->prev = temp_cell;
			}
			else {
				end = temp_cell;
			}
			temp->next = temp_cell;
		}
		else {
			temp_cell->next = temp;
			temp_cell->prev = temp->prev;
			if (temp->prev) {
				temp->prev->next = temp_cell;
			}
			else {
				start = temp_cell;
			}
			temp->prev = temp_cell;
		}
		*/
		space_occupied += content_size;
		temp_cell->data.parent = this;
		parent = get_folder();
		while (parent) {
			arc = parent->parent;
			if (arc) {
				arc->space_occupied += content_size;
				parent = arc->get_folder();
			}
			else { break; }
		}
		temp_cell->data.filetype = XAF_FILE;
		temp_cell->data.struct_size = sizeof(file);
		temp_cell->data.name_size = filename_size2;
		temp_cell->data.name = (wchar_t*)memalloc(filename_size * sizeof(wchar_t));
		memcpy(temp_cell->data.name, filename, filename_size * sizeof(wchar_t));
		temp_cell->data.content = memalloc(content_size);
		fread(temp_cell->data.content, content_size, 1, input);
		temp_cell->data.content_size = content_size;
		size++;
		wprintf(L"Added %.*s \n", temp_cell->data.name_size, temp_cell->data.name);
		return 0;
	}

	

	struct __xaf_directory_wcstr {
		xaf* ptr = 0;
		wchar_t* name = 0;
		size_t name_size = 0;
	};

#ifdef _WINDOWS_
	
	

	struct __xaf_bytes_upd {
		xaf* src =0 ;
		folder* fld = 0;
	};

	 void WINAPI folder_import(const wchar_t* foldername) {

		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile(foldername, &FindFileData);
		int is_next = 1;
		size_t count = 0;
		nc::basic_linked_list<__xaf_directory_wcstr> folders_queue;
		if (hFind == INVALID_HANDLE_VALUE){
			wprintf(L"FindFirstFile failed (%d) with foldername %s\n", GetLastError(), foldername);
			return;
		}
		FindClose(hFind);
		nc::wstr current_directory,temp;
		__xaf_directory_wcstr xdw;
		
		FILE* file_input;
		size_t foldername_size = wcslen(foldername),filename_size;
		__xaf_bytes_upd xbu;
		nc::basic_linked_list<__xaf_bytes_upd> xaf_bytes_updates;
		nc::wstr end_str;
		size_t distance = distance_to_folder(foldername, foldername_size);
		end_str.copy_from(foldername+foldername_size-distance-1,distance+2);
		xdw.name_size = foldername_size + 1;
		xdw.name = (wchar_t*)memalloc(SWCHAR * xdw.name_size);
		memcpy(xdw.name, foldername, SWCHAR * xdw.name_size);
		xdw.ptr = this;
		folders_queue.add_front(&xdw);
		file_cell* fc_temp;
		folder_cell* fdc_temp;
		file* f_temp;
		folder* fd_temp;
		xaf* current_arc;
		size_t dir_size ;
		while (folders_queue.count) {
			xdw = folders_queue.pop_front();
			//wprintf(L"Opening %u %s\n", xdw.name_size, xdw.name);
			current_arc = xdw.ptr;
			hFind = FindFirstFile(xdw.name, &FindFileData);
			xdw.name_size = decrease_to_folder(xdw.name, xdw.name_size);
			xdw.name[xdw.name_size] = 0;
			//current_directory.destroy();
			current_directory.copy_from(xdw.name, xdw.name_size);
			wprintf(L"Directory %u %.*s\n", current_directory.size, current_directory.size, current_directory.data);
			dir_size = xdw.name_size;
		    memfree(xdw.name);
			xdw.name_size = 0;
			do {
				if (count != 0 && count != 1) {
					current_directory.size = dir_size;
					filename_size = wcslen(FindFileData.cFileName);
					current_directory.add(FindFileData.cFileName, filename_size + 1);

					if(FindFileData.dwFileAttributes == 16) {
						wprintf(L"Adding Folder : %s\n", current_directory.data);
						fdc_temp = (folder_cell*)memalloc(sizeof(folder_cell));
						
						if (current_arc->size == 0) {
							current_arc->start = (file_cell*)fdc_temp;
							current_arc->end = (file_cell*)fdc_temp;
							memset(&fdc_temp->prev, 0, PTRSIZE * 2);
						}
						else {
							current_arc->end->next = (file_cell*)fdc_temp;
							fdc_temp->prev = (folder_cell*)end;
							current_arc->end = (file_cell*)fdc_temp;
							fdc_temp->next = 0;
						}
						current_arc->size++;
						fd_temp = &fdc_temp->data;
						fd_temp->build(FindFileData.cFileName, filename_size);
						xdw.ptr =(xaf*) &fd_temp->files;
						current_directory.size--;
						current_directory.add(end_str);
						xdw.name = (wchar_t*)memalloc(current_directory.size * SWCHAR);
						memcpy(xdw.name, current_directory.data, current_directory.size * SWCHAR);
						xdw.name_size = current_directory.size;
						folders_queue.add_front(&xdw);

						xbu.src = current_arc;
						xbu.fld =fd_temp;
						xaf_bytes_updates.add_front(&xbu);
					}
					if(FindFileData.dwFileAttributes==32 ) {
						wprintf(L"Adding File : %s\n", current_directory.data);

						fc_temp = (file_cell*)memalloc(sizeof(file_cell));
						
						if (current_arc->size == 0) {
							current_arc->start = fc_temp;
							current_arc->end = fc_temp;
							memset(&fc_temp->prev, 0, PTRSIZE * 2);
						}
						else{
							current_arc->end->next = fc_temp;
							fc_temp->prev = current_arc->end;
							current_arc->end = fc_temp;
							fc_temp->next = 0;
						}
						current_arc->size++;
						f_temp = &fc_temp->data;
						f_temp->filetype = XAF_FILE;
						f_temp->struct_size = sizeof(file);
						f_temp->parent = current_arc;
						f_temp->name_size = filename_size;
						f_temp->name = (wchar_t*)memalloc(sizeof(wchar_t) * f_temp->name_size);
						memcpy(f_temp->name, FindFileData.cFileName, sizeof(wchar_t) * f_temp->name_size);

						f_temp->content_size = FindFileData.nFileSizeHigh<<32 | FindFileData.nFileSizeLow;
						current_arc->space_occupied += f_temp->content_size;
						file_input = _wfopen(current_directory.data, L"rb");
						f_temp->content = memalloc(f_temp->content_size);
						fread(f_temp->content, f_temp->content_size, 1, file_input);
						fclose(file_input);
						

					}
					
				}
				is_next = FindNextFile(hFind, &FindFileData);
				count++;
				
			} while (is_next);
			FindClose(hFind);
			count = 0;
			//current_arc->print();
			puts("");
		}
		
		while (xaf_bytes_updates.count) {
			xbu = xaf_bytes_updates.pop_front();
			xbu.src->space_occupied += xbu.fld->files.space_occupied;
		}
		
		xaf_bytes_updates.destroy();
		current_directory.destroy();
		folders_queue.destroy();
		end_str.destroy();
	}

	void WINAPI folder_import_no_logs(const wchar_t* foldername) {

		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile(foldername, &FindFileData);
		int is_next = 1;
		size_t count = 0;
		nc::basic_linked_list<__xaf_directory_wcstr> folders_queue;
		if (hFind == INVALID_HANDLE_VALUE) {
			wprintf(L"FindFirstFile failed (%d) with foldername %s\n", GetLastError(), foldername);
			return;
		}
		FindClose(hFind);
		nc::wstr current_directory, temp;
		__xaf_directory_wcstr xdw;

		FILE* file_input;
		size_t foldername_size = wcslen(foldername), filename_size;
		__xaf_bytes_upd xbu;
		nc::basic_linked_list<__xaf_bytes_upd> xaf_bytes_updates;

		xdw.name_size = foldername_size + 1;
		xdw.name = (wchar_t*)memalloc(SWCHAR * xdw.name_size);
		memcpy(xdw.name, foldername, SWCHAR * xdw.name_size);
		xdw.ptr = this;
		folders_queue.add_front(&xdw);
		file_cell* fc_temp;
		folder_cell* fdc_temp;
		file* f_temp;
		folder* fd_temp;
		xaf* current_arc;
		size_t dir_size;
		while (folders_queue.count) {
			xdw = folders_queue.pop_front();
			current_arc = xdw.ptr;
			hFind = FindFirstFile(xdw.name, &FindFileData);
			xdw.name_size -= 2;
			//xdw.name[xdw.name_size] = 0;
			//current_directory.destroy();
			current_directory.copy_from(xdw.name, xdw.name_size);
			dir_size = xdw.name_size;
			memfree(xdw.name);
			xdw.name_size = 0;
			do {
				if (count != 0 && count != 1) {
					current_directory.size = dir_size;
					filename_size = wcslen(FindFileData.cFileName);
					current_directory.add(FindFileData.cFileName, filename_size + 1);

					if (FindFileData.dwFileAttributes == 16) {
						fdc_temp = (folder_cell*)memalloc(sizeof(folder_cell));
						if (current_arc->size == 0) {
							current_arc->start = (file_cell*)fdc_temp;
							current_arc->end = (file_cell*)fdc_temp;
							memset(&fdc_temp->prev, 0, PTRSIZE * 2);
						}
						else {
							current_arc->end->next = (file_cell*)fdc_temp;
							fdc_temp->prev = (folder_cell*)end;
							current_arc->end = (file_cell*)fdc_temp;
							fdc_temp->next = 0;
						}
						current_arc->size++;
						fd_temp = &fdc_temp->data;
						fd_temp->build(FindFileData.cFileName, filename_size);
						xdw.ptr = (xaf*)&fd_temp->files;
						current_directory.size--;
						current_directory.add(L"\\*", 3);
						xdw.name = (wchar_t*)memalloc(current_directory.size * SWCHAR);
						memcpy(xdw.name, current_directory.data, current_directory.size * SWCHAR);
						xdw.name_size = current_directory.size;
						folders_queue.add_front(&xdw);

						xbu.src = current_arc;
						xbu.fld = fd_temp;
						xaf_bytes_updates.add_front(&xbu);
					}
					if (FindFileData.dwFileAttributes == 32) {

						fc_temp = (file_cell*)memalloc(sizeof(file_cell));

						if (current_arc->size == 0) {
							current_arc->start = fc_temp;
							current_arc->end = fc_temp;
							memset(&fc_temp->prev, 0, PTRSIZE * 2);
						}
						else {
							current_arc->end->next = fc_temp;
							fc_temp->prev = current_arc->end;
							current_arc->end = fc_temp;
							fc_temp->next = 0;
						}
						current_arc->size++;
						f_temp = &fc_temp->data;
						f_temp->filetype = XAF_FILE;
						f_temp->struct_size = sizeof(file);
						f_temp->parent = current_arc;
						f_temp->name_size = filename_size;
						f_temp->name = (wchar_t*)memalloc(sizeof(wchar_t) * f_temp->name_size);
						memcpy(f_temp->name, FindFileData.cFileName, sizeof(wchar_t) * f_temp->name_size);

						f_temp->content_size = FindFileData.nFileSizeHigh << 32 | FindFileData.nFileSizeLow;
						current_arc->space_occupied += f_temp->content_size;
						file_input = _wfopen(current_directory.data, L"rb");
						f_temp->content = memalloc(f_temp->content_size);
						fread(f_temp->content, f_temp->content_size, 1, file_input);
						fclose(file_input);


					}

				}
				is_next = FindNextFile(hFind, &FindFileData);
				count++;

			} while (is_next);
			FindClose(hFind);
			count = 0;
		}

		while (xaf_bytes_updates.count) {
			xbu = xaf_bytes_updates.pop_front();
			xbu.src->space_occupied += xbu.fld->files.space_occupied;
		}
		xaf_bytes_updates.destroy();
		current_directory.destroy();
		folders_queue.destroy();
	}

	


#endif
#ifdef _INC_DIRECT

	void export_to(const wchar_t* destination_directory) {
		nc::wstr dir;
		wchar_t* current_directory; 
		size_t current_directory_size, cds_one;
		
		current_directory_size = GetCurrentDirectoryW(0, 0);

		cds_one = current_directory_size + 1;
		current_directory = (wchar_t*)memalloc(sizeof(wchar_t) * cds_one);
		GetCurrentDirectoryW(cds_one, current_directory);
		current_directory[current_directory_size] = 0;


		
		size_t destination_directory_size;
		SetCurrentDirectoryW(destination_directory);
		

		nc::basic_linked_list< __xaf_directory_wcstr> xaf_dir_ll;
		__xaf_directory_wcstr xaf_dir;
		
		if (is_folder) {
			destination_directory_size = wcslen(destination_directory);
			folder* folder_src = get_folder();
			dir.add(folder_src->name, folder_src->name_size);
			dir.add(L"",1);
			_wmkdir(dir.data); 
			dir.add_front(destination_directory,destination_directory_size);
			SetCurrentDirectoryW(dir.data);
			dir.destroy();
		}
		FILE* output;
		xaf_dir.ptr = this;
		xaf_dir.name = 0;
		xaf_dir.name_size = 0;
		xaf_dir_ll.add_front(&xaf_dir);
		file_cell* current_cell;
		folder* current_folder;
		file* current_file;
		char* backspaces = (char*)memalloc(1024);
		memset(backspaces, 8, 1024);
		int backspace_count = 0;

		while (xaf_dir_ll.count) {
			xaf_dir = xaf_dir_ll.pop_front();
			current_cell = xaf_dir.ptr->start;
			dir.copy_from(xaf_dir.name, xaf_dir.name_size);
			memfree(xaf_dir.name);
			destination_directory_size= xaf_dir.name_size;
			while (current_cell) {
				current_file = &current_cell->data;
				dir.add(current_file->name, current_file->name_size);
				dir.add(L"", 1);
				while (backspace_count) {
					if (backspace_count > 1024) {
						_write(1, backspaces, 1024);
						backspace_count -= 1024;
					}
					else {
						_write(1, backspaces, backspace_count);
						backspace_count = 0;
					}
				}
				backspace_count=wprintf(L"Saving %.*s %s", dir.size, dir.data,
					current_file->filetype== XAF_FILE ?L"File":L"Folder");
				if (current_file->filetype == XAF_FILE) {
					output = _wfopen(dir.data, L"wb");
					fwrite(current_file->content, current_file->content_size, 1, output);
					fclose(output);
					
				}
				else {
					_wmkdir(dir.data);
					xaf_dir.name = (wchar_t*)memalloc(SWCHAR * (dir.size+1));
					xaf_dir.name_size = dir.size ;
					memcpy(xaf_dir.name,dir.data , SWCHAR * (dir.size-1));
					xaf_dir.name[dir.size - 1] = L'\\';
					xaf_dir.name[dir.size] =0;
					current_folder =(folder*) current_file;
					xaf_dir.ptr =(xaf*) &current_folder->files;
					xaf_dir_ll.add_end(&xaf_dir);
					//wprintf(L"Making %s\n", xaf_dir.name);
				}
				dir.size = destination_directory_size;
				current_cell = current_cell->next;
			}
			//puts("\n\n");
		}
		memfree(backspaces);
		dir.destroy();
		SetCurrentDirectoryW(current_directory);
		memfree(current_directory);
		puts("");
	}
#endif
	void copyto(xaf* dest) {
		UINT i = size;
		nc::basic_linked_list<__temp_xaf_pair> sub_folders;
		__temp_xaf_pair pair;
		file_cell* src_cell ,**dest_cell,*last_cell;
		pair.src =(__temp_xaf*) this;
		pair.dst = (__temp_xaf*)dest;
		sub_folders.add_front(&pair);

		while (sub_folders.count) {
			pair = sub_folders.pop_front();
			src_cell =(file_cell*) pair.src->start;
			dest_cell= (file_cell**)&pair.dst->start;
			last_cell = 0;
			while (src_cell) {
				if (src_cell->data.filetype == XAF_FILE) {
					*dest_cell = (file_cell*)memalloc(sizeof(file_cell));
					file_cell* temp_dest_cell = *dest_cell;
					temp_dest_cell->data.filetype = XAF_FILE;
					temp_dest_cell->data.struct_size = sizeof(file);
					temp_dest_cell->data.name_size= src_cell->data.name_size;
					temp_dest_cell->data.content_size = src_cell->data.content_size;
					temp_dest_cell->data.content = memalloc(src_cell->data.content_size);
					memcpy(temp_dest_cell->data.content, src_cell->data.content, src_cell->data.content_size);
					temp_dest_cell->data.name = (wchar_t*)memalloc(src_cell->data.name_size*sizeof(wchar_t));
					memcpy(temp_dest_cell->data.name, src_cell->data.name, src_cell->data.name_size * sizeof(wchar_t));
				}
				else {
					*dest_cell = (file_cell*)memalloc(sizeof(folder_cell));
					folder_cell *temp_folder_dst = (folder_cell *)*dest_cell,*temp_folder_src=(folder_cell*) src_cell;
					temp_folder_dst->data.build(temp_folder_src->data.name, temp_folder_src->data.name_size);

					pair.src =&temp_folder_src->data.files;
					pair.dst = &temp_folder_dst->data.files;
					sub_folders.add_front(&pair);
				}
				src_cell = src_cell->next;
				dest_cell = &(*dest_cell)->next;
			}
		}
		
		sub_folders.destroy();
	}
	
	struct file_depth { file_cell* file=0; UINT depth = 0; };
	
	void print() {
		file_cell* temp_cell ;
		UINT offset = 0,temp_offset;
		char underscore_offset[128];
		memset(underscore_offset, '_', 128);
		printf("XAF : count : %u  ; total size %u{\n",size, space_occupied);
		nc::basic_linked_list <file_depth> ll;
		file_depth file_depth;
		folder_cell* folder;
		if (size == 0) { printf("}"); return; }
		file_depth.file = start;
		ll.add_front(&file_depth);
		UINT conserve_depth;
		while (ll.count) {
			file_depth = ll.pop_front();
			temp_cell = file_depth.file;
			temp_offset = file_depth.depth;
			
			while (temp_offset) {
				if (temp_offset > 127) {
					write(1, underscore_offset, 128);
					temp_offset -= 128;
				}
				else {
					write(1, underscore_offset, temp_offset);
					temp_offset = 0;
				}
			}
			
			if (temp_cell->data.filetype == XAF_FILE) {
				wprintf(L"%.*s : %u bytes\n", temp_cell->data.name_size, temp_cell->data.name, temp_cell->data.content_size);
				//printf("%.*s }\n", temp_cell->data.content_size< 16? temp_cell->data.content_size:16
				//	,temp_cell->data.content);
				if (temp_cell->next) {
					file_depth.file = temp_cell->next;
					ll.add_front(&file_depth);
				}
			}
			else {
				folder = (folder_cell*)temp_cell;
				wprintf(L"%.*s : %u files %u bytes :\n", folder->data.name_size, folder->data.name, folder->data.files.size,
					folder->data.files.space_occupied);
				temp_offset++;
				conserve_depth = file_depth.depth;
				
				if (folder->next) {
					file_depth.depth = conserve_depth;
					file_depth.file = (file_cell*)folder->next;
					//printf("folder->next : %p\n", folder->next);
					ll.add_front(&file_depth);
				}
				if (folder->data.files.size) {
					file_depth.depth++;
					file_depth.file = (file_cell*)folder->data.files.start;
					//printf("folder->data.files.start : %p\n", folder->data.files.start);
					ll.add_front(&file_depth);
				}
			}
			
		}
		printf("}");
	}
	void debug() {
		file_cell* temp_cell;
		UINT offset = 0, temp_offset;
		char underscore_offset[128];
		memset(underscore_offset, '_', 128);
 		printf("XAF : count : %u  ; total size %u{\n", size, space_occupied);
		nc::basic_linked_list <file_depth> ll;
		file_depth file_depth;
		folder_cell* folder;
		if (size == 0) { wprintf(L"}"); return; }
		file_depth.file = start;
		ll.add_front(&file_depth);
		UINT conserve_depth;
		while (ll.count) {
			file_depth = ll.pop_front();
			temp_cell = file_depth.file;
			temp_offset = file_depth.depth;

			while (temp_offset) {
				if (temp_offset > 127) {
					write(1, underscore_offset, 128);
					temp_offset -= 128;
				}
				else {
					write(1, underscore_offset, temp_offset);
					temp_offset = 0;
				}
			}

			if (temp_cell->data.filetype == XAF_FILE) {
				wprintf(L"File %.*s : %u bytes : { ", temp_cell->data.name_size, temp_cell->data.name, temp_cell->data.content_size);
				printf("%.*s }", temp_cell->data.content_size < 16 ? temp_cell->data.content_size : 16
					, temp_cell->data.content);
				printf("Filetype : %i  ; Struct Size : %u  ; Filename length : %u ; Parent : %p ; Location %p\n", (int)temp_cell->data.filetype, temp_cell->data.struct_size,
					(UINT)temp_cell->data.name_size, temp_cell->data.parent, &temp_cell->data);


				if (temp_cell->next) {
					printf("Going to next node %p -> %p\n", temp_cell, temp_cell->next);
					file_depth.file = temp_cell->next;
					ll.add_front(&file_depth);
				}
				else {
					printf("No next\n");
				}
			}
			else {
				folder = (folder_cell*)temp_cell;
				wprintf(L"Folder %.*s : %u files %u bytes :\n", folder->data.name_size, folder->data.name, folder->data.files.size,
					folder->data.files.space_occupied);
				printf("Filetype : %i  ; Struct Size : %u  ; Foldername length : %u ; Parent : %p ; Location %p\n", (int)folder->data.filetype, folder->data.struct_size,
					(UINT)folder->data.name_size, folder->data.parent, &temp_cell->data);
				temp_offset++;
				conserve_depth = file_depth.depth;
				
				
					if (folder->next) {
						file_depth.depth = conserve_depth;
						file_depth.file = (file_cell*)folder->next;
						printf("Going to next node %p -> %p\n", folder, folder->next);
						ll.add_front(&file_depth);
					}
					if (folder->data.files.size) {
						file_depth.depth++;
						file_depth.file = (file_cell*)folder->data.files.start;
						printf("Going to sub files ; First subfile : %p\n", file_depth.file);
						ll.add_front(&file_depth);
					}
			}

		}
		wprintf(L"}");
		ll.destroy();
	}
	file* travel(const wchar_t* path,size_t str_path_size) {
		const wchar_t* cursor= path,*last_cursor;
		size_t total_filename_len = 0, current_filename_len =0;
		file* file_ret;
		folder* folder_ret;
		file_cell* temp=start,*next;
		int func_ret;
		while (total_filename_len < str_path_size) {
			last_cursor = cursor;
			current_filename_len = 0;
			while (*cursor != L'/' && *cursor != L'\\' && str_path_size!= total_filename_len) {
				cursor++;
				total_filename_len++;
				current_filename_len++;
			}
			while (temp) {
				//printf("%u == %u \n", temp->data.name_size, current_filename_len);
				if (temp->data.name_size == current_filename_len) {
					func_ret = filename_cmp(last_cursor,temp->data.name, current_filename_len);
					//wprintf(L"func_ret %i %.*s %.*s\n", func_ret, current_filename_len, last_cursor,
					//	current_filename_len,temp->data.name);
					if (func_ret == 0) {
						file_ret = &temp->data;
						if (str_path_size == total_filename_len) {
							//printf("%p\n", file_ret);
							return file_ret;
						}
						else {
							if (file_ret->filetype == XAF_FOLDER) {
								folder_ret =(folder*) file_ret;
								if (folder_ret->files.size==0) {
									wprintf(L"%.*s is empty\n", file_ret->name_size, file_ret->name);
									return 0;
								}
								temp = (file_cell *)folder_ret->files.start;
								cursor ++ ;
								total_filename_len ++;
								break;
							}
							else {
								wprintf(L"%.*s is not a folder\n", file_ret->name_size, file_ret->name);
								return 0; 
							}
						}
					}
				}
				temp = temp->next;
				
				
			}
			
		}

		



	}


	file* travel(const wchar_t* path) {
		size_t str_path_size = wcslen(path);
		return travel(path, str_path_size);
	}

	file_cell* travel_cell(const wchar_t* path) {
		size_t str_path_size = wcslen(path);
		return travel_cell(path, str_path_size);
	}

	file_cell* travel_cell(const wchar_t* path, size_t str_path_size) {
		const wchar_t* cursor = path, * last_cursor;
		size_t total_filename_len = 0, current_filename_len = 0;
		file* file_ret;
		folder* folder_ret;
		file_cell* temp = start, * next;
		int func_ret;
		while (total_filename_len < str_path_size) {
			last_cursor = cursor;
			current_filename_len = 0;
			while (*cursor != L'/' && *cursor != L'\\' && str_path_size != total_filename_len) {
				cursor++;
				total_filename_len++;
				current_filename_len++;
			}
			while (temp) {
				if (temp->data.name_size == current_filename_len) {
					func_ret = filename_cmp(last_cursor, temp->data.name, current_filename_len);
					if (func_ret == 0) {
						if (str_path_size == total_filename_len) {
							return temp;
						}
						else {
							file_ret = &temp->data;
							if (file_ret->filetype == XAF_FOLDER) {
								folder_ret = (folder*)file_ret;
								if (folder_ret->files.size==0) {
									wprintf(L"%.*s is empty\n", file_ret->name_size, file_ret->name);
									return 0;
								}
								temp =(file_cell*) folder_ret->files.start;
								cursor += 2;
								total_filename_len += 2;
								break;
							}
							else {
								wprintf(L"%.*s is not a folder\n", file_ret->name_size, file_ret->name);
								return 0;
							}
						}
					}
				}
				temp = temp->next;
			}
		}
		return 0;


	}

	folder* get_folder() {
		if (is_folder) {
			return (folder*) (((UINT64*)this)-4);
		}
		else { return 0; }
	}

	int remove(const wchar_t* path) {
		size_t str_path_size = wcslen(path);
		return remove(path, str_path_size);
	}

	int remove(const wchar_t* path,size_t str_path_size) {
		file_cell *fc=travel_cell(path, str_path_size);
		folder_cell* fldc;
		folder* fparent;
		xaf* arc;
		if (fc) {

			size--;
			if (fc->prev) {
				fc->prev->next = fc->next;
			}
			else {
				start = fc->next;
			}

			if (fc->next) {
				fc->next->prev = fc->prev;
			}
			else {
				end = fc->next;
			}

			if (fc->data.filetype == XAF_FOLDER) {
				fldc = (folder_cell*)fc;
				space_occupied -= fldc->data.files.space_occupied;
				fparent = get_folder();
				while (fparent) {
					if (fparent->parent) {
						arc = fparent->parent;
						arc->space_occupied-= fldc->data.files.space_occupied;
						fparent = arc->get_folder();
					}
				}
				fldc->data.destroy();
				memfree(fldc);
				return 2;
			}
			space_occupied -= fc->data.content_size;
			fparent = get_folder();
			while (fparent) {
				if (fparent->parent) {
					arc = fparent->parent;
					arc->space_occupied -= fc->data.content_size;
					fparent = arc->get_folder();
				}
			}
			fc->destroy();
			memfree(fc);
			return 1;
		}
		return 0;
	}
	struct  data_node{
		BYTE type=0, filename_size=0;
		time_t modification_time=0;
		INT64 pos=0;
		UINT64 size = 0;
		INT64 parent = 0;
		INT64 tree_pos;
		UINT64 folder_size=0; //folder size or encryption
	};
#define DNF_FILE sizeof(data_node)	
	struct  xaf_parent_pair {
		__temp_xaf* xaf;
		INT64 parent=0;
	};

	nc::arr<data_node> save_temp(const wchar_t* filename_data_block) {
		FILE *f_data_block= _wfopen(filename_data_block, L"wb");
		nc::arr<data_node>data_tree;
		if (!f_data_block) {
			wprintf(L"Can't open %s", filename_data_block);
			return data_tree;
		}
		data_node dn_temp;
		INT64 dn_parent = -1;
		memset(&dn_temp, 0, sizeof(dn_temp));
		nc::basic_linked_list<xaf_parent_pair> file_ll;
		file_cell* temp = start;
		folder_cell* folder_temp;
		INT64 folder_pos= size * (sizeof(time_t) + sizeof(INT64) * 2 + 18)+28;
		xaf temp_arc,*temp_ptr;
		xaf_parent_pair* xpr_ptr, temp_xpr;
		temp_xpr.parent = -1;
		size_t count = 0;
		while (temp) {
			if (temp->data.filetype == XAF_FILE) {
				//wprintf(L"Saving file %.*s\n", temp->data.name_size, temp->data.name);
				dn_temp.type = XAF_FILE;
				dn_temp.pos= _ftelli64(f_data_block);
				dn_temp.size = temp->data.content_size;
				dn_temp.modification_time = time(NULL);
				dn_temp.filename_size = temp->data.name_size;
				dn_temp.parent = temp_xpr.parent;
				dn_temp.folder_size = 0;
				fwrite(temp->data.name, temp->data.name_size * sizeof(wchar_t), 1, f_data_block);
				fwrite(temp->data.content, temp->data.content_size, 1, f_data_block);
				data_tree.add(&dn_temp);
				if (temp->next) {
					temp = temp->next;
				}
				else {
					if (file_ll.count) {
						temp_xpr = file_ll.pop_front();
						temp_ptr = (xaf*)temp_xpr.xaf;
						temp = temp_ptr->start;
					}
					else {
						temp = NULL;
					}
				}
			}
			else {
				folder_temp =(folder_cell*) temp;
				//wprintf(L"Saving folder %.*s\n", (size_t)folder_temp->data.name_size, folder_temp->data.name);
				temp_ptr=(xaf*) &folder_temp->data.files;
				temp_xpr.xaf = &folder_temp->data.files;
				dn_temp.type = XAF_FOLDER;
				dn_temp.pos = _ftelli64(f_data_block);
				dn_temp.modification_time = time(NULL);
				dn_temp.size = temp_ptr->size;
				dn_temp.filename_size = folder_temp->data.name_size;
				dn_temp.parent = dn_parent;
				dn_temp.folder_size = folder_temp->data.files.space_occupied;
				data_tree.add(&dn_temp);
				temp_xpr.parent = data_tree.size - 1;
				fwrite(&folder_pos, sizeof(INT64), 1, f_data_block);
				fwrite(folder_temp->data.name, folder_temp->data.name_size * sizeof(wchar_t), 1, f_data_block);
				file_ll.add_end(&temp_xpr);
				folder_pos += dn_temp.size * (sizeof(time_t) + sizeof(INT64) * 2 + 18);
				if (folder_temp->next) {
					temp =(file_cell*) folder_temp->next;
				}
				else {
					if (file_ll.count) {
						temp_xpr = file_ll.pop_front();
						temp_ptr = (xaf*)temp_xpr.xaf;
						dn_parent = temp_xpr.parent;
						temp = temp_ptr->start;
					}
					else {
						temp = 0;
					}
				}
			}
			printf("Added data_node %s %llu\n",dn_temp.type==XAF_FOLDER?"Folder":"File", count++);
			
		}
		fflush(f_data_block);
		fclose(f_data_block);
		file_ll.destroy();
		return data_tree;
	}
	
	int save(const wchar_t* filename){
		size_t filename_l = wcslen(filename);
		nc::wstr wstr;

		wstr.alloc(4 + filename_l + 12);

		wstr.add(filename, filename_l);
		wstr.add(L"_data_block", 12);
		nc::arr<data_node> arr = save_temp(wstr.data);
		size_t arr_size_in_bytes = arr.size * (sizeof(time_t)+ sizeof(INT64)*2+18)+28;
		FILE* data_block_file,
			* output = _wfopen(filename, L"wb");
		if (!output) {
			wprintf(L"Can't open %s", filename);
			
			return 1; }
		fwrite("XAF", 4, 1, output);
		fwrite(&arr.size, 8, 1, output);
		fwrite(&size, 8, 1, output);
		fwrite(&space_occupied, 8, 1, output);
		int backspace_count=0;
		char* backspaces = (char*)memalloc(1024);
		memset(backspaces, 8, 1024);
		printf("Nodes count : %llu \n", arr.size);
		for (size_t i = 0; i != arr.size; i++) {
			arr.data[i].pos += arr_size_in_bytes;
			backspace_count=printf("Wrote data_node %llu / %llu", i, arr.size-1);
			while (backspace_count) {
				if (backspace_count > 1024) {
					_write(1, backspaces, 1024);
					backspace_count -= 1024;
				}
				else {
					_write(1, backspaces, backspace_count);
					backspace_count = 0;
				}
			}
			arr.data[i].tree_pos = _ftelli64(output);
			fwrite(&arr.data[i].type,  1, 1, output);	
			fwrite(&arr.data[i].filename_size, 1, 1, output); 
			fwrite(&arr.data[i].modification_time, sizeof(time_t), 1, output);
			fwrite(&arr.data[i].pos, sizeof(INT64)*2, 1, output);
			if (arr.data[i].parent!=-1) {
				fwrite(&arr.data[arr.data[i].parent].tree_pos, 8, 1, output);
				//fwrite(&arr.data[i].parent->tree_pos, 8, 1, output);
			}
			else {
				fwrite("\0\0\0\0\0\0\0", 8, 1, output);
			}
			fwrite(&arr.data[i].folder_size, 8 , 1, output);			
		}
		fflush(output);
		data_block_file = _wfopen(wstr.data, L"rb");
		if (!data_block_file) {
			printf("!data_block_file\n");
			memfree(backspaces);
			arr.destroy(); wstr.destroy();
			fclose(output);
			return 1;
		}
		BYTE byte_array[4096];
		int bytes_read;
		fseek(data_block_file, 0, SEEK_END);
		INT64 filesize = _ftelli64(data_block_file),filesize_total= filesize,file_read=0;
		fseek(data_block_file, 0, SEEK_SET);
		printf("\ndatablock %lli\n", filesize);
		while (filesize) {
			if (filesize >= 4096) {
				bytes_read = 4096;
				fread(byte_array, 4096, 1, data_block_file);
				filesize -= 4096;
			}
			else {
				bytes_read = filesize;
				fread(byte_array, filesize, 1, data_block_file);
				filesize = 0;
			}
			file_read += bytes_read;
			while (backspace_count) {
				if (backspace_count > 1024) {
					_write(1, backspaces, 1024);
					backspace_count -= 1024;
				}
				else {
					_write(1, backspaces, backspace_count);
					backspace_count = 0;
				}
			}
			backspace_count =printf("%lli / %lli bytes", file_read , filesize_total);
			fwrite(byte_array, bytes_read, 1, output);
		}
		fclose(data_block_file);
		fclose(output);
		memfree(backspaces);
		wstr.add_front(L"del ", 4);
		_wsystem(wstr.data);
		arr.destroy(); wstr.destroy();
		return 0;
	}

	int save(const wchar_t* filename,xaf_password *xp) {
		size_t filename_l = wcslen(filename);
		nc::wstr wstr;
		wstr.add(filename, filename_l);
		wstr.add(L"_data_block", 12);
		nc::arr<data_node> arr = save_temp(wstr.data);
		size_t arr_size_in_bytes = arr.size * (sizeof(time_t) + sizeof(INT64) * 2 + 18) + 28;
		FILE* data_block_file,
			* output = _wfopen(filename, L"wb");
		if (!output) {
			wprintf(L"Can't open %s", filename);
			return 1;
		}
		fwrite("XAF\x01", 4, 1, output);
		byte dn_temp_write[18 + sizeof(time_t) + sizeof(UINT64) * 2];
		byte header_temp_write[32];
		memcpy(header_temp_write, &arr.size,8);
		memcpy(header_temp_write+8, &size, 8);
		memcpy(header_temp_write+16, &space_occupied, 8);
		xp->encrypt(header_temp_write, 32);
		fwrite(header_temp_write,24, 1, output);
		//fwrite(&size, 8, 1, output);
		//fwrite(&space_occupied, 8, 1, output);
		int backspace_count=0;
		char* backspaces = (char*)memalloc(1024);
		memset(backspaces, 8, 1024);
		printf("Nodes count : %llu \n", arr.size);
		for (size_t i = 0; i != arr.size; i++) {
			arr.data[i].pos += arr_size_in_bytes;
			while (backspace_count) {
				if (backspace_count > 1024) {
					_write(1, backspaces, 1024);
					backspace_count -= 1024;
				}
				else {
					_write(1, backspaces, backspace_count);
					backspace_count = 0;
				}
			}
			backspace_count = printf("Wrote data_node %llu / %llu ", i, arr.size - 1);
			arr.data[i].tree_pos = _ftelli64(output);
			//fwrite(&arr.data[i].type, 1, 1, output);
			//fwrite(&arr.data[i].filename_size, 1, 1, output);
			//fwrite(&arr.data[i].modification_time, sizeof(time_t), 1, output);
			//fwrite(&arr.data[i].pos, sizeof(INT64) * 2, 1, output);
			*dn_temp_write = arr.data[i].type;
			dn_temp_write[1] = arr.data[i].filename_size;
			memcpy(dn_temp_write + 2, &arr.data[i].modification_time, sizeof(time_t));
			memcpy(dn_temp_write + 2+ sizeof(time_t), &arr.data[i].pos, sizeof(INT64) * 2);
			if (arr.data[i].parent) {
				//fwrite(&arr.data[i].parent->tree_pos, 8, 1, output);
				memcpy(dn_temp_write + 2 + sizeof(time_t)+ sizeof(INT64) * 2, &arr.data[arr.data[i].parent].tree_pos,8);
			}
			else {
				//fwrite("\0\0\0\0\0\0\0", 8, 1, output);
				memcpy(dn_temp_write + 2 + sizeof(time_t) + sizeof(INT64) * 2,"\0\0\0\0\0\0\0" , 8);
			}
			//fwrite(&arr.data[i].folder_size, 8, 1, output);
			memcpy(dn_temp_write + 10 + sizeof(time_t) + sizeof(INT64) * 2, &arr.data[i].folder_size, 8);
			xp->encrypt(dn_temp_write, sizeof(dn_temp_write));
			fwrite(&dn_temp_write, sizeof(dn_temp_write), 1, output);
		}
		fflush(output);
		data_block_file = _wfopen(wstr.data, L"rb");
		if (!data_block_file) {
			printf("!data_block_file\n");
			free(backspaces);
			arr.destroy(); wstr.destroy();
			fclose(output);
			return 1;
		}
		BYTE byte_array[4096];
		int bytes_read;
		fseek(data_block_file, 0, SEEK_END);
		INT64 filesize = _ftelli64(data_block_file), filesize_total = filesize, file_read = 0;
		fseek(data_block_file, 0, SEEK_SET);
		printf("\ndatablock %lli\n", filesize);


		while (filesize) {
			if (filesize >= 4096) {
				bytes_read = 4096;
				fread(byte_array, 4096, 1, data_block_file);
				filesize -= 4096;
			}
			else {
				bytes_read = filesize;
				fread(byte_array, filesize, 1, data_block_file);
				filesize = 0;
			}
			xp->encrypt(byte_array, bytes_read);
			file_read += bytes_read;

			while (backspace_count) {
				if (backspace_count > 1024) {
					_write(1, backspaces, 1024);
					backspace_count -= 1024;
				}
				else {
					_write(1, backspaces, backspace_count);
					backspace_count = 0;
				}
			}
			backspace_count = printf("%lli / %lli bytes", file_read, filesize_total);

			


			fwrite(byte_array, bytes_read, 1, output);
		}
		fclose(data_block_file);
		fclose(output);
		memfree(backspaces);
		wstr.add_front(L"del ", 4);
		_wsystem(wstr.data);
		arr.destroy(); wstr.destroy();
		return 0;
	}

	struct  xaf_dsize_pair {
		xaf* xaf=0;
		UINT64 desired_size = 0;
		long long pos=0;
	};


	int read(const char* filename) {
		FILE* input = fopen(filename, "rb");
		if (!input) { return 1; }
		file_cell** cursor_ptr = &start, * last_cell = 0, * cursor_cell;
		folder_cell* cursor_folder_cell;

		int xaf_signature = *(int*)"XAF", signature;
		fread(&signature, 4, 1, input);
		signature -= xaf_signature;

		if (signature & 0xffffff) {
			fclose(input);
			write(1, "This file is not a xaf file\n", 28);
			return 3;
		}

		if (signature & 0xff000000) {
			fclose(input);
			write(1, "This file is protected with a password\n", 39);
			return 2;
		}
		UINT64 total_number_of_files;
		fread(&total_number_of_files, 8, 1, input);
		printf("%llu files\n", total_number_of_files);
		fread(&size, 8, 1, input);
		printf("%llu nodes\n", size);
		fread(&space_occupied, 8, 1, input);
		printf("%llu bytes\n", space_occupied);
		data_node temp_dn;
		INT64 backup_pos;
		nc::basic_linked_list<xaf_dsize_pair> files_ll;
		UINT64 temp_size = size;
		xaf_dsize_pair xdp;
		char* backspaces = (char*)memalloc(1024);
		memset(backspaces, 8, 1024);
		int backspace_count = 0;
		xdp.xaf = this;
		for (UINT64 i = 0; i != total_number_of_files;) {
			while (temp_size--) {
				fread(&temp_dn.type, 1, 1, input);
				fread(&temp_dn.filename_size, 1, 1, input);
				fread(&temp_dn.modification_time, sizeof(time_t), 1, input);
				fread(&temp_dn.pos, sizeof(INT64) * 2, 1, input);
				while (backspace_count) {
					if (backspace_count > 1024) {
						_write(1, backspaces, 1024);
						backspace_count -= 1024;
					}
					else {
						_write(1, backspaces, backspace_count);
						backspace_count = 0;
					}
				}
				backspace_count = printf("Node %u / %u", i, total_number_of_files - 1);
				if (temp_dn.type == XAF_FILE) {
					*cursor_ptr = (file_cell*)memalloc(sizeof(file_cell));
					cursor_cell = *cursor_ptr;
					cursor_cell->prev = last_cell;
					cursor_cell->data.parent = xdp.xaf;
					cursor_cell->next = 0;
					if (last_cell) {
						last_cell->next = cursor_cell;
					}
					last_cell = cursor_cell;
					cursor_cell->data.struct_size = sizeof(file);
					cursor_cell->data.filetype = XAF_FILE;
					cursor_cell->data.name_size = temp_dn.filename_size;
					cursor_cell->data.content_size = temp_dn.size;
					backup_pos = _ftelli64(input) + 16;
					_fseeki64(input, temp_dn.pos, SEEK_SET);
					cursor_cell->data.name_size = temp_dn.filename_size;
					cursor_cell->data.name = (wchar_t*)memalloc(sizeof(wchar_t) * temp_dn.filename_size);
					fread(cursor_cell->data.name, sizeof(wchar_t) * temp_dn.filename_size, 1, input);
					//wprintf(L"Opening file : %.*s\n", cursor_cell->data.name_size, cursor_cell->data.name);
					cursor_cell->data.content = (wchar_t*)memalloc(cursor_cell->data.content_size);
					fread(cursor_cell->data.content, cursor_cell->data.content_size, 1, input);
					_fseeki64(input, backup_pos, SEEK_SET);
					cursor_ptr = &cursor_cell->next;
					//printf("pos end : %i\n", backup_pos);
				}
				else {
					*cursor_ptr = (file_cell*)memalloc(sizeof(folder_cell));
					cursor_folder_cell = (folder_cell*)*cursor_ptr;
					cursor_folder_cell->prev = (folder_cell*)last_cell;
					cursor_folder_cell->next = 0;
					cursor_folder_cell->data.files.is_folder = 1;
					cursor_folder_cell->data.parent = xdp.xaf;
					if (last_cell) {
						last_cell->next = (file_cell*)cursor_folder_cell;
					}
					last_cell = (file_cell*)cursor_folder_cell;
					fread(&xdp.xaf->space_occupied, 8, 1, input);
					backup_pos = _ftelli64(input);
					_fseeki64(input, temp_dn.pos, SEEK_SET);
					cursor_folder_cell->data.filetype = XAF_FOLDER;
					cursor_folder_cell->data.struct_size = sizeof(folder);
					cursor_folder_cell->data.name_size = temp_dn.filename_size;
					cursor_folder_cell->data.name = (wchar_t*)memalloc(sizeof(wchar_t) * temp_dn.filename_size);
					fread(&xdp.pos, 8, 1, input);
					fread(cursor_folder_cell->data.name, sizeof(wchar_t) * temp_dn.filename_size, 1, input);
					//wprintf(L"Opening folder : %.*s\n", cursor_folder_cell->data.name_size, cursor_folder_cell->data.name);
					_fseeki64(input, backup_pos, SEEK_SET);
					xdp.xaf = (xaf*)&cursor_folder_cell->data.files;
					xdp.xaf->size = temp_dn.size;
					xdp.desired_size = temp_dn.size;
					files_ll.add_front(&xdp);
					cursor_ptr = (file_cell**)&cursor_folder_cell->next;
					//printf("pos end : %i\n", backup_pos+8);
				}
				i++;
			}
			xdp.xaf->end = *cursor_ptr;
			if (files_ll.count) {
				xdp = files_ll.pop_end();
				temp_size = xdp.desired_size;
				_fseeki64(input, xdp.pos, SEEK_SET);
				cursor_ptr = &xdp.xaf->start;
				last_cell = NULL;
				//printf("going to %u size %u with %p\n", xdp.pos, xdp.desired_size, xdp.xaf);
			}
		}
		memfree(backspaces);
		fclose(input);
		return 0;
	}

	int read(const wchar_t* filename) {
		FILE *input = _wfopen(filename, L"rb");
		if (!input) { return 1; }
		file_cell** cursor_ptr=&start,*last_cell=0,*cursor_cell;
		folder_cell* cursor_folder_cell;
		
		int xaf_signature = *(int*)"XAF", signature;
     	fread(&signature,4, 1, input);
		signature -= xaf_signature;
		
		if (signature & 0xffffff) {
			fclose(input);
			write(1, "This file is not a xaf file\n", 28);
			return 3;
		}

		if (signature & 0xff000000) {
			fclose(input);
			write(1,"This file is protected with a password\n",39);
			return 2;
		}
		UINT64 total_number_of_files;
		fread(&total_number_of_files,8, 1, input);
		printf("%llu files\n", total_number_of_files);
		fread(&size, 8, 1, input);
		printf("%llu nodes\n", size);
		fread(&space_occupied, 8, 1, input);
		printf("%llu bytes\n", space_occupied);
		data_node temp_dn;
		INT64 backup_pos;
		nc::basic_linked_list<xaf_dsize_pair> files_ll;
		UINT64 temp_size = size;
		xaf_dsize_pair xdp;
		char* backspaces = (char*)memalloc(1024);
		memset(backspaces, 8, 1024);
		int backspace_count=0;
		xdp.xaf = this;
		for (UINT64 i = 0; i != total_number_of_files;) {
			while (temp_size--) {
				fread(&temp_dn.type, 1, 1, input);
				fread(&temp_dn.filename_size, 1, 1, input);
				fread(&temp_dn.modification_time, sizeof(time_t), 1, input);
				fread(&temp_dn.pos, sizeof(INT64) * 2, 1, input);
				while (backspace_count) {
					if (backspace_count > 1024) {
						_write(1, backspaces, 1024);
						backspace_count -= 1024;
					}
					else {
						_write(1, backspaces, backspace_count);
						backspace_count = 0;
					}
				}
				backspace_count=printf("Node %u / %u", i, total_number_of_files-1);
				if (temp_dn.type == XAF_FILE) {
					*cursor_ptr = (file_cell*)memalloc(sizeof(file_cell));
					cursor_cell = *cursor_ptr;
					cursor_cell->prev = last_cell;
					cursor_cell->data.parent = xdp.xaf;
					cursor_cell->next = 0;
					if (last_cell) {
						last_cell->next = cursor_cell;
					}
					last_cell = cursor_cell;
					cursor_cell->data.struct_size = sizeof(file);
					cursor_cell->data.filetype = XAF_FILE;
					cursor_cell->data.name_size = temp_dn.filename_size;
					cursor_cell->data.content_size = temp_dn.size;
					backup_pos = _ftelli64(input)+16;
					_fseeki64(input, temp_dn.pos, SEEK_SET);
					cursor_cell->data.name_size = temp_dn.filename_size;
					cursor_cell->data.name = (wchar_t*)memalloc(sizeof(wchar_t) * temp_dn.filename_size);
					fread(cursor_cell->data.name, sizeof(wchar_t) * temp_dn.filename_size, 1, input);
					//wprintf(L"Opening file : %.*s\n", cursor_cell->data.name_size, cursor_cell->data.name);
					cursor_cell->data.content = (wchar_t*)memalloc(cursor_cell->data.content_size);
					fread(cursor_cell->data.content, cursor_cell->data.content_size, 1, input);
					_fseeki64(input, backup_pos , SEEK_SET);
					cursor_ptr =&cursor_cell->next;
					//printf("pos end : %i\n", backup_pos);
				}
				else {
					*cursor_ptr = (file_cell*)memalloc(sizeof(folder_cell));
					cursor_folder_cell = (folder_cell*)*cursor_ptr;
					cursor_folder_cell->prev = (folder_cell*)last_cell;
					cursor_folder_cell->next = 0;
					cursor_folder_cell->data.files.is_folder = 1;
					cursor_folder_cell->data.parent = xdp.xaf;
					if (last_cell) {
						last_cell->next = (file_cell*)cursor_folder_cell;
					}
					last_cell =(file_cell*) cursor_folder_cell;
					fread(&xdp.xaf->space_occupied, 8, 1, input);
					backup_pos = _ftelli64(input);
					_fseeki64(input, temp_dn.pos, SEEK_SET);
					cursor_folder_cell->data.filetype = XAF_FOLDER;
					cursor_folder_cell->data.struct_size = sizeof(folder);
					cursor_folder_cell->data.name_size = temp_dn.filename_size;
					cursor_folder_cell->data.name = (wchar_t*)memalloc(sizeof(wchar_t)* temp_dn.filename_size);
					fread(&xdp.pos, 8, 1, input);
					fread(cursor_folder_cell->data.name, sizeof(wchar_t) * temp_dn.filename_size, 1, input);
					//wprintf(L"Opening folder : %.*s\n", cursor_folder_cell->data.name_size, cursor_folder_cell->data.name);
					_fseeki64(input, backup_pos, SEEK_SET);
					xdp.xaf = (xaf*)&cursor_folder_cell->data.files;
					xdp.xaf->size = temp_dn.size;
					xdp.desired_size = temp_dn.size;
					files_ll.add_front(&xdp);
					cursor_ptr =(file_cell**) &cursor_folder_cell->next;
					//printf("pos end : %i\n", backup_pos+8);
				}
				i++;
			}
			xdp.xaf->end = *cursor_ptr;
			if (files_ll.count) { 
				xdp = files_ll.pop_end();
				temp_size = xdp.desired_size;
				_fseeki64(input, xdp.pos, SEEK_SET);
				cursor_ptr = &xdp.xaf->start;
				last_cell =0;
				//printf("going to %u size %u with %p\n", xdp.pos, xdp.desired_size, xdp.xaf);
			}
		}
		memfree(backspaces);
		fclose(input);
		return 0;
	}

	int read(const wchar_t* filename, xaf_password* xp) {
		FILE* input = _wfopen(filename, L"rb");
		if (!input) { return 1; }
		file_cell** cursor_ptr = &start, * last_cell = 0, * cursor_cell;
		folder_cell* cursor_folder_cell;

		int xaf_signature = *(int*)"XAF", signature;
		fread(&signature, 4, 1, input);
		signature-=xaf_signature;

		if (signature & 0xffffff) {
			write(1, "This file is not a xaf file\n", 28);
			return 3;
		}

		if (((signature & 0xff000000)>>24)!=1) {
			write(1, "This file is not protected with a password\n", 39);
			return 2;
		}
		UINT64 total_number_of_files;
		byte header_temp_read[32];
		fread(header_temp_read, 8, 1, input);
		fread(header_temp_read+8, 8, 1, input);
		fread(header_temp_read+16, 8, 1, input);
		xp->decrypt(header_temp_read, 32);
		total_number_of_files = *(UINT64*)header_temp_read;
		size = *(UINT64*)(header_temp_read + 8);
		space_occupied = *(UINT64*)(header_temp_read + 16);
		printf("%llu files\n", total_number_of_files);
		printf("%llu nodes\n", size);
		printf("%llu bytes\n", space_occupied);
		data_node temp_dn;
		INT64 backup_pos;
		byte temp_dn_temp_read[10 + sizeof(time_t) + 2 * sizeof(INT64)];
		nc::basic_linked_list<xaf_dsize_pair> files_ll;
		UINT temp_size = size;
		xaf_dsize_pair xdp;
		char* backspaces = (char*)memalloc(1024);
		memset(backspaces, 8, 1024);
		int backspace_count = 0;
		xdp.xaf = this;
		for (UINT64 i = 0; i != total_number_of_files;) {
			while (temp_size--) {
				fread(temp_dn_temp_read, 1, 1, input);
				fread(&temp_dn_temp_read[1], 1, 1, input);
				fread(&temp_dn_temp_read[2], sizeof(time_t), 1, input);
				fread(&temp_dn_temp_read[2+ sizeof(time_t)], sizeof(INT64) * 2, 1, input);
				fseek(input, 8, SEEK_CUR);
				fread(&temp_dn_temp_read[2 + sizeof(time_t)+ sizeof(INT64) * 2],8 , 1, input);
				xp->decrypt(temp_dn_temp_read, sizeof(temp_dn_temp_read));
				

				/*fread(&temp_dn.type, 1, 1, input);
				fread(&temp_dn.filename_size, 1, 1, input);
				fread(&temp_dn.modification_time, sizeof(time_t), 1, input);
				fread(&temp_dn.pos, sizeof(INT64) * 2, 1, input);*/
				temp_dn.type = *temp_dn_temp_read;
				temp_dn.filename_size = temp_dn_temp_read[1];
				memcpy(&temp_dn.modification_time, temp_dn_temp_read+2, sizeof(time_t) + sizeof(INT64)*2);
				temp_dn.folder_size =*(UINT64*) &temp_dn_temp_read[2 + sizeof(time_t) + sizeof(INT64) * 2];

				while (backspace_count) {
					if (backspace_count > 1024) {
						_write(1, backspaces, 1024);
						backspace_count -= 1024;
					}
					else {
						_write(1, backspaces, backspace_count);
						backspace_count = 0;
					}
				}
				backspace_count = printf("Node %u / %u", i, total_number_of_files - 1);
				if (temp_dn.type == XAF_FILE) {
					*cursor_ptr = (file_cell*)memalloc(sizeof(file_cell));
					cursor_cell = *cursor_ptr;
					cursor_cell->prev = last_cell;
					cursor_cell->data.parent = xdp.xaf;
					cursor_cell->next = 0;
					if (last_cell) {
						last_cell->next = cursor_cell;
					}
					last_cell = cursor_cell;
					cursor_cell->data.struct_size = sizeof(file);
					cursor_cell->data.filetype = XAF_FILE;
					cursor_cell->data.name_size = temp_dn.filename_size;
					cursor_cell->data.content_size = temp_dn.size;
					backup_pos = _ftelli64(input);
					_fseeki64(input, temp_dn.pos, SEEK_SET);
					cursor_cell->data.name_size = temp_dn.filename_size;
					cursor_cell->data.name = (wchar_t*)memalloc(sizeof(wchar_t) * temp_dn.filename_size);
					fread(cursor_cell->data.name, sizeof(wchar_t) * temp_dn.filename_size, 1, input);
					xp->decrypt(cursor_cell->data.name, sizeof(wchar_t) * temp_dn.filename_size);
					//wprintf(L"Opening file : %.*s\n", cursor_cell->data.name_size, cursor_cell->data.name);
					cursor_cell->data.content = (wchar_t*)memalloc(cursor_cell->data.content_size);
					fread(cursor_cell->data.content, cursor_cell->data.content_size, 1, input);
					xp->decrypt(cursor_cell->data.content, cursor_cell->data.content_size);
					_fseeki64(input, backup_pos, SEEK_SET);
					cursor_ptr = &cursor_cell->next;
					//printf("pos end : %i\n", backup_pos);
				}
				else {
					*cursor_ptr = (file_cell*)memalloc(sizeof(folder_cell));
					cursor_folder_cell = (folder_cell*)*cursor_ptr;
					cursor_folder_cell->prev = (folder_cell*)last_cell;
					cursor_folder_cell->next = 0;
					cursor_folder_cell->data.files.is_folder = 1;
					cursor_folder_cell->data.parent = xdp.xaf;
					if (last_cell) {
						last_cell->next = (file_cell*)cursor_folder_cell;
					}
					last_cell = (file_cell*)cursor_folder_cell;
					backup_pos = _ftelli64(input);
					_fseeki64(input, temp_dn.pos, SEEK_SET);
					cursor_folder_cell->data.filetype = XAF_FOLDER;
					cursor_folder_cell->data.struct_size = sizeof(folder);
					cursor_folder_cell->data.name_size = temp_dn.filename_size;
					cursor_folder_cell->data.name = (wchar_t*)memalloc(sizeof(wchar_t) * temp_dn.filename_size);
					fread(&xdp.pos, 8, 1, input);
					xp->decrypt(&xdp.pos, 8);
					fread(cursor_folder_cell->data.name, sizeof(wchar_t) * temp_dn.filename_size, 1, input);
					xp->decrypt(cursor_folder_cell->data.name, sizeof(wchar_t) * temp_dn.filename_size);
					//wprintf(L"Opening folder : %.*s\n", cursor_folder_cell->data.name_size, cursor_folder_cell->data.name);
					_fseeki64(input, backup_pos, SEEK_SET);
					xdp.xaf = (xaf*)&cursor_folder_cell->data.files;
					//fread(&xdp.xaf->space_occupied, 8, 1, input);
					//xp->decrypt(&xdp.xaf->space_occupied,8);
					xdp.xaf->size = temp_dn.size;
					xdp.desired_size = temp_dn.size;
					files_ll.add_front(&xdp);
					cursor_ptr = (file_cell**)&cursor_folder_cell->next;
					//printf("pos end : %i\n", backup_pos + 8);
				}
				i++;
			}
			if (files_ll.count) {
				xdp = files_ll.pop_end();
				temp_size = xdp.desired_size;
				_fseeki64(input, xdp.pos, SEEK_SET);
				cursor_ptr = &xdp.xaf->start;
				last_cell = 0;
				//printf("going to %u size %u with %p\n", xdp.pos, xdp.desired_size, xdp.xaf);
			}
		}
		memfree(backspaces);
		fclose(input);
		return 0;
	}

	int read(const wchar_t* filename, nc::arr<folder_cell> *arr) {
		FILE* input = _wfopen(filename, L"rb");
		if (!input) { return 1; }
		file_cell** cursor_ptr = &start, * last_cell = 0, * cursor_cell;
		folder_cell* cursor_folder_cell;

		int xaf_signature = *(int*)"XAF", signature;
		fread(&signature, 4, 1, input);
		signature -= xaf_signature;

		if (signature & 0xffffff) {
			fclose(input);
			write(1, "This file is not a xaf file\n", 28);
			return 3;
		}

		if (signature & 0xff000000) {
			fclose(input);
			write(1, "This file is protected with a password\n", 39);
			return 2;
		}
		UINT64 total_number_of_files;
		fread(&total_number_of_files, 8, 1, input);
		printf("%llu files\n", total_number_of_files);
		fread(&size, 8, 1, input);
		printf("%llu nodes\n", size);
		fread(&space_occupied, 8, 1, input);
		printf("%llu bytes\n", space_occupied);
		data_node temp_dn;
		INT64 backup_pos;
		nc::basic_linked_list<xaf_dsize_pair> files_ll;
		UINT64 temp_size = size;
		xaf_dsize_pair xdp;
		char* backspaces = (char*)memalloc(1024);
		memset(backspaces, 8, 1024);
		int backspace_count = 0;
		xdp.xaf = this;
		for (UINT64 i = 0; i != total_number_of_files;) {
			while (temp_size--) {
				fread(&temp_dn.type, 1, 1, input);
				fread(&temp_dn.filename_size, 1, 1, input);
				fread(&temp_dn.modification_time, sizeof(time_t), 1, input);
				fread(&temp_dn.pos, sizeof(INT64) * 2, 1, input);
				while (backspace_count) {
					if (backspace_count > 1024) {
						_write(1, backspaces, 1024);
						backspace_count -= 1024;
					}
					else {
						_write(1, backspaces, backspace_count);
						backspace_count = 0;
					}
				}
				backspace_count = printf("Node %u / %u", i, total_number_of_files - 1);
				if (temp_dn.type == XAF_FILE) {
					*cursor_ptr = (file_cell*)memalloc(sizeof(file_cell));
					cursor_cell = *cursor_ptr;
					cursor_cell->prev = last_cell;
					cursor_cell->data.parent = xdp.xaf;
					cursor_cell->next = 0;
					if (last_cell) {
						last_cell->next = cursor_cell;
					}
					last_cell = cursor_cell;
					cursor_cell->data.struct_size = sizeof(file);
					cursor_cell->data.filetype = XAF_FILE;
					cursor_cell->data.name_size = temp_dn.filename_size;
					cursor_cell->data.content_size = temp_dn.size;
					backup_pos = _ftelli64(input) + 16;
					_fseeki64(input, temp_dn.pos, SEEK_SET);
					cursor_cell->data.name_size = temp_dn.filename_size;
					cursor_cell->data.name = (wchar_t*)memalloc(sizeof(wchar_t) * temp_dn.filename_size);
					fread(cursor_cell->data.name, sizeof(wchar_t) * temp_dn.filename_size, 1, input);
					//wprintf(L"Opening file : %.*s\n", cursor_cell->data.name_size, cursor_cell->data.name);
					cursor_cell->data.content = (wchar_t*)memalloc(cursor_cell->data.content_size);
					fread(cursor_cell->data.content, cursor_cell->data.content_size, 1, input);
					_fseeki64(input, backup_pos, SEEK_SET);
					cursor_ptr = &cursor_cell->next;
					//printf("pos end : %i\n", backup_pos);
				}
				else {
					*cursor_ptr = (file_cell*)memalloc(sizeof(folder_cell));
					cursor_folder_cell = (folder_cell*)*cursor_ptr;
					cursor_folder_cell->prev = (folder_cell*)last_cell;
					cursor_folder_cell->next = 0;
					cursor_folder_cell->data.files.is_folder = 1;
					cursor_folder_cell->data.parent = xdp.xaf;
					if (last_cell) {
						last_cell->next = (file_cell*)cursor_folder_cell;
					}
					last_cell = (file_cell*)cursor_folder_cell;
					fread(&xdp.xaf->space_occupied, 8, 1, input);
					backup_pos = _ftelli64(input);
					_fseeki64(input, temp_dn.pos, SEEK_SET);
					cursor_folder_cell->data.filetype = XAF_FOLDER;
					cursor_folder_cell->data.struct_size = sizeof(folder);
					cursor_folder_cell->data.name_size = temp_dn.filename_size;
					cursor_folder_cell->data.name = (wchar_t*)memalloc(sizeof(wchar_t) * temp_dn.filename_size);
					fread(&xdp.pos, 8, 1, input);
					fread(cursor_folder_cell->data.name, sizeof(wchar_t) * temp_dn.filename_size, 1, input);
					//wprintf(L"Opening folder : %.*s\n", cursor_folder_cell->data.name_size, cursor_folder_cell->data.name);
					_fseeki64(input, backup_pos, SEEK_SET);
					xdp.xaf = (xaf*)&cursor_folder_cell->data.files;
					xdp.xaf->size = temp_dn.size;
					xdp.desired_size = temp_dn.size;
					files_ll.add_front(&xdp);
					cursor_ptr = (file_cell**)&cursor_folder_cell->next;
					//printf("pos end : %i\n", backup_pos+8);
				}
				i++;
			}
			xdp.xaf->end = *cursor_ptr;
			if (files_ll.count) {
				xdp = files_ll.pop_end();
				temp_size = xdp.desired_size;
				_fseeki64(input, xdp.pos, SEEK_SET);
				cursor_ptr = &xdp.xaf->start;
				last_cell = 0;
				//printf("going to %u size %u with %p\n", xdp.pos, xdp.desired_size, xdp.xaf);
			}
		}
		memfree(backspaces);
		fclose(input);
		return 0;
	}

	void destroy() {
		file_cell* temp, * last;
		folder* folder_temp;
		nc::basic_linked_list<file_cell*> ll;
		if (!size) { return ; }
		ll.add_front(&start);
		while (ll.count) {
			temp =(file_cell*) ll.pop_front();
			while (temp) {
				if (temp->data.filetype == XAF_FILE) {
					//wprintf(L"temp : %p ; name : %.*s ; foldername length %u ; temp->next : %p\n", temp,  temp->data.name_size,
					//	temp->data.name, temp->data.name_size, temp->next);
					memfree(temp->data.name);
					memfree(temp->data.content);
				}
				else {					
					folder_temp = (folder*)&temp->data;
					//wprintf(L"temp : %p ; name : %.*s ; foldername length %u ; temp->next : %p\n", temp, folder_temp->name_size,
					//	folder_temp->name, folder_temp->name_size, temp->next);
					ll.add_front(&folder_temp->files.start);
					memfree(folder_temp->name);

				}
				last = temp;
				temp = temp->next;
				memfree(last);
			}
		}
		start = 0;
		size = 0;
		space_occupied = 0;
		end = 0;
		ll.destroy();
	}

	int contains_ptr(file* f) {
		xaf* arc=f->parent;
		folder* folder_cast;
		while(arc){
			if (arc == this) {
				return 1;
			}
			folder_cast = arc->get_folder();
			if (!folder_cast)	return 0;
			arc =(xaf*) &folder_cast->files;
		}
		return 0;
	}


	size_t count() {
		file_cell* temp ;
		nc::basic_linked_list<file_cell*>xaf_ll;
		folder* fold;
		xaf_ll.add_front(&start);
		size_t ret=size;
		while (xaf_ll.count) {
			temp = xaf_ll.pop_front();
			while (temp) {
				if (temp->data.filetype = XAF_FOLDER) {
					fold = (folder*)&temp->data;
					xaf_ll.add_front(&fold->files.start);
					ret += fold->files.size;
				}
				temp = temp->next;
			}
		}
		return ret;
	}
};


static void* th_copy(void* arg_ptr) {
	th_copy_args* tca = (th_copy_args*)arg_ptr;
	__temp_xaf_pair txp =tca->txp;
	___xaf_stack* stack = tca->stack;
	pthread_mutex_t* mutex = tca->stack_mutex;
	pthread_t* threads= tca->threads;
	th_copy_args* th_args= tca->th_args;
	size_t thread_id = tca->thread_id;

	pthread_attr_t th_attr;


	pthread_attr_init(&th_attr);
	pthread_attr_setdetachstate(&th_attr, PTHREAD_CREATE_DETACHED);
	nc::basic_linked_list<__temp_xaf_pair> sub_folders;

	file_cell* src_cur, ** dst_cur, * last_src_cur=NULL;
	sub_folders.add_front(&txp);
	size_t th_id;

	while (sub_folders.count) {

		txp = sub_folders.pop_front();
		dst_cur = (file_cell**)&txp.dst->start;
		src_cur = txp.src->start;
		txp.dst->size = txp.src->size;
		txp.dst->space_occupied = txp.src->space_occupied;
		while (src_cur) {
			*dst_cur = (file_cell*)memalloc(src_cur->data.struct_size + PTRSIZE * 2);
			file_cell* temp_dst_start = *dst_cur;
			temp_dst_start->prev = last_src_cur;
			if (last_src_cur) {
				last_src_cur->next = temp_dst_start;
			}
			temp_dst_start->next = 0;
			last_src_cur = temp_dst_start;
			if (src_cur->data.filetype == XAF_FILE) {
				temp_dst_start->build(src_cur->data.content, src_cur->data.content_size,
					src_cur->data.name, src_cur->data.name_size);
				temp_dst_start->data.parent =(xaf*) txp.dst;
				dst_cur = &temp_dst_start->next;
				src_cur = src_cur->next;
			}
			else {
				folder* temp_dst_folder_ptr = (folder*)&temp_dst_start->data,
					* temp_src_folder_ptr = (folder*)&src_cur->data;
				temp_dst_folder_ptr->parent = (xaf*)txp.dst;

				temp_dst_folder_ptr->build(temp_src_folder_ptr->name, temp_src_folder_ptr->name_size);
				txp.dst = &temp_dst_folder_ptr->files;
				txp.src = &temp_src_folder_ptr->files;
				txp.dst->size = txp.src->size;
				pthread_mutex_lock(mutex);
				if (stack->start == stack->size) {
					sub_folders.add_front(&txp);
				}
				else {
					th_id = stack->pop();
					th_args[th_id].txp = txp;
					pthread_create(threads + th_id, &th_attr, th_copy, th_args + th_id);
				}
				pthread_mutex_unlock(mutex);
				dst_cur = (file_cell**)&((folder_cell*)temp_dst_start)->next;
				src_cur = (file_cell*)((folder_cell*)src_cur)->next;
			}
		}
		*dst_cur = 0;


	}

	pthread_attr_destroy(&th_attr);



	pthread_mutex_lock(mutex);

	stack->push(thread_id);

	pthread_mutex_unlock(mutex);

	return NULL;
}





FILE* load_file_from_xaf(const wchar_t* xaf_filename, const wchar_t* filename) {
	FILE* input = _wfopen(xaf_filename, L"rb");
	int xaf_signature = *(int*)"XAF", signature;
	fread(&signature, 4, 1, input);
	signature -= xaf_signature;
	if (signature & 0xffffff) {
		write(1, "This file is not a xaf file\n", 28);
		fclose(input);
		return 0;
	}
	if (signature & 0xff000000) {
		write(1, "This file is protected with a password\n", 39);
		fclose(input);
		return 0;
	}
	INT64 input_pos;
	UINT64 size = 0;
	fseek(input, 8, SEEK_CUR);
	fread(&size, 8, 1, input);
	fseek(input, 8, SEEK_CUR);
	xaf::data_node temp_dn;
	int found = 0;
	nc::wstr wstr;
	const wchar_t* current_folder = xaf_filename;
	size_t current_folder_size = 0, xaf_filename_size = wcslen(xaf_filename);
	int ret;
	while (*xaf_filename != L'\\' && *xaf_filename != L'/' && *xaf_filename) {
		current_folder_size++; xaf_filename++;
	}
	while (size--) {
		fread(&temp_dn.type, 1, 1, input);
		fread(&temp_dn.filename_size, 1, 1, input);
		fseek(input, sizeof(time_t), SEEK_CUR);
		//fread(&temp_dn.modification_time, , 1, input);
		fread(&temp_dn.pos, sizeof(INT64) * 2, 1, input);
		if (temp_dn.filename_size == current_folder_size) {
			input_pos = _ftelli64(input);
			_fseeki64(input, temp_dn.pos, SEEK_SET);
			if (wstr.capacity < current_folder_size) {
				free(wstr.data);
				wstr.capacity = current_folder_size + NC_WSTR_GROWTH_FACTOR;
				wstr.data = (wchar_t*)memalloc(SWCHAR * wstr.capacity);
			}
			wstr.size = current_folder_size;
			fread(wstr.data, SWCHAR * current_folder_size, 1, input);
			ret = filename_cmp(wstr.data, current_folder, current_folder_size);
			if (ret == 0) {
				xaf_filename_size -= current_folder_size;
				if (xaf_filename_size == 0) {
					free(wstr.data);
					_fseeki64(input, input_pos - sizeof(INT64) * 2 - sizeof(time_t) - 2, SEEK_SET);
					return input;
				}
				xaf_filename++; xaf_filename_size--;
				current_folder = xaf_filename; current_folder_size = 0;
				while (*xaf_filename != L'\\' && *xaf_filename != L'/' && *xaf_filename) {
					current_folder_size++; xaf_filename++;
				}
			}
		}
	}


	fclose(input);
	return 0;
}

struct xaf_creator {
	struct __file_input {
		UINT filetype;
		wchar_t* name_t;
		__xaf_file_reader file;
	};
	struct __folder_input {
		UINT filetype;
		wchar_t* name_t;
		UINT64 size,
			sub_elements;

	};
	nc::arr< __file_input> files;
	void build() {

	}
};


struct xaf_22:xaf_creator{
	void huj() {
		__file_input temp;
		files.add(&temp);
	}
};


inline void __file_cell_free(void* p);


struct xaf_reader2 {
	FILE* finput=NULL;
	UINT64 current_num_of_nodes = 0, num_of_files = 0,total_data_size=0,
		current_data_size=0, start_num_of_nodes=0;
	
	struct file_cell {
		UINT64 type = 0;
		INT64 pos = 0,length=0;
		time_t modification_time = 0;


		wchar_t* name = 0; UINT64 name_size = 0;
		UINT64 parent_pos = 0, folder_size = 0;
	};
	
	nc::arr<xaf_reader2::file_cell> file_arr;
	nc::linked_list< xaf_reader2::file_cell> parents;


	int wchar_to_UTF8(wchar_t wc) {
		if (wc < 128) {
			return wc;
		}
		if (wc < 2048) {
			return 0b1100000000000000 | (wc & (0xb1111100000000)) |0b11000000| (wc & (0b111111));
		}
	}

		
		int read_xaf(const char* filename) {
			finput = fopen(filename, "rb");
			if (!finput)	return 1;
			return read_xaf_file();
		}
		int read_xaf(const wchar_t* filename){
			finput = _wfopen(filename, L"rb");
			if (!finput)	return 1;
			return read_xaf_file();
		}
		int read_xaf_file() {
			char key[4];
			fread(key, 4, 1, finput);
			if(memcmp(key,"XAF\0",4) != 0) {
				return 2;
			}
			
			//wchar_t temp_filename[256];
			BYTE temp_filename_size = 0;
			file_cell cell;
			UINT64 size;
			fread(&num_of_files, 8, 1, finput);
			//printf("%u files\n", total_number_of_files);
			fread(&size, 8, 1, finput);
			//printf("%u nodes\n", size);
			fread(&total_data_size, 8, 1, finput);
			//printf("%u bytes\n", space_occupied);
			current_num_of_nodes = size;
			current_data_size = total_data_size;
			start_num_of_nodes = size;
			file_arr.realloc(size);
			while (size--) {
				fread(&cell.type, 1, 1, finput);
				fread(&cell.name_size, 1, 1, finput);
				fread(&cell.modification_time, sizeof(time_t), 1, finput);
				fread(&cell.pos, sizeof(INT64)*2, 1, finput);			//parent
				fread(&cell.parent_pos, sizeof(UINT64)*2, 1, finput);

				INT64 comeback_pos = _ftelli64(finput);
				_fseeki64(finput, cell.pos, SEEK_SET);
				if (cell.type == XAF_FOLDER) {
					fread(&cell.pos, sizeof(INT64), 1, finput);
					
				}
				else {
					cell.pos+=sizeof(wchar_t)* cell.name_size;
				}
				cell.name = (wchar_t*)memalloc(sizeof(wchar_t) * (cell.name_size + 1));
				fread(cell.name, sizeof(wchar_t) * cell.name_size, 1, finput);
				cell.name[cell.name_size] = 0;
				_fseeki64(finput, comeback_pos, SEEK_SET);

				//https://en.wikipedia.org/wiki/UTF-8

				file_arr.add(&cell);

				//printf("%zu %hhi %hhu %lli %lli %lli %lli %lli\n", file_arr.size - 1,file_arr[file_arr.size-1].type
				//	, file_arr[file_arr.size - 1].name_size, file_arr[file_arr.size - 1].modification_time,
				//	file_arr[file_arr.size - 1].pos, file_arr[file_arr.size - 1].length, file_arr[file_arr.size - 1].parent_i, 
				//	file_arr[file_arr.size - 1].folder_size);
			}
			return 0;
		}

		__xaf_file_reader find_file(const wchar_t* path) {

			

		}


		 int find_file(const wchar_t* path,size_t path_size, __xaf_file_reader*out) {
			INT64 comeback_pos = _ftelli64(finput);
			out->file = finput;
			const wchar_t* current_filename = path;
			size_t temp_path_size = path_size;
			while (temp_path_size-- && (*current_filename) != L'/' && (*current_filename) != L'\\') {
				path++;
			}
			temp_path_size = current_filename- path;
			size_t min_wstr_len;
			UINT64 num_of_nodes=0;
			int ret;
			for (size_t i = 0; i != file_arr.size; i++) {
				if (temp_path_size == file_arr[i].name_size) {
					ret=filename_cmp(file_arr[i].name, path, temp_path_size);
					if (ret > 0) {
						return 1;
					}
					else if (ret==0) {
						
						path_size -= temp_path_size;
						if (!path_size) {
							out->size = file_arr[i].length;
							out->pos = file_arr[i].pos;
							return 0;
						}
						path_size--;
						temp_path_size++;
						_fseeki64(finput, file_arr[i].pos, SEEK_SET);
						num_of_nodes = file_arr[i].length;
					}
				}
			}
			xaf_reader2::file_cell cell;
			int not_found = 1;
			while (not_found) {
				while (num_of_nodes--) {
					fread(&cell.type, 1, 1, finput);
					fread(&cell.name_size, 1, 1, finput);
					fread(&cell.modification_time, sizeof(time_t), 1, finput);
					fread(&cell.pos, sizeof(INT64) * 2, 1, finput);			//parent
					fread(&cell.parent_pos, sizeof(UINT64) * 2, 1, finput);

					INT64 comeback_pos = _ftelli64(finput);
					_fseeki64(finput, cell.pos, SEEK_SET);
					if (cell.type == XAF_FOLDER) {
						fread(&cell.pos, sizeof(INT64), 1, finput);
					}
					else {
						cell.pos += sizeof(wchar_t) * cell.name_size;
					}
					cell.name = (wchar_t*)memalloc(sizeof(wchar_t) * (cell.name_size + 1));
					fread(cell.name, sizeof(wchar_t) * cell.name_size, 1, finput);
					cell.name[cell.name_size] = 0;
					_fseeki64(finput, comeback_pos, SEEK_SET);
					ret = filename_cmp(file_arr[num_of_nodes].name, path, temp_path_size);
					
				}
				
			}



		}
		void open_folder(size_t i) {
			//const size_t coefficient = 2 + sizeof(time_t) + sizeof(INT64) * 4;
			//size_t jump = file_arr.size* coefficient;
			parents.add_back(file_arr.data+i);
			for (size_t j = 0; j != file_arr.size; j++) {
				if(i!=j)
					memfree(file_arr[j].name);
			}
			wprintf(L"Entering %.*s %zu\n", file_arr[i].name_size, file_arr[i].name,i);
			_fseeki64(finput, file_arr[i].pos, SEEK_SET);
			
			
			file_cell cell;
			UINT64 size = file_arr[i].length;
			current_num_of_nodes = size;
			current_data_size = file_arr[i].folder_size;
			file_arr.realloc(file_arr[i].length);
			file_arr.size = 0;
			while (size--) {
				fread(&cell.type, 1, 1, finput);
				fread(&cell.name_size, 1, 1, finput);
				fread(&cell.modification_time, sizeof(time_t), 1, finput);
				fread(&cell.pos, sizeof(INT64) * 2, 1, finput);			//parent
				fread(&cell.parent_pos, sizeof(UINT64) * 2, 1, finput);
				INT64 comeback_pos = _ftelli64(finput);
				_fseeki64(finput, cell.pos, SEEK_SET);
				if (cell.type == XAF_FOLDER) {
					fread(&cell.pos, sizeof(INT64), 1, finput);

				}
				else {
					cell.pos += sizeof(wchar_t) * cell.name_size;
				}
				cell.name = (wchar_t*)memalloc(sizeof(wchar_t) * (cell.name_size + 1));
				fread(cell.name, sizeof(wchar_t) * cell.name_size, 1, finput);
				cell.name[cell.name_size] = 0;

				_fseeki64(finput, comeback_pos, SEEK_SET);

				//https://en.wikipedia.org/wiki/UTF-8

				file_arr.add(&cell);
			}

		}
		int  go_back_to_parent() {
			if (parents.size) {
				for (size_t i = 0; i != file_arr.size; i++) {
					memfree(file_arr[i].name);
				}
				file_arr.destroy();
				xaf_reader2::file_cell parent = parents.pop_back();
				UINT64 size = current_num_of_nodes;
				file_cell cell;
				if (parent.parent_pos) {
					_fseeki64(finput, parent.parent_pos, SEEK_SET);
					current_num_of_nodes = parent.length;
					current_data_size = parent.folder_size;
					size = current_num_of_nodes;
					while (size--) {
						fread(&cell.type, 1, 1, finput);
						fread(&cell.name_size, 1, 1, finput);
						fread(&cell.modification_time, sizeof(time_t), 1, finput);
						fread(&cell.pos, sizeof(INT64) * 2, 1, finput);			//parent
						fread(&cell.parent_pos, sizeof(UINT64) * 2, 1, finput);
						INT64 comeback_pos = _ftelli64(finput);
						_fseeki64(finput, cell.pos, SEEK_SET);
						if (cell.type == XAF_FOLDER) {
							fread(&cell.pos, sizeof(INT64), 1, finput);
							cell.name = (wchar_t*)memalloc(sizeof(wchar_t) * cell.name_size);
							fread(cell.name, sizeof(wchar_t) * cell.name_size, 1, finput);
						}
						else {
							cell.name = (wchar_t*)memalloc(sizeof(wchar_t) * cell.name_size);
							fread(cell.name, sizeof(wchar_t) * cell.name_size, 1, finput);
							cell.pos += sizeof(wchar_t) * cell.name_size;
						}

						_fseeki64(finput, comeback_pos, SEEK_SET);

						//https://en.wikipedia.org/wiki/UTF-8

						file_arr.add(&cell);
					}
					return 2;
				}
				_fseeki64(finput, 28, SEEK_SET);
				current_num_of_nodes = start_num_of_nodes;
				current_data_size = total_data_size;
				size = current_num_of_nodes;
				while (size--) {
					fread(&cell.type, 1, 1, finput);
					fread(&cell.name_size, 1, 1, finput);
					fread(&cell.modification_time, sizeof(time_t), 1, finput);
					fread(&cell.pos, sizeof(INT64) * 2, 1, finput);			//parent
					fread(&cell.parent_pos, sizeof(UINT64) * 2, 1, finput);
					INT64 comeback_pos = _ftelli64(finput);
					_fseeki64(finput, cell.pos, SEEK_SET);
					if (cell.type == XAF_FOLDER) {
						fread(&cell.pos, sizeof(INT64), 1, finput);

					}
					else {
						cell.pos += sizeof(wchar_t) * cell.name_size;
					}
					cell.name = (wchar_t*)memalloc(sizeof(wchar_t) * (cell.name_size + 1));
					fread(cell.name, sizeof(wchar_t) * cell.name_size, 1, finput);
					cell.name[cell.name_size] = 0;

					_fseeki64(finput, comeback_pos, SEEK_SET);

					//https://en.wikipedia.org/wiki/UTF-8

					file_arr.add(&cell);
				}
				return 1;
			}
			
			
			return 0;
		}
	
		void export_file(size_t i) {
			xaf_reader2::file_cell* cell = file_arr.data + i;
			FILE* foutput = _wfopen(cell->name, L"wb");
			wprintf(L"%.*s\n", cell->name_size, cell->name);
			char buffer[1024];
			size_t bytes_read,filesize=cell->length;
			INT64 comeback_pos = _ftelli64(finput);
			_fseeki64(finput, cell->pos, SEEK_SET);
			while (filesize) {
				bytes_read = fread(buffer, 1, 1024> filesize ? filesize :1024, finput);
				filesize -= bytes_read;
				fwrite(buffer, bytes_read, 1, foutput);
			}
			fclose(foutput);
			_fseeki64(finput, comeback_pos, SEEK_SET);
		}

		void export_file(size_t i,SDL_Renderer* renderer,SDL_Texture* loading,size_t row_w,size_t loading_w, size_t loading_h) {
			xaf_reader2::file_cell* cell = file_arr.data + i;
			FILE* foutput = _wfopen(cell->name, L"wb");
			

			size_t py = ((i / row_w)* loading_h)+80, px =((i % row_w)* loading_w)+81;
			SDL_Rect r_draw;
			r_draw.x = px;
			r_draw.y = py;
			r_draw.w = loading_w;
			wprintf(L"%.*s\n", cell->name_size, cell->name);
			char buffer[1024];
			size_t bytes_read, filesize = 0;
			INT64 comeback_pos = _ftelli64(finput);
			_fseeki64(finput, cell->pos, SEEK_SET);
			INT64 rest;
			while (filesize< cell->length) {
				rest = cell->length - filesize;
				bytes_read = fread(buffer, 1, 1024 > rest ? rest : 1024, finput);
				filesize += bytes_read;
				r_draw.h = (loading_h* filesize) / cell->length;
				SDL_RenderCopy(renderer, loading, NULL, &r_draw);
				fwrite(buffer, bytes_read, 1, foutput);
				SDL_RenderPresent(renderer);
			}
			fclose(foutput);
			_fseeki64(finput, comeback_pos, SEEK_SET);
		}

		void add_file(const wchar_t* filename) {
			size_t filename_size = wcslen(filename);
			size_t i = 0,len;
			int found = 0,ret;
			file_cell* tmp_fc;
			xaf_reader2::file_cell fc;
			fc.name_size = filename_size;
			fc.type = XAF_FILE;
			
			fc.name= (wchar_t*)memalloc(fc.name_size*sizeof(wchar_t));
			memcpy(fc.name, filename, fc.name_size * sizeof(wchar_t));
			char temp[1024];
			FILE* ftemp = fopen("XAF_TEMP_000","wb");
			while (i != file_arr.size) {
				tmp_fc = file_arr.data + i;
				len = filename_size > tmp_fc->name_size ? tmp_fc->name_size : filename_size;
				ret=filename_cmp(tmp_fc->name, filename, len);
				//1 is before i
				//2 is after i
				//3 is replace i
				if(ret){
					if (ret < 0) {
						if (i != file_arr.size - 1) {
							i++;
						}
						else {
							found = 2;
							break;
						}
					}
					else {
						found = 1;
						break;
					}
				}
				else {
					if (tmp_fc->name_size == filename_size) {
						found = 3;//REPLACE DATA
						break;
					}
					else {
						if (tmp_fc->name_size > filename_size) {
							found = 1;
							break;
						}
						else {
							if (i != file_arr.size - 1) {
								i++;
							}
							else {
								found = 2;
							}
						}
					}
				}


			}
			INT64 current_pos = _ftelli64(finput);
			switch (found) {
			case 1:
				
				break;
			case 2:

				break;
			case 3:

				break;

			}


		}

		void destroy() {
			for (size_t i = 0; i != file_arr.size; i++) {
				memfree(file_arr[i].name);
			}
			void(*destroyer)(void*) = __file_cell_free;
			parents.destroy(destroyer);
			file_arr.destroy();
			fclose(finput);
		}




};

inline void __file_cell_free(void* p) {
	xaf_reader2::file_cell* fc = (xaf_reader2::file_cell*)p;
	memfree(fc->name);
}









FILE* load_file_from_xaf(const wchar_t* xaf_filename, const wchar_t* filename,xaf_password*xp) {
	FILE* input = _wfopen(xaf_filename, L"rb");
	int xaf_signature = *(int*)"XAF", signature;
	fread(&signature, 4, 1, input);
	signature -= xaf_signature;
	if (signature & 0xffffff) {
		write(1, "This file is not a xaf file\n", 28);
		fclose(input);
		return 0;
	}
	if (signature & 0xff000000) {
		write(1, "This file is protected with a password\n", 39);
		fclose(input);
		return 0;
	}
	INT64 input_pos;
	UINT64 size = 0;
	fseek(input, 8, SEEK_CUR);
	fread(&size, 8, 1, input);
	fseek(input, 8, SEEK_CUR);
	xaf::data_node temp_dn;
	int found = 0;
	nc::wstr wstr;
	const wchar_t* current_folder = xaf_filename;
	size_t current_folder_size = 0, xaf_filename_size = wcslen(xaf_filename);
	int ret;
	while (*xaf_filename != L'\\' && *xaf_filename != L'/' && *xaf_filename) {
		current_folder_size++; xaf_filename++;
	}
	while (size--) {
		fread(&temp_dn.type, 1, 1, input);
		fread(&temp_dn.filename_size, 1, 1, input);
		fseek(input, sizeof(time_t), SEEK_CUR);
		//fread(&temp_dn.modification_time, , 1, input);
		fread(&temp_dn.pos, sizeof(INT64) * 2, 1, input);
		if (temp_dn.filename_size == current_folder_size) {
			input_pos = _ftelli64(input);
			_fseeki64(input, temp_dn.pos, SEEK_SET);
			if (wstr.capacity < current_folder_size) {
				free(wstr.data);
				wstr.capacity = current_folder_size + NC_WSTR_GROWTH_FACTOR;
				wstr.data = (wchar_t*)memalloc(SWCHAR * wstr.capacity);
			}
			wstr.size = current_folder_size;
			fread(wstr.data, SWCHAR * current_folder_size, 1, input);
			ret = filename_cmp(wstr.data, current_folder, current_folder_size);
			if (ret == 0) {
				xaf_filename_size -= current_folder_size;
				if (xaf_filename_size == 0) {
					free(wstr.data);
					_fseeki64(input, input_pos - sizeof(INT64) * 2 - sizeof(time_t) - 2, SEEK_SET);
					return input;
				}
				xaf_filename++; xaf_filename_size--;
				current_folder = xaf_filename; current_folder_size = 0;
				while (*xaf_filename != L'\\' && *xaf_filename != L'/' && *xaf_filename) {
					current_folder_size++; xaf_filename++;
				}
			}
		}
	}


	fclose(input);
	return 0;
}

struct __xaf_folder_pos {
	wchar_t* name = 0;
	size_t name_size = 0;
	INT64 pos = 0;
	UINT64 size = 0;
};

#ifdef _INC_DIRECT
int export_xaf_file(const wchar_t* xaf_filename, const wchar_t* foldername){
	__xaf_folder_pos xdp;
	FILE* input = _wfopen(xaf_filename, L"rb");
	int xaf_signature = *(int*)"XAF", signature;
	fread(&signature, 4, 1, input);
	UINT64 temp_size ;
	nc::basic_linked_list< __xaf_folder_pos> ll;
	signature -= xaf_signature;
	if (signature & 0xffffff) {
		fclose(input);
		write(1, "This file is not a xaf file\n", 28);
		return 3;
	}
	if (signature & 0xff000000) {
		fclose(input);
		write(1, "This file is protected with a password\n", 39);
		return 2;
	}
	UINT64 total_number_of_files;
	fread(&total_number_of_files, 8, 1, input);
	//printf("%u files\n", total_number_of_files);
	fread(&temp_size, 8, 1, input);
	//printf("%u nodes\n",temp_size);
	fseek(input, 8, SEEK_CUR);
	size_t current_directory_size;
	nc::wstr curr_filename;
	UINT dir_size=GetCurrentDirectoryW(0, 0),dsp= dir_size+1;
	wchar_t* dir = (wchar_t*)memalloc(SWCHAR * dsp);
	GetCurrentDirectoryW(dsp,dir);
	INT64 backup_pos;
	xaf::data_node dn;
	byte read_byte_buffer[4096];
	INT64 remaining_bytes,bytes_read;
	FILE* ftemp;
	xdp.name = 0;
	xdp.name_size = 0;
	xdp.pos = _ftelli64(input);
	xdp.size = temp_size;
	ll.add_end(&xdp);
	SetCurrentDirectoryW(foldername);
	while (total_number_of_files) {
		xdp = ll.pop_front();
		//printf("xdp : name_size %llu ; name %.*s ; pos %lli ; size %lli\n",xdp.name_size ,xdp.name_size,xdp.name,xdp.pos,xdp.size);
		if(xdp.name_size){
			curr_filename.copy_from(xdp.name, xdp.name_size);
			memfree(xdp.name);
		}
		_fseeki64(input, xdp.pos, SEEK_SET);
		current_directory_size = xdp.name_size;
		while(xdp.size--){
			fread(&dn.type, 1, 1, input);
			fread(&dn.filename_size, 1, 1, input);
			fread(&dn.modification_time, sizeof(time_t), 1, input);
			fread(&dn.pos, sizeof(INT64) * 2, 1, input);
			backup_pos = _ftelli64(input) + 16;
			_fseeki64(input, dn.pos, SEEK_SET);
			//printf("type %i ; filename_size %i ; modification_time %i ; pos %lli ; size %i\n",
			//	(int)dn.type, (int)dn.filename_size, dn.modification_time, dn.pos,dn.size);
			if (curr_filename.capacity < dn.filename_size + 1+ curr_filename.size) {
				curr_filename.reserve(curr_filename.size + dn.filename_size + 1);
			}
			fread(curr_filename.data + curr_filename.size, dn.filename_size * SWCHAR, 1, input);
			curr_filename.size += dn.filename_size;
			//wprintf(L"Node name : %u %.*s ; xdp_pos : %lli ; dn.pos : %lli \n", curr_filename.size, curr_filename.size, curr_filename.data, xdp.pos,dn.pos);
			curr_filename.add(L'\0');
			if (dn.type == XAF_FILE) {
				ftemp = _wfopen(curr_filename.data, L"wb");
				remaining_bytes = dn.size;
				while (remaining_bytes) {
					if (remaining_bytes > 4095) {
						fread(read_byte_buffer, 4096, 1, input);
						remaining_bytes -= 4096;
						bytes_read = 4096;
					}
					else {
						bytes_read = remaining_bytes;
						fread(read_byte_buffer, remaining_bytes, 1, input);
						remaining_bytes =0;
					}
					fwrite(read_byte_buffer, bytes_read, 1, ftemp);
				}
				fclose(ftemp);
			}
			else {
				_wmkdir(curr_filename.data);
				xdp.name = (wchar_t*)memalloc(SWCHAR * xdp.name_size);
				memcpy(xdp.name, curr_filename.data, SWCHAR * curr_filename.size);
				xdp.name_size = curr_filename.size;
				xdp.name[xdp.name_size -1] = L'/';
				xdp.pos = dn.pos;
				xdp.size = dn.size;
				ll.add_end(&xdp);
				curr_filename.size = current_directory_size;
			}
			_fseeki64(input, backup_pos, SEEK_SET);
			total_number_of_files--;
			curr_filename.size = current_directory_size;
		}
	}
	ll.destroy(); curr_filename.destroy();
	SetCurrentDirectoryW(dir); memfree(dir);
	fclose(input);
}


int export_xaf_file(const wchar_t* xaf_filename, const wchar_t* foldername,xaf_password*xp) {
	__xaf_folder_pos xdp;
	FILE* input = _wfopen(xaf_filename, L"rb");
	int xaf_signature = *(int*)"XAF", signature;
	fread(&signature, 4, 1, input);
	UINT64 temp_size;
	nc::basic_linked_list< __xaf_folder_pos> ll;
	signature -= xaf_signature;
	if (signature & 0xffffff) {
		fclose(input);
		write(1, "This file is not a xaf file\n", 28);
		return 3;
	}
	if (signature & 0xff000000) {
		fclose(input);
		write(1, "This file is protected with a password\n", 39);
		return 2;
	}
	UINT64 total_number_of_files;
	fread(&total_number_of_files, 8, 1, input);
	//printf("%u files\n", total_number_of_files);
	fread(&temp_size, 8, 1, input);
	//printf("%u nodes\n",temp_size);
	fseek(input, 8, SEEK_CUR);
	size_t current_directory_size;
	nc::wstr curr_filename;
	UINT dir_size = GetCurrentDirectoryW(0, 0), dsp = dir_size + 1;
	wchar_t* dir = (wchar_t*)memalloc(SWCHAR * dsp);
	GetCurrentDirectoryW(dsp, dir);
	INT64 backup_pos;
	xaf::data_node dn;
	byte read_byte_buffer[4096];
	INT64 remaining_bytes, bytes_read;
	FILE* ftemp;
	xdp.name = 0;
	xdp.name_size = 0;
	xdp.pos = _ftelli64(input);
	xdp.size = temp_size;
	ll.add_end(&xdp);
	SetCurrentDirectoryW(foldername);
	while (total_number_of_files) {
		xdp = ll.pop_front();
		//printf("xdp : name_size %llu ; name %.*s ; pos %lli ; size %lli\n",xdp.name_size ,xdp.name_size,xdp.name,xdp.pos,xdp.size);
		if (xdp.name_size) {
			curr_filename.copy_from(xdp.name, xdp.name_size);
			memfree(xdp.name);
		}
		_fseeki64(input, xdp.pos, SEEK_SET);
		current_directory_size = xdp.name_size;
		while (xdp.size--) {
			fread(&dn.type, 1, 1, input);
			fread(&dn.filename_size, 1, 1, input);
			fread(&dn.modification_time, sizeof(time_t), 1, input);
			fread(&dn.pos, sizeof(INT64) * 2, 1, input);
			backup_pos = _ftelli64(input) + 16;
			_fseeki64(input, dn.pos, SEEK_SET);
			//printf("type %i ; filename_size %i ; modification_time %i ; pos %lli ; size %i\n",
			//	(int)dn.type, (int)dn.filename_size, dn.modification_time, dn.pos,dn.size);
			if (curr_filename.capacity < dn.filename_size + 1 + curr_filename.size) {
				curr_filename.reserve(curr_filename.size + dn.filename_size + 1);
			}
			fread(curr_filename.data + curr_filename.size, dn.filename_size * SWCHAR, 1, input);
			curr_filename.size += dn.filename_size;
			//wprintf(L"Node name : %u %.*s ; xdp_pos : %lli ; dn.pos : %lli \n", curr_filename.size, curr_filename.size, curr_filename.data, xdp.pos,dn.pos);
			curr_filename.add(L'\0');
			if (dn.type == XAF_FILE) {
				ftemp = _wfopen(curr_filename.data, L"wb");
				remaining_bytes = dn.size;
				while (remaining_bytes) {
					if (remaining_bytes > 4095) {
						fread(read_byte_buffer, 4096, 1, input);
						remaining_bytes -= 4096;
						bytes_read = 4096;
					}
					else {
						bytes_read = remaining_bytes;
						fread(read_byte_buffer, remaining_bytes, 1, input);
						remaining_bytes = 0;
					}
					fwrite(read_byte_buffer, bytes_read, 1, ftemp);
				}
				fclose(ftemp);
			}
			else {
				_wmkdir(curr_filename.data);
				xdp.name = (wchar_t*)memalloc(SWCHAR * xdp.name_size);
				memcpy(xdp.name, curr_filename.data, SWCHAR * curr_filename.size);
				xdp.name_size = curr_filename.size;
				xdp.name[xdp.name_size - 1] = L'/';
				xdp.pos = dn.pos;
				xdp.size = dn.size;
				ll.add_end(&xdp);
				curr_filename.size = current_directory_size;
			}
			_fseeki64(input, backup_pos, SEEK_SET);
			total_number_of_files--;
			curr_filename.size = current_directory_size;
		}
	}
	ll.destroy(); curr_filename.destroy();
	SetCurrentDirectoryW(dir); memfree(dir);
	fclose(input); 
}
#endif
#ifdef _WINDOWS_

struct __xaf_data_node_2 {
	BYTE type = 0, filename_size = 0;
	time_t modification_time = 0;
	INT64 pos = 0;
	UINT64 size = 0;
	UINT64 parent =0;
	UINT64 folder_size = 0; //folder size or encryption
	FILE* input = 0;
	wchar_t* filename = 0;
};

struct __xaf_folder {
	nc::wstr str;
	INT64 parent =-1;
	//UINT64 size = 0;
	//UINT64 folder_size=0;
};

struct __xaf_parent_son {
	UINT64 parent = -1, son = -1;
	//UINT64 size = 0;
	//UINT64 folder_size=0;
};

int folder_to_xaf(const wchar_t* foldername,const wchar_t* xaf_filename) {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(foldername, &FindFileData);
	int is_next = 1;
	UINT64 count = 0,total_count=0;
	__xaf_data_node_2 xdn,*xdn_ptr;
	nc::arr<__xaf_data_node_2> arr;
	nc::basic_linked_list<__xaf_folder> folders_queue;
	__xaf_folder xf;
	if (hFind == INVALID_HANDLE_VALUE) {
		wprintf(L"FindFirstFile failed (%d) with foldername %s\n", GetLastError(), foldername);
		return -1;
	}
	FindClose(hFind);
	int isnext;
	__xaf_parent_son xps;
	nc::basic_linked_list<__xaf_parent_son>xps_ll;
	const size_t offset = sizeof(time_t) + sizeof(INT64) * 3 + 10;
	size_t foldername_size = wcslen(foldername),distance = distance_to_folder(foldername, foldername_size);
	nc::wstr end_str,temp;
	end_str.copy_from(foldername + foldername_size - distance - 1, distance + 2);
	xf.str.copy_from(foldername, foldername_size+1);
	xf.parent = -1;
	//wprintf(L"end_str : %llu %.*s", end_str.size, end_str.size, end_str.data);
	INT64 parent_index=-1;
	INT64 tree_pos=28;
	UINT64 file_count = 0;
	UINT64 total_amount_of_files = 0, total_files_size = 0, first_count = 0, temp_folder_size = 0;
	folders_queue.add_front(&xf);
	while (folders_queue.count) {
		xf = folders_queue.pop_end();
		parent_index = xf.parent;
		hFind=FindFirstFile(xf.str.data, &FindFileData);
		//C:\...\

		temp.copy_from(xf.str.data, decrease_to_folder(xf.str.data, xf.str.size));
		xf.str.destroy();
		//temp.data[temp.size] = 0;
		temp_folder_size = 0;
		foldername_size = temp.size;
		count = 0;
		file_count = 0;
		do{
			if (count !=0 && count != 1) {
				
				switch (FindFileData.dwFileAttributes) {	
				case 16: //Folder
					xdn.filename_size = wcslen(FindFileData.cFileName);
					xdn.filename = (wchar_t*)memalloc(SWCHAR * xdn.filename_size);
					memcpy(xdn.filename, FindFileData.cFileName, SWCHAR * xdn.filename_size);
					temp.add(xdn.filename, xdn.filename_size);
					xdn.parent = parent_index;
					xdn.modification_time = time(0);
					wprintf(L"Folder : %.*s\n", xdn.filename_size, xdn.filename);
					xdn.type = XAF_FOLDER;
					temp.add(end_str);
					xf.str.copy_from(temp);
					xdn.input = 0;
					xps.parent = parent_index;
					xps.son = arr.size;
					xf.parent = arr.size;
					xdn.folder_size = 0;
					folders_queue.add_front(&xf);
					memset(&xf,0, sizeof(xf));
					xps_ll.add_front(&xps);
					arr.add(&xdn);
					xdn.pos += SWCHAR * xdn.filename_size +  sizeof(UINT64);
					file_count++;
					
					break;
				case 32:
					xdn.filename_size = wcslen(FindFileData.cFileName);
					xdn.filename = (wchar_t*)memalloc(SWCHAR * xdn.filename_size);
					memcpy(xdn.filename, FindFileData.cFileName, SWCHAR * xdn.filename_size);
					temp.add(xdn.filename, xdn.filename_size);
					xdn.parent = parent_index;
					xdn.modification_time = time(0);
					xdn.type = XAF_FILE;
					temp.add(L'\0');
					xdn.input = _wfopen(temp.data, L"rb");
					xdn.size = (((UINT64)FindFileData.nFileSizeHigh) << 32) | FindFileData.nFileSizeLow;
					wprintf(L"File : %.*s filesize %llu\n", xdn.filename_size, xdn.filename, xdn.size);
					arr.add(&xdn);
					xdn.pos += SWCHAR * xdn.filename_size+ xdn.size;
					temp_folder_size += xdn.size;
					file_count++;
					break;
				}
				temp.size = foldername_size;
				//xdn.tree_pos = file_pos;
			}
			count++;
			isnext = FindNextFileW(hFind, &FindFileData);
		} while (isnext);
		if (parent_index == -1) {
			first_count = file_count;
			total_amount_of_files += first_count;
			total_files_size += temp_folder_size;
		}
		else{
			xdn_ptr = &arr.data[parent_index];
			xdn_ptr->size= file_count;
			total_amount_of_files += xdn_ptr->size;
			xdn_ptr->folder_size = temp_folder_size;
		}
		FindClose(hFind);
	}
	FindClose(hFind);
	while (xps_ll.count) {
		xps = xps_ll.pop_front();
		if (xps.parent == -1) {
			total_files_size+= arr[xps.son].folder_size;
		}
		else {
			arr.data[xps.parent].folder_size += arr[xps.son].folder_size;
		}
	}
	FILE* output = _wfopen(xaf_filename, L"wb");
	fwrite("XAF", 4, 1, output);
	printf("total_amount_of_files : %llu first_count : %llu total_files_size : %llu\n", total_amount_of_files, first_count, total_files_size);
	fwrite(&total_amount_of_files, 8, 1, output);
	fwrite(&first_count, 8, 1, output);
	fwrite(&total_files_size, 8, 1, output);
	folders_queue.destroy();
	end_str.destroy();
	temp.destroy();
	UINT64 to_add = arr.size * offset+28;
	for (size_t i = 0; i != arr.size; i++) {
		fwrite(&arr.data[i].type, 1, 1, output);
		fwrite(&arr.data[i].filename_size, 1, 1, output);
		fwrite(&arr.data[i].modification_time, sizeof(time_t), 1, output);
		arr.data[i].pos += to_add;
		fwrite(&arr.data[i].pos, sizeof(INT64) * 2, 1, output);
		if(arr.data[i].parent!=-1){
			parent_index = arr[i].parent * offset + 28;
			fwrite(&parent_index, 8, 1, output);
		}
		else {
			fwrite("\0\0\0\0\0\0\0\0", 8, 1, output);
		}
		fwrite(&arr.data[i].folder_size, sizeof(INT64), 1, output);
		//to_add += arr[i].filename_size * SWCHAR +
		//	(arr.data[i].type == XAF_FILE)* arr[i].size+
		//	(arr.data[i].type == XAF_FOLDER)*8;
		/*
		* arr.data[i].pos += arr_size_in_bytes;
			backspace_count=printf("Wrote data_node %llu / %llu", i, arr.size-1);
			while (backspace_count) {
				if (backspace_count > 1024) {
					_write(1, backspaces, 1024);
					backspace_count -= 1024;
				}
				else {
					_write(1, backspaces, backspace_count);
					backspace_count = 0;
				}
			}
			arr.data[i].tree_pos = _ftelli64(output);
			fwrite(&arr.data[i].type,  1, 1, output);
			fwrite(&arr.data[i].filename_size, 1, 1, output);
			fwrite(&arr.data[i].modification_time, sizeof(time_t), 1, output);
			fwrite(&arr.data[i].pos, sizeof(INT64)*2, 1, output);
			if (arr.data[i].parent!=-1) {
				fwrite(&arr.data[arr.data[i].parent].tree_pos, 8, 1, output);
				//fwrite(&arr.data[i].parent->tree_pos, 8, 1, output);
			}
			else {
				fwrite("\0\0\0\0\0\0\0", 8, 1, output);
			}
			fwrite(&arr.data[i].folder_size, 8 , 1, output);
		*/
	}
	tree_pos += first_count * offset;
	char* temp_read_buffer = (char*)memalloc(65536*sizeof(char));
	UINT64 file_size = 0;
	to_add = 0;
	for (size_t i = 0; i != arr.size; i++) {
		if (arr.data[i].type == XAF_FILE) {
			fwrite(arr[i].filename, arr[i].filename_size* SWCHAR, 1, output);
			file_size = arr[i].size;
			while (file_size) {
				if (file_size >= 65536) {
					fread(temp_read_buffer,65536 ,1, arr[i].input);
					fwrite(temp_read_buffer, 65536, 1, output);
					file_size -= 65536;
				}
				else{
					fread(temp_read_buffer, file_size, 1, arr[i].input);
					fwrite(temp_read_buffer, file_size, 1, output);
					file_size =0;
				}
			}
			fclose(arr[i].input);
		}
		else {
			fwrite(&tree_pos, sizeof(UINT64), 1, output);
			fwrite(arr[i].filename, arr[i].filename_size* SWCHAR, 1, output);
			tree_pos += arr.data[i].size * offset;
		}
		memfree(arr[i].filename);
	/*if (temp->data.filetype == XAF_FILE) {
		//wprintf(L"Saving file %.*s\n", temp->data.name_size, temp->data.name);
		dn_temp.type = XAF_FILE;
		dn_temp.pos = _ftelli64(f_data_block);
		dn_temp.size = temp->data.content_size;
		dn_temp.modification_time = time(NULL);
		dn_temp.filename_size = temp->data.name_size;
		dn_temp.parent = temp_xpr.parent;
		dn_temp.folder_size = 0;
		fwrite(temp->data.name, temp->data.name_size * sizeof(wchar_t), 1, f_data_block);
		fwrite(temp->data.content, temp->data.content_size, 1, f_data_block);
		data_tree.add(&dn_temp);
		if (temp->next) {
			temp = temp->next;
		}
		else {
			if (file_ll.count) {
				temp_xpr = file_ll.pop_front();
				temp_ptr = (xaf*)temp_xpr.xaf;
				temp = temp_ptr->start;
			}
			else {
				temp = 0;
			}
		}
	}
	else {
		folder_temp = (folder_cell*)temp;
		//wprintf(L"Saving folder %.*s\n", (size_t)folder_temp->data.name_size, folder_temp->data.name);
		temp_ptr = (xaf*)&folder_temp->data.files;
		temp_xpr.xaf = &folder_temp->data.files;
		dn_temp.type = XAF_FOLDER;
		dn_temp.pos = _ftelli64(f_data_block);
		dn_temp.modification_time = time(NULL);
		dn_temp.size = temp_ptr->size;
		dn_temp.filename_size = folder_temp->data.name_size;
		dn_temp.parent = dn_parent;
		dn_temp.folder_size = folder_temp->data.files.space_occupied;
		data_tree.add(&dn_temp);

		temp_xpr.parent = data_tree.size - 1;
		fwrite(&folder_pos, sizeof(INT64), 1, f_data_block);
		fwrite(folder_temp->data.name, folder_temp->data.name_size * sizeof(wchar_t), 1, f_data_block);
		file_ll.add_end(&temp_xpr);
		folder_pos += dn_temp.size * (sizeof(time_t) + sizeof(INT64) * 2 + 18);
		if (folder_temp->next) {
			temp = (file_cell*)folder_temp->next;
		}
		else {
			if (file_ll.count) {
				temp_xpr = file_ll.pop_front();
				temp_ptr = (xaf*)temp_xpr.xaf;
				dn_parent = temp_xpr.parent;
				temp = temp_ptr->start;
			}
			else {
				temp = 0;
			}
		}
	}*/
	}
	memfree(temp_read_buffer);
	fclose(output);
	
	arr.destroy();
}


int folder_to_xaf_password(const wchar_t* foldername, const wchar_t* xaf_filename,xaf_password*xp) {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(foldername, &FindFileData);
	int is_next = 1;
	UINT64 count = 0, total_count = 0;
	__xaf_data_node_2 xdn, * xdn_ptr;
	nc::arr<__xaf_data_node_2> arr;
	nc::basic_linked_list<__xaf_folder> folders_queue;
	__xaf_folder xf;
	if (hFind == INVALID_HANDLE_VALUE) {
		wprintf(L"FindFirstFile failed (%d) with foldername %s\n", GetLastError(), foldername);
		return -1;
	}
	FindClose(hFind);
	int isnext;
	__xaf_parent_son xps;
	nc::basic_linked_list<__xaf_parent_son>xps_ll;
	const size_t offset = sizeof(time_t) + sizeof(INT64) * 3 + 10;
	size_t foldername_size = wcslen(foldername), distance = distance_to_folder(foldername, foldername_size);
	nc::wstr end_str, temp, copy;
	end_str.copy_from(foldername + foldername_size - distance - 1, distance + 2);
	xf.str.copy_from(foldername, foldername_size + 1);
	xf.parent = -1;
	//wprintf(L"end_str : %llu %.*s", end_str.size, end_str.size, end_str.data);
	INT64 parent_index = -1;
	INT64 tree_pos = 28;
	UINT64 file_count = 0;
	UINT64 total_amount_of_files = 0, total_files_size = 0, first_count = 0, temp_folder_size = 0;
	folders_queue.add_front(&xf);
	while (folders_queue.count) {
		xf = folders_queue.pop_end();
		parent_index = xf.parent;
		hFind = FindFirstFile(xf.str.data, &FindFileData);
		//C:\...\

		temp.copy_from(xf.str.data, decrease_to_folder(xf.str.data, xf.str.size));
		xf.str.destroy();
		//temp.data[temp.size] = 0;
		temp_folder_size = 0;
		foldername_size = temp.size;
		count = 0;
		file_count = 0;
		do {
			if (count != 0 && count != 1) {
				xdn.filename_size = wcslen(FindFileData.cFileName);
				xdn.filename = (wchar_t*)memalloc(SWCHAR * xdn.filename_size);
				memcpy(xdn.filename, FindFileData.cFileName, SWCHAR * xdn.filename_size);

				temp.add(xdn.filename, xdn.filename_size);
				xdn.parent = parent_index;
				xdn.modification_time = time(0);
				switch (FindFileData.dwFileAttributes) {
				case 16: //Folder
					wprintf(L"Folder : %.*s\n", xdn.filename_size, xdn.filename);
					xdn.type = XAF_FOLDER;
					temp.add(end_str);
					xf.str.copy_from(temp);
					xdn.input = 0;
					xps.parent = parent_index;
					xps.son = arr.size;
					xf.parent = arr.size;
					xdn.folder_size = 0;
					folders_queue.add_front(&xf);
					memset(&xf, 0, sizeof(xf));
					xps_ll.add_front(&xps);
					arr.add(&xdn);
					xdn.pos += SWCHAR * xdn.filename_size + sizeof(UINT64);
					file_count++;

					break;
				case 32:
					xdn.type = XAF_FILE;
					temp.add(L'\0');
					xdn.input = _wfopen(temp.data, L"rb");
					xdn.size = (((UINT64)FindFileData.nFileSizeHigh) << 32) | FindFileData.nFileSizeLow;
					wprintf(L"File : %.*s filesize %llu\n", xdn.filename_size, xdn.filename, xdn.size);
					arr.add(&xdn);
					xdn.pos += SWCHAR * xdn.filename_size + xdn.size;
					temp_folder_size += xdn.size;
					file_count++;
					break;
				}
				temp.size = foldername_size;
				//xdn.tree_pos = file_pos;
			}
			count++;
			isnext = FindNextFileW(hFind, &FindFileData);
		} while (isnext);
		if (parent_index == -1) {
			first_count = file_count;
			total_amount_of_files += first_count;
			total_files_size += temp_folder_size;
		}
		else {
			xdn_ptr = &arr.data[parent_index];
			xdn_ptr->size = file_count;
			total_amount_of_files += xdn_ptr->size;
			xdn_ptr->folder_size = temp_folder_size;
		}
		FindClose(hFind);
	}
	while (xps_ll.count) {
		xps = xps_ll.pop_front();
		if (xps.parent == -1) {
			total_files_size += arr[xps.son].folder_size;
		}
		else {
			arr.data[xps.parent].folder_size += arr[xps.son].folder_size;
		}
	}
	FILE* output = _wfopen(xaf_filename, L"wb");
	byte header_temp_write[32];
	fwrite("XAF1", 4, 1, output);
	printf("total_amount_of_files : %llu first_count : %llu total_files_size : %llu\n", total_amount_of_files, first_count, total_files_size);
	memcpy(header_temp_write, &total_amount_of_files, 8);
	memcpy(header_temp_write+8, &first_count, 8);
	memcpy(header_temp_write + 16, &total_files_size, 8);
	xp->encrypt(header_temp_write, 32);
	fwrite(header_temp_write, 24, 1, output);
	//fwrite(&total_amount_of_files, 8, 1, output);
	//fwrite(&first_count, 8, 1, output);
	//fwrite(&total_files_size, 8, 1, output);
	folders_queue.destroy();
	end_str.destroy();
	byte dn_temp_write[18 + sizeof(time_t) + sizeof(UINT64) * 2];
	UINT64 to_add = arr.size * offset + 28;
	for (size_t i = 0; i != arr.size; i++) {
		*dn_temp_write = arr.data[i].type;
		dn_temp_write[1] = arr.data[i].filename_size;
		memcpy(dn_temp_write+2, &arr.data[i].modification_time, sizeof(time_t));
		arr.data[i].pos += to_add;
		memcpy(dn_temp_write + 2 + sizeof(time_t), &arr.data[i].pos, sizeof(INT64) * 2);
		if (arr.data[i].parent != -1) {
			parent_index = arr[i].parent * offset + 28;
			memcpy(dn_temp_write + 2 + sizeof(time_t)+ sizeof(INT64) * 2
				, &parent_index, 8);
		}
		else {
			memcpy(dn_temp_write + 2 + sizeof(time_t) + sizeof(INT64) * 2
				, "\0\0\0\0\0\0\0\0", 8);
		}
		memcpy(dn_temp_write + 10 + sizeof(time_t) + sizeof(INT64) * 2
			, &arr.data[i].folder_size, 8);
		xp->encrypt(dn_temp_write, 18 + sizeof(time_t) + sizeof(UINT64) * 2);
		fwrite(dn_temp_write, 18 + sizeof(time_t) + sizeof(UINT64) * 2, 1, output);
	}
	tree_pos += first_count * offset;
	UINT64 file_size = 0;
	to_add = 0;
	byte crypt_buffer[4096];
	byte* bptr;
	size_t cb_read,cb_to_be_read;
	for (size_t i = 0; i != arr.size; i++) {
		if (arr.data[i].type == XAF_FILE) {
			cb_to_be_read = 4096 - cb_read;
			if (cb_read > arr[i].filename_size * SWCHAR) {
				memcpy(crypt_buffer + cb_read, arr[i].filename, arr[i].filename_size* SWCHAR);
				cb_read += arr[i].filename_size * SWCHAR;
			}
			else {
				memcpy(crypt_buffer + cb_read, arr[i].filename, cb_to_be_read);
				xp->encrypt(crypt_buffer, 4096);
				fwrite(crypt_buffer, 4096, 1, output);
				cb_read = arr[i].filename_size * SWCHAR - cb_to_be_read;
				memcpy(crypt_buffer, arr[i].filename, cb_read);
			}
			file_size = arr[i].size;
			while (file_size) {
				cb_to_be_read =  4096- cb_read;
				if (file_size >= cb_to_be_read) {
					fread(crypt_buffer, cb_to_be_read, 1, arr[i].input);
					xp->encrypt(crypt_buffer, 4096);
					fwrite(crypt_buffer, 4096, 1, output);
					file_size -= cb_to_be_read;
					cb_read = 0;
				}
				else {
					fread(crypt_buffer, file_size, 1, arr[i].input);
					file_size = 0;
					cb_read += file_size;
				}
			}
			fclose(arr[i].input);
		}
		else {
			cb_to_be_read = 4096 - cb_read;
			if (cb_to_be_read > sizeof(UINT64)) {
				memcpy(crypt_buffer + cb_read, &tree_pos, sizeof(UINT64));
				cb_read += sizeof(UINT64);
				cb_to_be_read -= sizeof(UINT64);
			}
			else {
				memcpy(crypt_buffer + cb_read, &tree_pos, cb_to_be_read);
				xp->encrypt(crypt_buffer, 4096);
				fwrite(crypt_buffer, 4096, 1, output);
				cb_read = sizeof(UINT64) - cb_to_be_read;
				bptr = (byte*)&tree_pos;
				memcpy(crypt_buffer, bptr+ cb_read, cb_read);
				cb_to_be_read = 4096 - cb_read;
			}

			if (cb_to_be_read > arr[i].filename_size * SWCHAR) {
				memcpy(crypt_buffer + cb_read, arr[i].filename, arr[i].filename_size * SWCHAR);
				cb_read += arr[i].filename_size * SWCHAR;
			}
			else {
				memcpy(crypt_buffer + cb_read, arr[i].filename, cb_to_be_read);
				xp->encrypt(crypt_buffer, 4096);
				fwrite(crypt_buffer, 4096, 1, output);
				cb_read = arr[i].filename_size * SWCHAR - cb_to_be_read;
				memcpy(crypt_buffer, arr[i].filename+ cb_to_be_read, cb_read);
				cb_to_be_read = 4096 - cb_read;
			}
			tree_pos += arr.data[i].size * offset;
		}
		memfree(arr[i].filename);
	}
	cb_to_be_read = 4096 - cb_read;
	xp->encrypt(crypt_buffer, cb_to_be_read);
	fwrite(crypt_buffer, cb_to_be_read, 1, output);
	fclose(output);
	temp.destroy();
	arr.destroy();
}

#endif
int folder::add(file* f) {
	xaf* arc = (xaf*)&files;
	return arc->add(f);
}
int folder::add(folder* f) {
	xaf* arc = (xaf*)&files;
	folder_cell *t;
	return arc->add(f,&t);
}

int folder::add(const void* content, size_t content_size, const wchar_t* filename, BYTE filename_size) {
	xaf* arc = (xaf*)&files;
	return arc->add(content, content_size, filename, filename_size);
}

void folder::copyto(xaf* dest) {
	xaf* arc = (xaf*)&files;
	arc->copyto(dest);
}
void folder::print() {
	xaf* arc = (xaf*)&files;
	file_cell* temp_cell;
	UINT offset = 0, temp_offset;
	char underscore_offset[128];
	memset(underscore_offset, '_', 128);
	wprintf(L"Folder %.*s : count : %u  ; total size %u{\n", name_size, name, arc->size, arc->space_occupied);
	nc::basic_linked_list <xaf::file_depth> ll;
	xaf::file_depth file_depth;
	folder_cell* folder;
	if (arc->size == 0) { printf("}"); return; }
	file_depth.file = arc->start;
	ll.add_front(&file_depth);
	UINT conserve_depth;
	while (ll.count) {
		file_depth = ll.pop_front();
		temp_cell = file_depth.file;
		temp_offset = file_depth.depth;

		while (temp_offset) {
			if (temp_offset > 127) {
				write(1, underscore_offset, 128);
				temp_offset -= 128;
			}
			else {
				write(1, underscore_offset, temp_offset);
				temp_offset = 0;
			}
		}

		if (temp_cell->data.filetype == XAF_FILE) {
			wprintf(L"%.*s : %u bytes : { ", temp_cell->data.name_size, temp_cell->data.name, temp_cell->data.content_size);
			printf("%.*s }\n", temp_cell->data.content_size < 16 ? temp_cell->data.content_size : 16
				, temp_cell->data.content);
			if (temp_cell->next) {
				file_depth.file = temp_cell->next;
				ll.add_front(&file_depth);
			}
		}
		else {
			folder = (folder_cell*)temp_cell;
			wprintf(L"%.*s : %u files %u bytes :\n", folder->data.name_size, folder->data.name, folder->data.files.size,
				folder->data.files.space_occupied);
			temp_offset++;
			conserve_depth = file_depth.depth;
			if (folder->next) {
				file_depth.depth = conserve_depth;
				file_depth.file = (file_cell*)folder->next;
				//printf("folder->next : %p\n", folder->next);
				ll.add_front(&file_depth);
			}
			if (folder->data.files.size) {
				file_depth.depth++;
				file_depth.file = (file_cell*)folder->data.files.start;
				//printf("folder->data.files.start : %p\n", folder->data.files.start);
				ll.add_front(&file_depth);
			}
		}

	}
	printf("}");
}
file* folder::travel(const wchar_t* path, size_t str_path_size) {
	xaf* arc = (xaf*)&files;
	return arc->travel(path, str_path_size);
}

file_cell* folder::travel_cell(const wchar_t* path, size_t str_path_size) {
	xaf* arc = (xaf*)&files;
	return arc->travel_cell(path, str_path_size);
}

};