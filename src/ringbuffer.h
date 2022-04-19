#define CAPACITY 128

typedef struct ringbuffer
{
    int32_t head;
    int32_t tail;
    char thebuffer[CAPACITY+1];
}Ringbuffer;

// init or reset buffer
void ringbuffer_set(Ringbuffer *buffer) 
{
    buffer->head = -1;
    buffer->tail = -1;
}

bool ringbuffer_add(Ringbuffer *buffer, char byte)
{
    if ( ((buffer->tail + 1) % CAPACITY) == buffer->head) {
        return false; // buffer is full
    }
    else {
        buffer->tail = (buffer->tail + 1) % CAPACITY;
        (buffer->thebuffer)[buffer->tail] = byte;
        return true;
    } 
    
}
char ringbuffer_del(Ringbuffer *buffer)
{
    if (buffer->head == buffer->tail) {
        return 0; // buffer is empty
    }
    else {
        buffer->head = (buffer->head + 1) % CAPACITY;
        return (buffer->thebuffer)[buffer->head];
    }
}

// ringbuffer_add until 0x0A
char* ringbuffer_produce(Ringbuffer *buffer, char *start) // return &0x0A+1
{
    char byte;
    int32_t n = 0;
    
    do {
        byte = start[n];
        if ( !ringbuffer_add(buffer, byte) ) { // buffer is full
            return NULL; 
        }
        n += 1;
    }   while (byte != 0x0A);
    
    //ringbuffer_add(buffer, '\0');
    //printf("%s", buffer->thebuffer);
    
    return &start[n];
}

// 將buffer中的byte一一取出直到end, 並存入start
void ringbuffer_consume(Ringbuffer *buffer, char *start, char end)
{
    // buffer的內容是需要的
    if (start != NULL) { 
        char byte;
        int32_t n = 0;

        byte = ringbuffer_del(buffer);
        while (byte != end) {
            start[n] = byte;
            n++;
            byte = ringbuffer_del(buffer);
        }
        start[n] = '\0';
    }

    // buffer的內容都不需要
    else if (end == 0) {
        ringbuffer_set(buffer);
    }

    // buffer的內容在end之前是不需要的
    else {
        char byte;
        do {
            byte = ringbuffer_del(buffer);
        }   while (byte != end);
    }
}


