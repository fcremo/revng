#ifndef SORTEDVECTOR_H
#define SORTEDVECTOR_H

// LLVM includes
#include "llvm/ADT/STLExtras.h"

// Local libraries includes
#include "revng/ADT/KeyedObjectTraits.h"
#include "revng/Support/Assert.h"

template<class ForwardIt, class BinaryPredicate>
ForwardIt
unique_last(ForwardIt First, ForwardIt Last, BinaryPredicate Predicate) {
  if (First == Last)
    return Last;

  ForwardIt Result = First;
  while (++First != Last) {
    if (not Predicate(*Result, *First) and ++Result != First) {
      *Result = std::move(*First);
    } else {
      *Result = std::move(*First);
    }
  }

  return ++Result;
}

template<typename T>
class SortedVector {
public:
  using key_type = const decltype(KeyedObjectTraits<T>::key(std::declval<T>()));

private:
  using vector_type = std::vector<T>;

public:
  using size_type = typename vector_type::size_type;
  using value_type = typename vector_type::value_type;
  using difference_type = typename vector_type::difference_type;
  using allocator_type = typename vector_type::allocator_type;
  using reference = typename vector_type::reference;
  using const_reference = typename vector_type::const_reference;
  using pointer = typename vector_type::pointer;
  using const_pointer = typename vector_type::const_pointer;

public:
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;
  using reverse_iterator = typename vector_type::reverse_iterator;
  using const_reverse_iterator = typename vector_type::const_reverse_iterator;

private:
  vector_type TheVector;
  bool BatchInsertInProgress = false;

public:
  SortedVector() {}

  SortedVector(std::initializer_list<T> List) {
    auto Inserter = batch_insert_or_assign();
    for (const T &Element : List) {
      Inserter.insert_or_assign(Element);
    }
  }

public:
  void swap(SortedVector &Other) {
    revng_assert(not BatchInsertInProgress);
    TheVector.swap(Other.TheVector);
  }

  bool operator==(const SortedVector &) const = default;

public:
  T &at(const key_type &Key) {
    revng_assert(not BatchInsertInProgress);
    auto It = find(Key);
    revng_assert(It != end());
    return *It;
  }

  const T &at(const key_type &Key) const {
    revng_assert(not BatchInsertInProgress);
    auto It = find(Key);
    revng_assert(It != end());
    return *It;
  }

  T &operator[](const key_type &Key) {
    revng_assert(not BatchInsertInProgress);
    return *insert({ Key }).first;
  }

  T &operator[](key_type &&Key) {
    revng_assert(not BatchInsertInProgress);
    return *insert({ Key }).first;
  }

  iterator begin() {
    revng_assert(not BatchInsertInProgress);
    return TheVector.begin();
  }

  iterator end() {
    revng_assert(not BatchInsertInProgress);
    return TheVector.end();
  }

