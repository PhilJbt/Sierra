#pragma once

#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <mutex>
#include <vector>
#include <map>
#include <unordered_map>
#include <numeric>
#include <concepts>




typedef uint8_t  typeId_t;
typedef uint32_t typeSize_t;
typedef uint16_t dataCount_t;
typedef std::unordered_map<std::string, typeSize_t> mapTypesInfos;

/*
** 
*
*/
class Cserializing {
public:
    struct SvarInfo {
    public:
        template <typename T>
        SvarInfo(T &_var) {
            constexpr bool has_varInfos = requires(T & _var) {
                _var.__v;
            };

            if constexpr (has_varInfos) {
                m_ptr = &_var.__v;
                m_bIsVarInfosVec = true;
            }
            else {
                if (isTypeRegistered_byType(T())) {
                    getTypeInfos_byType(_var, &m_ui8TypeId, &m_ui32Size);
                    m_ptr = &_var;
                    m_ui16Count = 1;
                }
                else
                    throw std::runtime_error("Type not yet registered.");
            }
        }

        template <typename T, size_t N>
        SvarInfo(T(&_var)[N]) {
            constexpr bool has_varInfos = requires(T & _var) {
                _var.__v;
            };

            if constexpr (has_varInfos) {
                m_ptr = &_var.__v;
                m_bIsVarInfosVec = true;
            }
            else {
                if (isTypeRegistered_byType(T())) {
                    getTypeInfos_byType(_var[0], &m_ui8TypeId, &m_ui32Size);
                    m_ptr = &_var;
                    m_ui16Count = N;
                }
                else
                    throw std::runtime_error("Type not yet registered.");
            }
        }

        typeId_t     m_ui8TypeId      = 0;
        typeSize_t   m_ui32Size       = 0;
        dataCount_t  m_ui16Count      = 0;
        void        *m_ptr            = nullptr;
        bool         m_bIsVarInfosVec = false;
    };



    enum Eoperation_ {
        Eoperation_NULL,
        Eoperation_Get,
        Eoperation_Set
    };



    template<typename T>
    bool isNextData(const T &_var, typename std::enable_if<!std::is_pointer<T>::value >::type* = 0) {
        typeId_t ui8TypeId(0);
        getTypeInfos_byType(_var, &ui8TypeId, nullptr);

        return _isNextData(false, ui8TypeId);
    }

    template <typename T>
    bool isNextData(const T _var, int _iCount = 1, typename std::enable_if<std::is_pointer<T>::value >::type* = 0) {
        typeId_t ui8TypeId(0);
        getTypeInfos_byType(_var[0], &ui8TypeId, nullptr);

        return _isNextData(false, ui8TypeId) && (static_cast<dataCount_t>(_iCount) == static_cast<dataCount_t>(isNextCount()));
    }

    /*template <typename T, size_t N>
    bool isNextData(T(&_var)[N]) {
        typeId_t ui8TypeId(0);
        getTypeInfos_byType(_var[0], &ui8TypeId, nullptr);

        return _isNextData(false, ui8TypeId) && (static_cast<dataCount_t>(N) == static_cast<dataCount_t>(isNextCount()));
    }*/

    /*template <typename T>
    bool isNextData(const T _str, typename std::enable_if<std::is_same<T, std::string>::value && !std::is_pointer<T>::value>::type* = 0) {
        typeId_t ui8TypeId(0);
        getTypeInfos_byType(_str[0], &ui8TypeId, nullptr);

        return _isNextData(false, ui8TypeId);
    }*/

    template<typename U>
    bool isNextData(std::vector<U> &_vec) {
        return isNextData(&_vec);
    }

    template<typename U>
    bool isNextData(std::vector<U> *_vec) {
        typeId_t ui8VectId(0),
                 ui8TypeId(0);
        getTypeInfos_byName("std::vector", &ui8VectId, nullptr);
        getTypeInfos_byType(U(), &ui8TypeId, nullptr);

        return _isNextData(false, ui8VectId, ui8TypeId);
    }


    int isNextCount() {
        dataCount_t ui16Count(0);
        ::memcpy(&ui16Count, &m_vecBuffer[m_iIndex], sizeof(ui16Count));
        return static_cast<int>(ui16Count);
    }
    

