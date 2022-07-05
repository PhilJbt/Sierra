#pragma once


#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <mutex>
#include <vector>
#include <map>
#include <unordered_map>
#include <numeric>

#ifndef SK_PACKET_SIZE_MAX
    #define SK_PACKET_SIZE_MAX 14000
#endif



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
                    getTypeInfos(_var, &m_ui8TypeId, &m_ui32Size, nullptr);
                    m_ptr = &_var;
                }
            }
        }

        uint8_t   m_ui8TypeId = 0;
        uint32_t  m_ui32Size = 0;
        void     *m_ptr = nullptr;
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
    void setNextData(T &_var, int _iCount = 1) {
        constexpr bool has_varInfos = requires(T &_var) {
            _var.__v;
        };

        if constexpr (has_varInfos) {
            for (int i(0); i < _var.__v.size(); ++i)
                _setNextData_structvar(_var.__v[i]);
        }
        else
            _setNextData_template(_var, _iCount);
    }


    template<typename U>
    void setNextData(std::vector<U> &_vec) {
        _chkOperation(Eoperation_Set);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(_vec, &ui8TypeId, &ui32TypeSize, nullptr, true);

        int iCount(static_cast<int>(_vec.size()));
        _setHeader(ui8TypeId, iCount);

        uint32_t ui32DataSize(ui32TypeSize * iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &_vec[0], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename U, typename V>
    void setNextData(std::map<U, V> &_map) {
        _chkOperation(Eoperation_Set);

        uint8_t ui8TypeId(0);
        uint32_t ui32Size(0);
        std::vector<uint32_t> vecTypeSizes;
        _getTypeInfos(_map, &ui8TypeId, &ui32Size, &vecTypeSizes, true);

        int iCount(static_cast<int>(_map.size()));
        _setHeader(ui8TypeId, iCount);

        _reserveBufferSize((vecTypeSizes[0] + vecTypeSizes[1]) * iCount);

        for (const auto &[key, val] : _map) {
            ::memcpy(&m_vecBuffer[m_iIndex], &key, vecTypeSizes[0]);
            _incrementIndex(vecTypeSizes[0]);
            ::memcpy(&m_vecBuffer[m_iIndex], &val, vecTypeSizes[1]);
            _incrementIndex(vecTypeSizes[1]);
        }
    }
    
    template<typename T>
    void wtf(T &_var) {
        constexpr bool has_varInfos = requires(T & _var) {
            _var.__v;
        };

        if constexpr (has_varInfos) {
            for (int i(0); i < _var.__v.size(); ++i) {
                if (_var.__v[i].m_bIsVarInfosVec) {
                    std::vector<Cserializing::SvarInfo> &vec = (*reinterpret_cast<std::vector<Cserializing::SvarInfo>*>(_var.__v[i].m_ptr));
                    for (int i(0); i < vec.size(); ++i)
                        wtf2(vec[i]);
                }
                else
                    wtf2(_var.__v[i]);
            }
        }
        else
            getNextData(_var);
    }

    void wtf2(SvarInfo &_ptr) {
        _chkOperation(Eoperation_Get);

        m_iIndex += sizeof(uint8_t);

        uint16_t ui16Count(_getElemCount());
        uint32_t ui32DataSize(_ptr.m_ui32Size * ui16Count);

        ::memcpy(_ptr.m_ptr, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename T>
    void getNextData(T &_var) {
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        if (_getTypeInfos(_var, &ui8TypeId, &ui32TypeSize, nullptr, false))
            return;

        uint16_t ui16Count(_getElemCount());
        uint32_t ui32DataSize(ui32TypeSize * ui16Count);

        ::memcpy(&_var, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename U>
    void getNextData(std::vector<U> &_vec) {
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        if (_getTypeInfos(_vec, &ui8TypeId, &ui32TypeSize, nullptr, false))
            return;

        uint16_t ui16Count(_getElemCount());
        uint16_t ui16Size(ui32TypeSize * ui16Count);
        _vec.resize(ui16Count);

        ::memcpy(&_vec[0], &m_vecBuffer[m_iIndex], ui16Size);
        _incrementIndex(ui16Size);
    }

    template<typename U, typename V>
    void getNextData(std::map<U, V> &_map) {
        _chkOperation(Eoperation_Get);

        uint8_t ui8TypeId(0);
        std::vector<uint32_t> vecTypesSizes;
        if (_getTypeInfos(_map, &ui8TypeId, nullptr, &vecTypesSizes, false))
            return;

        uint16_t ui16Count(_getElemCount());

        for (uint16_t i(0); i < ui16Count; ++i) {
            U key;
            ::memcpy(&key, &m_vecBuffer[m_iIndex], vecTypesSizes[0]);
            _incrementIndex(vecTypesSizes[0]);

            V val;
            ::memcpy(&val, &m_vecBuffer[m_iIndex], vecTypesSizes[1]);
            _incrementIndex(vecTypesSizes[1]);

            _map.insert({ key, val });
        }
    }


    template<typename T>
    static void registerType(const T &_t) {
        s_mapTypes.insert({ std::type_index(typeid(_t)), { sizeof(_t) } });
    }

    template<typename U>
    static void registerType(const std::vector<U> &_t) {
        s_mapTypes.insert({ std::type_index(typeid(_t)), { sizeof(U) } });
    }

    template<typename U, typename V>
    static void registerType(const std::map<U, V> &_t) {
        std::vector<uint32_t> vecSizesOf;
        vecSizesOf.push_back(sizeof(U));
        vecSizesOf.push_back(sizeof(V));
        s_mapTypes.insert({ std::type_index(typeid(_t)), vecSizesOf });
    }


    void changeOperationType(const Eoperation_ &_eTypeOperation) {
        if (m_eOperation == Eoperation_NULL)
            throw std::runtime_error("Eoperation_NULL operation type cannot be set.");
        m_eOperation = _eTypeOperation;
        m_iIndex = 0;
    }

    template<typename T>
    static bool isTypeRegistered(const T &_t) {
        std::type_index tiCur(std::type_index(typeid(_t)));
        return s_mapTypes.count(tiCur) == 1;
    }

    template<typename T>
    static void getTypeInfos(const T &_t, uint8_t *_out_ui8TypeId, uint32_t *_out_ui32TypeFullSize, std::vector<uint32_t> *_out_vecTypeSizes) {
        std::type_index tiCur(std::type_index(typeid(_t)));
        bool bFound(s_mapTypes.count(tiCur) == 1);
        if (!bFound)
            throw std::runtime_error("Type not registered yet");
        std::unordered_map<std::type_index, std::vector<uint32_t>>::iterator itElem(s_mapTypes.find(tiCur));

        if (_out_ui8TypeId != nullptr)
            *_out_ui8TypeId = static_cast<uint8_t>(std::distance(s_mapTypes.begin(), itElem));
        if (_out_ui32TypeFullSize != nullptr)
            *_out_ui32TypeFullSize = std::accumulate(itElem->second.begin(), itElem->second.end(), 0);
        if (_out_vecTypeSizes != nullptr)
            *_out_vecTypeSizes = itElem->second;
    }


    static void initialization() {
        // Typically 1 Byte
        registerType(bool());
        registerType(char());
        registerType(char8_t());
        registerType(signed char());
        registerType(unsigned char());
        registerType(int8_t());
        registerType(uint8_t());

        // Typically 2 Byte
        registerType(short());
        registerType(short int());
        registerType(signed short());
        registerType(unsigned short());
        registerType(signed short int());
        registerType(unsigned short int());
        registerType(char16_t());
        registerType(wchar_t());
        registerType(int16_t());
        registerType(uint16_t());

        // Typically 4 Byte
        registerType(char32_t());
        registerType(int());
        registerType(unsigned int());
        registerType(unsigned long());
        registerType(unsigned());
        registerType(signed());
        registerType(signed int());
        registerType(int32_t());
        registerType(uint32_t());
        registerType(float());
        registerType(float_t());
        registerType(long());
        registerType(long int());
        registerType(signed long());
        registerType(signed long int());
        registerType(unsigned long int());

        // Typically 8 Byte
        registerType(int64_t());
        registerType(uint64_t());
        registerType(double());
        registerType(double_t());
        registerType(long double());
        registerType(long long());
        registerType(unsigned long long());
        registerType(unsigned long long int());
        registerType(long long());
        registerType(long long int());
        registerType(signed long long());
        registerType(signed long long int());
        registerType(unsigned long long());
    }

private:
    template<typename T>
    void _setNextData_template(const T &_var, const int &_iCount) {
        _chkOperation(Eoperation_Set);

        uint8_t ui8TypeId(0);
        uint32_t ui32TypeSize(0);
        _getTypeInfos(_var, &ui8TypeId, &ui32TypeSize, nullptr, true);

        int iCount(_iCount);
        _setHeader(ui8TypeId, iCount);

        uint32_t ui32DataSize(ui32TypeSize * _iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &_var, ui32TypeSize);
        _incrementIndex(ui32DataSize);
    }

    void _setNextData_structvar_var(SvarInfo &_ptr) {
        _chkOperation(Eoperation_Set);

        _reserveBufferSize(sizeof(_ptr.m_ui8TypeId) + sizeof(uint16_t) + _ptr.m_ui32Size);

        ::memcpy(&m_vecBuffer[m_iIndex], &_ptr.m_ui8TypeId, sizeof(_ptr.m_ui8TypeId));
        m_iIndex += sizeof(_ptr.m_ui8TypeId);

        uint16_t ui16Count(1);
        ::memcpy(&m_vecBuffer[m_iIndex], &ui16Count, sizeof(uint16_t));
        m_iIndex += sizeof(ui16Count);

        ::memcpy(&m_vecBuffer[m_iIndex], _ptr.m_ptr, _ptr.m_ui32Size);
        m_iIndex += _ptr.m_ui32Size;
    }

    void _setNextData_structvar(SvarInfo &_ptr) {
        if (_ptr.m_bIsVarInfosVec) {
            std::vector<Cserializing::SvarInfo> &vec = (*reinterpret_cast<std::vector<Cserializing::SvarInfo>*>(_ptr.m_ptr));
            for (int i(0); i < vec.size(); ++i)
                _setNextData_structvar(vec[i]);
        }
        else
            _setNextData_structvar_var(_ptr);
    }

    template<typename T>
    bool _getTypeInfos(const T &_t, uint8_t *_out_ui8TypeId, uint32_t *_out_ui32TypeFullSize, std::vector<uint32_t> *_out_vecTypeSizes, const bool &_bSet) {
        std::type_index tiCur(std::type_index(typeid(_t)));
        bool bFound(s_mapTypes.count(tiCur) == 1);
        if (!bFound)
            throw std::runtime_error("Type not registered yet");
        std::unordered_map<std::type_index, std::vector<uint32_t>>::iterator itElem(s_mapTypes.find(tiCur));

        if (_out_ui8TypeId != nullptr)
            *_out_ui8TypeId = static_cast<uint8_t>(std::distance(s_mapTypes.begin(), itElem));
        if (_out_ui32TypeFullSize != nullptr)
            *_out_ui32TypeFullSize = std::accumulate(itElem->second.begin(), itElem->second.end(), 0);
        if (_out_vecTypeSizes != nullptr)
            *_out_vecTypeSizes = itElem->second;

        if (!_bSet) {
            _chkExpectedType(*_out_ui8TypeId);
            m_iIndex += sizeof(uint8_t);
        }

        return false;
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

    static std::unordered_map<std::type_index, std::vector<uint32_t>> s_mapTypes;
};

std::unordered_map<std::type_index, std::vector<uint32_t>> Cserializing::s_mapTypes;



#define TEST2(VAR)  Cserializing::SvarInfo<decltype(VAR)>(VAR)
#define TEST(...)   friend class Cserializing; protected: std::vector<Cserializing::SvarInfo> __v { ##__VA_ARGS__ }