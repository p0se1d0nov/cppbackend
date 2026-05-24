#pragma once
#include "connection_pool.h"
#include <string>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace postgres {

struct PlayerRecord {
    std::string name;
    int score;
    double play_time;
};

class Database {
public:
    explicit Database(ConnectionPool& pool);

    void SaveRecord(const std::string& name, int score, double play_time);
    std::vector<PlayerRecord> GetRecords(size_t start, size_t limit);

private:
    ConnectionPool& pool_;
};

}
