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

# Look up a dependency override in the "branches" object of codal.json.
# This mirrors _find_override() in utils/python/codal_utils.py: every key is a
# repo URL, whose trailing path segment (minus a ".git" suffix) is the repo
# name. The first entry whose repo name matches <name> sets <out_url> to the
# key itself and <out_ref> to its value. Both are empty if there is no match.
# Note: looking this up by the dependency's *declared* URL (as a direct JSON
# key GET) silently misses entries that point to a fork of the same repo,
# which made fresh clones build pre-fix upstream code.
function(find_dependency_override out_url out_ref json_var name)
    set(_json_value "${${json_var}}")
    set(${out_url} "" PARENT_SCOPE)
    set(${out_ref} "" PARENT_SCOPE)
    string(JSON _len ERROR_VARIABLE _err LENGTH "${_json_value}" "target" "branches")
    if(_err)
        return()
    endif()
    if(NOT _len GREATER 0)
        return()
    endif()
    math(EXPR _last "${_len} - 1")
    foreach(_i RANGE ${_last})
        string(JSON _key MEMBER "${_json_value}" "target" "branches" ${_i})
        string(REGEX REPLACE "/+$" "" _base "${_key}")
        string(REGEX REPLACE ".*/" "" _repo "${_base}")
        string(REGEX REPLACE "\\.git$" "" _repo "${_repo}")
        if("${_repo}" STREQUAL "${name}")
            string(JSON _ref GET "${_json_value}" "target" "branches" "${_key}")
            set(${out_url} "${_key}" PARENT_SCOPE)
            set(${out_ref} "${_ref}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

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
        # A 40-hex ref is a pin: warn (never fail) when the existing checkout
        # is not at it, so builds from stale library trees cannot silently
        # diverge from what a fresh clone would produce.
        string(LENGTH "${branch}" _branch_len)
        if("${type}" STREQUAL "git" AND _branch_len EQUAL 40 AND "${branch}" MATCHES "^[0-9a-fA-F]+$")
            execute_process(
                COMMAND git rev-parse HEAD
                WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/${dir}/${name}"
                OUTPUT_VARIABLE _head OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
            string(TOLOWER "${branch}" _pin_lc)
            if(_head AND NOT "${_head}" STREQUAL "${_pin_lc}")
                message("${BoldYellow}WARNING: ${dir}/${name} HEAD is ${_head} but the pin wants ${_pin_lc} - run ./build.py --update${ColourReset}")
            endif()
        endif()
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
