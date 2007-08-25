#include "Logger.h"

template <>
FrontendLogger* Singleton<FrontendLogger>::_instance = NULL;

template <>
BackendLogger* Singleton<BackendLogger>::_instance = NULL;

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
