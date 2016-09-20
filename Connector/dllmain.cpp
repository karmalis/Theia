#include <stdio.h>
#include <mysql.h>
#include <windows.h>

// Database and table are pre-defined
#define THEIA_DBNAME "theia_poc"
#define THEIA_TABLE "images"

struct ConParams {
	char* hostname;
	char* username;
	char* password;
	char* database;
	char* table;
	unsigned short port;

	ConParams() :
		hostname(NULL),
		username(NULL),
		password(NULL),
		database(NULL),
		table(NULL),
		port(0)
	{
		database = (char*)malloc(sizeof(char)*(strlen(THEIA_DBNAME)+1));
		strcpy(database, THEIA_DBNAME);
		database[strlen(THEIA_DBNAME)] = '\0';

		table = (char*)malloc(sizeof(char)*(strlen(THEIA_TABLE)+1));
		strcpy(table, THEIA_TABLE);
		table[strlen(THEIA_TABLE)] = '\0';
	}
};

ConParams params;
char* error;

void logError(const char* string)
{
	// Free error if it's pre-set
	if (error) {
		free(error);
		error = NULL;
	}

	// Log the error
	error = (char*)malloc(sizeof(char) * strlen(string));
	strcpy(error, string);
}

struct Connection {

	MYSQL *con;
	MYSQL_RES *result;
	bool connected;

	Connection() :
		con(NULL),
		connected(false)
	{
		
	}

	bool Connect(const ConParams &params)
	{
		con = mysql_init(con);
		if (!con)
		{
			logError("Could not establish MySQLDriver");
			return false;
		}

		if (mysql_real_connect(con, params.hostname, params.username, params.password, params.database, params.port, NULL, 0) == NULL)
		{
			logError(mysql_error(con));
			return false;
		}

		connected = true;
		return true;
	}

	void Close()
	{
		mysql_close(con);
	}

	bool ExecuteStatement(const char* statement)
	{
		if (!connected)
		{
			if (!this->Connect(params)) return NULL;
		}

		if (result) mysql_free_result(result);

		if (mysql_real_query(con, statement, strlen(statement)) != 0)
		{
			logError(mysql_error(con));
			return false;
		}

		result = mysql_store_result(con);

		return true;
	}

	int GetLastInsertID()
	{
		if (!connected)
		{
			if (!this->Connect(params)) return -1;
		}

		return mysql_insert_id(con);
	}

};


Connection con;
unsigned char* downloadedImage;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH: break;
	case DLL_PROCESS_DETACH:
		// Clean up
		if (error) {
			free(error);
			error = NULL;
		}
		if (downloadedImage) {
			free(downloadedImage);
			downloadedImage = NULL;
		}
		if (params.database) {
			free(params.database);
			params.database = NULL;
		}
		if (params.hostname) {
			free(params.hostname);
			params.hostname = NULL;
		}
		if (params.password) {
			free(params.password);
			params.password = NULL;
		}
		if (params.table) {
			free(params.table);
			params.table = NULL;
		}
		if (params.username) {
			free(params.username);
			params.username = NULL;
		}
		con.Close();
		
		break;
	}
	return TRUE;
}

#define THEIA_CONNECTOR_EXPORT extern "C" __declspec(dllexport)

THEIA_CONNECTOR_EXPORT void SetTableAndDB(const char* database, const char* table)
{
	if (strlen(database) > 0)
	{
		if (params.database)
		{
			free(params.database);
			params.database = NULL;
		}

		params.database = (char*)malloc(sizeof(char) * (strlen(database) + 1));
		strcpy(params.database, database);
		params.database[strlen(database)] = '\0';
	}

	if (strlen(table) > 0)
	{
		if (params.table)
		{
			free(params.table);
			params.table = NULL;
		}

		params.table = (char*)malloc(sizeof(char) * (strlen(table) +1 ));
		strcpy(params.table, table);
		params.table[strlen(table)] = '\0';
	}
}

