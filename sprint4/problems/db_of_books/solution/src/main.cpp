#include <iostream>
#include <pqxx/pqxx>
#include <optional>
#include <boost/json.hpp>


namespace json = boost::json;
using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;

void CreateTable(pqxx::connection& connection) {
    pqxx::work work(connection);
    work.exec(
        "CREATE TABLE IF NOT EXISTS books ("
        "id SERIAL PRIMARY KEY,"
        " title varchar(100) NOT NULL,"
        " author varchar(100) NOT NULL,"
        " year integer NOT NULL,"
        " isbn char(13) UNIQUE NULL"
        ");"_zv); //создаем БД
    work.commit();
}

json::object MakeActionResponse(bool result) {
    json::object response;
    response["result"] = result;
    return response;
}

int main(int argc, const char* argv[]) {
    try {
        if (argc == 1) {
            std::cout << "Usage: book_manager <conn-string>\n"sv;
            return EXIT_SUCCESS;
        } else if (argc != 2) {
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }

        // Подключаемся к БД, указывая её параметры в качестве аргумента
        pqxx::connection conn{argv[1]};
        CreateTable(conn);

        constexpr auto add_book = "add_book"_zv;
        conn.prepare(add_book, "INSERT INTO books (title, author, year, isbn) VALUES ($1, $2, $3, $4)"_zv);


        // Транзакция нужна, чтобы выполнять запросы.


        std::string action;
        while (std::getline(std::cin, action)) {


            if (action.empty()) {
                continue;
            }

            json::value root = json::parse(action);
            json::object& obj = root.as_object();
            std::string action_type = json::value_to<std::string>(obj.at("action"));

            if (action_type == "exit") {
                break;
            }


            if (action_type == "add_book") {
                bool result = true;
                try {
                    json::object& payload = obj.at("payload").as_object();
                    std::string title = json::value_to<std::string>(payload.at("title"));
                    std::string author = json::value_to<std::string>(payload.at("author"));
                    int year = payload.at("year").as_int64();
                    json::value isbn_value = payload.at("ISBN");

                    pqxx::work w(conn);
                    std::optional<std::string> isbn;
                    if (!isbn_value.is_null()) {
                        isbn = json::value_to<std::string>(isbn_value);
                    }
                    w.exec_prepared(add_book,title, author, year, isbn);
                    w.commit();
                } catch (const pqxx::sql_error&) {
                    result = false;
                }
                std::cout << json::serialize(MakeActionResponse(result)) << '\n';
                continue;
            }

            if (action_type == "all_books") {
                pqxx::read_transaction read{conn};
                pqxx::result rows = read.exec("SELECT id, title, author, year, ISBN FROM books "
                                              "ORDER BY year DESC, title ASC, author ASC, ISBN ASC NULLS LAST");
                json::array out;
                out.reserve(rows.size());
                for (const auto& row: rows) {
                    json::object book;
                    book["id"] = row["id"].as<int>();
                    book["title"] = row["title"].c_str();
                    book["author"] = row["author"].c_str();
                    book["year"] = row["year"].as<int>();
                    if (row["ISBN"].is_null()) {
                        book["ISBN"] = nullptr;
                    } else {
                        book["ISBN"] = row["ISBN"].c_str();
                    }
                    out.emplace_back(std::move(book));
                }
                std::cout << json::serialize(out) << '\n';
                continue;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
