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

        dataCount_t  count()     const { return m_ui16Count;      }
        void        *ptrNode()   const { return m_ptrNode;        }
        bool         isInfoVec() const { return m_bIsVarInfosVec; }

        virtual void set(SnextSequence *_sSeq, Cserializing *_ptrSerializer) const {}
        virtual void get(SnextSequence *_sSeq, Cserializing *_ptrSerializer) const {}

    protected:
        dataCount_t  m_ui16Count      = 0;
        void        *m_ptrNode        = nullptr;
        bool         m_bIsVarInfosVec = false;
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
            _ptrSerializer->_getNextData_retrieve(_sSeq, _iReadOffset , * m_ptrVar);
        }

        void get(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) const {
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
            m_ui16Count = 1;
            m_ptrVar = &_var;
        }

        template <typename T, size_t N>
        void _storeVar(T(&_var)[N]) {
            m_ui16Count = N;
            m_ptrVar = &_var;
        }

        
        void _storeVar(std::string &_str) {
            m_ui16Count = static_cast<dataCount_t>(_str.length());
            m_ptrVar = &_str;
        }

        template <typename U>
        void _storeVar(std::vector<U> &_vec) {
            m_ui16Count = static_cast<dataCount_t>(_vec.size());
            m_ptrVar = &_vec;
        }

        T *m_ptrVar = nullptr;
    };

    struct SnextSequence {
    public:
        SnextSequence() {}

        template <typename T>
        SnextSequence(T *_var, uint32_t _ui32DataSize, dataCount_t _ui16Count)
            : m_ui32DataSize(_ui32DataSize), m_ui16Count(_ui16Count) {
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
            uint32_t ui32DataSize(sizeof(typeId_t));
            ::memcpy(&ui8ChunkId, &m_ui8ArrBuffer[_iOffset], ui32DataSize);
            _iOffset += ui32DataSize;
            return ui8ChunkId;
        }

        void setChunkId(typeId_t _ui8ChunkId, int &_iOffset) {
            uint32_t ui32DataSize(sizeof(typeId_t));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_ui8ChunkId, ui32DataSize);
            _iOffset += ui32DataSize;
        }

        void setChunkSize(uint32_t _ui32Weight, int &_iOffset) {
            m_ui8ArrBuffer = new uint8_t[_ui32Weight];
            ::memset(m_ui8ArrBuffer, 0, _ui32Weight);
            m_ui32DataSize = _ui32Weight;
        }

        dataCount_t readCount(int &_iOffset) {
            dataCount_t ui16Count(0);
            uint32_t ui32DataSize(sizeof(dataCount_t));
            ::memcpy(&ui16Count, &m_ui8ArrBuffer[_iOffset], ui32DataSize);
            _iOffset += ui32DataSize;
            return ui16Count;
        }

        void writeCount(dataCount_t _ui16Count, int &_iOffset) {
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_ui16Count, sizeof(dataCount_t));
            _iOffset += sizeof(dataCount_t);
        }

        template <typename T>
        void readData(T &_var, int &_iOffset) {
            uint32_t ui32DataSize(sizeof(_var));
            ::memcpy(&_var, &m_ui8ArrBuffer[_iOffset], ui32DataSize);
            _iOffset += ui32DataSize;
        }

        template <typename T>
        void writeData(T &_var, int &_iOffset) {
            uint32_t ui32DataSize(sizeof(_var));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_var, ui32DataSize);
            _iOffset += ui32DataSize;
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
                    NULL;
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

            (__v_push_var(__v, args), ...);
            
            return __v;
        }
        else
            return std::vector<std::unique_ptr<Cserializing::SvarInfo_base>>();
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


    void changeTypeTo_Set() {
        m_ptrSeqCur = m_ptrSeqEnd;
    }

    void changeTypeTo_Get() {
        m_ptrSeqCur = m_ptrSeqBeg;
    }


    int nextDataCount() {
        return m_ptrSeqCur->count();
    }


    template<typename T>
    void setNextData(const typeId_t &_ui8TypeId, const T &_var, int _iCount = 1, typename std::enable_if<!std::is_pointer<T>::value >::type* = 0) {
        int iWriteCursor(0);
        SnextSequence *sSeq(new SnextSequence());
        if (!_setNextData_structvar_(sSeq, _var)) {
            if (_iCount == 1) {
                int iWeight(_setNextData_calculateSize(_var));
                sSeq->setChunkSize(iWeight + sizeof(typeId_t), iWriteCursor);

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
    template<typename T>
    void _getNextData_retrieve(SnextSequence *_sSeq, int &_iReadOffset, T &_var, int _iCount = 1) {
        T var;
        _sSeq->readData(var, _iReadOffset);
        _var = var;
    }

    template<typename T, typename U>
    void _getNextData_retrieve(SnextSequence *_sSeq, int &_iReadOffset, std::pair<T, U> &_var) {
        _getNextData_retrieve(_sSeq, _iReadOffset, _var.first);
        _getNextData_retrieve(_sSeq, _iReadOffset, _var.second);
    }

    template<class T>
    void _getNextData_retrieve(SnextSequence *_sSeq, int &_iReadOffset, std::vector<T> &_vec) {
        int iCount(static_cast<int>(_sSeq->readCount(_iReadOffset)));

        _vec.resize(iCount);
        for (int i(0); i < iCount; ++i)
            _getNextData_retrieve(_sSeq, _iReadOffset, _vec[i]);
    }

    template <size_t I = 0, typename... T>
    typename std::enable_if<I == sizeof...(T), void>::type
    _getNextData_retrieve(SnextSequence *_sSeq, int &_iReadOffset, std::tuple<T&&...> &_tuple) {
        return;
    }
    template <size_t I = 0, typename... T>
    typename std::enable_if<(I < sizeof...(T)), void>::type
        _getNextData_retrieve(SnextSequence *_sSeq, int &_iReadOffset, std::tuple<T&&...> &_tuple) {
        _getNextData_retrieve(_sSeq, _iReadOffset, std::get<I>(_tuple));
        _getNextData_retrieve<I + 1>(_sSeq, _iReadOffset, _tuple);
    }


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
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec = reinterpret_cast<std::vector<std::unique_ptr<Cserializing::SvarInfo_base>>*>(_var.ptrNode());

            for (int i(0); i < vec->size(); ++i)
                _getNextData_structvar_run(_sSeq , *vec->at(i).get());
        }
        else
            _var.get(_sSeq, this);

        _updateSeqPtr_Get();
    }


    template <typename T>
    int _setNextData_calculateSize(const T &_var, const int &_iCount = 1) {
        return sizeof(T);
    }

    template<typename T, typename U>
    int _setNextData_calculateSize(const std::pair<T, U> &_pair) {
        int iWeight(0);

        iWeight += _setNextData_calculateSize(_pair.first);
        iWeight += _setNextData_calculateSize(_pair.second);

        return iWeight;
    }

    template<typename T>
    int _setNextData_calculateSize(const std::vector<T> &_vec) {
        int iWeight(sizeof(dataCount_t));

        for (int i(0); i < _vec.size(); ++i)
            iWeight += _setNextData_calculateSize(_vec[i]);

        return iWeight;
    }

    template <size_t I = 0, typename... T>
    typename std::enable_if<I == sizeof...(T), int>::type
    _setNextData_calculateSize(const std::tuple<T&&...> &_tuple) {
        return 0;
    }
    template <size_t I = 0, typename... T>
    typename std::enable_if<(I < sizeof...(T)), int>::type
    _setNextData_calculateSize(const std::tuple<T&&...> &_tuple) {
        int iWeight(_setNextData_calculateSize(std::get<I>(_tuple)));
        iWeight += _setNextData_calculateSize<I + 1>(_tuple);

        return iWeight;
    }


    template <typename T>
    void _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const T &_var, const int &_iCount = 1) {
        _sSeq->writeData(_var, _iWriteCursor);
    }

    template<typename T, typename U>
    void _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const std::pair<T, U> &_pair) {
        _setNextData_write(_sSeq, _iWriteCursor, _pair.first);
        _setNextData_write(_sSeq, _iWriteCursor, _pair.second);
    }

    template<typename T>
    void _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const std::vector<T> &_vec) {
        _sSeq->writeCount(static_cast<dataCount_t>(_vec.size()), _iWriteCursor);

        for (int i(0); i < _vec.size(); ++i)
            _setNextData_write(_sSeq, _iWriteCursor, _vec[i]);
    }

    template <size_t I = 0, typename... T>
    typename std::enable_if<I == sizeof...(T), void>::type
        _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const std::tuple<T&&...> &_tuple) {
        return;
    }
    template <size_t I = 0, typename... T>
    typename std::enable_if<(I < sizeof...(T)), void>::type
        _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const std::tuple<T&&...> &_tuple) {
        _setNextData_write(_sSeq, _iWriteCursor, std::get<I>(_tuple));
        _setNextData_write<I + 1>(_sSeq, _iWriteCursor, _tuple);
    }


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
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec = reinterpret_cast<std::vector<std::unique_ptr<Cserializing::SvarInfo_base>>*>(_var.ptrNode());
            
            for (int i(0); i < vec->size(); ++i)
                _setNextData_structvar_run(_sSeq, *vec->at(i).get());
        }
        else
            _var.set(_sSeq, this);
    }



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