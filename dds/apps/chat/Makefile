#
# Makefile for DDS test application.
#
#######################################################

ALT_NUM = 0
ALT_STR =
#ALT_NUM = 1
#ALT_STR = -DALT1
#ALT_NUM = 2
#ALT_STR = -DALT2

EXECUTABLE = chat${ALT_NUM}

BASE       = ../../src

RTPS       = ${BASE}/rtps
TRANS      = ${BASE}/trans
DISC       = ${BASE}/disc
CACHE      = ${BASE}/cache
DCPS       = ${BASE}/dcps
DDS        = ${BASE}/dds
CO         = ${BASE}/co
DBG        = ${BASE}/dbg
#TYPE       = ${BASE}/typecode
TYPE       = ${BASE}/xtypes
SQL        = ${BASE}/sql
SECURITY   = ${BASE}/security
DYNIP	   = ${BASE}/dynip

dds_CSRCS  = ${DDS}/dds.c ${DDS}/domain.c ${DDS}/locator.c ${DDS}/guid.c \
             ${DDS}/dds_seq.c
type_CSRCS = ${TYPE}/xtypecode.c ${TYPE}/xtypes.c ${TYPE}/xdata.c \
             ${TYPE}/xcdr.c ${TYPE}/tsm.c ${TYPE}/pl_cdr.c ${TYPE}/pid.c \
             ${TYPE}/xtopic.c ${TYPE}/builtin.c ${TYPE}/vtc.c
#type_CSRCS = ${TYPE}/typecode.c ${TYPE}/cdr.c ${TYPE}/pl_cdr.c ${TYPE}/pid.c
dcps_CSRCS = ${DCPS}/dcps_main.c ${DCPS}/uqos.c ${DCPS}/guard.c
cache_CSRCS= ${CACHE}/cache.c
disc_CSRCS = ${DISC}/disc.c
rtps_CSRCS = ${RTPS}/rtps_main.c ${RTPS}/rtps_mux.c
trans_CSRCS= ${TRANS}/ip/rtps_ip.c ${TRANS}/ip/ri_udp.c ${TRANS}/ip/ri_dtls.c
sec_CSRCS  = ${SECURITY}/security.c
co_CSRCS   = ${CO}/pool.c ${CO}/sys.c ${CO}/error.c ${CO}/ipc.c ${CO}/timer.c \
             ${CO}/sock.c ${CO}/skiplist.c ${CO}/str.c ${CO}/heap.c ${CO}/md5.c \
             ${CO}/db.c ${CO}/handle.c ${CO}/tty.c ${CO}/log.c ${CO}/ctrace.c \
             ${CO}/thread.c ${CO}/prof.c ${CO}/hash.c ${CO}/strseq.c \
             ${CO}/nmatch.c ${CO}/random.c ${CO}/ipfilter.c ${CO}/libx.c \
#            ../secplug/msecplug.c
sql_CSRCS  = ${SQL}/scan.c ${SQL}/parse.c ${SQL}/bytecode.c
dynip_CSRCS= ${DYNIP}/di_main.c ${DYNIP}/di_linux.c
dbg_CSRCS  = ${DBG}/debug.c

prog_CSRCS = main.c chat_msg.c ${dds_CSRCS} ${type_CSRCS} ${dcps_CSRCS} ${disc_CSRCS} \
             ${cache_CSRCS} ${dbg_CSRCS} ${sql_CSRCS} ${dynip_CSRCS} ${co_CSRCS}
#${sec_CSRCS} 

#comment the following && don't define RTPS_USED to drop RTPS/Discovery
prog_CSRCS += ${rtps_CSRCS} ${trans_CSRCS}

prog_COBJS = ${prog_CSRCS:.c=.o}
prog_CHDRS = ${BASE}/include/*.h ../../api/headers/dds/*.h chat_msg.h

prog_OBJS  = ${prog_COBJS}

${EXECUTABLE}:	${prog_OBJS}
		gcc -o ${EXECUTABLE} ${prog_OBJS} ${LDFLAGS}

#######################################################
# Setup Compile Environment

CC       = gcc

INC_PATH = -I${BASE}/include -I../../api/headers
LIB_PATH = 
LIBS     = 

DEFINES  = -DPTHREADS_USED -DFORCE_MALLOC -DTTY_NORMAL -DRTPS_USED -DCDD_USED -DDDS_DEBUG \
	   -DEXTRA_STATS -DLOG_FILE -DXTYPES_USED -DDDS_IPV6 -DDDS_TYPECODE
#-DDDS_BUILTINS -
#-DDDS_AUTO_LIVELINESS
#-DDDS_SECURITY
#-DDDS_IP_BCAST
#-DDDS_AUTO_LIVELINESS
#-DFORCE_MALLOC
#-DDDS_IP_BCAST
#-DLOG_FILE -DRTPS_TRACE -DRTPS_SEDP_TRACE
#-DDDS_NO_MCAST
#-DDDS_SERVER
#-DPROFILE 
#-DSTATIC_DISCOVERY
#-DCDD_USED
#-DLOG_FILE 
#-DRTPS_USED
#-DRTPS_TRACE 
#-DDDS_DEBUG 
#-DLOCK_TRACE
#-DCTRACE_USED
#-DDUMP_LOCATORS
#-DLOG_DOMAIN
#-DVALGRIND_USED
#-DDDS_STATUS
#-DFORCE_MALLOC
#-DFILT_VMWARE 

OPTTYPE  = -O0
# -fprofile-arcs -ftest-coverage

CFLAGS   = -Wall -Wextra -pedantic -Wno-long-long ${OPTTYPE} ${INC_PATH} ${DEFINES} ${ALT_STR} -g
LDFLAGS  = ${LIB_PATH} ${LIBS} -lnsl -lrt -lssl -g
# -lgcov

all: ${EXECUTABLE}

#######################################################
# Rules for cleaning

clean:
	\rm -f ${prog_OBJS} ${EXECUTABLE} core.*

reallyclean: clean
	\rm -f .log_file* ${BASE}/*/tags Makefile.bak tags

########################################################

# Object sizes

size:
	\size ${prog_OBJS} ${EXECUTABLE}

# coverage

coverage:
	cd ../api/coverage
	lcov -b ../../dcps --directory=../../.. --capture --output-file api.info

# Lint (if available)
LINT=/home/jvoe/bin/flint

lint:	${prog_CSRCS}
	\${LINT} std.lnt ${prog_CSRCS}

# Additional dependencies.

depend:
	\makedepend ${INC_PATH} ../*/*.[ch]
