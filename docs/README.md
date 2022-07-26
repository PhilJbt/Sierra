![Sierra logo](logo_v2.png)


&#160;

```diff
- ⚠️ STILL IN DEVELOPMENT, DO NOT USE IT FOR PRODUCTION IN ITS CURRENT STATE ⚠️ -
```

&#160;

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
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed et nisi efficitur, convallis tellus eget, volutpat nibh. Phasellus ut orci ut arcu tincidunt tincidunt. Quisque consequat vestibulum arcu quis euismod. Sed porta eu turpis nec iaculis.\
\
Check the [header file](../cpp/Cserializing.hpp#L01-L02) for explanations.\
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
```
| Description | From | To | Size | Time | CPU | Iterations |
| ------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------- |
| Pack structs | 3 aggregate structs | Serializer package | 3 x 500 bytes | 5316 ns | 5441 ns | 112000 |
| Serialize structs | Serializer package | Bytes array | 3 x 500 bytes | 341 ns | 338 ns | 2036364 |
| Unserialize structs | Bytes array | Serializer package | 3 x 500 bytes | 5159 ns | 5156 ns | 100000 |
| Unpack structs | Serializer package | 3 aggregate structs | 3 x 500 bytes | 493 ns | 488 ns | 1120000 |
| Pack variable | uin64_t Array | Serializer package | 1500 bytes | 693 ns | 670 ns | 1120000 |
| Serialize variable | Serializer package | Bytes array | 1500 bytes | 74.8 ns | 75.0 ns | 8960000 |
| Unserialize variable | Bytes array | Serializer package | 1500 bytes | 694 ns | 698 ns | 1120000 |
| Unpack variable | Serializer package | uin64_t Array | 1500 bytes | 53.9 ns | 54.7 ns | 10000000 |


###### [Return to index](#index)
