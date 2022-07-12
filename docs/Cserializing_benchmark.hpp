#pragma once


#if defined(SK_BENCHMARK)

    #include "F:\Perso\Dawa\Bureau\serializing\serializing\serializing\Cserializing.hpp"
    #include <benchmark/benchmark.h>


    struct Stest_child2 {
    public:
        Stest_child2() {
            NULL;
        }

        uint64_t arrUi3[500] { 0 };

        TEST(arrUi3);
    };

    struct Stest_child1 {
    public:
        Stest_child1() {
            NULL;
        }

        uint64_t arrUi2[500] { 0 };

        Stest_child2 s2;

        TEST(arrUi2, s2);
    };

    struct Stest {
    public:
        Stest() {
        }

        uint64_t arrUi1[500] { 0 };

        Stest_child1 s1;

        TEST(arrUi1, s1);
    };


    
    static void Cserializing_struct_setVar(benchmark::State &state) {
        Cserializing::initialization(Stest());

        Cserializing pe;
        benchmark::DoNotOptimize(pe);

        Stest sA;
        benchmark::DoNotOptimize(sA);
        for (int i(0); i < 500; ++i) {
            sA.arrUi1[i] = rand() % UINT64_MAX;
            sA.s1.arrUi2[i] = rand() % UINT64_MAX;
            sA.s1.s2.arrUi3[i] = rand() % UINT64_MAX;
        }

        for (auto _ : state) {
            pe.setNextData(sA);
        }
    }

    static void Cserializing_struct_getVar(benchmark::State &state) {
        Cserializing::initialization(Stest());

        Cserializing pe;
        benchmark::DoNotOptimize(pe);

        Stest sA;
        benchmark::DoNotOptimize(sA);
        for (int i(0); i < 500; ++i) {
            sA.arrUi1[i] = rand() % UINT64_MAX;
            sA.s1.arrUi2[i] = rand() % UINT64_MAX;
            sA.s1.s2.arrUi3[i] = rand() % UINT64_MAX;
        }
        pe.setNextData(sA);

        pe.changeTypeTo_Get();

        Stest sB;
        benchmark::DoNotOptimize(sB);
        for (auto _ : state) {
            pe.changeTypeTo_Get();
            pe.getNextData(sB);
        }
    }
    static void Cserializing_struct_setArr(benchmark::State &state) {
        Cserializing::initialization(Stest());

        uint8_t *ui8Buff(nullptr);
        benchmark::DoNotOptimize(ui8Buff);
        int      iLength(0);
        benchmark::DoNotOptimize(iLength);

        Cserializing pe;
        benchmark::DoNotOptimize(pe);

        Stest sA;
        benchmark::DoNotOptimize(sA);
        for (int i(0); i < 500; ++i) {
            sA.arrUi1[i] = rand() % UINT64_MAX;
            sA.s1.arrUi2[i] = rand() % UINT64_MAX;
            sA.s1.s2.arrUi3[i] = rand() % UINT64_MAX;
        }

        pe.setNextData(sA);

        for (auto _ : state) {
            pe.serialize(&ui8Buff, iLength, true);
            delete[] ui8Buff;
            iLength = 0;
        }
    }

    static void Cserializing_struct_getArr(benchmark::State &state) {
        Cserializing::initialization(Stest());

        uint8_t *ui8Buff(nullptr);
        benchmark::DoNotOptimize(ui8Buff);
        int      iLength(0);
        benchmark::DoNotOptimize(iLength);

        Cserializing pe1;
        benchmark::DoNotOptimize(pe1);

        Stest sA;
        benchmark::DoNotOptimize(sA);
        for (int i(0); i < 500; ++i) {
            sA.arrUi1[i] = rand() % UINT64_MAX;
            sA.s1.arrUi2[i] = rand() % UINT64_MAX;
            sA.s1.s2.arrUi3[i] = rand() % UINT64_MAX;
        }

        pe1.setNextData(sA);

        pe1.serialize(&ui8Buff, iLength, true);

        Cserializing pe2;
        benchmark::DoNotOptimize(pe2);

        for (auto _ : state) {
            pe2.unserialize(&ui8Buff, iLength);
        }
    }


    static void Cserializing_var_setVar(benchmark::State &state) {
        Cserializing::initialization(Stest());

        Cserializing pe;
        benchmark::DoNotOptimize(pe);

        uint8_t sA[1500] = { 0 };
        benchmark::DoNotOptimize(sA);
        for (int i(0); i < 1500; ++i)
            sA[i] = rand() % UINT64_MAX;

        for (auto _ : state) {
            pe.setNextData(sA);
        }
    }

    static void Cserializing_var_getVar(benchmark::State &state) {
        Cserializing::initialization(Stest());

        Cserializing pe;
        benchmark::DoNotOptimize(pe);

        uint8_t sA[1500] = { 0 };
        benchmark::DoNotOptimize(sA);
        for (int i(0); i < 1500; ++i)
            sA[i] = rand() % UINT64_MAX;

        pe.setNextData(sA);

        pe.changeTypeTo_Get();

        uint8_t sB[1500] = { 0 };
        benchmark::DoNotOptimize(sB);
        for (auto _ : state) {
            pe.changeTypeTo_Get();
            pe.getNextData(sB);
        }
    }
    static void Cserializing_var_setArr(benchmark::State &state) {
        Cserializing::initialization(Stest());

        uint8_t *ui8Buff(nullptr);
        benchmark::DoNotOptimize(ui8Buff);
        int      iLength(0);
        benchmark::DoNotOptimize(iLength);

        Cserializing pe;
        benchmark::DoNotOptimize(pe);

        uint8_t sA[1500] = { 0 };
        benchmark::DoNotOptimize(sA);
        for (int i(0); i < 1500; ++i)
            sA[i] = rand() % UINT64_MAX;

        pe.setNextData(sA);

        for (auto _ : state) {
            pe.serialize(&ui8Buff, iLength, true);
            delete[] ui8Buff;
            iLength = 0;
        }
    }

    static void Cserializing_var_getArr(benchmark::State &state) {
        Cserializing::initialization(Stest());

        uint8_t *ui8Buff(nullptr);
        benchmark::DoNotOptimize(ui8Buff);
        int      iLength(0);
        benchmark::DoNotOptimize(iLength);

        Cserializing pe1;
        benchmark::DoNotOptimize(pe1);

        uint8_t sA[1500] = { 0 };
        benchmark::DoNotOptimize(sA);
        for (int i(0); i < 1500; ++i)
            sA[i] = rand() % UINT64_MAX;

        pe1.setNextData(sA);

        pe1.serialize(&ui8Buff, iLength, true);

        Cserializing pe2;
        benchmark::DoNotOptimize(pe2);

        for (auto _ : state) {
            pe2.unserialize(&ui8Buff, iLength);
        }
    }

    BENCHMARK(Cserializing_struct_setVar);
    BENCHMARK(Cserializing_struct_getVar);
    BENCHMARK(Cserializing_struct_setArr);
    BENCHMARK(Cserializing_struct_getArr);
    BENCHMARK(Cserializing_var_setVar);
    BENCHMARK(Cserializing_var_getVar);
    BENCHMARK(Cserializing_var_setArr);
    BENCHMARK(Cserializing_var_getArr);

    BENCHMARK_MAIN();

#endif