#include "databaseobject.h"


DataBaseObject::DataBaseObject(std::string const &dbpath)
{
    try{
        driver = get_driver_instance();
        conn = driver->connect(Configuration::getDefaultPathDB(),
            Configuration::getDefaultUserDB(), Configuration::getDefaultPassDB());
        conn->setSchema("FileSharingSystem");
    }
    catch (sql::SQLException const &ex)
    {
        std::cout << ex.what() << std::endl;
    }
}

//Simple INSERT/DELETE query, without any returned values.
bool DataBaseObject::insertQuery(std::string querystr)
{
    sql::Statement* statement = conn->createStatement();

    try{
        statement->execute(querystr);
    }
    catch (sql::SQLException const &ex)
    {
        std::cout << ex.what() << std::endl;
        delete statement;
        return false;
    }

    delete statement;
    return true;
}

//Basic select query

sql::ResultSet* DataBaseObject::abstractSelectQuery(std::string const &query)
{
    sql::Statement* statement = conn->createStatement();
    sql::ResultSet* result = nullptr;
    
    try{
        result = statement->executeQuery(query);
    }

    catch (sql::SQLException const &ex)
    {
        std::cout << ex.what() << std::endl;
        delete statement;
        return nullptr;
    }
    delete statement;
    return result;
}

//Authorization method
bool DataBaseObject::authorizationQuery(std::string & email,
        std::string & password, size_t& userID)
{
    const std::string query = "select * from Users where Email = \'"
            + email + "\' and Password = \'" + password + "\';";
    bool returnValue = false;
    sql::ResultSet *result = abstractSelectQuery(query);
    
    if (result != nullptr && result->rowsCount() == 1)
    {
        result->next();
        userID = result->getInt("UserID");
        returnValue = true;
    }
    
    delete result;
    
    return returnValue;
}

//Method for new sessiong creating
bool DataBaseObject::createSessionQuery(std::uint32_t sessiongToken, int socketID, int userID)
{
    const std::string query = "insert into Sessions values (" + std::to_string(sessiongToken)
            + "," + std::to_string(socketID) + "," + std::to_string(userID) + ");";
    return insertQuery(query);
}

//Method for sessiong deleting
void DataBaseObject::closeSession(int socketFD)
{
    std::string query = "delete from Sessions where socketID = " + std::to_string(socketFD) + ";";
    insertQuery(query);
}

std::vector<std::string> DataBaseObject::nodesQuery(unsigned int sessionToken)
{
    const std::string query = "select * from Nodes where UserID = (select UserID from Sessions where sessionToken = " + std::to_string(sessionToken) + ");";
    sql::ResultSet *result = abstractSelectQuery(query);
    
    if (result == nullptr)
        throw std::runtime_error("Nodes query has been failed!");
    
    std::vector<std::string> nodes;
    while (result->next())
    {
        nodes.push_back(std::to_string(result->getInt("NodeID")));
    }

    return nodes;
}

DataBaseObject::~DataBaseObject()
{
    if (conn->isClosed())
        conn->close();
    delete conn;
}
