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
#include <tuple>


typedef uint8_t  typeId_t;
typedef uint32_t typeSize_t;
typedef uint16_t dataCount_t;


struct StypeInfos {
public:
    StypeInfos(typeId_t _ui8TypeId, typeSize_t _ui32Size, std::string _strTypeName)
        : m_ui8TypeId(_ui8TypeId), m_ui32Size(_ui32Size), m_strTypeName(_strTypeName) {}

    typeId_t        *typeId()   { return &m_ui8TypeId;  };
    typeSize_t       size()     { return m_ui32Size;    };
    std::string      typeName() { return m_strTypeName; };

private:
    typeId_t        m_ui8TypeId   = 0;
    typeSize_t      m_ui32Size    = 0;
    std::string     m_strTypeName = "";
};


typedef std::unordered_map<std::string, StypeInfos *> mapTypesInfos_byName;
typedef std::unordered_map<typeId_t, StypeInfos *>    mapTypesInfos_byId;
typedef std::vector<StypeInfos *>                     vecTypesIdsNames;

#define SK_TYPES_NBR_MAX 10


/*
** 
*
*/
class Cserializing {
public:
    struct SnextSequence;

    struct SvarInfo_var_base {
    public:
        virtual ~SvarInfo_var_base() {}
        virtual void populate(Cserializing *_ptrSerializer) const = 0;
    };

    template <typename T>
    struct SvarInfo_var : public SvarInfo_var_base {
    public:
        SvarInfo_var(T *_var) : SvarInfo_var_base(), m_val(_var) {}

        ~SvarInfo_var() {
            m_val = nullptr;
        }

        void populate(Cserializing *_ptrSerializer) {
            _ptrSerializer->_setNextData_write(this, m_val);
        }

        T *val() { return m_val; }
    private:
        T *m_val;
    };

    struct SvarInfo_base {
    public:
        virtual ~SvarInfo_base(){}

        const vecTypesIdsNames &typeIdsNames() const { return m_vecTypeIdsName; }
        dataCount_t             count()        const { return m_ui16Count; }
        void                   *ptrNode()      const { return m_ptrNode; }
        bool                    isInfoVec()    const { return m_bIsVarInfosVec; }

        virtual void                                                       set(SnextSequence *_sSeq, Cserializing *_ptrSerializer) const {}
        virtual void                                                       get(SnextSequence *_sSeq, Cserializing *_ptrSerializer) const {}

    protected:
        vecTypesIdsNames   m_vecTypeIdsName;
        dataCount_t        m_ui16Count      = 0;
        void              *m_ptrNode        = nullptr;
        bool               m_bIsVarInfosVec = false;
    };

    template <typename T>
    struct SvarInfo : public SvarInfo_base {
    public:
        SvarInfo(T &_var) {
            if (Cserializing::s_bRegisterDone) {
                if (!_storeNode(_var))
                    _storeVar(_var);
            }
        }

        void set(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) const {
            //m_ptrVar->populate(_ptrSerializer);
            _ptrSerializer->_getNextData_retrieve(_sSeq, _iReadOffset , * m_ptrVar);
        }

        void get(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) const {
            //m_ptrVar->populate(_ptrSerializer);
            _ptrSerializer->_getNextData_retrieve(_sSeq, _iReadOffset , *m_ptrVar);
        }

    private:
        template <typename T>
        bool _storeNode(T &_var) {
            constexpr bool has_varInfos = requires(T & _var) {
                _var.__v;
            };

            if constexpr (has_varInfos) {
                m_ptrNode = (void *)&_var.__v;
                m_bIsVarInfosVec = true;
            }
            else
                return false;

            return true;
        }

        template<typename T>
        void _storeVar(T &_var) {
            StypeInfos *sTypeInfos(nullptr);
            Cserializing::getTypeInfos_byType(_var, &sTypeInfos);
            m_vecTypeIdsName.push_back(sTypeInfos);

            //m_ptrNode = (void *)&_var;
            m_ui16Count = 1;
            //m_ptrVar = new SvarInfo_var<T>(&_var);
            m_ptrVar = &_var;
        }

        template <typename T, size_t N>
        void _storeVar(T(&_var)[N]) {
            StypeInfos *sTypeInfos(nullptr);
            Cserializing::getTypeInfos_byType(_var[0], &sTypeInfos);
            m_vecTypeIdsName.push_back(sTypeInfos);

            //m_ptrNode = (void *)&_var;
            m_ui16Count = N;
            //m_ptrVar = new SvarInfo_var<T[N]>(&_var);
            m_ptrVar = &_var;
        }

        
        void _storeVar(std::string &_str) {
            StypeInfos *sTypeInfos(nullptr);
            Cserializing::getTypeInfos_byName("std::string", &sTypeInfos);
            m_vecTypeIdsName.push_back(sTypeInfos);

            //m_ptrNode = (void *)&_str;
            m_ui16Count = static_cast<dataCount_t>(_str.length());
            //m_ptrVar = new SvarInfo_var(&_str);
            m_ptrVar = &_str;
        }

