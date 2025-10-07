#pragma once

#include <vector>
#include <list>
#include <iostream>
#include <map>
#include <algorithm>
#include <functional>

#include "debug_tool.h"

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

template <class key_type, class value_type, class map_type = std::map<key_type, std::size_t>>
class IndexedMap;

template <class key_type, class value_type, class map_type>
class IndexedMapIterator {
	friend class IndexedMap<key_type, value_type, map_type>;
public:
	IndexedMapIterator() = default;
	using IndexedMap = IndexedMap<key_type, value_type, map_type>;
	using MapIterator = typename map_type::iterator;

	const key_type& first();
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

template <class key_type, class value_type, class map_type>
class IndexedMapConstIterator {
	friend class IndexedMap<key_type, value_type, map_type>;
public:
	IndexedMapConstIterator() = default;
	using IndexedMap = IndexedMap<key_type, value_type, map_type>;
	using MapConstIterator = typename map_type::const_iterator;

	const key_type& first()const;
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

template <class key_type, class value_type, class map_type>
class IndexedMap {
public:
	using iterator = IndexedMapIterator<key_type, value_type, map_type>;
	using const_iterator = IndexedMapConstIterator<key_type, value_type, map_type>;
	using visitor = std::pair<const key_type*, value_type*>;
	using const_visitor = std::pair<const key_type*, const value_type*>;

	IndexedMap() = default;
	IndexedMap(const std::vector<std::pair<key_type, value_type>>& init_vector);

	bool empty()const;
	std::size_t size() const;
	void clear();

	void sort(const std::function<bool(const value_type&, const value_type&)>& cmpare);
	void swap(std::size_t x1, std::size_t x2);

	std::size_t insert(const key_type& key, const value_type& value);
	template<typename...KeyArgs,typename...ValueArgs>
	std::size_t emplace(
		std::piecewise_construct_t,
		std::tuple<KeyArgs...> key_args,
		std::tuple<ValueArgs...> value_args
	);
	void pop_back();

	bool have(const key_type& key);
	bool have(std::size_t pos);

	std::pair<std::size_t, bool> find_serial_number(const key_type& key)const;
	iterator find(const key_type& key);
	iterator find(std::size_t pos);
	const_iterator find(const key_type& key)const;
	const_iterator find(std::size_t pos)const;
	const_iterator cbegin()const;
	iterator begin();
	const_iterator cend()const;
	iterator end();

	//注意,擦除的开销极大(略大于std::vector擦除的开销)
	void erase(const key_type& key);
	void erase(std::size_t pos);

	//这种擦除方法代价较小,但会破坏原有顺序
	void unordered_erase(const key_type& key);
	void unordered_erase(std::size_t pos);

	std::vector<visitor> get_visitor();
	std::vector<const_visitor> get_visitor()const;

