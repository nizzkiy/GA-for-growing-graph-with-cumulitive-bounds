#ifndef SRC_LIST_H
#define SRC_LIST_H

#include <initializer_list>
#include <iostream>
#include <utility>
#include <cstddef>

#if __cplusplus >= 201103L
    //#include "move.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifndef NOEXCEPT_AVAL
#define NOEXCEPT_AVAL noexcept
#endif

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#if __cplusplus >= 201103L
#define NOEXCEPT_AVAL noexcept
#else
#define NOEXCEPT_AVAL
#endif
#endif

#include "base_def.h"

namespace containers {

template<typename T>
struct Node {
    T data;
    Node<T> *next;
    Node<T> *prev;

#if __cplusplus >= 201103L
    Node(): next(nullptr), prev(nullptr) {}
#endif

    Node(const T&_data): data(_data), next(nullptr), prev(nullptr) {}

    Node(T&& _data): data(std::forward<T>(_data)), next(nullptr), prev(nullptr) {}

    template<typename ...Args>
    Node(Args&& ...args): data(std::forward<Args>(args)...), next(nullptr), prev(nullptr) {}

    Node(const Node<T> *other): data(other->data), next(other->next), prev(other->prev) {}

    ~Node() {}
};

/**
 * Обычный итератор
*/
template<typename T>
struct list_iterator {
    list_iterator();

    list_iterator(Node<T> *_node);

    list_iterator &operator ++() NOEXCEPT_AVAL;

    list_iterator &operator --() NOEXCEPT_AVAL;

    T *operator ->() const NOEXCEPT_AVAL;

    T &operator *() const NOEXCEPT_AVAL;

    bool operator !=(const list_iterator<T> &other) const NOEXCEPT_AVAL;

    bool operator ==(const list_iterator<T> &other) const NOEXCEPT_AVAL;

    Node<T> *node;
};

template<typename T>
list_iterator<T>::list_iterator(): node(nullptr) {}

template<typename T>
list_iterator<T>::list_iterator(Node<T> *_node): node(_node) {}

template<typename T>
list_iterator<T> &list_iterator<T>::operator++ () NOEXCEPT_AVAL {
    node = node->next;
    return *this;
}

template<typename T>
list_iterator<T> &list_iterator<T>::operator-- () NOEXCEPT_AVAL {
    node = node->prev;
    return *this;
}

template<typename T>
T *list_iterator<T>::operator-> () const NOEXCEPT_AVAL {
    return &(node->data);
}

template<typename T>
T &list_iterator<T>::operator* () const NOEXCEPT_AVAL {
    return node->data;
}

template<typename T>
bool list_iterator<T>::operator!= (const list_iterator<T> &other) const NOEXCEPT_AVAL {
    return !(node == other.node);
}

template<typename T>
bool list_iterator<T>::operator== (const list_iterator<T> &other) const NOEXCEPT_AVAL {
    return (node == other.node);
}

template<typename T>
struct const_list_iterator {
    const_list_iterator();

    const_list_iterator(const Node<T> *_node);

    const_list_iterator &operator ++() NOEXCEPT_AVAL;

    const_list_iterator &operator --() NOEXCEPT_AVAL;

    T *operator ->() const NOEXCEPT_AVAL;

    T &operator *() const NOEXCEPT_AVAL;

    bool operator !=(const const_list_iterator<T> &other) const NOEXCEPT_AVAL;

    bool operator ==(const const_list_iterator<T> &other) const NOEXCEPT_AVAL;

    const Node<T> *node;
};

template<typename T>
const_list_iterator<T>::const_list_iterator(): node(nullptr) {}

template<typename T>
const_list_iterator<T>::const_list_iterator(const Node<T> *_node): node(_node) {}

template<typename T>
const_list_iterator<T> &const_list_iterator<T>::operator++ () NOEXCEPT_AVAL {
    node = node->next;
    return *this;
}

template<typename T>
const_list_iterator<T> &const_list_iterator<T>::operator-- () NOEXCEPT_AVAL {
    node = node->prev;
    return *this;
}

template<typename T>
T *const_list_iterator<T>::operator-> () const NOEXCEPT_AVAL {
    return &(node->data);
}

template<typename T>
T &const_list_iterator<T>::operator* () const NOEXCEPT_AVAL {
    return node->data;
}

template<typename T>
bool const_list_iterator<T>::operator!= (const const_list_iterator<T> &other) const NOEXCEPT_AVAL {
    return !(node == other.node);
}

template<typename T>
bool const_list_iterator<T>::operator== (const const_list_iterator<T> &other) const NOEXCEPT_AVAL {
    return (node == other.node);
}

template<typename T>
class list {
    Node<T> *root;
    Node<T> *back_root;
    size_t size;

//    static size_t max_size;

