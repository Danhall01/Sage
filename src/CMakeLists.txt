target_sources(
    ${SAGE_LIB}
    PUBLIC FILE_SET
           CXX_MODULES
           FILES
           sage.cppm
           core/types.cpp
           core/severity.cpp
           logging/logger.cpp
)
