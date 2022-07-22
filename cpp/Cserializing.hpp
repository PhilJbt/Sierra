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
#include <array>


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
            _ptrSerializer->_setNextData_write_(this, m_val);
        }

        T *val() { return m_val; }
    private:
        T *m_val;
    };

    struct SvarInfo_base {
    public:
        virtual ~SvarInfo_base() {}

        dataCount_t                                                count()     const { return m_ui16Count; }
        std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *ptrNode()   const { return m_ptrNode; }
        bool                                                       isInfoVec() const { return m_bIsVarInfosVec; }

        virtual int calcsize(Cserializing *_ptrSerializer) const { return 0; }
        virtual void write(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) const {}
        virtual void read(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) const {}

    protected:
        dataCount_t                                                m_ui16Count = 0;
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

        int calcsize(Cserializing *_ptrSerializer) const {
            return _ptrSerializer->_setNextData_calculateSize(*m_ptrVar, true);
        }

        void write(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) {
            _ptrSerializer->_setNextData_write_(_iReadOffset, *m_ptrVar);
        }

        void read(SnextSequence *_sSeq, int &_iReadOffset, Cserializing *_ptrSerializer) {
            _ptrSerializer->_getNextData_retrieve_(_iReadOffset, *m_ptrVar);
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
            uint32_t ui32DataSize(Cserializing::_setNextData_calculateSize(_var, false));
            ::memcpy((void *)&_var, &m_ui8ArrBuffer[_iOffset], ui32DataSize);
            _iOffset += ui32DataSize;
        }

        void readData(std::string &_str, int &_iOffset) {
            dataCount_t ui16BytesLen(readCount(_iOffset));

            _str.resize(ui16BytesLen / sizeof(_str[0]));
            ::memcpy(&_str[0], &m_ui8ArrBuffer[_iOffset], ui16BytesLen);
            _iOffset += ui16BytesLen;
        }

        void readData(std::u32string &_32str, int &_iOffset) {
            dataCount_t ui16BytesLen(readCount(_iOffset));

            _32str.resize(ui16BytesLen / sizeof(_32str[0]));
            ::memcpy(&_32str[0], &m_ui8ArrBuffer[_iOffset], ui16BytesLen);
            _iOffset += ui16BytesLen;
        }

        template<typename T, size_t N>
        void readData(T(&_arr)[N], int &_iOffset) {
            dataCount_t ui16ArrLen(readCount(_iOffset));

            uint32_t ui32DataSize(Cserializing::_setNextData_calculateSize(_arr, false));
            ::memset(&_arr[0], 0, N);
            ::memcpy(&_arr[0], &m_ui8ArrBuffer[_iOffset], ui32DataSize);
            _iOffset += ui32DataSize;
        }

        /*
        ** WRITEDATA
        */
        template <typename T>
        void writeData(const T &_var, int &_iOffset) {
            uint32_t ui32DataSize(Cserializing::_setNextData_calculateSize(_var, false));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_var, ui32DataSize);
            _iOffset += ui32DataSize;
        }

        void writeData(const std::string &_str, int &_iOffset) {
            uint32_t ui32DataSize(Cserializing::_setNextData_calculateSize(_str, false));
            writeCount(static_cast<dataCount_t>(ui32DataSize), _iOffset);

            ::memcpy(&m_ui8ArrBuffer[_iOffset], _str.data(), ui32DataSize);
            _iOffset += ui32DataSize;
        }

        void writeData(const std::u32string &_32str, int &_iOffset) {
            uint32_t ui32DataSize(Cserializing::_setNextData_calculateSize(_32str, false));
            writeCount(static_cast<dataCount_t>(ui32DataSize), _iOffset);

            ::memcpy(&m_ui8ArrBuffer[_iOffset], _32str.data(), ui32DataSize);
            _iOffset += ui32DataSize;
        }

        void writeData(const uint8_t *_ptr, int &_iOffset, const uint32_t &_ui32Size) {
            ::memcpy(&m_ui8ArrBuffer[_iOffset], _ptr, _ui32Size);
            _iOffset += _ui32Size;
        }

        template<typename T, size_t N>
        void writeData(T(&_arr)[N], int &_iOffset) {
            writeCount(static_cast<dataCount_t>(N), _iOffset);

            uint32_t ui32DataSize(Cserializing::_setNextData_calculateSize(_arr, false));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_arr[0], ui32DataSize);
            _iOffset += ui32DataSize;
        }

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
            const size_t iSizeof(sizeof(ChunkLength_t));
            ::memcpy((*_ui8Buffer) + _iIndex, &m_ui16ChunkLen, iSizeof);
            _iIndex += static_cast<int>(iSizeof);

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


        uint8_t *m_ui8ArrBuffer = nullptr;
        ChunkLength_t  m_ui16ChunkLen = 0;
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

    template<typename T, size_t N, std::enable_if_t<!std::is_pointer_v<T>>* = nullptr>
    auto setNextData(const typeId_t &_ui8TypeId, T(&_arr)[N]) {
        /*int iWriteCursor(0);
        _setNextData_allocSeq();

        _setNextData_initSeq(_ui8TypeId, sizeof(dataCount_t), iWriteCursor);
        m_ptrSeqCur->writeCount(N, iWriteCursor);

        for (int i(0); i < N; ++i) {
            int iWeight(0);
            _setNextData_structvar_calcsize((&_arr[i])->__v, iWeight);
            _setNextData_structvar_unfold(iWriteCursor, iWeight, (&_arr[i])->__v);
        }*/

        constexpr bool hasVarInfos = requires(T(&&_arr)[N]) {
            (&_arr[0])->__v;
        };

        int iWriteCursor(0);
        _setNextData_allocSeq();

        if constexpr (hasVarInfos) {
            _setNextData_initSeq(_ui8TypeId, sizeof(dataCount_t), iWriteCursor);
            m_ptrSeqCur->writeCount(N, iWriteCursor);

            for (int i(0); i < N; ++i) {
                int iWeight(0);
                _setNextData_structvar_calcsize((&_arr[i])->__v, iWeight);
                _setNextData_structvar_unfold(iWriteCursor, iWeight, (&_arr[i])->__v);
            }
        }
        else
            _setNextData_write(_ui8TypeId, _arr, iWriteCursor);
    }

    template<typename T, std::enable_if_t<!std::is_pointer_v<T>>* = nullptr>
    auto setNextData(const typeId_t &_ui8TypeId, const T &_var) {
        constexpr bool hasVarInfos = requires(T _var) {
            _var.__v;
        };

        int iWriteCursor(0);
        _setNextData_allocSeq();

        if constexpr (hasVarInfos) {
            _setNextData_initSeq(_ui8TypeId, 0, iWriteCursor);

            int iWeight(0);
            _setNextData_structvar_calcsize(_var.__v, iWeight);

            _setNextData_structvar_unfold(iWriteCursor, iWeight, _var.__v);
        }
        else
            _setNextData_write(_ui8TypeId, _var, iWriteCursor);
    }

    template<typename T, std::enable_if_t<std::is_pointer_v<T>>* = nullptr>
    auto setNextData(const typeId_t &_ui8TypeId, const T _var, const dataCount_t &_ui16Count) {
        constexpr bool hasVarInfos = requires(T _var) {
            (&_var[0])->__v;
        };

        int iWriteCursor(0);
        _setNextData_allocSeq();

        if constexpr (hasVarInfos) {
            _setNextData_initSeq(_ui8TypeId, sizeof(dataCount_t), iWriteCursor);
            m_ptrSeqCur->writeCount(_ui16Count, iWriteCursor);

            for (int i(0); i < _ui16Count; ++i) {
                int iWeight(0);
                _setNextData_structvar_calcsize((&_var[i])->__v, iWeight);
                _setNextData_structvar_unfold(iWriteCursor, iWeight, (&_var[i])->__v);
            }
        }
        else
            _setNextData_write(_ui8TypeId, _var, _ui16Count, iWriteCursor);
    }


    template<typename T, size_t N, std::enable_if_t<!std::is_pointer_v<T>>* = nullptr>
    auto getNextData(const typeId_t &_ui8TypeId, T(&_arr)[N]) {
        /*int iReadCursor(0);
        if (m_ptrSeqCur->getChunkId(iReadCursor) == _ui8TypeId) {
            m_ptrSeqCur->chunkIdPassed(iReadCursor);

            dataCount_t ui16CountNbrStructs(m_ptrSeqCur->readCount(iReadCursor));
            _updateSeqPtr_Get();

            for (dataCount_t i(0); i < ui16CountNbrStructs; ++i)
                _getNextData_structvar_unfold(iReadCursor, (&_arr[i])->__v);

            _updateSeqPtr_Get();
        }*/

        int iReadCursor(0);
        if (m_ptrSeqCur->getChunkId(iReadCursor) == _ui8TypeId) {
            m_ptrSeqCur->chunkIdPassed(iReadCursor);

            constexpr bool hasVarInfos = requires(T(&&_arr)[N]) {
                (&_arr[0])->__v;
            };

            if constexpr (hasVarInfos) {
                dataCount_t ui16CountNbrStructs(m_ptrSeqCur->readCount(iReadCursor));
                _updateSeqPtr_Get();

                for (dataCount_t i(0); i < ui16CountNbrStructs; ++i)
                    _getNextData_structvar_unfold(iReadCursor, (&_arr[i])->__v);
            }
            else
                _getNextData_retrieve(_arr, iReadCursor);

            _updateSeqPtr_Get();
        }
    }

    template<typename T, std::enable_if_t<!std::is_pointer_v<T>>* = nullptr>
    auto getNextData(const typeId_t &_ui8TypeId, T &_var) {
        int iReadCursor(0);
        if (m_ptrSeqCur->getChunkId(iReadCursor) == _ui8TypeId) {
            m_ptrSeqCur->chunkIdPassed(iReadCursor);

            constexpr bool hasVarInfos = requires(T & _var) {
                _var.__v;
            };

            if constexpr (hasVarInfos) {
                _updateSeqPtr_Get();

                _getNextData_structvar_unfold(iReadCursor, _var.__v);
            }
            else
                _getNextData_retrieve(_var, iReadCursor);

            _updateSeqPtr_Get();
        }
    }

    template<typename T, std::enable_if_t<std::is_pointer_v<T>>* = nullptr>
    auto getNextData(const typeId_t &_ui8TypeId, T _var, const dataCount_t &_ui16Count) {
        int iReadCursor(0);
        if (m_ptrSeqCur->getChunkId(iReadCursor) == _ui8TypeId) {
            m_ptrSeqCur->chunkIdPassed(iReadCursor);

            constexpr bool hasVarInfos = requires(T & _var) {
                (&_var[0])->__v;
            };

            if constexpr (hasVarInfos) {
                dataCount_t ui16CountNbrStructs(m_ptrSeqCur->readCount(iReadCursor));
                _updateSeqPtr_Get();

                for (dataCount_t i(0); i < ui16CountNbrStructs; ++i)
                    _getNextData_structvar_unfold(iReadCursor, (&_var[i])->__v);
            }
            else 
                _getNextData_retrieve(_var, iReadCursor, _ui16Count);

            _updateSeqPtr_Get();
        }
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
    template<typename T, std::enable_if_t<!std::is_pointer_v<T>>* = nullptr>
    auto _getNextData_retrieve(T &_var, int &_iReadCursor) {
        _getNextData_retrieve_(_iReadCursor, _var);
    }

    /*template <typename T, size_t N>
    void _getNextData_retrieve(T(&_arr)[N], int &_iReadCursor) {
        _getNextData_retrieve_(_iReadCursor, _arr);
    }*/

    template<typename T, std::enable_if_t<std::is_pointer_v<T>>* = nullptr>
    auto _getNextData_retrieve(T _var, int &_iReadCursor, const dataCount_t &_ui16Count) {
        dataCount_t ui16Count(m_ptrSeqCur->readCount(_iReadCursor));
        for (int i(0); i < _ui16Count; ++i)
            _getNextData_retrieve_(_iReadCursor, _var[i]);
    }


    template<typename T>
    void _getNextData_retrieve_(int &_iReadOffset, T &_var) {
        m_ptrSeqCur->readData(_var, _iReadOffset);
    }

    template<typename T, typename U>
    void _getNextData_retrieve_(int &_iReadOffset, std::pair<T, U> &_var) {
        _getNextData_retrieve_(_iReadOffset, _var.first);
        _getNextData_retrieve_(_iReadOffset, _var.second);
    }

    template<typename T>
    void _getNextData_retrieve_(int &_iReadOffset, std::vector<T> &_vec) {
        int iCount(static_cast<int>(m_ptrSeqCur->readCount(_iReadOffset)));

        _vec.resize(iCount);
        for (int i(0); i < iCount; ++i)
            _getNextData_retrieve_(_iReadOffset, _vec[i]);
    }

    template<typename T, typename U>
    void _getNextData_retrieve_(int &_iReadOffset, std::vector<std::pair<T, U>> &_vec) {
        int iCount(static_cast<int>(m_ptrSeqCur->readCount(_iReadOffset)));

        _vec.resize(iCount);

        if (typeid(T) == typeid(bool)) {
            int iBoolStackNbr(_opti_bool_calcsize(iCount));

            bool *bArrTemp(new bool[iCount]);
            ::memset(bArrTemp, 0, iCount);

            _opti_bool_discatenate(_iReadOffset, iBoolStackNbr, iCount, bArrTemp);

            for (int i(0); i < iCount; ++i)
                _vec[i].first = bArrTemp[i];

            delete[] bArrTemp;
        }
        else
            for (int i(0); i < iCount; ++i)
                _getNextData_retrieve_(_iReadOffset, _vec[i].first);

        if (typeid(U) == typeid(bool)) {
            int iBoolStackNbr(_opti_bool_calcsize(iCount));

            bool *bArrTemp(new bool[iCount]);
            ::memset(bArrTemp, 0, iCount);

            _opti_bool_discatenate(_iReadOffset, iBoolStackNbr, iCount, bArrTemp);

            for (int i(0); i < iCount; ++i)
                _vec[i].second = bArrTemp[i];

            delete[] bArrTemp;
        }
        else
            for (int i(0); i < iCount; ++i)
                _getNextData_retrieve_(_iReadOffset, _vec[i].second);
    }

    void _getNextData_retrieve_(int &_iReadOffset, std::vector<bool> &_vec) {
        int iBoolCount(static_cast<int>(m_ptrSeqCur->readCount(_iReadOffset))),
            iBoolStackNbr(_opti_bool_calcsize(iBoolCount));

        bool *bArrTemp(new bool[iBoolCount]);
        ::memset(bArrTemp, 0, iBoolCount);

        _opti_bool_discatenate(_iReadOffset, iBoolStackNbr, iBoolCount, bArrTemp);

        _vec.resize(iBoolCount);
        for (int i(0); i < iBoolCount; ++i)
            _vec[i] = bArrTemp[i];

        delete[] bArrTemp;
    }

    template<typename T, size_t N, std::enable_if_t<!std::is_same_v<T, bool>>* = nullptr>
    auto _getNextData_retrieve_(int &_iReadOffset, T(&_arr)[N]) {
        dataCount_t ui16Count(m_ptrSeqCur->readCount(_iReadOffset));
        int iSize(N > ui16Count ? ui16Count : N);
        for (int i(0); i < N; ++i)
            _getNextData_retrieve_(_iReadOffset, _arr[i]);
    }

    template<size_t N>
    void _getNextData_retrieve_(int &_iReadOffset, bool(&_arr)[N]) {
        int iBoolCount(static_cast<int>(m_ptrSeqCur->readCount(_iReadOffset)));
        if (iBoolCount > N)
            iBoolCount = N;
        int iBoolStackNbr(_opti_bool_calcsize(iBoolCount));

        ::memset(_arr, 0, iBoolCount * sizeof(bool));

        _opti_bool_discatenate(_iReadOffset, iBoolStackNbr, iBoolCount, &_arr[0]);
    }

    template <size_t I = 0, typename... T>
    typename std::enable_if<I == sizeof...(T), void>::type
        _getNextData_retrieve_(int &_iReadOffset, std::tuple<T...> &_tuple) {
        return;
    }
    template <size_t I = 0, typename... T>
    typename std::enable_if<(I < sizeof...(T)), void>::type
        _getNextData_retrieve_(int &_iReadOffset, std::tuple<T...> &_tuple) {
        _getNextData_retrieve_(_iReadOffset, std::get<I>(_tuple));
        _getNextData_retrieve_<I + 1>(_iReadOffset, _tuple);
    }


    void _getNextData_structvar_unfold(int &_iReadOffset, const std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec) {
        _iReadOffset = 0;

        const size_t iSize(_vec.size());
        for (int j(0); j < iSize; ++j)
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
    static int _setNextData_calculateSize(const T &_var, const bool &_bIncludeSize) {
        return sizeof(T);
    }

    template<typename T, size_t N>
    static int _setNextData_calculateSize(const T(&_arr)[N], const bool &_bIncludeSize) {
        int iWeight(_bIncludeSize ? sizeof(dataCount_t) : 0);

        for (int i(0); i < N; ++i)
            iWeight += _setNextData_calculateSize(_arr[i], true);

        return iWeight;
    }

    template <size_t N>
    static int _setNextData_calculateSize(const bool(&_arr)[N], const bool &_bIncludeSize) {
        int iWeight(_bIncludeSize ? sizeof(dataCount_t) : 0);

        iWeight += _opti_bool_calcsize(N);

        return iWeight;
    }

    template<typename T, typename U>
    static int _setNextData_calculateSize(const std::pair<T, U> &_pair, const bool &_bIncludeSize) {
        int iWeight(0);

        iWeight += _setNextData_calculateSize(_pair.first, true);
        iWeight += _setNextData_calculateSize(_pair.second, true);

        return iWeight;
    }

    static int _setNextData_calculateSize(const std::vector<bool> &_vec, const bool &_bIncludeSize) {
        int iWeight(_bIncludeSize ? sizeof(dataCount_t) : 0);

        iWeight += _opti_bool_calcsize(_vec.size());

        return iWeight;
    }

    template<typename T, typename U, std::enable_if_t<!std::is_same_v<T, bool> && !std::is_same_v<U, bool>>* = nullptr>
    static auto _setNextData_calculateSize(const std::vector<std::pair<T, U>> &_vec, const bool &_bIncludeSize) {
        int iWeight(_bIncludeSize ? sizeof(dataCount_t) : 0);

        const size_t iSize(_vec.size());
        for (int i(0); i < iSize; ++i) {
            iWeight += _setNextData_calculateSize(_vec[i].first, true);
            iWeight += _setNextData_calculateSize(_vec[i].second, true);
        }

        return iWeight;
    }
    
    template<typename T, typename U>
        static int _setNextData_calculateSize(const std::vector<std::pair<T, U>> &_vec, const bool &_bIncludeSize) {
        int iWeight(_bIncludeSize ? sizeof(dataCount_t) : 0);

        const size_t iSize(_vec.size());
        if (typeid(T) == typeid(bool)) {
            iWeight += _opti_bool_calcsize(iSize);
            for (int i(0); i < iSize; ++i)
                iWeight += _setNextData_calculateSize(_vec[i].second, true);
        }
        else {
            iWeight += _opti_bool_calcsize(iSize);
            for (int i(0); i < iSize; ++i)
                iWeight += _setNextData_calculateSize(_vec[i].first, true);
        }

        return iWeight;
    }

    template<typename T>
    static int _setNextData_calculateSize(const std::vector<T> &_vec, const bool &_bIncludeSize) {
        int iWeight(_bIncludeSize ? sizeof(dataCount_t) : 0);

        const size_t iSize(_vec.size());
        for (int i(0); i < iSize; ++i)
            iWeight += _setNextData_calculateSize(_vec[i], true);

        return iWeight;
    }

    template <size_t I = 0, typename... T>
    static typename std::enable_if<I == sizeof...(T), int>::type
    _setNextData_calculateSize(const std::tuple<T...> &_tuple, const bool &_bIncludeSize) {
        return 0;
    }
    template <size_t I = 0, typename... T>
    static typename std::enable_if<(I < sizeof...(T)), int>::type
    _setNextData_calculateSize(const std::tuple<T...> &_tuple, const bool &_bIncludeSize) {
        int iWeight(_setNextData_calculateSize(std::get<I>(_tuple), true));
        iWeight += _setNextData_calculateSize<I + 1>(_tuple, true);

        return iWeight;
    }

    static int _setNextData_calculateSize(const std::string &_str, const bool &_bIncludeSize) {
        int iWeight(static_cast<uint32_t>(_str.length()) * static_cast<uint32_t>(sizeof(_str[0])));

        if (_bIncludeSize)
            iWeight += sizeof(dataCount_t);

        return iWeight;
    }

    static int _setNextData_calculateSize(const std::u32string &_32str, const bool &_bIncludeSize) {
        int iWeight(static_cast<uint32_t>(_32str.length()) * static_cast<uint32_t>(sizeof(_32str[0])));

        if (_bIncludeSize)
            iWeight += sizeof(dataCount_t);

        return iWeight;
    }


    /*
    ** SETNEXTDATA_WRITE
    */
    template<typename T, std::enable_if_t<!std::is_pointer_v<T>>* = nullptr>
    auto _setNextData_write(const typeId_t &_ui8TypeId, const T &_var, int &_iWriteCursor) {
        int iWeight(_setNextData_calculateSize(_var, true));

        _setNextData_initSeq(_ui8TypeId, iWeight, _iWriteCursor);

        _setNextData_write_(_iWriteCursor, _var);
    }

    /*template <typename T, size_t N>
    void _setNextData_write(const typeId_t &_ui8TypeId, const T(&_arr)[N], int &_iWriteCursor) {
        int iWeight(_setNextData_calculateSize(_arr, true));

        _setNextData_initSeq(_ui8TypeId, iWeight, _iWriteCursor);

        _setNextData_write_(_iWriteCursor, _arr);
    }*/

    template<typename T, std::enable_if_t<std::is_pointer_v<T>>* = nullptr>
    auto _setNextData_write(const typeId_t &_ui8TypeId, const T _var, const dataCount_t &_ui16Count, int &_iWriteCursor) {
        int iWeight(sizeof(dataCount_t));
        for (int i(0); i < _ui16Count; ++i)
            iWeight += _setNextData_calculateSize(_var[i], true);

        _setNextData_initSeq(_ui8TypeId, iWeight, _iWriteCursor);
        m_ptrSeqCur->writeCount(_ui16Count, _iWriteCursor);

        for (int i(0); i < _ui16Count; ++i)
            _setNextData_write_(_iWriteCursor, _var[i]);
    }


    template<typename T, std::enable_if_t<std::is_fundamental_v<T>>* = nullptr>
    auto _setNextData_write_(int &_iWriteCursor, const T &_var) {
        m_ptrSeqCur->writeData(_var, _iWriteCursor);
    }

    template<typename T, std::enable_if_t<std::is_pointer_v<T>>* = nullptr>
    auto _setNextData_write_(int &_iWriteCursor, const T &_var, const uint32_t &_ui32Size) {
        m_ptrSeqCur->writeData(_var, _iWriteCursor, _ui32Size);
    }

    template<typename T, size_t N, std::enable_if_t<!std::is_same_v<T, bool>>* = nullptr>
    auto _setNextData_write_(int &_iWriteCursor, const T(&_arr)[N]) {
        m_ptrSeqCur->writeCount(static_cast<dataCount_t>(N), _iWriteCursor);

        for (int i(0); i < N; ++i)
            _setNextData_write_(_iWriteCursor, _arr[i]);
    }

    template<size_t N>
    void _setNextData_write_(int &_iWriteCursor, const bool(&_arr)[N]) {
        m_ptrSeqCur->writeCount(static_cast<dataCount_t>(N), _iWriteCursor);

        int iNbr(_opti_bool_calcsize(N));

        uint8_t *ui8Arr(new uint8_t[iNbr]);
        ::memset(ui8Arr, 0, iNbr);

        _opti_bool_concat(N, _arr, ui8Arr);

        m_ptrSeqCur->writeData(ui8Arr, _iWriteCursor, iNbr * sizeof(uint8_t));

        delete[] ui8Arr;
    }

    template<typename T, typename U>
    void _setNextData_write_(int &_iWriteCursor, const std::pair<T, U> &_pair) {
        _setNextData_write_(_iWriteCursor, _pair.first);
        _setNextData_write_(_iWriteCursor, _pair.second);
    }
    template<typename T, typename U, size_t N, std::enable_if_t<std::is_same_v<T, bool> || std::is_same_v<U, bool>>* = nullptr>
    auto _setNextData_write_(int &_iWriteCursor, const std::pair<T, U>(&_arr)[N]) {
        for (int i(0); i < N; ++i)
            _setNextData_write_(_iWriteCursor, _arr[i]);
    }

    template<typename T, typename U, std::enable_if_t<!std::is_same_v<T, bool>>* = nullptr>
    auto _setNextData_write_(int &_iWriteCursor, const std::vector<T> &_vec) {
        const size_t iSize(_vec.size());
        m_ptrSeqCur->writeCount(static_cast<dataCount_t>(iSize), _iWriteCursor);

        for (int i(0); i < iSize; ++i)
            _setNextData_write_(_iWriteCursor, _vec[i]);
    }

    /*template<typename T, typename U, std::enable_if_t<!std::is_same_v<T, bool> && !std::is_same_v<U, bool>>* = nullptr>
    auto _setNextData_write_(int &_iWriteCursor, const std::vector<std::pair<T, U>> &_vec) {
        const size_t iSize(_vec.size());
        m_ptrSeqCur->writeCount(static_cast<dataCount_t>(iSize), _iWriteCursor);

        for (int i(0); i < iSize; ++i)
            _setNextData_write_(_iWriteCursor, _vec[i]);
    }*/

    template<typename T, typename U>
    void _setNextData_write_(int &_iWriteCursor, const std::vector<std::pair<T, U>> &_vec) {
        const size_t iSize(_vec.size());
        m_ptrSeqCur->writeCount(static_cast<dataCount_t>(iSize), _iWriteCursor);

        if (typeid(T) == typeid(bool)) {
            const int iPackedBoolSize(_opti_bool_calcsize(_vec.size()));

            bool    *ui8ArrRaw(new bool[iSize]);
            ::memset(ui8ArrRaw, 0, iSize);
            uint8_t *ui8ArrCat(new uint8_t[iPackedBoolSize]);
            ::memset(ui8ArrCat, 0, iPackedBoolSize);

            for (int i(0); i < iSize; ++i)
                ui8ArrRaw[i] = _vec[i].first;

            _opti_bool_concat(iSize, ui8ArrRaw, ui8ArrCat);
            _setNextData_write_(_iWriteCursor, ui8ArrCat, iPackedBoolSize);

            delete[] ui8ArrRaw;
            delete[] ui8ArrCat;
        }
        else
            for (int i(0); i < iSize; ++i)
                _setNextData_write_(_iWriteCursor, _vec[i].first);

        if (typeid(U) == typeid(bool)) {
            const int iPackedBoolSize(_opti_bool_calcsize(_vec.size()));

            bool *ui8ArrRaw(new bool[iSize]);
            ::memset(ui8ArrRaw, 0, iSize);
            uint8_t *ui8ArrCat(new uint8_t[iPackedBoolSize]);
            ::memset(ui8ArrCat, 0, iPackedBoolSize);

            for (int i(0); i < iSize; ++i)
                ui8ArrRaw[i] = _vec[i].second;

            _opti_bool_concat(iSize, ui8ArrRaw, ui8ArrCat);
            _setNextData_write_(_iWriteCursor, ui8ArrCat, iPackedBoolSize);

            delete[] ui8ArrRaw;
            delete[] ui8ArrCat;
        }
        else
            for (int i(0); i < iSize; ++i)
                _setNextData_write_(_iWriteCursor, _vec[i].second);
    }

    void _setNextData_write_(int &_iWriteCursor, const std::vector<bool> &_vec) {
        const size_t iSize(_vec.size());
        m_ptrSeqCur->writeCount(static_cast<dataCount_t>(iSize), _iWriteCursor);

        int iNbr(_opti_bool_calcsize(iSize));

        uint8_t *ui8Arr(new uint8_t[iNbr]);
        ::memset(ui8Arr, 0, iNbr);

        bool *arrTemp(new bool[iSize]);
        ::memset(arrTemp, 0, iSize);
        std::copy(_vec.begin(), _vec.end(), arrTemp);

        _opti_bool_concat(iSize, arrTemp, ui8Arr);

        m_ptrSeqCur->writeData(ui8Arr, _iWriteCursor, iNbr * sizeof(uint8_t));

        delete[] ui8Arr;
        delete[] arrTemp;
    }

    template <size_t I = 0, typename... T>
    typename std::enable_if<I == sizeof...(T), void>::type
        _setNextData_write_(int &_iWriteCursor, const std::tuple<T...> &_tuple) {
        return;
    }
    template <size_t I = 0, typename... T>
    typename std::enable_if<(I < sizeof...(T)), void>::type
        _setNextData_write_(int &_iWriteCursor, const std::tuple<T...> &_tuple) {
        _setNextData_write_(_iWriteCursor, std::get<I>(_tuple));
        _setNextData_write_<I + 1>(_iWriteCursor, _tuple);
    }


    void _setNextData_allocSeq() {
        SnextSequence *sSeq(new SnextSequence());
        _updateSeqPtr_Set(sSeq);
    }

    void _setNextData_initSeq(const typeId_t &_ui8TypeId, const dataCount_t &_ui16Count, int &_iWriteCursor) {
        m_ptrSeqCur->allocChunkMemory(sizeof(typeId_t) + _ui16Count, _iWriteCursor);
        m_ptrSeqCur->setChunkId(_ui8TypeId, _iWriteCursor);
    }

    void _setNextData_structvar_calcsize(const std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec, int &_iWeight) {
        const size_t iSize(_vec.size());
        for (int j(0); j < iSize; ++j)
            _iWeight += _setNextData_structvar_calcsize_unfold(_vec[j].get());
    }

    void _setNextData_structvar_unfold(int &_iWriteCursor, const int &_iWeight, const std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec) {
        _setNextData_structvar_unfold_init(_iWriteCursor, _iWeight);

        const size_t iSize(_vec.size());
        for (int i(0); i < iSize; ++i)
            _setNextData_structvar_write(_iWriteCursor, _vec[i].get());
    }

    void _setNextData_structvar_unfold_init(int &_iWriteCursor, const int &_iWeight) {
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

    /*
    ** OPTIMIZATIONS
    */
    static inline int _opti_bool_calcsize(const size_t &_iSize) {
        return static_cast<int>(::ceil(_iSize / 8.0f));
    }

    static void _opti_bool_concat(const size_t &_iSize, const bool *const _arr, uint8_t *_ui8Arr) {
        for (int i(0), j(0); i < _iSize; ++j, i += 8) {
            int kLast(static_cast<int>(_iSize) - i);
            if (kLast > 8) kLast = 8;
            for (int k(0); k < kLast; ++k) {
                if (_arr[i + k])
                    _ui8Arr[j] |= (1 << k);
            }
        }
    }

    void _opti_bool_discatenate(int &_iReadOffset, const int &_iBoolStackNbr, const int &_iBoolNbr, bool *_bArr) {
        for (int i(0), j(0); j < _iBoolStackNbr; ++j, i += 8) {
            int iBoolInThisStack(_iBoolNbr - i);
            if (iBoolInThisStack > 8)
                iBoolInThisStack = 8;

            uint8_t ui8BoolStack(0);
            m_ptrSeqCur->readData(ui8BoolStack, _iReadOffset);

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