    void copy_list(const list<T> &other);

#if __cplusplus >= 201103L
    inline void move_list(list<T>&& other);
#endif

    inline void reverse_node(Node<T> *&);

    bool equal_list(const list<T> &other) const;

public:
    typedef T                       type;
    typedef T&                      type_reference;
    typedef const T&                const_type_reference;
    typedef list_iterator<T>        iterator;
    typedef const_list_iterator<T>  const_iterator;

#if __cplusplus >= 201103L
    list() noexcept;

    list(list<T>&& other);

    list(std::initializer_list<T> data);
#else
    list();
#endif

    list(const list<T> &other);

    explicit list(size_t _size);

    list(size_t _size, const T &data);

    ~list();

    list<T> &operator =(const list<T> &other);

#if __cplusplus >= 201102L
    list<T> &operator =(list<T>&& other);

    list<T> &operator +=(list<T>&& other);
#endif

    list<T> &operator +=(const list<T> &other);

    bool operator ==(const list<T> &other) const;

    bool operator !=(const list<T> &other) const;

    inline type_reference front();

    const_type_reference front() const;

    type_reference back();

    const_type_reference back() const;

    void insert(iterator pos, const_type_reference data);

#if __cplusplus >= 201103L
    typedef T&&         universal_reference;

    void insert(iterator pos, universal_reference data);

    void push_front(universal_reference data);

    void push_back(universal_reference data);

    template<typename ...Args>
    void emplace_front(Args&&... args);

    template<typename ...Args>
    void emplace_back(Args&&... args);
#endif

    void push_front(const_type_reference data);    

    void push_back(const_type_reference data);

    void pop_front();

    void pop_back();

    void clear();

    void resize(size_t count);

    void resize(size_t count, const_type_reference data);

    void reverse();

    inline bool empty() const;

    inline size_t len() const;

    iterator begin() NOEXCEPT_AVAL;

    iterator end() NOEXCEPT_AVAL;

    const_iterator cbegin() const NOEXCEPT_AVAL;

