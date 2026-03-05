SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= HW
SGX_ARCH ?= x64
SGX_DEBUG ?= 1

include $(SGX_SDK)/buildenv.mk

ifeq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
	App_Link_Flags := -l$(Urts_Library_Name) -lpthread
else
	Urts_Library_Name := sgx_urts_sim
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
	App_Link_Flags := -L$(SGX_SDK)/lib64 -l$(Urts_Library_Name) -lpthread
endif

App_Cpp_Files := sgx_service.cpp
Enclave_Include_Paths := -I. -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc -I$(SGX_SDK)/include/libcxx
Enclave_Cpp_Files := Enclave/Enclave.cpp

all: sgx_service enclave.signed.so

# App Compilation
sgx_service: Enclave_u.o $(App_Cpp_Files) setup-group
	g++ -o sgx_service Enclave_u.o $(App_Cpp_Files) -I. -I$(SGX_SDK)/include $(App_Link_Flags)

setup-group:
	- sudo groupadd sgxabac
	- sudo usermod -aG sgxabac postgres
	- sudo usermod -aG sgxabac $(USER)

Enclave_u.o: Enclave_u.c
	g++ -c Enclave_u.c -I$(SGX_SDK)/include

Enclave_u.c:
	$(SGX_SDK)/bin/x64/sgx_edger8r --untrusted Enclave/Enclave.edl --search-path $(SGX_SDK)/include



# Enclave Compilation

enclave.signed.so: enclave.so
	$(SGX_SDK)/bin/x64/sgx_sign sign -key Enclave/Enclave_private.pem -enclave enclave.so -out enclave.signed.so -config Enclave/Enclave.config.xml



enclave.so: Enclave_t.o Enclave/Enclave.o
	g++ -shared -o enclave.so Enclave_t.o Enclave/Enclave.o -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_SDK)/lib64 \
	-Wl,--no-undefined \
	-Wl,--whole-archive -lsgx_trts -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tservice -lsgx_tstdc -lsgx_tcrypto -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
	-Wl,--defsym,__ImageBase=0 -Wl,--version-script=Enclave/Enclave.lds



Enclave_t.o: Enclave_t.c
	g++ -c Enclave_t.c $(Enclave_Include_Paths) -IEnclave



Enclave_t.c:
	$(SGX_SDK)/bin/x64/sgx_edger8r --trusted Enclave/Enclave.edl --search-path $(SGX_SDK)/include



Enclave/Enclave.o: Enclave/Enclave.cpp
	g++ -c Enclave/Enclave.cpp -o Enclave/Enclave.o $(Enclave_Include_Paths) -IEnclave



# Helper to generate a dummy key for signing

Enclave/Enclave_private.pem:
	openssl genrsa -out Enclave/Enclave_private.pem -3 3072


clean:
	rm -f sgx_service enclave.so enclave.signed.so *.o *_u.* *_t.*

db_clean: clean
	rm -f sealed_env.bin /tmp/sgx_abac.sock
