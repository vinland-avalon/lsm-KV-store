#ifndef _SKIP_LIST_ITER_H_
#define _SKIP_LIST_ITER_H_

#include "skip_list.h"

template <typename T>
class ListIter {

  public:
    using value_type = T;
    using reference = T &;
    using const_referenct = const T &;
    using pointer = T *;
    using const_pointor = const T *;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    ListIter(pointer p = nullptr) : Iter(p) {}

    bool operator==(const ListIter &rhs) const noexcept {
        return Iter == rhs.Iter;
    }
    bool operator!=(const ListIter &rhs) const noexcept {
        return Iter != rhs.Iter;
    }
    ListIter &operator++() {
        Iter = Iter->next();
        return *this;
    }
    // ListIter &operator++(int) {
    //     value_type tmp = *this;
    //     ++&*this;
    //     return tmp;
    // }
    reference operator*() {
        return *Iter;
    }
    pointer operator->() {
        return Iter;
    }
    pointer Iter;
};

#endif