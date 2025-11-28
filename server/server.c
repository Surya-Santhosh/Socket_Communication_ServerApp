//******************************** server **************************************
// Copyright (c) 2025 Trenser Technology Solutions
// All Rights Reserved
//******************************************************************************
// File    : server.c
// Summary : Implements continuously running TCP server that listens for client 
//           commands and responds using corresponding handler functions.
// Note    : None
// Author  : Surya Santhosh
// Day     : 11/Nov/2025
//******************************************************************************

//**************************** Include Files ***********************************
#include "server.h"

//******************************* Local Types **********************************

//***************************** Local Constants ********************************

//**************************** Local Variables *********************************

//***************************** Local Functions ********************************
static bool serverSocket(int16* punSocket);
static bool serverBind(int16* punBind, int16* punSocket);
static bool serverListen(int16* punSocket, int16* punListen);
static bool serverAccept(int16* punSocket, int16* punAccept);
static bool serverHelloHandler(int16* punSocket, uint8* pucBuffer);
static bool serverStatusHandler(int16* punSocket, uint8* pucBuffer);
static bool serverUnknownHandler(int16* punSocket, uint8* pucBuffer);
static bool serverTimeHandler(int16* punSocket, uint8* pucBuffer);
static bool serverCurrentTime(uint8* pucBuffer);
static bool serverAllHandler(int16* punSocket, uint8* pucBuffer);
static bool serverListHandler(int16* punSocket, uint8* pucBuffer);
static bool serverSaveFile(uint8* pucBuffer);
static bool serverInvalidResponse(int16* punSocket);
static bool serverRequestHandler(int16* punSocket, uint8* pucBuffer,
                                 uint8* pucErrorFlag);
static bool serverGetRequestValidation(uint8* pucBuffer, 
                                       uint8* pucMessageValue);
static bool serverPostRequestValidation(uint8* pucBuffer, 
                                        uint8* pucMessageValue);

//*****************************.mainFunction.***********************************
// Purpose : Implements continuously running TCP server that listens for client 
//           commands and responds using corresponding handler functions.
// Inputs  : none
// Outputs : none
// Return  : 0
// Notes   : None
//******************************************************************************
int main()
{
    int16 unBind = 0;
    int16 unSocket = 0;
    int16 unListen = 0;
    int16 unAccept = 0;
    int16 unLength = 0;
    uint8 ucErrorFlag = 0;
    uint8 ucRecievedBuffer[MAX_CHAR_SIZE] = {0};

    serverSocket(&unSocket);
    serverBind(&unBind, &unSocket);
    serverListen(&unSocket, &unListen);

    while (1)
    {
        serverAccept(&unSocket, &unAccept);

        // To clear buffers.
        memset(ucRecievedBuffer, 0, sizeof(ucRecievedBuffer));
        unLength = recv(unAccept, ucRecievedBuffer, sizeof(ucRecievedBuffer),
                        0);

        if (0 >= unLength)
        {
            serverInvalidResponse(&unAccept);
            close(unAccept);
            continue;
        }

        ucRecievedBuffer[unLength] = '\0';
        serverRequestHandler(&unAccept, ucRecievedBuffer, &ucErrorFlag);
        
        if (0 != ucErrorFlag)
        {
            continue;
        }
    }

    close(unSocket);

    return 0;
}

//*****************************.serverSocket.***********************************
// Purpose : Create socket.
// Inputs  : punSocket - Pointer to socket descriptor.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverSocket(int16* punSocket)
{
    bool blReturn = false;
    int16 unSocket = 0;
    uint8 ucOption = 1;

    if (NULL != punSocket)
    {
        unSocket = socket(AF_INET, SOCK_STREAM, 0);

        if (ERROR_CODE == unSocket)
        {
            printf("Socket creation failed\n");
        }
        else
        {
            setsockopt(unSocket, SOL_SOCKET, SO_REUSEADDR, &ucOption, 
                       sizeof(ucOption));
            *punSocket = unSocket;
            blReturn = true;
        }
    }

    return blReturn;
}

//*******************************.serverBind.***********************************
// Purpose : Binds with the ip address and port.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : punBind - Pointer to store the return value of bind().
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverBind(int16* punBind, int16* punSocket)
{
    bool blReturn = false;

    // To hold the server address details.
    struct sockaddr_in stSocketAddress = {0};
    stSocketAddress.sin_family = AF_INET;
    stSocketAddress.sin_port = htons(PORT);
    stSocketAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    if ((NULL != punSocket) && (NULL != punBind))
    {
        *punBind = bind(*punSocket, (struct sockaddr *) &stSocketAddress, 
                         sizeof(stSocketAddress));

        if (ERROR_CODE == *punBind)
        {
            printf("Bind failed\n");
        }
        else
        {
            blReturn = true;
        }
    }

    return blReturn;
}

