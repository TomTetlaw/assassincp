#ifndef __ARRAY_H__
#define __ARRAY_H__

template<typename T>
struct Array {
	T *data = nullptr;
	int size = 0;
	int num = 0;

	~Array();
	void ensure_size(int new_size);
	void append(const T &value);
	void remove(int i);
	T *alloc();
	T &operator[](int index) { return data[index]; }
	T &first() { return data[0]; }
	//T *last() { if (num <= 0) return nullptr; else return &data[num - 1]; }
	bool find(const T &val);
	int find_index(const T &val);
};

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
	memset(data, 0, sizeof(T) * new_size);
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

#define For(x, code) { if(x.num > 0) { auto it = x.first(); for (int it_index = 0; it_index < x.num; it_index++, it = x[it_index]) code } }

#endif