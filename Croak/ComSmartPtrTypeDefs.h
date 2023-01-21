#pragma once
#include <comdef.h>
#include <AppxPackaging.h>

// appxpackaging.h
_COM_SMARTPTR_TYPEDEF(IAppxFactory, __uuidof(IAppxFactory));
_COM_SMARTPTR_TYPEDEF(IAppxPackageReader, __uuidof(IAppxPackageReader));
_COM_SMARTPTR_TYPEDEF(IAppxManifestReader, __uuidof(IAppxManifestReader));
_COM_SMARTPTR_TYPEDEF(IAppxManifestApplicationsEnumerator, __uuidof(IAppxManifestApplicationsEnumerator));
_COM_SMARTPTR_TYPEDEF(IAppxManifestProperties, __uuidof(IAppxManifestProperties));
_COM_SMARTPTR_TYPEDEF(IAppxManifestApplication, __uuidof(IAppxManifestApplication));