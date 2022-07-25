#include <iostream>


#include "Cserializing.hpp"


#if !defined(SK_BENCHMARK)
    #define execute (main)
#endif

#define SK_COMPARE_INT(INTA,        INTB)                   (INTA != INTB)
#define SK_COMPARE_BOOL(BOOLA,      BOOLB)                  (BOOLA != BOOLB)
#define SK_COMPARE_FLT(FLTA,        FLTB)                   (::fabs(FLTA - FLTB) > std::numeric_limits<float>::epsilon())
#define SK_COMPARE_CZ(CZA,          CZB)                    (::strcmp(CZA, CZB) != 0)
#define SK_COMPARE_ARR(ARRA,        ARRB,       SIZE)       (::memcmp(ARRA, ARRB, sizeof(ARRA[0]) * SIZE) != 0)
#define SK_COMPARE_ARRPTR(ARRA,     ARRB,       SIZE)       (::memcmp(ARRA, ARRB, sizeof(ARRA[0]) * SIZE) != 0)
#define SK_COMPARE_VEC(VECA,        VECB)                   (VECA != VECB)
#define SK_COMPARE_MAP(MAPA,        MAPB)                   (MAPA != MAPB)
#define SK_COMPARE_STR(STRA,        STRB)                   (STRA != STRB)

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

    //{
    //    Cserializing pe;

    //    int8_t varA(INT8_MAX);
    //    pe.setNextData(0, varA);

    //    pe.changeTypeTo_Get();

    //    int8_t varB(0);
    //    //pe.getNextData(0, varB);

    //    if (SK_COMPARE_INT(varA, varB)) throw std::runtime_error("");
    //}
    //{
    //    Cserializing pe;

    //    bool varA(true);
    //    pe.setNextData(0, varA);

    //    pe.changeTypeTo_Get();

    //    bool varB(0);
    //    //pe.getNextData(0, varB);

    //    if (SK_COMPARE_BOOL(varA, varB)) throw std::runtime_error("");
    //}
    //{
    //    Cserializing pe;

    //    int8_t varA[3] = { INT8_MIN, 0, INT8_MAX };
    //    pe.setNextData(0, varA, 3);

    //    pe.changeTypeTo_Get();

    //    int8_t varB[3] = { 0 };
    //    //pe.getNextData(0, varB, 3);

    //    for (int i(0); i < 3; ++i)
    //        if (SK_COMPARE_INT(varA[i], varB[i])) throw std::runtime_error("");
    //}
    //{
    //    Cserializing pe;

    //    bool varA[10] = { true, false, true, true, false, false, true, true, false, true };
    //    pe.setNextData(0, varA, 10);

    //    pe.changeTypeTo_Get();

    //    bool varB[10] = { 0 };
    //    //pe.getNextData(0, varB, 10);

    //    for (int i(0); i < 10; ++i)
    //        if (SK_COMPARE_BOOL(varA[i], varB[i])) throw std::runtime_error("");
    //}
    //{
    //    Cserializing pe;

    //    int8_t *varA(new int8_t[3]);
    //    varA[0] = INT8_MIN;
    //    varA[1] = 0;
    //    varA[2] = INT8_MAX;
    //    pe.setNextData(0, varA, 3);

    //    pe.changeTypeTo_Get();

    //    int8_t *varB(new int8_t[3]);
    //    ::memset(varB, 0, 3 * sizeof(int8_t));
    //    //pe.getNextData(0, varB, 3);

    //    for (int i(0); i < 3; ++i)
    //        if (SK_COMPARE_INT(varA[i], varB[i])) throw std::runtime_error("");

    //    delete[] varA;
    //    delete[] varB;
    //}
    {
        Cserializing pe;

        std::vector<std::vector<std::pair<int8_t, bool>>> *varA(new std::vector<std::vector<std::pair<int8_t, bool>>>[2]);
        varA[0].push_back({ {INT8_MIN, true}, {11, false}, {INT8_MAX, true} });
        varA[0].push_back({ {22, false} });
        varA[0].push_back({ {INT8_MAX, true}, {33, false} });
        varA[1].push_back({ {111, false} });
        varA[1].push_back({ {222, false}, {INT8_MIN, true}, {INT8_MAX, true} });
        pe.setNextData(0, &varA, 2);

        pe.changeTypeTo_Get();

        std::vector<std::vector<std::pair<int8_t, bool>>> *varB(new std::vector<std::vector<std::pair<int8_t, bool>>>[2]);
        pe.getNextData(0, &varB);
        for (int i(0); i < 2; ++i)
            for (int j(0); j < varA[i].size(); ++j)
                for (int k(0); k < varA[i][j].size(); ++k)
                    if (SK_COMPARE_INT(varA[i][j][k].first, varB[i][j][k].first) || SK_COMPARE_BOOL(varA[i][j][k].second, varB[i][j][k].second)) throw std::runtime_error("");
        delete[] varA;
        delete[] varB;
    }/*
    {
        Cserializing pe;

        bool *varA(new bool[10]);
        varA[0] = true;
        varA[1] = false;
        varA[2] = true;
        varA[3] = true;
        varA[4] = false;
        varA[5] = false;
        varA[6] = true;
        varA[7] = true;
        varA[8] = false;
        varA[9] = true;
        pe.setNextData(0, varA, 10);

        pe.changeTypeTo_Get();

        bool *varB(new bool[10]);
        ::memset(varB, 0, 10 * sizeof(bool));
        pe.getNextData(0, varB, 10);

        for (int i(0); i < 10; ++i)
            if (SK_COMPARE_BOOL(varA[i], varB[i])) throw std::runtime_error("");

        delete[] varA;
        delete[] varB;
    }
    {
        Cserializing pe;

        std::vector<std::pair<int8_t, bool>> varA = {
            {INT8_MIN, true}, {0, false}, {INT8_MAX, true}
        };
        pe.setNextData(0, varA);

        pe.changeTypeTo_Get();

        std::vector<std::pair<int8_t, bool>> varB;
        pe.getNextData(0, varB);
        for (int i(0); i < 3; ++i)
            if (SK_COMPARE_INT(varA[i].first, varB[i].first) || SK_COMPARE_BOOL(varA[i].second, varB[i].second)) throw std::runtime_error("");
    }
    {
        Cserializing pe;

        std::vector<std::tuple<int8_t, float, bool>> varA = {
            {INT8_MIN, FLT_MIN, true}, {0, .0f, false}, {INT8_MAX, FLT_MAX, true}
        };
        pe.setNextData(0, varA);

        pe.changeTypeTo_Get();

        std::vector<std::tuple<int8_t, float, bool>> varB;
        pe.getNextData(0, varB);
        for (int i(0); i < 3; ++i)
            if (SK_COMPARE_INT(std::get<0>(varA[i]), std::get<0>(varB[i]))
                || SK_COMPARE_FLT(std::get<1>(varA[i]), std::get<1>(varB[i]))
                || SK_COMPARE_BOOL(std::get<2>(varA[i]), std::get<2>(varB[i]))
            ) throw std::runtime_error("");
    }
    */

    /*
    // STD::PAIR
    {
        Cserializing pe;

        std::pair<uint8_t, bool> varA(5, true);
        pe.setNextData(0, varA);

        pe.changeTypeTo_Get();

        std::pair<uint8_t, bool> varB;
        if (!SK_COMPARE_INT(varA.first, varB.first) || !SK_COMPARE_BOOL(varA.second, varB.second)) throw std::runtime_error("");
        pe.getNextData(0, varB);
        if (SK_COMPARE_INT(varA.first, varB.first) || SK_COMPARE_BOOL(varA.second, varB.second)) throw std::runtime_error("");
    }
    
    // STD::PAIR *
    {
        Cserializing pe;

        std::pair<uint8_t, bool> *varA(new std::pair<uint8_t, bool>());
        varA->first  = UINT8_MAX;
        varA->second = true;
        pe.setNextData(0, varA, 1);

        pe.changeTypeTo_Get();

        std::pair<uint8_t, bool> *varB(new std::pair<uint8_t, bool>());
        if (!SK_COMPARE_INT(varA->first, varB->first) || !SK_COMPARE_BOOL(varA->second, varB->second)) throw std::runtime_error("");
        pe.getNextData(0, varB, 1);
        if (SK_COMPARE_INT(varA->first, varB->first) || SK_COMPARE_BOOL(varA->second, varB->second)) throw std::runtime_error("");
        delete varA;
        delete varB;
    }

    // STD::PAIR [N]
    {
        Cserializing pe;

        std::pair<uint8_t, bool> varA[3];
        varA[0].first = UINT8_MAX;
        varA[0].second = true;
        varA[1].first = 111;
        varA[1].second = false;
        varA[2].first = INT8_MIN;
        varA[2].second = true;
        pe.setNextData(0, varA);

        pe.changeTypeTo_Get();

        std::pair<uint8_t, bool> varB[3];
        varB[1].second = true;
        for (int i(0); i < 3; ++i)
            if (!SK_COMPARE_INT(varA[i].first, varB[i].first) || !SK_COMPARE_BOOL(varA[i].second, varB[i].second)) throw std::runtime_error("");
        pe.getNextData(0, varB);
        for (int i(0); i < 3; ++i)
            if (SK_COMPARE_INT(varA[i].first, varB[i].first) || SK_COMPARE_BOOL(varA[i].second, varB[i].second)) throw std::runtime_error("");
    }
    
    // STD::PAIR * [N]
    {
        Cserializing pe;

        std::pair<uint8_t, bool> *varA(new std::pair<uint8_t, bool>[3]);
        varA[0].first = UINT8_MAX;
        varA[0].second = true;
        varA[1].first = 111;
        varA[1].second = false;
        varA[2].first = INT8_MIN;
        varA[2].second = true;
        pe.setNextData(0, varA, 3);

        pe.changeTypeTo_Get();

        std::pair<uint8_t, bool> *varB(new std::pair<uint8_t, bool>[3]);
        varB[1].second = true;
        for (int i(0); i < 3; ++i)
            if (!SK_COMPARE_INT(varA[i].first, varB[i].first) || !SK_COMPARE_BOOL(varA[i].second, varB[i].second)) throw std::runtime_error("");
        pe.getNextData(0, varB, 3);
        for (int i(0); i < 3; ++i)
            if (SK_COMPARE_INT(varA[i].first, varB[i].first) || SK_COMPARE_BOOL(varA[i].second, varB[i].second)) throw std::runtime_error("");
        delete[] varA;
        delete[] varB;
    }
    */
    /*
    // OPTIMISATIONS
    {
        Cserializing pe;

        std::vector<bool> vecBoolA = { true, false, true, true, false, false, true, true, false, true };
        pe.setNextData(0, vecBoolA);

        // [] BOOL
        bool arrBoolA[10] = { true, false, false, true, false, true, false, true, true, false };
        pe.setNextData(1, arrBoolA, 10);

        // * [] BOOL
        bool *ptrArrBA(new bool[10]);
        ptrArrBA[0] = true;
        ptrArrBA[1] = false;
        ptrArrBA[2] = false;
        ptrArrBA[3] = true;
        ptrArrBA[4] = true;
        ptrArrBA[5] = false;
        ptrArrBA[6] = false;
        ptrArrBA[7] = true;
        ptrArrBA[8] = false;
        ptrArrBA[9] = true;
        pe.setNextData(2, ptrArrBA, 10);

        // [] PAIR T, BOOL
        std::pair<uint8_t, bool> arrPairBA[10] = {
            {1,     true},
            {50,    false},
            {7,     true},
            {210,   true},
            {45,    false},
            {30,    false},
            {70,    true},
            {111,   true},
            {92,    false},
            {8,     true}
        };
        pe.setNextData(3, arrPairBA, 10);

        // * [] PAIR T, BOOL
        // ...

        // VEC PAIR T, bool
        std::vector<std::pair<uint8_t, bool>> vecPairBA = {
            {8,     false},
            {92,    true},
            {111,   false},
            {70,    false},
            {30,    true},
            {45,    true},
            {210,   false},
            {7,     false},
            {50,    true},
            {1,     false}
        };
        pe.setNextData(5, vecPairBA);

        // * VEC PAIR T, bool
        // ...

        // [] TUPLE T, U, BOOL
        // ...

        // * [] TUPLE T, U, BOOL
        // ...

        // VEC TUPLE T, U, BOOL
        // ...

        // * VEC TUPLE T, U, BOOL
        // ...

        pe.changeTypeTo_Get();

        std::vector<bool> vecBoolB;
        if (!SK_COMPARE_VEC(vecBoolA, vecBoolB)) throw std::runtime_error("");
        pe.getNextData(0, vecBoolB);
        if (SK_COMPARE_VEC(vecBoolA, vecBoolB)) throw std::runtime_error("");

        bool arrBoolB[10] = { false };
        if (!SK_COMPARE_ARR(arrBoolA, arrBoolB, 10)) throw std::runtime_error("");
        pe.getNextData(1, arrBoolB, 10);
        if (SK_COMPARE_ARR(arrBoolA, arrBoolB, 10)) throw std::runtime_error("");

        bool *ptrArrBB(new bool[pe.nextDataCount()]);
        ::memset(ptrArrBB, 0, sizeof(bool) * 10);
        if (!SK_COMPARE_ARRPTR(ptrArrBA, ptrArrBB, 10)) throw std::runtime_error("");
        pe.getNextData(2, ptrArrBB, 10);
        if (SK_COMPARE_ARRPTR(ptrArrBA, ptrArrBB, 10)) throw std::runtime_error("");

        std::pair<uint8_t, bool> arrPairBB[10];
        arrPairBB[0].second = false;
        arrPairBB[1].second = true;
        arrPairBB[2].second = false;
        arrPairBB[3].second = false;
        arrPairBB[4].second = true;
        arrPairBB[5].second = true;
        arrPairBB[6].second = false;
        arrPairBB[7].second = false;
        arrPairBB[8].second = true;
        arrPairBB[9].second = false;
        for (int i(0); i < 10; ++i)
            if (!SK_COMPARE_INT(arrPairBA[i].first, arrPairBB[i].first) || !SK_COMPARE_BOOL(arrPairBA[i].second, arrPairBB[i].second)) throw std::runtime_error("");
        pe.getNextData(3, arrPairBB, 10);
        for (int i(0); i < 10; ++i)
            if (SK_COMPARE_INT(arrPairBA[i].first, arrPairBB[i].first) || SK_COMPARE_BOOL(arrPairBA[i].second, arrPairBB[i].second)) throw std::runtime_error("");

        // ...

        std::vector<std::pair<uint8_t, bool>> vecPairBB;
        for (int i(0); i < 3; ++i)
            if (!SK_COMPARE_VEC(vecPairBA[i].first, vecPairBB[i].first) || !SK_COMPARE_BOOL(vecPairBA[i].second, vecPairBB[i].second)) throw std::runtime_error("");
        pe.getNextData(5, vecPairBB);
        for (int i(0); i < 3; ++i)
            if (SK_COMPARE_VEC(vecPairBA[i].first, vecPairBB[i].first) || SK_COMPARE_BOOL(vecPairBA[i].second, vecPairBB[i].second)) throw std::runtime_error("");







        delete[] ptrArrBA;
        delete[] ptrArrBB;
    }
    
    // STORED DATA -> SERIALIZED ARRAY
    {
        uint8_t *ui8Buff(nullptr);
        int      iLength(0);

        uint64_t ui64A(UINT64_MAX);
        int8_t i8A(INT8_MAX);
        std::vector<int64_t> vecA = { INT64_MAX, 0, INT64_MIN };
        uint16_t *ptrUint16A = (new uint16_t);
                 *ptrUint16A = UINT16_MAX;
        int8_t arrInt8A[3] = { INT8_MIN, -3, INT8_MAX };
        std::vector<bool> vecBoolA = { true, false, true, true, false, false, true, true, false, true };
        float *ptrArrFloatA = (new float[3]);
            ptrArrFloatA[0] = FLT_MIN;
            ptrArrFloatA[1] = 1.654f;
            ptrArrFloatA[2] = FLT_MAX;
        std::vector<uint16_t> vecUi16EmptyA;
        std::vector<float> vecFemptyA;
        Stest sTestA(INT_MAX, -180.0f, "A0!-");
            sTestA.s1.i8Arr[0] = INT8_MIN;
            sTestA.s1.i8Arr[1] = 0;
            sTestA.s1.i8Arr[2] = INT8_MAX;
            sTestA.s1.str = "T95vZYftfY";
            sTestA.s1.s2.vec = { 127, 0, UINT8_MAX };
        {
            Cserializing pe;

            pe.setNextData(0, ui64A);
            pe.setNextData(1, i8A);
            pe.setNextData(2, vecA);
            pe.setNextData(3, ptrUint16A, 1);
            pe.setNextData(4, arrInt8A, 3);
            pe.setNextData(5, vecBoolA);
            pe.setNextData(6, ptrArrFloatA, 3);
            if (vecUi16EmptyA.size() > 0)
                pe.setNextData(7, vecUi16EmptyA);
            pe.setNextData(8, vecFemptyA);
            pe.setNextData(9, sTestA);

            pe.serialize(&ui8Buff, iLength, true);
        }

    // SERIALIZED ARRAY -> STORED DATA
        {
            Cserializing pe;

            pe.unserialize(&ui8Buff, iLength);


            uint64_t ui64B(0);
            int8_t i8B(0);
            std::vector<int64_t> vecB;
            uint16_t *ptrUint16B = (new uint16_t);
                *ptrUint16B = 0;
            int8_t arrInt8B[3] = { 0 };
            std::vector<bool> vecBoolB;
            float *ptrArrFloatB = (new float[3]);
                ptrArrFloatB[0] = .0f;
                ptrArrFloatB[1] = .0f;
                ptrArrFloatB[2] = .0f;
            std::vector<uint16_t> vecUi16EmptyB;
            std::vector<float> vecFemptyB;
            Stest sTestB;

     
            if (!SK_COMPARE_INT(ui64A, ui64B)) throw std::runtime_error("");
            pe.getNextData(0, ui64B);
            if (SK_COMPARE_INT(ui64A, ui64B)) throw std::runtime_error("");

            if (!SK_COMPARE_INT(i8A, i8B)) throw std::runtime_error("");
            pe.getNextData(1, i8B);
            if (SK_COMPARE_INT(i8A, i8B)) throw std::runtime_error("");

            if (!SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
            pe.getNextData(2, vecB);
            if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");

            if (!SK_COMPARE_INT(*ptrUint16A, *ptrUint16B)) throw std::runtime_error("");
            pe.getNextData(2, ptrUint16B, 1);
            if (!SK_COMPARE_INT(*ptrUint16A, *ptrUint16B)) throw std::runtime_error("");
            pe.getNextData(4, ptrUint16B, 1);
            if (!SK_COMPARE_INT(*ptrUint16A, *ptrUint16B)) throw std::runtime_error("");
            pe.getNextData(3, ptrUint16B, 1);
            if (SK_COMPARE_INT(*ptrUint16A, *ptrUint16B)) throw std::runtime_error("");

            if (!SK_COMPARE_ARR(arrInt8A, arrInt8B, 3)) throw std::runtime_error("");
            pe.getNextData(4, arrInt8B, 3);
            if (SK_COMPARE_ARR(arrInt8A, arrInt8B, 3)) throw std::runtime_error("");

            if (!SK_COMPARE_VEC(vecBoolA, vecBoolB)) throw std::runtime_error("");
            pe.getNextData(5, vecBoolB);
            if (SK_COMPARE_VEC(vecBoolA, vecBoolB)) throw std::runtime_error("");

            if (!SK_COMPARE_ARRPTR(ptrArrFloatA, ptrArrFloatB, 3)) throw std::runtime_error("");
            pe.getNextData(6, ptrArrFloatB, 3);
            if (SK_COMPARE_ARRPTR(ptrArrFloatA, ptrArrFloatB, 3)) throw std::runtime_error("");

            if (SK_COMPARE_VEC(vecUi16EmptyA, vecUi16EmptyB)) throw std::runtime_error("");
            pe.getNextData(7, vecUi16EmptyB);
            if (SK_COMPARE_VEC(vecUi16EmptyA, vecUi16EmptyB)) throw std::runtime_error("");

            if (SK_COMPARE_VEC(vecFemptyA, vecFemptyB)) throw std::runtime_error("");
            pe.getNextData(8, vecFemptyB);
            if (SK_COMPARE_VEC(vecFemptyA, vecFemptyB)) throw std::runtime_error("");

            if (!SK_COMPARE_INT(sTestA.i, sTestB.i)
                || !SK_COMPARE_FLT(sTestA.f, sTestB.f)
                || !SK_COMPARE_CZ(sTestA.cz, sTestB.cz)
                || !SK_COMPARE_ARR(sTestA.s1.i8Arr, sTestB.s1.i8Arr, 3)
                || !SK_COMPARE_STR(sTestA.s1.str, sTestB.s1.str)
                || !SK_COMPARE_VEC(sTestA.s1.s2.vec, sTestB.s1.s2.vec)) throw std::runtime_error("");
            pe.getNextData(9, sTestB);
            if (SK_COMPARE_INT(sTestA.i, sTestB.i)
                || SK_COMPARE_FLT(sTestA.f, sTestB.f)
                || SK_COMPARE_CZ(sTestA.cz, sTestB.cz)
                || SK_COMPARE_ARR(sTestA.s1.i8Arr, sTestB.s1.i8Arr, 3)
                || SK_COMPARE_STR(sTestA.s1.str, sTestB.s1.str)
                || SK_COMPARE_VEC(sTestA.s1.s2.vec, sTestB.s1.s2.vec)) throw std::runtime_error("");
        }
    }
    
    // T
    {
        Cserializing pe;

        int64_t i64A(INT64_MIN);
        pe.setNextData(250, i64A);

        pe.changeTypeTo_Get();

        int64_t i64B(0);
        if (!SK_COMPARE_INT(i64A, i64B)) throw std::runtime_error("");
        pe.getNextData(250, i64B);
        if (SK_COMPARE_INT(i64A, i64B)) throw std::runtime_error("");
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
        if (!SK_COMPARE_INT(*i64A, *i64B)) throw std::runtime_error("");
        pe.getNextData(250, i64B, 1);
        if (SK_COMPARE_INT(*i64A, *i64B)) throw std::runtime_error("");
        delete i64A;
        delete i64B;
    }
    
    // T [N]
    {
        Cserializing pe;

        int64_t i64A[3];
        i64A[0] = INT64_MIN;
        i64A[1] = 1;
        i64A[2] = INT64_MAX;
        pe.setNextData(250, i64A, 3);

        pe.changeTypeTo_Get();

        int64_t i64B[3];
        i64B[0] = 0;
        i64B[1] = 0;
        i64B[2] = 0;
        if (!SK_COMPARE_ARR(i64A, i64B, 3)) throw std::runtime_error("");
        pe.getNextData(250, i64B, 3);
        if (SK_COMPARE_ARR(i64A, i64B, 3)) throw std::runtime_error("");
    }
    
    // T [64]
    {
        Cserializing pe;

        int64_t i64ArrA[64] { 0 };
        for (int i(0); i < 64; ++i)
            i64ArrA[i] = (::rand() % INT64_MAX) * (::rand() % 2 == 0 ? -1 : 1);
        pe.setNextData(42, i64ArrA, 64);

        pe.changeTypeTo_Get();

        int64_t i64ArrB[64] { 0 };
        if (!SK_COMPARE_ARR(i64ArrA, i64ArrB, 64)) throw std::runtime_error("");
        pe.getNextData(42, i64ArrB, 64);
        if (SK_COMPARE_ARR(i64ArrA, i64ArrB, 64)) throw std::runtime_error("");
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
        if (!SK_COMPARE_ARR(i64A, i64B, 3)) throw std::runtime_error("");
        pe.getNextData(250, i64B, 3);
        if (SK_COMPARE_ARR(i64A, i64B, 3)) throw std::runtime_error("");
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
        if (!SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
        pe.getNextData(0, vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
    }
    
    // STD::VECTOR *
    {
        Cserializing pe;

        std::vector<uint8_t> *vecA(new std::vector<uint8_t>({ 11, 33, 22 }));
        pe.setNextData(0, vecA, 1);

        pe.changeTypeTo_Get();

        std::vector<uint8_t> *vecB(new std::vector<uint8_t>());
        if (!SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
        pe.getNextData(0, vecB, 1);
        if (SK_COMPARE_VEC((*vecA), (*vecB))) throw std::runtime_error("");
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
        if (!SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
        pe.getNextData(250, vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
    }
    
    // STD::VECTOR<STD::VECTOR<STD::PAIR<UINT8, BOOL>>>
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
        if (!SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
        pe.getNextData(37, vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
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
        //pe.setNextData(42, vecA);

        pe.changeTypeTo_Get();

        std::vector<std::vector<std::tuple<uint8_t, float, std::string>>> vecB;
        if (!SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
        pe.getNextData(42, vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
    }*/
    /*
    // STD::VECTOR<STD::VECTOR> [N]
    {
        Cserializing pe;

        std::vector<std::vector<std::string>> vecA = {
            { "8aVj3lSS", "1OXTs", "aytQ6m4" },
            { "eYa60NC", "51jYH0Y", "PstZJ0oy", "ZVhAE", "PcJfpJb" },
            { "YDHdOl" },
            { "8xElm", "XpC8WZR6", "pY63Y1rW", "TrIE3z6", "kWP5", "L2GWQOL", "OZ2F" }
        };
        pe.setNextData(14, vecA);

        pe.changeTypeTo_Get();

        std::vector<std::vector<std::string>> vecB;

        if (!SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
        pe.getNextData(14, vecB);
        if (SK_COMPARE_VEC(vecA, vecB)) throw std::runtime_error("");
    }

    // STD::STRING
    {
        Cserializing pe;

        std::string strA("OZ2FhLMeF");
        pe.setNextData(52, strA);

        pe.changeTypeTo_Get();

        std::string strB;

        if (!SK_COMPARE_STR(strA, strB)) throw std::runtime_error("");
        pe.getNextData(52, strB);
        if (SK_COMPARE_STR(strA, strB)) throw std::runtime_error("");
    }
    
    // STD::STRING *
    {
        Cserializing pe;

        std::string *strA(new std::string("OZ2FhLMeF"));
        pe.setNextData(52, strA, 1);

        pe.changeTypeTo_Get();

        std::string *strB(new std::string());
        if (!SK_COMPARE_STR(*strA, *strB)) throw std::runtime_error("");
        pe.getNextData(52, strB, 1);
        if (SK_COMPARE_STR(*strA, *strB)) throw std::runtime_error("");
        delete strA;
        delete strB;
    }

    // STD::STRING [N]
    {
        Cserializing pe;

        std::string strA[3] = {
            "XprC8R6",
            "pY63W",
            "TrGIE3z6"
        };
        pe.setNextData(42, strA);

        pe.changeTypeTo_Get();

        std::string strB[3];
        for (int i(0); i < 3; ++i)
            if (!SK_COMPARE_STR(strA[i], strB[i])) throw std::runtime_error("");
        pe.getNextData(42, strB);
        for (int i(0); i < 3; ++i)
            if (SK_COMPARE_STR(strA[i], strB[i])) throw std::runtime_error("");
    }

    // STD::STRING * [N]
    {
        Cserializing pe;

        std::string *strA(new std::string[3]);
        strA[0] = "XprC8R6";
        strA[1] = "pY63W";
        strA[2] = "TrGIE3z6";
        pe.setNextData(42, strA, 3);

        pe.changeTypeTo_Get();

        std::string *strB(new std::string[3]);
        for (int i(0); i < 3; ++i)
            if (!SK_COMPARE_STR(strA[i], strB[i])) throw std::runtime_error("");
        pe.getNextData(42, strB, 3);
        for (int i(0); i < 3; ++i)
            if (SK_COMPARE_STR(strA[i], strB[i])) throw std::runtime_error("");
        delete[] strA;
        delete[] strB;
    }
    
    // STD::U32STRING
    {
        Cserializing pe;
        
        std::u32string u32strA(U"\U000030C6\U000030EC\U000030D3\U000030B2\U000030FC\U000030E0");
        pe.setNextData(52, u32strA);

        pe.changeTypeTo_Get();

        std::u32string u32strB;
        if (!SK_COMPARE_STR(u32strA, u32strB)) throw std::runtime_error("");
        pe.getNextData(52, u32strB);
        if (SK_COMPARE_STR(u32strA, u32strB)) throw std::runtime_error("");
    }

    // STD::U32STRING *
    {
        Cserializing pe;

        std::u32string *u32strA(new std::u32string((U"\U00000070\U00000101\U000002BB\U00000061\U0000006E\U00000069\U00000020\U00000077\U00000069\U0000006B\U00000069\U0000014D")));
        pe.setNextData(20, u32strA, 1);

        pe.changeTypeTo_Get();

        std::u32string *u32strB(new std::u32string());
        if (!SK_COMPARE_STR(*u32strA, *u32strB)) throw std::runtime_error("");
        pe.getNextData(20, u32strB, 1);
        if (SK_COMPARE_STR(*u32strA, *u32strB)) throw std::runtime_error("");
    }

    // STD::U32STRING [N]
    {
        Cserializing pe;

        std::u32string u32strA[4] = {
            U"\U000030C6\U000030EC\U000030D3\U000030B2\U000030FC\U000030E0",
            U"\U0000BE44\U0000B514\U0000C624\U00000020\U0000AC8C\U0000C784",
            U"\U00000074\U00000072\U000000F2\U00000020\U00000063\U00000068\U000001A1\U00000069\U00000020\U00000111\U00000069\U00001EC7\U0000006E\U00000020\U00000074\U00001EED",
            U"\U00000070\U00000101\U000002BB\U00000061\U0000006E\U00000069\U00000020\U00000077\U00000069\U0000006B\U00000069\U0000014D"
        };
        pe.setNextData(52, u32strA);

        pe.changeTypeTo_Get();

        std::u32string u32strB[4];
        for (int i(0); i < 4; ++i)
            if (!SK_COMPARE_STR(u32strA[i], u32strB[i])) throw std::runtime_error("");
        pe.getNextData(52, u32strB);
        for (int i(0); i < 4; ++i)
            if (SK_COMPARE_STR(u32strA[i], u32strB[i])) throw std::runtime_error("");
    }

    // STD::U32STRING * [N]
    {
        Cserializing pe;

        std::u32string *u32strA(new std::u32string[4]);
        u32strA[0] = U"\U000030C6\U000030EC\U000030D3\U000030B2\U000030FC\U000030E0";
        u32strA[1] = U"\U0000BE44\U0000B514\U0000C624\U00000020\U0000AC8C\U0000C784";
        u32strA[2] = U"\U00000074\U00000072\U000000F2\U00000020\U00000063\U00000068\U000001A1\U00000069\U00000020\U00000111\U00000069\U00001EC7\U0000006E\U00000020\U00000074\U00001EED";
        u32strA[3] = U"\U00000070\U00000101\U000002BB\U00000061\U0000006E\U00000069\U00000020\U00000077\U00000069\U0000006B\U00000069\U0000014D";
        pe.setNextData(52, u32strA, 4);

        pe.changeTypeTo_Get();

        std::u32string *u32strB(new std::u32string[4]);
        for (int i(0); i < 4; ++i)
            if (!SK_COMPARE_STR(u32strA[i], u32strB[i])) throw std::runtime_error("");
        pe.getNextData(52, u32strB, 4);
        for (int i(0); i < 4; ++i)
            if (SK_COMPARE_STR(u32strA[i], u32strB[i])) throw std::runtime_error("");
        delete[] u32strA;
        delete[] u32strB;
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

        Stest sTestA(INT_MAX, -180.0f, "A0!-");
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
            || SK_COMPARE_VEC(sTestA.s1.s2.vec, sTestB.s1.s2.vec)) throw std::runtime_error("");
    }
    
    /// CUSTOM DATA STRUCTURE *
    {
        Cserializing pe;

        Stest *sTestA(new Stest);
        sTestA->i = INT_MIN;
        sTestA->f = FLT_MAX;
        ::strcpy_s(sTestA->cz, 5, "azAZ");
        sTestA->s1.i8Arr[0] = INT8_MIN;
        sTestA->s1.i8Arr[1] = 0;
        sTestA->s1.i8Arr[2] = INT8_MAX;
        sTestA->s1.str = "T95vZYftfY";
        sTestA->s1.s2.vec = { 127, 0, UINT8_MAX };
        pe.setNextData(250, sTestA, 1);

        pe.changeTypeTo_Get();

        Stest *sTestB(new Stest);
        pe.getNextData(250, sTestB, 1);

        if (SK_COMPARE_INT(sTestA->i, sTestB->i)
            || SK_COMPARE_FLT(sTestA->f, sTestB->f)
            || SK_COMPARE_CZ(sTestA->cz, sTestB->cz)
            || SK_COMPARE_ARR(sTestA->s1.i8Arr, sTestB->s1.i8Arr, 3)
            || SK_COMPARE_STR(sTestA->s1.str, sTestB->s1.str)
            || SK_COMPARE_VEC(sTestA->s1.s2.vec, sTestB->s1.s2.vec)) throw std::runtime_error("");
        delete sTestA;
        delete sTestB;
    }
    
    // CUSTOM DATA STRUCTURE [N]
    {
        Cserializing pe;

        Stest sTestA[3];

        sTestA[0].i = INT_MIN;
        sTestA[0].f = -180.0f;
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
        pe.getNextData(250, sTestB);

        for (int i(0); i < 3; ++i)
            if (SK_COMPARE_INT(sTestA[i].i, sTestB[i].i)
                || SK_COMPARE_FLT(sTestA[i].f, sTestB[i].f)
                || SK_COMPARE_CZ(sTestA[i].cz, sTestB[i].cz)
                || SK_COMPARE_ARR(sTestA[i].s1.i8Arr, sTestB[i].s1.i8Arr, 3)
                || SK_COMPARE_STR(sTestA[i].s1.str, sTestB[i].s1.str)
                || SK_COMPARE_VEC(sTestA[i].s1.s2.vec, sTestB[i].s1.s2.vec)) throw std::runtime_error("");
    }
    
    // CUSTOM DATA STRUCTURE * [N]
    {
        Cserializing pe;

        Stest *sTestA(new Stest[3]);

        sTestA[0].i = INT_MIN;
        sTestA[0].f = -180.0f;
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
        pe.getNextData(250, sTestB, 3);

        for (int i(0); i < 3; ++i)
            if (SK_COMPARE_INT(sTestA[i].i, sTestB[i].i)
                || SK_COMPARE_FLT(sTestA[i].f, sTestB[i].f)
                || SK_COMPARE_CZ(sTestA[i].cz, sTestB[i].cz)
                || SK_COMPARE_ARR(sTestA[i].s1.i8Arr, sTestB[i].s1.i8Arr, 3)
                || SK_COMPARE_STR(sTestA[i].s1.str, sTestB[i].s1.str)
                || SK_COMPARE_VEC(sTestA[i].s1.s2.vec, sTestB[i].s1.s2.vec)) throw std::runtime_error("");
        delete[] sTestA;
        delete[] sTestB;
    }
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
