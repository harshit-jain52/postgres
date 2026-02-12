#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for src/tee
#
# IDENTIFICATION
#    src/tee/Makefile
#
#-------------------------------------------------------------------------

subdir = src/tee
top_builddir = ../..
include $(top_builddir)/src/Makefile.global

OBJS = \
	Enclave_u.o \
	sgx_global.o \
	sgx_ocalls.o \

include $(top_srcdir)/src/backend/common.mk
CFLAGS += -I$(top_srcdir)/src/tee
CFLAGS += -I/opt/intel/sgxsdk/include