        template <typename U>
        void _storeVar(std::vector<U> &_vec) {
            StypeInfos *sTypeInfos_vec(nullptr);
            StypeInfos *sTypeInfos_var(nullptr);
            Cserializing::getTypeInfos_byName("std::vector", &sTypeInfos_vec);
            Cserializing::getTypeInfos_byType(U(), &sTypeInfos_var);
            m_vecTypeIdsName.push_back(sTypeInfos_vec);
            m_vecTypeIdsName.push_back(sTypeInfos_var);

            //m_ptrNode = (void*)&_vec;
            m_ui16Count = static_cast<dataCount_t>(_vec.size());
            //m_ptrVar = new SvarInfo_var<std::vector<U>>(&_vec);
            m_ptrVar = &_vec;
        }

        T *m_ptrVar = nullptr;
    };

    struct SnextSequence {
    public:
        SnextSequence() {}

        template <typename T>
        SnextSequence(T *_var, uint32_t _ui32DataSize, dataCount_t _ui16Count)
            : m_ui16Count(_ui16Count), m_ui32DataSize(_ui32DataSize) {
            if (_var) {
                m_ui8ArrBuffer = new uint8_t[_ui32DataSize];
                ::memset(m_ui8ArrBuffer, 0, _ui32DataSize);
                ::memcpy(m_ui8ArrBuffer, _var, _ui32DataSize);
            }
        }

        SnextSequence(dataCount_t _ui16Count) : m_ui16Count(_ui16Count), m_ui32DataSize(0) {
        }

        ~SnextSequence() {
            if (m_ui8ArrBuffer)
                delete[] m_ui8ArrBuffer;
        }

        typeId_t getChunkId(int &_iOffset) {
            typeId_t ui8ChunkId(0);
            ::memcpy(&ui8ChunkId, &m_ui8ArrBuffer[_iOffset], sizeof(typeId_t));
            _iOffset += sizeof(typeId_t);
            return ui8ChunkId;
        }

        void setChunkId(typeId_t _ui8ChunkId, int &_iOffset) {
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_ui8ChunkId, sizeof(typeId_t));
            _iOffset += sizeof(typeId_t);
        }

        void setChunkSize(uint32_t _ui32Weight, int &_iOffset) {
            m_ui8ArrBuffer = new uint8_t[_ui32Weight];
            ::memset(m_ui8ArrBuffer, 0, _ui32Weight);
            m_ui32DataSize = _ui32Weight;
        }

        dataCount_t readCount(int &_iOffset) {
            dataCount_t ui16Count(0);
            ::memcpy(&ui16Count, &m_ui8ArrBuffer[_iOffset], sizeof(dataCount_t));
            _iOffset += sizeof(dataCount_t);
            return ui16Count;
        }

