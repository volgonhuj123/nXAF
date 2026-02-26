#include <algorithm>
#include"xaf.h"
#include "QuickQueue.h"
#include "LinkedList.h"
#include "array.h"
namespace nc {
	UINT64 to_and_lut[7] = {1,3,7,15,31,63,127};
	

	struct byte_count_tree{
		UINT64 count;		//[0-255]
		UINT64 b;			//byte   data
		byte_count_tree* left=NULL, * right = NULL;
		bool is_leaf() {
			return (left == NULL) && (right == NULL);
		}

		void destroy() {
			linked_list<byte_count_tree*> ll;
			byte_count_tree *bct_t;
			if (left) {
				ll.add_front(&left);
			}
			if (right) {
				ll.add_front(&right);
			}
			while(ll.size){
				bct_t = ll.pop_front();
				if (bct_t->left) {
					ll.add_front(&bct_t->left);
				}
				if (bct_t->right) {
					ll.add_front(&bct_t->right);
				}
				memfree(bct_t);
			}
			memset(this, 0, sizeof(*this));
		}
	};
	bool __count_char_cmp(byte_count_tree& a, byte_count_tree& b)  { return a.count > b.count; }

	bool __count_char_cmp2(byte_count_tree& a, byte_count_tree& b) { 
		if (a.count != b.count) {
			return a.count > b.count;
		}
		return a.b > b.b;
	
	}
	union __bits_u {
		UINT64 ui64;			//Should be enough
		byte b[8];
	} ;
	struct __bits {
		__bits_u bits;
		UINT64 bits_count = 0;
	};

	struct __bct_bits {
		byte_count_tree bct;
		__bits_u bits;
		UINT64 bits_count=0;
		void add_bits(UINT64 b) {
			size_t pos = (++bits_count) >> 3,
				bit_pos= bits_count&7;
			if (b) {
				bits.b[pos] |= 1 << bit_pos;
			}
			else {
				bits.b[pos] &= ~(1 << bit_pos);
			}
		}

	};

	
	struct huffman_compressed_data {
		nc::arr<byte> data;
		UINT64 bit_count=0;
		__bits byte_lut[256];//Replace with a dictionnary or other LUT

		void add(UINT64 b, UINT64 b_count){
			byte* byte_ptr = (byte*)&b;
			size_t byte_count = b >> 3, rest_bit_counts = b & 7;
			if (bit_count) {
				UINT64 to_and = to_and_lut[bit_count];
				
				//if (space_left<8) {
				//	data.realloc(data.capacity + 64);
				//}
				while (byte_count--) {
					data.data[data.size] = (data.data[data.size] & to_and) || (*byte_ptr << rest_bit_counts);
					byte_ptr++;
				}
					

				return;
			}
			//if (!((space_left > byte_count) ||
			//	((space_left == byte_count) &&
			//		((8 - bit_count) >= rest_bit_counts)))) {
			//if(space_left < 8){					//branch predictability
			//	data.realloc(data.capacity + 64);
			//}

			data.add(byte_ptr, byte_count+ (rest_bit_counts!=0));

			bit_count = rest_bit_counts;

		}

		void destroy() {
			data.destroy();
			bit_count = 0;
			
		}

	};


	struct compressed_file {
		UINT filetype = XAF_FILE;
		UINT struct_size = sizeof(file);
		wchar_t* name = NULL;
		UINT64 name_size = 0;
		xaf* parent = NULL;
		huffman_compressed_data content ;
		


		void build(file* f) {
			byte* data =(byte*) f->content;
			quick_queue<byte_count_tree, 256> q;
			byte_count_tree* byte_count=q.data,*bct_a,*bct_b,
				bct_tot;
			//Initialising the queue
			for (size_t i = 0; i != 256; i++) {
				byte_count[i].b = i;
				byte_count[i].count = 0;
				byte_count[i].left = NULL;
				byte_count[i].right = NULL;
			}
			//Counting every bytes occurences
			for (size_t i = 0; i != f->content_size; i++) {
				byte_count[data[i]].count++;
			}
			//https://medium.com/stantmob/data-compression-with-huffman-coding-ad7bcb07c5d5
			//sorting the queue ( ascending order)
			std::sort(byte_count, byte_count + 255, __count_char_cmp);
			size_t count = 0;
			//building the tree
			while (q.count() > 1) {
				bct_a = (byte_count_tree*)memalloc(sizeof(*bct_a));
				bct_b = (byte_count_tree*)memalloc(sizeof(*bct_b));
				count += 2;
				*bct_a = q.dequeue();				//AVX2 accelerated copy?
				*bct_b = q.dequeue();				//AVX2 accelerated copy?
				bct_tot.b = (UINT64)-1;
				bct_tot.count = bct_a->count + bct_b->count;
				bct_tot.left = bct_a;
				bct_tot.right = bct_b;
				q.enqueue(&bct_tot);
				//std::sort(byte_count, byte_count + 255, __count_char_cmp2);		//maybe unecessary???
			}
			__bct_bits bb,temp_bb;
			count++;
			UINT64 bytes_counts[256] = {0};
			bb.bct = q.dequeue();		//root node
			memset(&bb.bits, 0, sizeof(bb.bits));
			__bct_bits* stack =(__bct_bits*) memalloc(count * sizeof(__bct_bits));
			size_t stack_size=1;
			*stack = bb;
			while (stack_size) {
				bb = stack[--stack_size];
				if (bb.bct.left) {
					stack[stack_size].bct = *bb.bct.left;
					memfree(bb.bct.left);
					memcpy(&stack[stack_size].bits, &bb.bits, sizeof(bb) - PTRSIZE);
					stack[stack_size++].add_bits(0);
				}
				else if (bb.bct.right) {
					stack[stack_size].bct = *bb.bct.right;
					memfree(bb.bct.right);
					memcpy(&stack[stack_size].bits, &bb.bits, sizeof(bb) - PTRSIZE);
					stack[stack_size++].add_bits(1);
				}
				else{
					content.byte_lut[bb.bct.b].bits = bb.bits;				//Do zweryfikowania
					content.byte_lut[bb.bct.b].bits_count = bb.bits_count;
					bytes_counts[bb.bct.b]= bb.bct.count;
				}
				
			}
			memfree(stack);
			size_t prealloc = 0;
			for (size_t i = 0; i != 256; i++) {
				prealloc += bytes_counts[i] * content.byte_lut[bb.bct.b].bits_count;
			}
			
			prealloc =(prealloc>> 3)+ ((prealloc&7)!=0);
			content.data.realloc(prealloc);
			__bits bs;

			
			for (size_t i = 0; i != f->content_size; i++) {
				bs = content.byte_lut[data[i]];
				content.add(bs.bits.ui64, bs.bits_count);
			}
		}

		void decompress(file* f_out) {

		}

		void destroy() {

		}
	};

};



