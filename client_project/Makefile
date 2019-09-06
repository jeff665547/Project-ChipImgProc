 # modify this path to your ChipImgProc install directory
ChipImgProc_INSTALL_PATH = D:/git/centrillion/ChipImgProc/stage

FLAGS = -std=c++17

DEFINE = -DNUCLEONA_RANGE_USE_V3
DEFINE += -DOPENCV_TRAITS_ENABLE_DEPRECATED

INCLUDE  = -I$(ChipImgProc_INSTALL_PATH)/include
INCLUDE += -isystem $(ChipImgProc_INSTALL_PATH)/third_party/include
INCLUDE += -isystem $(ChipImgProc_INSTALL_PATH)/third_party/include/opencv


LIB_DIR = -L$(ChipImgProc_INSTALL_PATH)/lib 
LIB_DIR += -L${ChipImgProc_INSTALL_PATH}/third_party/lib
LIB_DIR += -L$(ChipImgProc_INSTALL_PATH)/third_party/x64/mingw/staticlib

LIB = -lChipImgProc-logger -lChipImgProc-utils -lcpp_base64-base64
LIB += -lNucleona -lNucleona-sys-executable_dir -lNucleona-stream-null_buffer -lNucleona-parallel-thread_pool -lNucleona-util
LIB += -lopencv_ml340 -lopencv_objdetect340 -lopencv_shape340 -lopencv_stitching340 -lopencv_superres340  -lopencv_videostab340 -lopencv_calib3d340  -lopencv_features2d340 -lopencv_flann340  -lopencv_highgui340 
LIB += -lopencv_photo340 -lopencv_video340 -lopencv_videoio340 -lopencv_imgcodecs340 
LIB += -lopencv_imgproc340
LIB += -lopencv_core340
LIB += -llibjasper
LIB += -ljpeg -llibwebp -lpng -llibtiff
LIB += -lboost_filesystem-mt-x64
LIB += -lzlib
LIB += -lcomctl32 -lgdi32 -lsetupapi -lws2_32 -lm
LIB += -lavifil32 -lavicap32 -lwinmm -lmsvfw32 -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 


all:
	g++ $(INCLUDE) $(FLAGS) $(DEFINE) main.cpp -o main.exe $(LIB_DIR) ${LIB}