# include $(COMPONENT_PATH)/../ble_common.mk
# Add by qcc74x sdk
sdk_generate_library()
# 
# # Component Makefile
# #
# ## These include paths would be exported to project level
# COMPONENT_ADD_INCLUDEDIRS += ./ \
# 							 common/include \
# 							 common/btc/include \
# 							 qcc74x_ble_mesh/api \
# 							 qcc74x_ble_mesh/api/core/include \
# 							 qcc74x_ble_mesh/api/models/include \
# 							 qcc74x_ble_mesh/btc/include \
# 							 qcc74x_ble_mesh/mesh_common/include \
# 							 qcc74x_ble_mesh/mesh_models/include \
# 							 qcc74x_ble_mesh/mesh_models/server/include \
# 							 qcc74x_ble_mesh/mesh_models/client/include \
# 							 qcc74x_ble_mesh/mesh_models/common/include
sdk_add_include_directories(
	./
	common/include
	common/btc/include
	qcc74x_ble_mesh/api
	qcc74x_ble_mesh/api/core/include
	qcc74x_ble_mesh/api/models/include
	qcc74x_ble_mesh/btc/include
	qcc74x_ble_mesh/mesh_common/include
	qcc74x_ble_mesh/mesh_models/include
	qcc74x_ble_mesh/mesh_models/server/include
	qcc74x_ble_mesh/mesh_models/client/include
	qcc74x_ble_mesh/mesh_models/common/include
)
# 
# ## not be exported to project level
# COMPONENT_PRIV_INCLUDEDIRS   :=
# 
# ## This component's src 
# #for mesh model source
# #common
# COMPONENT_SRCS += common/btc/core/btc_manage.c \
# 				  common/btc/core/btc_task.c \
# 				  qcc74x_ble_mesh/mesh_common/mesh_mutex.c \
# 				  qcc74x_ble_mesh/mesh_common/mesh_common.c \
# 				  qcc74x_ble_mesh/api/core/qcc74x_ble_mesh_local_data_operation_api.c \
# 				  qcc74x_ble_mesh/api/core/qcc74x_ble_mesh_networking_api.c \
# 				  qcc74x_ble_mesh/api/models/qcc74x_ble_mesh_generic_model_api.c \
# 				  qcc74x_ble_mesh/api/models/qcc74x_ble_mesh_lighting_model_api.c \
# 				  qcc74x_ble_mesh/api/models/qcc74x_ble_mesh_time_scene_model_api.c \
# 				  qcc74x_ble_mesh/btc/btc_ble_mesh_generic_model.c \
# 				  qcc74x_ble_mesh/btc/btc_ble_mesh_lighting_model.c \
# 				  qcc74x_ble_mesh/btc/btc_ble_mesh_prov.c \
# 				  qcc74x_ble_mesh/btc/btc_ble_mesh_sensor_model.c \
# 				  qcc74x_ble_mesh/btc/btc_ble_mesh_time_scene_model.c
sdk_library_add_sources(
	common/btc/core/btc_manage.c
	common/btc/core/btc_task.c
	qcc74x_ble_mesh/mesh_common/mesh_mutex.c
	qcc74x_ble_mesh/mesh_common/mesh_common.c
	qcc74x_ble_mesh/api/core/qcc74x_ble_mesh_local_data_operation_api.c
	qcc74x_ble_mesh/api/core/qcc74x_ble_mesh_networking_api.c
	qcc74x_ble_mesh/api/models/qcc74x_ble_mesh_generic_model_api.c
	qcc74x_ble_mesh/api/models/qcc74x_ble_mesh_lighting_model_api.c
	qcc74x_ble_mesh/api/models/qcc74x_ble_mesh_time_scene_model_api.c
	qcc74x_ble_mesh/btc/btc_ble_mesh_generic_model.c
	qcc74x_ble_mesh/btc/btc_ble_mesh_lighting_model.c
	qcc74x_ble_mesh/btc/btc_ble_mesh_prov.c
	qcc74x_ble_mesh/btc/btc_ble_mesh_sensor_model.c
	qcc74x_ble_mesh/btc/btc_ble_mesh_time_scene_model.c
)
# #server
# COMPONENT_SRCS += qcc74x_ble_mesh/mesh_models/server/generic_server.c \
# 				  qcc74x_ble_mesh/mesh_models/server/lighting_server.c \
# 				  qcc74x_ble_mesh/mesh_models/server/state_transition.c \
# 				  qcc74x_ble_mesh/mesh_models/server/state_binding.c \
# 				  qcc74x_ble_mesh/mesh_models/server/device_property.c \
# 				  qcc74x_ble_mesh/mesh_models/server/server_common.c \
# 				  qcc74x_ble_mesh/mesh_models/server/sensor_server.c \
# 				  qcc74x_ble_mesh/mesh_models/server/time_scene_server.c
sdk_library_add_sources(
	qcc74x_ble_mesh/mesh_models/server/generic_server.c
	qcc74x_ble_mesh/mesh_models/server/lighting_server.c
	qcc74x_ble_mesh/mesh_models/server/state_transition.c
	qcc74x_ble_mesh/mesh_models/server/state_binding.c
	qcc74x_ble_mesh/mesh_models/server/device_property.c
	qcc74x_ble_mesh/mesh_models/server/server_common.c
	qcc74x_ble_mesh/mesh_models/server/sensor_server.c
	qcc74x_ble_mesh/mesh_models/server/time_scene_server.c
)
# #client
# COMPONENT_SRCS += qcc74x_ble_mesh/mesh_models/client/generic_client.c \
# 				  qcc74x_ble_mesh/mesh_models/client/lighting_client.c \
# 				  qcc74x_ble_mesh/mesh_models/client/client_common.c \
# 				  qcc74x_ble_mesh/mesh_models/client/sensor_client.c \
# 				  qcc74x_ble_mesh/mesh_models/client/time_scene_client.c
sdk_library_add_sources(
	qcc74x_ble_mesh/mesh_models/client/generic_client.c
	qcc74x_ble_mesh/mesh_models/client/lighting_client.c
	qcc74x_ble_mesh/mesh_models/client/client_common.c
	qcc74x_ble_mesh/mesh_models/client/sensor_client.c
	qcc74x_ble_mesh/mesh_models/client/time_scene_client.c
)
# 
# #for mesh model source end
# 
# COMPONENT_OBJS   := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
# #for mesh model source
# COMPONENT_SRCDIRS := qcc74x_ble_mesh/mesh_models/server \
# 					 qcc74x_ble_mesh/mesh_models/client \
# 					 qcc74x_ble_mesh/mesh_common \
# 					 qcc74x_ble_mesh/api/models \
# 					 qcc74x_ble_mesh/api/core \
# 					 qcc74x_ble_mesh/btc \
# 					 common/btc/core
# #for mesh model source end