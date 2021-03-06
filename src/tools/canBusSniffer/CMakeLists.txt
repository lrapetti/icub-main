# Copyright: (C) 2010 RobotCub Consortium
# Authors: Giorgio Metta, Lorenzo Natale
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

SET(PROJECTNAME canBusSniffer)

PROJECT(${PROJECTNAME})

FIND_PACKAGE(YARP)
FIND_PACKAGE(ICUB)
FIND_PACKAGE(ICUBDEV)

IF(USE_ICUB_MOD)
  ADD_DEFINITIONS(-DUSE_ICUB_MOD)
  FIND_PACKAGE(icubmod)
  IMPORT_DEVICES(${ICUB_DIR}/src/canBusSniffer/drivers.h icubmod)
ENDIF (USE_ICUB_MOD)


# Find .cpp and .h files automatically.  This is a bit lazy,
# and in principle it would be better to list these files manually.
FILE(GLOB folder_source *.cpp)
FILE(GLOB folder_header *.h)

# Declare groups of source and header files -- makes things pretty in MSVC.
SOURCE_GROUP("Source Files" FILES ${folder_source})
SOURCE_GROUP("Header Files" FILES ${folder_header})

# Add our include files into our compiler's search path.
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

# Create everything needed to build our executable.
ADD_EXECUTABLE(${PROJECTNAME} ${folder_source} ${folder_header})