	value_type& operator [] (std::size_t pos);
	const value_type& operator [] (std::size_t pos)const;
	value_type& operator [] (const key_type& key);
private:
	std::vector<value_type> container_;
	map_type indices_;
};

template <class key_type, class value_type, class map_type = std::map<key_type, std::size_t>>
class IndexedBiMap;

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

template <class key_type, class value_type, class map_type>
inline const key_type& IndexedMapIterator<key_type, value_type, map_type>::first() {
	return map_iterator_->first;
}

template <class key_type, class value_type, class map_type>
inline value_type& IndexedMapIterator<key_type, value_type, map_type>::second() {
	return (*container_)[map_iterator_->second];
}

template <class key_type, class value_type, class map_type>
inline std::size_t IndexedMapIterator<key_type, value_type, map_type>::position()const {
	return map_iterator_->second;
}

template <class key_type, class value_type, class map_type>
inline void IndexedMapIterator<key_type, value_type, map_type>::operator++() {
	map_iterator_++;
}

template <class key_type, class value_type, class map_type>
inline void IndexedMapIterator<key_type, value_type, map_type>::operator--() {
	map_iterator_--;
}

template <class key_type, class value_type, class map_type>
inline bool IndexedMapIterator<key_type, value_type, map_type>::operator==
(const IndexedMapIterator that) {
	return container_ == that.container_ && map_iterator_ == that.map_iterator_;
}

template <class key_type, class value_type, class map_type>
inline IndexedMapIterator<key_type, value_type, map_type>::
IndexedMapIterator(IndexedMap& container, MapIterator map_iterator)
	:container_(&container), map_iterator_(map_iterator) {
}

template <class key_type, class value_type, class map_type>
inline const key_type& IndexedMapConstIterator<key_type, value_type, map_type>::first() const {
	return map_const_iterator_->first;
}

template <class key_type, class value_type, class map_type>
inline const value_type& IndexedMapConstIterator<key_type, value_type, map_type>::second() const {
	return (*container_)[map_const_iterator_->second];
}

template <class key_type, class value_type, class map_type>
inline std::size_t IndexedMapConstIterator<key_type, value_type, map_type>::position()const {
	return map_const_iterator_->second;
}

template <class key_type, class value_type, class map_type>
inline void IndexedMapConstIterator<key_type, value_type, map_type>::operator++() {
	map_const_iterator_++;
}

template <class key_type, class value_type, class map_type>
inline void IndexedMapConstIterator<key_type, value_type, map_type>::operator--() {
	map_const_iterator_--;
}

template <class key_type, class value_type, class map_type>
inline bool IndexedMapConstIterator<key_type, value_type, map_type>::operator==
(const IndexedMapConstIterator that) {
	return container_ == that.container_ && map_const_iterator_ == that.map_const_iterator_;
}

template <class key_type, class value_type, class map_type>
inline IndexedMapConstIterator<key_type, value_type, map_type>::
IndexedMapConstIterator(const IndexedMap& container, MapConstIterator map_const_iterator)
	:container_(&container), map_const_iterator_(map_const_iterator) {
}

template <class key_type, class value_type, class map_type>
inline IndexedMap<key_type, value_type, map_type>::IndexedMap(
	const std::vector<std::pair<key_type, value_type>>& init_vector
) {
	std::size_t size = init_vector.size();
	container_.resize(size);
	for (std::size_t i = 0; i < size; i++) {
		container_[i] = init_vector[i].second;
		indices_.emplace(init_vector[i].first, i);
	}
}

template <class key_type, class value_type, class map_type>
inline bool IndexedMap<key_type, value_type, map_type>::empty() const {
	return container_.empty();
}

template <class key_type, class value_type, class map_type>
inline std::size_t IndexedMap<key_type, value_type, map_type>::size() const {
	return container_.size();
}

template <class key_type, class value_type, class map_type>
inline void IndexedMap<key_type, value_type, map_type>::clear() {
	container_.clear();
	indices_.clear();
}

template <class key_type, class value_type, class map_type>
void IndexedMap<key_type, value_type, map_type>::sort(
	const std::function<bool(const value_type&, const value_type&)>& cmpare
) {
	std::size_t i, now, size = container_.size();
	std::vector<std::size_t> new_order(size);
	for (i = 0; i < size; i++) {
		new_order[i] = i;
	}
	std::sort(new_order.begin(), new_order.end(), [&, this](
		std::size_t x1, std::size_t x2) {
			return cmpare(container_[x1], container_[x2]);
		}
	);
	std::vector<std::size_t> destination(size);
	for (std::size_t i = 0; i < size; i++) {
		destination[new_order[i]] = i;
	}
	for (auto it = indices_.begin(); it != indices_.end(); it++) {
		it->second = destination[it->second];
	}
	for (i = 0; i < size; i++) {
		now = i;
		while (destination[now] != now) {
			std::swap(destination[now], destination[new_order[now]]);
			std::swap(container_[now], container_[new_order[now]]);
			now = new_order[now];
		}
	}
}

template <class key_type, class value_type, class map_type>
void IndexedMap<key_type, value_type, map_type>::swap(std::size_t x1, std::size_t x2) {
	std::size_t i = 0, siz = container_.size();
	if (x1 >= siz || x2 >= siz) {
		return;
	}
	if (x1 == x2) {
		return;
	}
	for (auto it = indices_.begin(); it != indices_.end(); it++) {
		if (it->second == x1) {
			it->second = x2;
			i++;
		}
		else if (it->second == x2) {
			it->second = x1;
			i++;
		}
		if (i == 2) {
			break;
		}
	}
	std::swap(container_[x1], container_[x2]);
}

template <class key_type, class value_type, class map_type>
unsigned int IndexedMap<key_type, value_type, map_type>::insert
(const key_type& key, const value_type& value) {
	auto iter = indices_.find(key);
	if (iter != indices_.end()) {
		container_.emplace(container_.begin() + iter->second, value);
		return iter->second;
	}
	container_.emplace_back(value);
	std::size_t pos = container_.size() - 1;
	indices_.emplace(key, pos);
	return pos;
}

template<class key_type, class value_type, class map_type>
template<typename ...KeyArgs, typename ...ValueArgs>
inline std::size_t IndexedMap<key_type, value_type, map_type>::emplace(
	std::piecewise_construct_t,
	std::tuple<KeyArgs...> key_args,
	std::tuple<ValueArgs...> value_args
){
	key_type key(std::make_from_tuple<key_type>(std::move(key_args)));
	auto iter = indices_.find(key);
	if (iter != indices_.end()) {
		container_.emplace(
			container_.begin() + iter->second,
			std::make_from_tuple<value_type>(std::move(value_args))
		);
		return iter->second;
	}
	container_.emplace_back(
		std::make_from_tuple<value_type>(std::move(value_args))
	);
	std::size_t pos = container_.size() - 1;
	indices_.emplace(key, pos);
	return pos;
}

template<class key_type, class value_type, class map_type>
void IndexedMap<key_type, value_type, map_type>::pop_back() {
	if (empty() == true) {
		return;
	}
	auto iter = find(size() - 1);
	indices_.erase(iter.first());
	container_.pop_back();
}

template <class key_type, class value_type, class map_type>
inline bool IndexedMap<key_type, value_type, map_type>::have(const key_type& key) {
	return indices_.find(key) != indices_.end();
}

template <class key_type, class value_type, class map_type>
inline bool IndexedMap<key_type, value_type, map_type>::have(std::size_t pos) {
	return pos < size() && pos >= 0;
}

template <class key_type, class value_type, class map_type>
inline std::pair<std::size_t, bool>
IndexedMap<key_type, value_type, map_type>::find_serial_number(const key_type& key)const {
	auto iter = indices_.find(key);
	if (iter == indices_.end()) {
		return std::make_pair(0, false);
	}
	return std::make_pair(iter->second, true);
}

template <class key_type, class value_type, class map_type>
inline IndexedMap<key_type, value_type, map_type>::iterator
IndexedMap<key_type, value_type, map_type>::find(const key_type& key) {
	auto iter = indices_.find(key);
	return iterator(*this, iter);
}

template<class key_type, class value_type, class map_type>
inline IndexedMap<key_type, value_type, map_type>::iterator
IndexedMap<key_type, value_type, map_type>::find(std::size_t pos) {
	if (pos >= size()) {
		ERROR(true)
			<< "不存在第[" << pos << "]号元素" << std::endl;
		ASSERT(false);
	}
	auto iter = indices_.begin();
	for (; iter != indices_.end(); iter++) {
		if (iter->second == pos) {
			return iterator(*this, iter);
		}
	}
	return end();
}

template<class key_type, class value_type, class map_type>
inline IndexedMap<key_type, value_type, map_type>::const_iterator
IndexedMap<key_type, value_type, map_type>::find(std::size_t pos)const {
	if (pos >= size()) {
		ERROR(true)
			<< "不存在第[" << pos << "]号元素" << std::endl;
		ASSERT(false);
	}
	auto iter = indices_.cbegin();
	for (; iter != indices_.cend(); iter++) {
		if (iter->second == pos) {
			return const_iterator(*this, iter);
		}
	}
	return cend();
}

template <class key_type, class value_type, class map_type>
inline IndexedMap<key_type, value_type, map_type>::const_iterator
IndexedMap<key_type, value_type, map_type>::find(const key_type& key) const {
	auto iter = indices_.find(key);
	return const_iterator(*this, iter);
}

template <class key_type, class value_type, class map_type>
inline IndexedMap<key_type, value_type, map_type>::const_iterator
IndexedMap<key_type, value_type, map_type>::cbegin()const {
	return const_iterator(*this, indices_.cbegin());
}

template <class key_type, class value_type, class map_type>
inline IndexedMap<key_type, value_type, map_type>::iterator
IndexedMap<key_type, value_type, map_type>::begin() {
	return iterator(*this, indices_.begin());
}

template <class key_type, class value_type, class map_type>
inline IndexedMap<key_type, value_type, map_type>::const_iterator
IndexedMap<key_type, value_type, map_type>::cend()const {
	return const_iterator(*this, indices_.cend());
}

template <class key_type, class value_type, class map_type>
inline IndexedMap<key_type, value_type, map_type>::iterator
IndexedMap<key_type, value_type, map_type>::end() {
	return iterator(*this, indices_.end());
}

template<class key_type, class value_type, class map_type>
inline void IndexedMap<key_type, value_type, map_type>::erase(const key_type& key) {
	auto iter = indices_.find(key);
	if (iter == indices_.end()) {
		return;
	}
	auto viter = indices_.begin();
	for (; viter != indices_.end(); viter++) {
		if (viter->second > iter->second) {
			viter->second--;
		}
	}
	container_.erase(container_.begin() + iter->second);
	indices_.erase(key);
}

template<class key_type, class value_type, class map_type>
inline void IndexedMap<key_type, value_type, map_type>::erase(std::size_t pos) {
	if (pos >= size()) {
		return;
	}
	auto viter = indices_.begin();
	for (; viter != indices_.end(); viter++) {
		if (viter->second > pos) {
			viter->second--;
		}
	}
	container_.erase(container_.begin() + pos);
	auto iter = indices_.begin();
	for (; iter != indices_.end(); iter++) {
		if (iter->second == pos) {
			indices_.erase(iter);
			return;
		}
	}
	ASSERT(false);
}

template<class key_type, class value_type, class map_type>
inline void IndexedMap<key_type, value_type, map_type>::unordered_erase(
	const key_type& key
) {
	auto iter = indices_.find(key);
	if (iter == indices_.end()) {
		return;
	}
	swap(iter->second, size() - 1);
	pop_back();
}

template<class key_type, class value_type, class map_type>
inline void IndexedMap<key_type, value_type, map_type>::unordered_erase(
	std::size_t pos
) {
	if (pos == size() - 1) {
		pop_back();
	}
	swap(pos, size() - 1);
	pop_back();
}

template<class key_type, class value_type, class map_type>
inline std::vector<typename IndexedMap<key_type, value_type, map_type>::visitor>
IndexedMap<key_type, value_type, map_type>::get_visitor() {
	std::vector<visitor> data;
	auto iter = begin();
	std::vector<decltype(iter)> iters;
	for (; iter != end(); ++iter) {
		iters.emplace_back(iter);
	}
	std::sort(iters.begin(), iters.end(), [](const auto& x1, const auto& x2) {
		return x1.position() < x2.position();
		});
	std::size_t size = iters.size();
	for (std::size_t i = 0; i < size; i++) {
		data.emplace_back(&iters[i].first(), &iters[i].second());
	}
	return data;
}

template<class key_type, class value_type, class map_type>
inline std::vector<typename IndexedMap<key_type, value_type, map_type>::const_visitor>
IndexedMap<key_type, value_type, map_type>::get_visitor() const {
	std::vector<const_visitor> data;
	auto iter = cbegin();
	std::vector<decltype(iter)> iters;
	for (; iter != cend(); ++iter) {
		iters.emplace_back(iter);
	}
	std::sort(iters.begin(), iters.end(), [](const auto& x1, const auto& x2) {
		return x1.position() < x2.position();
		});
	std::size_t size = iters.size();
	for (std::size_t i = 0; i < size; i++) {
		data.emplace_back(&iters[i].first(), &iters[i].second());
	}
	return data;
}

template <class key_type, class value_type, class map_type>
inline value_type& IndexedMap<key_type, value_type, map_type>::operator []
(std::size_t pos) {
	if (container_.empty()) {
		ERROR(true)
			<< "ContainerType查询错误 : 容器为空\n";
		ASSERT(false);
	}
	if (pos >= size()) {
		ERROR(true)
			<< "ContainerType查询错误 : 容器不存在[" << pos << "]号元素\n";
		ASSERT(false);
	}
	return container_[pos];
}

template <class key_type, class value_type, class map_type>
inline const value_type& IndexedMap<key_type, value_type, map_type>::operator []
(std::size_t pos)const {
	if (container_.empty()) {
		ERROR(true)
			<< "ContainerType查询错误 : 容器为空\n";
		ASSERT(false);
	}
	if (pos >= size()) {
		ERROR(true)
			<< "ContainerType查询错误 : 容器不存在[" << pos << "]号元素\n";
		ASSERT(false);
	}
	return container_[pos];
}

template <class key_type, class value_type, class map_type>
inline value_type& IndexedMap<key_type, value_type, map_type>::operator []
(const key_type& key) {
	if (container_.empty()) {
		ERROR(true)
			<< "ContainerType查询错误 : 容器为空\n";
		ASSERT(false);
	}
	auto it = indices_.find(key);
	if (it == indices_.end()) {
		ERROR(true)
			<< "ContainerType查询错误 : 不存在元素[" << key << "]\n";
		ASSERT(false);
	}
	return container_[it->second];
}