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

    std::vector<uint8_t> vec;

    TEST(vec);
};

struct Stest_child1 {
public:
    Stest_child1() {
        NULL;
    }

    std::string str = "";
    int8_t i8Arr[3] = { 0 };

    Stest_child2 s2;

    TEST(str, i8Arr, s2);
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

    int   i = 0;
    float f = .0f;
    char  cz[5] { 0 };

    Stest_child1 s1;

    TEST(i, f, cz, s1);
};

/*
** 
*
*/
void unitaryTests() {
    //std::vector SIZE == 0
    //SERIALIZE
    //std::byte
    //std::pair
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
    

    // STORED DATA -> SERIALIZED ARRAY
    {
        uint8_t *ui8Buff(nullptr);
        int      iLength(0);
        {
            Cserializing pe;

            uint64_t ui64A(UINT64_MAX);
            int8_t   i8A(INT8_MAX);
            pe.setNextData(0, ui64A);
            pe.setNextData(1, i8A);

            pe.serialize(&ui8Buff, iLength, true);
        }

    // SERIALIZED ARRAY -> STORED DATA
        {
            Cserializing pe;

            pe.unserialize(&ui8Buff, iLength);

            uint64_t ui64B(0);
            int8_t   i8B(0);
            pe.getNextData(0, ui64B);
            pe.getNextData(1, i8B);

            if (SK_COMPARE_INT(UINT64_MAX, ui64B)) throw std::runtime_error("Get T failed.");
            if (SK_COMPARE_INT(INT8_MAX,   i8B))   throw std::runtime_error("Get T failed.");
        }
    }

    // T
    {
        Cserializing pe;

        int64_t i64A(INT64_MIN);
        pe.setNextData(250, i64A);

        pe.changeTypeTo_Get();

        int64_t i64B(0);
        pe.getNextData(250, i64B);
        if (SK_COMPARE_INT(i64A, i64B)) throw std::runtime_error("Get T failed.");
    }

    // T *
    {
        Cserializing pe;

        int64_t *i64A(new int64_t);
        i64A[0] = INT64_MIN;
        pe.setNextData(250, i64A, 1);

        pe.changeTypeTo_Get();

        int64_t *i64B(new int64_t);
        i64B[0] = 0;
        pe.getNextData(250, i64B);
        if (SK_COMPARE_ARR(i64A, i64B, sizeof(i64A))) throw std::runtime_error("Get T failed.");
        delete i64A;
        delete i64B;
    }
    
    // T[N]
    {
        Cserializing pe;

        int64_t i64A[3];
        i64A[0] = INT64_MIN;
        i64A[1] = 1;
        i64A[2] = INT64_MAX;
        pe.setNextData(250, i64A);

        pe.changeTypeTo_Get();

        int64_t i64B[3];
        i64B[0] = 0;
        i64B[1] = 0;
        i64B[2] = 0;
        pe.getNextData(250, i64B);
        if (SK_COMPARE_ARR(i64A, i64B, 3)) throw std::runtime_error("Get T failed.");
    }

    // T [64]
    {
        Cserializing pe;

        int64_t i64ArrA[64] { 0 };
        for (int i(0); i < 64; ++i)
            i64ArrA[i] = (rand() % (INT64_MAX - INT64_MIN)) + INT64_MIN;
        pe.setNextData(42, i64ArrA);

        pe.changeTypeTo_Get();

        int64_t i64ArrB[64] { 0 };
        pe.getNextData(42, i64ArrB);
        if (SK_COMPARE_ARR(i64ArrA, i64ArrB, 64)) throw std::runtime_error("Get ARR failed.");
    }

    // T * []
    {
        Cserializing pe;

        int64_t *i64A(new int64_t[3]);
        i64A[0] = INT64_MIN;
        i64A[1] = 1;
        i64A[2] = INT64_MAX;
        pe.setNextData(250, i64A, 3);

        pe.changeTypeTo_Get();

        int64_t *i64B(new int64_t[pe.nextDataCount()]);
        i64B[0] = 0;
        i64B[1] = 0;
        i64B[2] = 0;
        pe.getNextData(250, i64B);
        if (SK_COMPARE_ARR(i64A, i64B, 3)) throw std::runtime_error("Get T failed.");
        delete[] i64A;
        delete[] i64B;
    }

    // STD::VECTOR
    {
        Cserializing pe;

        std::vector<uint8_t> vecA { 11, 33, 22 };
        pe.setNextData(0, vecA);

        pe.changeTypeTo_Get();

        std::vector<uint8_t> vecB;
        pe.getNextData(0, vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("Get STD::VECTOR failed.");
    }

    // STD::VECTOR *
    {
        Cserializing pe;

        std::vector<uint8_t> *vecA(new std::vector<uint8_t>({ 11, 33, 22 }));
        pe.setNextData(0, vecA);

        pe.changeTypeTo_Get();

        std::vector<uint8_t> *vecB(new std::vector<uint8_t>());
        pe.getNextData(0, vecB);
        if (SK_COMPARE_VEC((*vecA), (*vecB))) throw std::runtime_error("Get STD::VECTOR * failed.");
        delete vecA;
        delete vecB;
    }

    // STD::VECTOR [N]
    {

    }

    // STD::VECTOR<STD::VECTOR>
    {
        Cserializing pe;

        std::vector<std::vector<uint8_t>> vecA { {10, 11, 12, 13, 14}, {20, 21}, {30}, {40, 41, 42} };
        pe.setNextData(250, vecA);

        pe.changeTypeTo_Get();

        std::vector<std::vector<uint8_t>> vecB;
        pe.getNextData(250, vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("Get STD::VECTOR failed.");
    }

    // // STD::VECTOR<STD::VECTOR<STD::PAIR<UINT8, BOOL>>>
    {
        Cserializing pe;

        std::vector<std::vector<std::pair<uint8_t, bool>>> vecA {
            {
                {10, true},
                {11, false},
                {12, true}
            },
            {
                {20, false},
                {21, false}
            },
            {
                {30, true}
            },
            {
                {40, false},
                {41, true},
                {42, true},
                {43, false}
            }
        };
        pe.setNextData(37, vecA);

        pe.changeTypeTo_Get();

        std::vector<std::vector<std::pair<uint8_t, bool>>> vecB;
        pe.getNextData(37, vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("Get STD::VECTOR failed.");
    }

    // STD::VECTOR<STD::VECTOR<STD::TUPLE<UINT8, FLT, STD::STRING>>>
    {
        Cserializing pe;

        std::vector<std::vector<std::tuple<uint8_t, float, std::string>>> vecA {
            {
                {10, 1.f, "dix"},
                {11, 1.1f, "onze"},
                {12, 1.2f, "douze"}
            },
            {
                {20, 2.f, "vingt"},
                {21, 2.1f, "vingt et un"}
            },
            {
                {30, 3.f, "trente"}
            },
            {
                {40, 4.0f, "quarante"},
                {41, 4.1f, "quarante et un"},
                {42, 4.2f, "quarante deux"},
                {43, 4.3f, "quarante trois"}
            }
        };
        pe.setNextData(42, vecA);

        pe.changeTypeTo_Get();

        std::vector<std::vector<std::tuple<uint8_t, float, std::string>>> vecB;
        pe.getNextData(42, vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("Get STD::VECTOR failed.");
    }

    // STD::VECTOR<STD::VECTOR> [N]
    {

    }

    // STD::STRING
    {

    }
    
    // STD::STRING *
    {
        
    }

    // STD::STRING [N]
    {

    }

    // STD::STRING * [N]
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

    // STD::WSTRING * [N]
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

    // STD::MAP * [N]
    {

    }

    // CUSTOM DATA STRUCTURE
    {
        Cserializing pe;

        Stest sTestA(INT_MAX, FLT_MIN, "A0!-");
        sTestA.s1.i8Arr[0] = INT8_MIN;
        sTestA.s1.i8Arr[1] = 0;
        sTestA.s1.i8Arr[2] = INT8_MAX;
        sTestA.s1.str = "T95vZYftfY";
        sTestA.s1.s2.vec = { 127, 0, UINT8_MAX };
        pe.setNextData(250, sTestA);

        pe.changeTypeTo_Get();

        Stest sTestB;
        pe.getNextData(250, sTestB);

        if (SK_COMPARE_INT(sTestA.i, sTestB.i)
            || SK_COMPARE_FLT(sTestA.f, sTestB.f)
            || SK_COMPARE_CZ(sTestA.cz, sTestB.cz)
            || SK_COMPARE_ARR(sTestA.s1.i8Arr, sTestB.s1.i8Arr, 3)
            || SK_COMPARE_STR(sTestA.s1.str, sTestB.s1.str)
            || SK_COMPARE_VEC(sTestA.s1.s2.vec, sTestB.s1.s2.vec)) throw std::runtime_error("Get T failed.");
    }

    /// CUSTOM DATA STRUCTURE *
    {
        Cserializing pe;

        Stest *sTestA(new Stest);
        sTestA->s1.i8Arr[0] = INT8_MIN;
        sTestA->s1.i8Arr[1] = 0;
        sTestA->s1.i8Arr[2] = INT8_MAX;
        sTestA->s1.str = "T95vZYftfY";
        sTestA->s1.s2.vec = { 127, 0, UINT8_MAX };
        pe.setNextData(250, sTestA);

        pe.changeTypeTo_Get();

        Stest *sTestB(new Stest);
        pe.getNextData(250, sTestB);

        if (SK_COMPARE_INT(sTestA->i, sTestB->i)
            || SK_COMPARE_FLT(sTestA->f, sTestB->f)
            || SK_COMPARE_CZ(sTestA->cz, sTestB->cz)
            || SK_COMPARE_ARR(sTestA->s1.i8Arr, sTestB->s1.i8Arr, 3)
            || SK_COMPARE_STR(sTestA->s1.str, sTestB->s1.str)
            || SK_COMPARE_VEC(sTestA->s1.s2.vec, sTestB->s1.s2.vec)) throw std::runtime_error("Get T failed.");
        delete sTestA;
        delete sTestB;
    }

    // CUSTOM DATA STRUCTURE [N]
    {
        Cserializing pe;

        Stest sTestA[3];

        sTestA[0].i = INT_MIN;
        sTestA[0].f = FLT_MIN;
        ::strcpy_s(sTestA[0].cz, 5, "Aa00");
        sTestA[0].s1.i8Arr[0] = INT8_MIN;
        sTestA[0].s1.i8Arr[1] = INT8_MAX;
        sTestA[0].s1.i8Arr[2] = 0;
        sTestA[0].s1.str = "aabbcc";
        sTestA[0].s1.s2.vec = { 0, 2, UINT8_MAX };


        sTestA[1].i = 1;
        sTestA[1].f = 1.1f;
        ::strcpy_s(sTestA[1].cz, 5, "Bb22");
        sTestA[1].s1.i8Arr[0] = INT8_MAX;
        sTestA[1].s1.i8Arr[1] = INT8_MIN;
        sTestA[1].s1.i8Arr[2] = 0;
        sTestA[1].s1.str = "123";
        sTestA[1].s1.s2.vec = { 4, UINT8_MAX, 0 };


        sTestA[2].i = INT_MAX;
        sTestA[2].f = FLT_MAX;
        ::strcpy_s(sTestA[2].cz, 5, "Cc33");
        sTestA[2].s1.i8Arr[0] = INT8_MAX;
        sTestA[2].s1.i8Arr[1] = 0;
        sTestA[2].s1.i8Arr[2] = INT8_MIN;
        sTestA[2].s1.str = "gghhi";
        sTestA[2].s1.s2.vec = { UINT8_MAX, 0, 7 };

        pe.setNextData(250, sTestA);

        pe.changeTypeTo_Get();

        Stest sTestB[3];
        sTestB[0].f = 1.1f;
        sTestB[1].f = 2.2f;
        sTestB[2].f = 3.3f;
        pe.getNextData(250, sTestB);

        for (int i(0); i < 3; ++i) {
            if (SK_COMPARE_INT(sTestA[i].i, sTestB[i].i)
                || SK_COMPARE_FLT(sTestA[i].f, sTestB[i].f)
                || SK_COMPARE_CZ(sTestA[i].cz, sTestB[i].cz)
                || SK_COMPARE_ARR(sTestA[i].s1.i8Arr, sTestB[i].s1.i8Arr, 3)
                || SK_COMPARE_STR(sTestA[i].s1.str, sTestB[i].s1.str)
                || SK_COMPARE_VEC(sTestA[i].s1.s2.vec, sTestB[i].s1.s2.vec)) throw std::runtime_error("Get T failed.");
        }
    }

    // CUSTOM DATA STRUCTURE * [N]
    {
        Cserializing pe;

        Stest *sTestA(new Stest[3]);

        sTestA[0].i = INT_MIN;
        sTestA[0].f = FLT_MIN;
        ::strcpy_s(sTestA[0].cz, 5, "Aa00");
        sTestA[0].s1.i8Arr[0] = INT8_MIN;
        sTestA[0].s1.i8Arr[1] = INT8_MAX;
        sTestA[0].s1.i8Arr[2] = 0;
        sTestA[0].s1.str = "aabbcc";
        sTestA[0].s1.s2.vec = { 0, 2, UINT8_MAX };


        sTestA[1].i = 1;
        sTestA[1].f = 1.1f;
        ::strcpy_s(sTestA[1].cz, 5, "Bb22");
        sTestA[1].s1.i8Arr[0] = INT8_MAX;
        sTestA[1].s1.i8Arr[1] = INT8_MIN;
        sTestA[1].s1.i8Arr[2] = 0;
        sTestA[1].s1.str = "123";
        sTestA[1].s1.s2.vec = { 4, UINT8_MAX, 0 };


        sTestA[2].i = INT_MAX;
        sTestA[2].f = FLT_MAX;
        ::strcpy_s(sTestA[2].cz, 5, "Cc33");
        sTestA[2].s1.i8Arr[0] = INT8_MAX;
        sTestA[2].s1.i8Arr[1] = 0;
        sTestA[2].s1.i8Arr[2] = INT8_MIN;
        sTestA[2].s1.str = "gghhi";
        sTestA[2].s1.s2.vec = { UINT8_MAX, 0, 7 };

        pe.setNextData(250, sTestA, 3);

        pe.changeTypeTo_Get();

        Stest *sTestB(new Stest[pe.nextDataCount()]);
        sTestB[0].f = 1.1f;
        sTestB[1].f = 2.2f;
        sTestB[2].f = 3.3f;
        pe.getNextData(250, sTestB);

        for (int i(0); i < 3; ++i) {
            if (SK_COMPARE_INT(sTestA[i].i, sTestB[i].i)
                || SK_COMPARE_FLT(sTestA[i].f, sTestB[i].f)
                || SK_COMPARE_CZ(sTestA[i].cz, sTestB[i].cz)
                || SK_COMPARE_ARR(sTestA[i].s1.i8Arr, sTestB[i].s1.i8Arr, 3)
                || SK_COMPARE_STR(sTestA[i].s1.str, sTestB[i].s1.str)
                || SK_COMPARE_VEC(sTestA[i].s1.s2.vec, sTestB[i].s1.s2.vec)) throw std::runtime_error("Get T failed.");
        }
        delete[] sTestA;
        delete[] sTestB;
    }
}


/*
** 
*
*/
int execute(void) {

    unitaryTests();

    return 0;
}
