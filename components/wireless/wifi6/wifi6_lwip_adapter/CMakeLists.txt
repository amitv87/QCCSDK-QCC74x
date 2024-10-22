sdk_generate_library()

sdk_library_add_sources(cli_al.c)
sdk_library_add_sources(net_al.c)
sdk_library_add_sources(net_al_ext.c)
sdk_library_add_sources(net_iperf_al.c)
sdk_library_add_sources(wifi_pkt_hooks.c)
sdk_library_add_sources(tx_buffer_copy.c)

sdk_library_add_sources_ifdef(CONFIG_WIFI_TG net_tg_al.c net_tg_al_priv.c net_ping_al_priv.c)

sdk_add_include_directories(include)
sdk_add_include_directories(.)

sdk_add_compile_definitions(-DPLATFORM_PRIVDE_ERRNO)
