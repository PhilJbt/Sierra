#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <any>


#include "Cserializing.hpp"


#if !defined(SK_BENCHMARK)
    #define execute (main)
#endif

struct Stest_child {
    int p = 0;

    TEST(p);
};

struct Stest {
    Stest(int _i, float _f, std::string _str) {
        i = _i;
        f = _f;
        ::strcpy_s(c, 5, _str.data());
    }

    Stest() {
    }

    int   i = 0;
    float f = 0;
    char  c[5] { '\0' };

    Stest_child s;

    TEST(i, f, c, s);
};

/*
** 
*
*/
void unitaryTests() {
    Cserializing::initialization();
    Cserializing::registerTypes(
        (char[5])("abcd"),
        std::vector<double>(),
        std::map<double, bool>()
    );

    Cserializing pe;



    Stest sA(9, 8.7f, "zzzz");
    sA.s.p = 987;
    pe.setNextData(sA);

    std::vector<double> vecA { 5.5, 9.9, 7.7 };
    pe.setNextData(vecA);

    std::map<double, bool> mapA { { 1.23, true }, { 6.54, false }, { 7.89, true } };
    pe.setNextData(mapA);



    pe.changeOperationType(Cserializing::Eoperation_Get);



    Stest sB(1, 2.3f, "aaaa");
    sB.s.p = 123;
    pe.wtf(sB);

    std::vector<double> vecB;
    pe.wtf(vecB);

    std::map<double, bool> mapB;
    pe.wtf(mapB);
}


/*
** 
*
*/
int execute(void) {

    unitaryTests();

    return 0;
}
