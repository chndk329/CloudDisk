INCLUDES := ../include
SRCS := $(wildcard ../src/*.cc)
LIBS := -lwfrest -lworkflow -lssl -lcrypto -lcrypt -lSimpleAmqpClient
CONSUMERLIBS := -fno-rtti -lalibabacloud-oss-cpp-sdk -lcurl -lpthread  -lssl -lcrypto -lcrypt -lSimpleAmqpClient
OBJS := $(patsubst ../src/%.cc, obj/%.o, $(SRCS))

SERVER := CloudDiskServer
CONSUMER := Consumer

.PHONY: all clean rebuild

all: $(SERVER)

$(SERVER): $(OBJS) obj/main.o
	g++ $^ -o $@ $(LIBS) -g

$(CONSUMER): Consumer.o
	g++ $^ -o $@ $(CONSUMERLIBS) -g

obj/%.o: ../src/%.cc
	@mkdir -p obj
	g++ -c $< -o $@ $(addprefix -I, $(INCLUDES)) -g

clean:
	rm -rf obj $(SERVER)

rebuild: clean all