    const_iterator cend() const NOEXCEPT_AVAL;
};

//template<typename T>
//size_t list<T>::max_size = static_cast<size_t>(-1);

template<typename T>
void list<T>::copy_list(const list<T> &other) {
    if (other.size == 0) {
        return;
    }
    
    push_back(other.root->data);

    Node<T> *iter = other.root->next;
    while (iter != nullptr) {
        push_back(iter->data);
        iter = iter->next;
    }
}

#if __cplusplus >= 201103L
template<typename T>
inline void list<T>::move_list(list<T>&& other) {
    root = other.root;
    back_root = other.back_root;
    size = other.size;

    other.root = nullptr;
    other.back_root = nullptr;
    other.size = 0;
}
#endif

template<typename T>
inline void list<T>::reverse_node(Node<T> *&node) {
    Node<T> *tmp_node = node->next;
    node->next = node->prev;
    node->prev = tmp_node;
}

template<typename T>
bool list<T>::equal_list(const list<T> &other) const {
    if (size != other.size) {
        return false;
    }

    Node<T> *iter = root;
    Node<T> *iterOther = other.root;
    while (iter != nullptr) {
        if (iter->data == iterOther->data) {
            iter = iter->next;
            iterOther = iterOther->next;
            continue;
        }
        return false;
    }

    return true;
}

template<typename T>
#if __cplusplus >= 201103L
list<T>::list() noexcept: root(nullptr), back_root(nullptr), size(0) {}

template<typename T>
list<T>::list(list<T>&& other) {
    move_list(other);
}

template<typename T>
list<T>::list(std::initializer_list<T> data): root(nullptr), back_root(nullptr), size(0) {
    for (auto i: data) {
        push_back(i);
    }
}
#else
list<T>::list(): root(NULL), back_root(NULL), size(0) {}
#endif

template<typename T>
list<T>::list(const list<T> &other): root(nullptr), back_root(nullptr), size(0) {
    copy_list(other);
}

template<typename T>
list<T>::list(size_t _size): root(nullptr), back_root(nullptr), size(0) {
    for (size_t i = _size; i > 0; i--) {
        emplace_back();
    }
}

template<typename T>
list<T>::list(size_t _size, const T &data): root(nullptr), back_root(nullptr), size(_size) {
    for (size_t i = _size; i > 0; i--) {
        push_back(data);
    }
}

template<typename T>
list<T>::~list() {
    clear();
}

template<typename T>
list<T> &list<T>::operator= (const list<T> &other) {
    if (*this == other) {
        return *this;
    }

    clear();
    copy_list(other);

    return *this;
}

#if __cplusplus >= 201103L
template<typename T>
list<T> &list<T>::operator= (list<T>&& other) {
    clear();
    move_list(other);

    return *this;
}

template<typename T>
list<T> &list<T>::operator+= (list<T> &&other) {
    size += other.size;
    Node<T> *old_back_root = back_root;
    old_back_root->next = other.root;
    other.root->prev = old_back_root;
    back_root = other.back_root;

    other.size = 0;
    other.root = nullptr;
    other.back_root = nullptr;

    return *this;
}

#endif

template<typename T>
list<T> &list<T>::operator+= (const list<T> &other) {
    list<T> *sumList = new list<T>(other);

    size += other.size;
    Node<T> *old_back_root = back_root;
    old_back_root->next = sumList->root;
    sumList->root->prev = old_back_root;
    back_root = sumList->back_root;

    return *this;
}

template<typename T>
bool list<T>::operator== (const list<T> &other) const {
    return equal_list(other);
}

template<typename T>
bool list<T>::operator!= (const list<T> &other) const {
    return !(equal_list(other));
}

template<typename T>
typename list<T>::type_reference list<T>::front() {
    return root->data;
}

template<typename T>
typename list<T>::const_type_reference list<T>::front() const {
    return root->data;
}

template<typename T>
typename list<T>::type_reference list<T>::back() {
    return back_root->data;
}

template<typename T>
typename list<T>::const_type_reference list<T>::back() const {
    return back_root->data;
}

template<typename T>
void list<T>::insert(typename list<T>::iterator pos, typename list<T>::const_type_reference data) {
    if ((pos.node == nullptr) || (size == 1)) {
        push_back(data);
    }
    else {
        Node<T> *n = new Node<T>(data);
        Node<T> *next_node = pos.node->next;
        pos.node->next = n;
        next_node->prev = n;
        n->next = next_node;
        n->prev = pos.node;
        size++;
    }
}

#if __cplusplus >= 201103L
template<typename T>
void list<T>::insert(typename list<T>::iterator pos, typename list<T>::universal_reference data) {
    if ((pos.node == nullptr) || (size == 1)) {
        push_back(data);
    }
    else {
        Node<T> *n = new Node<T>(std::forward<universal_reference>(data));
        Node<T> *next_node = pos.node->next;
        pos.node->next = n;
        next_node->prev = n;
        n->next = next_node;
        n->prev = pos.node;
        size++;
    }
}


template<typename T>
void list<T>::push_front(universal_reference data) {
    Node<T> *n = new Node<T>(std::forward<T>(data));

    if (size == 0) {
        root = n;
        back_root = n;
        size = 1;
    }
    else {
        Node<T> *old_root = root;
        root->prev = n;
        root = n;
        root->next = old_root;
        size++;
    }
}

template<typename T>
void list<T>::push_back(universal_reference data) {
    Node<T> *n = new Node<T>(std::forward<T>(data));

    if (size == 0) {
        root = n;
        back_root = n;
        size = 1;
    }
    else {
        Node<T> *old_back_root = back_root;
        back_root->next = n;
        back_root = n;
        back_root->prev = old_back_root;
        size++;
    }
} 

template<typename T>
template<typename ...Args>
void list<T>::emplace_front(Args&&... args) {
    Node<T> *n = new Node<T>(std::forward<Args>(args)...);

    if (size == 0) {
        root = n;
        back_root = n;
        size = 1;
    }
    else {
        Node<T> *old_root = root;
        root->prev = n;
        root = n;
        root->next = old_root;
        size++;
    }
}

template<typename T>
template<typename ...Args>
void list<T>::emplace_back(Args&& ...args) {
    Node<T> *n = new Node<T>(std::forward<Args>(args)...);
    
    if (size == 0) {
        root = n;
        back_root = n;
        size = 1;
    }
    else {
        Node<T> *old_back_root = back_root;
        back_root->next = n;
        back_root = n;
        back_root->prev = old_back_root;
        size++;
    }
}
#endif

template<typename T>
void list<T>::push_front(const_type_reference data) {
    Node<T> *n = new Node<T>(data);

    if (size == 0) {
        root = n;
        back_root = n;
        size = 1;
    }
    else {
        Node<T> *old_root = root;
        root->prev = n;
        root = n;
        root->next = old_root;
        size++;
    }
}

template<typename T>
void list<T>::push_back(const_type_reference data) {
    Node<T> *n = new Node<T>(data);

    if (size == 0) {
        root = n;
        back_root = n;
        size = 1;
    }
    else {
        Node<T> *old_back_root = back_root;
        back_root->next = n;
        back_root = n;
        back_root->prev = old_back_root;
        size++;
    }
}

template<typename T>
void list<T>::pop_front() {
    if (size == 0) {
        return;
    }
    else if (size == 1) {
        delete root;
        root = nullptr;
        back_root = nullptr;
    }
    else {
        Node<T> *delete_old_root = root;
        root = root->next;
        root->prev = nullptr;
        delete delete_old_root;
    }

    size--;
}

template<typename T>
void list<T>::pop_back() {
    if (size == 0) {
        return;
    }
    if (size == 1) {
        delete back_root;
        back_root = nullptr;
        root = nullptr;
    }
    else {
        Node<T> *delete_old_back_root = back_root;
        back_root = back_root->prev;
        back_root->next = nullptr;
        delete delete_old_back_root;
    }

    size--;
}

template<typename T> 
void list<T>::clear() {
    if (size == 0) {
        return;
    }
    else if (size == 1) {
        delete root;
    }
    else {
        Node<type> *delete_node;
        while (root != back_root) {
            delete_node = root;
            root = root->next;
            delete delete_node;
        }

        delete root;
    }
    
    root = nullptr;
    back_root = nullptr;
    size = 0;
}

template<typename T>
void list<T>::resize(size_t count) {
    if (count == size) {
        return;
    }
    else if (count < size) {
        Node<type> *delete_node;
        while (count != 0) {
            delete_node = back_root;
            back_root = back_root->prev;
            delete delete_node;
            count--;
        }
    }
    else {
        for (unsigned int i = 0; i < count; i++) {
            Node<T> *push_back_data = new Node<T>();
        }
    }
}

template<typename T>
void list<T>::resize(size_t count, typename list<T>::const_type_reference data) {
    if (count == size) {
        return;
    }
    else if (count < size) {
        Node<type> *delete_node;
        while (count != 0) {
            delete_node = back_root;
            back_root = back_root->prev;
            delete delete_node;
            count--;
        }
    }
    else {
        for (unsigned int i = 0; i < count; i++) {
            push_back(data);
        }
    }
}

template<typename T>
void list<T>::reverse() {
    if (size <= 1) {
        return;
    }
    else {
        Node<T> *tmp_node = root;
        Node<T> *tmp_back_node = back_root;
        root = back_root;
        back_root = tmp_node;

        root->next = tmp_back_node->prev;
        root->prev = nullptr;

        back_root->prev = tmp_node->next;
        back_root->next = nullptr;

        if (size == 2) {
            return;
        }
        else {
            for (Node<T> *iterate = root->next; iterate != back_root; iterate = iterate->next) {
                reverse_node(iterate);
            }
        }
    }
}

template<typename T>
inline bool list<T>::empty() const {
    return (size == 0);
}

template<typename T>
inline size_t list<T>::len() const {
    return size;
}

template<typename T>
typename list<T>::iterator list<T>::begin() NOEXCEPT_AVAL {
    return iterator(root);
}

template<typename T>
typename list<T>::iterator list<T>::end() NOEXCEPT_AVAL {
    return iterator();
}

template<typename T>
typename list<T>::const_iterator list<T>::cbegin() const NOEXCEPT_AVAL {
    return const_iterator(root);
}

template<typename T>
typename list<T>::const_iterator list<T>::cend() const NOEXCEPT_AVAL {
    return const_iterator();
}

/**
 * TODO
 * make AbstractList pure virtual
*/
template<typename Type>
class AbstractList {
protected:
    Node<Type> *root;
    Node<Type> *back_root;
    size_t size;
public:
    typedef Type            type;
    typedef Type&           type_reference;
    typedef Type*           type_pointer;
    typedef const Type&     const_type_reference;
    typedef const Type*     const_type_pointer;

#if __cplusplus >= 201103L
    AbstractList(): root(nullptr), back_root(nullptr), size(0) {}
#else
    AbstractList(): root(NULL), back_root(NULL), size(0) {}
#endif

