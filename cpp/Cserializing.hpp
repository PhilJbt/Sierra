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
    struct SnextSequence {
        friend Cserializing;
    public:
        SnextSequence() {}

        ~SnextSequence() {
            if (m_ui8ArrBuffer)
                delete[] m_ui8ArrBuffer;
        }

        typeId_t getChunkId() {
            typeId_t ui8ChunkId(0);
            uint32_t ui32DataSize(sizeof(typeId_t));
            ::memcpy(&ui8ChunkId, &m_ui8ArrBuffer[0], ui32DataSize);
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

        elemCnt_t readCount() {
            elemCnt_t ui16Count(0);
            uint32_t ui32DataSize(sizeof(elemCnt_t));
            ::memcpy(&ui16Count, &m_ui8ArrBuffer[sizeof(typeId_t)], ui32DataSize);
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
            const elemCnt_t ui16DataSize(Cserializing::_setNextData_calcSize_(&_var, false, _ui16ElemCount));

            ::memset(_var, 0, ui16DataSize);
            ::memcpy(_var, &m_ui8ArrBuffer[_iOffset], ui16DataSize);
            _iOffset += ui16DataSize;
        }

        void readData(int &_iReadOffset, std::string *_str, const elemCnt_t &_ui16ElemCount) {
            const elemCnt_t ui16BytesLen(readCount(_iReadOffset));

            _str->resize(ui16BytesLen / sizeof((*_str)[0]));
            ::memcpy(&_str[0], &m_ui8ArrBuffer[_iReadOffset], ui16BytesLen);
            _iReadOffset += ui16BytesLen;
        }

        void readData(int &_iReadOffset, std::u32string *_32str, const elemCnt_t &_ui16ElemCount) {
            const elemCnt_t ui16BytesLen(readCount(_iReadOffset));

            _32str->resize(ui16BytesLen / sizeof((*_32str)[0]));
            ::memcpy(&_32str[0], &m_ui8ArrBuffer[_iReadOffset], ui16BytesLen);
            _iReadOffset += ui16BytesLen;
        }

        /*template<typename T>
        void readData(int &_iReadOffset, T *_arr, elemCnt_t _ui16ElemCount) {
            const elemCnt_t ui16ArrLen(readCount(_iReadOffset));

            ::memset(&_arr[0], 0, ui16ArrLen);
            ::memcpy(&_arr[0], &m_ui8ArrBuffer[_iReadOffset], ui16ArrLen);
            _iReadOffset += ui16ArrLen;
        }*/

        /*
        ** WRITEDATA
        */
        template <typename T>
        void writeData(int &_iOffset, const T *const _var, const elemCnt_t &_ui16ElemCount) {
            const elemCnt_t ui16DataSize(Cserializing::_setNextData_calcSize_(&_var, false, _ui16ElemCount));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], _var, ui16DataSize);
            _iOffset += ui16DataSize;
        }

        void writeData(int &_iOffset, const std::string *const _str) {
            const elemCnt_t ui16DataSize(Cserializing::_setNextData_calcSize_(&_str, false));
            writeCount(_iOffset, ui16DataSize);

            ::memcpy(&m_ui8ArrBuffer[_iOffset], _str->data(), ui16DataSize);
            _iOffset += ui16DataSize;
        }

        void writeData(int &_iOffset, const std::u32string *const _32str) {
            const elemCnt_t ui16DataSize(Cserializing::_setNextData_calcSize_(&_32str, false));
            writeCount(_iOffset, ui16DataSize);

            ::memcpy(&m_ui8ArrBuffer[_iOffset], _32str->data(), ui16DataSize);
            _iOffset += ui16DataSize;
        }
        /*
        void writeData(const uint8_t *_ptr, int &_iOffset, const elemCnt_t &_ui16ElemCount) {
            ::memcpy(&m_ui8ArrBuffer[_iOffset], _ptr, _ui16ElemCount);
            _iOffset += _ui16ElemCount;
        }

        template<typename T>
        void writeData(const T * const _arr, int &_iOffset, const elemCnt_t &_ui16ElemCount) {
            writeCount(_ui16ElemCount, _iOffset);

            const elemCnt_t ui16DataSize(Cserializing::_setNextData_calcSize(&_arr, false));
            ::memcpy(&m_ui8ArrBuffer[_iOffset], &_arr[0], ui16DataSize);
            _iOffset += ui16DataSize;
        }*/

        void nextNode(SnextSequence *_ptrNext) {
            m_ptrNext = _ptrNext;
        }

        SnextSequence *nextNode() {
            return m_ptrNext;
        }


        void serialize(uint8_t **_ui8Buffer, int &_iIndex) {
            const size_t iSizeof(sizeof(ChunkLen_t));
            ::memcpy((*_ui8Buffer) + _iIndex, &m_ui16ChunkLen, iSizeof);
            _iIndex += static_cast<int>(iSizeof);

            const int iChunkLen(getChunkSize());
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

        int getChunkSize() {
            return static_cast<int>(m_ui16ChunkLen);
        }

    private:
        void chunkIdPassed(int &_iReadOffset) {
            _iReadOffset += sizeof(typeId_t);
        }

        int getElementCount() {
            elemCnt_t ui16Count(0);
            ::memcpy(&ui16Count, m_ui8ArrBuffer + sizeof(ChunkLen_t) + sizeof(typeId_t), sizeof(elemCnt_t));
            return static_cast<int>(ui16Count);
        }


        uint8_t       *m_ui8ArrBuffer = nullptr;
        ChunkLen_t     m_ui16ChunkLen = 0;
        SnextSequence *m_ptrNext      = nullptr;
    };

    struct SvarInfo_base {
    public:
        virtual ~SvarInfo_base() {}

        elemCnt_t                                                  count()     const { return m_ui16ElemCount; }
        std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *ptrNode()   const { return m_ptrNode; }
        bool                                                       isInfoVec() const { return m_bIsVarInfosVec; }

        virtual int calcsize() const { return 0; }
        virtual void write(int &_iReadOffset, SnextSequence *_sSeq, Cserializing *_ptrSerializer, const elemCnt_t &_ui16ElemCount) {}
        virtual void read(int &_iReadOffset, SnextSequence *_sSeq, Cserializing *_ptrSerializer, const elemCnt_t &_ui16ElemCount) {}

    protected:
        elemCnt_t                                                  m_ui16ElemCount  = 0;
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

        int calcsize() {
            return Cserializing ::_setNextData_calcSize(&m_ptrVar, true, m_ui16ElemCount);
        }

        void write(int &_iReadOffset, SnextSequence *_sSeq, Cserializing *_ptrSerializer, const elemCnt_t &_ui16ElemCount) {
            const T *ptr = const_cast<const T*>(m_ptrVar);
            //_ptrSerializer->_setNextData_write_(_iReadOffset, ptr, _ui16ElemCount);
        }

        void read(int &_iReadOffset, SnextSequence *_sSeq, Cserializing *_ptrSerializer, const elemCnt_t &_ui16ElemCount) {
            T *ptr = m_ptrVar;
            //_ptrSerializer->_getNextData_read_(_iReadOffset, ptr);
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


    /*template <typename T>
    static void __INTERNAL__v_push_var(std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec, T &_var) {
        _vec.push_back(std::make_unique<Cserializing::SvarInfo<T>>(_var));
    }

    template <typename... T>
    static std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __INTERNAL__v_push(T&&... args) {
        std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v;

        (__INTERNAL__v_push_var(__v, args), ...);

        return __v;
    }*/


    void changeTypeTo_Set() {
        m_ptrSeqCur = m_ptrSeqEnd;
    }

    void changeTypeTo_Get() {
        m_ptrSeqCur = m_ptrSeqBeg;
    }


    int nextChunk_Size() {
        if (m_ptrSeqCur)
            return m_ptrSeqCur->getChunkSize();
        else
            return -1;
    }

    int nextChunk_Id() {
        if (m_ptrSeqCur)
            return m_ptrSeqCur->getChunkId();
        else
            return -1;
    }

    int nextChunk_HeapCount() {
        if (m_ptrSeqCur)
            return static_cast<int>(m_ptrSeqCur->readCount());
        else
            return -1;
    }


    /*
    ** WRITE
    */

    // W STACK VAR
    template<typename T>
        requires(!std::is_array<T>::value && !std::is_pointer<T>::value)
    void nextChunk_SetData(const typeId_t &_ui8TypeId, const T &_var) {
        const T *const ptr = &_var;
        const T *const *const ptr2 = &ptr;
        _setNextData_write(_ui8TypeId, ptr2, 1);
    }

    // W VECTOR<T>
    /*template<typename T>
        requires(!std::is_array<T>::value && !std::is_pointer<T>::value)
    void nextChunk_SetData(const typeId_t &_ui8TypeId, const std::vector<T> &_var) {
        const std::vector<T> *const ptr = &_var;
        const std::vector<T> *const *const ptr2 = &ptr;
        _setNextData_write(_ui8TypeId, ptr2, static_cast<elemCnt_t>(_var.size()));
    }*/

    // W ARR[N]
    template<typename T, size_t N>
    void nextChunk_SetData(const typeId_t &_ui8TypeId, const T (&_var)[N]) {
        const T *ptr = &_var[0];
        _setNextData_write(_ui8TypeId, &ptr, N);
    }

    // W HEAP
    template<typename T>
    void nextChunk_SetData(const typeId_t &_ui8TypeId, const T * const  * const _var, const elemCnt_t &_ui16ElemCount) {
        _setNextData_write(_ui8TypeId, _var, _ui16ElemCount);
    }
    
    // W SEQUENCE
    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename T>
        requires(!std::is_pointer<T>::value)
    void nextChunk_SetData(const typeId_t &_ui8TypeId, const C<T, A<T>> &_var) {
        const std::vector<T> *const ptr = &_var;
        const std::vector<T> *const *const ptr2 = &ptr;
        _setNextData_write(_ui8TypeId, ptr2, 1);
    }
    /*
    // W ASSOCIATIVE
    template <
        template <typename, typename, typename, typename> class C,
        template <typename> class Comp,
        template <typename> class A,
        typename K,
        typename T>
        requires(!std::is_pointer<T>::value)
    void nextChunk_SetData(const typeId_t &_ui8TypeId, const C<K, T, Comp<K>, A<std::pair<const K, T>>> &_var) {
        const std::vector<T> *const ptr = &_var;
        const std::vector<T> *const *const ptr2 = &ptr;
        //_setNextData_write(_ui8TypeId, ptr2, static_cast<elemCnt_t>(_var.size()));
    }

    // W HASHED
    template <
        template <typename, typename, typename, typename, typename> class C,
        template <typename> class Hash,
        template <typename> class Pred,
        template <typename> class A,
        typename K,
        typename T>
        requires(!std::is_pointer<T>::value)
    void nextChunk_SetData(const typeId_t &_ui8TypeId, const C<K, T, Hash<K>, Pred<K>, A<std::pair<const K, T>>> &_var) {
        const std::vector<T> *const ptr = &_var;
        const std::vector<T> *const *const ptr2 = &ptr;
        //_setNextData_write(_ui8TypeId, ptr2, static_cast<elemCnt_t>(_var.size()));
    }
    */

    /*
    ** READ
    */

    // R STACK VAR
    template<typename T>
        requires(!std::is_array<T>::value && !std::is_pointer<T>::value)
    void nextChunk_GetData(const typeId_t &_ui8TypeId, T &_var) {
        T *ptr = &_var;
        T **ptr2 = &ptr;
        _getNextData_read(_ui8TypeId, ptr2);
    }

    // R VECTOR<T>
    /*template<typename T>
        requires(!std::is_array<T>::value && !std::is_pointer<T>::value)
    void nextChunk_GetData(const typeId_t &_ui8TypeId, const std::vector<T> &_var) {
        std::vector<T> * ptr = &_var;
        std::vector<T> ** ptr2 = &ptr;
        _setNextData_write(_ui8TypeId, ptr2);
    }*/

    // R ARR[N]
    template<typename T, size_t N>
    void nextChunk_GetData(const typeId_t &_ui8TypeId, T (&_var)[N]) {
        T *ptr = &_var[0];
        _getNextData_read(_ui8TypeId, &ptr);
    }

    // R HEAP
    template<typename T>
    void nextChunk_GetData(const typeId_t &_ui8TypeId, T **_var) {
        _getNextData_read(_ui8TypeId, _var);
    }
    
    // R SEQUENCE
    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename T>
        requires(!std::is_pointer<T>::value)
    void nextChunk_GetData(const typeId_t &_ui8TypeId, C<T, A<T>> &_var) {
        std::vector<T> * ptr = &_var;
        std::vector<T> ** ptr2 = &ptr;
        _getNextData_read(_ui8TypeId, ptr2);
    }
    /*
    // R ASSOCIATIVE
    template <
        template <typename, typename, typename, typename> class C,
        template <typename> class Comp,
        template <typename> class A,
        typename K,
        typename T>
        requires(!std::is_pointer<T>::value)
    void nextChunk_SetData(const typeId_t &_ui8TypeId, C<K, T, Comp<K>, A<std::pair<const K, T>>> &_var) {
        std::vector<T> *ptr = &_var;
        std::vector<T> **ptr2 = &ptr;
        //_getNextData_read(_ui8TypeId, ptr2);
    }

    // R HASHED
    template <
        template <typename, typename, typename, typename, typename> class C,
        template <typename> class Hash,
        template <typename> class Pred,
        template <typename> class A,
        typename K,
        typename T>
        requires(!std::is_pointer<T>::value)
    void nextChunk_SetData(const typeId_t &_ui8TypeId, C<K, T, Hash<K>, Pred<K>, A<std::pair<const K, T>>> &_var) {
        std::vector<T> *ptr = &_var;
        std::vector<T> **ptr2 = &ptr;
        //_getNextData_read(_ui8TypeId, ptr2);
    }
    */

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
    ** WRITE / CALCSIZE / READ
    */

    /*
    ** ENTRY POINT
    */

    // WRITE ENTRY POINT
    template<typename T>
    void _setNextData_write(const typeId_t &_ui8TypeId, const T *const *const _var, const elemCnt_t &_ui16ElemCount) {
        int iOffset(0),
            iSize(_setNextData_calcSize_(_var, true, _ui16ElemCount));

        _setNextData_initSeq(_ui8TypeId, static_cast<elemCnt_t>(iSize), iOffset);

        /*if (_setNextData_write_userstruct_check(_var, _ui16ElemCount))
            _setNextData_write_userstruct(iOffset, _var, _ui16ElemCount);
        else*/
            _setNextData_write_(iOffset, _var, _ui16ElemCount);
    }

    //// CALC ENTRY POINT
    //template<typename T>
    //static int _setNextData_calcSize(const T *const *const _var, const bool &_bIncludeElemeCntSize, const elemCnt_t &_ui16ElemCount) {
    //    return _setNextData_calcSize_(_var, _bIncludeElemeCntSize, _ui16ElemCount);
    //}

    //template<typename T>
    //static int _setNextData_calcSize(const T *const *const _var, const bool &_bIncludeElemeCntSize) {
    //    return _setNextData_calcSize_(_var, _bIncludeElemeCntSize);
    //}

    // READ ENTRY POINT
    template<typename T>
    void _getNextData_read(const typeId_t &_ui8TypeId, T ** _var) {
        int iReadCursor(0);
        if (m_ptrSeqCur->getChunkId() == _ui8TypeId) {
            m_ptrSeqCur->chunkIdPassed(iReadCursor);

            /*if (_setNextData_write_userstruct_check(_var, _ui16ElemCount))
                _setNextData_write_userstruct(iWriteCursor, _var, _ui16ElemCount);
            else*/
                _getNextData_read_(iReadCursor, _var);

            _updateSeqPtr_Get();
        }
    }


    /*
            USERSTRUCT
    */

    // WRITE USERSTRUCT
    template<typename T>
    bool _setNextData_write_userstruct_check(const T *const _var, const elemCnt_t &_ui16ElemCount) {
        constexpr bool hasVarInfos = requires(const T *const _var) {
            _var->__v;
        };

        if constexpr (hasVarInfos)
            return true;
        else
            return false;
    }

    /*template<typename T>
    void _setNextData_write_userstruct(const int &_iOffset, const T *const _var, const elemCnt_t &_ui16ElemCount) {
        _setNextData_userstruct_unfold(_iOffset, iWeight, _var->__v);
    }*/

    // CALC USERSTRUCT
    /*template <typename T>
    static int _setNextData_calcSize_(const T *const _var, const bool &_bIncludeElemeCntSize, const elemCnt_t &_ui16ElemCount) {
        _setNextData_userstruct_unfold(_iOffset, iWeight, _var->__v);
    }*/

    // READ USERSTRUCT
    // ...


    /*
    ** BUFFER
    */

    // WRITE BUFFER
    void _setNextData_write_rawdata(int &_iOffset, const uint8_t *const _ptr, const elemCnt_t &_ui16DataLen, const elemCnt_t &_ui16ElemCount) {
        m_ptrSeqCur->writeData(_iOffset, _ptr, _ui16DataLen);
    }

    // CALC BUFFER
    // ...

    // READ BUFFER
    // ...


    /*
            * T
            * [] T
    */

    // WRITE * * T
    // WRITE * * [] T
    template<typename T>
    void _setNextData_write_(int &_iOffset, const T *const *const _var, const elemCnt_t &_ui16ElemCount) {
        if (typeid(T) == typeid(const bool)) {
            m_ptrSeqCur->writeCount(_iOffset, _ui16ElemCount);

            _opti_bool_concat(_iOffset, this, _ui16ElemCount, reinterpret_cast<const bool *const>(_var[0]));
        }
        else {
            m_ptrSeqCur->writeCount(_iOffset, _ui16ElemCount);

            for (size_t i(0); i < _ui16ElemCount; ++i)
                _setNextData_write_(_iOffset, &(*_var)[i]);
        }
    }

    template<typename T>
    void _setNextData_write_(int &_iOffset, const T (*_var), const elemCnt_t &_ui16ElemCount) {
        m_ptrSeqCur->writeCount(_iOffset, _ui16ElemCount);

        for (size_t i(0); i < _ui16ElemCount; ++i)
            _setNextData_write_(_iOffset, &(*_var)[i]);
    }

    // CALC * * T
    // CALC * * [] T
    template<typename T>
    static int _setNextData_calcSize_(const T *const *const _var, const bool &_bIncludeElemeCntSize, const elemCnt_t &_ui16ElemCount) {
        if (typeid(T) == typeid(const bool)) {
            int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

            iWeight += _opti_bool_calcsize(_ui16ElemCount);

            return iWeight;
        }
        else {
            int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

            for (elemCnt_t i(0); i < _ui16ElemCount; ++i)
                iWeight += _setNextData_calcSize_(&(*_var)[i], _bIncludeElemeCntSize);

            return iWeight;
        }
    }

    template<typename T>
    static int _setNextData_calcSize_(const T (*_var), const bool &_bIncludeElemeCntSize, const elemCnt_t &_ui16ElemCount) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

        for (elemCnt_t i(0); i < _ui16ElemCount; ++i)
            iWeight += _setNextData_calcSize_(&(*_var)[i], _bIncludeElemeCntSize);

        return iWeight;
    }

    // READ * * T
    // READ * * [] T
    template<typename T>
    void _getNextData_read_(int &_iOffset, T **_var) {
        if (typeid(T) == typeid(const bool)) {
            elemCnt_t ui16Count(m_ptrSeqCur->readCount(_iOffset));

            bool *bArrTemp(new bool[ui16Count]);
            _opti_bool_discat(_iOffset, ui16Count, bArrTemp);

            for (elemCnt_t i(0); i < ui16Count; ++i)
                reinterpret_cast<bool &>((*_var)[i]) = bArrTemp[i];

            delete[] bArrTemp;
        }
        else {
            elemCnt_t ui16Count(m_ptrSeqCur->readCount(_iOffset));

            for (elemCnt_t i(0); i < ui16Count; ++i)
                _getNextData_read_(_iOffset, &(*_var)[i]);
        }
    }

    template<typename T>
    void _getNextData_read_(int &_iOffset, T (*_var)) {
        elemCnt_t ui16Count(m_ptrSeqCur->readCount(_iOffset));

        for (elemCnt_t i(0); i < ui16Count; ++i)
            _getNextData_read_(_iOffset, &(*_var)[i]);
    }


    // WRITE * T
    // WRITE * [] T
    template<typename T>
        requires (std::is_fundamental<T>::value)
    void _setNextData_write_(int &_iOffset, const T *const _var) {
        if (typeid(T) == typeid(const bool))
            _opti_bool_concat(_iOffset, this, 1, reinterpret_cast<const bool *>(_var));
        else
            m_ptrSeqCur->writeData(_iOffset, &(*_var), 1);
    }

    template<typename T>
        requires (std::is_fundamental<T>::value)
    void _setNextData_write_(int &_iOffset, const T *const _var, const elemCnt_t &_ui16ElemCount) {
        if (typeid(T) == typeid(const bool))
            _opti_bool_concat(_iOffset, this, _ui16ElemCount, reinterpret_cast<const bool *>(_var));
        else {
            m_ptrSeqCur->writeCount(_iOffset, _ui16ElemCount);
            m_ptrSeqCur->writeData(_iOffset, &(*_var), _ui16ElemCount);
        }
    }

    template<typename T>
        requires (!std::is_fundamental<T>::value)
    void _setNextData_write_(int &_iOffset, const T *const _var, const elemCnt_t &_ui16ElemCount) {
        m_ptrSeqCur->writeCount(_iOffset, _ui16ElemCount);

        for (size_t i(0); i < _ui16ElemCount; ++i)
            _setNextData_write_(_iOffset, &(_var[i]));
    }

    // CALC * T
    // CALC * [] T
    template<typename T>
        requires (std::is_fundamental<T>::value)
    static int _setNextData_calcSize_(const T *const _var, const bool &_bIncludeElemeCntSize) {
        int iWeight(0);

        if (typeid(T) == typeid(const bool))
            iWeight += _opti_bool_calcsize(1);
        else
            iWeight += static_cast<int>(sizeof(*_var));

        return iWeight;
    }

    template<typename T>
        requires (std::is_fundamental<T>::value)
    static int _setNextData_calcSize_(const T *const _var, const bool &_bIncludeElemeCntSize, const elemCnt_t &_ui16ElemCount) {
        int iWeight(0);

        if (typeid(T) == typeid(const bool)) {
            iWeight += (_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);
            iWeight += _opti_bool_calcsize(_ui16ElemCount);
        }
        else {
            if (_bIncludeElemeCntSize)
                iWeight += sizeof(elemCnt_t);
            iWeight += static_cast<int>(sizeof(*_var)) * static_cast<int>(_ui16ElemCount);
        }

        return iWeight;
    }

    template<typename T>
        requires (!std::is_fundamental<T>::value)
    static int _setNextData_calcSize_(const T *const _var, const bool &_bIncludeElemeCntSize, const elemCnt_t &_ui16ElemCount) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

        for (elemCnt_t i(0); i < _ui16ElemCount; ++i)
            iWeight += _setNextData_calcSize_(_var[i], _bIncludeElemeCntSize);

        return iWeight;
    }

    // READ * T
    // READ * [] T
    template <typename T>
        requires (std::is_fundamental<T>::value)
    void _getNextData_read_(int &_iOffset, T * _var) {
        if (typeid(T) == typeid(bool)) {
            bool bArrTemp(false);
            _opti_bool_discat(_iOffset, 1, &bArrTemp);

            *_var = bArrTemp;
        }
        else
            m_ptrSeqCur->readData(_iOffset, &(*_var), 1);
    }

    template <typename T>
        requires (std::is_fundamental<T>::value)
    void _getNextData_read_(int &_iOffset, T *_var, const elemCnt_t &_ui16ElemCount) {
        if (typeid(T) == typeid(bool)) {
            bool *bArrTemp(new bool[_ui16ElemCount]);
            _opti_bool_discat(_iOffset, _ui16ElemCount, bArrTemp);

            for (size_t i(0); i < _ui16ElemCount; ++i)
                _var[i] = bArrTemp[i];

            delete[] bArrTemp;
        }
        else
            m_ptrSeqCur->readData(_iOffset, &(*_var), _ui16ElemCount);
    }

    template<typename T>
        requires (!std::is_fundamental<T>::value)
    void _getNextData_read_(int &_iOffset, T * _var, const elemCnt_t &_ui16ElemCount) {
        for (elemCnt_t i(0); i < _ui16ElemCount; ++i)
            _getNextData_read_(_iOffset, &(_var[i]));
    }


    /*
            * PAIR<T, U>
    */

    // WRITE * PAIR<T, U>
    // ...

    // CALC * PAIR<T, U>
    // ...

    // READ * PAIR<T, U>
    // ...


    /*
            * TUPLE<T, U, ...>
    */

    // WRITE * TUPLE<T, U, ...>
    // ...

    // CALC * TUPLE<T, U, ...>
    // ...

    // READ * TUPLE<T, U, ...>
    // ...

    /*
            * [] PAIR<T, U>
    */

    // WRITE * [] PAIR<T, U>
    // ...

    // CALC * [] PAIR<T, U>
    // ...

    // READ * [] PAIR<T, U>
    // ...


    /*
            * [] TUPLE<T, ...>
    */

    // WRITE * [] TUPLE<T, ...>
    // ...

    // CALC * [] TUPLE<T, ...>
    // ...

    // READ * [] TUPLE<T, ...>
    // ...


    /*
            * SEQUENCE_CONTAINER<T>
    */

    // WRITE * SEQUENCE_CONTAINER<T>
    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename T>
        requires (std::is_fundamental<T>::value)
    void _setNextData_write_(int &_iOffset, const C<T, A<T>> *const _vec) {
        const size_t iSize(_vec->size());
        m_ptrSeqCur->writeCount(_iOffset, static_cast<elemCnt_t>(iSize));

        if (typeid(T) == typeid(const bool)) {
            bool *arrTemp(new bool[iSize]);
            ::memset(arrTemp, 0, iSize);
            for (size_t i(0); i < iSize; ++i)
                arrTemp[i] = (*_vec[i]);

            _opti_bool_concat(_iOffset, this, static_cast<elemCnt_t>(iSize), arrTemp);

            delete[] arrTemp;
        }
        else
            for (size_t i(0); i < iSize; ++i)
                _setNextData_write_(_iOffset, &((*_vec)[i]), 1);
    }

    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename T>
        requires (!std::is_fundamental<T>::value)
    void _setNextData_write_(int &_iOffset, const C<T, A<T>> *const _vec) {
        const size_t iSize(_vec->size());
        m_ptrSeqCur->writeCount(_iOffset, static_cast<elemCnt_t>(iSize));

        for (size_t i(0); i < iSize; ++i)
            _setNextData_write_(_iOffset, &((*_vec)[i]));
    }

    // CALC * SEQUENCE_CONTAINER<T>
    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename T>
        requires (std::is_fundamental<T>::value)
    static int _setNextData_calcSize_(const C<T, A<T>> *const _vec, const bool &_bIncludeElemeCntSize) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);
        const size_t iSize(_vec->size());

        if (typeid(T) == typeid(const bool))
            iWeight += _opti_bool_calcsize(iSize);
        else
            for (size_t i(0); i < iSize; ++i)
                _setNextData_calcSize_(&((*_vec)[i]), _bIncludeElemeCntSize);

        return iWeight;
    }

    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename T>
        requires (!std::is_fundamental<T>::value)
    static int _setNextData_calcSize_(const C<T, A<T>> *const _vec, const bool &_bIncludeElemeCntSize) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);
        const size_t iSize(_vec->size());

        for (size_t i(0); i < iSize; ++i)
            iWeight += _setNextData_calcSize_(&((*_vec)[i]), _bIncludeElemeCntSize);

        return iWeight;
    }

    // READ * SEQUENCE_CONTAINER<T>
    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename T>
        requires (std::is_fundamental<T>::value)
    void _getNextData_read_(int &_iOffset, C<T, A<T>> * _vec) {
        elemCnt_t ui16Count(m_ptrSeqCur->readCount(_iOffset));

        if (typeid(T) == typeid(const bool)) {
            bool *arrTemp(new bool[ui16Count]);
            ::memset(arrTemp, 0, ui16Count);
            for (size_t i(0); i < ui16Count; ++i)
                arrTemp[i] = (*_vec[i]);

            _opti_bool_concat(_iOffset, this, ui16Count, arrTemp);

            delete[] arrTemp;
        }
        else
            for (size_t i(0); i < ui16Count; ++i)
                _getNextData_read_(_iOffset, &((*_vec)[i]));
    }

    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename T>
        requires (!std::is_fundamental<T>::value)
    void _getNextData_read_(int &_iOffset, C<T, A<T>> *_vec) {
        elemCnt_t ui16Count(m_ptrSeqCur->readCount(_iOffset));

        _vec->resize(ui16Count);

        for (size_t i(0); i < ui16Count; ++i)
            _getNextData_read_(_iOffset, &((*_vec)[i]));
    }


    /*
            * SEQUENCE_CONTAINER<PAIR<T, U>>
    */

    // WRITE * SEQUENCE_CONTAINER<PAIR<T, U>>
    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename P1,
        typename P2>
    void _setNextData_write_(int &_iOffset, const C<std::pair<P1, P2>, A<std::pair<P1, P2>>> *const _vec) {
        const size_t iSize(_vec->size());

        m_ptrSeqCur->writeCount(_iOffset, static_cast<elemCnt_t>(iSize));

        if (typeid(P1) == typeid(bool)) {
            bool *ui8ArrRaw(new bool[iSize]);
            ::memset(ui8ArrRaw, 0, iSize);

            for (size_t i(0); i < iSize; ++i)
                ui8ArrRaw[i] = (*_vec)[i].first;

            _opti_bool_concat(_iOffset, this, static_cast<elemCnt_t>(iSize), ui8ArrRaw);

            delete[] ui8ArrRaw;
        }
        else
            for (int i(0); i < iSize; ++i)
                _setNextData_write_(_iOffset, &((*_vec)[i].first));

        if (typeid(P2) == typeid(bool)) {
            bool *ui8ArrRaw(new bool[iSize]);
            ::memset(ui8ArrRaw, 0, iSize);

            for (size_t i(0); i < iSize; ++i)
                ui8ArrRaw[i] = (*_vec)[i].second;

            _opti_bool_concat(_iOffset, this, static_cast<elemCnt_t>(iSize), ui8ArrRaw);

            delete[] ui8ArrRaw;
        }
        else
            for (size_t i(0); i < iSize; ++i)
                _setNextData_write_(_iOffset, &((*_vec)[i].second));
    }

    // CALC * SEQUENCE_CONTAINER<PAIR<T, U>>
    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename P1,
        typename P2>
    static int _setNextData_calcSize_(const C<std::pair<P1, P2>, A<std::pair<P1, P2>>> *const _vec, const bool &_bIncludeElemeCntSize) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);
        const size_t iSize(_vec->size());

        if (typeid(P1) == typeid(bool))
            iWeight += _opti_bool_calcsize(iSize);
        else
            for (int i(0); i < iSize; ++i)
                iWeight += _setNextData_calcSize_(&((*_vec)[i].first), _bIncludeElemeCntSize);

        if (typeid(P2) == typeid(bool))
            iWeight += _opti_bool_calcsize(iSize);
        else
            for (size_t i(0); i < iSize; ++i)
                iWeight += _setNextData_calcSize_(&((*_vec)[i].second), _bIncludeElemeCntSize);

        return iWeight;
    }

    // READ * SEQUENCE_CONTAINER<PAIR<T, U>>
    template <
        template <typename, typename> class C,
        template <typename> class A,
        typename P1,
        typename P2>
    void _getNextData_read_(int &_iOffset, C<std::pair<P1, P2>, A<std::pair<P1, P2>>> * _vec) {
        elemCnt_t ui16Count(m_ptrSeqCur->readCount(_iOffset));

        _vec->resize(ui16Count);

        if (typeid(P1) == typeid(bool)) {
            const int iCountBools(ui16Count);

            bool *bArrTemp(new bool[iCountBools]);
            _opti_bool_discat(_iOffset, iCountBools, bArrTemp);

            for (int i(0); i < iCountBools; ++i)
                (*_vec)[i].first = bArrTemp[i];

            delete[] bArrTemp;
        }
        else
            for (size_t i(0); i < ui16Count; ++i)
                _getNextData_read_(_iOffset, &((*_vec)[i].first), 1);

        if (typeid(P2) == typeid(bool)) {
            const int iCountBools(ui16Count);

            bool *bArrTemp(new bool[iCountBools]);
            _opti_bool_discat(_iOffset, iCountBools, bArrTemp);

            for (int i(0); i < iCountBools; ++i)
                (*_vec)[i].second = bArrTemp[i];

            delete[] bArrTemp;
        }
        else
            for (size_t i(0); i < ui16Count; ++i)
                _getNextData_read_(_iOffset, &((*_vec)[i].second), 1);
    }


    /*
            * SEQUENCE_CONTAINER<TUPLE<T, ...>>
    */

    // WRITE * SEQUENCE_CONTAINER<TUPLE<T, ...>>
    // ...

    // CALC * SEQUENCE_CONTAINER<TUPLE<T, ...>>
    // ...

    // READ * SEQUENCE_CONTAINER<TUPLE<T, ...>>
    // ...

    // --
    /*
        // SEQUENCE T
        template <
            template <typename, typename> class C,
            template <typename> class A,
            typename T>
        void func(C<T, A<T>> **_var) {

        }

        // ASSOCIATIVE T
        template <
            template <typename, typename, typename, typename> class C,
            template <typename> class Comp,
            template <typename> class A,
            typename K,
            typename T>
        void func(C<K, T, Comp<K>, A<std::pair<const K, T>>> **_var) {

        }

        // HASHED T
        template <
            template <typename, typename, typename, typename, typename> class C,
            template <typename> class Hash,
            template <typename> class Pred,
            template <typename> class A,
            typename K,
            typename T>
        void func(C<K, T, Hash<K>, Pred<K>, A<std::pair<const K, T>>> **_var) {

        }

        // SEQUENCE PAIR
        template <
            template <typename, typename> class C,
            template <typename> class A,
            typename P1,
            typename P2>
        void func(C<std::pair<P1, P2>, A<std::pair<P1, P2>>> **_var) {

        }

        // ASSOCIATIVE PAIR<KEY, PAIR>
        template <
            template <typename, typename, typename, typename> class C,
            template <typename> class Comp,
            template <typename> class A,
            typename P1,
            typename P2,
            typename KEY>
        void func(C<KEY, std::pair<P1, P2>, Comp<KEY>, A<std::pair<const KEY, std::pair<P1, P2>>>> **_var) {

        }

        // ASSOCIATIVE PAIR<PAIR, TYPE>
        template <
            template <typename, typename, typename, typename> class C,
            template <typename> class Comp,
            template <typename> class A,
            typename P1,
            typename P2,
            typename KEY>
        void func(C<std::pair<P1, P2>, KEY, Comp<Pair<P1, P2>>, A<std::pair<const std::pair<P1, P2>, KEY>>> **_var) {

        }

        // HASHED <KEY, PAIR>
        template <
            template <typename, typename, typename, typename, typename> class C,
            template <typename> class Hash,
            template <typename> class Pred,
            template <typename> class A,
            typename K,
            typename P1,
            typename P2>
        void func(C<K, std::pair<P1, P2>, Hash<K>, Pred<K>, A<std::pair<const K, std::pair<P1, P2>>>> **_var) {

        }
    */
    // --


    /*
            STRING, U32STRING
    */

    // CALC STRING
    static int _setNextData_calcSize_(const std::string *const *const _str, const bool &_bIncludeElemeCntSize) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

        iWeight += static_cast<uint32_t>((*_str)->length()) * static_cast<uint32_t>(sizeof(_str[0]));

        return iWeight;
    }

    // CALC U32STRING
    static int _setNextData_calcSize_(const std::u32string *const *const _32str, const bool &_bIncludeElemeCntSize) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

        iWeight += static_cast<uint32_t>((*_32str)->length()) * static_cast<uint32_t>(sizeof(_32str[0]));

        return iWeight;
    }



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


    /*template<typename T, std::enable_if_t<std::is_pointer_v<T> && !std::is_same_v<T, bool*>>* = nullptr>
    static auto _setNextData_calcSize_(const T _ptr, const bool &_bIncludeElemeCntSize, const elemCnt_t &_ui16ElemCount) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

        for (elemCnt_t i(0); i < _ui16ElemCount; ++i)
            iWeight += _setNextData_calcSize_(_ptr[i], true);

        return iWeight;
    }

    template<typename T, std::enable_if_t<std::is_pointer_v<T> && std::is_same_v<T, bool*>>* = nullptr>
    static auto _setNextData_calcSize_(const T _ptr, const bool &_bIncludeElemeCntSize, const elemCnt_t &_ui16ElemCount) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

        iWeight += _opti_bool_calcsize(_ui16ElemCount);

        return iWeight;
    }

    template <size_t N>
    static int _setNextData_calcSize_(const bool(&_arr)[N], const bool &_bIncludeElemeCntSize) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

        iWeight += _opti_bool_calcsize(N);

        return iWeight;
    }

    template<typename T, typename U>
    static int _setNextData_calcSize_(const std::pair<T, U> &_pair, const bool &_bIncludeElemeCntSize) {
        int iWeight(0);

        iWeight += _setNextData_calcSize_(_pair.first, true);
        iWeight += _setNextData_calcSize_(_pair.second, true);

        return iWeight;
    }

    static int _setNextData_calcSize_(const std::vector<bool> &_vec, const bool &_bIncludeElemeCntSize) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

        iWeight += _opti_bool_calcsize(_vec.size());

        return iWeight;
    }

    template<typename T, typename U, std::enable_if_t<!std::is_same_v<T, bool> && !std::is_same_v<U, bool>>* = nullptr>
    static auto _setNextData_calcSize_(const std::vector<std::pair<T, U>> &_vec, const bool &_bIncludeElemeCntSize) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

        const size_t iSize(_vec.size());
        for (int i(0); i < iSize; ++i) {
            iWeight += _setNextData_calcSize_(_vec[i].first, true);
            iWeight += _setNextData_calcSize_(_vec[i].second, true);
        }

        return iWeight;
    }
    
    template<typename T, typename U>
        static int _setNextData_calcSize_(const std::vector<std::pair<T, U>> &_vec, const bool &_bIncludeElemeCntSize) {
        const size_t iSize(sizeof(elemCnt_t));
        int iWeight(_bIncludeElemeCntSize ? iSize : 0);

        const size_t iSize(_vec.size());
        if (typeid(T) == typeid(bool)) {
            iWeight += iSize;
            iWeight += _opti_bool_calcsize(iSize);
            for (int i(0); i < iSize; ++i)
                iWeight += _setNextData_calcSize_(_vec[i].second, true);
        }
        
        if (typeid(U) == typeid(bool)) {
            iWeight += iSize;
            iWeight += _opti_bool_calcsize(iSize);
            for (int i(0); i < iSize; ++i)
                iWeight += _setNextData_calcSize_(_vec[i].first, true);
        }

        return iWeight;
    }

    template<typename T>
    static int _setNextData_calcSize_(const std::vector<T> &_vec, const bool &_bIncludeElemeCntSize) {
        int iWeight(_bIncludeElemeCntSize ? sizeof(elemCnt_t) : 0);

        const size_t iSize(_vec.size());
        for (int i(0); i < iSize; ++i)
            iWeight += _setNextData_calcSize_(_vec[i], true);

        return iWeight;
    }

    template <size_t I = 0, typename... T>
    static typename std::enable_if<I == sizeof...(T), int>::type
        _setNextData_calcSize_(const std::tuple<T...> &_tuple, const bool &_bIncludeElemeCntSize) {
        return 0;
    }
    template <size_t I = 0, typename... T>
    static typename std::enable_if<(I < sizeof...(T)), int>::type
        _setNextData_calcSize_(const std::tuple<T...> &_tuple, const bool &_bIncludeElemeCntSize) {
        int iWeight(_setNextData_calcSize_(std::get<I>(_tuple), true));
        iWeight += _setNextData_calcSize<I + 1>(_tuple, true);

        return iWeight;
    }
    */

    // 
    void _setNextData_allocSeq() {
        SnextSequence *sSeq(new SnextSequence());
        _updateSeqPtr_Set(sSeq);
    }

    void _setNextData_initSeq(const typeId_t &_ui8TypeId, const elemCnt_t &_ui16ElemCount, int &_iOffset) {
        _setNextData_allocSeq();
        m_ptrSeqCur->allocChunkMemory(_iOffset, sizeof(typeId_t) + _ui16ElemCount);
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
            iWeight += _var->calcsize();

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
};


namespace SK {
    auto SK__INTERNAL__VAR = []<typename T>(std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &_vec, T & _var) {
        _vec.push_back(std::make_unique<Cserializing::SvarInfo<T>>(_var)); \
    };

    auto SK__INTERNAL__VEC = []<typename... T>(T&&... args) {
        std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v; \
        (SK__INTERNAL__VAR(__v, args), ...); \
        return __v; \
    };
}

#define SK_SIERRA(...)   friend Cserializing; protected: std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v { SK::SK__INTERNAL__VEC(##__VA_ARGS__) };