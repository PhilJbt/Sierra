#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <any>


#include "Cserializing.hpp"


#if !defined(SK_BENCHMARK)
    #define execute (main)
#endif

#define SK_COMPARE_INT(INTA, INTB) (INTA != INTB)
#define SK_COMPARE_FLT(FLTA, FLTB) (::abs(FLTA - FLTB) >= std::numeric_limits<float>::epsilon())
#define SK_COMPARE_CZ(CZA, CZB)    (::strcmp(CZA, CZB) != 0)
#define SK_COMPARE_ARR(ARRA, ARRB, SIZE) (::memcmp(ARRA, ARRB, SIZE * sizeof(ARRB)) != 0)
#define SK_COMPARE_VEC(VECA, VECB) ((VECA.size() != VECB.size()) || (std::equal(VECA.begin(), VECA.end(), VECB.begin()) != true))
#define SK_COMPARE_MAP(MAPA, MAPB) ((MAPA.size() != MAPB.size()) || (std::equal(MAPA.begin(), MAPA.end(), MAPB.begin()) != true))
#define SK_COMPARE_STR(STRA, STRB) (STRA != STRB)


struct Stest_child {
    int i = 0;

    TEST(i);
};

struct Stest_child2 {
    int i = 0;
};

struct Stest {
    Stest(int _i, float _f, std::string _str) {
        i = _i;
        f = _f;
        ::strcpy_s(cz, 5, _str.data());
    }

    Stest() {
    }

    int   i = 0;
    float f = 0;
    char  cz[5] { '\0' };

    Stest_child s;

    TEST(i, f, cz, s);
};

