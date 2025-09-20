#ifndef MAIN_H
#define MAIN_H

#include "config/database.h"
#include "utils/db_pool.h"
#include "models/user_dao.h"
#include "services/user_service.h"
#include <memory>

class LowAltitudeTrafficSystem {
private:
    config::AppConfig app_config_;
    std::unique_ptr<utils::DatabaseConnectionPool> db_pool_;

public:
    LowAltitudeTrafficSystem();
    ~LowAltitudeTrafficSystem();

    bool initialize();
    void run();
    void shutdown();

private:
    bool initialize_database();
    bool initialize_services();
};

#endif // MAIN_H