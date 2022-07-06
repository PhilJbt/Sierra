#pragma once

#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <mutex>
#include <vector>
#include <map>
#include <unordered_map>
#include <numeric>



struct StypesInfos {
public:
    StypesInfos() {}
    StypesInfos(std::vector<uint32_t> _vecUi32Size, uint8_t  _ui8Id) : m_vecUi32Size(_vecUi32Size), m_ui8Id(_ui8Id) {}
    std::vector<uint32_t> m_vecUi32Size;
    uint8_t               m_ui8Id = 0;
};

typedef std::unordered_map<std::type_index, StypesInfos> mapTypesInfos;

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
                if (isTypeRegistered(_var)) {
                    getTypeInfos(_var, &m_ui8TypeId, &m_ui32Size, {});
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
                if (isTypeRegistered(_var)) {
                    getTypeInfos(_var[0], &m_ui8TypeId, &m_ui32Size, {});
                    m_ptr = &_var;
                    m_ui16Count = N;
                }
                else
                    throw std::runtime_error("Type not yet registered.");
            }
        }

        uint8_t   m_ui8TypeId      = 0;
        uint32_t  m_ui32Size       = 0;
        uint16_t  m_ui16Count      = 0;
        void     *m_ptr            = nullptr;
        bool      m_bIsVarInfosVec = false;
    };



    enum Eoperation_ {
        Eoperation_NULL,
        Eoperation_Get,
        Eoperation_Set
    };


    template <typename... T>
    static void registerTypes(T&&... args) {
        (registerType(args), ...);
    }

    template<typename T>
    bool isNextData(T &_var) {
        std::cerr << "isNextData(T &_var)" << std::endl;
        static_assert(!std::is_pointer<T &>::value, "Pointers are forbidden, use bool isNextData(T *_var, int _iCount).");
        uint8_t ui8TypeId(0);
        getTypeInfos(_var, &ui8TypeId, nullptr, {});

        if (ui8TypeId == m_vecBuffer[m_iIndex])
            return true;
        else
            return false;
    }

    template <typename T>
    bool isNextData(T *_var, int _iCount) {
        std::cerr << "isNextData(T *_var" << std::endl;
        uint8_t ui8TypeId(0);
        getTypeInfos(_var[0], &ui8TypeId, nullptr, {});

        uint16_t ui16Count(0);
        ::memcpy(&ui16Count, &m_vecBuffer[m_iIndex + 1], sizeof(ui16Count));

        if ((ui8TypeId == m_vecBuffer[m_iIndex]) && (static_cast<uint8_t>(_iCount) == ui16Count))
            return true;
        else
            return false;
    }

    template <typename T, size_t N>
    bool isNextData(T(&_var)[N]) {
        std::cerr << "isNextData(T(&_var)[N]" << std::endl;
        uint8_t ui8TypeId(0);
        getTypeInfos(_var[0], &ui8TypeId, nullptr, {});

        uint16_t ui16Count(0);
        ::memcpy(&ui16Count, &m_vecBuffer[m_iIndex + 1], sizeof(ui16Count));

        if ((ui8TypeId == m_vecBuffer[m_iIndex]) && (N == ui16Count))
            return true;
        else
            return false;
    }

    bool isNextData(std::string &_str) {
        std::cerr << "isNextData(std::string &_str" << std::endl;
        uint8_t ui8TypeId(0);
        getTypeInfos(_str, &ui8TypeId, nullptr, {});

        if (ui8TypeId == m_vecBuffer[m_iIndex])
            return true;
        else
            return false;
    }

    int isNextCount() {
        uint16_t ui16Count(0);
        ::memcpy(&ui16Count, &m_vecBuffer[m_iIndex + 1], sizeof(ui16Count));
        return static_cast<int>(ui16Count);
    }


    void setNextData(std::string &_var) {
        std::cerr << "setNextData(std::string &_var)" << std::endl;
        _chkOperation(Eoperation_Set);

        std::string str(_var + '\0');

        uint8_t ui8TypeId(0);
        uint32_t ui32CharSize(0);
        _getTypeInfos(str, &ui8TypeId, &ui32CharSize, {}, true);

        uint32_t ui32TypeSize(static_cast<int>(str.length()) * ui32CharSize);

        _setHeader(ui8TypeId, ui32TypeSize - 1);

        _reserveBufferSize(ui32TypeSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &str[0], ui32TypeSize);
        _incrementIndex(ui32TypeSize);
    }

    template<typename T>
    void setNextData(T *_var, int _iCount) {
        std::cerr << "setNextData(T *_var, int _iCount)" << std::endl;
        static_assert(std::is_pointer<T*>::value, "Pointers only.");

        constexpr bool has_varInfos = requires(T & _var) {
            _var.__v;
        };

        if constexpr (has_varInfos) {
            for (int i(0); i < _var.__v.size(); ++i)
                _setNextData_structvar(_var.__v[i]);
        }
        else
            _setNextData_template(const_cast<T*>(_var), _iCount);
    }

    template<typename T>
    void setNextData(T &_var) {
        std::cerr << "setNextData(T &_var)" << std::endl;
        static_assert(!std::is_pointer<T&>::value, "Pointers are forbidden, use void setNextData(T *_var, int _iCount).");

        constexpr bool has_varInfos = requires(T & _var) {
            _var.__v;
        };

        if constexpr (has_varInfos) {
            for (int i(0); i < _var.__v.size(); ++i)
                _setNextData_structvar(_var.__v[i]);
        }
        else
            _setNextData_template(_var);
    }

    template<typename U>
    void setNextData(std::vector<U> &_vec) {
        std::cerr << "setNextData(std::vector<U> &_vec)" << std::endl;
        static_assert(!std::is_pointer<std::vector<U>>::value, "Pointer are forbidden, use the dereference operator.");

        _chkOperation(Eoperation_Set);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(_vec, &ui8TypeId, &ui32TypeSize, {}, true);

        int iCount(static_cast<int>(_vec.size()));
        _setHeader(ui8TypeId, iCount);

        uint32_t ui32DataSize(ui32TypeSize * iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &_vec[0], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename U>
    void setNextData(std::vector<U> *_vec) {
        std::cerr << "setNextData(std::vector<U> *_vec)" << std::endl;
        static_assert(!std::is_pointer<std::vector<U>>::value, "Pointer only.");

        _chkOperation(Eoperation_Set);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(*_vec, &ui8TypeId, &ui32TypeSize, {}, true);

        int iCount(static_cast<int>(_vec->size()));
        _setHeader(ui8TypeId, iCount);

        uint32_t ui32DataSize(ui32TypeSize * iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &(*_vec)[0], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename U, typename V>
    void setNextData(std::map<U, V> *_map) {
        std::cerr << "setNextData(std::map<U, V> *_map)" << std::endl;
        static_assert(!std::is_pointer<std::map<U, V>>::value, "Pointer are forbidden, use the dereference operator.");

        _chkOperation(Eoperation_Set);

        uint8_t ui8TypeId(0);
        uint32_t ui32Size(0);
        std::vector<uint32_t> *vecTypeSizes(nullptr);
        _getTypeInfos(*_map, &ui8TypeId, &ui32Size, { &vecTypeSizes }, true);

        int iCount(static_cast<int>(_map->size()));
        _setHeader(ui8TypeId, iCount);

        _reserveBufferSize(((*vecTypeSizes)[0] + (*vecTypeSizes)[1]) * iCount);

        for (const auto &[key, val] : *_map) {
            ::memcpy(&m_vecBuffer[m_iIndex], &key, (*vecTypeSizes)[0]);
            _incrementIndex((*vecTypeSizes)[0]);
            ::memcpy(&m_vecBuffer[m_iIndex], &val, (*vecTypeSizes)[1]);
            _incrementIndex((*vecTypeSizes)[1]);
        }
    }

    template<typename U, typename V>
    void setNextData(std::map<U, V> &_map) {
        std::cerr << "setNextData(std::map<U, V> &_map)" << std::endl;
        static_assert(!std::is_pointer<std::map<U, V>>::value, "Pointer are forbidden, use the dereference operator.");

        _chkOperation(Eoperation_Set);

        uint8_t ui8TypeId(0);
        uint32_t ui32Size(0);
        std::vector<uint32_t> *vecTypeSizes(nullptr);
        _getTypeInfos(_map, &ui8TypeId, &ui32Size, { &vecTypeSizes }, true);

        int iCount(static_cast<int>(_map.size()));
        _setHeader(ui8TypeId, iCount);

        _reserveBufferSize(((*vecTypeSizes)[0] + (*vecTypeSizes)[1]) * iCount);

        for (const auto &[key, val] : _map) {
            ::memcpy(&m_vecBuffer[m_iIndex], &key, (*vecTypeSizes)[0]);
            _incrementIndex((*vecTypeSizes)[0]);
            ::memcpy(&m_vecBuffer[m_iIndex], &val, (*vecTypeSizes)[1]);
            _incrementIndex((*vecTypeSizes)[1]);
        }
    }

    template<typename T>
    void getNextData(T *_var) {
        std::cerr << "getNextData(T *_var" << std::endl;
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
            _getNextData(_var);
    }
    
    template<typename T>
    void getNextData(T &_var) {
        std::cerr << "getNextData(T &_var" << std::endl;
        constexpr bool has_varInfos = requires(T & _var) {
            _var.__v;
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
            _getNextData(_var);
    }


    template<typename T>
    static void registerType(const T &_t) {
        s_mapTypes.insert({ std::type_index(typeid(const_cast<T &>(_t))), StypesInfos({ static_cast<uint32_t>(sizeof(_t)) }, ++s_ui8LastId) });
    }

    template<typename U>
    static void registerType(const std::vector<U> &_t) {
        s_mapTypes.insert({ std::type_index(typeid(const_cast<std::vector<U> &>(_t))), StypesInfos({ static_cast<uint32_t>(sizeof(U)) }, ++s_ui8LastId) });
    }

    template<typename U, typename V>
    static void registerType(const std::map<U, V> &_t) {
        s_mapTypes.insert({ std::type_index(typeid(const_cast<std::map<U, V> &>(_t))), StypesInfos({ static_cast<uint32_t>(sizeof(U)), static_cast<uint32_t>(sizeof(V)) }, ++s_ui8LastId) });
    }


    void changeOperationType(const Eoperation_ &_eTypeOperation) {
        if (m_eOperation == Eoperation_NULL)
            throw std::runtime_error("Eoperation_NULL operation type cannot be set.");
        m_eOperation = _eTypeOperation;
        m_iIndex = 0;
    }

    template <typename T, size_t N>
    inline static bool isTypeRegistered(const T(&_t)[N]) {
        std::type_index tiCur(std::type_index(typeid(const_cast<T &>(_t[0]))));
        return s_mapTypes.count(tiCur) == 1;
    }

    template<typename T>
    inline static bool isTypeRegistered(const T &_t) {
        std::type_index tiCur(std::type_index(typeid(const_cast<T &>(_t))));
        return s_mapTypes.count(tiCur) == 1;
    }

    inline static bool isTypeRegistered(std::type_index _tiCur) {
        return s_mapTypes.count(_tiCur) == 1;
    }

    template<typename T>
    static void getTypeInfos(const T &_t, uint8_t *_out_ui8TypeId, uint32_t *_out_ui32TypeFullSize, std::vector<std::vector<uint32_t>**> _vecTypesSizes) {
        std::type_index tiCur(std::type_index(typeid(const_cast<T&>(_t))));
        if (!isTypeRegistered(tiCur))
            throw std::runtime_error("Type not registered yet");
        const mapTypesInfos::iterator &&itElem(s_mapTypes.find(tiCur));

        if (_out_ui8TypeId != nullptr)
            *_out_ui8TypeId = itElem->second.m_ui8Id;
        if (_out_ui32TypeFullSize != nullptr)
            *_out_ui32TypeFullSize = std::accumulate(itElem->second.m_vecUi32Size.begin(), itElem->second.m_vecUi32Size.end(), 0);
        if (_vecTypesSizes.size() == 1)
            *_vecTypesSizes[0] = &itElem->second.m_vecUi32Size;
    }


    static void initialization() {
        /*
        ***************************
        *  ID          TYPE    BYTE
        ***************************
        *   0          bool       1
        *   1           int       1
        *   2           int       2
        *   3           int       4
        *   4           int       8
        *   5           int      16
        *   6          uint       1
        *   7          uint       2
        *   8          uint       4
        *   9          uint       8
        *  10          uint      16
        *  11          char       1
        *  12          char       2
        *  13          char       4
        *  14         uchar       1
        *  15         uchar       2
        *  16         wchar       2
        *  17         wchar       4
        *  18         short       2
        *  19         short       4
        *  20         short       8
        *  21        ushort       2
        *  22        ushort       4
        *  23        ushort       8
        *  24         float       4
        *  25         float       8
        *  26         float      16
        *  27        double       8
        *  28        double      16
        *  29   std::string      --
        * 
        ** _registerType({ {iBytesSize, iIndex}[, {iBytesSize, iIndex}] }, type());
        **/

        // bool
        _registerType({ {1, 0} }, bool());
        // int
        _registerType({ {1, 1}, {2, 2}, {4, 3}, {8, 4}, {16, 5} }, int8_t(), int16_t(), int32_t(), int64_t(), signed(), int(), signed int(), long(), long int(), signed long(), signed long int(), long long int(), signed long long int(), long long(), unsigned long long(), long long(), signed long long(), unsigned long long());
        // uint
        _registerType({ {1, 6}, {2, 7}, {4, 8}, {8, 9}, {16, 10} }, uint8_t(), uint16_t(), uint32_t(), uint64_t(), unsigned(), unsigned int(), unsigned long(), unsigned long int(), unsigned long long int());
        // char
        _registerType({ {1, 11}, {2, 12}, {4, 13} }, char(), char8_t(), signed char(), char16_t(), char32_t());
        // uchar
        _registerType({ {1, 14}, {2, 15} }, unsigned char());
        // wchar
        _registerType({ {2, 16} , {4, 17} }, wchar_t());
        // short
        _registerType({ {2, 18}, {4, 19}, {8, 20} }, short(), short int(), signed short(), signed short int());
        // ushort
        _registerType({ {2, 21}, {4, 22}, {8, 23} }, unsigned short int());
        // float
        _registerType({ {4, 24}, {8, 25}, {16, 26} }, float(), float_t());
        // double
        _registerType({ {8, 27}, {16, 28} }, double(), double_t(), long double());
        // std::string
        _registerType_force(29, sizeof(std::string("TEST")[0]), std::string());
    }

private:
    template <typename... T>
    static void _registerType(const std::map<uint32_t, uint8_t> &_mapSizesBindedIndex, T&&... args) {
        (_registerType_unpack(_mapSizesBindedIndex, args), ...);
    }

    template<typename T>
    static void _registerType_force(const uint8_t &_ui8Index, const uint32_t &_ui32Size, const T &_t) {
        if (s_ui8LastId < _ui8Index)
            s_ui8LastId = _ui8Index;

        s_mapTypes.insert(
            {
                std::type_index(typeid(const_cast<T &>(_t))),
                StypesInfos(
                    {
                        _ui32Size
                    },
                    _ui8Index
                )
            }
        );
    }

    template<typename T>
    static void _registerType_unpack(const std::map<uint32_t, uint8_t> &_mapSizesBindedIndex, const T &_t) {
        uint32_t ui32Size(sizeof(_t));

        if (_mapSizesBindedIndex.find(ui32Size) == _mapSizesBindedIndex.end())
            throw std::runtime_error("This type size has less or more bytes than expected.");

        uint8_t iId(_mapSizesBindedIndex.at(ui32Size));

        if (s_ui8LastId < iId)
            s_ui8LastId = iId;

        s_mapTypes.insert(
            {
                std::type_index(typeid(const_cast<T &>(_t))),
                StypesInfos(
                    {
                        ui32Size
                    },
                    iId
                )
            }
        );
    }
    

    void _getNextData_varInfo(SvarInfo &_ptr) {
        std::cerr << "_getNextData_varInfo(SvarInfo &_ptr" << std::endl;
        _chkOperation(Eoperation_Get);

        m_iIndex += sizeof(uint8_t);

        uint16_t ui16Count(_getElemCount());
        uint32_t ui32DataSize(_ptr.m_ui32Size * ui16Count);

        ::memcpy(_ptr.m_ptr, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template <typename T, size_t N>
    void _getNextData(const T(&_var)[N]) {
        std::cerr << "_getNextData(const T(&_var)[N]" << std::endl;
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(_var[0], &ui8TypeId, &ui32TypeSize, {}, false);

        uint16_t ui16Count(_getElemCount());
        uint32_t ui32DataSize(ui32TypeSize * ui16Count);

        ::memcpy(_var, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    void _getNextData(std::string &_var) {
        std::cerr << "_getNextData(std::string &_var" << std::endl;
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(_var, &ui8TypeId, &ui32TypeSize, {}, false);

        uint16_t ui16Count(_getElemCount());
        uint32_t ui32DataSize(ui32TypeSize * ui16Count);

        _var = std::string(reinterpret_cast<char*>(&m_vecBuffer[m_iIndex]), ui32DataSize);

        _incrementIndex(ui32DataSize + 1);
    }

    template<typename T>
    void _getNextData(T *_var) {
        std::cerr << "_getNextData(T *_var" << std::endl;
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(*_var, &ui8TypeId, &ui32TypeSize, {}, false);

        uint16_t ui16Count(_getElemCount());
        uint32_t ui32DataSize(ui32TypeSize * ui16Count);

        ::memcpy(_var, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename T>
    void _getNextData(T &_var) {
        std::cerr << "_getNextData(T &_var" << std::endl;
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(_var, &ui8TypeId, &ui32TypeSize, {}, false);

        uint16_t ui16Count(_getElemCount());
        uint32_t ui32DataSize(ui32TypeSize * ui16Count);

        ::memcpy(&_var, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename U>
    void _getNextData(std::vector<U> *_vec) {
        std::cerr << "_getNextData(std::vector<U> &_vec" << std::endl;
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(*_vec, &ui8TypeId, &ui32TypeSize, {}, false);

        uint16_t ui16Count(_getElemCount());
        uint16_t ui16Size(ui32TypeSize * ui16Count);
        _vec->resize(ui16Count);

        ::memcpy(&(*_vec)[0], &m_vecBuffer[m_iIndex], ui16Size);
        _incrementIndex(ui16Size);
    }

    template<typename U>
    void _getNextData(std::vector<U> &_vec) {
        std::cerr << "_getNextData(std::vector<U> &_vec" << std::endl;
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(_vec, &ui8TypeId, &ui32TypeSize, {}, false);

        uint16_t ui16Count(_getElemCount());
        uint16_t ui16Size(ui32TypeSize * ui16Count);
        _vec.resize(ui16Count);

        ::memcpy(&_vec[0], &m_vecBuffer[m_iIndex], ui16Size);
        _incrementIndex(ui16Size);
    }

    template<typename U, typename V>
    void _getNextData(std::map<U, V> *_map) {
        std::cerr << "_getNextData(std::map<U, V> *_map" << std::endl;
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        std::vector<uint32_t> *vecTypesSizes(nullptr);
        _getTypeInfos(*_map, &ui8TypeId, nullptr, { &vecTypesSizes }, false);

        uint16_t ui16Count(_getElemCount());

        for (uint16_t i(0); i < ui16Count; ++i) {
            U key;
            ::memcpy(&key, &m_vecBuffer[m_iIndex], (*vecTypesSizes)[0]);
            _incrementIndex((*vecTypesSizes)[0]);

            V val;
            ::memcpy(&val, &m_vecBuffer[m_iIndex], (*vecTypesSizes)[1]);
            _incrementIndex((*vecTypesSizes)[1]);

            _map->insert({ key, val });
        }
    }

    template<typename U, typename V>
    void _getNextData(std::map<U, V> &_map) {
        std::cerr << "_getNextData(std::map<U, V> &_map" << std::endl;
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        std::vector<uint32_t> *vecTypesSizes(nullptr);
        _getTypeInfos(_map, &ui8TypeId, nullptr, { &vecTypesSizes }, false);

        uint16_t ui16Count(_getElemCount());

        for (uint16_t i(0); i < ui16Count; ++i) {
            U key;
            ::memcpy(&key, &m_vecBuffer[m_iIndex], (*vecTypesSizes)[0]);
            _incrementIndex((*vecTypesSizes)[0]);

            V val;
            ::memcpy(&val, &m_vecBuffer[m_iIndex], (*vecTypesSizes)[1]);
            _incrementIndex((*vecTypesSizes)[1]);

            _map.insert({ key, val });
        }
    }


    template <typename T>
    void _setNextData_template(const T* _var, int _iCount) {
        std::cerr << "_setNextData_template(const T* _var, int _iCount" << std::endl;
        _chkOperation(Eoperation_Set);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(*_var, &ui8TypeId, &ui32TypeSize, {}, true);

        int iCount(_iCount);
        _setHeader(ui8TypeId, iCount);

        uint32_t ui32DataSize(ui32TypeSize * iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], _var, ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template <typename T, size_t N>
    void _setNextData_template(const T(&_var)[N]) {
        std::cerr << "_setNextData_template(const T(&_var)[N]" << std::endl;
        _chkOperation(Eoperation_Set);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(_var[0], &ui8TypeId, &ui32TypeSize, {}, true);

        int iCount(N);
        _setHeader(ui8TypeId, iCount);

        uint32_t ui32DataSize(ui32TypeSize * iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], _var, ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename T>
    void _setNextData_template(const T &_var) {
        std::cerr << "_setNextData_template(const T &_var" << std::endl;
        _chkOperation(Eoperation_Set);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(_var, &ui8TypeId, &ui32TypeSize, {}, true);

        _setHeader(ui8TypeId, 1);

        _reserveBufferSize(ui32TypeSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &_var, ui32TypeSize);
        _incrementIndex(ui32TypeSize);
    }

    void _setNextData_structvar(SvarInfo &_ptr) {
        std::cerr << "_setNextData_structvar(SvarInfo &_ptr" << std::endl;
        if (_ptr.m_bIsVarInfosVec) {
            std::vector<Cserializing::SvarInfo> &vec = (*reinterpret_cast<std::vector<Cserializing::SvarInfo>*>(_ptr.m_ptr));
            for (int i(0); i < vec.size(); ++i)
                _setNextData_structvar(vec[i]);
        }
        else
            _setNextData_structvar_var(_ptr);
    }

    void _setNextData_structvar_var(SvarInfo &_ptr) {
        std::cerr << "_setNextData_structvar_var(SvarInfo &_ptr" << std::endl;
        _chkOperation(Eoperation_Set);

        int iArrOrVarSize(_ptr.m_ui32Size * _ptr.m_ui16Count);
        _reserveBufferSize(sizeof(_ptr.m_ui8TypeId) + sizeof(_ptr.m_ui16Count) + iArrOrVarSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &_ptr.m_ui8TypeId, sizeof(_ptr.m_ui8TypeId));
        m_iIndex += sizeof(_ptr.m_ui8TypeId);

        ::memcpy(&m_vecBuffer[m_iIndex], &_ptr.m_ui16Count, sizeof(_ptr.m_ui16Count));
        m_iIndex += sizeof(_ptr.m_ui16Count);

        ::memcpy(&m_vecBuffer[m_iIndex], _ptr.m_ptr, iArrOrVarSize);
        m_iIndex += iArrOrVarSize;
    }

    template <typename T, size_t N>
    void _getTypeInfos(const T(&_t)[N], uint8_t *_out_ui8TypeId, uint32_t *_out_ui32TypeFullSize, std::vector<std::vector<uint32_t> **> _vecTypesSizes, const bool &_bSet) {
        std::cerr << "_getTypeInfos(const T(&_t)[N]," << std::endl; 
        getTypeInfos(_t[0], _out_ui8TypeId, _out_ui32TypeFullSize, _vecTypesSizes);

        if (!_bSet) {
            _chkExpectedType(*_out_ui8TypeId);
            m_iIndex += sizeof(uint8_t);
        }
    }

    template<typename T>
    void _getTypeInfos(const T &_t, uint8_t *_out_ui8TypeId, uint32_t *_out_ui32TypeFullSize, std::vector<std::vector<uint32_t>**> _vecTypesSizes, const bool &_bSet) {
        std::cerr << "_getTypeInfos(const T &_t," << std::endl;
        getTypeInfos(_t, _out_ui8TypeId, _out_ui32TypeFullSize, _vecTypesSizes);

        if (!_bSet) {
            _chkExpectedType(*_out_ui8TypeId);
            m_iIndex += sizeof(uint8_t);
        }
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


    void _setHeader(const uint8_t &_ui8TypeId, const int &_iLength) {
        _reserveBufferSize(sizeof(uint8_t) + sizeof(uint16_t));

        ::memcpy(&m_vecBuffer[m_iIndex], &_ui8TypeId, sizeof(uint8_t));
        ++m_iIndex;

        if (_iLength > static_cast<int>(UINT16_MAX))
            std::runtime_error("The number of elements exceeds the limit value.");

        ::memcpy(&m_vecBuffer[m_iIndex], reinterpret_cast<const uint16_t*>(&_iLength), sizeof(uint16_t));
        m_iIndex += sizeof(uint16_t);
    }


    inline uint16_t _getElemCount() {
        uint16_t ui16Count(0);
        ::memcpy(&ui16Count, &m_vecBuffer[m_iIndex], sizeof(uint16_t));
        _incrementIndex(sizeof(uint16_t));
        return ui16Count;
    }


    inline void _chkExpectedType(const uint8_t &_ui8TypeId) {
        if (_ui8TypeId != m_vecBuffer[m_iIndex])
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

    static mapTypesInfos s_mapTypes;
    static uint8_t       s_ui8LastId;
};



#define TEST2(VAR)  Cserializing::SvarInfo<decltype(VAR)>(VAR)
#define TEST(...)   friend class Cserializing; protected: std::vector<Cserializing::SvarInfo> __v { ##__VA_ARGS__ }