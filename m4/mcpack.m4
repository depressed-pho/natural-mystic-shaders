# -*- autoconf -*-
# -----------------------------------------------------------------------------
AC_DEFUN([AX_MCPACK_INSTALLATION_DIRECTORY],
[
    AC_ARG_VAR(
        [MCPACK_INSTALLATION_DIRECTORY_][$1],
        [Installation directory for add-on `$1' @<:@none@:>@])
])
