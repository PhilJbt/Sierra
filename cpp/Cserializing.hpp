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
typedef uint16_t ChunkLength_t;

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

        dataCount_t                                                count()     const { return m_ui16Count;      }
        std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *ptrNode()   const { return m_ptrNode;        }
        bool                                                       isInfoVec() const { return m_bIsVarInfosVec; }

        virtual int calcsize(Cserializing *_ptrSerializer) const { return 0; }
        virtual void write(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) const {}
        virtual void read(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) const {}

    protected:
        dataCount_t                                                m_ui16Count      = 0;
        std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *m_ptrNode        = nullptr;
        bool                                                       m_bIsVarInfosVec = false;
    };

    template <typename T>
    struct SvarInfo : public SvarInfo_base {
    public:
        SvarInfo(T &_var) {
            if (!_storeNode(_var))
                _storeVar(_var);
        }

        int calcsize(Cserializing *_ptrSerializer) const {
            return _ptrSerializer->_setNextData_calculateSize(*m_ptrVar);
        }

        void write(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) const {
            _ptrSerializer->_setNextData_write(_sSeq, _iReadOffset, *m_ptrVar);
        }

        void read(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) const {
            _ptrSerializer->_getNextData_retrieve(_sSeq, _iReadOffset, *m_ptrVar);
        }

    private:
        template <typename T>
        bool _storeNode(T &_var) {
            constexpr bool has_varInfos = requires(T & _var) {
                _var.__v;
            };

            if constexpr (has_varInfos) {
                m_ptrNode = &_var.__v;
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
        friend Cserializing;
    public:
        SnextSequence() {}

        ~SnextSequence() {
            if (m_ui8ArrBuffer)
                delete[] m_ui8ArrBuffer;
        }

        typeId_t getChunkId(int &_iOffset) {
            typeId_t ui8ChunkId(0);
            uint32_t ui32DataSize(sizeof(typeId_t));
            ::memcpy(&ui8ChunkId, &m_ui8ArrBuffer[_iOffset], ui32DataSize);
            return ui8ChunkId;
        }

        void setChunkId(typeId_t _ui8ChunkId, int &_iOffset) {
            uint32_t ui32DataSize(sizeof(typeId_t));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_ui8ChunkId, ui32DataSize);
            _iOffset += ui32DataSize;
        }

        void allocChunkMemory(int _iWeight, int &_iOffset) {
            m_ui8ArrBuffer = new uint8_t[_iWeight];
            ::memset(m_ui8ArrBuffer, 0, _iWeight);
            m_ui16ChunkLen = static_cast<ChunkLength_t>(_iWeight);
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

        /*
        ** READDATA
        */
        template <typename T>
        void readData(T &_var, int &_iOffset) {
            uint32_t ui32DataSize(sizeof(_var));
            ::memcpy((void*)&_var, &m_ui8ArrBuffer[_iOffset], ui32DataSize);
            _iOffset += ui32DataSize;
        }

        void readData(std::string &_str, int &_iOffset) {
            _str.clear();
            _str = std::string(reinterpret_cast<char*>(&m_ui8ArrBuffer[_iOffset]));
            _iOffset += static_cast<int>(_str.length()) + 1;
        }

        template<typename T, size_t N>
        void readData(T(&_arr)[N], int &_iOffset) {
            dataCount_t ui16ArrLen(readCount(_iOffset));

            uint32_t ui32DataSize(static_cast<uint32_t>(ui16ArrLen) * sizeof(T));
            ::memset(&_arr[0], 0,                         N);
            ::memcpy(&_arr[0], &m_ui8ArrBuffer[_iOffset], ui32DataSize);
            _iOffset += ui32DataSize;
        }

        /*
        ** WRITEDATA
        */
        template <typename T>
        void writeData(const T &_var, int &_iOffset) {
            uint32_t ui32DataSize(sizeof(_var));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_var, ui32DataSize);
            _iOffset += ui32DataSize;
        }

        void writeData(const std::string &_str, int &_iOffset) {
            std::string str(_str + '\0');
            uint32_t ui32DataSize(static_cast<uint32_t>(str.length()));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], str.data(), ui32DataSize);
            _iOffset += ui32DataSize;
        }

        template<typename T, size_t N>
        void writeData(T(&_arr)[N], int &_iOffset) {
            writeCount(static_cast<dataCount_t>(N), _iOffset);

            uint32_t ui32DataSize(static_cast<uint32_t>(N) * sizeof(T));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_arr[0], ui32DataSize);
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


        void serialize(uint8_t **_ui8Buffer, int &_iIndex) {
            size_t iSizeof(sizeof(ChunkLength_t));
            ::memcpy((*_ui8Buffer) + _iIndex, &m_ui16ChunkLen, iSizeof);
            _iIndex += iSizeof;

            int iChunkLen(getChunkLen());
            ::memcpy((*_ui8Buffer) + _iIndex, m_ui8ArrBuffer, iChunkLen);
            _iIndex += iChunkLen;
        }

        void unserialize(uint8_t **_ui8Buffer, int &_iIndex) {
            typeSize_t ui16ChunkLen(0);

            int iChunkLenSizeof(sizeof(ChunkLength_t));

            ::memcpy(&ui16ChunkLen, (*_ui8Buffer) + _iIndex, iChunkLenSizeof);
            _iIndex += iChunkLenSizeof;


            int iChunkLen(static_cast<int>(ui16ChunkLen));

            m_ui8ArrBuffer = new uint8_t[iChunkLen];
            ::memset(m_ui8ArrBuffer, 0, iChunkLen);
            ::memcpy(m_ui8ArrBuffer, (*_ui8Buffer) + _iIndex, iChunkLen);
            _iIndex += iChunkLen;
        }

    private:
        void chunkIdPassed(int &_iOffset) {
            _iOffset += sizeof(typeId_t);
        }

        int getChunkLen() {
            return static_cast<int>(m_ui16ChunkLen);
        }

        int getElementCount() {
            dataCount_t ui16Count(0);
            ::memcpy(&ui16Count, m_ui8ArrBuffer + sizeof(ChunkLength_t) + sizeof(typeId_t), sizeof(dataCount_t));
            return static_cast<int>(ui16Count);
        }


        uint8_t       *m_ui8ArrBuffer  = nullptr;
        ChunkLength_t  m_ui16ChunkLen  = 0;
        SnextSequence *m_ptrNext       = nullptr;
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
        std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v;

        (__v_push_var(__v, args), ...);

        return __v;
    }


    void _updateSeqPtr_Set(SnextSequence *_ptrSeq) {
        if (m_ptrSeqBeg == nullptr)
            m_ptrSeqBeg = _ptrSeq;
        if (m_ptrSeqCur)
            m_ptrSeqCur->nextNode(_ptrSeq);
        m_ptrSeqEnd = m_ptrSeqCur = _ptrSeq;
    }

    void _updateSeqPtr_Get() {
        if (m_ptrSeqCur)
            m_ptrSeqCur = m_ptrSeqCur->nextNode();
    }

    void changeTypeTo_Set() {
        m_ptrSeqCur = m_ptrSeqEnd;
    }

    void changeTypeTo_Get() {
        m_ptrSeqCur = m_ptrSeqBeg;
    }


    int nextDataCount() {
        int iOffset(sizeof(typeId_t));
        return static_cast<int>(m_ptrSeqCur->readCount(iOffset));
    }


    template <typename T> std::enable_if_t<!std::is_pointer_v<T>>
    setNextData(const typeId_t &_ui8TypeId, const T &_var) {
        setNextData(_ui8TypeId, &_var, 1);
    }

    template <typename T> std::enable_if_t<std::is_pointer_v<T>>
    setNextData(const typeId_t &_ui8TypeId, const T _var, const dataCount_t &_ui16Count = 1) {
        constexpr bool hasVarInfos = requires(T _var) {
            (&_var)[0]->__v;
        };

        int iWriteCursor(0);
        SnextSequence *sSeq(new SnextSequence());
        _updateSeqPtr_Set(sSeq);

        if constexpr (hasVarInfos) {
            m_ptrSeqCur->allocChunkMemory(sizeof(typeId_t) + sizeof(dataCount_t), iWriteCursor);
            m_ptrSeqCur->setChunkId(_ui8TypeId, iWriteCursor);
            m_ptrSeqCur->writeCount(static_cast<dataCount_t>(_ui16Count), iWriteCursor);

            int iWeight(0);
            _setNextData_structvar_calcsize(_var[0].__v, iWeight);

            for (int i(0); i < _ui16Count; ++i)
                _setNextData_structvar_unfold(iWriteCursor, iWeight, _var[i].__v);
        }
        else {
            int iWeight(_setNextData_calculateSize(_var[0]) *_ui16Count);
            sSeq->allocChunkMemory(sizeof(typeId_t) + sizeof(dataCount_t) + iWeight, iWriteCursor);
            sSeq->setChunkId(_ui8TypeId, iWriteCursor);
            sSeq->writeCount(static_cast<dataCount_t>(_ui16Count), iWriteCursor);

            for (int i(0); i < _ui16Count; ++i)
                _setNextData_write(m_ptrSeqCur, iWriteCursor, _var[i]);
        }
    }

    template <typename T, size_t N> std::enable_if_t<!std::is_pointer_v<T>>
    setNextData(const typeId_t &_ui8TypeId, const T (&_arr)[N]) {
        setNextData(_ui8TypeId, &_arr[0], N);
    }


    template <typename T> std::enable_if_t<!std::is_pointer_v<T>>
    getNextData(const typeId_t &_ui8TypeId, T &_var) {
        getNextData(_ui8TypeId, &_var);
    }

    template <typename T> std::enable_if_t<std::is_pointer_v<T>>
    getNextData(const typeId_t &_ui8TypeId, T _var) {
        int iReadCursor(0);
        if (m_ptrSeqCur->getChunkId(iReadCursor) == _ui8TypeId) {
            m_ptrSeqCur->chunkIdPassed(iReadCursor);

            constexpr bool hasVarInfos = requires(T & _var) {
                (&_var)[0]->__v;
            };

            if constexpr (hasVarInfos) {
                dataCount_t ui16CountNbrStructs(m_ptrSeqCur->readCount(iReadCursor));
                _updateSeqPtr_Get();

                for (dataCount_t i(0); i < ui16CountNbrStructs; ++i)
                    _getNextData_structvar_unfold(iReadCursor, _var[i].__v);
            }
            else {
                dataCount_t ui16Count(m_ptrSeqCur->readCount(iReadCursor));
                for (int i(0); i < ui16Count; ++i)
                    _getNextData_retrieve(m_ptrSeqCur, iReadCursor, _var[i]);
            }
        }

        _updateSeqPtr_Get();
    }

    template <typename T, size_t N> std::enable_if_t<!std::is_pointer_v<T>>
    getNextData(const typeId_t &_ui8TypeId, T(&_arr)[N]) {
        getNextData(_ui8TypeId, &_arr[0]);
    }


    void serialize(uint8_t **_ptrEmptyBuffer, int &_iBufferLength, const bool &_bAllocateMemory) {
        if (_bAllocateMemory) {
            _iBufferLength = 0;
            SnextSequence *ptrSeq(m_ptrSeqBeg);

            while (ptrSeq) {
                _iBufferLength += sizeof(ChunkLength_t);
                _iBufferLength += ptrSeq->getChunkLen();
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
    /*
    ** GETNEXTDATA_RETRIEVE
    */
    template<typename T/*, typename std::enable_if<std::is_class<T>::value, bool> ::type = true*/>
    void _getNextData_retrieve(SnextSequence *_sSeq, int &_iReadOffset, T &_var) {
        _sSeq->readData(_var, _iReadOffset);
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
    _getNextData_retrieve(SnextSequence *_sSeq, int &_iReadOffset, std::tuple<T...> &_tuple) {
        return;
    }
    template <size_t I = 0, typename... T>
    typename std::enable_if<(I < sizeof...(T)), void>::type
        _getNextData_retrieve(SnextSequence *_sSeq, int &_iReadOffset, std::tuple<T...> &_tuple) {
        _getNextData_retrieve(_sSeq, _iReadOffset, std::get<I>(_tuple));
        _getNextData_retrieve<I + 1>(_sSeq, _iReadOffset, _tuple);
    }


    void _getNextData_structvar_unfold(int &_iReadOffset, const std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec) {
        _iReadOffset = 0;

        for (int j(0); j < _vec.size(); ++j)
            _getNextData_structvar_read(_iReadOffset, *_vec[j].get());

        _updateSeqPtr_Get();
    }


    void _getNextData_structvar_read(int &_iReadOffset, const SvarInfo_base &_var) {
        if (_var.isInfoVec()) {
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec(_var.ptrNode());

            for (int i(0); i < vec->size(); ++i)
                _getNextData_structvar_read(_iReadOffset, *vec->at(i).get());
        }
        else
            _var.read(m_ptrSeqCur, _iReadOffset, this);
    }


    /*
    ** SETNEXTDATA_CALCULATESIZE
    */
    template <typename T>
    int _setNextData_calculateSize(const T &_var) {
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
    _setNextData_calculateSize(const std::tuple<T...> &_tuple) {
        return 0;
    }
    template <size_t I = 0, typename... T>
    typename std::enable_if<(I < sizeof...(T)), int>::type
    _setNextData_calculateSize(const std::tuple<T...> &_tuple) {
        int iWeight(_setNextData_calculateSize(std::get<I>(_tuple)));
        iWeight += _setNextData_calculateSize<I + 1>(_tuple);

        return iWeight;
    }

    int _setNextData_calculateSize(const std::string &_str) {
        int iWeight(static_cast<int>(_str.length()) + 1);

        return iWeight;
    }

    template<typename T, size_t N>
    int _setNextData_calculateSize(T(&_arr)[N]) {
        return (sizeof(dataCount_t) + (sizeof(T) * N));
    }


    /*
    ** SETNEXTDATA_WRITE
    */
    template <typename T>
    void _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const T &_var) {
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
    _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const std::tuple<T...> &_tuple) {
        return;
    }
    template <size_t I = 0, typename... T>
    typename std::enable_if<(I < sizeof...(T)), void>::type
        _setNextData_write(SnextSequence *_sSeq, int &_iWriteCursor, const std::tuple<T...> &_tuple) {
        _setNextData_write(_sSeq, _iWriteCursor, std::get<I>(_tuple));
        _setNextData_write<I + 1>(_sSeq, _iWriteCursor, _tuple);
    }


    void _setNextData_structvar_calcsize(const std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec, int &_iWeight) {
        for (int j(0); j < _vec.size(); ++j)
            _iWeight += _setNextData_structvar_calcsize_unfold(_vec[j].get());
    }

    void _setNextData_structvar_unfold(int &_iWriteCursor, const int &_iWeight, const std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec) {
        _setNextData_structvar_init(_iWriteCursor, _iWeight);

        for (int i(0); i < _vec.size(); ++i)
            _setNextData_structvar_write(_iWriteCursor, _vec[i].get());
    }


    void _setNextData_structvar_init(int &_iWriteCursor, const int &_iWeight) {
        _iWriteCursor = 0;
        SnextSequence *sSeq(new SnextSequence());
        sSeq->allocChunkMemory(_iWeight, _iWriteCursor);
        _updateSeqPtr_Set(sSeq);
    }

    int _setNextData_structvar_calcsize_unfold(const SvarInfo_base *_var) {
        int iWeight(0);

        if (_var->isInfoVec()) {
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec = _var->ptrNode();

            for (int i(0); i < vec->size(); ++i)
                iWeight += _setNextData_structvar_calcsize_unfold(vec->at(i).get());
        }
        else
            iWeight += _var->calcsize(this);

        return iWeight;
    }

    void _setNextData_structvar_write(int &_iWriteCursor, const SvarInfo_base *_var) {
        if (_var->isInfoVec()) {
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec = _var->ptrNode();
            
            for (int i(0); i < vec->size(); ++i)
                _setNextData_structvar_write(_iWriteCursor, vec->at(i).get());
        }
        else
            _var->write(m_ptrSeqCur, _iWriteCursor, this);
    }



    SnextSequence *m_ptrSeqBeg = nullptr,
                  *m_ptrSeqCur = nullptr,
                  *m_ptrSeqEnd = nullptr;
    std::mutex     m_mtx;
};


#define TEST(...)   friend Cserializing; protected: std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v { Cserializing::__v_push(##__VA_ARGS__) };