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
static bool serverSocket(uint16* punSocket);
static bool serverBind(uint16* punBind, uint16* punSocket);
static bool serverListen(uint16* punSocket, uint16* punListen);
static bool serverAccept(uint16* punSocket, uint16* punAccept);
static bool serverHelloHandler(uint16* punSocket, uint8* pucBuffer);
static bool serverStatusHandler(uint16* punSocket, uint8* pucBuffer);
static bool serverUnKnownHandler(uint16* punSocket, uint8* pucBuffer);
static bool serverTimeHandler(uint16* punSocket, uint8* pucBuffer);
static bool serverCurrentTime(uint8* pucBuffer);
static bool serverAllHandler(uint16* punSocket, uint8* pucBuffer);

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
    uint8 ucFlag = 0;
    uint8 ucIndex = 0;
    uint16 unBind = 0;
    uint16 unSocket = 0;
    uint16 unListen = 0;
    uint16 unAccept = 0;
    uint8 ucRecievedBuffer[MAX_CHAR_SIZE] = "";
    _DATA_HANDLER_ stDataHandler[] = 
        {
            {"Hello", serverHelloHandler}, {"Status", serverStatusHandler}, 
            {"Time", serverTimeHandler}, {"All", serverAllHandler}
        };
    uint8 ucSizeData = sizeof(stDataHandler) / sizeof(stDataHandler[0]);

    serverSocket(&unSocket);
    serverBind(&unBind, &unSocket);
    serverListen(&unSocket, &unListen);

    while (1)
    {
        ucFlag = 0;

        serverAccept(&unSocket, &unAccept);
        recv(unAccept, ucRecievedBuffer, sizeof(ucRecievedBuffer), 0);

        for (ucIndex = 0; ucIndex < ucSizeData; ucIndex++)
        {
            if (0 == strcmp(stDataHandler[ucIndex].ucData, ucRecievedBuffer))
            {
                ucFlag = 1;
                stDataHandler[ucIndex].pFunction(&unAccept, ucRecievedBuffer);
            }
        }

        // Check if the received buffer is matched with data handler.
        if (0 == ucFlag)
        {
            serverUnKnownHandler(&unAccept, ucRecievedBuffer);
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
static bool serverSocket(uint16* punSocket)
{
    bool blReturn = false;

    if (NULL != punSocket)
    {
        *punSocket = socket(AF_INET, SOCK_STREAM, 0);

        if (ERROR_CODE == *punSocket)
        {
            printf("Socket creation failed\n");
        }
        else
        {
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
static bool serverBind(uint16* punBind, uint16* punSocket)
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
static bool serverListen(uint16* punSocket, uint16* punListen)
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
static bool serverAccept(uint16* punSocket, uint16* punAccept)
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

//**************************.serverHelloHandler.********************************
// Purpose : Send command to server.
// Inputs  : punSocket - Pointer to socket descriptor.
//         : pucBuffer - Pointer to the key name to be added in the json object.
// Outputs : none
// Return  : blReturn
// Notes   : None
//******************************************************************************
static bool serverHelloHandler(uint16* punSocket, uint8* pucBuffer)
{
    bool blReturn = false;
    uint16 unLength = 0;

    if ((NULL != punSocket) && (NULL != pucBuffer))
    {
        // Create JSON object.
        cJSON *pstJsonObject = cJSON_CreateObject();                
        cJSON_AddStringToObject(pstJsonObject, pucBuffer, "Hi"); 

        // Convert to JSON string
        uint8 *pcJsonResponse = cJSON_Print(pstJsonObject);
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
static bool serverTimeHandler(uint16* punSocket, uint8* pucBuffer)
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
        uint8 *pcJsonResponse = cJSON_Print(pstJsonObject); 
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
static bool serverStatusHandler(uint16* punSocket, uint8* pucBuffer)
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
        uint8 *pcJsonResponse = cJSON_Print(pstJsonArray); 
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
static bool serverUnKnownHandler(uint16* punSocket, uint8* pucBuffer)
{
    bool blReturn = false;
    uint16 unLength = 0;

    if ((NULL != punSocket) && (NULL != pucBuffer))
    {
        // Create JSON object.
        cJSON *pstJsonObject = cJSON_CreateObject();                
        cJSON_AddStringToObject(pstJsonObject, pucBuffer, "Unknown Command"); 

        // Convert to JSON string
        uint8 *pcJsonResponse = cJSON_Print(pstJsonObject); 
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
static bool serverAllHandler(uint16* punSocket, uint8* pucBuffer)
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
        uint8 *pcJsonResponse = cJSON_Print(pstJsonArray); 
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

// EOF