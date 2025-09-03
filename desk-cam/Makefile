
DESK_CAM_ROOT:=$(CURDIR)/../
DESK_CAM_OUT:=$(CURDIR)/out/

COMPILER:=$(DESK_CAM_ROOT)/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc
SRC+=main.c
INC:=
LIBS:=

# add rockit lib
INC += -I$(DESK_CAM_ROOT)/media/rockit/rockit/mpi/sdk/include/
LIBS += -L$(DESK_CAM_ROOT)/media/rockit/rockit/lib/lib32/
LIBS += -L$(DESK_CAM_ROOT)/media/rga/release_rga_rv1106_arm-rockchip830-linux-uclibcgnueabihf/lib
LIBS += -lrockit -lrga

# add mpp lib
LIBS += -L$(DESK_CAM_ROOT)/media/mpp/release_mpp_rv1106_arm-rockchip830-linux-uclibcgnueabihf/lib
LIBS += -lrockchip_mpp

# add isp lib
INC += -I$(DESK_CAM_ROOT)/media/isp/release_camera_engine_rkaiq_rv1106_arm-rockchip830-linux-uclibcgnueabihf/include/rkaiq/uAPI2
INC += -I$(DESK_CAM_ROOT)/media/isp/release_camera_engine_rkaiq_rv1106_arm-rockchip830-linux-uclibcgnueabihf/include/rkaiq/common/
INC += -I$(DESK_CAM_ROOT)/media/isp/release_camera_engine_rkaiq_rv1106_arm-rockchip830-linux-uclibcgnueabihf/include/rkaiq/xcore/
INC += -I$(DESK_CAM_ROOT)/media/isp/release_camera_engine_rkaiq_rv1106_arm-rockchip830-linux-uclibcgnueabihf/include/rkaiq/
INC += -I$(DESK_CAM_ROOT)/media/isp/release_camera_engine_rkaiq_rv1106_arm-rockchip830-linux-uclibcgnueabihf/include/rkaiq/algos
INC += -I$(DESK_CAM_ROOT)/media/isp/release_camera_engine_rkaiq_rv1106_arm-rockchip830-linux-uclibcgnueabihf/include/rkaiq/iq_parser/
INC += -I$(DESK_CAM_ROOT)/media/isp/release_camera_engine_rkaiq_rv1106_arm-rockchip830-linux-uclibcgnueabihf/include/rkaiq/iq_parser_v2/
LIBS += -L$(DESK_CAM_ROOT)/media/isp/release_camera_engine_rkaiq_rv1106_arm-rockchip830-linux-uclibcgnueabihf/lib
LIBS += -lrkaiq

# add rtsp lib
INC += -I$(DESK_CAM_ROOT)/media/common_algorithm/common_algorithm/misc/include
LIBS += -L$(DESK_CAM_ROOT)/media/common_algorithm/common_algorithm/misc/lib/arm-rockchip830-linux-uclibcgnueabihf/
LIBS += -lrtsp

all:
	if [ ! -d "$(DESK_CAM_OUT)"]; then \
		mkdir $(DESK_CAM_OUT); \
	fi
	bear -- $(COMPILER) -o $(DESK_CAM_OUT)/desk-cam $(SRC) $(INC) $(LIBS)

clean:
	rm -rf $(DESK_CAM_OUT)
