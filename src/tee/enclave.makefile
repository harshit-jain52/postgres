SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM        # HW or SIM
SGX_ARCH ?= x64
SGX_DEBUG ?= 1

include $(SGX_SDK)/buildenv.mk

ifeq ($(SGX_MODE), HW)
    Urts_Library_Name := sgx_urts
    Trts_Library_Name := sgx_trts
    Service_Library_Name := sgx_tservice
else
    Urts_Library_Name := sgx_urts_sim
    Trts_Library_Name := sgx_trts_sim
    Service_Library_Name := sgx_tservice_sim
endif

##########################################
# Include Paths
##########################################

Enclave_Include_Paths := -I. \
                         -I$(SGX_SDK)/include \
                         -I$(SGX_SDK)/include/tlibc \
                         -I$(SGX_SDK)/include/libcxx \
                         -IEnclave

##########################################
# Targets
##########################################

all: enclave.signed.so Enclave_u.o

##########################################
# Generate EDL Stubs (Trusted + Untrusted)
##########################################

Enclave_t.c Enclave_u.c:
	$(SGX_SDK)/bin/$(SGX_ARCH)/sgx_edger8r \
	    Enclave/Enclave.edl \
	    --search-path $(SGX_SDK)/include \
	    --trusted-dir . \
	    --untrusted-dir .

##########################################
# Compile Trusted Stub
##########################################

Enclave_t.o: Enclave_t.c
	g++ -c Enclave_t.c $(Enclave_Include_Paths)

##########################################
# Compile Enclave Logic
##########################################

Enclave/Enclave.o: Enclave/Enclave.cpp
	g++ -c Enclave/Enclave.cpp \
	    -o Enclave/Enclave.o \
	    $(Enclave_Include_Paths) \
	    -fPIC

##########################################
# Build Enclave Shared Object
##########################################

enclave.so: Enclave_t.o Enclave/Enclave.o Enclave/Enclave.lds
	g++ -shared -o enclave.so Enclave_t.o Enclave/Enclave.o \
	    -nostdlib -nodefaultlibs -nostartfiles \
	    -L$(SGX_SDK)/lib64 \
	    -Wl,--no-undefined \
	    -Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	    -Wl,--start-group -l$(Service_Library_Name) -lsgx_tstdc -lsgx_tcrypto -Wl,--end-group \
	    -Wl,-Bstatic -Wl,-Bsymbolic \
	    -Wl,-pie,-eenclave_entry \
	    -Wl,--export-dynamic \
	    -Wl,--defsym,__ImageBase=0 \
	    -Wl,--version-script=Enclave/Enclave.lds

##########################################
# Sign Enclave
##########################################

enclave.signed.so: enclave.so Enclave/Enclave_private.pem Enclave/Enclave.config.xml
	$(SGX_SDK)/bin/$(SGX_ARCH)/sgx_sign sign \
	    -key Enclave/Enclave_private.pem \
	    -enclave enclave.so \
	    -out enclave.signed.so \
	    -config Enclave/Enclave.config.xml

##########################################
# Compile Untrusted Stub (for PostgreSQL)
##########################################

Enclave_u.o: Enclave_u.c
	g++ -c Enclave_u.c \
	    -I$(SGX_SDK)/include \
	    -fPIC

##########################################
# Auto-Generate Signing Key
##########################################

Enclave/Enclave_private.pem:
	openssl genrsa -out Enclave/Enclave_private.pem -3 3072

##########################################
# Auto-Generate Config
##########################################

Enclave/Enclave.config.xml:
	echo '<EnclaveConfiguration>' > Enclave/Enclave.config.xml
	echo '<ProdID>0</ProdID>' >> Enclave/Enclave.config.xml
	echo '<ISVSVN>0</ISVSVN>' >> Enclave/Enclave.config.xml
	echo '<StackMaxSize>0x40000</StackMaxSize>' >> Enclave/Enclave.config.xml
	echo '<HeapMaxSize>0x200000</HeapMaxSize>' >> Enclave/Enclave.config.xml
	echo '<TCSNum>1</TCSNum>' >> Enclave/Enclave.config.xml
	echo '</EnclaveConfiguration>' >> Enclave/Enclave.config.xml

##########################################
# Auto-Generate LDS
##########################################

Enclave/Enclave.lds:
	echo '{' > Enclave/Enclave.lds
	echo '    global:' >> Enclave/Enclave.lds
	echo '        enclave_entry;' >> Enclave/Enclave.lds
	echo '        g_global_data_sim;' >> Enclave/Enclave.lds
	echo '        g_global_data;' >> Enclave/Enclave.lds
	echo '    local:' >> Enclave/Enclave.lds
	echo '        *;' >> Enclave/Enclave.lds
	echo '};' >> Enclave/Enclave.lds

##########################################
# Clean
##########################################

clean:
	rm -f enclave.so enclave.signed.so \
	      Enclave_t.* Enclave_u.* \
	      *.o \
	      Enclave/Enclave.lds \
	      Enclave/Enclave.config.xml \
	      Enclave/Enclave_private.pem
