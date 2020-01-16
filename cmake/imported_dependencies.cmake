include(${CMAKE_CURRENT_LIST_DIR}/scripts.cmake)

# the names of the targets need to differ from the library filenames
# this is especially an issue with libcairo, where the library is called libcairo
# therefore, all libs imported this way have been prefixed with lib
import_pkgconfig_target(TARGET_NAME libglib PKGCONFIG_TARGET glib-2.0>=2.40)
import_pkgconfig_target(TARGET_NAME libgobject PKGCONFIG_TARGET gobject-2.0>=2.40)
import_pkgconfig_target(TARGET_NAME libgio PKGCONFIG_TARGET gio-2.0>=2.40)
import_pkgconfig_target(TARGET_NAME libzlib PKGCONFIG_TARGET zlib)
import_pkgconfig_target(TARGET_NAME libcairo PKGCONFIG_TARGET cairo)
import_pkgconfig_target(TARGET_NAME librsvg PKGCONFIG_TARGET librsvg-2.0)