    ~AbstractList() {
        if (isEmpty()) {
            return;
        }
        else if (size == 1) {
            delete root;
        }
        else {
            Node<type> *delete_node;
            while (root != back_root) {
                delete_node = root;
                root = root->next;
                delete delete_node;
            }

            delete root;
        }
    }

    bool isEmpty() {
        return root == nullptr;
    }

    size_t len() {
        return size;
    }

    virtual void insert(const_type_reference data) = 0;

    bool inList(const_type_reference data_compare) {
        if (isEmpty()) {
            return false;
        }
        if (this->size == 1) {
            return (this->root->data == data_compare);
        }
        for (Node<type> *t = this->root; t != nullptr; t = t->next) {
            if (t->data == data_compare) {
                return true;
            }
        }

        return false;
    }

    bool operator== (const_type_reference data_compare) {
        return this->root->data == data_compare;
    }
};

/**
 * list containing only unique values
*/
template<typename Type>
class UniqueList: public AbstractList<Type> {
private:
    using AbstractList<Type>::root;
    using AbstractList<Type>::back_root;
    using AbstractList<Type>::size;
public:
    typedef Type            type;
    typedef Type&           type_reference;
    typedef Type*           type_pointer;
    typedef const Type&     const_type_reference;
    typedef const Type*     const_type_pointer;

