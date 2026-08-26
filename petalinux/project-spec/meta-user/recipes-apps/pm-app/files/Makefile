APP = pm-app
LDLIBS += -lreadline

all: $(APP)

$(APP): pm-app.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f $(APP) *.o
