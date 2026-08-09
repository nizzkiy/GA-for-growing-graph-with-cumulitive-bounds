#pragma once
#include <random>

namespace math {
    // Указатель на генератор, специфичный для текущей особи/итерации
    inline thread_local std::mt19937* current_task_rng = nullptr;

    // Глобальный генератор для главного потока (до OpenMP секций)
    inline std::mt19937 global_main_rng(12345);

    // Функция для привязки генератора к текущему потоку на время итерации
    inline void set_current_rng(std::mt19937* rng) {
        current_task_rng = rng;
    }

    // Получение актуального генератора
    inline std::mt19937& get_random_generator() {
        if (current_task_rng) return *current_task_rng;
        return global_main_rng;
    }

    inline int random_int(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(get_random_generator());
    }

    inline double random_double(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(get_random_generator());
    }
}