// Non-throwing adaptation of PoDoFo auxiliary/nullable.h.
// Keeps reference support while replacing throwing value() with try_get().
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include <optional>
#include <type_traits>
namespace PoDoFo {
template <typename T,typename Enable=void> class nullable;
template <typename T> class nullable<T,std::enable_if_t<!std::is_reference_v<T> && !std::is_pointer_v<T>>> final {
public:
    nullable()=default; nullable(std::nullptr_t) noexcept{} nullable(T value):m_value(std::move(value)){}
    nullable& operator=(std::nullptr_t) noexcept{m_value.reset();return *this;}
    nullable& operator=(T value){m_value=std::move(value);return *this;}
    bool has_value() const noexcept{return m_value.has_value();}
    T* try_get() noexcept{return m_value?&*m_value:nullptr;}
    const T* try_get() const noexcept{return m_value?&*m_value:nullptr;}
    T* operator->() noexcept{return try_get();} const T* operator->() const noexcept{return try_get();}
    T& operator*() noexcept{return *m_value;} const T& operator*() const noexcept{return *m_value;}
    explicit operator bool() const noexcept{return has_value();}
private: std::optional<T> m_value;
};
template <typename T> class nullable<T&,void> final {
public:
    nullable() noexcept=default; nullable(std::nullptr_t) noexcept{} nullable(T& value) noexcept:m_value(&value){}
    nullable& operator=(std::nullptr_t) noexcept{m_value=nullptr;return *this;}
    nullable& operator=(T& value) noexcept{m_value=&value;return *this;}
    bool has_value() const noexcept{return m_value!=nullptr;}
    T* try_get() const noexcept{return m_value;}
    T* operator->() const noexcept{return m_value;}
    T& operator*() const noexcept{return *m_value;}
    explicit operator bool() const noexcept{return has_value();}
private: T* m_value=nullptr;
};
}
