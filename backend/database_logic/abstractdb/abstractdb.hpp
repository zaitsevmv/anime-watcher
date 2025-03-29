#ifndef __ABSTRACT_DB_H__
#define __ABSTRACT_DB_H__


class Database{
public:
    virtual void CreateDatabase() = 0;
    virtual ~Database() = default;
};

#endif