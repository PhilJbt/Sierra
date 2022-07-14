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
#define SK_COMPARE_ARR(ARRA, ARRB, SIZE) (::memcmp(ARRA, ARRB, SIZE) != 0)
#define SK_COMPARE_ARRPTR(ARRA, ARRB, SIZE) (::memcmp(ARRA, ARRB, sizeof(ARRA) * SIZE) != 0)
#define SK_COMPARE_VEC(VECA, VECB) ((VECA.size() != VECB.size()) || (std::equal(VECA.begin(), VECA.end(), VECB.begin()) != true))
#define SK_COMPARE_MAP(MAPA, MAPB) ((MAPA.size() != MAPB.size()) || (std::equal(MAPA.begin(), MAPA.end(), MAPB.begin()) != true))
#define SK_COMPARE_STR(STRA, STRB) (STRA != STRB)

struct Stest_child2 {
public:
    Stest_child2() {
        NULL;
    }

    std::vector<uint8_t> vec = { 0, 127, 255 };

    TEST(vec);
};

struct Stest_child1 {
public:
    Stest_child1() {
        NULL;
    }

    std::string str = "qzd";

    Stest_child2 s2;

    TEST(str, s2);
};

struct Stest {
public:
    Stest(int _i, float _f, std::string _str) {
        i = _i;
        f = _f;
        ::strcpy_s(cz, 5, _str.data());
    }

    Stest() {
    }

    int   i = 1;
    float f = 2.2f;
    char  cz[5] { '3' };

    Stest_child1 s1;

    TEST(i, f, cz, s1);
};

