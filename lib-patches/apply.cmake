# The changes lib/rt64, lib/RecompFrontend and lib/N64ModernRuntime need, which are upstream's commits: first this
# game's own, as a patch a project, then N64Recomp-browser's patches/ (the web support) on top of them.
#
# - rt64.patch: the players' shadows (a G_ZS_PRIM draw's depth exactly its prim depth), the images the game draws and
#   then samples as textures read from RDRAM rather than through GPU tile copies (shadows, the speed board, the smash
#   replay's frame), a texture address the RDP wraps to 24 bits, and an inverted load tile clamped. Each change's
#   comment says what it fixes.
#
# CMakeLists.txt applies them when it configures, and tools/build_web.sh before it builds N64Recomp from
# lib/N64ModernRuntime/N64Recomp, from the repository's root:
#
#   cmake -P lib-patches/apply.cmake lib
#
# A patch that is in already is left as it is, so running it again does nothing. What went in is kept in the
# checkout's git dir (mt-<name>.patch), so a patch edited since (a newer commit of this repository) comes out of a
# checkout as that copy, after N64Recomp-browser's patch on top of it, which goes back in after. A copy whose patch
# isn't in the checkout any more (put back to its commit) is dropped, and the patch goes in again. git apply --reverse
# "$(git rev-parse --git-path mt-<name>.patch)" in a checkout takes one of this game's out by hand (after
# N64Recomp-browser's, which lib/N64Recomp-browser/patches/apply.cmake's header says how to take out).

set(MT_LIB_PATCHES_DIR "${CMAKE_CURRENT_LIST_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/../lib/N64Recomp-browser/patches/apply.cmake")

function(mt_apply_lib_patches lib)
    foreach (name rt64)
        set(patch "${MT_LIB_PATCHES_DIR}/${name}.patch")
        set(dir "${lib}/${name}")
        execute_process(COMMAND git rev-parse --path-format=absolute --git-path "mt-${name}.patch"
            WORKING_DIRECTORY "${dir}" OUTPUT_VARIABLE copy OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND git rev-parse --path-format=absolute --git-path "n64recomp-browser-${name}"
            WORKING_DIRECTORY "${dir}" OUTPUT_VARIABLE web OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (EXISTS "${copy}")
            # In still? With the web's patch on top (its copy comes out), or alone (this one's copy comes out). A
            # checkout put back to its commit (publish_pages.sh does when N64Recomp-browser moves) holds neither, and
            # the copy is left from before.
            set(web_in FALSE)
            if (EXISTS "${web}.patch")
                execute_process(COMMAND git apply --whitespace=nowarn --reverse --check "${web}.patch"
                    WORKING_DIRECTORY "${dir}" RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
                if (result EQUAL 0)
                    set(web_in TRUE)
                endif()
            endif()
            set(in ${web_in})
            if (NOT in)
                execute_process(COMMAND git apply --whitespace=nowarn --reverse --check "${copy}"
                    WORKING_DIRECTORY "${dir}" RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
                if (result EQUAL 0)
                    set(in TRUE)
                endif()
            endif()
            file(SHA1 "${copy}" had)
            file(SHA1 "${patch}" has)
            if (NOT in)
                file(REMOVE "${copy}")
            elseif (had STREQUAL has)
                continue()
            else()
                # Edited since it went in: the web's patch comes out (its own copy of what it changed), then the old one.
                if (web_in)
                    execute_process(COMMAND git apply --whitespace=nowarn --reverse "${web}.patch" WORKING_DIRECTORY "${dir}"
                        RESULT_VARIABLE result)
                    if (NOT result EQUAL 0)
                        message(FATAL_ERROR "can't take N64Recomp-browser's ${name}.patch out of ${dir}")
                    endif()
                    file(REMOVE "${web}.patch" "${web}.from" "${web}")
                endif()
                execute_process(COMMAND git apply --whitespace=nowarn --reverse "${copy}" WORKING_DIRECTORY "${dir}"
                    RESULT_VARIABLE result)
                if (NOT result EQUAL 0)
                    message(FATAL_ERROR "can't take the older lib-patches/${name}.patch out of ${dir}")
                endif()
                file(REMOVE "${copy}")
                message(STATUS "Took the older lib-patches/${name}.patch out of ${name} (${dir})")
            endif()
        else()
            # In already without a copy (by hand, or by an apply.cmake that kept none).
            execute_process(COMMAND git apply --reverse --check "${patch}" WORKING_DIRECTORY "${dir}"
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
            if (result EQUAL 0)
                configure_file("${patch}" "${copy}" COPYONLY)
                continue()
            endif()
        endif()
        execute_process(COMMAND git apply --whitespace=nowarn "${patch}" WORKING_DIRECTORY "${dir}"
            RESULT_VARIABLE result ERROR_VARIABLE error)
        if (NOT result EQUAL 0)
            message(FATAL_ERROR "lib-patches/${name}.patch doesn't apply to ${dir}:\n${error}")
        endif()
        configure_file("${patch}" "${copy}" COPYONLY)
        message(STATUS "Patched ${name} (${dir}) with lib-patches/${name}.patch")
    endforeach()
    n64recomp_browser_apply_patches("${lib}")
endfunction()

if (CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
    if (NOT CMAKE_ARGV3)
        message(FATAL_ERROR "usage: cmake -P lib-patches/apply.cmake <the folder holding rt64, N64ModernRuntime and RecompFrontend>")
    endif()
    get_filename_component(lib "${CMAKE_ARGV3}" ABSOLUTE)
    mt_apply_lib_patches("${lib}")
endif()
