#pragma once

#include "def.h"
#include "__/dtoa_milo.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utility>

namespace fast {

// double to ascii string
inline int dtoa(double v, char* buf) {
    return milo::dtoa(v, buf);
}

// integer to hex string  (255 -> "0xff". eg.)
__coapi int u32toh(uint32 v, char* buf);
__coapi int u64toh(uint64 v, char* buf);

// integer to ascii string
__coapi int u32toa(uint32 v, char* buf);
__coapi int u64toa(uint64 v, char* buf);

inline int i32toa(int32 v, char* buf) {
    if (v >= 0) return u32toa((uint32)v, buf);
    *buf = '-';
    return u32toa((uint32)(-v), buf + 1) + 1;
}

inline int i64toa(int64 v, char* buf) {
    if (v >= 0) return u64toa((uint64)v, buf);
    *buf = '-';
    return u64toa((uint64)(-v), buf + 1) + 1;
}

class __coapi stream {
  public:
    constexpr stream() noexcept : _cap(0), _size(0), _p(0) {}
    
    explicit stream(size_t cap)
        : _cap(cap), _size(0), _p((char*) malloc(cap)) {
    }

    ~stream() {
        if (_p) free(_p);
    }

    stream(const stream&) = delete;
    void operator=(const stream&) = delete;

    stream(stream&& s) noexcept
        : _cap(s._cap), _size(s._size), _p(s._p) {
        s._p = 0;
        s._cap = s._size = 0;
    }

    stream& operator=(stream&& s) noexcept {
        if (&s != this) {
            if (_p) free(_p);
            _cap = s._cap;
            _size = s._size;
            _p = s._p;
            s._p = 0;
            s._cap = s._size = 0;
        }
        return *this;
    }

    const char* data() const {
        return _p;
    }

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    size_t capacity() const {
        return _cap;
    }

    void clear() {
        _size = 0;
    }

    void safe_clear() {
        memset(_p, 0, _size);
        _size = 0;
    }

    void resize(size_t n) {
        this->reserve(n);
        _size = n;
    }

    void reserve(size_t n) {
        if (_cap < n) {
            _p = (char*) realloc(_p, n); assert(_p);
            _cap = n;
        }
    }

    void ensure(size_t n) {
        if (_cap < _size + n) {
            _cap += ((_cap >> 1) + n);
            _p = (char*) realloc(_p, _cap); assert(_p);
        }
    }

    const char* c_str() const {
        ((stream*)this)->reserve(_size + 1);
        if (_p[_size] != '\0') _p[_size] = '\0';
        return _p;
    }

    char& back() const {
        return _p[_size - 1];
    }

    char& front() const {
        return _p[0];
    }

    char& operator[](size_t i) const {
        return _p[i];
    }

    void swap(stream& fs) noexcept {
        std::swap(fs._cap, _cap);
        std::swap(fs._size, _size);
        std::swap(fs._p, _p);
    }

    void swap(stream&& fs) noexcept {
        fs.swap(*this);
    }

    stream& append(size_t n, char c) {
        this->ensure(n);
        memset(_p + _size, c, n);
        _size += n;
        return *this;
    }

    stream& append(char c) {
        this->ensure(1);
        _p[_size++] = c;
        return *this;
    }

    stream& operator<<(bool v) {
        return v ? this->append("true", 4) : this->append("false", 5);
    }

    stream& operator<<(char v) {
        return this->append(v);
    }

    stream& operator<<(signed char v) {
        return this->operator<<((char)v);
    }

    stream& operator<<(unsigned char v) {
        return this->operator<<((char)v);
    }

    stream& operator<<(short v) {
        this->ensure(8);
        _size += fast::i32toa(v, _p + _size);
        return *this;
    }

    stream& operator<<(unsigned short v) {
        this->ensure(8);
        _size += fast::u32toa(v, _p + _size);
        return *this;
    }

    stream& operator<<(int v) {
        this->ensure(12);
        _size += fast::i32toa(v, _p + _size);
        return *this;
    }

    stream& operator<<(unsigned int v) {
        this->ensure(12);
        _size += fast::u32toa(v, _p + _size);
        return *this;
    }

    stream& operator<<(long v) {
        this->ensure(sizeof(v) * 3);
        _size += fast::i64toa(v, _p + _size);
        return *this;
    }

    stream& operator<<(unsigned long v) {
        this->ensure(sizeof(v) * 3);
        _size += fast::u64toa(v, _p + _size);
        return *this;
    }

    stream& operator<<(long long v) {
        this->ensure(24);
        _size += fast::i64toa(v, _p + _size);
        return *this;
    }

    stream& operator<<(unsigned long long v) {
        this->ensure(24);
        _size += fast::u64toa(v, _p + _size);
        return *this;
    }

    stream& operator<<(const char* v) {
        return this->append(v, strlen(v));
    }

    stream& operator<<(const void* v) {
        this->ensure(20);
        _size += fast::u64toh((uint64)v, _p + _size);
        return *this;
    }

    stream& operator<<(float v) {
        this->ensure(24);
        _size += fast::dtoa(v, _p + _size);
        return *this;
    }

    stream& operator<<(double v) {
        this->ensure(24);
        _size += fast::dtoa(v, _p + _size);
        return *this;
    }

  protected:
    stream& append(const void* p, size_t n) {
        this->ensure(n);
        memcpy(_p + _size, p, n);
        _size += n;
        return *this;
    }
    
    size_t _cap;
    size_t _size;
    char* _p;
};

} // namespace fast