//*****************************.serverListen.***********************************
// Purpose : Listen for connections on a socket.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : punListen - Pointer to store the return value of listen().
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverListen(int16* punSocket, int16* punListen)
{
    bool blReturn = false;

    if ((NULL != punSocket) && (NULL != punListen))
    {
        *punListen = listen(*punSocket, 1);
    
        if (ERROR_CODE == *punListen)
        {
            printf("Listen failed\n");
        }
        else
        {
            blReturn = true;
        }
    }

    return blReturn;
}

//*****************************.serverAccept.***********************************
// Purpose : Accept a connection on a socket.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : punAccept - Pointer to accepted socket descriptor.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverAccept(int16* punSocket, int16* punAccept)
{
    bool blReturn = false;

    // To hold the client address details.
    struct sockaddr_in stClientAddress = {0};
    uint16 unLength = sizeof(stClientAddress);

    if ((NULL != punSocket) && (NULL != punAccept))
    {
        *punAccept = accept(*punSocket, (struct sockaddr *) &stClientAddress, 
                             (socklen_t *) &unLength);

        if (ERROR_CODE == *punAccept)
        {
            printf("Accept failed\n");
        }
        else
        {
            blReturn = true;
        }
    }

    return blReturn;
}

//*************************.serverRequestHandler.*******************************
// Purpose : Handle request and send corresponding responses.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : pucBuffer - Pointer to the Received request buffer.
//         : pucErrorFlag - Pointer to a flag used to indicates request errors.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverRequestHandler(int16* punSocket, uint8* pucBuffer, 
                                 uint8* pucErrorFlag)
{
    bool blReturn = false;
    uint16 unLength = 0;
    uint8 ucFlag = 0;
    uint8 ucIndex = 0;
    uint8 ucValidation = 0;
    uint32 ulFileListLength = 0;
    int8 *pucUploadedFileList = NULL;
    uint8 ucMessageValue[MAX_CHAR_SIZE] = {0};
    uint8 ucRequestData[MAX_CHAR_SIZE] = {0};
    uint8 ucRequestMethod[MAX_CHAR_SIZE] = {0};
    static uint8 sucFileList[NUM_FILES][MAX_CHAR_SIZE] = {0};
    static uint8 sucFileCount = 0;
    _DATA_HANDLER_ stDataHandler[] = 
        {
            {"Hello", serverHelloHandler}, {"Status", serverStatusHandler}, 
            {"Time", serverTimeHandler}, {"All", serverAllHandler}
        };
    uint8 ucSizeData = sizeof(stDataHandler) / sizeof(stDataHandler[0]);

    if ((NULL != punSocket) && (NULL != pucBuffer) && (NULL != pucErrorFlag))
    {
        // POST/GET request 
        if (NULL != strstr(pucBuffer, "type="))
        {
            ucRequestData[0] = '\0';
            ucRequestMethod[0] = '\0';
            sscanf(pucBuffer, "type=%[^&]&%s", ucRequestMethod, ucRequestData);

            // GET request
            if (0 == strcmp(ucRequestMethod, "GET"))
            {
                if (true != serverGetRequestValidation(ucRequestData, 
                                                       ucMessageValue))
                {
                    serverInvalidResponse(punSocket);
                    *pucErrorFlag = 1;
                }
                else
                {
                    ucValidation = 1;
                }
            }
            // POST request.
            else if (0 == strcmp(ucRequestMethod, "POST"))
            {
                if (true != serverPostRequestValidation(ucRequestData, 
                                                        ucMessageValue))
                {
                    serverInvalidResponse(punSocket);
                    *pucErrorFlag = 1;
                }
                else
                {
                    ucValidation = 1;
                }
            }

            if ((NULL != ucMessageValue) && (0 != ucValidation))
            {
                for (ucIndex = 0; ucIndex < ucSizeData; ucIndex++)
                {
                    if (0 == strcmp(stDataHandler[ucIndex].ucData, 
                                    ucMessageValue))
                    {
                        ucFlag = 1;
                        stDataHandler[ucIndex].pFunction(punSocket, 
                                                         ucMessageValue);
                    }
                }

                // Check if the received buffer is matched with data handler.
                if (0 == ucFlag)
                {
                    serverUnknownHandler(punSocket, ucMessageValue);
                }
            }
            else
            {
                serverInvalidResponse(punSocket);
            }
        }
        // File upload request.
        else if (NULL != strstr(pucBuffer, "FilePath:"))
        {
            serverSaveFile(pucBuffer);
            printf("Received buffer: %s\n", pucBuffer);
            strcpy(sucFileList[sucFileCount], pucBuffer);
            sucFileCount ++;
        }
        // File list request.
        else if (NULL != strstr(pucBuffer, "List"))
        {
            if ('\0' == sucFileList[0][0])
            {
                strcpy(sucFileList[0], "No files uploaded");
                serverListHandler(punSocket, sucFileList[0]);
            }
            else
            {
                for (ucIndex = 0; ucIndex < sucFileCount; ucIndex++)
                {
                    ulFileListLength += strlen(sucFileList[ucIndex]);
                    ulFileListLength++;
                }

                ulFileListLength++;
                pucUploadedFileList = malloc(ulFileListLength + 1);
                pucUploadedFileList[0] = '\0';

                for (ucIndex = 0; ucIndex < sucFileCount; ucIndex++)
                {
                    strcat(pucUploadedFileList, sucFileList[ucIndex]);

                    if ((1 < sucFileCount) && ((sucFileCount - 1) > ucIndex))
                    {
                        strcat(pucUploadedFileList, ",");
                    }
                }

                serverListHandler(punSocket, pucUploadedFileList);
                free(pucUploadedFileList);
            }
        }
        
        close(*punSocket);

        blReturn = true;
    }

    return blReturn;
}

