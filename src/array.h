#ifndef __ARRAY_H__
#define __ARRAY_H__

template<typename T>
struct Array {
	T *data = nullptr;
	int size = 0;
	int num = 0;

	~Array();
	// make sure the memory is big enough for new_size elements
	void ensure_size(int new_size);
	void append(const T &value);
	// removes the value at index i and moves all elements infront of i back one
	void remove(int i);
	// adds a new element to the array and returns a pointer to it
	T *alloc();
	// returns true if the value exists
	bool find(const T &val);
	// returns the index if the value exists or -1 if not
	int find_index(const T &val);

	T &operator[](int index) { return data[index]; }
	T &first() { return data[0]; }
};

// macro so that you don't have to type for(int i = 0; i < x.num; i++) every time
// generally you want to do auto it = x[it_index] in the first line of the loop
// to get the reference to the element
#define For(x) for(int it_index = 0; it_index < x.num; it_index++)

template<typename T>
Array<T>::~Array() {
	delete[] data;
}

template<typename T>
void Array<T>::ensure_size(int new_size) {
	if (size >= new_size) {
		return;
	}

	T *old_data = data;
	data = new T[new_size];
	for (int i = 0; i < num; i++) {
		data[i] = old_data[i];
	}
	delete[] old_data;
	size = new_size;
}

template<typename T>
void Array<T>::append(const T &value) {
	ensure_size(num + 1);

	data[num] = value;
	num += 1;
}

template<typename T>
void Array<T>::remove(int index) {
	if (index < 0 || index >= num) {
		return;
	}

	T *old_data = data;
	data = new T[size - 1];
	size = size - 1;
	num = num - 1;

	for (int i = 0; i < index; i++) {
		data[i] = old_data[i];
	}
	// move everything after the removed element one to the left
	for (int i = index; i < size; i++) {
		data[i] = old_data[i + 1];
	}

	delete[] old_data;
}

template<typename T>
T *Array<T>::alloc() {
	ensure_size(num + 1);
	T *out = &data[num];
	num += 1;
	return out;
}

template<typename T>
bool Array<T>::find(const T &val) {
	return find_index(val) != -1;
}

template<typename T>
int Array<T>::find_index(const T &val) {
	for (int i = 0; i < num; i++) {
		if (val == data[i]) {
			return i;
		}
	}

	return -1;
}

template<typename T, int size>
class Contiguous_Array {
public:
    Array<T> elements;
    int last_freed_index = 0;
	int max_index = 0;

	inline void init() {
		elements.ensure_size(size);
		remove_all();
	}

    inline T *alloc() {
		assert(max_index < size);
		
		if(elements.size < size) init();

        if(max_index > 0) {
            T &last_freed = elements[last_freed_index];
            // can be false if none have been freed yet.
			if (last_freed._deleted) {
				last_freed._deleted = false;
				last_freed._index = last_freed_index;
				return &last_freed;
			}
        }

        for(int i = last_freed_index; i < elements.num; i++) {
            if(elements[i]._deleted) {
                elements[i]._deleted = false;
				elements[i]._index = i;
                return &elements[i];
            }
        }

		int index = elements.num;
        T *value = elements.alloc();
		value->_deleted = false;
		value->_index = index;
        max_index = elements.num;
        return value;
    }

    inline void remove(T *value) {
		assert(value);
        value->_deleted = true;
		value->_index = -1;
    }

	inline void remove_all() {
		for (int i = 0; i < size; i++) {
			elements.data[i]._deleted = true;
			elements.data[i]._index = -1;
		}
		max_index = 0;
		last_freed_index = 0;
	}

	inline T *operator[](int i) { 
		if (elements[i]._deleted) return nullptr;
		return &elements[i];
	}
};

#endif