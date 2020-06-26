# -*- autoconf -*-
# -----------------------------------------------------------------------------
# Usage: AX_MCPACK_INSTALLATION_DIRECTORY
# -----------------------------------------------------------------------------
AC_DEFUN([AX_MCPACK_INSTALLATION_DIRECTORY],
[
    AC_ARG_VAR(
        [MCPACK_INSTALLATION_DIRECTORY_][$1],
        [Installation directory for add-on `$1' @<:@none@:>@])
])
# -----------------------------------------------------------------------------
# Usage: AX_MCPACK_GENERATE_DIFF_FILE(yes|no)
# -----------------------------------------------------------------------------
AC_DEFUN([AX_MCPACK_GENERATE_DIFF_FILE],
[
    AM_CONDITIONAL(
        [GENERATE_DIFF_FILE],
        [test x"$1" != x"no"])
])