  const_iterator begin() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.begin();
  }

  const_iterator end() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.end();
  }

  const_iterator cbegin() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.cbegin();
  }

  const_iterator cend() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.cend();
  }

  reverse_iterator rbegin() {
    revng_assert(not BatchInsertInProgress);
    return TheVector.rbegin();
  }

  reverse_iterator rend() {
    revng_assert(not BatchInsertInProgress);
    return TheVector.rend();
  }

  const_reverse_iterator rbegin() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.rbegin();
  }

  const_reverse_iterator rend() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.rend();
  }

  const_reverse_iterator crbegin() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.crbegin();
  }

  const_reverse_iterator crend() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.crend();
  }

  bool empty() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.empty();
  }

  size_type size() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.size();
  }

  size_type max_size() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.max_size();
  }

  void clear() {
    revng_assert(not BatchInsertInProgress);
    TheVector.clear();
  }

  void reserve(size_type NewSize) {
    revng_assert(not BatchInsertInProgress);
    TheVector.reserve(NewSize);
  }

  size_type capacity() const {
    revng_assert(not BatchInsertInProgress);
    return TheVector.capacity();
  }

  std::pair<iterator, bool> insert(const T &Value) {
    revng_assert(not BatchInsertInProgress);
    auto Key = KeyedObjectTraits<T>::key(Value);
    auto It = lower_bound(Key);
    if (It == end()) {
      TheVector.push_back(Value);
      return { --end(), true };
    } else if (keysEqual(KeyedObjectTraits<T>::key(*It), Key)) {
      return { It, false };
    } else {
      return { TheVector.insert(It, Value), true };
    }
  }

  std::pair<iterator, bool> insert_or_assign(const T &Value) {
    revng_assert(not BatchInsertInProgress);
    auto Key = KeyedObjectTraits<T>::key(Value);
    auto It = lower_bound(Key);
    if (It == end()) {
      TheVector.push_back(Value);
      return { --end(), true };
    } else if (keysEqual(KeyedObjectTraits<T>::key(*It), Key)) {
      *It = Value;
      return { It, false };
    } else {
      return { TheVector.insert(It, Value), true };
    }
  }

  iterator erase(iterator Pos) {
    revng_assert(not BatchInsertInProgress);
    return TheVector.erase(Pos);
  }

  iterator erase(const_iterator First, const_iterator Last) {
    revng_assert(not BatchInsertInProgress);
    return TheVector.erase(First, Last);
  }

  size_type erase(const key_type &Key) {
    revng_assert(not BatchInsertInProgress);
    auto It = find(Key);
    if (It == end()) {
      return 0;
    } else {
      erase(It);
      return 1;
    }
  }

  size_type count(const key_type &Key) const {
    revng_assert(not BatchInsertInProgress);
    return find(Key) != end() ? 1 : 0;
  }

  iterator find(const key_type &Key) {
    revng_assert(not BatchInsertInProgress);
    auto It = lower_bound(Key);
    auto End = end();
    if (!(It == End)
        and std::less<key_type>()(Key, KeyedObjectTraits<T>::key(*It))) {
      return End;
    } else {
      return It;
    }
  }

  const_iterator find(const key_type &Key) const {
    revng_assert(not BatchInsertInProgress);
    auto It = lower_bound(Key);
    auto End = end();
    if (!(It == End)
        and std::less<key_type>()(Key, KeyedObjectTraits<T>::key(*It))) {
      return End;
    } else {
      return It;
    }
  }

  iterator lower_bound(const key_type &Key) {
    revng_assert(not BatchInsertInProgress);
    return std::lower_bound(begin(), end(), value_type{ Key }, compareElements);
  }

  const_iterator lower_bound(const key_type &Key) const {
    revng_assert(not BatchInsertInProgress);
    return std::lower_bound(begin(), end(), value_type{ Key }, compareElements);
  }

  iterator upper_bound(const key_type &Key) {
    revng_assert(not BatchInsertInProgress);
    return std::upper_bound(begin(), end(), value_type{ Key }, compareElements);
  }

  const_iterator upper_bound(const key_type &Key) const {
    revng_assert(not BatchInsertInProgress);
    return std::upper_bound(begin(), end(), value_type{ Key }, compareElements);
  }

public:
  class BatchInserter {
  private:
    SortedVector &SV;

  public:
    BatchInserter(SortedVector &SV) : SV(SV) {
      SV.BatchInsertInProgress = true;
    }

    ~BatchInserter() { commit(); }

    void commit() {
      revng_assert(SV.BatchInsertInProgress);
      SV.BatchInsertInProgress = false;
      SV.sort<true>();
    }

    void insert(const T &Value) {
      revng_assert(SV.BatchInsertInProgress);
      SV.TheVector.push_back(Value);
    }
  };

  BatchInserter batch_insert() {
    revng_assert(not BatchInsertInProgress);
    return BatchInserter(*this);
  }

  class BatchInsertOrAssigner {
  private:
    SortedVector &SV;

  public:
    BatchInsertOrAssigner(SortedVector &SV) : SV(SV) {
      SV.BatchInsertInProgress = true;
    }

    ~BatchInsertOrAssigner() { commit(); }

    void commit() {
      revng_assert(SV.BatchInsertInProgress);
      SV.BatchInsertInProgress = false;
      SV.sort<false>();
    }

    void insert_or_assign(const T &Value) {
      revng_assert(SV.BatchInsertInProgress);
      SV.TheVector.push_back(Value);
    }
  };

  BatchInsertOrAssigner batch_insert_or_assign() {
    revng_assert(not BatchInsertInProgress);
    return BatchInsertOrAssigner(*this);
  }

private:
  static bool compareElements(const T &LHS, const T &RHS) {
    return compareKeys(KeyedObjectTraits<T>::key(LHS),
                       KeyedObjectTraits<T>::key(RHS));
  }

  static bool compareKeys(const key_type &LHS, const key_type &RHS) {
    return std::less<key_type>()(LHS, RHS);
  }

  static bool elementsEqual(const T &LHS, const T &RHS) {
    return keysEqual(KeyedObjectTraits<T>::key(LHS),
                     KeyedObjectTraits<T>::key(RHS));
  }

  static bool keysEqual(const key_type &LHS, const key_type &RHS) {
    return not compareKeys(LHS, RHS) and not compareKeys(RHS, LHS);
  }

  template<bool KeepFirst>
  void sort() {
    // Sort
    std::stable_sort(begin(), end(), compareElements);

    // Remove duplicates keeping the last instance of each element
    auto NewEnd = end();
    if (KeepFirst) {
      NewEnd = std::unique(begin(), end(), elementsEqual);
    } else {
      NewEnd = unique_last(begin(), end(), elementsEqual);
    }
    TheVector.erase(NewEnd, end());
  }
};

#endif // SORTEDVECTOR_H