//**************************.serverHelloHandler.********************************
// Purpose : Send command to server.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : pucBuffer - Pointer to the key name to be added in the json object.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverHelloHandler(int16* punSocket, uint8* pucBuffer)
{
    bool blReturn = false;
    uint16 unLength = 0;
    uint8 ucTimeData[MAX_CHAR_SIZE] = "";

    if ((NULL != punSocket) && (NULL != pucBuffer))
    {
        // To get current date and time.
        serverCurrentTime(ucTimeData);

        // Create JSON object.
        cJSON *pstJsonObject = cJSON_CreateObject();                
        cJSON_AddStringToObject(pstJsonObject, pucBuffer, "Hi"); 
        cJSON_AddStringToObject(pstJsonObject, "Time", ucTimeData);

        // Convert to JSON string
        uint8 *pcJsonResponse = cJSON_PrintUnformatted(pstJsonObject);
        unLength = strlen(pcJsonResponse);

        // Send size of the response to the client.
        send(*punSocket, &unLength, sizeof(unLength), 0);
        send(*punSocket, pcJsonResponse, unLength, 0);
        
        // Free JSON object
        cJSON_Delete(pstJsonObject);    

        // Free string memory
        free(pcJsonResponse); 

        blReturn = true;
    }

    return blReturn;
}

//****************************.serverTimeHandler.*******************************
// Purpose : Send command to server.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : pucBuffer - Pointer to the key name to be added in the json object.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverTimeHandler(int16* punSocket, uint8* pucBuffer)
{
    bool blReturn = false;
    uint8 ucTimeData[MAX_CHAR_SIZE] = "";
    uint16 unLength = 0;

    if ((NULL != punSocket) && (NULL != pucBuffer))
    {
        // To get current date and time.
        serverCurrentTime(ucTimeData);

        // Create JSON object.
        cJSON *pstJsonObject = cJSON_CreateObject();                
        cJSON_AddStringToObject(pstJsonObject, pucBuffer, ucTimeData); 

        // Convert to JSON string
        uint8 *pcJsonResponse = cJSON_PrintUnformatted(pstJsonObject); 
        unLength = strlen(pcJsonResponse);

        // Send size of the response to the client.
        send(*punSocket, &unLength, sizeof(unLength), 0);
        send(*punSocket, pcJsonResponse, unLength, 0);

        // Free JSON object
        cJSON_Delete(pstJsonObject);
        
        // Free string memory
        free(pcJsonResponse);
        
        blReturn = true;
    }

    return blReturn;
}

