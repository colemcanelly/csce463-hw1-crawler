/*
* Cole McAnelly
* CSCE 463 - Dist. Network Systems
* Fall 2024
*/
#pragma once

#include "pch.h"

namespace sync {

    // Mutex Container providing mutually exclusive access to the data type
    // Modeled after Rust's `Mutex<T>` type
    template<typename T>
    class Mutex
    {
        mutable std::mutex mutex;
        mutable T data;

    public:
        template<typename... Args>
        explicit Mutex(Args&&... args) : data(std::forward<Args>(args)...) {}

        class MutexGuardPtr {
        private:
            std::unique_lock<std::mutex> lock;
            T& data;

            MutexGuardPtr(std::unique_lock<std::mutex>&& lock, T& data)
                : lock(std::move(lock)), data(data)
            {}


        public:
            friend class Mutex<T>;
            //~MutexGuardPtr() { std::cout << "Dropping mutex guard accessor\n"; }

            MutexGuardPtr(const MutexGuardPtr&) = delete;
            MutexGuardPtr& operator=(const MutexGuardPtr&) = delete;
            MutexGuardPtr(MutexGuardPtr&&) = default;
            MutexGuardPtr& operator=(MutexGuardPtr&&) = default;

            T* operator->() { return &data; }
            const T* operator->() const { return &data; }
            T& operator*() { return data; }
            const T& operator*() const { return data; }
        };

        MutexGuardPtr lock() const {
            return MutexGuardPtr(std::unique_lock(mutex), data);
        }

        template<typename F>
        auto with_lock(F&& func) -> std::invoke_result_t<F, T&> {
            std::unique_lock lock(mutex);
            return std::invoke(std::forward<F>(func), data);
        }

        template<typename F>
        auto try_lock(F&& func) -> std::optional<std::invoke_result_t<F, T&>> {
            std::unique_lock lock(mutex, std::try_to_lock);
            if (lock.owns_lock()) {
                return std::invoke(std::forward<F>(func), data);
            }
            return std::nullopt;
        }
    };
}
