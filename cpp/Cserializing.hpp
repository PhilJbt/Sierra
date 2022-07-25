#pragma once

#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <mutex>
#include <vector>
#include <algorithm> 
#include <map>
#include <unordered_map>
#include <numeric>
#include <concepts>
#include <tuple>
#include <array>


typedef uint8_t  typeId_t;
typedef uint32_t typeSize_t;
typedef uint16_t elemCnt_t;
typedef uint16_t ChunkLen_t;

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
            _ptrSerializer->_setNextData_write_(this, m_val);
        }

        T *val() { return m_val; }
    private:
        T *m_val;
    };

    struct SvarInfo_base {
    public:
        virtual ~SvarInfo_base() {}

        elemCnt_t                                                count()     const { return m_ui16ElemCount; }
        std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *ptrNode()   const { return m_ptrNode; }
        bool                                                       isInfoVec() const { return m_bIsVarInfosVec; }

        virtual int calcsize(Cserializing *_ptrSerializer, const elemCnt_t &_ui16ElemCount) const { return 0; }
        virtual void write(int &_iReadOffset, SnextSequence *_sSeq, Cserializing *_ptrSerializer, const elemCnt_t &_ui16ElemCount) const {}
        virtual void read(int &_iReadOffset, SnextSequence *_sSeq, Cserializing *_ptrSerializer, const elemCnt_t &_ui16ElemCount) const {}

    protected:
        elemCnt_t                                                m_ui16ElemCount = 0;
        std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *m_ptrNode = nullptr;
        bool                                                       m_bIsVarInfosVec = false;
    };

    template <typename T>
    struct SvarInfo : public SvarInfo_base {
    public:
        SvarInfo(T &_var) {
            if (!_storeNode(_var))
                _storeVar(_var);
        }

        int calcsize(Cserializing *_ptrSerializer, const elemCnt_t &_ui16ElemCount) const {
            return _ptrSerializer->_setNextData_calcSize(m_ptrVar, _ui16ElemCount, true);
        }

        void write(int &_iReadOffset, SnextSequence *_sSeq, Cserializing *_ptrSerializer, const elemCnt_t &_ui16ElemCount) {
            _ptrSerializer->_setNextData_write_(_iReadOffset, m_ptrVar, _ui16ElemCount);
        }

        void read(int &_iReadOffset, SnextSequence *_sSeq, Cserializing *_ptrSerializer, const elemCnt_t &_ui16ElemCount) {
            _ptrSerializer->_getNextData_read_(_iReadOffset, m_ptrVar, _ui16ElemCount);
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
            m_ui16ElemCount = 1;
            m_ptrVar = &_var;
        }

        template <typename T, size_t N>
        void _storeVar(T(&_var)[N]) {
            m_ui16ElemCount = N;
            m_ptrVar = &_var;
        }


        void _storeVar(std::string &_str) {
            m_ui16ElemCount = static_cast<elemCnt_t>(_str.length());
            m_ptrVar = &_str;
        }

        template <typename U>
        void _storeVar(std::vector<U> &_vec) {
            m_ui16ElemCount = static_cast<elemCnt_t>(_vec.size());
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

        void setChunkId(int &_iOffset, typeId_t _ui8ChunkId) {
            const uint32_t ui32DataSize(sizeof(typeId_t));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_ui8ChunkId, ui32DataSize);
            _iOffset += ui32DataSize;
        }

        void allocChunkMemory(int &_iOffset, int _iWeight) {
            m_ui8ArrBuffer = new uint8_t[_iWeight];
            ::memset(m_ui8ArrBuffer, 0, _iWeight);
            m_ui16ChunkLen = static_cast<ChunkLen_t>(_iWeight);
        }

        elemCnt_t readCount(int &_iOffset) {
            elemCnt_t ui16Count(0);
            uint32_t ui32DataSize(sizeof(elemCnt_t));
            ::memcpy(&ui16Count, &m_ui8ArrBuffer[_iOffset], ui32DataSize);
            _iOffset += ui32DataSize;
            return ui16Count;
        }

        void writeCount(int &_iOffset, elemCnt_t _ui16ElemCount) {
            const size_t ui32DataSize(sizeof(elemCnt_t));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_ui16ElemCount, ui32DataSize);
            _iOffset += ui32DataSize;
        }

        /*
        ** READDATA
        */
        template <typename T>
        void readData(int &_iOffset, T *_var, const elemCnt_t &_ui16ElemCount) {
            const elemCnt_t ui16DataSize(Cserializing::_setNextData_calcSize(_var, _ui16ElemCount, false));

            ::memcpy(_var, &m_ui8ArrBuffer[_iOffset], ui16DataSize);
            _iOffset += ui16DataSize;
        }

        /*void readData(std::string &_str, int &_iReadOffset) {
            const elemCnt_t ui16BytesLen(readCount(_iReadOffset));

            _str.resize(ui16BytesLen / sizeof(_str[0]));
            ::memcpy(&_str[0], &m_ui8ArrBuffer[_iReadOffset], ui16BytesLen);
            _iReadOffset += ui16BytesLen;
        }

        void readData(std::u32string &_32str, int &_iReadOffset) {
            const elemCnt_t ui16BytesLen(readCount(_iReadOffset));

            _32str.resize(ui16BytesLen / sizeof(_32str[0]));
            ::memcpy(&_32str[0], &m_ui8ArrBuffer[_iReadOffset], ui16BytesLen);
            _iReadOffset += ui16BytesLen;
        }

        template<typename T>
        void readData(T *_arr, int &_iReadOffset, elemCnt_t _ui16ElemCount) {
            const elemCnt_t ui16ArrLen(std::min(readCount(_iReadOffset), _ui16ElemCount));

            ::memset(&_arr[0], 0, ui16ArrLen);
            ::memcpy(&_arr[0], &m_ui8ArrBuffer[_iReadOffset], ui16ArrLen);
            _iReadOffset += ui16ArrLen;
        }*/

        /*
        ** WRITEDATA
        */
        template <typename T>
        void writeData(int &_iOffset, const T * const _var, const elemCnt_t &_ui16ElemCount) {
            const elemCnt_t ui16DataSize(Cserializing::_setNextData_calcSize(_var, _ui16ElemCount, false));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], _var, ui16DataSize);
            _iOffset += ui16DataSize;
        }

        /*void writeData(const std::string &_str, int &_iOffset) {
            const elemCnt_t ui16DataSize(Cserializing::_setNextData_calcSize(_str, false));
            writeCount(ui16DataSize, _iOffset);

            ::memcpy(&m_ui8ArrBuffer[_iOffset], _str.data(), ui16DataSize);
            _iOffset += ui16DataSize;
        }

        void writeData(const std::u32string &_32str, int &_iOffset) {
            const elemCnt_t ui16DataSize(Cserializing::_setNextData_calcSize(_32str, false));
            writeCount(ui16DataSize, _iOffset);

            ::memcpy(&m_ui8ArrBuffer[_iOffset], _32str.data(), ui16DataSize);
            _iOffset += ui16DataSize;
        }

        void writeData(const uint8_t *_ptr, int &_iOffset, const elemCnt_t &_ui16ElemCount) {
            ::memcpy(&m_ui8ArrBuffer[_iOffset], _ptr, _ui16ElemCount);
            _iOffset += _ui16ElemCount;
        }

        template<typename T>
        void writeData(const T * const _arr, int &_iOffset, const elemCnt_t &_ui16ElemCount) {
            writeCount(_ui16ElemCount, _iOffset);

            const elemCnt_t ui16DataSize(Cserializing::_setNextData_calcSize(_arr, false));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_arr[0], ui16DataSize);
            _iOffset += ui16DataSize;
        }*/

        void nextNode(SnextSequence *_ptrNext) {
            m_ptrNext = _ptrNext;
        }

        SnextSequence *nextNode() {
            return m_ptrNext;
        }

        uint8_t *buffer(uint8_t _ui8Offset = 0) {
            return (m_ui8ArrBuffer + _ui8Offset);
        }


        void serialize(uint8_t **_ui8Buffer, int &_iIndex) {
            const size_t iSizeof(sizeof(ChunkLen_t));
            ::memcpy((*_ui8Buffer) + _iIndex, &m_ui16ChunkLen, iSizeof);
            _iIndex += static_cast<int>(iSizeof);

            const int iChunkLen(getChunkLen());
            ::memcpy((*_ui8Buffer) + _iIndex, m_ui8ArrBuffer, iChunkLen);
            _iIndex += iChunkLen;
        }

        void unserialize(uint8_t **_ui8Buffer, int &_iIndex) {
            typeSize_t ui16ChunkLen(0);

            const int iChunkLenSizeof(sizeof(ChunkLen_t));

            ::memcpy(&ui16ChunkLen, (*_ui8Buffer) + _iIndex, iChunkLenSizeof);
            _iIndex += iChunkLenSizeof;


            const int iChunkLen(static_cast<int>(ui16ChunkLen));

            m_ui8ArrBuffer = new uint8_t[iChunkLen];
            ::memset(m_ui8ArrBuffer, 0, iChunkLen);
            ::memcpy(m_ui8ArrBuffer, (*_ui8Buffer) + _iIndex, iChunkLen);
            _iIndex += iChunkLen;
        }

    private:
        void chunkIdPassed(int &_iReadOffset) {
            _iReadOffset += sizeof(typeId_t);
        }

        int getChunkLen() {
            return static_cast<int>(m_ui16ChunkLen);
        }

        int getElementCount() {
            elemCnt_t ui16Count(0);
            ::memcpy(&ui16Count, m_ui8ArrBuffer + sizeof(ChunkLen_t) + sizeof(typeId_t), sizeof(elemCnt_t));
            return static_cast<int>(ui16Count);
        }


        uint8_t *m_ui8ArrBuffer = nullptr;
        ChunkLen_t  m_ui16ChunkLen = 0;
        SnextSequence *m_ptrNext = nullptr;
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
    static void __INTERNAL__v_push_var(std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec, T &_var) {
        _vec.push_back(std::make_unique<Cserializing::SvarInfo<T>>(_var));
    }

    template <typename... T>
    static std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __INTERNAL__v_push(T&&... args) {
        std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v;

        (__INTERNAL__v_push_var(__v, args), ...);

        return __v;
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

    template<typename T, std::enable_if_t<!std::is_pointer_v<T> && !std::is_array_v<T>>* = nullptr>
    auto setNextData(const typeId_t &_ui8TypeId, const T &_var) {
        _setNextData_write(_ui8TypeId, &_var, 1);
    }

    template<typename T, std::enable_if_t<std::is_pointer_v<T> && !std::is_array_v<T>>* = nullptr>
    auto setNextData(const typeId_t &_ui8TypeId, const T _var, const elemCnt_t &_ui16ElemCount) {
        _setNextData_write(_ui8TypeId, _var, _ui16ElemCount);
    }


    template<typename T, std::enable_if_t<!std::is_pointer_v<T> && !std::is_array_v<T>>* = nullptr>
    auto getNextData(const typeId_t &_ui8TypeId, T &_var) {
        int iReadCursor(0);
        if (m_ptrSeqCur->getChunkId(iReadCursor) == _ui8TypeId) {
            m_ptrSeqCur->chunkIdPassed(iReadCursor);

            _getNextData_read(iReadCursor, &_var, 1);
        }
    }

    template<typename T, std::enable_if_t<std::is_pointer_v<T> && !std::is_array_v<T>>* = nullptr>
    auto getNextData(const typeId_t &_ui8TypeId, T _var, const elemCnt_t &_ui16ElemCount) {
        int iReadCursor(0);
        if (m_ptrSeqCur->getChunkId(iReadCursor) == _ui8TypeId) {
            m_ptrSeqCur->chunkIdPassed(iReadCursor);

            _getNextData_read(iReadCursor, _var, _ui16ElemCount);
        }
    }


    /*void serialize(uint8_t **_ptrEmptyBuffer, int &_iBufferLength, const bool &_bAllocateMemory) {
        if (_bAllocateMemory) {
            _iBufferLength = 0;
            SnextSequence *ptrSeq(m_ptrSeqBeg);

            while (ptrSeq) {
                _iBufferLength += sizeof(ChunkLen_t);
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
    }*/

private:
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

    /*
    ** GETNEXTDATA_RETRIEVE
    */

    // R ENTRY POINT
    template<typename T>
    void _getNextData_read(int &_iOffset, T * _var, const elemCnt_t &_ui16ElemCount) {
        _getNextData_read_(_iOffset, _var, _ui16ElemCount);

        _updateSeqPtr_Get();
    }

    // R BUFFER
    // ... ?

    // R * T
    // R * [] T
    template<typename T, std::enable_if_t<std::is_fundamental_v<T>>* = nullptr>
    auto _getNextData_read_(int &_iOffset, T * _var, const elemCnt_t &_ui16ElemCount) {
        elemCnt_t ui16Count(m_ptrSeqCur->readCount(_iOffset));
        size_t iSize(std::min(_ui16ElemCount, ui16Count));

        if (typeid(T) == typeid(bool)) {
            bool *bArrTemp(new bool[iSize]);
            _opti_bool_discat(_iOffset, ui16Count, bArrTemp);

            for (size_t i(0); i < iSize; ++i)
                _var[i] = bArrTemp[i];

            delete[] bArrTemp;
        }
        else 
            m_ptrSeqCur->readData(_iOffset, _var, static_cast<elemCnt_t>(iSize));
    }

    // R * pair<T, U>


    // R * tuple<T, U, ...>


    // R * [] pair<T, U>


    // R * [] tuple<T, ...>


    // R * vector<T>
    template<typename T>
    void _getNextData_read_(int &_iOffset, std::vector<T> *_vec, const elemCnt_t &_ui16ElemCount) {
        elemCnt_t ui16Count(m_ptrSeqCur->readCount(_iOffset));
        size_t iSize(std::min(_ui16ElemCount, ui16Count));

        for (size_t i(0); i < iSize; ++i)
            _getNextData_read_(_iOffset, _vec[i], _ui16ElemCount);
    }

    // R * vector<pair<T, U>>
    template<typename T, typename U>
    void _getNextData_read_(int &_iOffset, std::vector<std::pair<T, U>> *_vec, const elemCnt_t &_ui16ElemCount) {
        elemCnt_t ui16Count(m_ptrSeqCur->readCount(_iOffset));
        size_t iSize(std::min(_ui16ElemCount, ui16Count));

        _vec.resize(iSize);

        if (typeid(T) == typeid(bool)) {
            const int iCountBools(static_cast<int>(m_ptrSeqCur->readCount(_iOffset)));

            bool *bArrTemp(new bool[iCountBools]);
            _opti_bool_discat(_iOffset, iCountBools, bArrTemp);

            for (int i(0); i < iCountBools; ++i)
                _vec[i].first = bArrTemp[i];

            delete[] bArrTemp;
        }
        else
            for (size_t i(0); i < iSize; ++i)
                _getNextData_read_(_iOffset, _vec[i], _ui16ElemCount);

        if (typeid(U) == typeid(bool)) {
            const int iCountBools(static_cast<int>(m_ptrSeqCur->readCount(_iOffset)));

            bool *bArrTemp(new bool[iCountBools]);
            _opti_bool_discat(_iOffset, iCountBools, bArrTemp);

            for (int i(0); i < iCountBools; ++i)
                _vec[i].second = bArrTemp[i];

            delete[] bArrTemp;
        }
        else
            for (size_t i(0); i < iSize; ++i)
                _getNextData_read_(_iOffset, _vec[i], _ui16ElemCount);
    }

    // R * vector<tuple<T, ...>>



    /*void _getNextData_userstruct_unfold(int &_iReadOffset, const std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec) {
        _iReadOffset = 0;

        const size_t iSize(_vec.size());
        for (int j(0); j < iSize; ++j)
            _getNextData_userstruct_read(_iReadOffset, *_vec[j].get());

        _updateSeqPtr_Get();
    }

    void _getNextData_userstruct_read(int &_iReadOffset, const SvarInfo_base &_var) {
        if (_var.isInfoVec()) {
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec(_var.ptrNode());

            for (int i(0); i < vec->size(); ++i)
                _getNextData_userstruct_read(_iReadOffset, *vec->at(i).get());
        }
        else
            _var.read(m_ptrSeqCur, _iReadOffset, this);
    }*/


    /*
    ** SETNEXTDATA_CALCULATESIZE
    */
    template <typename T, std::enable_if_t<std::is_pointer_v<T>>* = nullptr>
    static auto _setNextData_calcSize(const T &_var, const elemCnt_t &_ui16ElemCount, const bool &_bIncludeSize) {
        return static_cast<int>(sizeof(*_var)) * static_cast<int>(_ui16ElemCount);
    }

    /*template<typename T, std::enable_if_t<std::is_pointer_v<T> && !std::is_same_v<T, bool*>>* = nullptr>
    static auto _setNextData_calcSize(const T _ptr, const bool &_bIncludeSize, const elemCnt_t &_ui16ElemCount) {
        int iWeight(_bIncludeSize ? sizeof(elemCnt_t) : 0);

        for (elemCnt_t i(0); i < _ui16ElemCount; ++i)
            iWeight += _setNextData_calcSize(_ptr[i], true);

        return iWeight;
    }

    template<typename T, std::enable_if_t<std::is_pointer_v<T> && std::is_same_v<T, bool*>>* = nullptr>
    static auto _setNextData_calcSize(const T _ptr, const bool &_bIncludeSize, const elemCnt_t &_ui16ElemCount) {
        int iWeight(_bIncludeSize ? sizeof(elemCnt_t) : 0);

        iWeight += _opti_bool_calcsize(_ui16ElemCount);

        return iWeight;
    }

    template <size_t N>
    static int _setNextData_calcSize(const bool(&_arr)[N], const bool &_bIncludeSize) {
        int iWeight(_bIncludeSize ? sizeof(elemCnt_t) : 0);

        iWeight += _opti_bool_calcsize(N);

        return iWeight;
    }

    template<typename T, typename U>
    static int _setNextData_calcSize(const std::pair<T, U> &_pair, const bool &_bIncludeSize) {
        int iWeight(0);

        iWeight += _setNextData_calcSize(_pair.first, true);
        iWeight += _setNextData_calcSize(_pair.second, true);

        return iWeight;
    }

    static int _setNextData_calcSize(const std::vector<bool> &_vec, const bool &_bIncludeSize) {
        int iWeight(_bIncludeSize ? sizeof(elemCnt_t) : 0);

        iWeight += _opti_bool_calcsize(_vec.size());

        return iWeight;
    }

    template<typename T, typename U, std::enable_if_t<!std::is_same_v<T, bool> && !std::is_same_v<U, bool>>* = nullptr>
    static auto _setNextData_calcSize(const std::vector<std::pair<T, U>> &_vec, const bool &_bIncludeSize) {
        int iWeight(_bIncludeSize ? sizeof(elemCnt_t) : 0);

        const size_t iSize(_vec.size());
        for (int i(0); i < iSize; ++i) {
            iWeight += _setNextData_calcSize(_vec[i].first, true);
            iWeight += _setNextData_calcSize(_vec[i].second, true);
        }

        return iWeight;
    }
    
    template<typename T, typename U>
        static int _setNextData_calcSize(const std::vector<std::pair<T, U>> &_vec, const bool &_bIncludeSize) {
        const size_t iSize(sizeof(elemCnt_t));
        int iWeight(_bIncludeSize ? iSize : 0);

        const size_t iSize(_vec.size());
        if (typeid(T) == typeid(bool)) {
            iWeight += iSize;
            iWeight += _opti_bool_calcsize(iSize);
            for (int i(0); i < iSize; ++i)
                iWeight += _setNextData_calcSize(_vec[i].second, true);
        }
        
        if (typeid(U) == typeid(bool)) {
            iWeight += iSize;
            iWeight += _opti_bool_calcsize(iSize);
            for (int i(0); i < iSize; ++i)
                iWeight += _setNextData_calcSize(_vec[i].first, true);
        }

        return iWeight;
    }

    template<typename T>
    static int _setNextData_calcSize(const std::vector<T> &_vec, const bool &_bIncludeSize) {
        int iWeight(_bIncludeSize ? sizeof(elemCnt_t) : 0);

        const size_t iSize(_vec.size());
        for (int i(0); i < iSize; ++i)
            iWeight += _setNextData_calcSize(_vec[i], true);

        return iWeight;
    }

    template <size_t I = 0, typename... T>
    static typename std::enable_if<I == sizeof...(T), int>::type
        _setNextData_calcSize(const std::tuple<T...> &_tuple, const bool &_bIncludeSize) {
        return 0;
    }
    template <size_t I = 0, typename... T>
    static typename std::enable_if<(I < sizeof...(T)), int>::type
        _setNextData_calcSize(const std::tuple<T...> &_tuple, const bool &_bIncludeSize) {
        int iWeight(_setNextData_calcSize(std::get<I>(_tuple), true));
        iWeight += _setNextData_calcSize<I + 1>(_tuple, true);

        return iWeight;
    }

    static int _setNextData_calcSize(const std::string &_str, const bool &_bIncludeSize) {
        int iWeight(static_cast<uint32_t>(_str.length()) * static_cast<uint32_t>(sizeof(_str[0])));

        if (_bIncludeSize)
            iWeight += sizeof(elemCnt_t);

        return iWeight;
    }

    static int _setNextData_calcSize(const std::u32string &_32str, const bool &_bIncludeSize) {
        int iWeight(static_cast<uint32_t>(_32str.length()) * static_cast<uint32_t>(sizeof(_32str[0])));

        if (_bIncludeSize)
            iWeight += sizeof(elemCnt_t);

        return iWeight;
    }*/


    /*
    ** SETNEXTDATA_WRITE
    */

    // W ENTRY POINT
    template<typename T>
    void _setNextData_write(const typeId_t &_ui8TypeId, const T * const _var, const elemCnt_t &_ui16ElemCount) {
        int iWriteCursor(0),
            iSize(_setNextData_calcSize(_var, _ui16ElemCount, true));

        _setNextData_initSeq(_ui8TypeId, static_cast<elemCnt_t>(iSize), iWriteCursor);

        if (_setNextData_write_userstruct_check(_var, _ui16ElemCount))
            _setNextData_write_userstruct(iWriteCursor, _var, _ui16ElemCount);
        else
            _setNextData_write_(iWriteCursor, _var, _ui16ElemCount);
    }


    // W CHECK USERSTRUCT
    template<typename T>
    bool _setNextData_write_userstruct_check(const T * const _var, const elemCnt_t &_ui16ElemCount) {
        constexpr bool hasVarInfos = requires(const T * const _var) {
            _var->__v;
        };

        if constexpr (hasVarInfos)
            return true;
        else
            return false;
    }

    // W PROCESS USERSTRUCT
    template<typename T>
    void _setNextData_write_userstruct(const int &_iOffset, const T *const _var, const elemCnt_t &_ui16ElemCount) {
        //_setNextData_userstruct_unfold(_iOffset, iWeight, _var->__v);
    }


    // W BUFFER
    void _setNextData_write_rawdata(int &_iOffset, const uint8_t *const _ptr, const elemCnt_t &_ui16DataLen, const elemCnt_t &_ui16ElemCount) {
        m_ptrSeqCur->writeCount(_iOffset, _ui16ElemCount);
        m_ptrSeqCur->writeData(_iOffset, _ptr, _ui16DataLen);
    }

    // W * T
    // W * [] T
    template<typename T>
        requires (std::is_fundamental<T>::value)
    void _setNextData_write_(int &_iOffset, const T * const _var, const elemCnt_t &_ui16ElemCount) {
        if (typeid(T) == typeid(const bool))
            _opti_bool_concat(_iOffset, this, _ui16ElemCount, reinterpret_cast<const bool *>(_var));
        else {
            if (_ui16ElemCount > 0)
                m_ptrSeqCur->writeCount(_iOffset, _ui16ElemCount);
            m_ptrSeqCur->writeData(_iOffset, &(*_var), _ui16ElemCount);
        }
    }

    template<typename T>
        requires (!std::is_fundamental<T>::value)
    void _setNextData_write_(int &_iOffset, const T * const _var, const elemCnt_t &_ui16ElemCount) {
        for (elemCnt_t i(0); i < _ui16ElemCount; ++i)
            _setNextData_write_(_iOffset, &(_var[i]), 1);
    }

    // W * pair<T, U>


    // W * tuple<T, U, ...>


    // W * [] pair<T, U>


    // W * [] tuple<T, ...>


    // W * vector<T>
    template<typename T>
        requires (std::is_fundamental<T>::value)
    void _setNextData_write_(int &_iOffset, const std::vector<T> *const _vec, const elemCnt_t &_ui16ElemCount) {
        const size_t iSize(_vec->size());
        m_ptrSeqCur->writeCount(_iOffset, static_cast<elemCnt_t>(iSize));

        if (typeid(T) == typeid(const bool)) {
            bool *arrTemp(new bool[iSize]);
            ::memset(arrTemp, 0, iSize);
            for (size_t i(0); i < iSize; ++i)
                arrTemp[i] = ((*_vec)[i]);

            _opti_bool_concat(_iOffset, this, static_cast<elemCnt_t>(iSize), arrTemp);

            delete[] arrTemp;
        }
        else
            for (size_t i(0); i < iSize; ++i)
                _setNextData_write_(_iOffset, &((*_vec)[i]), 0);
    }

    template<typename T>
        requires (!std::is_fundamental<T>::value)
    void _setNextData_write_(int &_iOffset, const std::vector<T> *const _vec, const elemCnt_t &_ui16ElemCount) {
        const size_t iSize(_vec->size());
        m_ptrSeqCur->writeCount(_iOffset, static_cast<elemCnt_t>(iSize));

        for (size_t i(0); i < iSize; ++i)
            _setNextData_write_(_iOffset, &((*_vec)[i]), 0);
    }

    // W * vector<pair<T, U>>
    template<typename T, typename U>
    void _setNextData_write_(int &_iOffset, const std::vector<std::pair<T, U>> * const _vec, const elemCnt_t &_ui16ElemCount) {
        const size_t iSize(_vec->size());

        m_ptrSeqCur->writeCount(_iOffset, static_cast<elemCnt_t>(iSize));

        if (typeid(T) == typeid(bool)) {
            bool *ui8ArrRaw(new bool[iSize]);
            ::memset(ui8ArrRaw, 0, iSize);

            for (size_t i(0); i < iSize; ++i)
                ui8ArrRaw[i] = ((*_vec)[i]).first;

            _opti_bool_concat(_iOffset, this, static_cast<elemCnt_t>(iSize), ui8ArrRaw);

            delete[] ui8ArrRaw;
        }
        else
            for (int i(0); i < iSize; ++i)
                _setNextData_write_(_iOffset, &(((*_vec)[i]).first), 0);

        if (typeid(U) == typeid(bool)) {
            bool *ui8ArrRaw(new bool[iSize]);
            ::memset(ui8ArrRaw, 0, iSize);

            for (size_t i(0); i < iSize; ++i)
                ui8ArrRaw[i] = ((*_vec)[i]).second;

            _opti_bool_concat(_iOffset, this, static_cast<elemCnt_t>(iSize), ui8ArrRaw);

            delete[] ui8ArrRaw;
        }
        else
            for (size_t i(0); i < iSize; ++i)
                _setNextData_write_(_iOffset, &(((*_vec)[i]).second), 0);
    }

    // W * vector<tuple<T, ...>>



    // 
    void _setNextData_allocSeq() {
        SnextSequence *sSeq(new SnextSequence());
        _updateSeqPtr_Set(sSeq);
    }

    void _setNextData_initSeq(const typeId_t &_ui8TypeId, const elemCnt_t &_ui16ElemCount, int &_iOffset) {
        _setNextData_allocSeq();
        m_ptrSeqCur->allocChunkMemory(_iOffset, sizeof(typeId_t) + sizeof(elemCnt_t) + _ui16ElemCount);
        m_ptrSeqCur->setChunkId(_iOffset, _ui8TypeId);
    }

    /*void _setNextData_userstruct_calcsize(const std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec, int &_iWeight) {
        const size_t iSize(_vec.size());
        for (int j(0); j < iSize; ++j)
            _iWeight += _setNextData_userstruct_calcsize_unfold(_vec[j].get());
    }

    void _setNextData_userstruct_unfold(int &_iOffset, const int &_iWeight, const std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec) {
        _setNextData_userstruct_unfold_init(_iOffset, _iWeight);

        const size_t iSize(_vec.size());
        for (int i(0); i < iSize; ++i)
            _setNextData_userstruct_write(_iOffset, _vec[i].get());
    }

    void _setNextData_userstruct_unfold_init(int &_iOffset, const int &_iWeight) {
        _iOffset = 0;
        SnextSequence *sSeq(new SnextSequence());
        sSeq->allocChunkMemory(_iWeight, _iOffset);
        _updateSeqPtr_Set(sSeq);
    }

    int _setNextData_userstruct_calcsize_unfold(const SvarInfo_base *_var) {
        int iWeight(0);

        if (_var->isInfoVec()) {
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec = _var->ptrNode();

            for (int i(0); i < vec->size(); ++i)
                iWeight += _setNextData_userstruct_calcsize_unfold(vec->at(i).get());
        }
        else
            iWeight += _var->calcsize(this);

        return iWeight;
    }

    void _setNextData_userstruct_write(int &_iOffset, const SvarInfo_base *_var) {
        if (_var->isInfoVec()) {
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec = _var->ptrNode();
            
            for (int i(0); i < vec->size(); ++i)
                _setNextData_userstruct_write(_iOffset, vec->at(i).get());
        }
        else
            _var->write(m_ptrSeqCur, _iOffset, this);
    }*/

    /*
    ** OPTIMIZATIONS
    */
    static inline int _opti_bool_calcsize(const size_t &_iSize) {
        return static_cast<int>(::ceil(_iSize / 8.0f));
    }

    static void _opti_bool_concat(int &_iOffset, Cserializing *const _this, const elemCnt_t &_ui16ElemCount, const bool *_ui8ArrRaw) {
        const int iPackedBoolSize(_opti_bool_calcsize(_ui16ElemCount));

        uint8_t *ui8ArrCat(new uint8_t[iPackedBoolSize]);
        ::memset(ui8ArrCat, 0, iPackedBoolSize);

        _opti_bool_concat_(ui8ArrCat, _ui8ArrRaw, _ui16ElemCount);

        _this->_setNextData_write_rawdata(_iOffset, ui8ArrCat, static_cast<elemCnt_t>(iPackedBoolSize), _ui16ElemCount);

        delete[] ui8ArrCat;
    }

    static void _opti_bool_concat_(uint8_t *_dst, const bool *const _src, const size_t &_size) {
        for (int i(0), j(0); i < _size; ++j, i += 8) {
            int kLast(static_cast<int>(_size) - i);
            if (kLast > 8) kLast = 8;
            for (int k(0); k < kLast; ++k) {
                if (_src[i + k])
                    _dst[j] |= (1 << k);
            }
        }
    }

    void _opti_bool_discat(int &_iReadOffset, const elemCnt_t &_ui16ElemCount, bool *_bArrTemp) {
        const int iBoolStackNbr(_opti_bool_calcsize(_ui16ElemCount));

        ::memset(_bArrTemp, 0, _ui16ElemCount);

        _opti_bool_discat_(_iReadOffset, iBoolStackNbr, _ui16ElemCount, _bArrTemp);
    }

    void _opti_bool_discat_(int &_iReadOffset, const int &_iBoolStackNbr, const elemCnt_t &_ui16BoolNbr, bool *_bArr) {
        for (int i(0), j(0); j < _iBoolStackNbr; ++j, i += 8) {
            const int iBoolInThisStack(std::min(static_cast<int>(_ui16BoolNbr - i), 8));

            uint8_t ui8BoolStack(0);
            m_ptrSeqCur->readData(_iReadOffset, &ui8BoolStack, sizeof(uint8_t));

            for (int k(0); k < iBoolInThisStack; ++k) {
                uint8_t ui8Temp(ui8BoolStack);
                ui8Temp &= (1 << k);

                if (ui8Temp != 0)
                    _bArr[i + k] = true;
            }
        }
    }



    SnextSequence *m_ptrSeqBeg = nullptr,
                  *m_ptrSeqCur = nullptr,
                  *m_ptrSeqEnd = nullptr;
    std::mutex     m_mtx;
};


#define TEST(...)   friend Cserializing; protected: std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v { Cserializing::__INTERNAL__v_push(##__VA_ARGS__) };