//**************************.serverStatusHandler.*******************************
// Purpose : Send command to server.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : pucBuffer - Pointer to the key name to be added in the json object.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverStatusHandler(int16* punSocket, uint8* pucBuffer)
{
    bool blReturn = false;
    uint8 ucTimeData[MAX_CHAR_SIZE] = "";
    uint16 unLength = 0;

    if (NULL != punSocket)
    {
        // To get current date and time.
        serverCurrentTime(ucTimeData);

        // Create JSON array.
        cJSON *pstJsonArray = cJSON_CreateArray();

        // Create JSON object.
        cJSON *pstJsonObject = cJSON_CreateObject();   

        cJSON_AddStringToObject(pstJsonObject, "Name", "Server"); 
        cJSON_AddStringToObject(pstJsonObject, "Status", "Active"); 
        cJSON_AddStringToObject(pstJsonObject, "Time", ucTimeData); 
        cJSON_AddItemToArray(pstJsonArray, pstJsonObject);

        // Convert to JSON string
        uint8 *pcJsonResponse = cJSON_PrintUnformatted(pstJsonArray); 
        unLength = strlen(pcJsonResponse);

        // Send size of the response to the client.
        send(*punSocket, &unLength, sizeof(unLength), 0);
        send(*punSocket, pcJsonResponse, unLength, 0);

        // Free JSON array
        cJSON_Delete(pstJsonArray);  
        
        // Free string memory
        free(pcJsonResponse);
        
        blReturn = true;
    }

    return blReturn;
}

//**************************.serverUnKnownHandler.******************************
// Purpose : Send command to server.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : pucBuffer - Pointer to the key name to be added in the json object.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverUnknownHandler(int16* punSocket, uint8* pucBuffer)
{
    bool blReturn = false;
    uint16 unLength = 0;

    if ((NULL != punSocket) && (NULL != pucBuffer))
    {
        // Create JSON object.
        cJSON *pstJsonObject = cJSON_CreateObject();                
        cJSON_AddStringToObject(pstJsonObject, pucBuffer, "Unknown Command"); 

        // Convert to JSON string
        uint8 *pcJsonResponse = cJSON_PrintUnformatted(pstJsonObject); 
        unLength = strlen(pcJsonResponse);

        // Send size of the response to the client.
        send(*punSocket, &unLength, sizeof(unLength), 0);
        send(*punSocket, pcJsonResponse, unLength, 0);

        // Free JSON object
        cJSON_Delete(pstJsonObject);   
        
        // Free string memory
        free(pcJsonResponse);
        
        blReturn = true;
    }

    return blReturn;
}

//****************************.serverListHandler.*******************************
// Purpose : Send command to server.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : pucBuffer - Pointer to the key name to be added in the json object.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverListHandler(int16* punSocket, uint8* pucBuffer)
{
    bool blReturn = false;
    uint16 unLength = 0;

    if ((NULL != punSocket) && (NULL != pucBuffer))
    {
        // Create JSON object.
        cJSON *pstJsonObject = cJSON_CreateObject();                
        cJSON_AddStringToObject(pstJsonObject, "Files", pucBuffer); 

        // Convert to JSON string
        uint8 *pcJsonResponse = cJSON_PrintUnformatted(pstJsonObject); 
        unLength = strlen(pcJsonResponse);

        // Send size of the response to the client.
        send(*punSocket, &unLength, sizeof(unLength), 0);
        send(*punSocket, pcJsonResponse, unLength, 0);

        // Free JSON object
        cJSON_Delete(pstJsonObject);   
        
        // Free string memory
        free(pcJsonResponse);
        
        blReturn = true;
    }

    return blReturn;
}

//******************************.serverSaveFile.********************************
// Purpose : Extract file path from the received buffer and copy to the buffer.
// Inputs  : pucBuffer - Pointer to the buffer to store the file path.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverSaveFile(uint8* pucBuffer)
{
    bool blReturn = false;
    uint16 unLength = 0;

    if (NULL != pucBuffer)
    {
        int8 *pucActualPath = malloc(strlen(pucBuffer) + 1);
        int8 *pucPath = strstr(pucBuffer, ":");

        strncpy(pucActualPath, pucPath + 1, strlen(pucBuffer));
        pucActualPath[strlen(pucBuffer)] = '\0';
        strcpy(pucBuffer, pucActualPath);
        free(pucActualPath);

        blReturn = true;
    }

    return blReturn;
}

//*****************************.serverCurrentTime.******************************
// Purpose : Generate current date and time.
// Inputs  : pucBuffer - Pointer to the Buffer where the time will be stored.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverCurrentTime(uint8* pucBuffer)
{
    bool blReturn = false;

    if (NULL != pucBuffer)
    {
        // Get current time in seconds.
        time_t ntime;
        time(&ntime);

        // Convert to local time structure.
        struct tm *pstLocalTime = localtime(&ntime);

        // Get formated time into buffer
        strftime(pucBuffer, MAX_CHAR_SIZE, "%Y-%m-%d %H:%M:%S", pstLocalTime);

        blReturn = true;
    }

    return blReturn;
}

