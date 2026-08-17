#pragma once

#include <mysql/mysql.h>
#include <string>

class UserManager
{
private:
    MYSQL *mysql_;

public:
    UserManager(const std::string &host,
                const std::string &user,
                const std::string &password,
                const std::string &db,
                unsigned int port = 3306)
        : mysql_(nullptr)
    {
        mysql_ = mysql_init(nullptr);

        mysql_real_connect(mysql_,
                           host.c_str(),
                           user.c_str(),
                           password.c_str(),
                           db.c_str(),
                           port,
                           nullptr,
                           0);
        mysql_set_character_set(mysql_, "utf8");
    }

    ~UserManager()
    {
        if (nullptr != mysql_)
        {
            mysql_close(mysql_);
        }
    }

    std::string Register(const std::string &username, const std::string &password)
    {
        if (username.empty())
        {
            return "用户名不能为空";
        }

        if (password.empty())
        {
            return "密码不能为空";
        }

        char user_buf[256];
        char pwd_buf[256];

        mysql_real_escape_string(mysql_, user_buf, username.c_str(), username.size());
        mysql_real_escape_string(mysql_, pwd_buf, password.c_str(), password.size());

        std::string check_sql = "select id from user where username = '";
        check_sql += user_buf;
        check_sql += "';";

        if (0 != mysql_query(mysql_, check_sql.c_str()))
        {
            return "查询用户失败";
        }

        MYSQL_RES *res = mysql_store_result(mysql_);
        if (nullptr == res)
        {
            return "查询用户失败";
        }

        MYSQL_ROW row = mysql_fetch_row(res);
        if (nullptr != row)
        {
            mysql_free_result(res);
            return "用户名已经存在";
        }

        mysql_free_result(res);

        std::string insert_sql = "insert into user(username, password) values('";
        insert_sql += user_buf;
        insert_sql += "', '";
        insert_sql += pwd_buf;
        insert_sql += "');";

        if (0 != mysql_query(mysql_, insert_sql.c_str()))
        {
            return "注册失败";
        }
        return "注册成功";
    }

    std::string Login(const std::string &username, const std::string &password)
    {
        char user_buf[256];
        char pwd_buf[256];

        mysql_real_escape_string(mysql_, user_buf, username.c_str(), username.size());
        mysql_real_escape_string(mysql_, pwd_buf, password.c_str(), password.size());

        std::string sql = "select password from user where username = '";
        sql += user_buf;
        sql += "';";

        if (0 != mysql_query(mysql_, sql.c_str()))
        {
            return "查询用户失败";
        }

        MYSQL_RES *res = mysql_store_result(mysql_);
        if (nullptr == res)
        {
            return "查询用户失败";
        }

        MYSQL_ROW row = mysql_fetch_row(res);
        if(nullptr == row)
        {
            mysql_free_result(res);
            return "用户名不存在";
        }
        std::string db_password = row[0] == nullptr ? "" : row[0];
        mysql_free_result(res);

        if(password != db_password)
        {
            return "密码错误";
        }

        return "登录成功";
    }

};