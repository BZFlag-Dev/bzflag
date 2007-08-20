#include "Logger.h"

template <>
FrontendLogger* Singleton<FrontendLogger>::_instance = NULL;

template <>
BackendLogger* Singleton<BackendLogger>::_instance = NULL;
