#pragma once

#include <vector>
#include <list>
#include <iostream>
#include <map>
#include <algorithm>

#include "debugtool.h"

template<class Type>
class FastArray {
public:
	std::size_t insert(const Type& value);
	void remove(std::size_t index);

	std::size_t size();
	std::size_t total_size();

	void clear();

	bool is_available(std::size_t index);

	Type& operator[](std::size_t index);
private:
	std::vector<Type> container_;
	std::vector<bool> available_;
	std::list<std::size_t> available_list_;
};

template <class index_type, class value_type>
class IndexedMap;

template<class index_type, class value_type>
class IndexedMapIterator {
	friend class IndexedMap<index_type, value_type>;
public:
	IndexedMapIterator() = default;
	using IndexedMap = IndexedMap<index_type, value_type>;
	using MapIterator = typename std::map<index_type, std::size_t>::iterator;

	index_type& first();
	value_type& second();
	std::size_t position()const;

	void operator ++();
	void operator --();
	bool operator ==(const IndexedMapIterator that);
private:
	IndexedMapIterator(IndexedMap& container, MapIterator map_iterator);

	IndexedMap* container_;
	MapIterator map_iterator_;
};

template<class index_type, class value_type>
class IndexedMapConstIterator {
	friend class IndexedMap<index_type, value_type>;
public:
	IndexedMapConstIterator() = default;
	using IndexedMap = IndexedMap<index_type, value_type>;
	using MapConstIterator = typename std::map<index_type, std::size_t>::const_iterator;

	const index_type& first()const;
	const value_type& second()const;
	std::size_t position()const;

	void operator ++();
	void operator --();
	bool operator ==(const IndexedMapConstIterator that);
private:
	IndexedMapConstIterator(const IndexedMap& container, MapConstIterator map_const_iterator);

	const IndexedMap* container_;
	MapConstIterator map_const_iterator_;
};

template <class index_type, class value_type>
class IndexedMap {
public:
	using iterator = IndexedMapIterator<index_type, value_type>;
	using const_iterator = IndexedMapConstIterator<index_type, value_type>;

	IndexedMap() = default;
	IndexedMap(const std::vector<std::pair<index_type, value_type>> init_list);

	bool empty()const;
	std::size_t size() const;
	void clear();

	void sort();
	void swap(const std::size_t x1, const std::size_t x2);

	std::size_t emplace(const index_type& index, const value_type& content);

	bool have(const index_type& index);
	bool have(const std::size_t ser);

	std::pair<std::size_t, bool> find_serial_number(const index_type& index)const;
	const value_type& cfind(std::size_t ser)const;
	const_iterator cfind(const index_type& index)const;
	iterator find(const index_type& index);
	const_iterator find(const index_type& index)const;
	const_iterator cbegin()const;
	iterator begin();
	const_iterator cend()const;
	iterator end();

	value_type& operator [] (const std::size_t ser);
	const value_type& operator [] (const std::size_t ser)const;
	value_type& operator [] (const index_type& index);
private:
	std::vector<value_type> container_;
	std::map<index_type, std::size_t> indices_;

	struct sort_ptr {
		std::size_t ser = 0;
		const value_type* ptr = nullptr;

		sort_ptr() {};

		sort_ptr(const std::size_t ser_in, const value_type& content) {
			ser = ser_in, ptr = &content;
		}
	};
};

template<class Type>
inline unsigned int FastArray<Type>::insert(const Type& value) {
	if (available_list_.size() == 0) {
		container_.push_back(value);
		available_.push_back(false);
		return container_.size() - 1;
	}
	std::size_t index = available_list_.front();
	available_list_.pop_front();
	container_[index] = value;
	available_[index] = false;
	return index;
}

template<class Type>
inline void FastArray<Type>::remove(std::size_t index) {
	if (index >= container_.size()) {
		ASSERT(false);
	}
	available_list_.push_back(index);
	available_[index] = true;
}

template<class Type>
inline std::size_t FastArray<Type>::size() {
	return container_.size() - available_list_.size();
}

template<class Type>
inline std::size_t FastArray<Type>::total_size() {
	return container_.size();
}

template<class Type>
inline void FastArray<Type>::clear() {
	std::size_t i;
	for (i = 0; i < available_.size(); i++) {
		if (available_[i] == false) {
			available_[i] = true;
			available_list_.push_back(i);
		}
	}
}

template<class Type>
inline bool FastArray<Type>::is_available(std::size_t index) {
	if (index >= container_.size()) {
		ASSERT(false);
	}
	return available_[index];
}

template<class Type>
inline Type& FastArray<Type>::operator[](std::size_t index) {
	if (index >= container_.size()) {
		ASSERT(false);
	}
	if (available_[index] == true) {
		ASSERT(false);
	}
	return container_[index];
}

template<class index_type, class value_type>
inline index_type& IndexedMapIterator<index_type, value_type>::first() {
	return map_iterator_->first;
}