        void writeCount(dataCount_t _ui16Count, int &_iOffset) {
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_ui16Count, sizeof(dataCount_t));
            _iOffset += sizeof(dataCount_t);
        }

        template <typename T>
        void readData(T &_var, int &_iOffset) {
            ::memcpy(&_var, &m_ui8ArrBuffer[_iOffset], sizeof(_var));
            _iOffset += sizeof(_var);
        }

        template <typename T>
        void writeData(T &_var, int &_iOffset) {
            uint32_t ui32DataSize(sizeof(_var));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_var, ui32DataSize);
            _iOffset += sizeof(_var);
        }

        void nextNode(SnextSequence *_ptrNext) {
            m_ptrNext = _ptrNext;
        }

        SnextSequence* nextNode() {
            return m_ptrNext;
        }

        uint8_t *buffer(uint8_t _ui8Offset = 0) {
            return (m_ui8ArrBuffer + _ui8Offset);
        }

        int length() {
            dataCount_t ui16Count(0);
            ::memcpy(&ui16Count, &m_ui8ArrBuffer[0], sizeof(ui16Count));
            return static_cast<int>(ui16Count);
        }

        int count() {
            return static_cast<int>(m_ui16Count);
        }

        int dataSize() {
            return static_cast<int>(m_ui32DataSize);
        }

        bool type(const int &_iTypeOrderIndex, const typeId_t *const _ui8TypeId) {
            return ((*_ui8TypeId) == m_ui8Types[_iTypeOrderIndex]);
        }

        uint8_t *type() {
            return m_ui8Types;
        }


        void serialize(uint8_t **_ui8Buffer, int &_iIndex) {
            _serialize_types(_ui8Buffer, _iIndex);
            _serialize_count(_ui8Buffer, _iIndex);
            _serialize_data(_ui8Buffer, _iIndex);
        }

        void unserialize(uint8_t **_ui8Buffer, int &_iIndex) {
            _unserialize_types(_ui8Buffer, _iIndex);
            _unserialize_count(_ui8Buffer, _iIndex);
            _unserialize_size();
            _unserialize_data(_ui8Buffer, _iIndex);
        }


        void typeSet(const int &_iTypeOrderIndex, const typeId_t *const _ui8TypeId) {
            m_ui8Types[_iTypeOrderIndex] = (*_ui8TypeId);
        }

    private:

        void _serialize_types(uint8_t **_ui8Buffer, int &_iLength) {
            for (int i(0); i < SK_TYPES_NBR_MAX; ++i) {
                ::memcpy((*_ui8Buffer) + _iLength, &m_ui8Types[i], sizeof(typeId_t));
                _iLength += sizeof(typeId_t);

                if (m_ui8Types[i] == 0)
                    return;
            }
        }

        void _serialize_count(uint8_t **_ui8Buffer, int &_iLength) {
            int iValueMaxNbr(static_cast<int>(::floor(static_cast<float>(_iLength) / static_cast<float>(std::numeric_limits<dataCount_t>::max()))));

            for (int i(0); i < iValueMaxNbr + 2; ++i) {
                switch (iValueMaxNbr - i) {
                case 0:
                    ::memcpy((*_ui8Buffer) + _iLength, &m_ui16Count, sizeof(dataCount_t));
                    break;

                case -1:
                {
                    dataCount_t ui16CountValNull(0);
                    ::memcpy((*_ui8Buffer) + _iLength, &ui16CountValNull, sizeof(dataCount_t));
                }
                break;

                default:
                {
                    dataCount_t ui16CountValMax(std::numeric_limits<dataCount_t>::max());
                    ::memcpy((*_ui8Buffer) + _iLength, &ui16CountValMax, sizeof(dataCount_t));
                }
                break;
                }

                _iLength += sizeof(dataCount_t);
            }
        }

        void _serialize_data(uint8_t **_ui8Buffer, int &_iLength) {
            ::memcpy((*_ui8Buffer) + _iLength, m_ui8ArrBuffer, m_ui32DataSize);
            _iLength += m_ui32DataSize;
        }


        void _unserialize_types(uint8_t **_ui8Buffer, int &_iIndex) {
            for (int i(0); i < SK_TYPES_NBR_MAX; ++i) {
                ::memcpy(&m_ui8Types[i], (*_ui8Buffer) + _iIndex, sizeof(typeId_t));
                _iIndex += sizeof(typeId_t);
                if (m_ui8Types[i] == 0)
                    return;
            }
        }

        void _unserialize_count(uint8_t **_ui8Buffer, int &_iIndex) {
            dataCount_t ui16DataCount(0);

            do {
                ::memcpy(&ui16DataCount, (*_ui8Buffer) + _iIndex, sizeof(dataCount_t));
                _iIndex += sizeof(dataCount_t);
                m_ui16Count += static_cast<dataCount_t>(ui16DataCount);
            } while (ui16DataCount != 0);
        }

        void _unserialize_size() {
            m_ui32DataSize = 0;
            for (int i(0); i < SK_TYPES_NBR_MAX; ++i) {
                if (m_ui8Types[i] != 0) {
                    StypeInfos *sTypeInfos(nullptr);
                    getTypeInfos_byId(m_ui8Types[i], &sTypeInfos);
                    m_ui32DataSize += static_cast<uint32_t>(sTypeInfos->size());
                }
                else
                    i = SK_TYPES_NBR_MAX;
            }
            
            m_ui32DataSize *= static_cast<uint32_t>(m_ui16Count);
        }

        void _unserialize_data(uint8_t **_ui8Buffer, int &_iIndex) {
            m_ui8ArrBuffer = new uint8_t[m_ui32DataSize];
            ::memset(m_ui8ArrBuffer, 0, m_ui32DataSize);
            ::memcpy(m_ui8ArrBuffer, (*_ui8Buffer) + _iIndex, m_ui32DataSize);
            _iIndex += m_ui32DataSize;
        }


        uint8_t       *m_ui8ArrBuffer               = nullptr;
        dataCount_t    m_ui16Count                  = 0;
        uint32_t       m_ui32DataSize               = 0;
        typeId_t       m_ui8Types[SK_TYPES_NBR_MAX] = { 0 };
        SnextSequence *m_ptrNext                    = nullptr;
    };


    ~Cserializing() {
        SnextSequence *ptrCur(m_ptrSeqBeg),
                      *ptrTmp(nullptr);

        while (ptrCur) {
            ptrTmp = ptrCur->nextNode();
            if (ptrCur)
                delete ptrCur;
            ptrCur = ptrTmp;
        }
    }


    template <typename T>
    static void __v_push_var(std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec, T &_var) {
        _vec.push_back(std::make_unique<Cserializing::SvarInfo<T>>(_var));
    }

    template <typename... T>
    static std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v_push(T&&... args) {
        if (Cserializing::s_bRegisterDone) {

            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v;

            //(__v.push_back( std::move(std::unique_ptr<T>(new Cserializing::SvarInfo(args))) ), ...);
            //(__v.push_back(std::move(new Cserializing::SvarInfo(args))), ...);
            (__v_push_var(__v, args), ...);
            
            return __v;
        }
        else
            return std::vector<std::unique_ptr<Cserializing::SvarInfo_base>>();
    }


    //bool _nextDataType(const std::string &_str) {
    //    StypeInfos *sTypeInfos(nullptr);
    //    getTypeInfos_byName("std::string", &sTypeInfos);

    //    return m_ptrSeqCur->type(0, sTypeInfos->typeId());
    //}

    //template<typename U>
    //bool _nextDataType(const std::vector<U> &_vec) {
    //    StypeInfos *sTypeInfos_vec(nullptr);
    //    StypeInfos *sTypeInfos_var(nullptr);
    //    getTypeInfos_byName("std::vector", &sTypeInfos_vec);
    //    getTypeInfos_byType(U(), &sTypeInfos_var);

    //    return (m_ptrSeqCur->type(0, sTypeInfos_vec->typeId()) && m_ptrSeqCur->type(1, sTypeInfos_var->typeId()));
    //}

    //template<typename T>
    //bool _nextDataType(const T &_var) {
    //    StypeInfos *sTypeInfos(nullptr);
    //    getTypeInfos_byType(_var, &sTypeInfos);

    //    return m_ptrSeqCur->type(0, sTypeInfos->typeId());
    //}

    //template <typename T, size_t N>
    //bool _nextDataType(T(&_var)[N]) {
    //    StypeInfos *sTypeInfos(nullptr);
    //    getTypeInfos_byType(_var[0], &sTypeInfos);

    //    return (m_ptrSeqCur->type(0, sTypeInfos->typeId()) && (static_cast<dataCount_t>(N) == static_cast<dataCount_t>(m_ptrSeqCur->count())));
    //}


    int nextDataCount() {
        return m_ptrSeqCur->count();
    }


    template<typename T>
    void setNextData(const typeId_t &_ui8TypeId, const T &_var, int _iCount = 1, typename std::enable_if<!std::is_pointer<T>::value >::type* = 0) {
        int iWriteCursor(0);
        SnextSequence *sSeq(new SnextSequence());
        if (!_setNextData_structvar_(sSeq, _var)) {
            if (_iCount == 1) {
                auto [iCount, iWeight] = _setNextData_calculateSize(_var);
                sSeq->setChunkSize(iWeight, iWriteCursor);

                sSeq->setChunkId(_ui8TypeId, iWriteCursor);

                _setNextData_write(sSeq, iWriteCursor, _var);
            }
            else {
                _setNextData_write(sSeq, iWriteCursor, _var, _iCount);
            }
        }
        _updateSeqPtr_Set(sSeq);
    }


    template<typename T>
    void getNextData(const typeId_t &_ui8TypeId, T &_var, int _iCount = 1, typename std::enable_if<!std::is_pointer<T>::value>::type* = 0) {
        int iReadCursor(0);
        if (m_ptrSeqCur->getChunkId(iReadCursor) == _ui8TypeId) {
            if (!_getNextData_structvar_(m_ptrSeqCur, _var)) {
                if (_iCount == 1)
                    _getNextData_retrieve(m_ptrSeqCur, iReadCursor, _var);
                /*else
                    _getNextData_template<T>(m_ptrSeqCur, iReadCursor, _iCount);*/

                m_ptrSeqCur = m_ptrSeqCur->nextNode();
            }
        }
    }


    /*template <typename T, size_t N>
    inline static bool isTypeRegistered(const T(&_t)[N]) {
        std::string strTypeName(std::type_index(typeid(const_cast<T &>(_t[0]))).name());
        return s_mapTypes_byName.find(strTypeName) != s_mapTypes_byName.end();
    }*/

    template<typename T>
    inline static bool isTypeRegistered_byType(const T &_t, typename std::enable_if<!std::is_pointer<T>::value >::type* = 0) {
        std::string strTypeName(std::type_index(typeid(const_cast<T &>(_t))).name());
        return s_mapTypes_byName.find(strTypeName) != s_mapTypes_byName.end();
    }

    template<typename T>
    inline static bool isTypeRegistered_byType(const T _t, typename std::enable_if<std::is_pointer<T>::value >::type* = 0) {
        std::string strTypeName(std::type_index(typeid(const_cast<T &>(_t))).name());
        return s_mapTypes_byName.find(strTypeName) != s_mapTypes_byName.end();
    }

    template<typename T>
    inline static bool isTypeRegistered_byName(const T &_strTypeName, typename std::enable_if<std::is_same<T, std::string>::value && !std::is_pointer<T>::value >::type* = 0) {
        return s_mapTypes_byName.find(_strTypeName) != s_mapTypes_byName.end();
    }

    inline static bool isTypeRegistered_byId(const typeId_t &_ui8Type) {
        return s_mapTypes_byId.find(_ui8Type) != s_mapTypes_byId.end();
    }


    template<typename T>
    static void getTypeInfos_byType(const T &_t, StypeInfos **_out_sTypeInfos) {
        std::string strTypeName(std::type_index(typeid(const_cast<T &>(_t))).name());
        if (strTypeName.find("std::vector") != std::string::npos)
            return getTypeInfos_byName("std::vector", _out_sTypeInfos);
        else if (!isTypeRegistered_byName(strTypeName))
            throw std::runtime_error("Type not registered");
        const mapTypesInfos_byName::iterator &&itElem(s_mapTypes_byName.find(strTypeName));
        
        *_out_sTypeInfos = itElem->second;
    }

    static void getTypeInfos_byId(const typeId_t &_ui8Type, StypeInfos **_out_sTypeInfos) {
        if (!isTypeRegistered_byId(_ui8Type))
            throw std::runtime_error("Type not registered");
        const mapTypesInfos_byId::iterator &&itElem(s_mapTypes_byId.find(_ui8Type));

        *_out_sTypeInfos = itElem->second;
    }

    static void getTypeInfos_byName(std::string _strTypeName, StypeInfos **_out_sTypeInfos) {
        if (!isTypeRegistered_byName(_strTypeName))
            throw std::runtime_error("Type not registered");
        const mapTypesInfos_byName::iterator &&itElem(s_mapTypes_byName.find(_strTypeName));

        *_out_sTypeInfos = itElem->second;
    }


    template <typename... T>
    static void initialization(T&&... args) {
        _initialization();

        (_registerType_force(typeid(args).name(), 0), ...);

        Cserializing::s_bRegisterDone = true;
    }

    static void desinitialisation() {
        for (auto &&elem : s_mapTypes_byName)
            delete elem.second;

        s_mapTypes_byName.clear();
        s_mapTypes_byId.clear();
    }


    void changeTypeTo_Set() {
        m_ptrSeqCur = m_ptrSeqEnd;
    }

    void changeTypeTo_Get() {
        m_ptrSeqCur = m_ptrSeqBeg;
    }


    void serialize(uint8_t **_ptrEmptyBuffer, int &_iBufferLength, const bool &_bAllocateMemory) {
        if (_bAllocateMemory) {
            _iBufferLength = 0;
            SnextSequence *ptrSeq(m_ptrSeqBeg);

            while (ptrSeq) {
                int iNbrTypes(0);
                typeId_t *ui8Types(ptrSeq->type());
                for (int i(0); i < SK_TYPES_NBR_MAX; ++i)
                    if (ui8Types[i] > 0)
                        ++iNbrTypes;
                    else
                        i = SK_TYPES_NBR_MAX;
                _iBufferLength += static_cast<int>(sizeof(typeId_t)) * (iNbrTypes + 1);



                _iBufferLength += ((static_cast<int>(::ceil(static_cast<float>(ptrSeq->count()) / static_cast<float>(UINT16_MAX))) + 1) * static_cast<int>(sizeof(dataCount_t)));



                _iBufferLength += ptrSeq->dataSize();



                ptrSeq = ptrSeq->nextNode();
            }

            *_ptrEmptyBuffer = new uint8_t[_iBufferLength];
            ::memset(*_ptrEmptyBuffer, 0, _iBufferLength);
        }

        int iIndex(0);
        SnextSequence *ptrSeq(m_ptrSeqBeg);
        while (ptrSeq && iIndex < _iBufferLength) {
            ptrSeq->serialize(_ptrEmptyBuffer, iIndex);
            
            /*int iIndexTemp(0),
                iIbeg(0),
                iIend(0);
            typeId_t *ui8Types(ptrSeq->type());

            if (ui8Types[1] == 0) {
                iIbeg = 0;
                iIend = 1;
            }
            else {
                iIbeg = 1;
                iIend = SK_TYPES_NBR_MAX;
            }

            for (int i(iIbeg); i < iIend; ++i) {
                StypeInfos *sTypeInfos(nullptr);
                getTypeInfos_byId(ui8Types[i], &sTypeInfos);
                iIndexTemp += static_cast<int>(ptrSeq->count()) * static_cast<int>(sTypeInfos->size());
            }

            iIndex += iIndexTemp;*/

            ptrSeq = ptrSeq->nextNode();
        }
    }

    void unserialize(uint8_t **_ptrBuffer, const int &_iSize) {
        int iIndex(0);
        
        while (iIndex < _iSize) {
            SnextSequence *ptrSeq(new SnextSequence());
            ptrSeq->unserialize(_ptrBuffer, iIndex);
            _updateSeqPtr_Set(ptrSeq);
        }

        changeTypeTo_Get();
    }

