LIB:= libTracker.so
CXX:=  $(DDK_HOME)/toolchains/aarch64-linux-gcc6.3/bin/aarch64-linux-gnu-g++
#C:=  $(DDK_HOME)/toolchains/aarch64-linux-gcc6.3/bin/aarch64-linux-gnu-gcc
#CPPSRCS:= $(wildcard *.cpp)
#CSRCS:= $(wildcard basefunction\*.c baselist\*.c *.c)
INCS:= $(wildcard *.h basefunction\*.h baselist\*.h )

#PKGS:= opencv

OBJS:= ./basefunction/basefunction.o ./baselist/baselist.o ./tracker_engine.o ./zstracker.o
#COBJS:= $(SRCS:.c=.o)


CFLAGS:= -fPIC -I../common -I./baselist -I./basefunction -I./\
		-I$(DDK_HOME)/include/inc/ \
		-I$(DDK_HOME)/include/libc_sec/include/ \
		-I$(DDK_HOME)/include/third_party/protobuf/include/ \
		-I$(DDK_HOME)/include/third_party/cereal/include/ \
		-I$(DDK_HOME)/include/third_party/opencv/include/ \
		-I$(DDK_HOME)/include/third_party/opencv/include/opencv \
		-I$(DDK_HOME)/include/third_party/opencv/include/opencv2 \
	
LIBS:= -shared -Wl,-rpath, -Wl,-no-undefined \
	   -L$(DDK_HOME)/device/lib/ \
	    -laicpu_engine -lcce_aicore -lcce_aicpudev_alg -lcce_aicpudev -lcce -lcrypto -lc_sec \
		-ldevmm -ldrvaicpu -ldrvdevdrv -ldrvhdc -ldrvtcp -lDvpp_api -lDvpp_jpeg_decoder    \
		-lDvpp_jpeg_encoder -lDvpp_png_decoder -lDvpp_vpc -lfmk_common -lfusionengine   \
		-lgraph -lhiai_client -lhiai_common -lhiai_server -lidedaemon -lmatrixdaemon   \
		-lmemory -lmmpa -lome -lomg -lOMX_hisi_video_decoder -lOMX_hisi_video_encoder  \
		-lopencv_world -lprotobuf -lruntime -lslog -lssl -lteec -ltiling   \

all: $(LIB)
%.o: %.cpp $(INCS) Makefile
	@echo $(CFLAGS)
	$(CXX) -c -o $@ $(CFLAGS) $<

$(LIB): $(OBJS) Makefile
	@echo $(CFLAGS)
	$(CXX) -o $@ $(OBJS) $(LIBS)
	cp ./libTracker.so /home/HwHiAiUser/projects/bin/Debug/

clean:
	rm -rf $(OBJS) $(LIB)
	rm -rf /home/HwHiAiUser/projects/bin/Debug/libTracker.so