    using AbstractList<Type>::isEmpty;
    using AbstractList<Type>::len;
    using AbstractList<Type>::inList;
    using AbstractList<Type>::operator==;

    void insert(const_type_reference insert_data) {
        if (isEmpty()) {
            Node<type> *n = new Node<type>(insert_data);
            root = n;
            back_root = n;
            size = 1;
            return;
        }

        for (Node<type> *t = root; t != nullptr; t = t->next) {
            if (t->data == insert_data) {
                return;
            }
        }

        Node<type> *n = new Node<type>(insert_data);
        Node<type> *old_back_root = back_root;
        back_root->next = n;
        back_root = n;
        back_root->prev = old_back_root;
        size++;
    }
};

/**
 * Список, содержащий отсортированные уникальные значения
 * TODO: перенести всю логику в containers::list для единообразия
*/
template<typename Type>
class UniqueSortedList: public AbstractList<Type> {
private:
    using AbstractList<Type>::root;
    using AbstractList<Type>::back_root;
    using AbstractList<Type>::size;
public:
    typedef Type            type;
    typedef Type&           type_reference;
    typedef Type*           type_pointer;
    typedef const Type&     const_type_reference;
    typedef const Type*     const_type_pointer;

    using AbstractList<Type>::isEmpty;
    using AbstractList<Type>::len;
    using AbstractList<Type>::inList;
    using AbstractList<Type>::operator==;

    void insert(const_type_reference insert_data) {
        if (isEmpty()) {
            Node<type> *n = new Node<type>(insert_data);
            root = n;
            back_root = n;
            size = 1;
            return;
        }

        if (size == 1) {
            if (insert_data < root->data) {
                Node<type> *n = new Node<type>(insert_data);
                Node<type> *old_root = root;
                root = n;
                n->next = old_root;
                back_root = old_root;
                back_root->prev = n;
            }
            else if (insert_data > root->data) {
                Node<type> *n = new Node<type>(insert_data);
                Node<type> *old_back_root = back_root;
                back_root = n;
                n->prev = root;
                root->next = n;
            }
            else {
                return;
            }
            size++;
            return;
        }

        for (Node<type> *t = root; t != nullptr; t = t->next) {
            if (insert_data < t->data) {
                Node<type> *n = new Node<type>(insert_data);
                t->prev->next = n;
                n->prev = t->prev;
                n->next = t;
                t->prev = n;
                size++;
                return;
            }
            else if (insert_data > t->data) {
                continue;
            }
            else if (insert_data == t->data) {
                return;
            }
        }

        Node<type> *n = new Node<type>(insert_data);
        Node<type> *old_back_root = back_root;
        back_root->next = n;
        back_root = n;
        back_root->prev = old_back_root;
        size++;
    }

    unsigned int positionBetween(type_reference _between_data) 
    {
        if (_between_data < root->data) {
            return 0;
        }

        u_int result = 0;
        for (Node<type> *t = root; t != nullptr; t = t->next) {
            if (_between_data > t->data) {
                result++;
            }
            else {
                return result;
            }
        }

        return result;
    }

    bool inListWithNum(type_reference _data, type_reference _num) {
        u_int iC = 1;
        for (Node<type> *t = root; t != nullptr; t = t->next) {
            if (t->data == _data) {
                _num = iC;
                return true;
            }
            else {
                iC++;
            }
        }

        return false;
    }

    type_reference getRootData() const {
        return root->data;
    }

    type pop_back() {
        //if (isEmpty()) {
        //    return nullptr;
        //}
        type result = back_root->data;

        Node<type> *old_back_root = back_root;
        if (size > 1) {
            back_root = back_root->prev;
            back_root->next = nullptr;
        }
        else if (size == 1) {
            root = nullptr;
            back_root = nullptr;
        }
        delete old_back_root;
        size--;

        return result; 
    }
};


}

#endif