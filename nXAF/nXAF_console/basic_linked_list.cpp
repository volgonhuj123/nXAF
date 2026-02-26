#include"memalloc.h"
#define NC_TSIZE sizeof(T)
#define NC_BCSIZE sizeof(basic_cell<T>)
#define NC_LL_ARR_SIZE 128
#define NC_LL_RESERVE_SIZE 4
namespace nc {
	template  <typename T> struct  basic_cell {
		basic_cell<T>* next = 0, * previous = 0;
		T data;
	};
	template  <typename T> struct basic_linked_list {
		basic_cell<T>* last = 0, *first = 0;
		size_t count=0;

		void add_end(T * data) {
			if (count == 0) {
				last = (basic_cell<T>*) memalloc(NC_BCSIZE);
				last->previous = 0;
				last->next = 0;
				memcpy(&last->data, data, NC_TSIZE);
				first = last;
				count = 1;
				return;
			}
			basic_cell<T>* temp= (basic_cell<T>*) memalloc(NC_BCSIZE);
			temp->next = 0;
			temp->previous = last;
			last->next = temp;
			last = temp;
			memcpy(&temp->data, data, NC_TSIZE);
			count++;
		}
		void add_front(T* data) {
			if (count == 0) {
				last =(basic_cell<T>*)memalloc(NC_BCSIZE);
				last->previous = 0;
				last->next = 0;
				memcpy(&last->data, data, NC_TSIZE);
				first = last;
				count = 1;
				return;
			}
			basic_cell<T>* temp = (basic_cell<T>*) memalloc(NC_BCSIZE);
			temp->previous = 0;
			temp->next = first;
			first->previous = temp;
			first = temp;
			memcpy(&temp->data, data, NC_TSIZE);
			count++;
		}
		T pop_front() {
			T ret = first->data;
			void* temp = first;
			first =(basic_cell<T>*) first->next;
			memfree(temp);
			count--;
			if (count == 0) {
				last = 0;
			}
			return ret;
		}
		T pop_end() {
			T ret = last->data;
			void* temp = last;
			last = last->previous;
			memfree(temp);
			count--;
			if (count == 0) {
				first = 0;
			}
			return ret;
		}
		
		void destroy() {
			if (!count) { return; }
			basic_cell<T>* temp = first, * temp_prev;
			while (count--) {
				temp_prev = temp;
				temp = temp->next;
				memfree(temp_prev);
			}
			first = 0;
			last = 0;
		}
	
		

		basic_cell<T>* operator[](size_t i) {
			basic_cell<T>* temp = first;
			while (i) { temp = temp->next; }

			return temp;
		}

	};

}
#undef NC_TSIZE
#undef NC_BCSIZE
#undef NC_LL_ARR_SIZE 
#undef NC_LL_RESERVE_SIZE