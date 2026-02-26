namespace nc {

	template<typename T,size_t n> struct quick_queue {

		size_t read_pos=0, insert_pos=0;
		T data[n];

		void enqueue(T* data) {
			insert_pos++;
			insert_pos %= n;
			data[insert_pos] = *data;
		}
		T dequeue() {
			T ret= data[read_pos++] ;
			read_pos %= n;
			return ret;
		}

		size_t count() {
			return insert_pos > read_pos ? insert_pos - read_pos :n- read_pos+ insert_pos;

		}

	};


};