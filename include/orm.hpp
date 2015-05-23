#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <atomic>
#include <map>
#include <cassert>
#include <libsqlite.hpp>


#pragma once

#define ORM_OBJECT(NAME) friend class orm_object< NAME >; \
        public: static std::string get_object_name() {return #NAME ;}\
        protected: NAME (unsigned int id) : orm_object(id){this->_load(id);}; private:\
        NAME () : orm_object< NAME >(){}

#define HAS_ONE_THROUGH(TYPE, THROUGH) public: std::shared_ptr<TYPE> get_ ## THROUGH (){return TYPE::get(_ ## THROUGH ## _id);};\
    private: unsigned int _ ## THROUGH ## _id;
#define HAS_ONE(TYPE) HAS_ONE_THROUGH(TYPE, TYPE)
#define HAS_ONE_FIELD(TYPE) HAS_ONE_THROUGH_FIELD(TYPE, TYPE)
//#define HAS_MANY(TYPE) public: std::vector<std::shared_ptr<TYPE>> get_ ## TYPE ## s (){}

namespace mvcpp {

enum class TYPE {string, integer, uinteger, primary};

template<typename T>
class _orm_field
{
public:
    TYPE type;
    bool is_relation = false;
    virtual std::string to_string(T*) = 0;
    void write(std::shared_ptr<T> obj, void* data)
    {
        this->write(obj.get(), data);
    }
    virtual void write(T* obj, void* data) = 0;
};

template<typename T, typename R>
class orm_field_base : public _orm_field<T>
{
public:
    R T::* ptr;
    std::string to_string(T* obj) override
    {
        std::stringstream s;
        s << (*obj).*(this->ptr);
        return s.str();
    }
    void write(T* obj, void* data) override
    {
        R* d = (R*) data;
        (*obj).*(this->ptr) = *d;
    }
    R read(T* obj)
    {
        return (*obj).*(this->ptr);
    }
};

template<typename T, typename R>
class orm_field : public orm_field_base<T, R>
{
};

template<typename T>
class orm_field<T, int> : public orm_field_base<T, int>
{
public:
    static TYPE type_by_template() {return TYPE::integer;}
};

template<typename T>
class orm_field<T, unsigned int> : public orm_field_base<T, unsigned int>
{
public:
    static TYPE type_by_template() {return TYPE::uinteger;}
};

template<typename T>
class orm_field<T, std::string> : public orm_field_base<T, std::string>
{
public:
    static TYPE type_by_template() {return TYPE::string;}
};

class database : public std::enable_shared_from_this<database>
{
public:
    class not_found_exception : public std::exception{};
    struct table_definition
    {
        struct field
        {
            std::string key;
            TYPE type;
        };

        std::string table_name;
        std::vector<field> fields;
        bool if_not_exists;
    };
    static database* current_database;
    virtual void create_table(const table_definition&) = 0;
    static std::string safe_field_name(std::string in)
    {
        std::transform(in.begin(), in.end(), in.begin(), 
            [](char ch) {
                char c = (ch == ' ' ? '_' : ch);
                return tolower(c);
            }
        );
        return in;
    }
    virtual std::string get_type_name_by_type(TYPE t)
    {
        switch(t)
        {
        case TYPE::string:
            return "TEXT";
            break;
        case TYPE::integer:
            return "INTEGER";
            break;
        case TYPE::uinteger:
            return "UNSIGNED INT";
            break;
        case TYPE::primary:
            return "INTEGER PRIMARY KEY";
            break;
        default:
            return "";
        }
    }
    virtual unsigned int insert(std::string table, std::vector<std::pair<std::string, std::string>>) = 0;
    virtual void update(std::string table, unsigned int id, std::vector<std::pair<std::string, std::string>>) = 0;
    virtual void fetch(const database::table_definition& def, unsigned int id, std::function<void (std::string field, void* data)> field_callback) = 0;
    virtual std::vector<unsigned int> fetch_all(std::string table) = 0;
};

template<class T>
class orm_object : public std::enable_shared_from_this<T>
{
    friend T;
    friend class database;
public:
    orm_object()
        : is_new(true)
    {
        orm_object<T>::_register_fields();
    };
    orm_object(unsigned long id)
        : is_new(false)
    {
        orm_object<T>::_register_fields();
    };
    static std::shared_ptr<T> get(unsigned int id)
    {
        std::shared_ptr<T> res(_cache[id].lock());
        if(!res)
        {
            res = std::shared_ptr<T>(new T(id));
            _cache[id] = res;
        }
        return res;
    }
    static std::vector<std::shared_ptr<T>> find_all()
    {
        std::vector<std::shared_ptr<T>> objects;
        auto ids = database::current_database->fetch_all(database::safe_field_name(T::get_object_name()));
        for(auto it : ids)
        {
            auto o = T::get(it);
            objects.push_back(o);
        }
        return objects;
    }
    static std::shared_ptr<T> factory(bool save, std::map<std::string, const void*> params)
    {
        orm_object<T>::_register_fields();
        std::shared_ptr<T> instance(new T);
        for(auto it = params.begin(); it != params.end(); ++it)
        {
            switch(T::_fields[it->first]->type)
            {
            case TYPE::string:
                {
                    std::string d((char*)it->second);
                    T::_fields[it->first]->write(instance, (void*)&d);
                    break;
                }
            case TYPE::integer:
                {
                    int d = *((int*)it->second);
                    T::_fields[it->first]->write(instance, (void*)&d);
                    break;
                }
            case TYPE::uinteger:
                {
                    unsigned int d = *((unsigned int*)it->second);
                    T::_fields[it->first]->write(instance, (void*)&d);
                    break;
                }
            default:
                break;
            }
        }
        if(save)
        {
            instance->save();
        }
        return instance;
    }
    static void create_tables(bool if_not_exists = false)
    {
        if(!_fields_registered.exchange(true))
            T::register_fields();
        database::table_definition def;
        def.if_not_exists = if_not_exists;
        def.table_name = database::safe_field_name(T::get_object_name());
        def.fields.push_back(database::table_definition::field{def.table_name + "_id", TYPE::primary});
        for(auto it = T::_fields.begin(); it != T::_fields.end(); ++it)
        {
            def.fields.push_back(
                    database::table_definition::field
                        {
                            database::safe_field_name(it->first),
                            it->second->type
                        }
                    );
        }
        database::current_database->create_table(def);
    };
    virtual void save()
    {
        if(!_fields_registered.exchange(true))
            T::register_fields();

        std::vector<std::pair<std::string, std::string>> fields;
        for(auto it = T::_fields.begin(); it != T::_fields.end(); ++it)
        {
            fields.push_back(std::make_pair<>(database::safe_field_name(it->first), it->second->to_string((T*)this)));
        }

        std::string table_name = database::safe_field_name(T::get_object_name());
        if(is_new)
            _id = database::current_database->insert(table_name, fields);
        else
            database::current_database->update(table_name, _id, fields);
        is_new.store(false);
        _cache[_id] = ((T*)(this))->shared_from_this();
    }
    unsigned int id()
    {
        return _id;
    }
    explicit operator unsigned int()
    {
        return id();
    }
    template<typename R>
    R value (const std::string field_name)
    {
        return ((orm_field_base<T, R>*)orm_object<T>::_fields.at(field_name))->read((T*)this);
    }
protected:
    unsigned int _id;
    void _load(unsigned int id)
    {
        database::table_definition def;
        def.table_name = database::safe_field_name(T::get_object_name());

        for(auto it = T::_fields.begin(); it != T::_fields.end(); ++it)
        {
            def.fields.push_back(
                    database::table_definition::field
                        {
                            database::safe_field_name(it->first),
                            it->second->type
                        }
                    );
        }
        database::current_database->fetch(def, id, [&](std::string field, void* data)
                {
                // Ugly for loop. Use the std::map
                    for(auto it = T::_fields.begin(); it != T::_fields.end(); ++it)
                    {
                        if(database::safe_field_name(it->first) == field)
                        {
                            it->second->write((T*)this, data);
                        }
                    }
                });
        _id = id;
    }
private:
    std::atomic_bool is_new;
    static void _register_fields()
    {
        if(!_fields_registered.exchange(true))
            T::register_fields();
    }
    template<typename R>
    static void field(R T::* field, const std::string field_name)
    {
        orm_field<T, R>* f = new orm_field<T, R>();
        f->type = orm_field<T, R>::type_by_template();
        f->ptr = field;
        orm_object<T>::_fields[field_name] = f;
    }

    static std::map<std::string, _orm_field<T>*> _fields;
    static std::map<unsigned int, std::weak_ptr<T>> _cache;
    static std::atomic_bool _fields_registered;
};
template<class T>
std::map<std::string, _orm_field<T>*> orm_object<T>::_fields = std::map<std::string, _orm_field<T>*>();
template<class T>
std::atomic_bool orm_object<T>::_fields_registered(false);
template<class T>
std::map<unsigned int, std::weak_ptr<T>> orm_object<T>::_cache = std::map<unsigned int, std::weak_ptr<T>> ();

}
