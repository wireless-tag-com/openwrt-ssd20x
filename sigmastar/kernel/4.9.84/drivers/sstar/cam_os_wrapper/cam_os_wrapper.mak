#-------------------------------------------------------------------------------
#	Description of some variables owned by the library
#-------------------------------------------------------------------------------
# Library module (lib) or Binary module (bin)
PROCESS = lib
PATH_C +=\
  $(PATH_cam_os_wrapper)/src \
  $(PATH_cam_os_wrapper)/test

PATH_H +=\
  $(PATH_cam_os_wrapper)/pub \
  $(PATH_cam_os_wrapper)/inc

#-------------------------------------------------------------------------------
#	List of source files of the library or executable to generate
#-------------------------------------------------------------------------------
SRC_C_LIST =\
  cam_os_wrapper.c \
  cam_os_informal_idr.c \
  cam_os_wrapper_test.c