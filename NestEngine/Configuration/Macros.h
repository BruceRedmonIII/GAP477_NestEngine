#pragma once
#include "../Math/Hash.h"
#if (_DEBUG)
#define _DEBUG_SETTINGS 1
#else
#define _DEBUG_SETTINGS 0   
#endif

#if _DEBUG_SETTINGS == 1
    #define _LOGS_ENABLED 1 // set to 0 if you don't want a log file 
    #define _LOG_TO_TERMINAL 1
    #define _PRINT_NEW_TRANSFORMS 0
    #define _PRINT_MATERIAL_PROPERTIES 1
    #define _ENGINE_BASIC_LOGGING 0
#endif

#define _SDL_IMAGE 1 // set to 0 if you don't want to use SDL_image

#define SET_HASHED_ID(_type) \
            static constexpr HashedId kHashedId = nest::Hash(#_type);
