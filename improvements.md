# GegeChat Improvements

## Error Handling Enhancement in client.c

### Problem
The original client code had inadequate error handling in the child process (reading task). When `recv()` returned a negative value indicating an error, the code would only print an error message and continue looping, potentially leading to:
- Infinite error loops when network connections fail
- Repeated error messages flooding the console
- No proper handling of server disconnections

### Solution Implemented
Enhanced the error handling logic in the child process (lines 93-114) to properly distinguish between different types of recv() return values:

#### Original Code
```c
if (recv(sd, bufferIn, MAXCHR, 0) < 0) {
    perror("C: child recv error");
} else {
    // Process message...
}
```

#### Improved Code
```c
int bytes_received = recv(sd, bufferIn, MAXCHR, 0);
if (bytes_received < 0) {
    if (errno == EINTR) {
        // Interrupted by signal, continue
        continue;
    } else {
        // Real network error, exit
        perror("C: child recv error");
        exit(5);
    }
} else if (bytes_received == 0) {
    // Connection closed by server
    printf("C: server closed connection\n");
    exit(0);
} else {
    // Process message...
}
```

### Improvements Made

1. **Proper Return Value Handling**
   - Store `recv()` return value in `bytes_received` variable
   - Handle three distinct cases: error (<0), connection closed (0), and data received (>0)

2. **Signal Interruption Handling**
   - Check for `EINTR` errno when `recv()` returns -1
   - Continue operation if interrupted by signal (normal behavior)
   - Only exit on real network errors

3. **Server Disconnection Detection**
   - Detect when server closes connection cleanly (`recv()` returns 0)
   - Exit gracefully with status 0 instead of treating as error

4. **Network Error Management**
   - Exit child process on real network errors with status 5
   - Prevent infinite error loops
   - Provide clear error messages

### Benefits

- **Robustness**: Client handles network failures gracefully
- **Standards Compliance**: Follows Unix network programming best practices  
- **User Experience**: Clear feedback on different disconnection scenarios
- **Resource Management**: Prevents runaway processes consuming CPU
- **Debugging**: Appropriate exit codes for different failure modes

### Exit Codes Reference

- `exit(0)`: Normal termination (server closed connection)
- `exit(4)`: Normal termination (received ACK_S "OK" message)
- `exit(5)`: Network error in child process

This enhancement makes the GegeChat client more reliable and suitable for real-world network conditions while maintaining the original fork-based architecture.

## Race Condition Fix: Timeout-based Shutdown

### Problem
The original client had a race condition during shutdown that could cause the application to hang indefinitely:

1. Parent process sends "exit\n" to server and exits main loop
2. Parent process calls `wait()` and blocks, waiting for child to terminate
3. Child process waits for server to send "OK" acknowledgment
4. If server never sends "OK" (due to network issues, server problems, etc.), parent hangs forever

### Original Code
```c
} while (cont);
if (wait(&status) < 0) {
    printf("C: parent wait status 0x%X\n", status);
    perror("C: parent wait error");
} else {
    printf("C: disconnect from server\n");
    close(sd);
}
```

### Improved Code
```c
} while (cont);

// Timeout-based wait for child process
int child_status;
int wait_time = 0;
const int MAX_WAIT_SECONDS = 5;  // 5 second timeout

printf("C: waiting for server acknowledgment...\n");
while (wait_time < MAX_WAIT_SECONDS) {
    pid_t result = waitpid(pid, &child_status, WNOHANG);
    if (result == pid) {
        // Child has terminated normally
        printf("C: disconnect from server\n");
        close(sd);
        return 0;
    } else if (result == -1) {
        perror("C: waitpid error");
        break;
    }
    // Child still running, wait a bit more
    sleep(1);
    wait_time++;
}

if (wait_time >= MAX_WAIT_SECONDS) {
    printf("C: timeout waiting for server response, terminating child\n");
    kill(pid, SIGTERM);  // Send termination signal to child
    sleep(1);            // Give child time to exit gracefully
    if (waitpid(pid, &child_status, WNOHANG) == 0) {
        // Child still running, force kill
        printf("C: forcing child termination\n");
        kill(pid, SIGKILL);
        waitpid(pid, &child_status, 0);
    }
}

close(sd);
```

### Solution Features

