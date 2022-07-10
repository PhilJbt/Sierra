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


struct StypeInfos {
public:
    StypeInfos(typeId_t _ui8TypeId, typeSize_t _ui32Size, std::string _strTypeName)
        : m_ui8TypeId(_ui8TypeId), m_ui32Size(_ui32Size), m_strTypeName(_strTypeName) {}

    typeId_t        typeId()   { return m_ui8TypeId;   };
    typeSize_t      size()     { return m_ui32Size;    };
    std::string     typeName() { return m_strTypeName; };

private:
    typeId_t        m_ui8TypeId   = 0;
    typeSize_t      m_ui32Size    = 0;
    std::string     m_strTypeName = "";
};


typedef std::unordered_map<std::string, StypeInfos *> mapTypesInfos_byName;
typedef std::unordered_map<typeId_t, StypeInfos *>    mapTypesInfos_byId;
typedef std::vector<StypeInfos *>                     vecTypesIdsNames;


/*
** 
*
*/
class Cserializing {
public:
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
            _ptrSerializer->_setNextData_template(m_val);
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

        virtual void                                                       ptrVar(Cserializing *_ptrSerializer) const {}

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

        void ptrVar(Cserializing *_ptrSerializer) const {
            //m_ptrVar->populate(_ptrSerializer);
            _ptrSerializer->_setNextData_template(*m_ptrVar);
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


    enum Eoperation_ {
        Eoperation_NULL,
        Eoperation_Get,
        Eoperation_Set
    };


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


    template<typename T>
    bool nextDataType(const T &_var, typename std::enable_if<!std::is_pointer<T>::value >::type* = 0) {
        return _nextDataType(_var);
    }

    bool _nextDataType(const std::string &_str) {
        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byName("std::string", &sTypeInfos);

        return _isNextData(false, { sTypeInfos->typeId() });
    }

    template<typename U>
    bool _nextDataType(const std::vector<U> &_vec) {
        StypeInfos *sTypeInfos_vec(nullptr);
        StypeInfos *sTypeInfos_var(nullptr);
        getTypeInfos_byName("std::vector", &sTypeInfos_vec);
        getTypeInfos_byType(U(), &sTypeInfos_var);

        return _isNextData(false, { sTypeInfos_vec->typeId(), sTypeInfos_var->typeId() });
    }

    template<typename T>
    bool _nextDataType(const T &_var) {
        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byType(_var, &sTypeInfos);

        return _isNextData(false, { sTypeInfos->typeId() });
    }

    /*template <typename T, size_t N>
    bool _nextDataType(T(&_var)[N]) {
        typeId_t ui8TypeId(0);
        getTypeInfos_byType(_var[0], &ui8TypeId, nullptr, nullptr);

        return _isNextData(false, ui8TypeId) && (static_cast<dataCount_t>(N) == static_cast<dataCount_t>(isNextCount()));
    }*/


    int nextDataSize() {
        dataCount_t ui16Count(0);
        ::memcpy(&ui16Count, &m_vecBuffer[m_iIndex], sizeof(ui16Count));
        return static_cast<int>(ui16Count);
    }


    template<typename T>
    void setNextData(const T &_var, typename std::enable_if<!std::is_pointer<T>::value >::type* = 0) {
        if (!_setNextData_structvar(_var))
            _setNextData_template(_var);
    }


    template<typename T>
    void getNextData(T &_var, typename std::enable_if<!std::is_pointer<T>::value>::type* = 0) {
        constexpr bool has_varInfos = requires(T & _var) {
            _var.__v;
        };

        if constexpr (has_varInfos) {
            _getNextData_template(_var);


        }
        else
            _getNextData_template(_var);
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


    template<typename T>
    static void getTypeInfos_byType(const T &_t, StypeInfos **_out_sTypeInfos) {
        std::string strTypeName(std::type_index(typeid(const_cast<T &>(_t))).name());
        if (!isTypeRegistered_byName(strTypeName))
            throw std::runtime_error("Type not registered");
        const mapTypesInfos_byName::iterator &&itElem(s_mapTypes_byName.find(strTypeName));
        
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
        typeId_t         ui8TypeId(static_cast<typeId_t>(s_mapTypes_byName.size()) + 1);
        typeSize_t       ui32Size(sizeof(_t));
        std::type_index  type(typeid(_t));
        std::string      strTypeName(typeid(_t).name());
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


    template<typename T>
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
    }

    void _getNextData_varInfo(const SvarInfo_base &_ptr) {
        _chkOperation(Eoperation_Get);

        m_iIndex += sizeof(typeId_t);

        dataCount_t ui16Count(_isNextCount());
        uint32_t    ui32DataSize(_ptr.typeIdsNames()[0]->size() * ui16Count);

        ::memcpy(_ptr.ptrNode(), &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    /*template <typename T, size_t N>
    void _getNextData_template(const T(&_var)[N]) {
        _chkOperation(Eoperation_Get);

        dataCount_t ui16Count(_isNextCount());

        typeId_t   ui8TypeId(0);
        typeSize_t ui32TypeSize(0);
        getTypeInfos_byType(T(), &ui8TypeId, &ui32TypeSize, nullptr);
        _chkExpectedType(ui8TypeId);

        uint32_t ui32DataSize(ui32TypeSize * ui16Count);

        ::memcpy(_var, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }*/

    template<typename T>
    void _getNextData_template(T &_var) {
        _chkOperation(Eoperation_Get);

        dataCount_t ui16Count(_isNextCount());

        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byType(T(), &sTypeInfos);
        _chkExpectedType({ sTypeInfos->typeId() });

        uint32_t ui32DataSize(sTypeInfos->size() * ui16Count);

        ::memcpy((void *)&_var, &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }
    
    void _getNextData_template(std::string &_var) {
        _chkOperation(Eoperation_Get);

        dataCount_t ui16Count(_isNextCount());

        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byName("std::string", &sTypeInfos);
        _chkExpectedType({ sTypeInfos->typeId() });

        uint32_t ui32DataSize(sTypeInfos->size() * ui16Count);

        _var = std::string(reinterpret_cast<char*>(&m_vecBuffer[m_iIndex]), ui32DataSize);

        _incrementIndex(ui32DataSize);
    }

    template<typename U>
    void _getNextData_template(std::vector<U> &_vec) {
        _chkOperation(Eoperation_Get);

        dataCount_t ui16Count(_isNextCount());

        StypeInfos *sTypeInfos_vec(nullptr);
        StypeInfos *sTypeInfos_var(nullptr);
        getTypeInfos_byName("std::vector", &sTypeInfos_vec);
        getTypeInfos_byType(U(), &sTypeInfos_var);
        _chkExpectedType({ sTypeInfos_vec->typeId(), sTypeInfos_var->typeId() });

        uint32_t ui32DataSize(sTypeInfos_var->size() * static_cast<uint32_t>(ui16Count));
        _vec.resize(ui16Count);

        ::memcpy(&_vec[0], &m_vecBuffer[m_iIndex], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename U, typename V>
    void _getNextData_template(std::map<U, V> &_map) {
        _chkOperation(Eoperation_Get);

        dataCount_t ui16Count(_isNextCount());

        StypeInfos *sTypeInfos_map(nullptr),
                   *sTypeInfos_var1(nullptr),
                   *sTypeInfos_var2(nullptr);
        getTypeInfos_byName("std::map", &sTypeInfos_map);
        getTypeInfos_byType(U(), &sTypeInfos_var1);
        getTypeInfos_byType(V(), &sTypeInfos_var2);
        _chkExpectedType({ sTypeInfos_map->typeId(), sTypeInfos_var1->typeId(), sTypeInfos_var2->typeId() });

        typeSize_t ui32SizeType1(sizeof(U)),
                   ui32SizeType2(sizeof(V));

        for (uint16_t i(0); i < ui16Count; ++i) {
            U key;
            ::memcpy(&key, &m_vecBuffer[m_iIndex], ui32SizeType1);
            _incrementIndex(ui32SizeType1);

            V val;
            ::memcpy(&val, &m_vecBuffer[m_iIndex], ui32SizeType2);
            _incrementIndex(ui32SizeType2);

            _map.insert({ key, val });
        }
    }


    template <typename T>
    void _setNextData_template(const T &_var, const int &_iCount = 1) {
        _chkOperation(Eoperation_Set);

        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byType(T(), &sTypeInfos);

        _setHeader(_iCount, { sTypeInfos->typeId()});

        uint32_t ui32DataSize(sTypeInfos->size() * _iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &_var, ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template <typename T, size_t N>
    void _setNextData_template(const T(&_var)[N]) {
        _chkOperation(Eoperation_Set);

        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byType(_var[0], &sTypeInfos);

        int iCount(N);
        _setHeader(iCount, { sTypeInfos->typeId()});

        uint32_t ui32DataSize(sTypeInfos->size() * iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], _var, ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    template<typename U>
    void _setNextData_template(const std::vector<U> &_vec) {
        _chkOperation(Eoperation_Set);

        StypeInfos *sTypeInfos_vec(nullptr);
        StypeInfos *sTypeInfos_var(nullptr);
        getTypeInfos_byName("std::vector", &sTypeInfos_vec);
        getTypeInfos_byType(_vec[0], &sTypeInfos_var);

        int iCount(static_cast<int>(_vec.size()));
        _setHeader(iCount, { sTypeInfos_vec->typeId(), sTypeInfos_var->typeId() });

        uint32_t ui32DataSize(sTypeInfos_var->size() * iCount);
        _reserveBufferSize(ui32DataSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &_vec[0], ui32DataSize);
        _incrementIndex(ui32DataSize);
    }

    /*template<typename U, typename V>
    void _setNextData_template(const std::map<U, V> *_map) {
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
            ::memcpy(&m_vecBuffer[m_iIndex], &key, ui32SizeType1);
            _incrementIndex(ui32SizeType1);
            ::memcpy(&m_vecBuffer[m_iIndex], &val, ui32SizeType2);
            _incrementIndex(ui32SizeType2);
        }
    }*/

    void _setNextData_template(const std::string &_str) {
        _chkOperation(Eoperation_Set);

        std::string str(_str.c_str() + '\0');

        StypeInfos *sTypeInfos(nullptr);
        getTypeInfos_byName("std::string", &sTypeInfos);

        typeSize_t ui32TypeSize(static_cast<int>(str.length()) * sTypeInfos->size());

        _setHeader(ui32TypeSize, { sTypeInfos->typeId()});

        _reserveBufferSize(ui32TypeSize);

        ::memcpy(&m_vecBuffer[m_iIndex], &str[0], ui32TypeSize);
        _incrementIndex(ui32TypeSize);
    }

    template<typename T>
    bool _setNextData_structvar(const T &_var) {
        constexpr bool has_varInfos = requires(T &_var) {
            _var.__v;
        };

        if constexpr (has_varInfos) {
            _chkOperation(Eoperation_Set);

            StypeInfos *sTypeInfos_UserStruct(nullptr);
            getTypeInfos_byType(_var, &sTypeInfos_UserStruct);
            _setHeader(1, { sTypeInfos_UserStruct->typeId() });

            for (int i(0); i < _var.__v.size(); ++i)
                _setNextData_structvar_run(*_var.__v[i].get());
        }
        else
            return false;

        return true;
    }

    void _setNextData_structvar_run(const SvarInfo_base &_var) {
        if (_var.isInfoVec()) {
            //std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> &vec = (*reinterpret_cast<std::vector<std::unique_ptr<Cserializing::SvarInfo_base>>>(_var.ptrNode()));
            std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> *vec = reinterpret_cast<std::vector<std::unique_ptr<Cserializing::SvarInfo_base>>*>(_var.ptrNode());
            
            for (int i(0); i < vec->size(); ++i)
                _setNextData_structvar_run(*vec->at(i).get());
        }
        else
            _setNextData_structvar_var(_var);
    }

    void _setNextData_structvar_var(const SvarInfo_base &_var) {
        const std::string strTypeName(_var.typeIdsNames()[0]->typeName());

        if (strTypeName == "std::string") {
            //const std::string &str(reinterpret_cast<std::string &>(*reinterpret_cast<std::string *>(_var.ptr())));
            //_setNextData_template(str);
            _var.ptrVar(this);
        }
        else if (strTypeName == "std::vector") {
            //_var.populate(this);
            _var.ptrVar(this);

            //using t = decltype(_var.typeIdsNames()[1]->type());

            //std::vector<t> *vec(reinterpret_cast<std::vector<t>*>(_var.ptr()));
            //_setNextData_template(*vec);
        }
        else if(strTypeName == "std::map") {
            _var.ptrVar(this);
        }
        else {
            _var.ptrVar(this);
            /*dataCount_t ui16Count(_var.count());
            int iDataSize(_var.typeIdsNames()[0]->size() * ui16Count);
            _reserveBufferSize(sizeof(ui16Count) + iDataSize);

            ::memcpy(&m_vecBuffer[m_iIndex], &ui16Count, sizeof(ui16Count));
            m_iIndex += sizeof(ui16Count);

            if (ui16Count > 0) {
                ::memcpy(&m_vecBuffer[m_iIndex], _var.ptr(), iDataSize);
                m_iIndex += iDataSize;
            }*/
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

    bool _isNextData(bool _bWalking, std::vector<typeId_t> _vec) {
        _vec.push_back(0);
        int iIndex((_bWalking ? m_iIndex : _isNextData_seekSeq()));

        for (int i(0); i < _vec.size(); ++i) {
            if (_vec[i] != static_cast<typeId_t>(m_vecBuffer[iIndex]))
                return false;
            iIndex += sizeof(typeId_t);
        }

        if (_bWalking)
            m_iIndex = iIndex;

        return true;
    }

    inline void _chkExpectedType(std::vector<typeId_t> _vec) {
        if (!_isNextData(true, _vec))
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

    static mapTypesInfos_byName s_mapTypes_byName;
    static mapTypesInfos_byId   s_mapTypes_byId;
    static std::atomic<bool>    s_bRegisterDone;
};


#define TEST2(VAR)  Cserializing::SvarInfo<decltype(VAR)>(VAR)
#define TEST(...)   friend class Cserializing; protected: std::vector<std::unique_ptr<Cserializing::SvarInfo_base>> __v { Cserializing::__v_push(##__VA_ARGS__) };