/*
** 
*
*/
void unitaryTests() {
    Cserializing::initialization(Stest());
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


    // TEST
    {
        Cserializing pe;

        std::vector<std::vector<uint8_t>> vecA { {10, 11, 12, 13, 14}, {20, 21}, {30}, {40, 41, 42} };
        pe.setNextData(250, vecA);

        pe.changeTypeTo_Get();

        std::vector<std::vector<uint8_t>> vecB;
        pe.getNextData(250, vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("Get STD::VECTOR failed.");
    }
    {
        Cserializing pe;

        int64_t i64A(INT64_MIN);
        pe.setNextData(250, i64A);

        pe.changeTypeTo_Get();

        int64_t i64B(0);
        pe.getNextData(250, i64B);
        if (SK_COMPARE_INT(i64A, i64B)) throw std::runtime_error("Get T failed.");
    }










    /*
    // STORED DATA -> STACK ARRAY
    {
        uint8_t *ui8Buff(nullptr);
        int      iLength(0);
        {
            Cserializing pe;

            uint64_t ui64A(UINT64_MAX);
            int8_t   i8A(INT8_MAX);
            pe.setNextData(ui64A);
            pe.setNextData(i8A);

            pe.serialize(&ui8Buff, iLength, true);
        }

    // STACK ARRAY -> STORED DATA
        {
            Cserializing pe;

            pe.unserialize(&ui8Buff, iLength);

            uint64_t ui64B(0);
            int8_t   i8B(0);
            pe.getNextData(ui64B);
            pe.getNextData(i8B);
        }
    }

    
    // T
    {
        Cserializing pe;

        int64_t i64A(INT64_MIN);
        pe.setNextData(i64A);

        pe.changeTypeTo_Get();
        
        int64_t i64B(0);
        if (!pe.nextDataType(i64B)) throw std::runtime_error("IsNextData T failed.");
        pe.getNextData(i64B);
        if (SK_COMPARE_INT(i64A, i64B)) throw std::runtime_error("Get T failed.");
    }
   
    // T *
    {
        Cserializing pe;

        int64_t *i64A(new int64_t(INT64_MIN));
        pe.setNextData(*i64A);

        pe.changeTypeTo_Get();

        int64_t *i64B(new int64_t(0));
        if (!pe.nextDataType(*i64B)) throw std::runtime_error("IsNextData T * failed.");
        pe.getNextData(*i64B);
        if (SK_COMPARE_INT((*i64A), (*i64B))) throw std::runtime_error("Get T * failed.");
        delete i64A;
        delete i64B;
    }
    
    // T[N]
    {
        Cserializing pe;

        int8_t arrI32A[64] { 0 };
        for (int i(0); i < 64; ++i)
            arrI32A[i] = rand() % ((INT32_MAX - INT32_MIN + 1) + INT32_MIN);
        pe.setNextData(arrI32A);

        pe.changeTypeTo_Get();

        int8_t arrI32B[64] { 0 };
        if (!pe.nextDataType(arrI32B)) throw std::runtime_error("IsNextData T[N] failed.");
        pe.getNextData(arrI32B);
        if (SK_COMPARE_ARR(&arrI32A, &arrI32B, sizeof(arrI32A))) throw std::runtime_error("Get T[N] failed.");
        arrI32B[63] *= 2;
        if (!SK_COMPARE_ARR(&arrI32A, &arrI32B, sizeof(arrI32A))) throw std::runtime_error("Get T[N] failed.");
    }

    // T * []
    {
        Cserializing pe;

        int64_t *arrI64A(new int64_t[64]);
        for (int i(0); i < 64; ++i)
            arrI64A[i] = rand() % ((INT64_MAX - INT64_MIN + 1) + INT64_MIN);
        pe.setNextData(*arrI64A, 64);

        pe.changeTypeTo_Get();

        int64_t *arrI64B(new int64_t[64]);
        if (!pe.nextDataType(*arrI64B)) throw std::runtime_error("IsNextData T * [] failed.");
        if (pe.nextDataCount() != 64) throw std::runtime_error("Size T * [] failed.");
        pe.getNextData(*arrI64B, 64);
        if (SK_COMPARE_ARRPTR(arrI64A, arrI64B, 64)) throw std::runtime_error("Get T * [] failed.");
        arrI64B[63] *= 2;
        if (!SK_COMPARE_ARRPTR(arrI64A, arrI64B, 64)) throw std::runtime_error("Get T * [] failed.");
        delete arrI64A;
        delete arrI64B;
    }

    // STD::VECTOR
    {
        Cserializing pe;

        std::vector<uint8_t> vecA { 11, 33, 22 };
        pe.setNextData(vecA);

        pe.changeTypeTo_Get();

        std::vector<uint8_t> vecB;
        if (!pe.nextDataType(vecB)) throw std::runtime_error("IsNextData STD::VECTOR failed.");
        pe.getNextData(vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("Get STD::VECTOR failed.");
    }

    // STD::VECTOR *
    {
        Cserializing pe;

        std::vector<uint8_t> *vecA(new std::vector<uint8_t>({ 11, 33, 22 }));
        pe.setNextData(*vecA);

        pe.changeTypeTo_Get();

        std::vector<uint8_t> *vecB(new std::vector<uint8_t>());
        if (!pe.nextDataType(*vecB)) throw std::runtime_error("IsNextData STD::VECTOR * failed.");
        pe.getNextData(*vecB);
        if (SK_COMPARE_VEC((*vecA), (*vecB))) throw std::runtime_error("Get STD::VECTOR * failed.");
        delete vecA;
        delete vecB;
    }

    // STD::VECTOR [N]
    {
        Cserializing pe;

        std::vector<std::vector<uint16_t>> vecA = { {1, 2, 3}, { 6, 5 } };
        pe.setNextData(vecA);

        pe.changeTypeTo_Get();

        std::vector<std::vector<uint16_t>> vecB;
        if (!pe.nextDataType(vecB)) throw std::runtime_error("IsNextData STD::VECTOR [] failed.");
        pe.getNextData(vecB);
        if (SK_COMPARE_VEC(vecA[0], vecB[0]) || SK_COMPARE_VEC(vecA[1], vecB[1]) || SK_COMPARE_VEC(vecA[2], vecB[2])) throw std::runtime_error("Get STD::VECTOR [] failed.");
    }

    // SAME TYPE
    {
        Cserializing pe;

        float fA(5.7f);
        pe.setNextData(fA);

        pe.changeTypeTo_Get();

        double dB(.0);
        if (pe.nextDataType(dB)) throw std::runtime_error("IsNextData FLT/DBL failed.");
        float fB(.0f);
        if (!pe.nextDataType(fB)) throw std::runtime_error("IsNextData FLT/DBL failed.");
        pe.getNextData(fB);
        if (SK_COMPARE_FLT(fA, fB)) throw std::runtime_error("Get FLT/DBL failed.");
    }

    // SAME TYPE *
    {
        Cserializing pe;

        float *fA(new float(5.7f));
        pe.setNextData(*fA);

        pe.changeTypeTo_Get();

        double *dB(new double(.0));
        if (pe.nextDataType(*dB)) throw std::runtime_error("IsNextData FLT/DBL * failed.");
        float *fB(new float(.0f));
        if (!pe.nextDataType(*fB)) throw std::runtime_error("IsNextData FLT/DBL * failed.");
        pe.getNextData(*fB);
        if (SK_COMPARE_FLT((*fA), (*fB))) throw std::runtime_error("Get FLT/DBL * failed.");
        delete fA;
        delete dB;
        delete fB;
    }

    // STD::STRING
    {

    }
    
    // STD::STRING *
    {
        Cserializing pe;

        std::string *strA(new std::string);
        *strA = "oYfgi2libM";
        pe.setNextData(*strA);

        pe.changeTypeTo_Get();

        std::string *strB(new std::string);
        if (!pe.nextDataType(*strB)) throw std::runtime_error("IsNextData STD::STRING * failed.");
        pe.getNextData(*strB);
        if (SK_COMPARE_STR(*strA, *strB)) throw std::runtime_error("Get STD::STRING * failed.");
        delete strA;
        delete strB;
    }

    // STD::STRING [N]
    {

    }
    
    // STD::WSTRING
    {

    }

    // STD::WSTRING *
    {

    }

    // STD::WSTRING [N]
    {

    }

    // STD::MAP
    {

    }

    // STD::MAP *
    {

    }

    // STD::MAP [N]
    {

    }

    // CUSTOM DATA STRUCTURE
    {
        Cserializing pe;

        Stest sA(9, 8.7f, "zyxw");
        sA.s1.str = "v5xA9QyJAQ";
        sA.s1.s2.vec = { 22, 11, 33 };
        pe.setNextData(sA);

        pe.changeTypeTo_Get();

        Stest sB(0, .0f, "\0\0\0\0");
        sB.s1.str = "";
        sB.s1.s2.vec = std::vector<uint8_t>();
        if (!pe.nextDataType(sB)) throw std::runtime_error("IsNextData CUSTOM DATA STRUCTURE failed.");
        pe.getNextData(sB);
        if (SK_COMPARE_INT(sA.i, sB.i) || SK_COMPARE_FLT(sA.f, sB.f) || SK_COMPARE_CZ(sA.cz, sB.cz) || SK_COMPARE_STR(sA.s1.str, sB.s1.str) || SK_COMPARE_VEC(sA.s1.s2.vec, sB.s1.s2.vec))
            throw std::runtime_error("Get CUSTOM DATA STRUCTURE failed.");
    }

    /// CUSTOM DATA STRUCTURE *
    {

    }

    // CUSTOM DATA STRUCTURE [N]
    {

    }
    */
    Cserializing::desinitialisation();


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