/*
** 
*
*/
void unitaryTests() {
    Cserializing::initialization();

    //vector of vector of builtin_type

    //std::byte
    //std::pair
    //std::tuple
    //std::stack
    //std::deque
    //std::queue
    //std::priority_queue
    //std::list
    //std::forward_list
    //std::array
    //std::set
    //std::multiset
    //std::multimap
    //std::unordered_set
    //std::unordered_map
    //std::unordered_multiset
    //std::unordered_multimap

    // T
    {
        Cserializing pe;

        int64_t i64A(INT64_MIN);
        pe.setNextData(i64A);

        pe.changeOperationType(Cserializing::Eoperation_Get);

        int64_t i64B(0);
        if (pe.isNextData(i64B))
            pe.getNextData(i64B);
        if (SK_COMPARE_INT(i64A, i64B)) throw std::runtime_error("Get UINT64 failed.");
    }

    // T *
    {
        Cserializing pe;

        int64_t *i64A(new int64_t(INT64_MIN));
        pe.setNextData(i64A);

        pe.changeOperationType(Cserializing::Eoperation_Get);

        int64_t *i64B(new int64_t(0));
        if (pe.isNextData(i64B))
            pe.getNextData(i64B);
        if (SK_COMPARE_INT((*i64A), (*i64B))) throw std::runtime_error("Get UINT64 * failed.");
        delete i64A;
        delete i64B;
    }

    // STD::VECTOR
    {
        Cserializing pe;

        std::vector<uint8_t> vecA { 11, 33, 22 };
        pe.setNextData(vecA);

        pe.changeOperationType(Cserializing::Eoperation_Get);

        std::vector<uint8_t> vecB;
        if (pe.isNextData(vecB))
            pe.getNextData(vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("Get STD::VECTOR failed.");
    }

    // STD::VECTOR *
    {
        Cserializing pe;

        std::vector<uint8_t> *vecA(new std::vector<uint8_t>({ 11, 33, 22 }));
        pe.setNextData(vecA);

        pe.changeOperationType(Cserializing::Eoperation_Get);

        std::vector<uint8_t> *vecB(new std::vector<uint8_t>());
        if (pe.isNextData(vecB))
            pe.getNextData(vecB);
        if (SK_COMPARE_VEC((*vecA), (*vecB))) throw std::runtime_error("Get STD::VECTOR * failed.");
        delete vecA;
        delete vecB;
    }



    /*
    int iA(INT_MAX);
    pe.setNextData(iA);

    pe.changeOperationType(Cserializing::Eoperation_Get);

    int iB(0);
    if (pe.isNextData(iB))
        pe.getNextData(iB);
    if (SK_COMPARE_INT(iA, iB)) throw std::runtime_error("Get INT failed.");
    */



    /*
    int iA(INT_MAX);
    pe.setNextData(iA);

    int iArrA[3] { INT_MAX, INT_MIN, INT_MAX };
    pe.setNextData(iArrA);

    unsigned int uiArrA[5] { UINT_MAX, 0, UINT_MAX, 0, UINT_MAX };
    pe.setNextData(uiArrA);

    std::vector<double> vecA { 5.5, 9.9, 7.7 };
    pe.setNextData(vecA);

    std::map<double, bool> mapA { { 1.23, true }, { 6.54, false }, { 7.89, true } };
    pe.setNextData(mapA);

    char czA2[3] { "99" };
    pe.setNextData(czA2);

    Stest sA(9, 8.7f, "zyxw");
    sA.s.i = 987;
    pe.setNextData(sA);

    pe.setNextData("mqVfWYj7g");

    std::string strA("Gt7xTdbPeMXlAAF");
    pe.setNextData(strA);

    int *iptrA1(new int());
    *iptrA1 = 5;
    pe.setNextData(iptrA1, 1);

    double *iptrA2(new double[3]);
    iptrA2[0] = 7.7;
    iptrA2[1] = 1.1;
    iptrA2[2] = 9.9;
    pe.setNextData(iptrA2, 3);

    std::vector<int64_t> *vecPtrA(new std::vector<int64_t>());
    vecPtrA->push_back(INT64_MAX);
    vecPtrA->push_back(0);
    vecPtrA->push_back(INT64_MIN);
    pe.setNextData(vecPtrA);

    std::map<int16_t, int64_t> *mapPtrA(new std::map<int16_t, int64_t>());
    mapPtrA->insert({ INT16_MAX, INT64_MAX });
    mapPtrA->insert({ 0, 0 });
    mapPtrA->insert({ INT16_MIN, INT64_MIN });
    pe.setNextData(mapPtrA);



    pe.changeOperationType(Cserializing::Eoperation_Get);

    

    int iB(0);
    if (pe.isNextData(iB))
        pe.getNextData(iB);
    if (SK_COMPARE_INT(iA, iB)) throw std::runtime_error("Get INT failed.");

    int iArrB[3] { 0 };
    pe.getNextData(iArrB);
    if (SK_COMPARE_ARR(iArrA, iArrB, 3)) throw std::runtime_error("Get INT ARRAY failed.");

    unsigned int uiArrB[5] { 0 };
    pe.getNextData(uiArrB);
    if (SK_COMPARE_ARR(uiArrB, uiArrB, 5)) throw std::runtime_error("Get UNSIGNED INT ARRAY failed.");

    std::vector<double> vecB;
    if (pe.isNextData(vecB))
        pe.getNextData(vecB);
    if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("Get VECTOR<DOUBLE> failed.");

    std::map<double, bool> mapB;
    if (pe.isNextData(mapB))
        pe.getNextData(mapB);
    if (SK_COMPARE_MAP(mapA, mapB)) throw std::runtime_error("Get MAP<DOUBLE, BOOL> failed.");

    int iArrC[3] { }; // Good size, not good type
    if (pe.isNextData(iArrC))
        pe.getNextData(iArrC);

    char czB1[5] { '\0' };
    if (pe.isNextData(czB1)) // Not good size, good type
        pe.getNextData(czB1);

    char czB2[3] { '\0' };
    if (pe.isNextData(czB2)) // Good size, good type
        pe.getNextData(czB2);
    if (SK_COMPARE_CZ(czA2, czB2)) throw std::runtime_error("Get CHAR[5] failed.");

    Stest sB;
    sB.s.i = 123;
    pe.getNextData(sB);
    if (SK_COMPARE_INT(sA.i, sB.i) || SK_COMPARE_FLT(sA.f, sB.f) || SK_COMPARE_CZ(sA.cz, sB.cz) || SK_COMPARE_INT(sA.s.i, sB.s.i))
        throw std::runtime_error("Get CUSTOM DATA STRUCTURE failed.");

    char czB3[10] { '\0' };
    pe.getNextData(czB3);
    if (SK_COMPARE_CZ("mqVfWYj7g", czB3)) throw std::runtime_error("Get CHAR[5] from CONST CHAR * failed.");

    std::string strB;
    if (pe.isNextData(strB))
        pe.getNextData(strB);
    if (SK_COMPARE_STR(strA, strB)) throw std::runtime_error("Get CHAR[5] from STD::STRING.");

    int *iptrB1(new int());
    *iptrB1 = 0;
    if (pe.isNextData(iptrB1, 1))
        pe.getNextData(iptrB1);
    if (SK_COMPARE_INT(*iptrA1, *iptrB1)) throw std::runtime_error("Get INT* failed.");
    delete iptrA1;
    delete iptrB1;

    int i = pe.isNextCount();
    double *iptrB2(new double[pe.isNextCount()]);
    if (pe.isNextData(iptrB2, 3))
        pe.getNextData(iptrB2);
    if (SK_COMPARE_ARR(iptrA2, iptrB2, i)) throw std::runtime_error("Get *INT[5] from CONST CHAR * failed.");
    delete[] iptrA2;
    delete[] iptrB2;

    std::vector<int64_t> *vecPtrB(new std::vector<int64_t>());
    pe.getNextData(vecPtrB);
    if (SK_COMPARE_VEC((*vecPtrA), (*vecPtrB))) throw std::runtime_error("Get VECTOR<INT64_T>* failed.");
    delete vecPtrA;
    delete vecPtrB;

    std::map<int16_t, int64_t> *mapPtrB(new std::map<int16_t, int64_t>());
    pe.getNextData(mapPtrB);
    if (SK_COMPARE_VEC((*mapPtrA), (*mapPtrB))) throw std::runtime_error("Get MAP<INT16_T, INT64_T>* failed.");
    delete mapPtrA;
    delete mapPtrB;
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
