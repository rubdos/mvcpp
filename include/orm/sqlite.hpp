#include <sstream>

#include "orm.hpp"

namespace mvcpp {
class sqlite : public database
{
    friend class database;
public:
    sqlite(std::string path = ":memory:")
        : database()
        , _db(path)
    {
    }
private:
    virtual void create_table(const table_definition& def) override
    {
        auto s = _db.get_statement();
        std::stringstream sql;
        sql << "CREATE TABLE ";
        if(def.if_not_exists)
            sql << "IF NOT EXISTS ";
        sql << "`" << def.table_name << "`"
            << " " << "(";
        for(auto it = def.fields.begin(); it != def.fields.end(); ++it)
        {
            sql << "`" << it->key << "` " << get_type_name_by_type(it->type) << (it+1 == def.fields.end() ? "":", ");
        }
        sql << ")";
        s->set_sql(sql.str());
        s->exec();
    }
    virtual unsigned int insert(std::string table, std::vector<std::pair<std::string, std::string>> fields) override
    {
        auto s = _db.get_statement();
        std::stringstream sql, values;
        sql << "INSERT INTO ";
        sql << "`" << table << "`"
            << " " << "(";
        values << " (";
        for(auto it = fields.begin(); it != fields.end(); ++it)
        {
            sql << "`" << it->first << "`" << (it+1 == fields.end() ? ")":", ");
            values << "'" << it->second << "'" << (it+1 == fields.end() ? ")":", ");
        }
        sql << " VALUES " << values.str();
        s->set_sql(sql.str());
        s->exec();
        return _db.last_insert_id();
    }
    virtual void update(std::string table, unsigned int id, std::vector<std::pair<std::string, std::string>> fields) override
    {
        auto s = _db.get_statement();
        std::stringstream sql;
        sql << "UPDATE ";
        sql << "`" << table << "` SET ";
        for(auto it = fields.begin(); it != fields.end(); ++it)
        {
            sql << "`" << it->first << "` = " << "'" << it->second << "'" << (it+1 == fields.end() ? "":", ");
        }
        sql << " WHERE `" << table << "_id` = " << id;
        std::cout << sql.str() << std::endl;
        s->set_sql(sql.str());
        s->exec();
    }
    virtual void fetch(const database::table_definition& def, unsigned int id, std::function<void (std::string field, void* data)> field_callback)
    {
        auto s = _db.get_statement();
        std::stringstream sql;
        sql << "SELECT ";
        for(auto it = def.fields.begin(); it != def.fields.end(); ++it)
        {
            sql << "`" << it->key << "`" << (it+1 == def.fields.end() ? "":", ");
        }
        sql << " FROM "
            << "`" << def.table_name << "` WHERE `" << def.table_name << "_id`=" << id;
        s->set_sql(sql.str());
        s->prepare();
        if(!s->step())
            throw database::not_found_exception();

        size_t i = 0;
        for(auto it = def.fields.begin(); it != def.fields.end(); ++it)
        {
            switch(it->type)
            {
            case TYPE::string:
                {
                    std::string data(s->get_text(i));
                    field_callback(it->key, &data);
                    break;
                }
            case TYPE::integer:
                {
                    int data(s->get_int(i));
                    field_callback(it->key, &data);
                    break;
                }
            case TYPE::uinteger:
                {
                    unsigned int data(s->get_int(i));
                    field_callback(it->key, &data);
                    break;
                }
            default:
                break;
            }
            ++i;
        }
    }
    virtual std::vector<unsigned int> fetch_all(std::string table) override
    {
        std::vector<unsigned int> _ids;
        std::stringstream sql;
        auto s = _db.get_statement();
        sql << "SELECT `" << table << "_id" << "` FROM `" << table << "`";
        s->set_sql(sql.str());
        s->prepare();
        while(s->step())
        {
            _ids.push_back(s->get_int(0));
        }
        return _ids;
    }

    ::sqlite::sqlite _db;
};
}
