//
//  DataStore.cpp
//  Chat
//
//  Created by Cho Chong on 2/10/16.
//  Copyright © 2016 Cho Chong. All rights reserved.
//

#include "DataStore.hpp"

using namespace Model;

DataStore::DataStore() : IsDirty(false)
{
    
}

DataStore::~DataStore()
{
    
}

void DataStore::Add(Record* rec)
{
    if(Records.find(rec->GetId()) != Records.end())
    {
        Records[rec->GetId()] = *rec;
    }
}

void DataStore::Delete(const Record* rec)
{
    Records.erase(rec->GetId());
}

void DataStore::Update(const Record* rec)
{
    auto record = Records[rec->GetId()];
    
    record = *rec;
}