private:
    static void _initialization() {
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

    template <typename... T>
    static void _registerTypes(T&&... args) {
        (_registerType_unpack(args), ...);
    }

    template<typename T>
    static void _registerType_unpack(const T &_t) {
        std::string strTypeName(typeid(_t).name());
        if (!isTypeRegistered_byName(strTypeName)) {
            typeId_t         ui8TypeId(static_cast<typeId_t>(s_mapTypes_byName.size()) + 1);
            typeSize_t       ui32Size(sizeof(_t));
            std::type_index  type(typeid(_t));
            StypeInfos      *sTemp(new StypeInfos(ui8TypeId, ui32Size, strTypeName));

            s_mapTypes_byId.insert(
                {
                    ui8TypeId,
                    sTemp
                }
            );

            s_mapTypes_byName.insert(
                {
                    std::type_index(typeid(_t)).name(),
                    sTemp
                }
            );
        }
    }

    static void _registerType_force(std::string _str, const typeSize_t &_ui32Size = 0) {
        typeId_t         ui8TypeId(static_cast<typeId_t>(s_mapTypes_byName.size()) + 1);
        std::type_index  type(typeid(void));
        StypeInfos      *sTemp(new StypeInfos(ui8TypeId, _ui32Size, _str));

        s_mapTypes_byId.insert(
            {
                ui8TypeId,
                sTemp
            }
        );

        s_mapTypes_byName.insert(
            {
                _str,
                sTemp
            }
        );
    }


    /*template<typename T>
    void _getNextData_unfold(T *_var) {
        constexpr bool has_varInfos = requires(T * _var) {
            _var->__v;
        };

        if constexpr (has_varInfos) {
            for (int i(0); i < _var.__v.size(); ++i) {
                if (_var.__v[i].m_bIsVarInfosVec) {
                    std::vector<Cserializing::SvarInfo> &vec = (*reinterpret_cast<std::vector<Cserializing::SvarInfo>*>(_var.__v[i].ptr()));
                    for (int i(0); i < vec.size(); ++i)
                        _getNextData_varInfo(vec[i]);
                }
                else
                    _getNextData_varInfo(_var.__v[i]);
            }
        }
        else
            _getNextData_var(_var);
    }*/

    /*void _getNextData_varInfo(const SvarInfo_base &_ptr) {
        m_iIndexCur += sizeof(typeId_t);

        dataCount_t ui16Count(m_ptrSeqCur->nextCount());
        uint32_t    ui32DataSize(_ptr.typeIdsNames()[0]->size() * ui16Count);

        ::memcpy(_ptr.ptrNode(), &m_ptrSeqCur->m_ui8ArrBuffer[m_iIndexCur], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }*/

    /*template <typename T, size_t N>
    void _getNextData_template(SnextSequence *_sSeq, T(&_var)[N]) {
        dataCount_t ui16Count(m_ptrSeqCur->count());

        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byType(T(), &sTypeInfos);

        if (ui16Count != N
            || !m_ptrSeqCur->type(0, sTypeInfos->typeId()))
            return;

        uint32_t ui32DataSize(sTypeInfos->size() * ui16Count);

        ::memcpy(_var, m_ptrSeqCur->buffer(), ui32DataSize);
    }*/

    template<typename T>
    void _getNextData_retrieve(SnextSequence *_sSeq, int &_iReadOffset, T &_var, int _iCount = 1) {
        T var;
        _sSeq->readData(var, _iReadOffset);
        _var = var;
        /*dataCount_t ui16Count(m_ptrSeqCur->count());

        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byType(T(), &sTypeInfos);

        if (ui16Count != _iCount
            || !m_ptrSeqCur->type(0, sTypeInfos->typeId()))
            return;

        uint32_t ui32DataSize(sTypeInfos->size() * ui16Count);

        ::memcpy((void *)&_var, m_ptrSeqCur->buffer(), ui32DataSize);*/
    }
    
    /*void _getNextData_template(SnextSequence *_sSeq, std::string &_var) {
        dataCount_t ui16Count(m_ptrSeqCur->count());

        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byName("std::string", &sTypeInfos);

        if (!m_ptrSeqCur->type(0, sTypeInfos->typeId()))
            return;

        uint32_t ui32DataSize(sTypeInfos->size() * ui16Count);

        _var = std::string(reinterpret_cast<char*>(m_ptrSeqCur->buffer()), ui32DataSize);
    }*/

    template<class T>
    void _getNextData_retrieve(SnextSequence *_sSeq, int &_iReadOffset, std::vector<T> &_vec) {
        int iCount(static_cast<int>(_sSeq->readCount(_iReadOffset)));

        _vec.resize(iCount);
        for (int i(0); i < iCount; ++i)
            _getNextData_retrieve(_sSeq, _iReadOffset, _vec[i]);

        /*dataCount_t ui16Count(m_ptrSeqCur->count());

        StypeInfos *sTypeInfos_vec(nullptr);
        StypeInfos *sTypeInfos_var(nullptr);
        getTypeInfos_byName("std::vector", &sTypeInfos_vec);
        getTypeInfos_byType(U(), &sTypeInfos_var);

        uint32_t ui32DataSize(sTypeInfos_var->size() * static_cast<uint32_t>(ui16Count));

        if (!m_ptrSeqCur->type(0, sTypeInfos_vec->typeId())
        || !m_ptrSeqCur->type(1, sTypeInfos_var->typeId()))
            return;

        _vec.resize(ui16Count);

        ::memcpy(&_vec[0], m_ptrSeqCur->buffer(), ui32DataSize);*/
    }

    /*template<typename U, typename V>
    void _getNextData_template(SnextSequence *_sSeq, std::map<U, V> &_map) {
        dataCount_t ui16Count(m_ptrSeqCur->count());

        StypeInfos *sTypeInfos_map(nullptr),
                   *sTypeInfos_var1(nullptr),
                   *sTypeInfos_var2(nullptr);
        getTypeInfos_byName("std::map", &sTypeInfos_map);
        getTypeInfos_byType(U(), &sTypeInfos_var1);
        getTypeInfos_byType(V(), &sTypeInfos_var2);

        typeSize_t ui32SizeType1(sizeof(U)),
                   ui32SizeType2(sizeof(V));

        if (!m_ptrSeqCur->type(0, sTypeInfos_map->typeId())
            || !m_ptrSeqCur->type(1, sTypeInfos_var1->typeId())
            || !m_ptrSeqCur->type(2, sTypeInfos_var2->typeId()))
            return;

        int iIndex(0);
        for (uint16_t i(0); i < ui16Count; ++i) {
            U key;
            ::memcpy(&key, &m_ptrSeqCur->buffer(iIndex), ui32SizeType1);
            iIndex += ui32SizeType1;

            V val;
            ::memcpy(&val, &m_ptrSeqCur->buffer(iIndex), ui32SizeType2);
            iIndex += ui32SizeType2;

            _map.insert({ key, val });
        }
    }*/

    template<typename T>
    bool _getNextData_structvar_(SnextSequence *_sSeq, const T &_var) {
        constexpr bool has_varInfos = requires(T & _var) {
            _var.__v;
        };

        if constexpr (has_varInfos) {
            dataCount_t ui16Count(m_ptrSeqCur->count());

            StypeInfos *sTypeInfos_UserStruct(nullptr);
            getTypeInfos_byType(_var, &sTypeInfos_UserStruct);

            if (!m_ptrSeqCur->type(0, sTypeInfos_UserStruct->typeId()))
                return false;

            _updateSeqPtr_Get();

            for (int i(0); i < _var.__v.size(); ++i)
                _getNextData_structvar_run(_sSeq, *_var.__v[i].get());
        }
        else
            return false;

        return true;
    }

    void _getNextData_structvar_run(SnextSequence *_sSeq, const SvarInfo_base &_var) {
        if (_var.isInfoVec()) {
            //std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &vec = (*reinterpret_cast<std::vector<std::unique_ptr<Cserializing::SvarInfo_base>>>(_var.ptrNode()));
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec = reinterpret_cast<std::vector<std::unique_ptr<Cserializing::SvarInfo_base>>*>(_var.ptrNode());

            for (int i(0); i < vec->size(); ++i)
                _getNextData_structvar_run(_sSeq , *vec->at(i).get());
        }
        else
            _var.get(_sSeq, this);

        _updateSeqPtr_Get();
    }


    void _updateSeqPtr_Set(SnextSequence *_ptrSeq) {
        if (m_ptrSeqBeg == nullptr)
            m_ptrSeqBeg = _ptrSeq;
        if (m_ptrSeqCur)
            m_ptrSeqCur->nextNode(_ptrSeq);
        m_ptrSeqEnd = m_ptrSeqCur = _ptrSeq;
    }

    void _updateSeqPtr_Get() {
        if (m_ptrSeqCur->nextNode())
            m_ptrSeqCur = m_ptrSeqCur->nextNode();
    }


    template <typename T>
    std::tuple<int, int> _setNextData_calculateSize(const T &_var, const int &_iCount = 1) {
        return std::tuple<int, int>(1, sizeof(T));

        /*StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byType(T(), &sTypeInfos);
        uint32_t ui32DataSize(sTypeInfos->size() * _iCount);

        SnextSequence *sSeq(new SnextSequence(&_var, ui32DataSize, _iCount));
        sSeq->typeSet(0, sTypeInfos->typeId());
        _updateSeqPtr_Set(sSeq);*/
    }

    template <typename T>
    void _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const T &_var, const int &_iCount = 1) {
        _sSeq->writeData(_var, _iWriteCursor);

        /*StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byType(T(), &sTypeInfos);
        uint32_t ui32DataSize(sTypeInfos->size() * _iCount);

        SnextSequence *sSeq(new SnextSequence(&_var, ui32DataSize, _iCount));
        sSeq->typeSet(0, sTypeInfos->typeId());
        _updateSeqPtr_Set(sSeq);*/
    }

    /*template <typename T, size_t N>
    void _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const T(&_var)[N]) {
        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byType(_var[0], &sTypeInfos);
        uint32_t ui32DataSize(sTypeInfos->size() * N);

        SnextSequence *sSeq(new SnextSequence(&_var, ui32DataSize, N));
        sSeq->typeSet(0, sTypeInfos->typeId());
        _updateSeqPtr_Set(sSeq);
    }*/

    template<typename U>
    std::tuple<int, int> _setNextData_calculateSize(const std::vector<U> &_vec) {
        int iCount (0),
            iWeight(sizeof(dataCount_t));

        for (int i(0); i < _vec.size(); ++i) {
            auto [iGetCount, iGetWeight] = _setNextData_calculateSize(_vec[i]);
            iCount += iGetCount;
            iWeight += iGetWeight;
        }

        return std::tuple<int, int> (iCount, iWeight);

        /*StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byType(T(), &sTypeInfos);
        uint32_t ui32DataSize(sTypeInfos->size() * _iCount);

        SnextSequence *sSeq(new SnextSequence(&_var, ui32DataSize, _iCount));
        sSeq->typeSet(0, sTypeInfos->typeId());
        _updateSeqPtr_Set(sSeq);*/
    }

    template<typename U>
    void _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const std::vector<U> &_vec) {
        _sSeq->writeCount(static_cast<dataCount_t>(_vec.size()), _iWriteCursor);

        for (int i(0); i < _vec.size(); ++i)
            _setNextData_write(_sSeq, _iWriteCursor, _vec[i]);

        /*StypeInfos *sTypeInfos_vec(nullptr),
                   *sTypeInfos_var(nullptr);
        getTypeInfos_byName("std::vector", &sTypeInfos_vec);
        getTypeInfos_byType(_vec[0],       &sTypeInfos_var);

        int iCount(static_cast<int>(_vec.size()));
        uint32_t ui32DataSize(sTypeInfos_var->size() * iCount);

        SnextSequence *sSeq(new SnextSequence(&_vec[0], ui32DataSize, iCount));
        sSeq->typeSet(0, sTypeInfos_vec->typeId());
        sSeq->typeSet(1, sTypeInfos_var->typeId());
        _updateSeqPtr_Set(sSeq);*/
    }

    /*template<typename U, typename V>
    void _setNextData_write(SnextSequence *_sSeq, const std::map<U, V> *_map) {
        _chkOperation(Eoperation_Set);

        typeId_t   ui8TypeId_map(0),
                   ui8TypeId_var1(0),
                   ui8TypeId_var2(0);
        typeSize_t ui32Count(0);
        getTypeInfos_byName("std::map", &ui8TypeId_map, &ui32Count, nullptr);
        getTypeInfos_byType(U(), &ui8TypeId_var1, &ui32Count, nullptr);
        getTypeInfos_byType(V(), &ui8TypeId_var2, &ui32Count, nullptr);

        typeSize_t ui32SizeType1(sizeof(U)),
                   ui32SizeType2(sizeof(V));

        int iCount(static_cast<int>(_map->size()));
        _setHeader(iCount, { ui8TypeId_map, ui8TypeId_var1, ui8TypeId_var2 });

        _reserveBufferSize((ui32SizeType1 + ui32SizeType2) * iCount);

        for (const auto &[key, val] : *_map) {
            ::memcpy(&m_ptrSequenceCur->m_ui8ArrBuffer[m_iIndex], &key, ui32SizeType1);
            _incrementIndex(ui32SizeType1);
            ::memcpy(&m_ptrSequenceCur->m_ui8ArrBuffer[m_iIndex], &val, ui32SizeType2);
            _incrementIndex(ui32SizeType2);
        }
    }*/

    /*void _setNextData_write(SnextSequence *_sSeq, const std::string &_str) {
        std::string str(_str.c_str() + '\0');

        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byName("std::string", &sTypeInfos);
        typeSize_t ui32DataSize(static_cast<int>(str.length()) * sTypeInfos->size());

        SnextSequence *sSeq(new SnextSequence(&str[0], ui32DataSize, static_cast<dataCount_t>(str.length())));
        sSeq->typeSet(0, sTypeInfos->typeId());
        _updateSeqPtr_Set(sSeq);
    }*/

    template<typename T>
    bool _setNextData_structvar_(SnextSequence *_sSeq, const T &_var) {
        constexpr bool has_varInfos = requires(T &_var) {
            _var.__v;
        };

        if constexpr (has_varInfos) {
            _setNextData_structvar_prepare(_var);

            for (int i(0); i < _var.__v.size(); ++i)
                _setNextData_structvar_run(_sSeq, *_var.__v[i].get());
        }
        else
            return false;

        return true;
    }

    template<typename T>
    void _setNextData_structvar_prepare(const T &_var) {
        StypeInfos *sTypeInfos_var(nullptr);
        getTypeInfos_byType(_var, &sTypeInfos_var);

        SnextSequence *sSeq(new SnextSequence(1));
        sSeq->typeSet(0, sTypeInfos_var->typeId());
        _updateSeqPtr_Set(sSeq);
    }

    void _setNextData_structvar_run(SnextSequence *_sSeq, const SvarInfo_base &_var) {
        if (_var.isInfoVec()) {
            //std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &vec = (*reinterpret_cast<std::vector<std::unique_ptr<Cserializing::SvarInfo_base>>>(_var.ptrNode()));
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec = reinterpret_cast<std::vector<std::unique_ptr<Cserializing::SvarInfo_base>>*>(_var.ptrNode());
            
            for (int i(0); i < vec->size(); ++i)
                _setNextData_structvar_run(_sSeq, *vec->at(i).get());
        }
        else
            _var.set(_sSeq, this);
    }

    //void _setNextData_structvar_var(const SvarInfo_base &_var) {
    //    const std::string strTypeName(_var.typeIdsNames()[0]->typeName());

    //    if (strTypeName == "std::string") {
    //        //const std::string &str(reinterpret_cast<std::string &>(*reinterpret_cast<std::string *>(_var.ptr())));
    //        //_setNextData_write(str);
    //        _var.set(this);
    //    }
    //    else if (strTypeName == "std::vector") {
    //        //_var.populate(this);
    //        _var.set(this);

    //        //using t = decltype(_var.typeIdsNames()[1]->type());

    //        //std::vector<t> *vec(reinterpret_cast<std::vector<t>*>(_var.ptr()));
    //        //_setNextData_write(*vec);
    //    }
    //    else if(strTypeName == "std::map") {
    //        _var.set(this);
    //    }
    //    else {
    //        _var.set(this);
    //        /*dataCount_t ui16Count(_var.count());
    //        int iDataSize(_var.typeIdsNames()[0]->size() * ui16Count);
    //        _reserveBufferSize(sizeof(ui16Count) + iDataSize);

    //        ::memcpy(&m_ptrSequenceCur->m_ui8ArrBuffer[m_iIndex], &ui16Count, sizeof(ui16Count));
    //        m_iIndex += sizeof(ui16Count);

    //        if (ui16Count > 0) {
    //            ::memcpy(&m_ptrSequenceCur->m_ui8ArrBuffer[m_iIndex], _var.ptr(), iDataSize);
    //            m_iIndex += iDataSize;
    //        }*/
    //    }
    //}


    SnextSequence *m_ptrSeqBeg = nullptr,
                  *m_ptrSeqCur = nullptr,
                  *m_ptrSeqEnd = nullptr;
    std::mutex     m_mtx;

    static mapTypesInfos_byName s_mapTypes_byName;
    static mapTypesInfos_byId   s_mapTypes_byId;
    static std::atomic<bool>    s_bRegisterDone;
};


#define TEST2(VAR)  Cserializing::SvarInfo<decltype(VAR)>(VAR)
#define TEST(...)   friend class Cserializing; protected: std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v { Cserializing::__v_push(##__VA_ARGS__) };