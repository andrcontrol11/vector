#include <memory>

template<typename T>
class RawMemory {
public:
    T *ptr_ = nullptr;
    size_t capacity_ = 0;

    T *allocate(size_t n) {
        return static_cast<T *>(operator new(n * sizeof(T)));
    }

    void deallocate(T *buf) {
        operator delete(buf);
    }

    void swap(RawMemory &other) {
        std::swap(ptr_, other.ptr_);
        std::swap(capacity_, other.capacity_);
    }

    RawMemory() = default;

    RawMemory(size_t n) {
        ptr_ = allocate(n);
        capacity_ = n;
    }

    RawMemory &operator=(const RawMemory &other) = delete;

    RawMemory &operator=(RawMemory &&other) {
        swap(other);
        return *this;
    }


    T &operator[](size_t iter) {
        return ptr_[iter];
    }

    const T &operator[](size_t iter) const {
        return ptr_[iter];
    }

    ~RawMemory() {
        deallocate(ptr_);
    }
};

// T* = new[capacity_];
template<typename T>
class Vector {
private:
    RawMemory<T> data;

    size_t sz = 0;

public:
    Vector<T>() = default;

    Vector<T>(size_t n) : data(n) {
        std::uninitialized_value_construct_n(data.ptr_, n);
        sz = n;
    }

    Vector<T>(const Vector<T> &other) : data(other.sz) {
        std::uninitialized_copy_n(other.data.ptr_, other.sz, data.ptr_);
        sz = other.sz;
    }

    T &operator[](size_t iter) {
        return data[iter];
    }

    const T &operator[](size_t iter) const {
        return data[iter];
    }

    void swap(Vector<T> &other) {
        data.swap(other.data);
        std::swap(sz, other.sz);
    }

    Vector<T> &operator=(const Vector<T> &other) {
        if (data.capacity_ < other.data.capacity_) {
            Vector tmp(other);
            swap(tmp);
        } else {
            for (size_t i = 0; i != std::min(sz, other.sz); ++i) {
                data[i] = other[i];
            }
            if (other.sz < sz) {
                std::destroy_n(data.ptr_ + other.sz, sz - other.sz);
            } else {
                std::uninitialized_copy_n(other.data.ptr_ + sz, other.sz - sz, data.ptr_ + sz);
            }
            sz = other.sz;
        }
        return *this;
    }

    void reserve(size_t n) {
        if (n > data.capacity_) {
            RawMemory<T> newData(n);
            std::uninitialized_move_n(data.ptr_, sz, newData.ptr_);
            std::destroy_n(data.ptr_, sz);
            data = std::move(newData);
        }
    }

    void resize(size_t NewSize) {
        reserve(NewSize);
        if (NewSize < sz) {
            std::destroy_n(data.ptr_ + NewSize, sz - NewSize);
        } else if (NewSize > sz) {
            std::uninitialized_value_construct_n(data.ptr_ + sz, NewSize - sz);
        }
        sz = NewSize;
    }

    void pop_back() {
        std::destroy_at(data.ptr_ + sz - 1);
        --sz;
    }

    void push_back(T &elem) {
        if (sz == data.capacity_) {
            reserve(sz == 0 ? 1 : sz * 2);
        }
        new(data.ptr_ + sz) T(elem);
        ++sz;
    }

    void push_back(const T &&elem) {
        if (sz == data.capacity_) {
            reserve(sz == 0 ? 1 : sz * 2);
        }
        new(data.ptr_ + sz) T(std::move(elem));
        ++sz;
    }

    size_t size() const {
        return sz;
    }

    size_t capacity() const {
        return data.capacity_;
    }

    T *begin() {
        return data.ptr_;
    }

    const T *begin() const {
        return data.ptr_;
    }

    T *end() {
        return data.ptr_ + sz;
    }

    const T *end() const {
        return data.ptr_ + sz;
    }

    void clear() {
        resize(0);
    }

    ~Vector<T>() {
        std::destroy_n(data.ptr_, sz);
    }
};
