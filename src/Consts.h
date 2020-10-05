/************************************************************************************
   Copyright (C) 2020 MariaDB Corporation AB

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not see <http://www.gnu.org/licenses>
   or write to the Free Software Foundation, Inc.,
   51 Franklin St., Fifth Floor, Boston, MA 02110, USA
*************************************************************************************/


/* MariaDB Connector/C++ Generally used consts, types definitions, enums,  macros, small classes, templates */

#ifndef __CONSTS_H_
#define __CONSTS_H_

#include <map>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <stdexcept>
#include <cstring>

#include "StringImp.h"
#include "Version.h"
#include "util/ServerStatus.h"
#include "util/String.h"

#include "ResultSet.h"
#include "CallableStatement.h"
#include "Statement.h"
#include "Connection.h"
#include "ResultSetMetaData.h"
#include "ParameterMetaData.h"

#include "Charset.h"
#include "MariaDbServerCapabilities.h"

#include "options/Options.h"
#include "logger/Logger.h"

/* Helper to write ClassField map initializer list */
#define CLASS_FIELD(_CLASS, _FIELD) {#_FIELD, &_CLASS::_FIELD}
#define INSTANCEOF(OBJ, CLASSNAME) (OBJ != nullptr && dynamic_cast<CLASSNAME>(OBJ) != NULL)


namespace sql
{

template <class T> CArray<T>::CArray(int64_t len) : arr(nullptr), length(len)
{
  if (length < 0)
  {
    throw std::invalid_argument("Invalid length");
  }
  if (length > 0)
  {
    arr= new T[static_cast<size_t>(length)];
    if (arr == NULL)
    {
      throw std::runtime_error("Could not allocate memory");
    }
  }
}


template <class T> CArray<T>::CArray(int64_t len, const T& fillValue) : CArray<T>(len)
{
  std::fill(this->begin(), this->end(), fillValue);
}

#ifndef _WIN32
# define ZEROI64 0LL
#else
# define ZEROI64 0I64
#endif
/* This constructor takes existin(stack?) array for "storing". Won't delete */
template <class T> CArray<T>::CArray(T _arr[], size_t len) : arr(_arr), length(ZEROI64 - len)
{
}


template <class T> CArray<T>::CArray(const T _arr[], size_t len)
  : CArray(len)
{
  std::memcpy(arr, _arr, len*sizeof(T));
}


template <class T> CArray<T>::~CArray()
{
  if (arr != NULL && length > 0)
  {
    delete[] arr;
  }
}


template <class T> CArray<T>::CArray(std::initializer_list<T> const& initList) : CArray(initList.end() - initList.begin())
{
  std::copy(initList.begin(), initList.end(), arr);
}


/*template <class T> CArray<T>::CArray(CArray&& rhs)
  : arr(rhs.arr)
  , length(rhs.length)
{
  if (rhs.length > 0)
  {
    rhs.arr= nullptr;
    length = 0;
  }
}*/

template <class T> CArray<T>::CArray(const CArray& rhs)
  : arr(rhs.arr)
  , length(rhs.length)
{
  if (length > 0)
  {
    arr= new T[static_cast<size_t>(length)];
    std::memcpy(arr, rhs.arr, length);
  }
}


template <class T> T* CArray<T>::end()
{
  return arr + (length > 0 ? length : -length);
}


template <class T> const T* CArray<T>::end() const
{
  return arr + (length > 0 ? length : -length);
}


template <class T> void CArray<T>::assign(const T* _arr, std::size_t size)
{
  if (size == 0)
  {
    if (length == 0)
    {
      throw std::invalid_argument("Size is not given, and the array is not yet allocated");
    }
    else
    {
      size= this->size();
    }
  }
  else if (size > this->size())
  {
    if (arr == nullptr/* && length == 0*/)
    {
      length= size;
      arr= new T[size];
    }
    else
    {
      throw std::invalid_argument("Size is greater, then array's capacity");
    }
  }

  std::memcpy(arr, _arr, size*sizeof(T));
}

template <class T> void CArray<T>::wrap(T* _arr, std::size_t size)
{
  if (length > 0/* && arr != nullptr*/)
  {
    delete[] arr;
  }

  arr= _arr;
  if (arr == nullptr)
  {
    length= 0;
  }
  else
  {
    length= ZEROI64 - size;
  }
}


template <class T> void CArray<T>::reserve(std::size_t size)
{
  if (size > 0)
  {
    if (length > 0)
    {
      if (static_cast<std::size_t>(length) < size)
      {
        delete[] arr;

      }
      else
      {
        return;
      }
    }
    arr= new T[size];
    length= size;
  }
  // else deallocate?
}
/*template <class T> T* operator+(CArray<T>& arr, size_t offset)
{
  // Should check the range? and return no further than the end of the array?
  return static_cast<T*>(arr) + offset;
}*/

namespace mariadb
{
  enum HaMode
  {
    NONE=0,
    AURORA,
    REPLICATION,
    SEQUENTIAL,
    LOADBALANCE
  };

  enum ColumnFlags {
    NOT_NULL = 1,
    PRIMARY_KEY = 2,
    UNIQUE_KEY = 4,
    MULTIPLE_KEY = 8,
    BLOB = 16,
    UNSIGNED = 32,
    DECIMAL = 64,
    BINARY_COLLATION = 128,
    ENUM = 256,
    AUTO_INCREMENT = 512,
    TIMESTAMP = 1024,
    SET = 2048
  };

