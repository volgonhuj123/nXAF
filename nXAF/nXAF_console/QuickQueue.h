#pragma once
namespace nc {
	template<typename T, size_t n> struct quick_queue {
		size_t read_pos = 0, insert_pos = 0;
		T data[n];
		void enqueue(T* data);
		T dequeue();
		size_t count();
	};
};