    template <typename T>
    void setNextData(const T _var, typename std::enable_if<std::is_pointer<T>::value >::type* = 0) {
        _setNextData(_var);
    }

    template <typename T>
    void setNextData(const T _var, int const _iCount, typename std::enable_if<std::is_pointer<T>::value >::type* = 0) {
        _setNextData(_var, _iCount);
    }

    template <typename T>
    void setNextData(const T &_var, typename std::enable_if<!std::is_pointer<T>::value >::type* = 0) {
        _setNextData(&_var);
    }


    template<typename T>
    void getNextData(T _var, typename std::enable_if<std::is_pointer<T>::value>::type* = 0) {
        _getNextData_template(_var);
    }

    template<typename T>
    void getNextData(T &_var, typename std::enable_if<!std::is_pointer<T>::value>::type* = 0) {
        _getNextData_template(&_var);
    }


    void changeOperationType(const Eoperation_ &_eTypeOperation) {
        if (m_eOperation == Eoperation_NULL)
            throw std::runtime_error("Eoperation_NULL operation type cannot be set.");
        m_eOperation = _eTypeOperation;
        m_iIndex = 0;
    }


    /*template <typename T, size_t N>
    inline static bool isTypeRegistered(const T(&_t)[N]) {
        std::string strTypeName(std::type_index(typeid(const_cast<T &>(_t[0]))).name());
        return s_mapTypes.find(strTypeName) != s_mapTypes.end();
    }*/

    template<typename T>
    inline static bool isTypeRegistered_byType(const T &_t, typename std::enable_if<!std::is_pointer<T>::value >::type* = 0) {
        std::string strTypeName(std::type_index(typeid(const_cast<T &>(_t))).name());
        return s_mapTypes.find(strTypeName) != s_mapTypes.end();
    }

    template<typename T>
    inline static bool isTypeRegistered_byType(const T _t, typename std::enable_if<std::is_pointer<T>::value >::type* = 0) {
        std::string strTypeName(std::type_index(typeid(const_cast<T &>(_t))).name());
        return s_mapTypes.find(strTypeName) != s_mapTypes.end();
    }

    template<typename T>
    inline static bool isTypeRegistered_byName(const T &_strTypeName, typename std::enable_if<std::is_same<T, std::string>::value && !std::is_pointer<T>::value >::type* = 0) {
        return s_mapTypes.find(_strTypeName) != s_mapTypes.end();
    }


    template<typename T>
    static void getTypeInfos_byType(const T &_t, typeId_t *_out_ui8TypeId, typeSize_t *_out_ui32TypeSize) {
        std::string strTypeName(std::type_index(typeid(const_cast<T &>(_t))).name());
        if (!isTypeRegistered_byName(strTypeName))
            throw std::runtime_error("Type not registered");
        const mapTypesInfos::iterator &&itElem(s_mapTypes.find(strTypeName));
        
        if (_out_ui8TypeId != nullptr)
            *_out_ui8TypeId = static_cast<typeId_t>(std::distance(s_mapTypes.begin(), itElem)) + 1;
        if (_out_ui32TypeSize != nullptr)
            *_out_ui32TypeSize = itElem->second;
    }

    static void getTypeInfos_byName(std::string _strTypeName, typeId_t *_out_ui8TypeId, typeSize_t *_out_ui32TypeSize) {
        if (!isTypeRegistered_byName(_strTypeName))
            throw std::runtime_error("Type not registered");
        const mapTypesInfos::iterator &&itElem(s_mapTypes.find(_strTypeName));

        if (_out_ui8TypeId != nullptr)
            *_out_ui8TypeId = static_cast<typeId_t>(std::distance(s_mapTypes.begin(), itElem)) + 1;
        if (_out_ui32TypeSize != nullptr)
            *_out_ui32TypeSize = itElem->second;
    }


    static void initialization() {
        _registerTypes(
            bool(),
            int8_t(), char(), signed char(),
            int16_t(), short(), short int(), signed short int(),
            int32_t(), int(), signed(), signed int(), long(), long int(), signed long int(),
            int64_t(), long long(), signed long long(), long long(),
            uint8_t(), unsigned char(),
            uint16_t(), wchar_t(), unsigned short(), unsigned short int(), unsigned long(), unsigned long int(),
            uint32_t(), unsigned int(), unsigned(),
            uint64_t(), unsigned long long(),
            float_t(), float(),
            double_t(), double(), long double()
        );
        _registerType_force("std::string", sizeof(std::string("TEST")[0]));
        _registerType_force("std::wstring", sizeof(std::wstring(L"TEST")[0]));
        _registerType_force("std::vector");
        _registerType_force("std::map");
    }

private:
    template <typename... T>
    static void _registerTypes(T&&... args) {
        (_registerType_unpack(args), ...);
    }