  /* Also probably temporary location. Using it as it's generally included by everybody */
  /* typedefs for shared_ptr types, so the code looks nicer and cleaner */
  class Pool;
  class Protocol;
  class Listener;
  class MariaDbConnection;
  class MariaDbStatement;
  class Results;
  class CmdInformation;
  class Buffer;
  class ClientPrepareResult;
  //class ClientSidePreparedStatement;
  class ServerSidePreparedStatement;
  class SelectResultSet;
  typedef ServerSidePreparedStatement ClientSidePreparedStatement;
  typedef SelectResultSet UpdatableResultSet;
  class ServerPrepareResult;
  class MariaDbParameterMetaData;
  class MariaDbResultSetMetaData;
  class CallableParameterMetaData;
  class ColumnDefinition;
  class Credential;
  class ParameterHolder;
  class RowProtocol;
  class SelectResultSet;
  class ExceptionFactory;

  class CloneableCallableStatement : public CallableStatement
  {
    CloneableCallableStatement(const CloneableCallableStatement&)=delete;
    void operator=(CloneableCallableStatement&)=delete;
  public:
    CloneableCallableStatement()=default;
    virtual ~CloneableCallableStatement() {}
    virtual CloneableCallableStatement* clone(MariaDbConnection* connection)=0;
  };


  extern std::map<std::string, enum HaMode> StrHaModeMap;
  extern const char* HaModeStrMap[LOADBALANCE + 1];
  extern const SQLString emptyStr;
  extern const char QUOTE;
  extern const char DBL_QUOTE;
  extern const char ZERO_BYTE;
  extern const char BACKSLASH;


  struct ParameterConstant
  {
    static const SQLString TYPE_MASTER;// = "master";
    static const SQLString TYPE_SLAVE;// = "slave";
  };


  /* Temporary here, need to find better place for it */
  template <class T> int64_t hash(T v)
  {
    return static_cast<int64_t>(std::hash<T>{}(v));
  }

#define HASHCODE(_expr) std::hash<decltype(_expr)>{}(_expr)

  template <class KT, class VT> int64_t hashMap(std::map<KT, VT>& v)
  {
    int64_t result= 0;
    for (auto it : v)
    {
      /* I don't think it's guaranteed, that pairs returned in same order, and we need the same value for same map content */
      result+= hash<KT>(it.first) ^ (hash<KT>(it.second) << 1);
    }

    return result;
  }

  namespace Shared
  {
    typedef std::shared_ptr<std::mutex> mutex;

    typedef std::shared_ptr<sql::mariadb::Options> Options;
    typedef std::shared_ptr<sql::mariadb::Logger> Logger;
    typedef std::shared_ptr<sql::mariadb::Pool> Pool;
    typedef std::shared_ptr<sql::mariadb::Protocol> Protocol;
    typedef std::shared_ptr<sql::mariadb::Listener> Listener;
    typedef std::shared_ptr<sql::mariadb::CmdInformation> CmdInformation;
    typedef std::shared_ptr<sql::mariadb::Results> Results;
    typedef std::shared_ptr<sql::mariadb::Buffer> Buffer;
    typedef std::shared_ptr<sql::mariadb::ClientPrepareResult> ClientPrepareResult;
    typedef std::shared_ptr<sql::mariadb::MariaDbResultSetMetaData> MariaDbResultSetMetaData;
    typedef std::shared_ptr<sql::mariadb::MariaDbParameterMetaData> MariaDbParameterMetaData;
    typedef std::shared_ptr<sql::mariadb::CallableParameterMetaData> CallableParameterMetaData;
    typedef std::shared_ptr<sql::mariadb::ColumnDefinition> ColumnDefinition;
    typedef std::shared_ptr<sql::mariadb::ParameterHolder> ParameterHolder;
    typedef std::shared_ptr<sql::mariadb::SelectResultSet> SelectResultSet;
    typedef std::shared_ptr<sql::mariadb::ExceptionFactory> ExceptionFactory;
    /* Shared sql classes - there is probably no sense to keep them in sql::Shared */
    typedef std::shared_ptr<sql::CallableStatement> CallableStatement;
    typedef std::shared_ptr<sql::ResultSetMetaData> ResultSetMetaData;
    typedef std::shared_ptr<sql::Connection> Connection;
    typedef std::shared_ptr<sql::ParameterMetaData> ParameterMetaData;
  }

  namespace Unique
  {
    /* Unique sql classes */
    /* atm I doubt we need these three */
    typedef std::unique_ptr<sql::Statement> Statement;
    typedef std::unique_ptr<sql::ResultSet> ResultSet;
    typedef std::unique_ptr<sql::SQLException> SQLException;
    typedef std::unique_ptr<sql::mariadb::MariaDbStatement> MariaDbStatement;
    typedef std::unique_ptr<sql::mariadb::CallableParameterMetaData> CallableParameterMetaData;
    typedef std::unique_ptr<sql::mariadb::Credential> Credential;
    typedef std::unique_ptr<sql::mariadb::Results> Results;
    typedef std::unique_ptr<sql::mariadb::RowProtocol> RowProtocol;
    typedef std::unique_ptr<sql::mariadb::SelectResultSet> SelectResultSet;

    typedef std::unique_ptr<sql::mariadb::ServerSidePreparedStatement> ServerSidePreparedStatement;
    typedef std::unique_ptr<sql::mariadb::ClientSidePreparedStatement> ClientSidePreparedStatement;
    typedef std::unique_ptr<sql::mariadb::ServerPrepareResult> ServerPrepareResult;
  }

  namespace Weak
  {
    typedef std::weak_ptr<MariaDbConnection> MariaDbConnection;
  }
} //---- namespace mariadb

  //namespace Shared
  //{
  //}
  //namespace Unique
  //{
  //}
} //---- namespave sql
#endif
