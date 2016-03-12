//
//  DataStore.hpp
//  Chat
//
//  Created by Cho Chong on 2/10/16.
//  Copyright © 2016 Cho Chong. All rights reserved.
//

#ifndef DataStore_hpp
#define DataStore_hpp

#include <stdio.h>
#include <map>
#include "Record.hpp"



class DataStore
{
public:
    DataStore();
    ~DataStore();
    
    void Add(Model::Record* rec);
    void Delete(const Model::Record* rec);
    void Update(const Model::Record* rec);
    
private:
    bool IsDirty;
    std::map<ID_T, Model::Record> Records;
};

#endif /* DataStore_hpp */
