
/*#include"memalloc.h"
namespace nc {

#define NC_LINKED_CELL_DATA_SIZE		128
#define NC_LINKED_CELL_LEFT_CAPACITY	32
#define NC_LINKED_LIST_RESERVE_SIZE		4
#define NC_LINKED_LIST_SIZE				sizeof(linked_list<T>)
#define NC_LINKED_CELL_SIZE				sizeof(linked_cell<T>)
#define NC_LINKED_T_SIZE				sizeof(T)
	template<typename T> struct linked_cell {
		linked_cell<T>* next = NULL, *prev = NULL;
		size_t lpos = NC_LINKED_CELL_LEFT_CAPACITY,	//lpos is inclusive 
			rpos= NC_LINKED_CELL_LEFT_CAPACITY;		//rpos isn't
		T data[NC_LINKED_CELL_DATA_SIZE];


	};

	template<typename T> struct linked_list {
		linked_cell<T>* last = NULL, * first = NULL;
		size_t size = 0, capacity = 0;
		//linked_cell<T>* hot[3];
		linked_cell<T>* reserve[NC_LINKED_LIST_RESERVE_SIZE] = {
			(linked_cell<T>*)memalloc(NC_LINKED_CELL_SIZE),
			(linked_cell<T>*)memalloc(NC_LINKED_CELL_SIZE),
			(linked_cell<T>*)memalloc(NC_LINKED_CELL_SIZE),
			(linked_cell<T>*)memalloc(NC_LINKED_CELL_SIZE)
		};
		size_t reserve_size = NC_LINKED_LIST_RESERVE_SIZE;

		void add_front(T* data) {
			if (size == 0) {
				first = reserve[--reserve_size];
				first->next = NULL;
				first->prev = NULL;
				first->lpos = NC_LINKED_CELL_LEFT_CAPACITY;
				first->rpos = NC_LINKED_CELL_LEFT_CAPACITY+1;
				last = first;
				size++;
				capacity = NC_LINKED_CELL_DATA_SIZE - 1;
				memcpy(first->data + first->lpos, data, NC_LINKED_T_SIZE);
				return;
			}

			linked_cell<T>* temp=first;

			if (temp->lsize) {
				capacity--;
				size++;
				memcpy(temp->data+ --temp->lpos,data, NC_LINKED_T_SIZE);
				return;
			}
			
			temp = reserve_size? reserve[--reserve_size]: (linked_cell<T>*)memalloc(NC_LINKED_CELL_SIZE);
			temp->next = first;
			temp->prev = NULL;
			first->lpos = NC_LINKED_CELL_LEFT_CAPACITY;
			first->rpos = NC_LINKED_CELL_LEFT_CAPACITY+1;
			first = temp;
			size++;
			capacity += NC_LINKED_CELL_DATA_SIZE - 1;
			memcpy(first->data + first->lpos, data, NC_LINKED_T_SIZE);
			return;
			


		}
		void add_back(T* data) {
			if (size == 0) {
				first = reserve[--reserve_size];
				first->next = NULL;
				first->prev = NULL;
				first->lpos = NC_LINKED_CELL_LEFT_CAPACITY;
				first->rpos = NC_LINKED_CELL_LEFT_CAPACITY + 1;
				last = first;
				size++;
				capacity = NC_LINKED_CELL_DATA_SIZE - 1;
				memcpy(first->data + first->lpos, data, NC_LINKED_T_SIZE);
				return;
			}

			linked_cell<T>* temp = first;

			if (temp->rpos != NC_LINKED_CELL_DATA_SIZE) {
				capacity--;
				size++;
				memcpy(temp->data + temp->rpos++, data, NC_LINKED_T_SIZE);
				return;
			}

			temp = reserve_size ? reserve[--reserve_size] : (linked_cell<T>*)memalloc(NC_LINKED_CELL_SIZE);
			temp->next = first;
			temp->prev = NULL;
			first->lpos = NC_LINKED_CELL_LEFT_CAPACITY;
			first->rpos = NC_LINKED_CELL_LEFT_CAPACITY + 1;
			first = temp;
			size++;
			capacity += NC_LINKED_CELL_DATA_SIZE - 1;
			memcpy(first->data + first->lpos, data, NC_LINKED_T_SIZE);

			return;



		}
		
		T pop_front() {
			
			T ret=first->data[first->lpos++];
			if (first->lpos == first->rpos) {
				linked_cell<T>* lc = first->next;
				if (size < NC_LINKED_LIST_RESERVE_SIZE + 1) {
					reserve[reserve_size++] = first;
					if (first == last) {
						first = NULL;
						last = NULL;
					}
					else {
						first = lc;
					}
				}
				else {
					free(first);
					first = lc;
				}
			}
			return ret;
		}
		T pop_back() {
			T ret = last->data[first->rpos--];
			if (last->lpos == last->rpos) {
				linked_cell<T>* lc = last->prev;
				if (size < NC_LINKED_LIST_RESERVE_SIZE + 1) {
					reserve[reserve_size++] = last;
					if (first == last) {
						first = NULL;
						last = NULL;
					}
					else {
						last = lc;
					}
				}
				else {
					free(last);
					last = lc;
				}
			}
			return ret;
		}
		
		void destroy() {
			size = 0; capacity = 0;
			linked_cell<T>* temp = first;
			while (temp) {
				if (reserve_size != NC_LINKED_LIST_RESERVE_SIZE) {
					reserve[reserve_size++] = temp;
				}
				else {
					memfree(temp);
				}
				temp = temp->next;
			}
		}
		void destroy(void(*destroyer)(void*)) {
			size = 0; capacity = 0;
			linked_cell<T>* temp = first;
			while (temp) {
				for (size_t i = temp->lpos; i != temp->rpos; i++) {
					destroyer(temp->data[i]);
				}
				if (reserve_size != NC_LINKED_LIST_RESERVE_SIZE) {
					reserve[reserve_size++] = temp;
				}
				else {
					memfree(temp);
				}
				temp = temp->next;
			}


		}

		void add_front(T* data,size_t count) {
			size_t diff;
			linked_cell<T>* tlc,last_tlc;
			size+=count;
			if (size == 0) {
				tlc = reserve[--reserve_size];
				last_tlc = tlc;
				first = tlc;
				first->next = NULL;
				first->prev = NULL;
				while (count > NC_LINKED_CELL_DATA_SIZE) {
					tlc->lpos = 0;
					tlc->rpos = NC_LINKED_CELL_DATA_SIZE;
					memcpy(first->data, data, NC_LINKED_CELL_DATA_SIZE * NC_LINKED_T_SIZE);
					count -= NC_LINKED_CELL_DATA_SIZE;
					tlc = reserve_size ? reserve[--reserve_size] : (linked_cell<T>*)memalloc(NC_LINKED_CELL_SIZE);
					tlc->previous = last_tlc;
					last_tlc->next = tlc;
					last_tlc = tlc;
				}
				if (count) {
					tlc = reserve_size ? reserve[--reserve_size] : (linked_cell<T>*)memalloc(NC_LINKED_CELL_SIZE);
					tlc->previous = first;
					first->next = tlc;
					if (count > NC_LINKED_CELL_DATA_SIZE - NC_LINKED_CELL_LEFT_CAPACITY) {
						tlc->rpos = NC_LINKED_CELL_DATA_SIZE;
						tlc->lpos = count - NC_LINKED_CELL_DATA_SIZE + NC_LINKED_CELL_LEFT_CAPACITY;
						capacity = tlc->lpos;
					}
					else {
						tlc->rpos = NC_LINKED_CELL_LEFT_CAPACITY + count;
						tlc->lpos = NC_LINKED_CELL_LEFT_CAPACITY;
						capacity = tlc->lpos+ NC_LINKED_CELL_DATA_SIZE- tlc->rpos;
					}
				}
				last = tlc;
				return;
			}

			linked_cell<T>* temp = first;

			if (temp->lsize) {
				capacity--;
				size++;
				memcpy(temp->data + --temp->lpos, data, NC_LINKED_T_SIZE);
				return;
			}

			temp = reserve_size ? reserve[--reserve_size] : (linked_cell<T>*)memalloc(NC_LINKED_CELL_SIZE);
			temp->next = first;
			temp->prev = NULL;
			first->lpos = NC_LINKED_CELL_LEFT_CAPACITY;
			first->rpos = NC_LINKED_CELL_LEFT_CAPACITY + 1;
			first = temp;
			size++;
			capacity += NC_LINKED_CELL_DATA_SIZE - 1;
			memcpy(first->data + first->lpos, data, NC_LINKED_T_SIZE);
			return;



		}
	};


};

*/