    template<typename T>
    static void _registerType_unpack(const T &_t) {
        typeSize_t ui32Size(sizeof(_t));

        s_mapTypes.insert(
            {
                std::type_index(typeid(_t)).name(),
                ui32Size
            }
        );
    }

    static void _registerType_force(std::string _str, const typeSize_t &_ui32Size = 0) {
        s_mapTypes.insert(
            {
                _str,
                _ui32Size
            }
        );
    }


    template<typename T>
    void _getNextData_unfold(T *_var) {
        constexpr bool has_varInfos = requires(T * _var) {
            _var->__v;
        };

        if constexpr (has_varInfos) {
            for (int i(0); i < _var.__v.size(); ++i) {
                if (_var.__v[i].m_bIsVarInfosVec) {
                    std::vector<Cserializing::SvarInfo> &vec = (*reinterpret_cast<std::vector<Cserializing::SvarInfo>*>(_var.__v[i].m_ptr));
                    for (int i(0); i < vec.size(); ++i)
                        _getNextData_varInfo(vec[i]);
                }
                else
                    _getNextData_varInfo(_var.__v[i]);
            }
        }
        else
            _getNextData_var(_var);
    }

    void _getNextData_varInfo(SvarInfo &_ptr) {
        _chkOperation(Eoperation_Get);

        m_iIndex += sizeof(typeId_t);

        dataCount_t ui16Count(_isNextCount());
        uint32_t    ui32DataSize(_ptr.m_ui32Size * ui16Count);

        ::memcpy(_ptr.m_ptr, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template <typename T, size_t N>
    void _getNextData_template(const T(&_var)[N]) {
        _chkOperation(Eoperation_Get);

        dataCount_t ui16Count(_isNextCount());

        typeId_t   ui8TypeId(0);
        typeSize_t ui32TypeSize(0);
        _getTypeInfos_byType(T(), &ui8TypeId, &ui32TypeSize);
        _chkExpectedType(ui8TypeId);

        uint32_t ui32DataSize(ui32TypeSize * ui16Count);

        ::memcpy(_var, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    void _getNextData_template(std::string &_var) {
        _chkOperation(Eoperation_Get);

        dataCount_t ui16Count(_isNextCount());

        typeId_t   ui8TypeId(0);
        typeSize_t ui32TypeSize(0);
        _getTypeInfos_byType(_var[0], &ui8TypeId, &ui32TypeSize);
        _chkExpectedType(ui8TypeId);

        uint32_t ui32DataSize(ui32TypeSize * ui16Count);

        _var = std::string(reinterpret_cast<char*>(&m_vecBuffer[m_iIndex]), ui32DataSize);

        _incrementIndex(ui32DataSize);
    }

    template<typename T>
    void _getNextData_template(T *_var) {
        _chkOperation(Eoperation_Get);

        dataCount_t ui16Count(_isNextCount());

        typeId_t   ui8TypeId(0);
        typeSize_t ui32TypeSize(0);
        _getTypeInfos_byType(T(), &ui8TypeId, &ui32TypeSize);
        _chkExpectedType(ui8TypeId);

        uint32_t ui32DataSize(ui32TypeSize * ui16Count);

        ::memcpy(_var, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename U>
    void _getNextData_template(std::vector<U> *_vec) {
        _getNextData_vec(_vec);
    }

    template<typename U>
    void _getNextData_template(std::vector<U> &_vec) {
        _getNextData_vec(&_vec);
    }

    template<typename U>
    void _getNextData_vec(std::vector<U> *_vec) {
        _chkOperation(Eoperation_Get);

        dataCount_t ui16Count(_isNextCount());

        typeId_t   ui8TypeId(0),
                   ui8VecId(0);
        typeSize_t ui32TypeSize(0);
        _getTypeInfos_byName("std::vector", &ui8VecId, nullptr);
        _getTypeInfos_byType(U(), &ui8TypeId, &ui32TypeSize);
        _chkExpectedType(ui8VecId, ui8TypeId);

        uint32_t ui32DataSize(ui32TypeSize * ui16Count);
        _vec->resize(ui16Count);

        ::memcpy(&(*_vec)[0], &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename U, typename V>
    void _getNextData_template(std::map<U, V> *_map) {
        _getNextData_map(_map);
    }

    template<typename U, typename V>
    void _getNextData_template(std::map<U, V> &_map) {
        _getNextData_map(&_map);
    }

    template<typename U, typename V>
    void _getNextData_map(std::map<U, V> *_map) {
        _chkOperation(Eoperation_Get);

        dataCount_t ui16Count(_isNextCount());

        typeId_t ui8TypeId_map(0),
                 ui8TypeId_var1(0),
                 ui8TypeId_var2(0);
        _getTypeInfos_byName("std::map", &ui8TypeId_map, nullptr);
        _getTypeInfos_byType(U(), &ui8TypeId_var1, nullptr);
        _getTypeInfos_byType(V(), &ui8TypeId_var2, nullptr);
        _chkExpectedType(ui8TypeId_map, ui8TypeId_var1, ui8TypeId_var2);

        typeSize_t ui32SizeType1(sizeof(U)),
                   ui32SizeType2(sizeof(V));

        for (uint16_t i(0); i < ui16Count; ++i) {
            U key;
            ::memcpy(&key, &m_vecBuffer[m_iIndex], ui32SizeType1);
            _incrementIndex(ui32SizeType1);

            V val;
            ::memcpy(&val, &m_vecBuffer[m_iIndex], ui32SizeType2);
            _incrementIndex(ui32SizeType2);

            _map->insert({ key, val });
        }
    }


    /*template<typename T>
    void _setNextData(const T *_var, int _iCount) {
        static_assert(std::is_pointer<T *>::value, "Pointers only.");

        constexpr bool has_varInfos = requires(T * _var) {
            _var->__v;
        };

        if constexpr (has_varInfos) {
            for (int i(0); i < _var->__v.size(); ++i)
                _setNextData_structvar(_var->__v[i]);
        }
        else
            _setNextData_template(_var, _iCount);
    }*/

    template<typename T>
    void _setNextData(const T *_var) {
        static_assert(std::is_pointer<T *>::value, "Pointers only.");

        constexpr bool has_varInfos = requires(T * _var) {
            _var->__v;
        };

        if constexpr (has_varInfos) {
            for (int i(0); i < _var->__v.size(); ++i)
                _setNextData_structvar(_var->__v[i]);
        }
        else
            _setNextData_template(_var);
    }

    template <typename T>
    void _setNextData_var(const T *_var, const int &_iCount) {
        _chkOperation(Eoperation_Set);

        typeId_t   ui8TypeId(0);
        typeSize_t ui32TypeSize(0);
        _getTypeInfos_byType(T(), &ui8TypeId, &ui32TypeSize);

        int iCount(_iCount);
        _setHeader(iCount, { ui8TypeId });

        uint32_t ui32DataSize(ui32TypeSize * iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], _var, ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template <typename T>
    void _setNextData_template(const T *_var) {
        _setNextData_var(_var, 1);
    }

    template <typename T>
    void _setNextData_template(const T *_var, const int &_iCount) {
        _setNextData_var(_var, _iCount);
    }

    /*template <typename T, size_t N>
    void _setNextData_template(const T(&_var)[N]) {
        _chkOperation(Eoperation_Set);

        typeId_t   ui8TypeId(0);
        typeSize_t ui32TypeSize(0);
        _getTypeInfos_byType(T(), &ui8TypeId, &ui32TypeSize);

        int iCount(N);
        _setHeader(iCount, { ui8TypeId });

        uint32_t ui32DataSize(ui32TypeSize * iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], _var, ui32DataSize);
        _incrementIndex(ui32DataSize);
    }*/

    template<typename U>
    void _setNextData_template(const std::vector<U> *_vec) {
        static_assert(!std::is_pointer<std::vector<U>>::value, "Pointer only.");

        _chkOperation(Eoperation_Set);

        typeId_t   ui8TypeId_vec(0),
            ui8TypeId_var(0);
        typeSize_t ui32TypeSize_var(0);
        _getTypeInfos_byName("std::vector", &ui8TypeId_vec, nullptr);
        _getTypeInfos_byType(U(), &ui8TypeId_var, &ui32TypeSize_var);

        int iCount(static_cast<int>(_vec->size()));
        _setHeader(iCount, { ui8TypeId_vec, ui8TypeId_var });

        uint32_t ui32DataSize(ui32TypeSize_var * iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &(*_vec)[0], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    /*template<typename U, typename V>
    void _setNextData_template(const std::map<U, V> *_map) {
        static_assert(!std::is_pointer<std::map<U, V>>::value, "Pointer are forbidden, use the dereference operator.");

        _chkOperation(Eoperation_Set);

        typeId_t   ui8TypeId_map(0),
                   ui8TypeId_var1(0),
                   ui8TypeId_var2(0);
        typeSize_t ui32Count(0);
        _getTypeInfos_byName("std::map", &ui8TypeId_map, &ui32Count);
        _getTypeInfos_byType(U(), &ui8TypeId_var1, &ui32Count);
        _getTypeInfos_byType(V(), &ui8TypeId_var2, &ui32Count);

        typeSize_t ui32SizeType1(sizeof(U)),
                   ui32SizeType2(sizeof(V));

        int iCount(static_cast<int>(_map->size()));
        _setHeader(iCount, { ui8TypeId_map, ui8TypeId_var1, ui8TypeId_var2 });

        _reserveBufferSize((ui32SizeType1 + ui32SizeType2) * iCount);

        for (const auto &[key, val] : *_map) {
            ::memcpy(&m_vecBuffer[m_iIndex], &key, ui32SizeType1);
            _incrementIndex(ui32SizeType1);
            ::memcpy(&m_vecBuffer[m_iIndex], &val, ui32SizeType2);
            _incrementIndex(ui32SizeType2);
        }
    }*/

    /*template <typename T>
    void _setNextData_template(const std::string *_var) {
        _chkOperation(Eoperation_Set);

        std::string str(_var->c_str() + '\0');

        typeId_t   ui8TypeId(0);
        typeSize_t ui32CharSize(0);
        _getTypeInfos_byType(str[0], &ui8TypeId, &ui32CharSize);

        typeSize_t ui32TypeSize(static_cast<int>(str.length()) * ui32CharSize);

        _setHeader(ui32TypeSize - 1, { ui8TypeId });

        _reserveBufferSize(ui32TypeSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &str[0], ui32TypeSize);
        _incrementIndex(ui32TypeSize);
    }*/

    /*void _setNextData_structvar(const SvarInfo &_ptr) {
        if (_ptr.m_bIsVarInfosVec) {
            std::vector<Cserializing::SvarInfo> &vec = (*reinterpret_cast<std::vector<Cserializing::SvarInfo>*>(_ptr.m_ptr));
            for (int i(0); i < vec.size(); ++i)
                _setNextData_structvar(vec[i]);
        }
        else
            _setNextData_structvar_var(_ptr);
    }*/

    /*void _setNextData_structvar_var(const SvarInfo &_ptr) {
        _chkOperation(Eoperation_Set);

        int iArrOrVarSize(_ptr.m_ui32Size * _ptr.m_ui16Count);
        _reserveBufferSize(sizeof(_ptr.m_ui8TypeId) + sizeof(_ptr.m_ui16Count) + iArrOrVarSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &_ptr.m_ui8TypeId, sizeof(_ptr.m_ui8TypeId));
        m_iIndex += sizeof(_ptr.m_ui8TypeId);

        ::memcpy(&m_vecBuffer[m_iIndex], &_ptr.m_ui16Count, sizeof(_ptr.m_ui16Count));
        m_iIndex += sizeof(_ptr.m_ui16Count);

        ::memcpy(&m_vecBuffer[m_iIndex], _ptr.m_ptr, iArrOrVarSize);
        m_iIndex += iArrOrVarSize;
    }*/


    void _getTypeInfos_byName(const std::string _strTypeName, typeId_t *_out_ui8TypeId, typeSize_t *_out_ui32TypeSize) {
        getTypeInfos_byName(_strTypeName, _out_ui8TypeId, _out_ui32TypeSize);
    }

    template<typename T>
    void _getTypeInfos_byType(const T &_t, typeId_t *_out_ui8TypeId, typeSize_t *_out_ui32TypeSize) {
        getTypeInfos_byType(_t, _out_ui8TypeId, _out_ui32TypeSize);
    }


    void _chkOperation(const Eoperation_ &_eTypeOperation) {
        if (m_eOperation == _eTypeOperation)
            return;
        else if (m_eOperation == Eoperation_NULL) {
            m_eOperation = _eTypeOperation;
            m_iIndex = 0;
        }
        else
            throw std::runtime_error("The current operation type does not match the function type.");
    }

    void _setHeader(const int &_iLength, std::vector<typeId_t> _vecTypeIds) {
        int iValueMaxNbr(static_cast<int>(::floor(static_cast<float>(_iLength) / static_cast<float>(std::numeric_limits<dataCount_t>::max()))));

        _vecTypeIds.push_back(0);

        _reserveBufferSize(((static_cast<int>(_vecTypeIds.size()) * sizeof(typeId_t))) + ((iValueMaxNbr + 2) * sizeof(dataCount_t)));

        for (int i(0); i < iValueMaxNbr + 2; ++i) {
            switch (iValueMaxNbr - i) {
            case 0:
                ::memcpy(&m_vecBuffer[m_iIndex], reinterpret_cast<const dataCount_t *>(&_iLength), sizeof(dataCount_t));
                break;

            case -1:
                {
                    dataCount_t ui16CountValNull(0);
                    ::memcpy(&m_vecBuffer[m_iIndex], &ui16CountValNull, sizeof(dataCount_t));
                }
                break;

            default:
                {
                    dataCount_t ui16CountValMax(std::numeric_limits<dataCount_t>::max());
                    ::memcpy(&m_vecBuffer[m_iIndex], &ui16CountValMax, sizeof(dataCount_t));
                }
                break;
            }

            m_iIndex += sizeof(dataCount_t);
        }

        for (int i(0); i < _vecTypeIds.size(); ++i) {
            ::memcpy(&m_vecBuffer[m_iIndex], &_vecTypeIds[i], sizeof(typeId_t));
            m_iIndex += sizeof(typeId_t);
        }
    }

    int _isNextCount() {
        int         iDataCount(0);
        dataCount_t ui16DataCount(0);

        do {
            ::memcpy(&ui16DataCount, &m_vecBuffer[m_iIndex], sizeof(dataCount_t));
            m_iIndex += sizeof(dataCount_t);
            iDataCount += static_cast<int>(ui16DataCount);
        } while (ui16DataCount != 0);

        return iDataCount;
    }

    int _isNextData_seekSeq() {
        int iIndex(m_iIndex);
        dataCount_t ui16DataCount(0);
        do {
            ::memcpy(&ui16DataCount, &m_vecBuffer[iIndex], sizeof(dataCount_t));
            iIndex += sizeof(dataCount_t);
        } while (ui16DataCount != 0);

        return iIndex;
    }

    template <typename... T>
    bool _isNextData(bool _bWalking, T&&... args) {
        std::vector<typeId_t> vec = { args... , 0 };
        int iIndex((_bWalking ? m_iIndex : _isNextData_seekSeq()));

        for (int i(0); i < vec.size(); ++i) {
            if (vec[i] != static_cast<typeId_t>(m_vecBuffer[iIndex]))
                return false;
            iIndex += sizeof(typeId_t);
        }

        if (_bWalking)
            m_iIndex = iIndex;

        return true;
    }

    template <typename... T>
    inline void _chkExpectedType(T&&... args) {
        if (!_isNextData(true, args...))
            throw std::runtime_error("The next stored type does not match with the output requested type.");
    }

    inline void _reserveBufferSize(const int &_iSize) {
        m_vecBuffer.resize(m_vecBuffer.size() + _iSize);
    }


    inline void _incrementIndex(const int &_iTypeSize) {
        m_iIndex += _iTypeSize;
    }



    int                  m_iIndex = 0;
    Eoperation_          m_eOperation = Eoperation_NULL;
    std::vector<uint8_t> m_vecBuffer;
    std::mutex           m_mtx;

    static mapTypesInfos     s_mapTypes;
};



#define TEST2(VAR)  Cserializing::SvarInfo<decltype(VAR)>(VAR)
#define TEST(...)   friend class Cserializing; protected: std::vector<Cserializing::SvarInfo> __v { ##__VA_ARGS__ }