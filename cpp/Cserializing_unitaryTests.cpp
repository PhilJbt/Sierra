#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <any>


#include "Cserializing.hpp"


#if !defined(SK_BENCHMARK)
    #define execute (main)
#endif



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

    TEST(i, f, c);
};

class Cdummy {
    int i;
    char c;
    float f;
};

/*
** 
*
*/
void unitaryTests() {
    Cserializing::init(
        (char [5])("abcd"),
        Cdummy()
    );



    Stest test(123, 1.23f, "abcd");

    Cserializing pe;


    pe.setNextData(test.__v[0]);
    pe.setNextData(test.__v[1]);
    pe.setNextData(test.__v[2]);

    pe.changeOperationType(Cserializing::Eoperation_Get);

    test.i = 987;
    test.f = 7.89f;
    ::strcpy_s(test.c, "zyxw");

    int i(0);
    pe.getNextData(i);
    float f(.0f);
    pe.getNextData(f);
    char c[5] { '\0' };
    pe.getNextData(c);

    std::cout << i << std::endl;

    /*
    return;
    Cserializing::init();

    int iArr[10000] { 0 };
    for (int i(0); i < 10000; ++i)
        iArr[i] = rand() % INT_MAX;

    Cserializing pe;
    for (int i(0); i < 10000; ++i)
        pe.setNextData(&iArr[i]);

    pe.changeOperationType(Cserializing::Eoperation_Get);

    for (int i(0); i < 10000; ++i) {
        int j = 0;
        pe.getNextData(&j);
        if (j != iArr[i]) throw std::runtime_error("Get returned a wrong value.");
    }

    return;

    Cserializing::registerType(std::vector<double>());
    Cserializing::registerType(std::map<double, bool>());
    Cserializing::registerType(Stest());

    Cserializing qzd;

    int *i = new int;
    *i = 5;
    qzd.setNextData(i);
    delete i;

    float f(5.123456789f);
    qzd.setNextData(&f);

    std::vector<double> vec { 5.5, 9.9, 7.7 };
    qzd.setNextData(&vec);

    std::map<double, bool> map { { 1.23, true }, { 6.54, false }, { 7.89, true } };
    qzd.setNextData(&map);

    Stest stc(5, 7, "abcd");
    qzd.setNextData(&stc);



    qzd.changeOperationType(Cserializing::Eoperation_Get);



    int j = 0;
    qzd.getNextData(&j);
    float l = .0f;
    qzd.getNextData(&l);
    std::vector<double> m;
    qzd.getNextData(&m);
    std::map<double, bool> n;
    qzd.getNextData(&n);
    Stest o;
    qzd.getNextData(&o);
    */
}


/*
** 
*
*/
int execute(void) {

    unitaryTests();

    return 0;
}