THEIA_CONNECTOR_EXPORT bool Connect(
	const char* username,
	const char* password,
	const char* hostname,
	unsigned short port
	)
{
	if (strlen(username) > 0)
	{
		if (params.username)
		{
			free(params.username);
			params.username = NULL;
		}

		params.username = (char*)malloc(sizeof(char) * (strlen(username) + 1));
		strcpy(params.username, username);
		params.username[strlen(username)] = '\0';
	}

	if (strlen(password) > 0)
	{
		if (params.password)
		{
			free(params.password);
			params.password = NULL;
		}

		params.password = (char*)malloc(sizeof(char) * (strlen(password) + 1));
		strcpy(params.password, password);
		params.password[strlen(password)] = '\0';
	}

	if (strlen(hostname) > 0)
	{
		if (params.hostname)
		{
			free(params.hostname);
			params.hostname = NULL;
		}

		params.hostname = (char*)malloc(sizeof(char) * (strlen(hostname) + 1));
		strcpy(params.hostname, hostname);
		params.hostname[strlen(hostname)] = '\0';
	}

	params.port = port;

	if (!con.Connect(params)) return false;

	return true;
}

THEIA_CONNECTOR_EXPORT int Upload(
	const unsigned char* imageData,
	unsigned int imageDataSize,
	const char* imageName
	)
{
	// Escape the query variables
	char* eImageName = (char*)malloc(2*strlen(imageName) + 1);
	size_t eImageNameSize = mysql_real_escape_string(con.con, eImageName, imageName, strlen(imageName));

	char* eImage = (char*)malloc(2 * imageDataSize + 1);
	size_t eImageDataSize = mysql_real_escape_string(con.con, eImage, (const char*)imageData, imageDataSize);
	
	size_t calculated = 0;
	// Construct the statement
	char* stTemp = "INSERT INTO `%s` (`filename`, `data`) VALUES ('%s', '%s')";
	size_t tempSize = strlen(stTemp);

	calculated = tempSize + strlen(params.table); // Add table name to total
	calculated += eImageDataSize;	// Add image data size to total
	calculated += eImageNameSize; // Add image name size to total
	
	size_t finalSize = sizeof(char) * calculated;
	char* resQuery = (char*)malloc(finalSize);

	sprintf_s(resQuery, finalSize, stTemp, params.table, eImageName, eImage);
	free(eImageName);
	free(eImage);

	if (!con.ExecuteStatement(resQuery)) {
		free(resQuery);
		return -1;
		
	}
	free(resQuery);
	return con.GetLastInsertID();
}

THEIA_CONNECTOR_EXPORT bool Download(
	int id,
	unsigned char** data,
	unsigned int* dataSize,
	char* filename
	)
{
	char idBuffer[10];
	char* stTemp = "SELECT `filename`, `data` FROM `%s` WHERE id = '%s'";
	_itoa_s(id, idBuffer, 10, 10);

	size_t calculated = strlen(stTemp) + strlen(idBuffer) + strlen(params.table);
	size_t finalSize = sizeof(char) * calculated;
	char* resQuery = (char*)malloc(finalSize);
	sprintf_s(resQuery, finalSize, stTemp, params.table, idBuffer);

	if (!con.ExecuteStatement(resQuery)) {
		free(resQuery);
		return false;
	}
	free(resQuery);
	MYSQL_ROW row = mysql_fetch_row(con.result);
	unsigned long *lengths = mysql_fetch_lengths(con.result);
	if (lengths == NULL) return false;

	// Row structure based on parameters
	// 0 -> Filename
	// 1 -> Data

	// Set the filename
	char* tempFilename;
	if (lengths[0])
	{
		size_t filenameSize = sizeof(char*) * lengths[0];
		tempFilename = (char*)malloc(filenameSize);
		sprintf_s(tempFilename, filenameSize, "%s", row[0]);
	}
	else {
		// Undefined
		tempFilename = (char*)malloc(sizeof(char*) * 9);
		sprintf_s(tempFilename, sizeof(char*) * 9, "%s", "Undefined");
	}
	
	// Set the filesize
	if (!dataSize) dataSize = (unsigned int*) malloc(sizeof(unsigned int));
	if (lengths[1]) *dataSize = lengths[1];
	else *dataSize = 0;

	// Set the downloaded image
	if (*dataSize == 0) return false;

	size_t dataSizeActual = *dataSize;
	if (downloadedImage) {
		downloadedImage = NULL; // Possible memory leak if the image data was not freed via FreeImageData
	}	
	downloadedImage = (unsigned char*)malloc(sizeof(unsigned char) * dataSizeActual);
	// Copy the image into memory
	memcpy(downloadedImage, (unsigned char*)row[1], sizeof(unsigned char) * dataSizeActual);

	*data = downloadedImage;
	strcpy(filename,tempFilename);
	free(tempFilename);

	return true;
}

THEIA_CONNECTOR_EXPORT void FreeImageData(const unsigned char* data)
{
	free((void*)data);
}

THEIA_CONNECTOR_EXPORT const char* FetchLastError()
{
	return error;
}