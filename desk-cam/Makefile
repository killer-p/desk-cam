
DESK_CAM_ROOT:=$(CURDIR)/../
DESK_CAM_OUT:=$(CURDIR)/out/

COMPILER:=$(DESK_CAM_ROOT)/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc
SRC+=main.c
INC:=
LIBS:=

# add rockit lib
INC += -I$(DESK_CAM_ROOT)/media/rockit/rockit/mpi/sdk/include/
LIBS += -L$(DESK_CAM_ROOT)/media/rockit/rockit/lib/lib32/
LIBS += -lrockit_tiny

# add mpp lib
LIBS += -L$(DESK_CAM_ROOT)/media/mpp/release_mpp_rv1106_arm-rockchip830-linux-uclibcgnueabihf/lib
LIBS += -lrockchip_mpp

all:
	mkdir $(DESK_CAM_OUT)
	bear -- $(COMPILER) -o $(DESK_CAM_OUT)/desk-cam $(SRC) $(INC) $(LIBS)

clean:
	rm -rf $(DESK_CAM_OUT)
