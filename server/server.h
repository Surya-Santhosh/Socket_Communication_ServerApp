//******************************* server ***************************************
// Copyright (c) 2025 Trenser Technology Solutions
// All Rights Reserved 
//****************************************************************************** 
// 
// Summary : Typedefs are included.
// Note    : None
// 
//******************************************************************************
#ifndef SERVER_H
#define SERVER_H 

//**************************** Include Files ***********************************
#include "common.h"

//**************************** Global Types ************************************
typedef struct _DATA_HANDLER_
{
    uint8 ucData[MAX_CHAR_SIZE];
    bool (*pFunction)(int16* punSocket, uint8* pucBuffer);
}_DATA_HANDLER_;

//************************* Global Constants ***********************************

//************************* Global Variables *********************************** 

//************************* Forward Declarations *******************************  

//************************ Inline Method Implementations *********************** 

#endif 

// SERVER_H 
// EOF