template<class index_type, class value_type>
inline value_type& IndexedMapIterator<index_type, value_type>::second() {
	return (*container_)[map_iterator_->second];
}

template<class index_type, class value_type>
inline std::size_t IndexedMapIterator<index_type, value_type>::position()const {
	return map_iterator_->second;
}

template<class index_type, class value_type>
inline void IndexedMapIterator<index_type, value_type>::operator++() {
	map_iterator_++;
}

template<class index_type, class value_type>
inline void IndexedMapIterator<index_type, value_type>::operator--() {
	map_iterator_--;
}

template<class index_type, class value_type>
inline bool IndexedMapIterator<index_type, value_type>::operator==
(const IndexedMapIterator that) {
	return container_ == that.container_ && map_iterator_ == that.map_iterator_;
}

template<class index_type, class value_type>
inline IndexedMapIterator<index_type, value_type>::
IndexedMapIterator(IndexedMap& container, MapIterator map_iterator)
	:container_(&container), map_iterator_(map_iterator) {
}

template<class index_type, class value_type>
inline const index_type& IndexedMapConstIterator<index_type, value_type>::first() const {
	return map_const_iterator_->first;
}

template<class index_type, class value_type>
inline const value_type& IndexedMapConstIterator<index_type, value_type>::second() const {
	return (*container_)[map_const_iterator_->second];
}

template<class index_type, class value_type>
inline std::size_t IndexedMapConstIterator<index_type, value_type>::position()const {
	return map_const_iterator_->second;
}

template<class index_type, class value_type>
inline void IndexedMapConstIterator<index_type, value_type>::operator++() {
	map_const_iterator_++;
}

template<class index_type, class value_type>
inline void IndexedMapConstIterator<index_type, value_type>::operator--() {
	map_const_iterator_--;
}

template<class index_type, class value_type>
inline bool IndexedMapConstIterator<index_type, value_type>::operator==
(const IndexedMapConstIterator that) {
	return container_ == that.container_ && map_const_iterator_ == that.map_const_iterator_;
}

template<class index_type, class value_type>
inline IndexedMapConstIterator<index_type, value_type>::
IndexedMapConstIterator(const IndexedMap& container, MapConstIterator map_const_iterator)
	:container_(&container), map_const_iterator_(map_const_iterator) {
}

template<class index_type, class value_type>
inline IndexedMap<index_type, value_type>::IndexedMap(
	const std::vector<std::pair<index_type, value_type>> init_list
) {
	std::size_t size = init_list.size();
	container_.resize(size);
	for (std::size_t i = 0; i < size; i++) {
		container_[i] = init_list[i].second;
		indices_.emplace(init_list[i].first, i);
	}
}

template<class index_type, class value_type>
inline bool IndexedMap<index_type, value_type>::empty() const {
	return container_.empty();
}

template <class index_type, class value_type>
inline std::size_t IndexedMap<index_type, value_type>::size() const {
	return container_.size();
}

template <class index_type, class value_type>
inline void IndexedMap<index_type, value_type>::clear() {
	container_.clear();
	indices_.clear();
}

template <class index_type, class value_type>
void IndexedMap<index_type, value_type>::sort() {
	std::size_t i, now, siz = container_.size();
	sort_ptr* temp = (sort_ptr*)alloca(siz * sizeof(sort_ptr));
	std::size_t* ind = (std::size_t*)alloca(siz * sizeof(std::size_t));
	for (i = 0; i < siz; i++) {
		temp[i] = sort_ptr(i, container_[i]);
	}

	std::sort(temp, temp + siz, [](const sort_ptr& a, const sort_ptr& b) {
		return *a.ptr < *b.ptr;
		});

	for (i = 0; i < siz; i++) {
		ind[temp[i].ser] = i;
	}
	for (auto it = indices_.begin(); it != indices_.end(); it++) {
		it->second = ind[it->second];
	}
	for (i = 0; i < siz; i++) {
		now = i;
		while (ind[now] != now) {
			std::swap(ind[now], ind[temp[now].ser]);
			std::swap(container_[now], container_[temp[now].ser]);
			now = temp[now].ser;
		}
	}
}

template <class index_type, class value_type>
void IndexedMap<index_type, value_type>::swap(const std::size_t x1, const std::size_t x2) {
	std::size_t i = 0, siz = container_.size();
	if (x1 >= siz || x2 >= x2) {
		ERROR(true)
			<< "ÈÝÆ÷ÐÞ¸Ä´íÎó : ²»´æÔÚ½»»»ÔªËØ\n";
		ASSERT(false);
	}
	for (auto it = indices_.begin(); it != indices_.end(); it++) {
		if (it->second == x1) {
			it->second = x2;
			i++;
		}
		if (it->second == x2) {
			it->second = x1;
			i++;
		}
		if (i == 2) {
			break;
		}
	}
	std::swap(container_[x1], container_[x2]);
}