//****************************.serverAllHandler.********************************
// Purpose : Send command to server.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : pucBuffer - Pointer to the key name to be added in the json object.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverAllHandler(int16* punSocket, uint8* pucBuffer)
{
    bool blReturn = false;
    uint8 ucIndex = 0;
    uint8 ucTimeData[MAX_CHAR_SIZE] = "";
    uint16 unLength = 0;

    // To get current date and time.
    serverCurrentTime(ucTimeData);

    // Array of JSON Keys.
    uint8 *pucKeyList[] = {"Hello", "Name", "Status", "Time"};

    // Array of JSON values corresponding to key.
    uint8 *pucDataList[] = {"Hi", "Server", "Active", ucTimeData}; 
    uint8 ucSizeDataList = sizeof(pucDataList) / sizeof(pucDataList[0]);

    if (NULL != punSocket)
    {
        // Create JSON array.
        cJSON *pstJsonArray = cJSON_CreateArray();

        // Create JSON object.
        cJSON *pstJsonObject = cJSON_CreateObject();

        for (ucIndex = 0; ucIndex < ucSizeDataList; ucIndex++)
        {                
            cJSON_AddStringToObject(pstJsonObject, pucKeyList[ucIndex], 
                                    pucDataList[ucIndex]);
        }

        cJSON_AddItemToArray(pstJsonArray, pstJsonObject);

        // Convert to JSON string
        uint8 *pcJsonResponse = cJSON_PrintUnformatted(pstJsonArray); 
        unLength = strlen(pcJsonResponse);

        // Send size of the response to the client.
        send(*punSocket, &unLength, sizeof(unLength), 0);
        send(*punSocket, pcJsonResponse, unLength, 0);

        // Free JSON array
        cJSON_Delete(pstJsonArray);
        
        // Free string memory
        free(pcJsonResponse);

        blReturn = true;
    }

    return blReturn;
}

//****************************.serverInvalidResponse.***************************
// Purpose : Handle invalide request.
// Inputs  : punSocket - Pointer to socket descriptor.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverInvalidResponse(int16* punSocket)
{
    bool blReturn = false;
    uint16 unLength = 0;

    if (NULL != punSocket)
    {
        unLength = strlen("Invalid Request");
        send(*punSocket, &unLength, sizeof(unLength), 0);
        send(*punSocket, "Invalid Request", unLength, 0);

        blReturn = true;
    }

    return blReturn;
}

//**************************.serverGetRequestValidation.************************
// Purpose : Validate GET request buffer and extract message value.
// Inputs  : pucBuffer - Pointer to the buffer containg get request.
//         : pucMessageValue - Pointer to the extracted message value.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverGetRequestValidation(uint8* pucBuffer, uint8* pucMessageValue)
{
    bool blReturn = false;
    uint16 unLength = 0;

    // Check if data is empty
    if ((NULL != pucBuffer) && (NULL != pucMessageValue))
    {
        // Check whether '=' is present
        if (NULL != strchr(pucBuffer, '='))
        {
            // Check whether key msg is present
            if (0 == strncmp(pucBuffer, "msg=", 4))
            {
                sscanf(pucBuffer, "msg=%s", pucMessageValue);

                // Check wheather value is present
                if (0 < strlen(pucMessageValue))
                {
                    blReturn = true;
                }
            }
        }
    }

    return blReturn;
}

//************************.serverPostRequestValidation.*************************
// Purpose : Validate POST request buffer and extract message value.
// Inputs  : pucBuffer - Pointer to the buffer containg get request.
//         : pucMessageValue - Pointer to the extracted message value.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverPostRequestValidation(uint8* pucBuffer, 
                                        uint8* pucMessageValue)
{
    bool blReturn = false;
    uint16 unLength = 0;

    // Check if data is empty
    if ((NULL != pucBuffer) && (NULL != pucMessageValue))
    {
        // To convert JSON String to object.
        cJSON *pJsonObject = cJSON_Parse(pucBuffer);

        // Check if data is JSON format.
        if (NULL != pJsonObject)
        {
            cJSON *pMessage = cJSON_GetObjectItem(pJsonObject, "msg");

            // Check whether key msg is present.
            if (NULL != pMessage)
            {
                strcpy((char* )pucMessageValue, pMessage->valuestring);

                // Check wheather value is present
                if (0 < strlen(pucMessageValue))
                {
                    blReturn = true;
                }
            }
        }

        cJSON_Delete(pJsonObject);
    }

    return blReturn;
}

// EOF