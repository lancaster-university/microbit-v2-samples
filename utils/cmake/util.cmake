MACRO(RECURSIVE_FIND_DIR return_list dir pattern)
    FILE(GLOB_RECURSE new_list "${dir}/${pattern}")
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

MACRO(RECURSIVE_FIND_FILE return_list dir pattern)
    FILE(GLOB_RECURSE new_list "${dir}/${pattern}")
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        SET(dir_list ${dir_list} ${file_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

MACRO(SOURCE_FILES return_list dir pattern)
    FILE(GLOB new_list "${dir}/${pattern}")
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        LIST(APPEND dir_list ${file_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

# Read a value out of the JSON document held in variable <json_var>.  The
# arguments after <json_var> form the member/index path into the document.
# The value is written to <result>; a missing path yields the empty string.
function(json_get_or_empty result json_var)
    set(_json_value "${${json_var}}")
    string(JSON _out ERROR_VARIABLE _err GET "${_json_value}" ${ARGN})
    if(_err)
        set(${result} "" PARENT_SCOPE)
    else()
        set(${result} "${_out}" PARENT_SCOPE)
    endif()
endfunction()

# Collect the member names and values of the JSON object at the path given by
# the arguments after <json_var> into the parallel lists <fields> and
# <values>.  A missing path yields empty lists.  Booleans come back from
# string(JSON) as ON/OFF; they are normalized to true/false to match the
# values the old flattened parser produced.
function(json_collect_object fields values json_var)
    set(_json_value "${${json_var}}")
    set(_fields "")
    set(_values "")
    string(JSON _len ERROR_VARIABLE _err LENGTH "${_json_value}" ${ARGN})
    if(NOT _err AND _len GREATER 0)
        math(EXPR _last "${_len} - 1")
        foreach(_i RANGE ${_last})
            string(JSON _field MEMBER "${_json_value}" ${ARGN} ${_i})
            string(JSON _type TYPE "${_json_value}" ${ARGN} ${_field})
            string(JSON _value GET "${_json_value}" ${ARGN} ${_field})
            if("${_type}" STREQUAL "BOOLEAN")
                if("${_value}" STREQUAL "ON")
                    set(_value "true")
                else()
                    set(_value "false")
                endif()
            elseif("${_type}" STREQUAL "NULL")
                set(_value "null")
            endif()
            list(APPEND _fields "${_field}")
            list(APPEND _values "${_value}")
        endforeach()
    endif()
    set(${fields} ${_fields} PARENT_SCOPE)
    set(${values} ${_values} PARENT_SCOPE)
endfunction()

function(FORM_DEFINITIONS fields values definitions)

    set(DEFINITIONS "")
    list(LENGTH ${fields} LEN)

    # - 1 for for loop index...
    MATH(EXPR LEN "${LEN}-1")

    foreach(i RANGE ${LEN})
        list(GET ${fields} ${i} DEFINITION)
        list(GET ${values} ${i} VALUE)

        set(DEFINITIONS "${DEFINITIONS} #define ${DEFINITION}\t ${VALUE}\n")
    endforeach()

    set(${definitions} ${DEFINITIONS} PARENT_SCOPE)
endfunction()

function(UNIQUE_JSON_KEYS priority_fields priority_values secondary_fields secondary_values merged_fields merged_values)

    # always keep the first fields and values
    set(MERGED_FIELDS ${${priority_fields}})
    set(MERGED_VALUES ${${priority_values}})

    # measure the second set...
    list(LENGTH ${secondary_fields} LEN)
    # - 1 for for loop index...
    MATH(EXPR LEN "${LEN}-1")

    # iterate, dropping any duplicate fields regardless of the value
    foreach(i RANGE ${LEN})
        list(GET ${secondary_fields} ${i} FIELD)
        list(GET ${secondary_values} ${i} VALUE)

        list(FIND MERGED_FIELDS ${FIELD} INDEX)

        if (${INDEX} GREATER -1)
            continue()
        endif()

        list(APPEND MERGED_FIELDS ${FIELD})
        list(APPEND MERGED_VALUES ${VALUE})
    endforeach()

    set(${merged_fields} ${MERGED_FIELDS} PARENT_SCOPE)
    set(${merged_values} ${MERGED_VALUES} PARENT_SCOPE)
endfunction()

MACRO(HEADER_FILES return_list dir)
    FILE(GLOB new_list "${dir}/*.h")
    SET(${return_list} ${new_list})
ENDMACRO()

function(INSTALL_DEPENDENCY dir name url branch type)
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/${dir}")
        message("Creating libraries folder")
        FILE(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/${dir}")
    endif()

    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${dir}/${name}")
        message("${name} is already installed")
        return()
    endif()

    if(${type} STREQUAL "git")
        message("Cloning into: ${url}")
	    # git clone -b doesn't work with SHAs
        execute_process(
            COMMAND git clone --recurse-submodules ${url} ${name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${dir}
        )

        if(NOT "${branch}" STREQUAL "")
            message("Checking out branch: ${branch}")
            execute_process(
                COMMAND git -c advice.detachedHead=false checkout ${branch}
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${dir}/${name}
            )
            execute_process(
                COMMAND git submodule update --init
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${dir}/${name}
            )
            execute_process(
                COMMAND git submodule sync
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${dir}/${name}
            )
            execute_process(
                COMMAND git submodule update
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${dir}/${name}
            )
        endif()
    else()
        message("No mechanism exists to install this library.")
    endif()
endfunction()

MACRO(SUB_DIRS return_dirs dir)
    FILE(GLOB list "${PROJECT_SOURCE_DIR}/${dir}/*")
    SET(dir_list "")
    FOREACH(file_path ${list})
        SET(dir_list ${dir_list} ${file_path})
    ENDFOREACH()
    set(${return_dirs} ${dir_list})
ENDMACRO()
