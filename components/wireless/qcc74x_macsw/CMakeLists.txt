sdk_add_include_directories(include)
if(DEFINED CONFIG_LPAPP)
sdk_add_static_library(lib/libqcc74x_macsw_lp.a)
elseif(DEFINED CONFIG_COEX)
sdk_add_static_library(lib/libqcc74x_macsw_coex.a)
elseif(DEFINED CONFIG_WFA)
sdk_add_static_library(lib/libqcc74x_macsw_wfa.a)
elseif(DEFINED CONFIG_ADHOC)
sdk_add_static_library(lib/libqcc74x_macsw_adhoc.a)
else()
sdk_add_static_library(lib/libqcc74x_macsw.a)
endif()
