#include "crow_all.h"
#include <mysql/mysql.h>
#include <string>
#include <sstream>

// Replace with your own database credentials
#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "yourpassword"
#define DB_NAME "prateek_enterprises"

MYSQL* connect_db() {
    MYSQL* conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        crow::log << "DB Connection Failed: " << mysql_error(conn);
        return NULL;
    }
    return conn;
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/register").methods("POST"_method)([] (const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        std::string name = body["name"].s();
        std::string email = body["email"].s();
        std::string password = body["password"].s();

        MYSQL* conn = connect_db();
        if (!conn) return crow::response(500, "DB error");

        std::stringstream query;
        query << "INSERT INTO users(name, email, password) VALUES('" << name << "', '"
              << email << "', '" << password << "')";

        if (mysql_query(conn, query.str().c_str())) {
            mysql_close(conn);
            return crow::response(400, crow::json::wvalue{{"success", false}, {"message", mysql_error(conn)}});
        }

        mysql_close(conn);
        return crow::response(200, crow::json::wvalue{{"success", true}});
    });

    CROW_ROUTE(app, "/api/login").methods("POST"_method)([] (const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        std::string email = body["email"].s();
        std::string password = body["password"].s();

        MYSQL* conn = connect_db();
        if (!conn) return crow::response(500, "DB error");

        std::stringstream query;
        query << "SELECT id FROM users WHERE email='" << email << "' AND password='" << password << "' LIMIT 1";
        if (mysql_query(conn, query.str().c_str())) {
            mysql_close(conn);
            return crow::response(500, "Query failed");
        }

        MYSQL_RES* res = mysql_store_result(conn);
        bool found = (mysql_num_rows(res) > 0);
        mysql_free_result(res);
        mysql_close(conn);

        if (found)
            return crow::response(200, crow::json::wvalue{{"success", true}, {"token", "dummy-token"}});
        else
            return crow::response(401, crow::json::wvalue{{"success", false}});
    });

    app.port(5000).multithreaded().run();
} 
