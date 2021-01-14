#include "NodesManager.h"

NodesManager::NodesManager(std::shared_ptr<DataBaseObject> const & db) : database(db)
{
    auto allNodesResult = database->allNodes();
    nodesSet = allNodesResult.first;
    nodesIDs = allNodesResult.second;
}

std::vector<std::string> NodesManager::getNodesList(std::uint32_t sessionToken, bool &success) const
{
    if (!database)
        throw std::runtime_error("Database wasn't initialized");

    success = true; // Just for test.

    std::vector<std::string> result;
    try{
        result = database->nodesQuery(sessionToken);
    } catch (std::runtime_error const & err)
    {
        success = false;
    }

    return result;
}

std::uint32_t NodesManager::generateID()
{
    if (nodesIDs.size() < 2)
        return rand(); // If nodes' set contains too few elements

    else
    {
        for (auto currNodeIter = nodesIDs.begin(); currNodeIter != --nodesIDs.end();)
        {
            int currID = *currNodeIter, nextID = *(++currNodeIter);
            if (nextID - currID > 1)
            {
                return rand() % (nextID  - currID - 1) + currID + 1; //Smart (i hope) generation
            }
        }

        if (*nodesIDs.end() != std::uint32_t{0xFFFFFFFF})
            nodesIDs.insert(rand() + *nodesIDs.end() + 1);
    }

    return 0; //Couldn`t generate a new node`s ID
}

std::uint32_t NodesManager::createNode(const std::uint32_t sessionToken, const long long lifeTimeInMins)
{
    if (!database)
        throw std::runtime_error("Database wasn't initialized");
    
    std::uint32_t generatedNodeID = generateID();
    if (!generatedNodeID)
        return 0;
    auto res = database->createNode(sessionToken, lifeTimeInMins, generatedNodeID);

    if (res)
    {
        nodesIDs.insert(generatedNodeID);
    }

    return res ? generatedNodeID : 0;
}
