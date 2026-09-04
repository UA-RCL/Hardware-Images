APP = pm-app
LDLIBS += -lreadline

all: $(APP)

OBJS = pm-app.o layout.o dma.o

$(APP): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

pm-app.o: pm-app.c layout.h dma.h pm_layout.h
layout.o: layout.c layout.h pm_layout.h
dma.o:    dma.c dma.h layout.h pm_layout.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f $(APP) *.o
