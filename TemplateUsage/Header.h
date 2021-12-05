#pragma once
#include <vector>

//函数模板
template <typename T>
int compare(const T& v1, const T& v2)
{
	return (v1 >= v2);
}

//类模板
template <typename T> class Blob
{
public:
	typedef T value_type;
	typedef typename std::vector<T>::size_type size_type;
	Blob();
	Blob(std::initializer_list<T> il);

	size_type size() const { return data->size(); }
	bool empty() const { return data->empty(); }

	void push_back(const T& t) { data->push_back(t); }
	void push_back(T&& t) { data->push_back(std::move(t)); }
	void pop_back();

	T& back();
	T& operator[](size_type i);

private:
	std::shared_ptr<std::vector<T>> data;
	void check(size_type i, const std::string& msg) const;
};

template <typename T>
Blob<T>::Blob(std::initializer_list<T> il)
{

}

template <typename T>
Blob<T>::Blob()
{

}

template <typename T>
T& Blob<T>::operator[](size_type i)
{
	check(i, "subscript out of range");
	return (*data)[i];
}

template <typename T>
T& Blob<T>::back()
{
	check(0, "back on empty Blob");
	return data->back();
}

template <typename T>
void Blob<T>::pop_back()
{
	check(0, "pop_back on empty Blob");
	data->pop_back();
}

template <typename T>
void Blob<T>::check(size_type i, const std::string& msg) const
{
	if (i >= data->size())
		throw std::out_of_range(msg);
}