# INDEX

| [PRESENSATION](#presentation) &#65293; [HOW IT WORKS](#how-it-works) &#65293; [USAGE](#usage) &#65293; [BENCHMARK](#benchmark) |
:----------------------------------------------------------: |

&nbsp;

# PRESENTATION
**Sierra** is a serializer written in C++20.\
It handle builtin types as user struct and classes.

###### [Return to index](#index)

&nbsp;

# HOW IT WORKS
Writing in progress.\
Check the [header file](../cpp/Cserializing.hpp#L01-L02) for explanations.\
\
There are also [some examples](../cpp/Cserializing_unitaryTests.cpp#L01-L02) and [unitary tests](../cpp/Cserializing_unitaryTests.cpp#L03-L04).

###### [Return to index](#index)

&nbsp;

# USAGE
```cpp
Cserializing::initialization();

Cserializing::registerTypes(
    std::vector<double>(),
    std::map<double, bool>()
);



Cserializing pe;



Stest sA(9, 8.7f, "zzzz");
sA.s.p = 987;
pe.setNextData(sA);

int iA(INT_MAX);
pe.setNextData(iA);

int iArrA[5] { 1, 2, 3, 4, 5 };
pe.setNextData(iArrA);

std::vector<double> vecA { 5.5, 9.9, 7.7 };
pe.setNextData(vecA);

std::map<double, bool> mapA { { 1.23, true }, { 6.54, false }, { 7.89, true } };
pe.setNextData(mapA);



pe.changeOperationType(Cserializing::Eoperation_Get);



Stest sB(1, 2.3f, "aaaa");
sB.s.p = 123;
pe.getNextData(sB);

int iB(0);
pe.getNextData(iB);

int iArrB[5] { 0 };
pe.getNextData(iArrB);

std::vector<double> vecB;
pe.getNextData(vecB);

std::map<double, bool> mapB;
pe.getNextData(mapB);
```

###### [Return to index](#index)

&nbsp;

# BENCHMARK

```
2022-07-13T01:04:54+02:00
Running Sierra.exe
DoNotOptimize enabled
Run on (8 X 3655.17 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 256 KiB (x8)
  L3 Unified 12288 KiB (x1)
----------------------------------------------------------------------------------------
Benchmark                                              Time             CPU   Iterations
----------------------------------------------------------------------------------------
Store 3 vars (500 bytes each)                       5334 ns         5312 ns       100000
Retrieve 3 vars (500 bytes each)                     495 ns          488 ns      1120000
Serialize 3 stored vars to an array (500 bytes)      337 ns          337 ns      2133333
Unserialize 1500 bytes array                        5169 ns         5156 ns       100000
```

###### [Return to index](#index)