1. **Non-blocking Wait**: Uses `waitpid()` with `WNOHANG` to check child status without blocking
2. **Configurable Timeout**: 5-second maximum wait time (easily adjustable)
3. **User Feedback**: Clear messages about shutdown progress
4. **Graceful Termination**: First attempts `SIGTERM` for clean child shutdown
5. **Force Termination**: Uses `SIGKILL` as last resort if child doesn't respond
6. **Proper Cleanup**: Ensures socket closure in all scenarios

### Benefits

- **Prevents Hanging**: Client never waits indefinitely for server response
- **Robust Shutdown**: Handles network failures and unresponsive servers
- **User Experience**: Provides feedback during shutdown process
- **Resource Management**: Guarantees proper cleanup of child process and socket
- **Reliability**: Works correctly even when server-client protocol breaks down

### Shutdown Flow

1. **Normal Case**: Child receives "OK" from server within 5 seconds and exits cleanly
2. **Timeout Case**: After 5 seconds, parent sends `SIGTERM` to child, then `SIGKILL` if needed
3. **Error Case**: Handles `waitpid()` errors gracefully

This improvement eliminates the race condition while preserving the original fork-based architecture and maintaining backward compatibility with properly functioning servers.

## Send Error Handling Enhancement

### Problem
The original client had inadequate error handling in the parent process (writing task) when sending messages to the server. The code would only print an error message and continue in the loop when `send()` failed:

- Could lead to repeated send failures without proper recovery
- No distinction between different types of network errors
- No graceful handling of connection failures
- User would not be informed about connection status

### Original Code
```c
if (send(sd, bufferOut, strlen(bufferOut), 0) < 0) {
    perror("C: parent send error");
}
if (strcmp(bufferOut, MSG_C) == 0) {
    cont = 0;
}
```

### Improved Code
```c
int bytes_sent = send(sd, bufferOut, strlen(bufferOut), 0);
if (bytes_sent < 0) {
    if (errno == EINTR) {
        // Interrupted by signal, try again
        printf("C: send interrupted, retrying...\n");
        continue;
    } else if (errno == EPIPE || errno == ECONNRESET) {
        // Connection broken
        printf("C: connection lost, exiting\n");
        cont = 0;
    } else {
        // Other network error
        perror("C: parent send error");
        printf("C: network error, exiting\n");
        cont = 0;
    }
} else if (bytes_sent == 0) {
    // This shouldn't happen with send(), but handle it
    printf("C: send returned 0, connection may be closed\n");
    cont = 0;
} else {
    // Successful send, check for exit command
    if (strcmp(bufferOut, MSG_C) == 0) {
        cont = 0;
    }
}
```

### Solution Features

1. **Return Value Analysis**: Captures and analyzes `send()` return value
2. **Signal Interruption Handling**: Retries operation when interrupted by signals (`EINTR`)
3. **Connection Failure Detection**: Identifies broken connections (`EPIPE`, `ECONNRESET`)
4. **Graceful Error Handling**: Exits cleanly on network failures instead of looping
5. **User Feedback**: Provides clear messages about error types and actions taken
6. **Edge Case Handling**: Handles the rare case where `send()` returns 0

### Error Categories Handled

- **`EINTR`**: Signal interruption → Retry with user notification
- **`EPIPE`**: Broken pipe (server closed connection) → Graceful exit
- **`ECONNRESET`**: Connection reset by peer → Graceful exit  
- **Other errors**: Generic network failures → Exit with error details
- **Zero return**: Potential connection closure → Graceful exit

### Benefits

- **Robust Network Handling**: Properly handles various network failure scenarios
- **No Infinite Loops**: Prevents repeated failed send attempts
- **User Awareness**: Clear feedback about connection status and errors
- **Graceful Degradation**: Clean shutdown triggers timeout-based child cleanup
- **Signal Safety**: Correctly handles system call interruptions
- **Resource Management**: Ensures proper cleanup when network fails

### Integration with Existing Features

This improvement works seamlessly with the timeout-based shutdown mechanism:
- When parent exits due to send error, it triggers the timeout-based child cleanup
- Maintains the same graceful shutdown flow regardless of exit reason
- Preserves the fork-based architecture while adding network resilience

The enhanced error handling makes the GegeChat client robust against network instability while providing clear feedback to users about connection status.
