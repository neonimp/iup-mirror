FIND_PATH(CD_INCLUDE_DIR cd.h
	PATH_SUFFIXES 
		cd
	PATHS
		$ENV{CD_DIR}
		$ENV{CD_DIR}/include
)

FIND_LIBRARY(CD_LIBRARY 
	NAMES 
		cd
	PATH_SUFFIXES 
		Linux32_64
	PATHS
		$ENV{CD_DIR}
		$ENV{CD_DIR}/lib
		$ENV{CD_DIR}/lib/Linux32_64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	CD DEFAULT_MSG
	CD_LIBRARY CD_INCLUDE_DIR
)

IF(CD_FOUND)
	SET(CD_INCLUDE_DIRS ${CD_INCLUDE_DIR})
	SET(CD_LIBRARIES ${CD_LIBRARY})
ELSE(CD_FOUND)
	SET(CD_INCLUDE_DIRS "")
	SET(CD_LIBRARIES "")
ENDIF(CD_FOUND)