template <class index_type, class value_type>
unsigned int IndexedMap<index_type, value_type>::emplace
(const index_type& index, const value_type& content) {
	auto iter = indices_.find(index);
	if (iter != indices_.end()) {
		container_[iter->second] = content;
		return iter->second;
	}
	container_.push_back(content);
	std::size_t ser = container_.size() - 1;
	indices_.emplace(index, ser);
	return ser;
}

template<class index_type, class value_type>
inline bool IndexedMap<index_type, value_type>::have(const index_type& index) {
	return indices_.find(index) != indices_.end();
}

template<class index_type, class value_type>
inline bool IndexedMap<index_type, value_type>::have(const std::size_t ser) {
	return ser < size() && ser >= 0;
}

template<class index_type, class value_type>
inline std::pair<std::size_t, bool>
IndexedMap<index_type, value_type>::find_serial_number(const index_type& index)const {
	auto iter = indices_.find(index);
	if (iter == indices_.end()) {
		return std::make_pair(0, false);
	}
	return std::make_pair(iter->second, true);
}

template <class index_type, class value_type>
inline const value_type&
IndexedMap<index_type, value_type>::cfind(std::size_t ser)const {
	if (container_.empty()) {
		ERROR(true)
			<< "ContainerType²éÑ¯´íÎó : ÈÝÆ÷Îª¿Õ\n";
		ASSERT(false);
	}
	if (ser >= size()) {
		ERROR(true)
			<< "ContainerType²éÑ¯´íÎó : ÈÝÆ÷²»´æÔÚ[" << ser << "]ºÅÔªËØ\n";
		ASSERT(false);
	}
	return container_[ser];
}

template<class index_type, class value_type>
inline IndexedMap<index_type, value_type>::const_iterator
IndexedMap<index_type, value_type>::cfind(const index_type& index) const {
	auto iter = indices_.find(index);
	return const_iterator(*this, iter);
}

template <class index_type, class value_type>
inline IndexedMap<index_type, value_type>::iterator
IndexedMap<index_type, value_type>::find(const index_type& index) {
	auto iter = indices_.find(index);
	return iterator(*this, iter);
}

template<class index_type, class value_type>
inline IndexedMap<index_type, value_type>::const_iterator
IndexedMap<index_type, value_type>::find(const index_type& index) const {
	return cfind(index);
}

template<class index_type, class value_type>
inline IndexedMap<index_type, value_type>::const_iterator
IndexedMap<index_type, value_type>::cbegin()const {
	return const_iterator(*this, indices_.cbegin());
}

template<class index_type, class value_type>
inline IndexedMap<index_type, value_type>::iterator
IndexedMap<index_type, value_type>::begin() {
	return iterator(*this, indices_.begin());
}

template<class index_type, class value_type>
inline IndexedMap<index_type, value_type>::const_iterator
IndexedMap<index_type, value_type>::cend()const {
	return const_iterator(*this, indices_.cend());
}

template<class index_type, class value_type>
inline IndexedMap<index_type, value_type>::iterator
IndexedMap<index_type, value_type>::end() {
	return iterator(*this, indices_.end());
}

template <class index_type, class value_type>
inline value_type& IndexedMap<index_type, value_type>::operator []
(const std::size_t ser) {
	if (container_.empty()) {
		ERROR(true)
			<< "ContainerType²éÑ¯´íÎó : ÈÝÆ÷Îª¿Õ\n";
		ASSERT(false);
	}
	if (ser >= size()) {
		ERROR(true)
			<< "ContainerType²éÑ¯´íÎó : ÈÝÆ÷²»´æÔÚ[" << ser << "]ºÅÔªËØ\n";
		ASSERT(false);
	}
	return container_[ser];
}

template <class index_type, class value_type>
inline const value_type& IndexedMap<index_type, value_type>::operator []
(const std::size_t ser)const {
	if (container_.empty()) {
		ERROR(true)
			<< "ContainerType²éÑ¯´íÎó : ÈÝÆ÷Îª¿Õ\n";
		ASSERT(false);
	}
	if (ser >= size()) {
		ERROR(true)
			<< "ContainerType²éÑ¯´íÎó : ÈÝÆ÷²»´æÔÚ[" << ser << "]ºÅÔªËØ\n";
		ASSERT(false);
	}
	return container_[ser];
}

template <class index_type, class value_type>
inline value_type& IndexedMap<index_type, value_type>::operator []
(const index_type& index) {
	if (container_.empty()) {
		ERROR(true)
			<< "ContainerType²éÑ¯´íÎó : ÈÝÆ÷Îª¿Õ\n";
		ASSERT(false);
	}
	auto it = indices_.find(index);
	if (it == indices_.end()) {
		ERROR(true)
			<< "ContainerType²éÑ¯´íÎó : ²»´æÔÚÔªËØ[" << index << "]\n";
		ASSERT(false);
	}
	return container_[it->second];
}