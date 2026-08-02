# Thin wrapper over CMake's built-in string(JSON) parser.
#
# This supersedes the vendored "JSONParser.cmake" (Stefan Bellus, MIT): parsing
# is delegated to CMake's native JSON support instead of a hand-rolled
# tokenizer.  A side benefit is that native string(JSON) understands // and
# /* */ comments, so commented-out entries in codal.json are truly ignored.
#
# The public API and the flattened-variable contract are unchanged:
#
#   sbeParseJson(prefix jsonString)
#     - sets <prefix> to the list of all flattened variable names, and
#     - sets <prefix>.<path> to the value found at that path.
#
# As before, only leaves (scalars) and arrays emit a variable; an object emits
# variables for its scalar members only.  A <prefix>.<path> value is the raw
# JSON text with string quotes/escapes removed, so a JSON string "true" is a
# string while a bare true is the boolean true.
#
# Requires CMake >= 3.19 (introduction of string(JSON)).
#
# Implementation notes:
#  * JSON text is passed between macros by *variable name* and dereferenced
#    with ${${var}}.  Passing it as a macro argument would re-parse escape
#    sequences (e.g. \" inside a JSON string) and corrupt the text.
#  * Macros share scope, so the recursive walker must be reentrant.  Each
#    nested value is copied into a uniquely named _json_tmp_<n> variable, and
#    the loop counters are saved on/popped from LIFO stacks rather than stored
#    in shared variables that nested frames would clobber.
#  * Scalar leaves are never recursed into: a bare scalar is not a valid JSON
#    document, so string(JSON) would refuse to parse it.

if(DEFINED JSonParserGuard)
    return()
endif()
set(JSonParserGuard yes)

macro(sbeParseJson prefix jsonString)
    set(_json_all_vars "")
    set(_json_tmps "")
    set(_json_next 0)
    set(_json_stack_i "")
    set(_json_stack_len "")
    _sbeJsonFlatten("${prefix}" "${jsonString}")
    set("${prefix}" ${_json_all_vars})
    foreach(_json_t ${_json_tmps})
        unset(${_json_t})
    endforeach()
endmacro()

# Convert a native string(JSON) scalar to the value the old parser produced:
# booleans come back as ON/OFF but the contract expects true/false, and NULL
# members must yield "null".  <out> is the name of a variable holding the value.
macro(_sbeJsonLeafValue type out)
    if("${type}" STREQUAL "BOOLEAN")
        if("${${out}}" STREQUAL "ON")
            set(${out} "true")
        else()
            set(${out} "false")
        endif()
    elseif("${type}" STREQUAL "NULL")
        set(${out} "null")
    endif()
endmacro()

# Recursively flatten the JSON value stored in variable <json> into
# <prefix>.<path> variables.
macro(_sbeJsonFlatten prefix json)
    string(JSON _json_type TYPE "${${json}}")

    if("${_json_type}" STREQUAL "OBJECT")
        string(JSON _json_len LENGTH "${${json}}")
        set(_json_i 0)
        while(_json_i LESS _json_len)
            string(JSON _json_key MEMBER "${${json}}" ${_json_i})
            string(JSON _json_child_type TYPE "${${json}}" ${_json_key})
            if("${_json_child_type}" STREQUAL "OBJECT"
                    OR "${_json_child_type}" STREQUAL "ARRAY")
                string(JSON _json_val GET "${${json}}" ${_json_key})
                list(APPEND _json_stack_i ${_json_i})
                list(APPEND _json_stack_len ${_json_len})
                set(_json_tmp "_json_tmp_${_json_next}")
                math(EXPR _json_next "${_json_next} + 1")
                list(APPEND _json_tmps "${_json_tmp}")
                set("${_json_tmp}" "${_json_val}")
                _sbeJsonFlatten("${prefix}.${_json_key}" "${_json_tmp}")
                list(GET _json_stack_i -1 _json_i)
                list(REMOVE_AT _json_stack_i -1)
                list(GET _json_stack_len -1 _json_len)
                list(REMOVE_AT _json_stack_len -1)
            else()
                string(JSON _json_leaf GET "${${json}}" ${_json_key})
                _sbeJsonLeafValue("${_json_child_type}" _json_leaf)
                # unquoted: matches the old parser, which dropped a trailing ';'
                set("${prefix}.${_json_key}" ${_json_leaf})
                list(APPEND _json_all_vars "${prefix}.${_json_key}")
            endif()
            math(EXPR _json_i "${_json_i} + 1")
        endwhile()

    elseif("${_json_type}" STREQUAL "ARRAY")
        # the array variable itself holds the element indices (0;1;2;...)
        set("${prefix}" "")
        list(APPEND _json_all_vars "${prefix}")
        string(JSON _json_len LENGTH "${${json}}")
        set(_json_i 0)
        while(_json_i LESS _json_len)
            list(APPEND "${prefix}" ${_json_i})
            string(JSON _json_child_type TYPE "${${json}}" ${_json_i})
            if("${_json_child_type}" STREQUAL "OBJECT"
                    OR "${_json_child_type}" STREQUAL "ARRAY")
                string(JSON _json_val GET "${${json}}" ${_json_i})
                list(APPEND _json_stack_i ${_json_i})
                list(APPEND _json_stack_len ${_json_len})
                set(_json_tmp "_json_tmp_${_json_next}")
                math(EXPR _json_next "${_json_next} + 1")
                list(APPEND _json_tmps "${_json_tmp}")
                set("${_json_tmp}" "${_json_val}")
                _sbeJsonFlatten("${prefix}_${_json_i}" "${_json_tmp}")
                list(GET _json_stack_i -1 _json_i)
                list(REMOVE_AT _json_stack_i -1)
                list(GET _json_stack_len -1 _json_len)
                list(REMOVE_AT _json_stack_len -1)
            else()
                string(JSON _json_leaf GET "${${json}}" ${_json_i})
                _sbeJsonLeafValue("${_json_child_type}" _json_leaf)
                set("${prefix}_${_json_i}" ${_json_leaf})
                list(APPEND _json_all_vars "${prefix}_${_json_i}")
            endif()
            math(EXPR _json_i "${_json_i} + 1")
        endwhile()

    else()
        # scalar at the top level (not a full JSON document, but keep the value)
        set("${prefix}" ${${json}})
        list(APPEND _json_all_vars "${prefix}")
    endif()
endmacro()
