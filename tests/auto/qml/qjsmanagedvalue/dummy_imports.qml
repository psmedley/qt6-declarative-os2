// This file exists for the sole purpose for qmlimportscanner to find
// which modules it needs to extract for deployment.
// Otherwise, it fails to find the imports that are expressed in C++

import QtQml

QtObject { }    // This is needed in order to keep importscanner happy
