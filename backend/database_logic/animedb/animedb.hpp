#ifndef __ANIME_DB_H__
#define __ANIME_DB_H__

#include "abstractdb.hpp"

class AnimeDB: public Database{
public:
    void CreateDatabase() override;
    ~AnimeDB() override = default;
};

#endif