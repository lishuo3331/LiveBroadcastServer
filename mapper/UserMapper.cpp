//
// Created by rjd
//

#include "UserMapper.h"
#include "utils/Logger.h"

bool UserMapper::Initialize(const std::string& host, const std::string& user, const std::string& passwd,
		const std::string& db_name)
{
	return mysql_.Initialize(host, user, passwd, db_name);
}

std::string UserMapper::GetPasswdByUser(const std::string& username)
{
	QueryResultPtr result = mysql_.Query("select user, passwd from live_user");

	if (result)
	{
		do
		{
			if ((*result)["user"].GetString() == username)
			{
				LOG_INFO << "user: " << username << ", passwd: " << (*result)["passwd"].GetString();

				return (*result)["passwd"].GetString();
			}

		} while (result->NextRow());

		LOG_WARN << "cannot find user: ", username.c_str();
	}

	return "";
}
